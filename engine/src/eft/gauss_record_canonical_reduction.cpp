#include "ftd/eft/gauss_record_canonical_reduction.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  const int result = value % L;
  return result < 0 ? result + L : result;
}

int scalar_index(int L, int x, int y, int z) {
  return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

bool valid_face_shape(const MatchedFaceFlux& field) {
  if (field.L <= 0) return false;
  const std::size_t expected = static_cast<std::size_t>(
      field.L * field.L * field.L);
  return field.x.size() == expected && field.y.size() == expected
      && field.z.size() == expected;
}

bool finite_values(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool finite_face(const MatchedFaceFlux& field) {
  return valid_face_shape(field) && finite_values(field.x)
      && finite_values(field.y) && finite_values(field.z);
}

long double dot(const std::vector<double>& first,
                const std::vector<double>& second) {
  long double result = 0.0L;
  for (std::size_t index = 0; index < first.size(); ++index) {
    result += static_cast<long double>(first[index]) * second[index];
  }
  return result;
}

long double face_pairing(const MatchedFaceFlux& first,
                         const MatchedFaceFlux& second) {
  long double result = 0.0L;
  for (std::size_t index = 0; index < first.x.size(); ++index) {
    result += static_cast<long double>(first.x[index]) * second.x[index];
    result += static_cast<long double>(first.y[index]) * second.y[index];
    result += static_cast<long double>(first.z[index]) * second.z[index];
  }
  return result;
}

double maximum_abs(const std::vector<double>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

double maximum_face_difference(const MatchedFaceFlux& first,
                               const MatchedFaceFlux& second) {
  double result = 0.0;
  for (std::size_t index = 0; index < first.x.size(); ++index) {
    result = std::max({result,
        std::abs(first.x[index] - second.x[index]),
        std::abs(first.y[index] - second.y[index]),
        std::abs(first.z[index] - second.z[index])});
  }
  return result;
}

std::vector<double> divergence_vector(const MatchedFaceFlux& field) {
  std::vector<double> result(field.x.size(), 0.0);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const int i = field.index(x, y, z);
        result[static_cast<std::size_t>(i)] = divergence_at(field, x, y, z);
      }
    }
  }
  return result;
}

void subtract_mean(std::vector<double>& values) {
  if (values.empty()) return;
  long double sum = 0.0L;
  for (double value : values) sum += value;
  const double mean = static_cast<double>(sum / values.size());
  for (double& value : values) value -= mean;
}

void apply_laplacian(int L, const std::vector<double>& scalar,
                     std::vector<double>& result) {
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = scalar_index(L, x, y, z);
        result[static_cast<std::size_t>(i)] =
            6.0 * scalar[static_cast<std::size_t>(i)]
            - scalar[static_cast<std::size_t>(scalar_index(L, x + 1, y, z))]
            - scalar[static_cast<std::size_t>(scalar_index(L, x - 1, y, z))]
            - scalar[static_cast<std::size_t>(scalar_index(L, x, y + 1, z))]
            - scalar[static_cast<std::size_t>(scalar_index(L, x, y - 1, z))]
            - scalar[static_cast<std::size_t>(scalar_index(L, x, y, z + 1))]
            - scalar[static_cast<std::size_t>(scalar_index(L, x, y, z - 1))];
      }
    }
  }
}

MatchedFaceFlux incidence_adjoint(int L, const std::vector<double>& scalar) {
  MatchedFaceFlux result(L);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = result.index(x, y, z);
        const double value = scalar[static_cast<std::size_t>(i)];
        result.x[static_cast<std::size_t>(i)] = value
            - scalar[static_cast<std::size_t>(result.index(x + 1, y, z))];
        result.y[static_cast<std::size_t>(i)] = value
            - scalar[static_cast<std::size_t>(result.index(x, y + 1, z))];
        result.z[static_cast<std::size_t>(i)] = value
            - scalar[static_cast<std::size_t>(result.index(x, y, z + 1))];
      }
    }
  }
  return result;
}

struct MeanZeroSolve {
  bool valid = false;
  bool compatible = false;
  bool converged = false;
  std::vector<double> potential;
  double residual = std::numeric_limits<double>::infinity();
};

MeanZeroSolve solve_mean_zero_laplacian(
    int L, const std::vector<double>& source,
    double tolerance, int max_iterations) {
  MeanZeroSolve result;
  const std::size_t count = static_cast<std::size_t>(L * L * L);
  result.potential.assign(count, 0.0);
  if (L <= 0 || source.size() != count || !finite_values(source)
      || !std::isfinite(tolerance) || tolerance <= 0.0) {
    return result;
  }

  long double sum = 0.0L;
  for (double value : source) sum += value;
  const double scale = std::max(1.0, maximum_abs(source));
  result.compatible = std::abs(static_cast<double>(sum))
      <= tolerance * scale * static_cast<double>(count);
  if (!result.compatible) return result;

  std::vector<double> residual = source;
  subtract_mean(residual);
  result.residual = maximum_abs(residual);
  if (result.residual <= tolerance * scale) {
    result.valid = true;
    result.converged = true;
    return result;
  }

  if (max_iterations <= 0) max_iterations = 8 * static_cast<int>(count);
  std::vector<double> direction = residual;
  std::vector<double> image(count, 0.0);
  long double rr = dot(residual, residual);
  for (int iteration = 0; iteration < max_iterations; ++iteration) {
    apply_laplacian(L, direction, image);
    const long double pAp = dot(direction, image);
    if (!(pAp > 0.0L) || !std::isfinite(static_cast<double>(pAp))) break;
    const long double alpha = rr / pAp;
    for (std::size_t i = 0; i < count; ++i) {
      result.potential[i] += static_cast<double>(alpha * direction[i]);
      residual[i] -= static_cast<double>(alpha * image[i]);
    }
    subtract_mean(result.potential);
    subtract_mean(residual);
    result.residual = maximum_abs(residual);
    if (result.residual <= tolerance * scale) {
      result.converged = true;
      break;
    }
    const long double rr_next = dot(residual, residual);
    if (!(rr_next >= 0.0L) || !std::isfinite(static_cast<double>(rr_next))) {
      break;
    }
    const long double beta = rr_next / rr;
    for (std::size_t i = 0; i < count; ++i) {
      direction[i] = residual[i] + static_cast<double>(beta * direction[i]);
    }
    subtract_mean(direction);
    rr = rr_next;
  }
  result.valid = result.compatible && result.converged
      && finite_values(result.potential);
  return result;
}

MatchedFaceFlux add_faces(const MatchedFaceFlux& first,
                          const MatchedFaceFlux& second,
                          double second_scale = 1.0) {
  MatchedFaceFlux result = first;
  for (std::size_t index = 0; index < result.x.size(); ++index) {
    result.x[index] += second_scale * second.x[index];
    result.y[index] += second_scale * second.y[index];
    result.z[index] += second_scale * second.z[index];
  }
  return result;
}

double maximum_charge_residual(const MatchedFaceFlux& field,
                               const std::vector<double>& target) {
  const auto measured = divergence_vector(field);
  double result = 0.0;
  for (std::size_t index = 0; index < target.size(); ++index) {
    result = std::max(result, std::abs(measured[index] - target[index]));
  }
  return result;
}

}  // namespace

GaussRecordCanonicalDecomposition decompose_matched_gauss_canonical(
    const MatchedFaceFlux& flux,
    const MatchedFaceFlux& momentum,
    double tolerance,
    int max_iterations) {
  GaussRecordCanonicalDecomposition result;
  result.L = flux.L;
  result.flux = flux;
  result.momentum = momentum;
  result.longitudinal_flux = MatchedFaceFlux(flux.L);
  result.longitudinal_momentum = MatchedFaceFlux(flux.L);
  result.transverse_flux = MatchedFaceFlux(flux.L);
  result.transverse_momentum = MatchedFaceFlux(flux.L);
  if (flux.L <= 0) return result;
  if (!std::isfinite(tolerance) || tolerance <= 0.0) {
    result.status = GaussRecordReductionStatus::InvalidTolerance;
    return result;
  }
  if (flux.L != momentum.L || !valid_face_shape(flux)
      || !valid_face_shape(momentum)) {
    result.status = GaussRecordReductionStatus::ShapeMismatch;
    return result;
  }
  if (!finite_face(flux) || !finite_face(momentum)) {
    result.status = GaussRecordReductionStatus::NonFiniteInput;
    return result;
  }

  result.charge = divergence_vector(flux);
  const auto momentum_divergence = divergence_vector(momentum);
  const auto charge_solve = solve_mean_zero_laplacian(
      flux.L, result.charge, tolerance, max_iterations);
  const auto momentum_solve = solve_mean_zero_laplacian(
      flux.L, momentum_divergence, tolerance, max_iterations);
  if (!charge_solve.compatible || !momentum_solve.compatible) {
    result.status = GaussRecordReductionStatus::IncompatibleCharge;
    return result;
  }
  if (!charge_solve.valid || !momentum_solve.valid) {
    result.status = GaussRecordReductionStatus::SolverFailure;
    return result;
  }

  result.charge_momentum = momentum_solve.potential;
  result.longitudinal_flux = incidence_adjoint(
      flux.L, charge_solve.potential);
  result.longitudinal_momentum = incidence_adjoint(
      flux.L, result.charge_momentum);
  result.transverse_flux = add_faces(flux, result.longitudinal_flux, -1.0);
  result.transverse_momentum = add_faces(
      momentum, result.longitudinal_momentum, -1.0);

  const auto reconstructed_flux = add_faces(
      result.transverse_flux, result.longitudinal_flux);
  const auto reconstructed_momentum = add_faces(
      result.transverse_momentum, result.longitudinal_momentum);
  result.maximum_flux_reconstruction_residual = maximum_face_difference(
      flux, reconstructed_flux);
  result.maximum_momentum_reconstruction_residual = maximum_face_difference(
      momentum, reconstructed_momentum);
  result.maximum_transverse_flux_divergence = max_divergence(
      result.transverse_flux);
  result.maximum_transverse_momentum_divergence = max_divergence(
      result.transverse_momentum);
  result.longitudinal_transverse_pairing_residual = std::max(
      std::abs(static_cast<double>(face_pairing(
          result.longitudinal_flux, result.transverse_momentum))),
      std::abs(static_cast<double>(face_pairing(
          result.transverse_flux, result.longitudinal_momentum))));
  const double scale = std::max({1.0, l1_norm(flux), l1_norm(momentum)});
  result.charge_bracket_identity_on_mean_zero_space = true;
  result.canonical_split_verified =
      result.maximum_flux_reconstruction_residual <= tolerance * scale
      && result.maximum_momentum_reconstruction_residual <= tolerance * scale
      && result.maximum_transverse_flux_divergence <= tolerance * scale
      && result.maximum_transverse_momentum_divergence <= tolerance * scale
      && result.longitudinal_transverse_pairing_residual <= tolerance * scale;
  result.status = result.canonical_split_verified
      ? GaussRecordReductionStatus::Valid
      : GaussRecordReductionStatus::SolverFailure;
  return result;
}

StaticTernaryGaussRecord make_static_ternary_gauss_record(
    int L,
    const std::vector<int>& ternary_state,
    double coupling,
    double tolerance,
    int max_iterations) {
  StaticTernaryGaussRecord result;
  result.L = L;
  result.ternary_state = ternary_state;
  result.coupling = coupling;
  result.flux = MatchedFaceFlux(L);
  result.momentum = MatchedFaceFlux(L);
  if (L <= 0) return result;
  const std::size_t count = static_cast<std::size_t>(L * L * L);
  if (ternary_state.size() != count) {
    result.status = GaussRecordReductionStatus::ShapeMismatch;
    return result;
  }
  if (!std::isfinite(tolerance) || tolerance <= 0.0) {
    result.status = GaussRecordReductionStatus::InvalidTolerance;
    return result;
  }
  if (!std::isfinite(coupling) || coupling == 0.0) {
    result.status = GaussRecordReductionStatus::InvalidCoupling;
    return result;
  }
  long long state_sum = 0;
  for (int value : ternary_state) {
    if (value < -1 || value > 1) {
      result.status = GaussRecordReductionStatus::InvalidTernaryState;
      return result;
    }
    state_sum += value;
  }
  result.mean_state = static_cast<double>(state_sum)
      / static_cast<double>(count);
  result.neutral_without_background = state_sum == 0;
  result.background_subtracted = state_sum != 0;
  result.compatible_charge.resize(count);
  for (std::size_t index = 0; index < count; ++index) {
    result.compatible_charge[index] = coupling
        * (static_cast<double>(ternary_state[index]) - result.mean_state);
  }
  const auto solve = solve_mean_zero_laplacian(
      L, result.compatible_charge, tolerance, max_iterations);
  if (!solve.compatible) {
    result.status = GaussRecordReductionStatus::IncompatibleCharge;
    return result;
  }
  if (!solve.valid) {
    result.status = GaussRecordReductionStatus::SolverFailure;
    return result;
  }
  result.flux = incidence_adjoint(L, solve.potential);
  result.maximum_gauss_residual = maximum_charge_residual(
      result.flux, result.compatible_charge);
  result.static_charge_momentum_zero = l1_norm(result.momentum) == 0.0;
  result.minimum_energy_longitudinal =
      max_curl_adjoint(result.flux) <= 10.0 * tolerance;
  const double scale = std::max(1.0, maximum_abs(result.compatible_charge));
  result.status = result.maximum_gauss_residual <= 10.0 * tolerance * scale
      && result.static_charge_momentum_zero
      && result.minimum_energy_longitudinal
      ? GaussRecordReductionStatus::Valid
      : GaussRecordReductionStatus::SolverFailure;
  return result;
}

GaussRecordPreparationLedger prepare_matched_gauss_record(
    const MatchedFaceFlux& input,
    const std::vector<double>& target_charge,
    double tolerance,
    int max_iterations) {
  GaussRecordPreparationLedger result;
  result.input = input;
  result.prepared = MatchedFaceFlux(input.L);
  result.discarded_longitudinal_discrepancy = MatchedFaceFlux(input.L);
  result.recovered = MatchedFaceFlux(input.L);
  result.target_charge = target_charge;
  if (input.L <= 0) return result;
  if (!std::isfinite(tolerance) || tolerance <= 0.0) {
    result.status = GaussRecordReductionStatus::InvalidTolerance;
    return result;
  }
  if (!finite_face(input) || !finite_values(target_charge)) {
    result.status = GaussRecordReductionStatus::NonFiniteInput;
    return result;
  }
  if (target_charge.size() != input.x.size()) {
    result.status = GaussRecordReductionStatus::ShapeMismatch;
    return result;
  }
  long double target_sum = 0.0L;
  for (double value : target_charge) target_sum += value;
  const double scale = std::max(1.0, maximum_abs(target_charge));
  if (std::abs(static_cast<double>(target_sum))
      > tolerance * scale * static_cast<double>(target_charge.size())) {
    result.status = GaussRecordReductionStatus::IncompatibleCharge;
    return result;
  }

  auto discrepancy_charge = divergence_vector(input);
  for (std::size_t index = 0; index < discrepancy_charge.size(); ++index) {
    discrepancy_charge[index] -= target_charge[index];
  }
  const auto solve = solve_mean_zero_laplacian(
      input.L, discrepancy_charge, tolerance, max_iterations);
  if (!solve.compatible) {
    result.status = GaussRecordReductionStatus::IncompatibleCharge;
    return result;
  }
  if (!solve.valid) {
    result.status = GaussRecordReductionStatus::SolverFailure;
    return result;
  }
  result.discarded_longitudinal_discrepancy = incidence_adjoint(
      input.L, solve.potential);
  result.prepared = add_faces(
      input, result.discarded_longitudinal_discrepancy, -1.0);
  result.recovered = add_faces(
      result.prepared, result.discarded_longitudinal_discrepancy);
  result.maximum_target_residual = maximum_charge_residual(
      result.prepared, target_charge);
  result.maximum_recovery_residual = maximum_face_difference(
      input, result.recovered);
  result.maximum_discrepancy_curl_adjoint = max_curl_adjoint(
      result.discarded_longitudinal_discrepancy);

  auto second_discrepancy_charge = divergence_vector(result.prepared);
  for (std::size_t index = 0; index < second_discrepancy_charge.size(); ++index) {
    second_discrepancy_charge[index] -= target_charge[index];
  }
  const auto second_solve = solve_mean_zero_laplacian(
      input.L, second_discrepancy_charge, tolerance, max_iterations);
  const auto second_discrepancy = second_solve.valid
      ? incidence_adjoint(input.L, second_solve.potential)
      : MatchedFaceFlux(input.L);
  result.affine_projection_idempotent = second_solve.valid
      && l1_norm(second_discrepancy)
          <= 10.0 * tolerance * std::max(1.0, l1_norm(result.prepared));
  result.reversible_with_discrepancy_ledger =
      result.maximum_recovery_residual <= 10.0 * tolerance
      * std::max(1.0, l1_norm(input));
  result.status = result.maximum_target_residual <= 10.0 * tolerance * scale
      && result.maximum_discrepancy_curl_adjoint <= 10.0 * tolerance * scale
      && result.affine_projection_idempotent
      && result.reversible_with_discrepancy_ledger
      ? GaussRecordReductionStatus::Valid
      : GaussRecordReductionStatus::SolverFailure;
  return result;
}

double gauss_canonical_symplectic_pairing(
    const MatchedFaceFlux& delta_flux_first,
    const MatchedFaceFlux& delta_momentum_first,
    const MatchedFaceFlux& delta_flux_second,
    const MatchedFaceFlux& delta_momentum_second) {
  if (delta_flux_first.L != delta_momentum_first.L
      || delta_flux_first.L != delta_flux_second.L
      || delta_flux_first.L != delta_momentum_second.L
      || !finite_face(delta_flux_first) || !finite_face(delta_momentum_first)
      || !finite_face(delta_flux_second) || !finite_face(delta_momentum_second)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return static_cast<double>(
      face_pairing(delta_flux_first, delta_momentum_second)
      - face_pairing(delta_momentum_first, delta_flux_second));
}

double reduced_gauss_symplectic_pairing(
    const GaussRecordCanonicalDecomposition& first,
    const GaussRecordCanonicalDecomposition& second) {
  if (!first.valid() || !second.valid() || first.L != second.L
      || first.charge.size() != second.charge.size()
      || first.charge_momentum.size() != second.charge_momentum.size()) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  const long double transverse =
      face_pairing(first.transverse_flux, second.transverse_momentum)
      - face_pairing(first.transverse_momentum, second.transverse_flux);
  const long double charge = dot(first.charge, second.charge_momentum)
      - dot(first.charge_momentum, second.charge);
  return static_cast<double>(transverse + charge);
}

ProductionGaussSymbolBoundary production_gauss_symbol_boundary() {
  return {};
}

}  // namespace ftd::eft

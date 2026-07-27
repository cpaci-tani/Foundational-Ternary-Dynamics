#include "ftd/eft/quadratic_coat_neutral_pair_work.h"

#include "ftd/eft/matched_face_energy_transaction.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>

namespace ftd::eft {
namespace {

int wrap(int coordinate, int L) {
  const int remainder = coordinate % L;
  return remainder < 0 ? remainder + L : remainder;
}

std::size_t index(int L, int x, int y, int z) {
  return static_cast<std::size_t>(
      (wrap(x, L) * L + wrap(y, L)) * L + wrap(z, L));
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool finite(const MatchedFaceFlux& field) {
  return finite(field.x) && finite(field.y) && finite(field.z);
}

void add(MatchedFaceFlux& target,
         const MatchedFaceFlux& value,
         double scale = 1.0) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

void add_current(MatchedFaceFlux& target,
                 const QuadraticCoatFaceCurrent& current,
                 double scale = 1.0) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * current.current_x[i];
    target.y[i] += scale * current.current_y[i];
    target.z[i] += scale * current.current_z[i];
  }
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  return matched_face_max_difference(lhs, rhs);
}

double max_gauss_residual(const MatchedFaceFlux& field,
                          const std::vector<double>& source) {
  if (field.x.size() != source.size()) return INFINITY;
  double residual = 0.0;
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto i = static_cast<std::size_t>(field.index(x, y, z));
        residual = std::max(residual, std::abs(
            divergence_at(field, x, y, z) - source[i]));
      }
    }
  }
  return residual;
}

void apply_negative_laplacian(int L,
                              const std::vector<double>& input,
                              std::vector<double>& output) {
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = index(L, x, y, z);
        output[i] = 6.0 * input[i]
            - input[index(L, x + 1, y, z)]
            - input[index(L, x - 1, y, z)]
            - input[index(L, x, y + 1, z)]
            - input[index(L, x, y - 1, z)]
            - input[index(L, x, y, z + 1)]
            - input[index(L, x, y, z - 1)];
      }
    }
  }
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

double max_abs(const std::vector<double>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

struct PoissonResult {
  bool converged = false;
  int iterations = 0;
  double residual = INFINITY;
  std::vector<double> potential;
};

PoissonResult solve_neutral_poisson(int L,
                                    const std::vector<double>& source,
                                    double tolerance,
                                    int max_iterations) {
  PoissonResult result;
  const std::size_t count = source.size();
  result.potential.assign(count, 0.0);
  std::vector<double> residual = source;
  const long double source_mean = std::accumulate(
      source.begin(), source.end(), 0.0L) / count;
  for (double& value : residual)
    value -= static_cast<double>(source_mean);
  std::vector<double> direction = residual;
  std::vector<double> applied(count, 0.0);
  long double residual_squared = dot(residual, residual);
  result.residual = max_abs(residual);
  if (result.residual <= tolerance) {
    result.converged = true;
    return result;
  }
  for (int iteration = 1; iteration <= max_iterations; ++iteration) {
    apply_negative_laplacian(L, direction, applied);
    const long double denominator = dot(direction, applied);
    if (!(denominator > 0.0L) || !std::isfinite(
            static_cast<double>(denominator))) break;
    const long double alpha = residual_squared / denominator;
    for (std::size_t i = 0; i < count; ++i) {
      result.potential[i] += static_cast<double>(alpha * direction[i]);
      residual[i] -= static_cast<double>(alpha * applied[i]);
    }
    result.iterations = iteration;
    result.residual = max_abs(residual);
    if (result.residual <= tolerance) {
      result.converged = true;
      break;
    }
    const long double next_squared = dot(residual, residual);
    if (!(next_squared >= 0.0L) || !std::isfinite(
            static_cast<double>(next_squared))) break;
    const long double ratio = next_squared / residual_squared;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i]
          + static_cast<double>(ratio * direction[i]);
    residual_squared = next_squared;
  }
  const long double potential_mean = std::accumulate(
      result.potential.begin(), result.potential.end(), 0.0L) / count;
  for (double& value : result.potential)
    value -= static_cast<double>(potential_mean);
  return result;
}

double energy_change(const QuadraticCoatMatterWorkResult& matter) {
  return matter.matter_energy_after - matter.matter_energy_before;
}

}  // namespace

QuadraticCoatNeutralPairWorkResult::QuadraticCoatNeutralPairWorkResult(
    int size, double time_scale)
    : L(size), temporal_scale(time_scale),
      slab(size, time_scale),
      current_start(size), current_end(size), current_total(size),
      electric_slab(size), electric_before(size), electric_after(size),
      electric_midpoint(size) {
  const std::size_t count = size > 0
      ? static_cast<std::size_t>(size) * size * size : 0;
  rho_before.assign(count, 0.0);
  rho_after.assign(count, 0.0);
  temporal_charge.assign(count, 0.0);
}

QuadraticCoatNeutralPairWorkResult
evaluate_quadratic_coat_neutral_pair_work(
    int L,
    const std::array<Vec3, 2>& start_position,
    const std::array<Vec3, 2>& end_position,
    const std::array<int, 2>& charge,
    double rest_energy,
    double c_speed,
    double beta,
    double poisson_tolerance,
    int poisson_max_iterations) {
  QuadraticCoatNeutralPairWorkResult result(L, c_speed);
  result.start_position = start_position;
  result.end_position = end_position;
  result.charge = charge;
  result.beta = beta;
  const bool input_valid = L >= 5 && charge[0] + charge[1] == 0
      && (charge[0] == -1 || charge[0] == 1)
      && (charge[1] == -1 || charge[1] == 1)
      && finite(start_position[0]) && finite(start_position[1])
      && finite(end_position[0]) && finite(end_position[1])
      && rest_energy > 0.0 && std::isfinite(rest_energy)
      && c_speed > 0.0 && std::isfinite(c_speed)
      && beta > 0.0 && std::isfinite(beta)
      && poisson_tolerance > 0.0 && std::isfinite(poisson_tolerance)
      && poisson_max_iterations > 0;
  if (!input_valid) return result;

  for (std::size_t carrier = 0; carrier < 2; ++carrier) {
    result.current[carrier] = make_quadratic_coat_spacetime_current(
        L, start_position[carrier], end_position[carrier], charge[carrier],
        c_speed);
    if (!result.current[carrier].valid
        || result.current[carrier].spatial.causal_excess > 0.0) return result;
    add(result.current_start, result.current[carrier].spatial_start);
    add(result.current_end, result.current[carrier].spatial_end);
    add_current(result.current_total, result.current[carrier].spatial);
    for (std::size_t i = 0; i < result.rho_before.size(); ++i) {
      result.rho_before[i] +=
          result.current[carrier].spatial.rho_before[i];
      result.rho_after[i] +=
          result.current[carrier].spatial.rho_after[i];
      result.temporal_charge[i] +=
          result.current[carrier].temporal_charge[i];
    }
    result.split_continuity_residual = std::max({
        result.split_continuity_residual,
        result.current[carrier].spatial_split_residual,
        result.current[carrier].split_continuity_start_residual,
        result.current[carrier].split_continuity_end_residual});
  }

  result.neutrality_residual = std::abs(static_cast<double>(
      std::accumulate(result.temporal_charge.begin(),
                      result.temporal_charge.end(), 0.0L)));
  if (result.neutrality_residual > 1e-12) return result;
  const PoissonResult poisson = solve_neutral_poisson(
      L, result.temporal_charge, poisson_tolerance,
      poisson_max_iterations);
  result.poisson_converged = poisson.converged;
  result.poisson_iterations = poisson.iterations;
  result.poisson_residual = poisson.residual;
  result.slab.Phi = poisson.potential;
  if (!poisson.converged) return result;

  result.electric_slab = slab_electric_field(result.slab);
  result.temporal_gauss_residual = max_gauss_residual(
      result.electric_slab, result.temporal_charge);
  result.electric_before = result.electric_slab;
  add(result.electric_before, result.current_start);
  result.electric_after = result.electric_slab;
  add(result.electric_after, result.current_end, -1.0);
  result.electric_midpoint = result.electric_before;
  add(result.electric_midpoint, result.electric_after);
  for (std::size_t i = 0; i < result.electric_midpoint.x.size(); ++i) {
    result.electric_midpoint.x[i] *= 0.5;
    result.electric_midpoint.y[i] *= 0.5;
    result.electric_midpoint.z[i] *= 0.5;
  }

  result.endpoint_gauss_residual = std::max(
      max_gauss_residual(result.electric_before, result.rho_before),
      max_gauss_residual(result.electric_after, result.rho_after));
  MatchedFaceFlux predicted_after = result.electric_before;
  add(predicted_after, result.current_total, -1.0);
  result.field_update_residual = max_difference(
      predicted_after, result.electric_after);
  MatchedFaceFlux predicted_midpoint = result.electric_slab;
  add(predicted_midpoint, result.current_start, 0.5);
  add(predicted_midpoint, result.current_end, -0.5);
  result.midpoint_split_residual = max_difference(
      predicted_midpoint, result.electric_midpoint);
  for (std::size_t i = 0; i < result.temporal_charge.size(); ++i) {
    result.temporal_endpoint_average_mismatch = std::max(
        result.temporal_endpoint_average_mismatch,
        std::abs(result.temporal_charge[i]
            - 0.5 * (result.rho_before[i] + result.rho_after[i])));
    const int x = static_cast<int>(i) / (L * L);
    const int yz = static_cast<int>(i) % (L * L);
    const int y = yz / L;
    const int z = yz % L;
    result.split_continuity_residual = std::max({
        result.split_continuity_residual,
        std::abs(divergence_at(result.current_start, x, y, z)
            + result.temporal_charge[i] - result.rho_before[i]),
        std::abs(divergence_at(result.current_end, x, y, z)
            - result.temporal_charge[i] + result.rho_after[i])});
  }

  for (std::size_t carrier = 0; carrier < 2; ++carrier) {
    result.matter[carrier] = evaluate_quadratic_coat_matter_work(
        start_position[carrier], end_position[carrier], charge[carrier],
        rest_energy, c_speed, beta, result.slab);
    if (!result.matter[carrier].valid) return result;
    result.action_residual = std::max(
        result.action_residual,
        result.matter[carrier].deposited_action_residual);
    result.matter_energy_change += energy_change(result.matter[carrier]);
  }
  result.field_energy_change = beta * (
      quadratic_energy(result.electric_after)
      - quadratic_energy(result.electric_before));
  result.field_work = beta * static_cast<double>(
      matched_face_dot(result.electric_midpoint, result.current_total));
  result.field_work_residual = std::abs(
      result.field_energy_change + result.field_work);
  result.pair_matter_work_defect = result.matter_energy_change
      - result.field_work;
  result.total_energy_defect = result.matter_energy_change
      + result.field_energy_change;

  result.valid = result.poisson_converged
      && finite(result.electric_slab) && finite(result.electric_before)
      && finite(result.electric_after) && finite(result.electric_midpoint)
      && finite(result.rho_before) && finite(result.rho_after)
      && finite(result.temporal_charge)
      && std::isfinite(result.poisson_residual)
      && std::isfinite(result.neutrality_residual)
      && std::isfinite(result.temporal_gauss_residual)
      && std::isfinite(result.split_continuity_residual)
      && std::isfinite(result.endpoint_gauss_residual)
      && std::isfinite(result.field_update_residual)
      && std::isfinite(result.midpoint_split_residual)
      && std::isfinite(result.action_residual)
      && std::isfinite(result.matter_energy_change)
      && std::isfinite(result.field_energy_change)
      && std::isfinite(result.field_work)
      && std::isfinite(result.field_work_residual)
      && std::isfinite(result.pair_matter_work_defect)
      && std::isfinite(result.total_energy_defect);
  return result;
}

}  // namespace ftd::eft

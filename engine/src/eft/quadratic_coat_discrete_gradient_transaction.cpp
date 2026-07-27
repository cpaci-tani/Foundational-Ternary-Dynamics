#include "ftd/eft/quadratic_coat_discrete_gradient_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

void set_component(Vec3& value, int axis, double entry) {
  if (axis == 0) value.x = entry;
  else if (axis == 1) value.y = entry;
  else value.z = entry;
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

std::size_t expected_volume(int L) {
  return L > 0 ? static_cast<std::size_t>(L)*L*L : 0;
}

template <typename Field>
bool finite_field(const Field& field) {
  const std::size_t expected = expected_volume(field.L);
  if (field.L <= 0 || field.x.size() != expected
      || field.y.size() != expected || field.z.size() != expected)
    return false;
  const auto values_finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  return values_finite(field.x) && values_finite(field.y)
      && values_finite(field.z);
}

int wrap(int value, int L) {
  const int remainder = value%L;
  return remainder < 0 ? remainder+L : remainder;
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

MatchedMatterPoint point_at(const Vec3& position,
                            int L, const Vec3& momentum) {
  MatchedMatterPoint result;
  const long long x = std::llround(position.x);
  const long long y = std::llround(position.y);
  const long long z = std::llround(position.z);
  result.anchor = {wrap(static_cast<int>(x), L),
                   wrap(static_cast<int>(y), L),
                   wrap(static_cast<int>(z), L)};
  result.remainder = {position.x-x, position.y-y, position.z-z};
  result.momentum = momentum;
  return result;
}

double periodic_difference(double lhs, double rhs, int L) {
  double value = lhs-rhs;
  return value-std::round(value/L)*L;
}

double point_residual(const MatchedMatterPoint& lhs,
                      const MatchedMatterPoint& rhs, int L) {
  const Vec3 a = effective_position(lhs);
  const Vec3 b = effective_position(rhs);
  return std::max({std::abs(periodic_difference(a.x, b.x, L)),
      std::abs(periodic_difference(a.y, b.y, L)),
      std::abs(periodic_difference(a.z, b.z, L)),
      maximum_component(lhs.momentum-rhs.momentum)});
}

MatchedFaceFlux midpoint(const MatchedFaceFlux& lhs,
                         const MatchedFaceFlux& rhs) {
  MatchedFaceFlux result(lhs.L);
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result.x[i] = 0.5*(lhs.x[i]+rhs.x[i]);
    result.y[i] = 0.5*(lhs.y[i]+rhs.y[i]);
    result.z[i] = 0.5*(lhs.z[i]+rhs.z[i]);
  }
  return result;
}

void add_current(MatchedFaceFlux& field,
                 const QuadraticCoatFaceCurrent& segment,
                 double scale) {
  for (std::size_t i = 0; i < field.x.size(); ++i) {
    field.x[i] += scale*segment.current_x[i];
    field.y[i] += scale*segment.current_y[i];
    field.z[i] += scale*segment.current_z[i];
  }
}

std::vector<double> total_density(const std::vector<double>& moving,
                                  const std::vector<double>& stationary) {
  std::vector<double> result = moving;
  for (std::size_t i = 0; i < result.size(); ++i) result[i] += stationary[i];
  return result;
}

struct PreparedFields {
  MatchedEdgeField magnetic_after;
  MatchedFaceFlux electric_pre_current;
};

PreparedFields prepare_fields(const CoupledMatchedFaceState& before,
                              double lambda) {
  PreparedFields result{before.magnetic_half, before.electric};
  const MatchedEdgeField electric_curl = matched_curl_adjoint(before.electric);
  for (std::size_t i = 0; i < result.magnetic_after.x.size(); ++i) {
    result.magnetic_after.x[i] -= lambda*electric_curl.x[i];
    result.magnetic_after.y[i] -= lambda*electric_curl.y[i];
    result.magnetic_after.z[i] -= lambda*electric_curl.z[i];
  }
  const MatchedFaceFlux magnetic_curl = matched_curl(result.magnetic_after);
  for (std::size_t i = 0; i < result.electric_pre_current.x.size(); ++i) {
    result.electric_pre_current.x[i] += lambda*magnetic_curl.x[i];
    result.electric_pre_current.y[i] += lambda*magnetic_curl.y[i];
    result.electric_pre_current.z[i] += lambda*magnetic_curl.z[i];
  }
  return result;
}

struct Candidate {
  bool valid = false;
  Vec3 momentum_after{};
  Vec3 velocity{};
  Vec3 residual{};
  Vec3 electric_impulse{};
  Vec3 magnetic_impulse{};
  Vec3 total_impulse{};
  MatchedMatterPoint matter_after{};
  QuadraticCoatFaceCurrent segment{};
  QuadraticCoatOrbitGatherResult gather{};
  MatchedFaceFlux electric_after{};
  MatchedEdgeField magnetic_after{};
};

Candidate evaluate_candidate(const CoupledMatchedFaceState& before,
                             int charge,
                             const QuadraticCoatDGOptions& options,
                             const PreparedFields& prepared,
                             double interaction_scale,
                             const Vec3& momentum_after) {
  Candidate candidate;
  candidate.momentum_after = momentum_after;
  const double energy_before = production_flat_energy_from_momentum(
      before.matter.momentum);
  const double energy_after = production_flat_energy_from_momentum(
      momentum_after);
  const double denominator = energy_before+energy_after;
  if (!(denominator > 0.0) || !std::isfinite(denominator)) return candidate;
  candidate.velocity = (before.matter.momentum+momentum_after)
      *(C_SPEED*C_SPEED/denominator);
  const Vec3 start = effective_position(before.matter);
  const Vec3 end = start+candidate.velocity*options.dt;
  candidate.matter_after = point_at(
      end, before.electric.L, momentum_after);
  candidate.segment = make_quadratic_coat_face_current(
      before.electric.L, start, end, charge);
  if (!candidate.segment.valid) return candidate;

  candidate.electric_after = prepared.electric_pre_current;
  add_current(candidate.electric_after, candidate.segment, -1.0);
  candidate.magnetic_after = prepared.magnetic_after;
  const MatchedFaceFlux electric_midpoint = midpoint(
      before.electric, candidate.electric_after);
  candidate.gather = evaluate_quadratic_coat_orbit_gather(
      candidate.segment, electric_midpoint, candidate.magnetic_after,
      candidate.velocity, options.dt, interaction_scale);
  if (!candidate.gather.valid) return candidate;
  candidate.electric_impulse = candidate.gather.electric_force
      *(options.dt*interaction_scale);
  candidate.magnetic_impulse = candidate.gather.magnetic_impulse;
  candidate.total_impulse = candidate.electric_impulse
      +candidate.magnetic_impulse;
  candidate.residual = momentum_after-before.matter.momentum
      -candidate.total_impulse;
  candidate.valid = finite(candidate.velocity) && finite(candidate.residual)
      && finite(candidate.total_impulse) && finite_field(candidate.electric_after)
      && finite_field(candidate.magnetic_after);
  return candidate;
}

double solve_linear(std::array<std::array<double, 3>, 3> matrix,
                    std::array<double, 3> rhs,
                    Vec3& solution) {
  double determinant = 1.0;
  int sign = 1;
  for (int column = 0; column < 3; ++column) {
    int pivot = column;
    for (int row = column+1; row < 3; ++row)
      if (std::abs(matrix[row][column]) > std::abs(matrix[pivot][column]))
        pivot = row;
    if (!std::isfinite(matrix[pivot][column])
        || std::abs(matrix[pivot][column]) < 1e-14) return 0.0;
    if (pivot != column) {
      std::swap(matrix[pivot], matrix[column]);
      std::swap(rhs[pivot], rhs[column]);
      sign = -sign;
    }
    determinant *= matrix[column][column];
    for (int row = column+1; row < 3; ++row) {
      const double factor = matrix[row][column]/matrix[column][column];
      for (int entry = column; entry < 3; ++entry)
        matrix[row][entry] -= factor*matrix[column][entry];
      rhs[row] -= factor*rhs[column];
    }
  }
  std::array<double, 3> values{};
  for (int row = 2; row >= 0; --row) {
    double value = rhs[row];
    for (int column = row+1; column < 3; ++column)
      value -= matrix[row][column]*values[column];
    values[row] = value/matrix[row][row];
  }
  solution = {values[0], values[1], values[2]};
  return sign*determinant;
}

struct RootResult {
  Candidate candidate{};
  LocalImplicitSolveDiagnostics diagnostics{};
};

RootResult solve_root(const Vec3& initial,
                      const QuadraticCoatDGOptions& options,
                      const std::function<Candidate(const Vec3&)>& evaluate) {
  RootResult result;
  result.diagnostics.attempted = true;
  result.diagnostics.minimum_abs_jacobian_determinant = INFINITY;
  Vec3 momentum = initial;
  Candidate current = evaluate(momentum);
  if (!current.valid) {
    result.diagnostics.residual = INFINITY;
    result.diagnostics.minimum_abs_jacobian_determinant = 0.0;
    return result;
  }
  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    const double residual = maximum_component(current.residual);
    result.diagnostics.iterations = iteration;
    result.diagnostics.residual = residual;
    if (residual <= options.solve_tolerance) {
      result.diagnostics.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;
    std::array<std::array<double, 3>, 3> jacobian{};
    bool usable = true;
    for (int axis = 0; axis < 3; ++axis) {
      const double step = options.finite_difference_scale
          *std::max(1.0, std::abs(component(momentum, axis)));
      Vec3 high_momentum = momentum;
      Vec3 low_momentum = momentum;
      set_component(high_momentum, axis,
                    component(momentum, axis)+step);
      set_component(low_momentum, axis,
                    component(momentum, axis)-step);
      const Candidate high = evaluate(high_momentum);
      const Candidate low = evaluate(low_momentum);
      if (!high.valid || !low.valid) { usable = false; break; }
      for (int row = 0; row < 3; ++row)
        jacobian[row][axis] = (component(high.residual, row)
            -component(low.residual, row))/(2.0*step);
    }
    Vec3 step = current.residual*(-1.0);
    const double determinant = usable ? solve_linear(jacobian,
        {{-current.residual.x, -current.residual.y, -current.residual.z}},
        step) : 0.0;
    if (determinant != 0.0 && std::isfinite(determinant))
      result.diagnostics.minimum_abs_jacobian_determinant = std::min(
          result.diagnostics.minimum_abs_jacobian_determinant,
          std::abs(determinant));
    bool accepted = false;
    double scale = 1.0;
    for (int line = 0; line < 16; ++line) {
      const Vec3 trial_momentum = momentum+step*scale;
      Candidate trial = evaluate(trial_momentum);
      if (trial.valid && maximum_component(trial.residual) < residual) {
        result.diagnostics.step_residual = maximum_component(
            trial_momentum-momentum);
        momentum = trial_momentum;
        current = std::move(trial);
        accepted = true;
        break;
      }
      scale *= 0.5;
      ++result.diagnostics.rejected_steps;
    }
    if (!accepted) break;
  }
  result.candidate = std::move(current);
  if (!std::isfinite(result.diagnostics.minimum_abs_jacobian_determinant))
    result.diagnostics.minimum_abs_jacobian_determinant = 0.0;
  return result;
}

}  // namespace

QuadraticCoatDGTransaction solve_quadratic_coat_dg_transaction(
    const CoupledMatchedFaceState& before,
    int charge,
    const std::vector<double>& stationary_density,
    const QuadraticCoatDGOptions& options) {
  QuadraticCoatDGTransaction result;
  result.before = before;
  result.after = CoupledMatchedFaceState(before.electric.L);
  result.charge = charge;
  result.normalization = measure_face_flux_normalization();
  result.interaction_scale =
      result.normalization.mapped_field_work_coefficient;
  const std::size_t expected = expected_volume(before.electric.L);
  const bool input_valid = before.electric.L >= 2
      && before.magnetic_half.L == before.electric.L
      && finite_field(before.electric) && finite_field(before.magnetic_half)
      && stationary_density.size() == expected
      && std::all_of(stationary_density.begin(), stationary_density.end(),
          [](double value) { return std::isfinite(value); })
      && finite(before.matter.remainder) && finite(before.matter.momentum)
      && (charge == -1 || charge == 1)
      && options.dt > 0.0 && std::isfinite(options.dt)
      && options.wave_speed >= 0.0 && std::isfinite(options.wave_speed)
      && options.gate_tolerance > 0.0
      && options.solve_tolerance > 0.0
      && options.finite_difference_scale > 0.0
      && options.max_iterations > 0 && result.normalization.valid
      && result.interaction_scale > 0.0;
  if (!input_valid) return result;
  const QuadraticPolarityCoat start_coat = make_quadratic_polarity_coat(
      effective_position(before.matter), charge);
  if (!start_coat.valid) return result;

  const double lambda = options.wave_speed*options.dt;
  const PreparedFields prepared = prepare_fields(before, lambda);
  const auto evaluator = [&](const Vec3& momentum_after) {
    return evaluate_candidate(before, charge, options, prepared,
                              result.interaction_scale, momentum_after);
  };
  const RootResult root = solve_root(before.matter.momentum, options, evaluator);
  result.solve = root.diagnostics;
  result.solve_residual = root.diagnostics.residual;
  if (!root.candidate.valid) return result;
  const Candidate& candidate = root.candidate;
  result.after.electric = candidate.electric_after;
  result.after.magnetic_half = candidate.magnetic_after;
  result.after.matter = candidate.matter_after;
  result.segment = candidate.segment;
  result.gather = candidate.gather;
  result.displacement = result.segment.end_effective_position
      -result.segment.start_effective_position;
  result.discrete_gradient_velocity = candidate.velocity;
  result.electric_impulse = candidate.electric_impulse;
  result.magnetic_impulse = candidate.magnetic_impulse;
  result.total_impulse = candidate.total_impulse;

  result.particle_energy_before = production_flat_energy_from_momentum(
      before.matter.momentum);
  result.particle_energy_after = production_flat_energy_from_momentum(
      result.after.matter.momentum);
  result.field_energy_before = result.interaction_scale
      *matched_modified_energy(before.electric, before.magnetic_half, lambda);
  result.field_energy_after = result.interaction_scale
      *matched_modified_energy(result.after.electric,
                               result.after.magnetic_half, lambda);
  result.current_work = result.interaction_scale*result.gather.current_work;
  const std::vector<double> density_before = total_density(
      result.segment.rho_before, stationary_density);
  const std::vector<double> density_after = total_density(
      result.segment.rho_after, stationary_density);
  result.continuity_residual = result.segment.continuity_residual;
  result.gauss_before_residual = max_fractional_gauss_residual(
      before.electric, density_before);
  result.gauss_after_residual = max_fractional_gauss_residual(
      result.after.electric, density_after);
  result.force_residual = maximum_component(
      result.after.matter.momentum-before.matter.momentum
      -result.total_impulse);
  const double particle_change = result.particle_energy_after
      -result.particle_energy_before;
  const double field_change = result.field_energy_after
      -result.field_energy_before;
  result.discrete_gradient_residual = std::abs(particle_change
      -result.discrete_gradient_velocity.dot(
          result.after.matter.momentum-before.matter.momentum));
  result.electric_work_residual = std::abs(
      particle_change-result.current_work);
  result.field_work_residual = std::abs(field_change+result.current_work);
  result.total_energy_residual = std::abs(particle_change+field_change);
  result.magnetic_work_residual = std::abs(
      result.discrete_gradient_velocity.dot(result.magnetic_impulse));
  result.kinematic_residual = maximum_component(result.displacement
      -result.discrete_gradient_velocity*options.dt);
  result.causal_speed_excess = std::max(0.0,
      result.discrete_gradient_velocity.mag()-C_SPEED);

  MatchedFaceFlux recovered_electric = result.after.electric;
  add_current(recovered_electric, result.segment, +1.0);
  const MatchedFaceFlux magnetic_curl = matched_curl(
      result.after.magnetic_half);
  for (std::size_t i = 0; i < recovered_electric.x.size(); ++i) {
    recovered_electric.x[i] -= lambda*magnetic_curl.x[i];
    recovered_electric.y[i] -= lambda*magnetic_curl.y[i];
    recovered_electric.z[i] -= lambda*magnetic_curl.z[i];
  }
  MatchedEdgeField recovered_magnetic = result.after.magnetic_half;
  const MatchedEdgeField electric_curl = matched_curl_adjoint(
      recovered_electric);
  for (std::size_t i = 0; i < recovered_magnetic.x.size(); ++i) {
    recovered_magnetic.x[i] += lambda*electric_curl.x[i];
    recovered_magnetic.y[i] += lambda*electric_curl.y[i];
    recovered_magnetic.z[i] += lambda*electric_curl.z[i];
  }
  MatchedMatterPoint recovered_matter = point_at(
      result.segment.start_effective_position, before.electric.L,
      result.after.matter.momentum-result.total_impulse);
  result.inverse_residual = std::max({
      matched_face_max_difference(before.electric, recovered_electric),
      matched_edge_max_difference(before.magnetic_half, recovered_magnetic),
      point_residual(before.matter, recovered_matter, before.electric.L)});

  result.valid = result.solve.converged
      && std::isfinite(result.total_energy_residual)
      && std::isfinite(result.inverse_residual);
  const double gate = options.gate_tolerance;
  result.gates_pass = result.valid
      && result.solve_residual <= gate
      && result.continuity_residual <= gate
      && result.gauss_before_residual <= gate
      && result.gauss_after_residual <= gate
      && result.force_residual <= gate
      && result.discrete_gradient_residual <= gate
      && result.gather.electric_adjoint_residual <= gate
      && result.electric_work_residual <= gate
      && result.field_work_residual <= gate
      && result.total_energy_residual <= gate
      && result.magnetic_work_residual <= gate
      && result.kinematic_residual <= gate
      && result.causal_speed_excess <= gate
      && result.inverse_residual <= gate;
  return result;
}

}  // namespace ftd::eft

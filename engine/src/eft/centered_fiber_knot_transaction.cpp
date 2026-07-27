#include "ftd/eft/centered_fiber_knot_transaction.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : axis == 1 ? value.y : value.z;
}

Vec3 with_component(Vec3 value, int axis, double component_value) {
  if (axis == 0) value.x = component_value;
  else if (axis == 1) value.y = component_value;
  else value.z = component_value;
  return value;
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x),
                   std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const Vec3& lhs, const Vec3& rhs) {
  return max_abs(lhs - rhs);
}

double norm(const Vec3& value) {
  return std::sqrt(value.x * value.x + value.y * value.y
                   + value.z * value.z);
}

double matter_energy(const Vec3& momentum,
                     double rest_energy,
                     double causal_speed) {
  return std::sqrt(rest_energy * rest_energy
      + causal_speed * causal_speed
          * (momentum.x * momentum.x
             + momentum.y * momentum.y
             + momentum.z * momentum.z));
}

Vec3 discrete_gradient_displacement(
    const Vec3& momentum_before,
    const Vec3& momentum_after,
    double dt,
    double rest_energy,
    double causal_speed) {
  const double denominator = matter_energy(
      momentum_before, rest_energy, causal_speed)
      + matter_energy(momentum_after, rest_energy, causal_speed);
  return (momentum_before + momentum_after)
      * (dt * causal_speed * causal_speed / denominator);
}

MatchedFaceFlux as_face_flux(const FaceCurrentSegment& current) {
  MatchedFaceFlux result(current.L);
  result.x = current.current_x;
  result.y = current.current_y;
  result.z = current.current_z;
  return result;
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value,
                double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    residual = std::max({residual,
        std::abs(lhs.x[i] - rhs.x[i]),
        std::abs(lhs.y[i] - rhs.y[i]),
        std::abs(lhs.z[i] - rhs.z[i])});
  }
  return residual;
}

long double dot(const MatchedFaceFlux& lhs,
                const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result += static_cast<long double>(lhs.x[i]) * rhs.x[i]
        + static_cast<long double>(lhs.y[i]) * rhs.y[i]
        + static_cast<long double>(lhs.z[i]) * rhs.z[i];
  }
  return result;
}

double energy(const MatchedFaceFlux& field) {
  return 0.5 * static_cast<double>(dot(field, field));
}

bool valid_field(const MatchedFaceFlux& field) {
  const std::size_t side = field.L > 0
      ? static_cast<std::size_t>(field.L) : 0;
  const std::size_t count = side * side * side;
  return field.L >= 3 && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

Vec3 current_reaction(const Vec3& displacement) {
  Vec3 result{};
  for (int axis = 0; axis < 3; ++axis) {
    const int b = (axis + 1) % 3;
    const int c = (axis + 2) % 3;
    const double db = std::abs(component(displacement, b));
    const double dc = std::abs(component(displacement, c));
    const double integral = 1.0 - 0.5 * (db + dc)
        + db * dc / 3.0;
    result = with_component(
        result, axis, component(displacement, axis) * integral);
  }
  return result;
}

Vec3 fixed_point_map(const CenteredFiberKnotInput& input,
                     const Vec3& centered_before,
                     const Vec3& momentum_after) {
  const Vec3 displacement = discrete_gradient_displacement(
      input.momentum_before, momentum_after, input.dt,
      input.rest_energy, input.causal_speed);
  return input.momentum_before
      + centered_before
          * (input.dt * input.coupling
             * static_cast<double>(input.charge))
      - current_reaction(displacement)
          * (input.dt * input.coupling * input.coupling / 4.0);
}

}  // namespace

Vec3 predict_centered_knot_current_trace(
    const Vec3& displacement,
    int charge) {
  if (charge != -1 && charge != 1) return {NAN, NAN, NAN};
  return current_reaction(displacement)
      * (0.5 * static_cast<double>(charge));
}

double centered_knot_contraction_bound(
    double coupling,
    double dt,
    double rest_energy,
    double causal_speed) {
  if (!std::isfinite(coupling) || !std::isfinite(dt)
      || !std::isfinite(rest_energy) || rest_energy <= 0.0
      || !std::isfinite(causal_speed) || causal_speed <= 0.0) {
    return INFINITY;
  }
  return coupling * coupling * dt * dt
      * causal_speed * causal_speed
      * (1.0 + causal_speed * std::abs(dt))
      / (4.0 * rest_energy);
}

CenteredFiberKnotStep solve_centered_fiber_knot_step(
    const CenteredFiberKnotInput& input) {
  CenteredFiberKnotStep result;
  result.site = input.site;
  result.charge = input.charge;
  result.coupling = input.coupling;
  result.dt = input.dt;
  result.rest_energy = input.rest_energy;
  result.causal_speed = input.causal_speed;
  result.momentum_before = input.momentum_before;
  result.dressing_before = input.dressing_before;
  result.electric_before = input.electric_before;
  result.contraction_bound = centered_knot_contraction_bound(
      input.coupling, input.dt, input.rest_energy, input.causal_speed);
  result.uniqueness_certified = result.contraction_bound < 1.0;

  if (!valid_field(input.electric_before)
      || (input.charge != -1 && input.charge != 1)
      || !std::isfinite(input.coupling)
      || !std::isfinite(input.dt) || input.dt == 0.0
      || input.rest_energy <= 0.0 || input.causal_speed <= 0.0
      || input.causal_speed * std::abs(input.dt) >= 1.0
      || input.max_iterations <= 0
      || input.solver_tolerance <= 0.0) {
    return result;
  }
  const CenteredKnotTrace trace_before = evaluate_centered_knot_trace(
      input.electric_before, input.site);
  if (!trace_before.valid) return result;
  result.centered_field_before = trace_before.centered;

  Vec3 momentum = input.use_initial_guess
      ? input.initial_guess
      : input.momentum_before
          + trace_before.centered
              * (input.dt * input.coupling
                 * static_cast<double>(input.charge));
  for (int iteration = 1; iteration <= input.max_iterations; ++iteration) {
    const Vec3 next = fixed_point_map(input, trace_before.centered, momentum);
    result.iterations = iteration;
    if (max_difference(next, momentum) <= input.solver_tolerance) {
      momentum = next;
      result.converged = true;
      break;
    }
    momentum = next;
  }
  result.momentum_after = momentum;
  result.fixed_point_residual = max_difference(
      momentum, fixed_point_map(input, trace_before.centered, momentum));
  result.displacement = discrete_gradient_displacement(
      input.momentum_before, result.momentum_after, input.dt,
      input.rest_energy, input.causal_speed);

  result.current = make_face_current_segment(
      input.electric_before.L, input.site, {},
      input.site, result.displacement, input.charge);
  if (!result.current.valid) return result;
  const MatchedFaceFlux current = as_face_flux(result.current);
  result.electric_midpoint = input.electric_before;
  result.electric_after = input.electric_before;
  add_scaled(result.electric_midpoint, current, -0.5 * input.coupling);
  add_scaled(result.electric_after, current, -input.coupling);
  const CenteredKnotTrace trace_mid = evaluate_centered_knot_trace(
      result.electric_midpoint, input.site);
  const CenteredKnotTrace current_trace = evaluate_centered_knot_trace(
      current, input.site);
  if (!trace_mid.valid || !current_trace.valid) return result;
  result.centered_field_midpoint = trace_mid.centered;
  result.centered_current_trace = current_trace.centered;
  result.predicted_centered_current_trace =
      predict_centered_knot_current_trace(
          result.displacement, input.charge);
  result.centered_current_trace_residual = max_difference(
      result.centered_current_trace,
      result.predicted_centered_current_trace);

  MatchedFaceFlux midpoint_check = input.electric_before;
  add_scaled(midpoint_check, result.electric_after, 1.0);
  for (std::size_t i = 0; i < midpoint_check.x.size(); ++i) {
    midpoint_check.x[i] *= 0.5;
    midpoint_check.y[i] *= 0.5;
    midpoint_check.z[i] *= 0.5;
  }
  result.midpoint_field_residual = max_difference(
      midpoint_check, result.electric_midpoint);

  const Vec3 expected_impulse = result.centered_field_midpoint
      * (input.dt * input.coupling
         * static_cast<double>(input.charge));
  result.impulse_residual = max_difference(
      result.momentum_after - input.momentum_before,
      expected_impulse);
  const Vec3 expected_displacement = discrete_gradient_displacement(
      input.momentum_before, result.momentum_after, input.dt,
      input.rest_energy, input.causal_speed);
  result.displacement_residual = max_difference(
      result.displacement, expected_displacement);

  result.field_work = input.coupling
      * static_cast<double>(dot(current, result.electric_midpoint));
  result.centered_work = input.coupling
      * static_cast<double>(input.charge)
      * (result.centered_field_midpoint.x * result.displacement.x
         + result.centered_field_midpoint.y * result.displacement.y
         + result.centered_field_midpoint.z * result.displacement.z);
  const double matter_change = matter_energy(
      result.momentum_after, input.rest_energy, input.causal_speed)
      - matter_energy(input.momentum_before,
                      input.rest_energy, input.causal_speed);
  result.matter_work_residual = std::abs(
      matter_change - result.centered_work);
  result.dressing_change = result.field_work - result.centered_work;
  result.dressing_after = input.dressing_before + result.dressing_change;
  const double field_change = energy(result.electric_after)
      - energy(input.electric_before);
  result.total_energy_residual = std::abs(
      field_change + matter_change + result.dressing_change);
  result.continuity_residual = result.current.continuity_residual;
  result.locality_residual = result.current.locality_residual;

  result.relative_gauss_residual = 0.0;
  for (int x = 0; x < input.electric_before.L; ++x) {
    for (int y = 0; y < input.electric_before.L; ++y) {
      for (int z = 0; z < input.electric_before.L; ++z) {
        const double residual = divergence_at(result.electric_after, x, y, z)
            - divergence_at(input.electric_before, x, y, z)
            + input.coupling * face_current_divergence_at(
                result.current, x, y, z);
        result.relative_gauss_residual = std::max(
            result.relative_gauss_residual, std::abs(residual));
      }
    }
  }
  result.speed = norm(result.displacement) / std::abs(input.dt);
  result.causal_excess = std::max(0.0,
      result.speed - input.causal_speed);

  const FaceCurrentSegment reverse_current = make_face_current_segment(
      input.electric_before.L, input.site, result.displacement,
      input.site, {}, input.charge);
  if (!reverse_current.valid) return result;
  const MatchedFaceFlux reverse = as_face_flux(reverse_current);
  MatchedFaceFlux restored_field = result.electric_after;
  add_scaled(restored_field, reverse, -input.coupling);
  const Vec3 reverse_displacement = discrete_gradient_displacement(
      result.momentum_after, input.momentum_before, -input.dt,
      input.rest_energy, input.causal_speed);
  const Vec3 reverse_impulse = result.centered_field_midpoint
      * (-input.dt * input.coupling
         * static_cast<double>(input.charge));
  const double restored_dressing = result.dressing_after
      - result.dressing_change;
  MatchedFaceFlux current_sum = current;
  add_scaled(current_sum, reverse, 1.0);
  MatchedFaceFlux zero(input.electric_before.L);
  result.inverse_residual = std::max({
      max_difference(current_sum, zero),
      max_difference(restored_field, input.electric_before),
      max_difference(reverse_displacement, result.displacement * -1.0),
      max_difference(input.momentum_before - result.momentum_after,
                     reverse_impulse),
      std::abs(restored_dressing - input.dressing_before)});

  result.valid = result.converged && result.uniqueness_certified
      && result.fixed_point_residual <= 1e-12
      && result.centered_current_trace_residual <= 1e-12
      && result.midpoint_field_residual <= 1e-12
      && result.impulse_residual <= 1e-12
      && result.displacement_residual <= 1e-12
      && result.matter_work_residual <= 1e-12
      && result.total_energy_residual <= 1e-12
      && result.continuity_residual <= 1e-12
      && result.relative_gauss_residual <= 1e-12
      && result.locality_residual <= 1e-12
      && result.causal_excess <= 1e-12
      && result.inverse_residual <= 1e-10;
  return result;
}

}  // namespace ftd::eft

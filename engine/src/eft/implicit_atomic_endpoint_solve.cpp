#include "ftd/eft/implicit_atomic_endpoint_solve.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/spacetime_worldline_coupling.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {carrier.anchor.x+carrier.remainder.x,
          carrier.anchor.y+carrier.remainder.y,
          carrier.anchor.z+carrier.remainder.z};
}

void add(MatchedFaceFlux& target, const MatchedFaceFlux& value) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += value.x[i];
    target.y[i] += value.y[i];
    target.z[i] += value.z[i];
  }
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

void scale(MatchedFaceFlux& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

double momentum_from_speed(double speed) {
  const double h = E_REST/std::sqrt(
      1.0-speed*speed/(C_SPEED*C_SPEED));
  return h*speed/(C_SPEED*C_SPEED);
}

std::array<double, 6> flatten(
    const std::array<Vec3, 2>& values) {
  return {values[0].x, values[0].y, values[0].z,
          values[1].x, values[1].y, values[1].z};
}

std::array<Vec3, 2> unflatten(
    const std::array<double, 6>& values) {
  return {{{values[0], values[1], values[2]},
           {values[3], values[4], values[5]}}};
}

double infinity_norm(const std::array<double, 6>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

bool solve_linear(double matrix[6][7],
                  std::array<double, 6>& solution,
                  double& minimum_pivot) {
  minimum_pivot = INFINITY;
  for (int column = 0; column < 6; ++column) {
    int pivot_row = column;
    for (int row = column+1; row < 6; ++row) {
      if (std::abs(matrix[row][column])
          > std::abs(matrix[pivot_row][column])) pivot_row = row;
    }
    const double pivot = std::abs(matrix[pivot_row][column]);
    minimum_pivot = std::min(minimum_pivot, pivot);
    if (!(pivot > 1e-12) || !std::isfinite(pivot)) return false;
    if (pivot_row != column) {
      for (int j = column; j <= 6; ++j)
        std::swap(matrix[column][j], matrix[pivot_row][j]);
    }
    for (int row = column+1; row < 6; ++row) {
      const double factor = matrix[row][column]/matrix[column][column];
      for (int j = column; j <= 6; ++j)
        matrix[row][j] -= factor*matrix[column][j];
    }
  }
  for (int row = 5; row >= 0; --row) {
    double value = matrix[row][6];
    for (int j = row+1; j < 6; ++j) value -= matrix[row][j]*solution[j];
    solution[row] = value/matrix[row][row];
    if (!std::isfinite(solution[row])) return false;
  }
  return true;
}

AtomicFaceEndpointTrialResult evaluate(
    const ImplicitAtomicInitialFixture& fixture,
    const std::array<Vec3, 2>& endpoint,
    double derivative_step, double algebra_tolerance,
    bool chart_contained_derivative) {
  return evaluate_atomic_face_endpoint_trial(
      fixture.start_position, endpoint, fixture.charge,
      fixture.prescribed_kinetic_start,
      fixture.potential_before, fixture.electric_before,
      fixture.beta, fixture.temporal_scale, E_REST, C_SPEED,
      derivative_step, algebra_tolerance, chart_contained_derivative);
}

}  // namespace

ImplicitAtomicInitialFixture make_implicit_atomic_initial_fixture(
    int L, const Vec3& contact_position, Coord diagonal_direction,
    int polarity, double speed, double tolerance) {
  ImplicitAtomicInitialFixture result(L);
  result.shell = diagonal_direction.x*diagonal_direction.x
      +diagonal_direction.y*diagonal_direction.y
      +diagonal_direction.z*diagonal_direction.z;
  result.speed = speed;
  result.temporal_scale = C_SPEED;
  if (L < 3 || (result.shell != 2 && result.shell != 3)
      || (polarity != -1 && polarity != 1)
      || !(speed > 0.0) || !(speed < C_SPEED)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  result.rebase = analyze_overshoot_preserving_contact_rebase(
      L, contact_position, diagonal_direction, polarity, speed, tolerance);
  const FaceFluxNormalization normalization =
      measure_face_flux_normalization();
  if (!result.rebase.valid || !normalization.valid) return result;
  result.beta = normalization.mapped_field_work_coefficient;
  MatchedFaceFlux reference_current(L);
  const double p0 = momentum_from_speed(speed);
  double continuity_residual = 0.0;
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    const auto& source = result.rebase.bounce_preimage.carrier[i];
    result.start_position[i] = position(source);
    const Vec3 unit = source.velocity*(1.0/speed);
    result.free_end_position[i] = result.start_position[i]+unit*speed;
    result.charge[i] = source.polarity;
    result.prescribed_kinetic_start[i] = unit*p0;
    Coord end_anchor{
        static_cast<int>(std::floor(result.free_end_position[i].x)),
        static_cast<int>(std::floor(result.free_end_position[i].y)),
        static_cast<int>(std::floor(result.free_end_position[i].z))};
    const Vec3 end_remainder{
        result.free_end_position[i].x-end_anchor.x,
        result.free_end_position[i].y-end_anchor.y,
        result.free_end_position[i].z-end_anchor.z};
    const auto current = make_spacetime_worldline_current(
        L, source.anchor, source.remainder, end_anchor, end_remainder,
        source.polarity, result.temporal_scale);
    if (!current.valid) return result;
    MatchedFaceFlux total(L);
    total.x = current.spatial.current_x;
    total.y = current.spatial.current_y;
    total.z = current.spatial.current_z;
    add(reference_current, total);
    continuity_residual = std::max(
        continuity_residual, current.spatial.continuity_residual);
  }
  result.electric_before = reference_current;
  scale(result.electric_before, 0.5);
  const MatchedFaceFlux challenge = matched_curl(
      matched_curl_adjoint(reference_current));
  add_scaled(result.electric_before, challenge, 0.125);
  result.potential_before = result.electric_before;
  scale(result.potential_before, result.temporal_scale);
  result.valid = continuity_residual <= tolerance;
  return result;
}

ImplicitAtomicEndpointSolveResult solve_implicit_atomic_endpoint(
    int L, const Vec3& contact_position, Coord diagonal_direction,
    int polarity, double speed, double derivative_step,
    double root_tolerance, double algebra_tolerance,
    bool chart_contained_derivative) {
  ImplicitAtomicEndpointSolveResult result;
  result.fixture = make_implicit_atomic_initial_fixture(
      L, contact_position, diagonal_direction,
      polarity, speed, algebra_tolerance);
  if (!result.fixture.valid || !(derivative_step > 0.0)
      || !std::isfinite(derivative_step) || !(root_tolerance > 0.0)
      || !std::isfinite(root_tolerance)) return result;
  std::array<Vec3, 2> endpoint = result.fixture.free_end_position;
  AtomicFaceEndpointTrialResult trial = evaluate(
      result.fixture, endpoint, derivative_step, algebra_tolerance,
      chart_contained_derivative);
  if (!trial.valid) {
    result.trial = trial;
    return result;
  }
  result.initial_residual = trial.residual_infinity_norm;
  result.minimum_pivot = INFINITY;
  result.minimum_accepted_step_factor = 1.0;
  const double jacobian_step = derivative_step/64.0;
  for (int iteration = 0; iteration <= 20; ++iteration) {
    result.iterations = iteration;
    if (trial.residual_infinity_norm <= root_tolerance) {
      result.converged = true;
      break;
    }
    if (iteration == 20) break;
    const std::array<double, 6> residual = flatten(trial.start_residual);
    const std::array<double, 6> flat_endpoint = flatten(endpoint);
    double augmented[6][7]{};
    bool jacobian_valid = true;
    for (int column = 0; column < 6; ++column) {
      auto plus_flat = flat_endpoint;
      auto minus_flat = flat_endpoint;
      plus_flat[static_cast<std::size_t>(column)] += jacobian_step;
      minus_flat[static_cast<std::size_t>(column)] -= jacobian_step;
      const auto plus = evaluate(result.fixture, unflatten(plus_flat),
                                 derivative_step, algebra_tolerance,
                                 chart_contained_derivative);
      const auto minus = evaluate(result.fixture, unflatten(minus_flat),
                                  derivative_step, algebra_tolerance,
                                  chart_contained_derivative);
      if (!plus.valid || !minus.valid) {
        jacobian_valid = false;
        break;
      }
      const auto plus_residual = flatten(plus.start_residual);
      const auto minus_residual = flatten(minus.start_residual);
      for (int row = 0; row < 6; ++row) {
        augmented[row][column] = (
            plus_residual[static_cast<std::size_t>(row)]
            -minus_residual[static_cast<std::size_t>(row)])
            /(2.0*jacobian_step);
      }
    }
    if (!jacobian_valid) break;
    for (int row = 0; row < 6; ++row)
      augmented[row][6] = -residual[static_cast<std::size_t>(row)];
    std::array<double, 6> delta{};
    double iteration_pivot = 0.0;
    if (!solve_linear(augmented, delta, iteration_pivot)) break;
    result.minimum_pivot = std::min(result.minimum_pivot, iteration_pivot);
    bool accepted = false;
    double factor = 1.0;
    for (int backtrack = 0; backtrack < 10; ++backtrack) {
      auto candidate_flat = flat_endpoint;
      for (int i = 0; i < 6; ++i)
        candidate_flat[static_cast<std::size_t>(i)] += factor*delta[i];
      auto candidate = evaluate(result.fixture, unflatten(candidate_flat),
                                derivative_step, algebra_tolerance,
                                chart_contained_derivative);
      if (candidate.valid
          && candidate.residual_infinity_norm
              < trial.residual_infinity_norm) {
        endpoint = unflatten(candidate_flat);
        trial = candidate;
        result.minimum_accepted_step_factor = std::min(
            result.minimum_accepted_step_factor, factor);
        accepted = true;
        break;
      }
      factor *= 0.5;
    }
    if (!accepted) break;
  }
  result.trial = trial;
  result.final_residual = trial.residual_infinity_norm;
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    result.displacement[i] = endpoint[i]-result.fixture.start_position[i];
    result.maximum_endpoint_change = std::max(
        result.maximum_endpoint_change,
        (endpoint[i]-result.fixture.free_end_position[i]).mag());
  }
  result.valid = result.converged && trial.valid
      && result.final_residual <= root_tolerance
      && std::isfinite(result.minimum_pivot)
      && result.minimum_pivot > 1e-12;
  return result;
}

}  // namespace ftd::eft

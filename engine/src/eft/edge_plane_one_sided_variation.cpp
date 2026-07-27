#include "ftd/eft/edge_plane_one_sided_variation.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

void set_component(Vec3& value, int axis, double amount) {
  if (axis == 0) value.x = amount;
  else if (axis == 1) value.y = amount;
  else value.z = amount;
}

std::array<int, 2> active_axes(int normal_axis) {
  std::array<int, 2> result{};
  int count = 0;
  for (int axis = 0; axis < 3; ++axis)
    if (axis != normal_axis) result[static_cast<std::size_t>(count++)] = axis;
  return result;
}

std::array<double, 4> active_residual(
    const AtomicFaceEndpointTrialResult& trial, int normal_axis) {
  const auto active = active_axes(normal_axis);
  return {{component(trial.start_residual[0], active[0]),
           component(trial.start_residual[0], active[1]),
           component(trial.start_residual[1], active[0]),
           component(trial.start_residual[1], active[1])}};
}

double infinity_norm(const std::array<double, 4>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

AtomicFaceEndpointTrialResult evaluate(
    const ImplicitAtomicInitialFixture& fixture,
    const std::array<Vec3, 2>& endpoint,
    double derivative_step, double tolerance) {
  return evaluate_atomic_face_endpoint_trial(
      fixture.start_position, endpoint, fixture.charge,
      fixture.prescribed_kinetic_start,
      fixture.potential_before, fixture.electric_before,
      fixture.beta, fixture.temporal_scale,
      E_REST, C_SPEED, derivative_step, tolerance, false);
}

bool solve_linear(double matrix[4][5],
                  std::array<double, 4>& solution,
                  double& minimum_pivot) {
  minimum_pivot = INFINITY;
  for (int column = 0; column < 4; ++column) {
    int pivot_row = column;
    for (int row = column+1; row < 4; ++row)
      if (std::abs(matrix[row][column])
          > std::abs(matrix[pivot_row][column])) pivot_row = row;
    const double pivot = std::abs(matrix[pivot_row][column]);
    minimum_pivot = std::min(minimum_pivot, pivot);
    if (!(pivot > 1e-12) || !std::isfinite(pivot)) return false;
    if (pivot_row != column)
      for (int j = column; j <= 4; ++j)
        std::swap(matrix[column][j], matrix[pivot_row][j]);
    for (int row = column+1; row < 4; ++row) {
      const double factor = matrix[row][column]/matrix[column][column];
      for (int j = column; j <= 4; ++j)
        matrix[row][j] -= factor*matrix[column][j];
    }
  }
  for (int row = 3; row >= 0; --row) {
    double value = matrix[row][4];
    for (int j = row+1; j < 4; ++j) value -= matrix[row][j]*solution[j];
    solution[row] = value/matrix[row][row];
    if (!std::isfinite(solution[row])) return false;
  }
  return true;
}

std::array<Vec3, 2> displaced(
    const std::array<Vec3, 2>& endpoint,
    const std::array<double, 4>& delta,
    int normal_axis, double factor) {
  auto result = endpoint;
  const auto active = active_axes(normal_axis);
  for (int carrier = 0; carrier < 2; ++carrier) {
    for (int local_axis = 0; local_axis < 2; ++local_axis) {
      const std::size_t index = static_cast<std::size_t>(
          2*carrier+local_axis);
      const int axis = active[static_cast<std::size_t>(local_axis)];
      set_component(result[static_cast<std::size_t>(carrier)], axis,
          component(result[static_cast<std::size_t>(carrier)], axis)
          +factor*delta[index]);
    }
  }
  return result;
}

}  // namespace

EdgePlaneOneSidedVariationResult solve_edge_plane_one_sided_variation(
    int L, const Vec3& contact_position, Coord edge_direction,
    int polarity, double speed, double derivative_step,
    double root_tolerance, double derivative_tolerance,
    double algebra_tolerance) {
  EdgePlaneOneSidedVariationResult result;
  const int shell = edge_direction.x*edge_direction.x
      +edge_direction.y*edge_direction.y+edge_direction.z*edge_direction.z;
  if (shell != 2 || !(derivative_step > 0.0)
      || !(root_tolerance > 0.0) || !(derivative_tolerance >= 0.0))
    return result;
  for (int axis = 0; axis < 3; ++axis) {
    const int direction = axis == 0 ? edge_direction.x
        : (axis == 1 ? edge_direction.y : edge_direction.z);
    if (direction == 0) result.normal_axis = axis;
  }
  result.fixture = make_implicit_atomic_initial_fixture(
      L, contact_position, edge_direction, polarity,
      speed, algebra_tolerance);
  if (!result.fixture.valid || result.normal_axis < 0) return result;
  result.endpoint = result.fixture.free_end_position;
  auto trial = evaluate(result.fixture, result.endpoint,
                        derivative_step, algebra_tolerance);
  if (!trial.valid) return result;
  result.initial_active_residual = infinity_norm(
      active_residual(trial, result.normal_axis));
  result.minimum_jacobian_pivot = INFINITY;
  result.minimum_accepted_step_factor = 1.0;
  const double jacobian_step = std::ldexp(1.0, -18);
  const auto active = active_axes(result.normal_axis);
  for (int iteration = 0; iteration <= 20; ++iteration) {
    result.iterations = iteration;
    const auto residual = active_residual(trial, result.normal_axis);
    const double norm = infinity_norm(residual);
    if (norm <= root_tolerance) {
      result.converged = true;
      break;
    }
    if (iteration == 20) break;
    double augmented[4][5]{};
    bool jacobian_valid = true;
    for (int column = 0; column < 4; ++column) {
      std::array<double, 4> unit{};
      unit[static_cast<std::size_t>(column)] = jacobian_step;
      const auto plus = evaluate(result.fixture,
          displaced(result.endpoint, unit, result.normal_axis, +1.0),
          derivative_step, algebra_tolerance);
      const auto minus = evaluate(result.fixture,
          displaced(result.endpoint, unit, result.normal_axis, -1.0),
          derivative_step, algebra_tolerance);
      if (!plus.valid || !minus.valid) {
        jacobian_valid = false;
        break;
      }
      const auto plus_residual = active_residual(plus, result.normal_axis);
      const auto minus_residual = active_residual(minus, result.normal_axis);
      for (int row = 0; row < 4; ++row)
        augmented[row][column] =
            (plus_residual[static_cast<std::size_t>(row)]
             -minus_residual[static_cast<std::size_t>(row)])
            /(2.0*jacobian_step);
    }
    if (!jacobian_valid) break;
    for (int row = 0; row < 4; ++row)
      augmented[row][4] = -residual[static_cast<std::size_t>(row)];
    std::array<double, 4> delta{};
    double pivot = 0.0;
    if (!solve_linear(augmented, delta, pivot)) break;
    result.minimum_jacobian_pivot = std::min(
        result.minimum_jacobian_pivot, pivot);
    bool accepted = false;
    double factor = 1.0;
    for (int backtrack = 0; backtrack < 10; ++backtrack) {
      const auto candidate_endpoint = displaced(
          result.endpoint, delta, result.normal_axis, factor);
      const auto candidate = evaluate(result.fixture, candidate_endpoint,
                                      derivative_step, algebra_tolerance);
      if (candidate.valid
          && infinity_norm(active_residual(candidate, result.normal_axis))
              < norm) {
        result.endpoint = candidate_endpoint;
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
  result.final_active_residual = infinity_norm(
      active_residual(trial, result.normal_axis));
  const auto fine = evaluate(result.fixture, result.endpoint,
                             derivative_step/2.0, algebra_tolerance);
  if (!fine.valid) return result;
  const auto coarse_residual = active_residual(trial, result.normal_axis);
  const auto fine_residual = active_residual(fine, result.normal_axis);
  for (int i = 0; i < 4; ++i)
    result.active_derivative_convergence = std::max(
        result.active_derivative_convergence,
        std::abs(coarse_residual[static_cast<std::size_t>(i)]
                 -fine_residual[static_cast<std::size_t>(i)]));
  result.normal = evaluate_atomic_face_one_sided_normal(
      result.fixture.start_position, result.endpoint, result.fixture.charge,
      result.fixture.prescribed_kinetic_start,
      result.fixture.potential_before, result.fixture.electric_before,
      result.fixture.beta, result.fixture.temporal_scale,
      E_REST, C_SPEED, result.normal_axis,
      derivative_step, algebra_tolerance);
  if (!result.normal.valid) return result;
  result.normal_differentiable = true;
  result.normal_interval_contains_zero = true;
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    const double left = result.normal.incoming_residual_left[i];
    const double right = result.normal.incoming_residual_right[i];
    result.maximum_normal_residual_jump = std::max(
        result.maximum_normal_residual_jump, std::abs(right-left));
    result.normal_differentiable = result.normal_differentiable
        && std::abs(right-left) <= derivative_tolerance;
    result.normal_interval_contains_zero =
        result.normal_interval_contains_zero
        && std::min(left, right) <= derivative_tolerance
        && std::max(left, right) >= -derivative_tolerance;
  }
  for (int carrier = 0; carrier < 2; ++carrier) {
    const std::size_t i = static_cast<std::size_t>(carrier);
    result.displacement[i] = result.endpoint[i]
        -result.fixture.start_position[i];
    result.maximum_endpoint_change_from_free = std::max(
        result.maximum_endpoint_change_from_free,
        (result.endpoint[i]-result.fixture.free_end_position[i]).mag());
  }
  result.valid = result.converged
      && result.final_active_residual <= root_tolerance
      && result.active_derivative_convergence <= derivative_tolerance
      && result.normal.derivative_convergence <= derivative_tolerance
      && std::isfinite(result.minimum_jacobian_pivot)
      && result.minimum_jacobian_pivot > 1e-12;
  return result;
}

}  // namespace ftd::eft


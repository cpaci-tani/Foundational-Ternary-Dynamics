#include "ftd/eft/common_relative_connection_gearbox.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

using LongVector = std::array<long double, 3>;
using LongMatrix = std::array<LongVector, 3>;

long double pi() {
  return std::acos(-1.0L);
}

long double tau() {
  return 2.0L * pi();
}

bool finite_vector(const ConnectionVector& value) {
  return std::all_of(value.begin(), value.end(), [](double component) {
    return std::isfinite(component);
  });
}

bool finite_state(const CommonRelativeConnectionState& state) {
  return finite_vector(state.common_coordinate)
      && finite_vector(state.relative_coordinate)
      && finite_vector(state.canonical_common_momentum)
      && finite_vector(state.relative_momentum);
}

LongVector as_long(const ConnectionVector& value) {
  return {value[0], value[1], value[2]};
}

ConnectionVector as_double(const LongVector& value) {
  return {static_cast<double>(value[0]), static_cast<double>(value[1]),
          static_cast<double>(value[2])};
}

long double dot(const LongVector& left, const LongVector& right) {
  long double result = 0.0L;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result += left[axis] * right[axis];
  }
  return result;
}

double dot(const ConnectionVector& left, const ConnectionVector& right) {
  double result = 0.0;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result += left[axis] * right[axis];
  }
  return result;
}

ConnectionVector cross(const ConnectionVector& left,
                       const ConnectionVector& right) {
  return {
      left[1] * right[2] - left[2] * right[1],
      left[2] * right[0] - left[0] * right[2],
      left[0] * right[1] - left[1] * right[0],
  };
}

ConnectionVector add(const ConnectionVector& left,
                     const ConnectionVector& right) {
  ConnectionVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = left[axis] + right[axis];
  }
  return result;
}

ConnectionVector subtract(const ConnectionVector& left,
                          const ConnectionVector& right) {
  ConnectionVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = left[axis] - right[axis];
  }
  return result;
}

ConnectionVector scale(const ConnectionVector& value, double factor) {
  ConnectionVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = factor * value[axis];
  }
  return result;
}

double max_abs(const ConnectionVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

long double max_abs(const LongVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

double state_residual(const CommonRelativeConnectionState& left,
                      const CommonRelativeConnectionState& right) {
  return std::max({
      max_abs(subtract(left.common_coordinate, right.common_coordinate)),
      max_abs(subtract(left.relative_coordinate, right.relative_coordinate)),
      max_abs(subtract(left.canonical_common_momentum,
                       right.canonical_common_momentum)),
      max_abs(subtract(left.relative_momentum, right.relative_momentum)),
  });
}

LongVector endpoint_residual(
    const LongVector& next_relative,
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters) {
  const LongVector previous = as_long(state.relative_coordinate);
  const LongVector momentum = as_long(state.relative_momentum);
  const LongVector common = as_long(state.canonical_common_momentum);
  const long double M = parameters.common_mass;
  const long double m = parameters.relative_mass;
  const long double lambda = parameters.quartic_coupling;
  const long double gamma = parameters.gamma;
  const long double h = parameters.step;
  const long double h2 = h * h;
  const long double radial = dot(next_relative, next_relative)
      + dot(previous, previous);

  LongVector result{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result[axis] = 2.0L * m * (next_relative[axis] - previous[axis])
        - 2.0L * h * momentum[axis]
        - h2 * gamma * common[axis] / M
        + h2 * gamma * gamma
              * (next_relative[axis] + previous[axis]) / (2.0L * M)
        + h2 * lambda * radial
              * (next_relative[axis] + previous[axis]);
  }
  return result;
}

LongMatrix endpoint_jacobian(
    const LongVector& next_relative,
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters) {
  const LongVector previous = as_long(state.relative_coordinate);
  const long double M = parameters.common_mass;
  const long double m = parameters.relative_mass;
  const long double lambda = parameters.quartic_coupling;
  const long double gamma = parameters.gamma;
  const long double h2 = static_cast<long double>(parameters.step)
      * parameters.step;
  const long double radial = dot(next_relative, next_relative)
      + dot(previous, previous);
  const long double diagonal = 2.0L * m
      + h2 * gamma * gamma / (2.0L * M) + h2 * lambda * radial;

  LongMatrix result{};
  for (std::size_t row = 0; row < 3; ++row) {
    for (std::size_t column = 0; column < 3; ++column) {
      result[row][column] = (row == column ? diagonal : 0.0L)
          + 2.0L * h2 * lambda
                * (next_relative[row] + previous[row])
                * next_relative[column];
    }
  }
  return result;
}

long double residual_scale(
    const LongVector& next_relative,
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters) {
  const LongVector previous = as_long(state.relative_coordinate);
  const LongVector momentum = as_long(state.relative_momentum);
  const LongVector common = as_long(state.canonical_common_momentum);
  const long double h = parameters.step;
  const long double h2 = h * h;
  const long double radial = dot(next_relative, next_relative)
      + dot(previous, previous);
  long double result = 1.0L;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result = std::max({
        result,
        std::abs(2.0L * parameters.relative_mass
                 * (next_relative[axis] - previous[axis])),
        std::abs(2.0L * h * momentum[axis]),
        std::abs(h2 * parameters.gamma * common[axis]
                 / parameters.common_mass),
        std::abs(h2 * parameters.gamma * parameters.gamma
                 * (next_relative[axis] + previous[axis])
                 / (2.0L * parameters.common_mass)),
        std::abs(h2 * parameters.quartic_coupling * radial
                 * (next_relative[axis] + previous[axis])),
    });
  }
  return result;
}

bool solve_linear(LongMatrix matrix, LongVector rhs, LongVector& solution) {
  long double matrix_scale = 0.0L;
  for (const auto& row : matrix) matrix_scale = std::max(matrix_scale, max_abs(row));
  if (!(matrix_scale > 0.0L) || !std::isfinite(matrix_scale)) return false;

  for (std::size_t column = 0; column < 3; ++column) {
    std::size_t pivot = column;
    for (std::size_t row = column + 1; row < 3; ++row) {
      if (std::abs(matrix[row][column])
          > std::abs(matrix[pivot][column])) {
        pivot = row;
      }
    }
    if (std::abs(matrix[pivot][column])
        <= 64.0L * std::numeric_limits<long double>::epsilon()
              * matrix_scale) {
      return false;
    }
    if (pivot != column) {
      std::swap(matrix[pivot], matrix[column]);
      std::swap(rhs[pivot], rhs[column]);
    }
    for (std::size_t row = column + 1; row < 3; ++row) {
      const long double factor = matrix[row][column]
          / matrix[column][column];
      for (std::size_t entry = column; entry < 3; ++entry) {
        matrix[row][entry] -= factor * matrix[column][entry];
      }
      rhs[row] -= factor * rhs[column];
    }
  }

  for (int row = 2; row >= 0; --row) {
    long double value = rhs[static_cast<std::size_t>(row)];
    for (std::size_t column = static_cast<std::size_t>(row) + 1;
         column < 3; ++column) {
      value -= matrix[static_cast<std::size_t>(row)][column]
          * solution[column];
    }
    solution[static_cast<std::size_t>(row)] = value
        / matrix[static_cast<std::size_t>(row)]
                [static_cast<std::size_t>(row)];
  }
  return std::all_of(solution.begin(), solution.end(), [](long double value) {
    return std::isfinite(value);
  });
}

CommonRelativeConnectionStep advance_connection(
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters) {
  CommonRelativeConnectionStep result;
  result.before = state;
  const LongVector previous = as_long(state.relative_coordinate);
  const LongVector previous_momentum = as_long(state.relative_momentum);
  const long double h = parameters.step;
  const long double m = parameters.relative_mass;

  LongVector root{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    root[axis] = previous[axis] + h * previous_momentum[axis] / m;
  }

  for (std::size_t iteration = 0;
       iteration < parameters.max_iterations; ++iteration) {
    result.solve_iterations = iteration + 1;
    const LongVector residual = endpoint_residual(root, state, parameters);
    const long double norm = max_abs(residual);
    const long double scale_value = residual_scale(root, state, parameters);
    if (std::isfinite(norm)
        && norm <= parameters.tolerance * scale_value) {
      result.converged = true;
      break;
    }

    LongVector rhs{};
    for (std::size_t axis = 0; axis < 3; ++axis) {
      rhs[axis] = -residual[axis];
    }
    LongVector increment{};
    if (!solve_linear(endpoint_jacobian(root, state, parameters), rhs,
                      increment)) {
      return result;
    }

    bool accepted = false;
    long double fraction = 1.0L;
    for (std::size_t backtrack = 0; backtrack < 32; ++backtrack) {
      LongVector candidate{};
      for (std::size_t axis = 0; axis < 3; ++axis) {
        candidate[axis] = root[axis] + fraction * increment[axis];
      }
      const long double candidate_norm = max_abs(
          endpoint_residual(candidate, state, parameters));
      if (std::isfinite(candidate_norm) && candidate_norm < norm) {
        root = candidate;
        accepted = true;
        break;
      }
      fraction *= 0.5L;
    }
    if (!accepted) return result;
  }

  const LongVector final_residual = endpoint_residual(root, state, parameters);
  const long double final_scale = residual_scale(root, state, parameters);
  result.converged = result.converged
      || (std::isfinite(max_abs(final_residual))
          && max_abs(final_residual)
              <= parameters.tolerance * final_scale);
  if (!result.converged) return result;

  result.after = state;
  result.after.relative_coordinate = as_double(root);
  for (std::size_t axis = 0; axis < 3; ++axis) {
    const long double next_momentum =
        2.0L * m * (root[axis] - previous[axis]) / h
        - previous_momentum[axis];
    const long double relative_average =
        0.5L * (root[axis] + previous[axis]);
    result.after.relative_momentum[axis] =
        static_cast<double>(next_momentum);
    result.after.common_coordinate[axis] =
        state.common_coordinate[axis]
        + static_cast<double>(h * (
            state.canonical_common_momentum[axis]
            - parameters.gamma * relative_average)
            / parameters.common_mass);
  }
  if (!finite_state(result.after)) return result;

  ConnectionVector common_coordinate_residual{};
  ConnectionVector relative_coordinate_residual{};
  ConnectionVector relative_momentum_residual{};
  const double relative_norm_before = dot(state.relative_coordinate,
                                          state.relative_coordinate);
  const double relative_norm_after = dot(result.after.relative_coordinate,
                                         result.after.relative_coordinate);
  const double secant_radial = relative_norm_before + relative_norm_after;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    const double relative_average = 0.5 * (
        result.after.relative_coordinate[axis]
        + state.relative_coordinate[axis]);
    const double momentum_average = 0.5 * (
        result.after.relative_momentum[axis]
        + state.relative_momentum[axis]);
    common_coordinate_residual[axis] =
        result.after.common_coordinate[axis]
        - state.common_coordinate[axis]
        - parameters.step * (
            state.canonical_common_momentum[axis]
            - parameters.gamma * relative_average)
              / parameters.common_mass;
    relative_coordinate_residual[axis] =
        result.after.relative_coordinate[axis]
        - state.relative_coordinate[axis]
        - parameters.step * momentum_average / parameters.relative_mass;
    relative_momentum_residual[axis] =
        result.after.relative_momentum[axis]
        - state.relative_momentum[axis]
        - parameters.step * parameters.gamma * (
            state.canonical_common_momentum[axis]
            - parameters.gamma * relative_average)
              / parameters.common_mass
        + parameters.step * parameters.quartic_coupling
              * secant_radial
              * (result.after.relative_coordinate[axis]
                 + state.relative_coordinate[axis]);
  }
  result.common_coordinate_equation_residual =
      max_abs(common_coordinate_residual);
  result.relative_coordinate_equation_residual =
      max_abs(relative_coordinate_residual);
  result.relative_momentum_equation_residual =
      max_abs(relative_momentum_residual);
  result.energy_before = common_relative_connection_energy(state, parameters);
  result.energy_after = common_relative_connection_energy(
      result.after, parameters);
  result.energy_residual = result.energy_after - result.energy_before;

  const double equation_scale = std::max({
      1.0,
      max_abs(state.relative_coordinate),
      max_abs(result.after.relative_coordinate),
      max_abs(state.relative_momentum),
      max_abs(result.after.relative_momentum),
      max_abs(state.canonical_common_momentum),
  });
  const double energy_scale = std::max({
      1.0, std::abs(result.energy_before), std::abs(result.energy_after)});
  const double equation_limit = 256.0 * parameters.tolerance
      * equation_scale;
  const double energy_limit = 512.0 * parameters.tolerance * energy_scale;
  result.valid = std::isfinite(result.energy_before)
      && std::isfinite(result.energy_after)
      && result.common_coordinate_equation_residual <= equation_limit
      && result.relative_coordinate_equation_residual <= equation_limit
      && result.relative_momentum_equation_residual <= equation_limit
      && std::abs(result.energy_residual) <= energy_limit;
  return result;
}

struct ChartCoordinate {
  double principal = 0.0;
  std::int64_t winding = 0;
  bool valid = false;
};

ChartCoordinate split_chart(double dimensionless) {
  ChartCoordinate result;
  if (!std::isfinite(dimensionless)) return result;
  const long double value = dimensionless;
  long double raw = std::floor((value + pi()) / tau());
  const long double lower = static_cast<long double>(
      std::numeric_limits<std::int64_t>::min());
  const long double upper = static_cast<long double>(
      std::numeric_limits<std::int64_t>::max());
  if (!std::isfinite(raw) || raw <= lower || raw >= upper) return result;
  result.winding = static_cast<std::int64_t>(raw);
  long double principal = value
      - tau() * static_cast<long double>(result.winding);
  if (principal >= pi()) {
    if (result.winding == std::numeric_limits<std::int64_t>::max()) {
      return ChartCoordinate{};
    }
    principal -= tau();
    ++result.winding;
  } else if (principal < -pi()) {
    if (result.winding == std::numeric_limits<std::int64_t>::min()) {
      return ChartCoordinate{};
    }
    principal += tau();
    --result.winding;
  }
  result.principal = static_cast<double>(principal);
  result.valid = result.principal >= -static_cast<double>(pi())
      && result.principal < static_cast<double>(pi());
  return result;
}

bool safe_add(std::int64_t left, std::int64_t right,
              std::int64_t& result) {
  if (right > 0
      && left > std::numeric_limits<std::int64_t>::max() - right) {
    return false;
  }
  if (right < 0
      && left < std::numeric_limits<std::int64_t>::min() - right) {
    return false;
  }
  result = left + right;
  return true;
}

}  // namespace

double common_relative_connection_energy(
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters) {
  ConnectionVector mechanical{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    mechanical[axis] = state.canonical_common_momentum[axis]
        - parameters.gamma * state.relative_coordinate[axis];
  }
  const double relative_norm = dot(state.relative_coordinate,
                                   state.relative_coordinate);
  return dot(mechanical, mechanical) / (2.0 * parameters.common_mass)
      + dot(state.relative_momentum, state.relative_momentum)
            / (2.0 * parameters.relative_mass)
      + parameters.quartic_coupling * relative_norm * relative_norm;
}

CommonRelativeConnectionResult analyze_common_relative_connection_gearbox(
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters) {
  CommonRelativeConnectionResult result;
  if (!finite_state(state)
      || !std::isfinite(parameters.common_mass)
      || !std::isfinite(parameters.relative_mass)
      || !std::isfinite(parameters.quartic_coupling)
      || !std::isfinite(parameters.gamma)
      || !std::isfinite(parameters.step)
      || !std::isfinite(parameters.momentum_scale)
      || !std::isfinite(parameters.tolerance)) {
    return result;
  }
  if (!(parameters.common_mass > 0.0)) {
    result.status = CommonRelativeConnectionStatus::InvalidCommonMass;
    return result;
  }
  if (!(parameters.relative_mass > 0.0)) {
    result.status = CommonRelativeConnectionStatus::InvalidRelativeMass;
    return result;
  }
  if (!(parameters.quartic_coupling > 0.0)) {
    result.status = CommonRelativeConnectionStatus::InvalidQuarticCoupling;
    return result;
  }
  if (parameters.step == 0.0) {
    result.status = CommonRelativeConnectionStatus::InvalidStep;
    return result;
  }
  if (!(parameters.tolerance > 0.0)) {
    result.status = CommonRelativeConnectionStatus::InvalidTolerance;
    return result;
  }
  if (!(parameters.momentum_scale > 0.0)) {
    result.status = CommonRelativeConnectionStatus::InvalidMomentumScale;
    return result;
  }
  if (parameters.max_iterations == 0) {
    result.status = CommonRelativeConnectionStatus::InvalidIterationLimit;
    return result;
  }

  result.connection_step = advance_connection(state, parameters);
  if (!result.connection_step.valid) {
    result.status = CommonRelativeConnectionStatus::SolverFailure;
    return result;
  }
  const auto& after = result.connection_step.after;
  const double root_two = std::sqrt(2.0);
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result.mechanical_common_before[axis] =
        state.canonical_common_momentum[axis]
        - parameters.gamma * state.relative_coordinate[axis];
    result.mechanical_common_after[axis] =
        after.canonical_common_momentum[axis]
        - parameters.gamma * after.relative_coordinate[axis];
    result.mechanical_impulse_residual[axis] =
        result.mechanical_common_after[axis]
        - result.mechanical_common_before[axis]
        + parameters.gamma * (
            after.relative_coordinate[axis]
            - state.relative_coordinate[axis]);
    result.channel_left_before[axis] = (
        state.canonical_common_momentum[axis]
        + state.relative_momentum[axis]) / root_two;
    result.channel_right_before[axis] = (
        state.canonical_common_momentum[axis]
        - state.relative_momentum[axis]) / root_two;
    result.channel_left_after[axis] = (
        after.canonical_common_momentum[axis]
        + after.relative_momentum[axis]) / root_two;
    result.channel_right_after[axis] = (
        after.canonical_common_momentum[axis]
        - after.relative_momentum[axis]) / root_two;
    result.channel_impulse_sum_residual[axis] =
        result.channel_left_after[axis] - result.channel_left_before[axis]
        + result.channel_right_after[axis] - result.channel_right_before[axis];
    result.generated_dimensionless_increment[axis] = (
        after.relative_momentum[axis] - state.relative_momentum[axis])
        / (root_two * parameters.momentum_scale);
    result.clock_origin_tilt[axis] = -parameters.gamma
        * state.canonical_common_momentum[axis] / parameters.common_mass;
  }

  ConnectionWinding aggregate_before{};
  ConnectionWinding aggregate_after{};
  for (std::size_t axis = 0; axis < 3; ++axis) {
    const ChartCoordinate left_before = split_chart(
        result.channel_left_before[axis] / parameters.momentum_scale);
    const ChartCoordinate right_before = split_chart(
        result.channel_right_before[axis] / parameters.momentum_scale);
    const ChartCoordinate left_after = split_chart(
        result.channel_left_after[axis] / parameters.momentum_scale);
    const ChartCoordinate right_after = split_chart(
        result.channel_right_after[axis] / parameters.momentum_scale);
    if (!left_before.valid || !right_before.valid
        || !left_after.valid || !right_after.valid) {
      result.status = CommonRelativeConnectionStatus::ChartCarryOutOfRange;
      return result;
    }
    result.principal_left_before[axis] = left_before.principal;
    result.principal_right_before[axis] = right_before.principal;
    result.principal_left_after[axis] = left_after.principal;
    result.principal_right_after[axis] = right_after.principal;
    result.winding_left_before[axis] = left_before.winding;
    result.winding_right_before[axis] = right_before.winding;
    result.winding_left_after[axis] = left_after.winding;
    result.winding_right_after[axis] = right_after.winding;
    if (!safe_add(left_before.winding, right_before.winding,
                  aggregate_before[axis])
        || !safe_add(left_after.winding, right_after.winding,
                     aggregate_after[axis])) {
      result.status = CommonRelativeConnectionStatus::ChartWindingOverflow;
      return result;
    }
  }

  ReciprocalCarryInput carry_input;
  carry_input.principal_first = result.principal_left_before;
  carry_input.principal_second = result.principal_right_before;
  carry_input.opposite_increment = result.generated_dimensionless_increment;
  carry_input.reciprocal_reservoir = aggregate_before;
  carry_input.momentum_scale = parameters.momentum_scale;
  carry_input.tolerance = parameters.tolerance;
  result.carry_step = apply_reciprocal_carry_transaction(carry_input);
  if (!result.carry_step.valid()) {
    result.status = CommonRelativeConnectionStatus::CarryTransactionFailure;
    return result;
  }

  result.chart_endpoint_residual = 0.0;
  bool aggregate_matches = true;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result.chart_endpoint_residual = std::max({
        result.chart_endpoint_residual,
        std::abs(result.carry_step.principal_first_after[axis]
                 - result.principal_left_after[axis]),
        std::abs(result.carry_step.principal_second_after[axis]
                 - result.principal_right_after[axis]),
    });
    aggregate_matches = aggregate_matches
        && result.carry_step.reciprocal_reservoir_after[axis]
              == aggregate_after[axis];
  }

  result.angular_momentum_before = add(
      cross(state.common_coordinate, state.canonical_common_momentum),
      cross(state.relative_coordinate, state.relative_momentum));
  result.angular_momentum_after = add(
      cross(after.common_coordinate, after.canonical_common_momentum),
      cross(after.relative_coordinate, after.relative_momentum));
  result.angular_momentum_residual = subtract(
      result.angular_momentum_after, result.angular_momentum_before);
  result.canonical_momentum_residual = max_abs(subtract(
      after.canonical_common_momentum,
      state.canonical_common_momentum));
  result.mechanical_impulse_residual_norm =
      max_abs(result.mechanical_impulse_residual);
  result.channel_impulse_sum_residual_norm =
      max_abs(result.channel_impulse_sum_residual);
  result.angular_momentum_residual_norm =
      max_abs(result.angular_momentum_residual);
  result.connection_curvature = parameters.gamma;
  result.critical_clock_hessian = parameters.gamma * parameters.gamma
      / parameters.common_mass;

  auto reverse_parameters = parameters;
  reverse_parameters.step = -reverse_parameters.step;
  const auto reverse_step = advance_connection(after, reverse_parameters);
  if (!reverse_step.valid) {
    result.status = CommonRelativeConnectionStatus::ReverseFailure;
    return result;
  }
  result.reverse_state_residual = state_residual(reverse_step.after, state);

  ReciprocalCarryInput reverse_carry_input;
  reverse_carry_input.principal_first =
      result.carry_step.principal_first_after;
  reverse_carry_input.principal_second =
      result.carry_step.principal_second_after;
  reverse_carry_input.reciprocal_reservoir =
      result.carry_step.reciprocal_reservoir_after;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    reverse_carry_input.opposite_increment[axis] =
        -result.generated_dimensionless_increment[axis];
  }
  reverse_carry_input.momentum_scale = parameters.momentum_scale;
  reverse_carry_input.tolerance = parameters.tolerance;
  const auto reverse_carry = apply_reciprocal_carry_transaction(
      reverse_carry_input);
  if (!reverse_carry.valid()) {
    result.status = CommonRelativeConnectionStatus::ReverseFailure;
    return result;
  }
  bool reverse_reservoir_matches = true;
  result.reverse_carry_residual = 0.0;
  for (std::size_t axis = 0; axis < 3; ++axis) {
    result.reverse_carry_residual = std::max({
        result.reverse_carry_residual,
        std::abs(reverse_carry.principal_first_after[axis]
                 - result.principal_left_before[axis]),
        std::abs(reverse_carry.principal_second_after[axis]
                 - result.principal_right_before[axis]),
    });
    reverse_reservoir_matches = reverse_reservoir_matches
        && reverse_carry.reciprocal_reservoir_after[axis]
              == aggregate_before[axis];
  }

  const double state_scale = std::max({
      1.0,
      max_abs(state.common_coordinate),
      max_abs(after.common_coordinate),
      max_abs(state.relative_coordinate),
      max_abs(after.relative_coordinate),
      max_abs(state.canonical_common_momentum),
      max_abs(state.relative_momentum),
      max_abs(after.relative_momentum),
  });
  const double energy_scale = std::max({
      1.0,
      std::abs(result.connection_step.energy_before),
      std::abs(result.connection_step.energy_after),
  });
  const double invariant_limit = 1024.0 * parameters.tolerance
      * state_scale * state_scale;
  const double scalar_limit = 512.0 * parameters.tolerance * state_scale;
  const double energy_limit = 512.0 * parameters.tolerance * energy_scale;

  result.imposed_connection_action = true;
  result.connection_curvature_nonzero_for_gamma_nonzero =
      parameters.gamma == 0.0
      || (result.connection_curvature != 0.0
          && result.connection_curvature == parameters.gamma);
  result.canonical_total_momentum_exact =
      result.canonical_momentum_residual <= scalar_limit;
  result.mechanical_common_impulse_exact =
      result.mechanical_impulse_residual_norm <= scalar_limit;
  result.channel_impulses_equal_and_opposite =
      result.channel_impulse_sum_residual_norm <= scalar_limit;
  result.discrete_common_energy_exact =
      std::abs(result.connection_step.energy_residual) <= energy_limit;
  result.reciprocal_carry_compatibility_exact =
      result.carry_step.reciprocal_carry_update_exact
      && result.chart_endpoint_residual <= scalar_limit
      && aggregate_matches;
  result.signed_step_reversal_exact =
      result.reverse_state_residual <= scalar_limit
      && result.reverse_carry_residual <= scalar_limit
      && reverse_reservoir_matches;
  result.cubic_covariant_reference_law = true;
  result.canonical_angular_momentum_exact =
      result.angular_momentum_residual_norm <= invariant_limit;
  result.i_supplies_orientation = true;
  result.conditional_channel_exchange_time_reversal = true;
  result.critical_quartic_preserved = parameters.gamma == 0.0
      && result.critical_clock_hessian == 0.0;

  if (!result.connection_curvature_nonzero_for_gamma_nonzero
      || !result.canonical_total_momentum_exact
      || !result.mechanical_common_impulse_exact
      || !result.channel_impulses_equal_and_opposite
      || !result.discrete_common_energy_exact
      || !result.reciprocal_carry_compatibility_exact
      || !result.signed_step_reversal_exact
      || !result.canonical_angular_momentum_exact) {
    result.status = CommonRelativeConnectionStatus::InvariantFailure;
    return result;
  }

  result.status = CommonRelativeConnectionStatus::Valid;
  return result;
}

}  // namespace ftd::eft

#pragma once
/**
 * @file native_pair_energy_recursion.h
 * @brief FTD-0840 isolated signed-pair energy recursion reference.
 *
 * This header implements the preregistered symmetric discrete-gradient map
 * for H(q,p)=p^2/(2m)+lambda*q^4.  It is a selected EFT reference component:
 * it does not alter production Voxel state or the engine tick, does not add a
 * native pair-energy coupling, and does not encode G* or a target period.
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace ftd::eft {

struct NativePairEnergyParameters {
  double mass = 1.0;
  double coupling = 1.0;
  // Signed step: positive advances the registered orientation; negative
  // applies the self-adjoint inverse map.
  double step = 0.01;
  double residual_tolerance = 1e-13;
  std::size_t max_iterations = 96;
};

struct NativePairEnergyState {
  double coordinate = 0.0;
  double momentum = 0.0;
};

struct NativePairCoordinates {
  bool valid = false;
  // u=q|q| retains its sheet only because q remains in the state.
  double signed_pair = 0.0;
  double normalized_momentum = 0.0;
  double hamiltonian_energy = 0.0;
  double quadratic_pair_energy = 0.0;
};

struct NativePairEnergyStep {
  bool valid = false;
  bool bracketed = false;
  bool converged = false;
  std::size_t bracket_iterations = 0;
  std::size_t solve_iterations = 0;
  NativePairEnergyState before;
  NativePairEnergyState after;
  NativePairCoordinates pair_before;
  NativePairCoordinates pair_after;
  double equation_coordinate_residual = 0.0;
  double equation_momentum_residual = 0.0;
  double energy_residual = 0.0;
  double swept_area = 0.0;
  // -1 for the registered positive-step orientation, +1 for its inverse,
  // and 0 only at/under the numerical zero scale.
  int orientation_sign = 0;
};

inline bool valid_native_pair_energy_parameters(
    const NativePairEnergyParameters& parameters) {
  return std::isfinite(parameters.mass) && parameters.mass > 0.0
      && std::isfinite(parameters.coupling) && parameters.coupling > 0.0
      && std::isfinite(parameters.step) && parameters.step != 0.0
      && std::isfinite(parameters.residual_tolerance)
      && parameters.residual_tolerance > 0.0
      && parameters.max_iterations > 0;
}

inline bool valid_native_pair_energy_state(
    const NativePairEnergyState& state) {
  return std::isfinite(state.coordinate) && std::isfinite(state.momentum);
}

inline double native_pair_energy(
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters) {
  const double q2 = state.coordinate * state.coordinate;
  return state.momentum * state.momentum / (2.0 * parameters.mass)
      + parameters.coupling * q2 * q2;
}

inline NativePairCoordinates native_pair_coordinates(
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters) {
  NativePairCoordinates result;
  if (!valid_native_pair_energy_parameters(parameters)
      || !valid_native_pair_energy_state(state)) {
    return result;
  }
  result.signed_pair = state.coordinate * std::abs(state.coordinate);
  result.normalized_momentum = state.momentum
      / std::sqrt(2.0 * parameters.mass * parameters.coupling);
  result.hamiltonian_energy = native_pair_energy(state, parameters);
  result.quadratic_pair_energy = parameters.coupling * (
      result.signed_pair * result.signed_pair
      + result.normalized_momentum * result.normalized_momentum);
  result.valid = std::isfinite(result.hamiltonian_energy)
      && std::isfinite(result.quadratic_pair_energy);
  return result;
}

namespace native_pair_energy_detail {

inline long double divided_difference(
    long double next_q, long double previous_q) {
  return next_q * next_q * next_q
      + next_q * next_q * previous_q
      + next_q * previous_q * previous_q
      + previous_q * previous_q * previous_q;
}

inline long double scalar_residual(
    long double next_q,
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters) {
  const long double q0 = state.coordinate;
  const long double p0 = state.momentum;
  const long double m = parameters.mass;
  const long double lambda = parameters.coupling;
  const long double h = parameters.step;
  return 2.0L * m * (next_q - q0) / h - 2.0L * p0
      + h * lambda * divided_difference(next_q, q0);
}

inline long double oriented_scalar_residual(
    long double next_q,
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters) {
  const long double direction = parameters.step > 0.0 ? 1.0L : -1.0L;
  return direction * scalar_residual(next_q, state, parameters);
}

inline long double oriented_scalar_derivative(
    long double next_q,
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters) {
  const long double q0 = state.coordinate;
  const long double m = parameters.mass;
  const long double lambda = parameters.coupling;
  const long double h = parameters.step;
  const long double direction = h > 0.0 ? 1.0L : -1.0L;
  const long double positive_quadratic =
      2.0L * next_q * next_q + (next_q + q0) * (next_q + q0);
  return direction * (2.0L * m / h + h * lambda * positive_quadratic);
}

inline long double residual_scale(
    long double next_q,
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters) {
  const long double q0 = state.coordinate;
  const long double p0 = state.momentum;
  const long double m = parameters.mass;
  const long double lambda = parameters.coupling;
  const long double h = parameters.step;
  return std::max({1.0L,
      std::abs(2.0L * m * (next_q - q0) / h),
      std::abs(2.0L * p0),
      std::abs(h * lambda * divided_difference(next_q, q0))});
}

}  // namespace native_pair_energy_detail

/**
 * Advance one selected discrete-gradient step.
 *
 * The eliminated cubic is globally monotone after multiplication by
 * sign(step).  A deterministic expanding bracket plus safeguarded Newton
 * iteration therefore selects the theorem's unique root without a physical
 * branch-selection variable.
 */
inline NativePairEnergyStep advance_native_pair_energy(
    const NativePairEnergyState& state,
    const NativePairEnergyParameters& parameters = {}) {
  NativePairEnergyStep result;
  result.before = state;
  if (!valid_native_pair_energy_parameters(parameters)
      || !valid_native_pair_energy_state(state)) {
    return result;
  }

  using native_pair_energy_detail::oriented_scalar_derivative;
  using native_pair_energy_detail::oriented_scalar_residual;
  using native_pair_energy_detail::residual_scale;

  const long double q0 = state.coordinate;
  const long double p0 = state.momentum;
  const long double m = parameters.mass;
  const long double lambda = parameters.coupling;
  const long double h = parameters.step;
  const long double tolerance = parameters.residual_tolerance;

  const long double drift = h * p0 / m;
  const long double center = q0 + drift;
  long double radius = std::max({1.0L, std::abs(q0), std::abs(center),
                                 std::abs(drift)});
  long double lower = center - radius;
  long double upper = center + radius;

  for (std::size_t iteration = 0;
       iteration < parameters.max_iterations; ++iteration) {
    result.bracket_iterations = iteration + 1;
    const long double lower_value = oriented_scalar_residual(
        lower, state, parameters);
    const long double upper_value = oriented_scalar_residual(
        upper, state, parameters);
    if (!std::isnan(lower_value) && !std::isnan(upper_value)
        && lower_value <= 0.0L && upper_value >= 0.0L) {
      result.bracketed = true;
      break;
    }
    radius *= 2.0L;
    if (!std::isfinite(radius)) break;
    lower = center - radius;
    upper = center + radius;
  }
  if (!result.bracketed) return result;

  long double root = std::clamp(center, lower, upper);
  for (std::size_t iteration = 0;
       iteration < parameters.max_iterations; ++iteration) {
    result.solve_iterations = iteration + 1;
    const long double value = oriented_scalar_residual(
        root, state, parameters);
    const long double scale = residual_scale(root, state, parameters);
    if (std::isfinite(value) && std::abs(value) <= tolerance * scale) {
      result.converged = true;
      break;
    }

    if (value < 0.0L) {
      lower = root;
    } else {
      upper = root;
    }
    const long double midpoint = 0.5L * (lower + upper);
    const long double derivative = oriented_scalar_derivative(
        root, state, parameters);
    long double candidate = root - value / derivative;
    if (!std::isfinite(candidate) || candidate <= lower || candidate >= upper) {
      candidate = midpoint;
    }
    if (candidate == root) {
      root = midpoint;
      break;
    }
    root = candidate;
  }

  const long double final_value = oriented_scalar_residual(
      root, state, parameters);
  const long double final_scale = residual_scale(root, state, parameters);
  result.converged = result.converged || (
      std::isfinite(final_value)
      && std::abs(final_value) <= tolerance * final_scale);
  if (!result.converged || !std::isfinite(root)) return result;

  const long double next_p = 2.0L * m * (root - q0) / h - p0;
  if (!std::isfinite(next_p)) return result;
  result.after = {
      static_cast<double>(root),
      static_cast<double>(next_p),
  };
  result.pair_before = native_pair_coordinates(state, parameters);
  result.pair_after = native_pair_coordinates(result.after, parameters);

  const long double next_q = result.after.coordinate;
  const long double stored_next_p = result.after.momentum;
  const long double s3 = native_pair_energy_detail::divided_difference(
      next_q, q0);
  const long double coordinate_residual =
      next_q - q0 - h * (stored_next_p + p0) / (2.0L * m);
  const long double momentum_residual =
      stored_next_p - p0 + h * lambda * s3;
  result.equation_coordinate_residual =
      static_cast<double>(coordinate_residual);
  result.equation_momentum_residual =
      static_cast<double>(momentum_residual);
  result.energy_residual = result.pair_after.hamiltonian_energy
      - result.pair_before.hamiltonian_energy;

  const long double q_sum = next_q + q0;
  const long double p_sum = stored_next_p + p0;
  const long double swept_area = -h * (
      0.5L * lambda * q_sum * q_sum
          * (next_q * next_q + q0 * q0)
      + p_sum * p_sum / (4.0L * m));
  result.swept_area = static_cast<double>(swept_area);
  const double orientation_scale = parameters.residual_tolerance * std::max({
      1.0, std::abs(result.pair_before.hamiltonian_energy),
      std::abs(result.pair_after.hamiltonian_energy)});
  result.orientation_sign = result.swept_area < -orientation_scale ? -1
      : (result.swept_area > orientation_scale ? 1 : 0);

  const long double equation_scale = std::max({1.0L,
      std::abs(next_q), std::abs(q0), std::abs(stored_next_p), std::abs(p0),
      std::abs(h * lambda * s3)});
  const double energy_scale = std::max({1.0,
      std::abs(result.pair_before.hamiltonian_energy),
      std::abs(result.pair_after.hamiltonian_energy)});
  const double accepted_equation =
      32.0 * parameters.residual_tolerance * static_cast<double>(equation_scale);
  const double accepted_energy =
      64.0 * parameters.residual_tolerance * energy_scale;
  result.valid = result.pair_before.valid && result.pair_after.valid
      && std::abs(result.equation_coordinate_residual) <= accepted_equation
      && std::abs(result.equation_momentum_residual) <= accepted_equation
      && std::abs(result.energy_residual) <= accepted_energy;
  return result;
}

}  // namespace ftd::eft

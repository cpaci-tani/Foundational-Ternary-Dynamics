/** FTD-0840 isolated native-pair energy recursion regression. */

#include "ftd/eft/native_pair_energy_recursion.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

using ftd::eft::NativePairEnergyParameters;
using ftd::eft::NativePairEnergyState;
using ftd::eft::advance_native_pair_energy;
using ftd::eft::native_pair_coordinates;
using ftd::eft::native_pair_energy;

int failures = 0;

void check(const std::string& label, bool condition) {
  std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!condition) ++failures;
}

bool close(double first, double second, double tolerance = 1e-12) {
  return std::abs(first - second) <= tolerance
      * std::max({1.0, std::abs(first), std::abs(second)});
}

double state_residual(
    const NativePairEnergyState& first,
    const NativePairEnergyState& second) {
  return std::max(std::abs(first.coordinate - second.coordinate),
                  std::abs(first.momentum - second.momentum));
}

}  // namespace

int main() {
  NativePairEnergyParameters parameters;
  parameters.mass = 1.7;
  parameters.coupling = 0.8;
  parameters.step = 0.025;
  parameters.residual_tolerance = 2e-14;
  parameters.max_iterations = 96;

  const std::array<NativePairEnergyState, 6> states{{
      {0.0, 0.0},
      {0.7, 0.0},
      {-0.7, 0.0},
      {0.31, -0.42},
      {-0.28, 0.63},
      {1.15, -0.37},
  }};

  bool pair_identity = true;
  bool sheet_retained = true;
  for (const auto& state : states) {
    const auto pair = native_pair_coordinates(state, parameters);
    pair_identity = pair_identity && pair.valid
        && close(pair.hamiltonian_energy, pair.quadratic_pair_energy, 2e-15);
    if (state.coordinate != 0.0) {
      sheet_retained = sheet_retained
          && std::signbit(pair.signed_pair) == std::signbit(state.coordinate);
    }
  }
  check("signed self-pair has the exact quadratic energy identity",
        pair_identity);
  check("retained coordinate preserves the square sheet", sheet_retained);

  bool all_steps_valid = true;
  bool positive_steps_clockwise = true;
  double maximum_equation_residual = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_reverse_residual = 0.0;
  for (std::size_t index = 1; index < states.size(); ++index) {
    const auto forward = advance_native_pair_energy(states[index], parameters);
    all_steps_valid = all_steps_valid && forward.valid && forward.bracketed
        && forward.converged;
    positive_steps_clockwise = positive_steps_clockwise
        && forward.orientation_sign == -1 && forward.swept_area < 0.0;
    maximum_equation_residual = std::max({maximum_equation_residual,
        std::abs(forward.equation_coordinate_residual),
        std::abs(forward.equation_momentum_residual)});
    maximum_energy_residual = std::max(maximum_energy_residual,
        std::abs(forward.energy_residual));

    auto inverse_parameters = parameters;
    inverse_parameters.step = -parameters.step;
    const auto inverse = advance_native_pair_energy(
        forward.after, inverse_parameters);
    maximum_reverse_residual = std::max(maximum_reverse_residual,
        state_residual(inverse.after, states[index]));

    const NativePairEnergyState physically_reversed{
        forward.after.coordinate, -forward.after.momentum};
    const auto physical_reverse = advance_native_pair_energy(
        physically_reversed, parameters);
    const NativePairEnergyState expected_reverse{
        states[index].coordinate, -states[index].momentum};
    maximum_reverse_residual = std::max(maximum_reverse_residual,
        state_residual(physical_reverse.after, expected_reverse));
  }
  check("all nonzero reference steps converge and close their equations",
        all_steps_valid && maximum_equation_residual <= 2e-12);
  check("every positive nonzero step has the registered orientation",
        positive_steps_clockwise);
  check("energy is closed for every reference arm",
        maximum_energy_residual <= 2e-12);
  check("signed-step and physical momentum reversal recover the input",
        maximum_reverse_residual <= 2e-12);

  const auto origin = advance_native_pair_energy({0.0, 0.0}, parameters);
  check("the origin is the unique zero-area fixed witness",
        origin.valid && close(origin.after.coordinate, 0.0)
        && close(origin.after.momentum, 0.0)
        && close(origin.swept_area, 0.0) && origin.orientation_sign == 0);

  NativePairEnergyState trajectory{0.83, 0.0};
  const double initial_energy = native_pair_energy(trajectory, parameters);
  const double q_bound = std::pow(initial_energy / parameters.coupling, 0.25);
  const double p_bound = std::sqrt(2.0 * parameters.mass * initial_energy);
  double maximum_long_energy_drift = 0.0;
  double maximum_q_excess = 0.0;
  double maximum_p_excess = 0.0;
  bool long_valid = true;
  for (int tick = 0; tick < 20000; ++tick) {
    const auto step = advance_native_pair_energy(trajectory, parameters);
    long_valid = long_valid && step.valid;
    trajectory = step.after;
    maximum_long_energy_drift = std::max(maximum_long_energy_drift,
        std::abs(native_pair_energy(trajectory, parameters) - initial_energy));
    maximum_q_excess = std::max(maximum_q_excess,
        std::abs(trajectory.coordinate) - q_bound);
    maximum_p_excess = std::max(maximum_p_excess,
        std::abs(trajectory.momentum) - p_bound);
  }
  check("long recursion remains on its compact energy shell",
        long_valid && maximum_long_energy_drift <= 2e-10
        && maximum_q_excess <= 2e-10 && maximum_p_excess <= 2e-10);

  auto invalid_mass = parameters;
  invalid_mass.mass = 0.0;
  auto invalid_coupling = parameters;
  invalid_coupling.coupling = -1.0;
  auto invalid_step = parameters;
  invalid_step.step = 0.0;
  auto impossible_solve = parameters;
  impossible_solve.residual_tolerance = 1e-30;
  impossible_solve.max_iterations = 1;
  const auto nan_state = advance_native_pair_energy(
      {std::numeric_limits<double>::quiet_NaN(), 0.0}, parameters);
  check("invalid parameters and nonfinite state fail closed",
        !advance_native_pair_energy(states[1], invalid_mass).valid
        && !advance_native_pair_energy(states[1], invalid_coupling).valid
        && !advance_native_pair_energy(states[1], invalid_step).valid
        && !nan_state.valid);
  const auto nonconvergent = advance_native_pair_energy(
      states[5], impossible_solve);
  check("an unresolved implicit solve fails closed",
        !nonconvergent.valid && !nonconvergent.converged);

  std::cout.precision(17);
  std::cout << "maximum_equation_residual=" << maximum_equation_residual << '\n'
            << "maximum_energy_residual=" << maximum_energy_residual << '\n'
            << "maximum_reverse_residual=" << maximum_reverse_residual << '\n'
            << "maximum_long_energy_drift=" << maximum_long_energy_drift << '\n'
            << "scope=SELECTED_EFT_REFERENCE_NOT_PRODUCTION_PAIR_COUPLING\n"
            << "cadence=CONTINUUM_GSTAR_FACTOR_FINITE_TICK_OPEN\n"
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

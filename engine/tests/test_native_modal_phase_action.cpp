/** Target-blind native modal phase/action carrier regression. */

#include "ftd/eft/native_modal_phase_action.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

using ftd::eft::NativeModalState;
using ftd::eft::advance_native_modal_tick;
using ftd::eft::native_modal_phase_action;
using ftd::eft::production18_mode_eigenvalue;
using ftd::eft::production18_spatial_symbol;
using ftd::eft::wrap_native_modal_phase;

int failures = 0;

void check(const std::string& label, bool condition) {
  std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!condition) ++failures;
}

bool close(double first, double second, double tolerance = 1e-12) {
  return std::abs(first - second) <= tolerance;
}

bool throws_invalid(double eigenvalue) {
  try {
    (void)native_modal_phase_action({0.3, -0.2}, eigenvalue);
  } catch (const std::invalid_argument&) {
    return true;
  }
  return false;
}

}  // namespace

int main() {
  const double pi = std::acos(-1.0);

  check("production zero mode has a=0",
        close(production18_mode_eigenvalue(0.0, 0.0, 0.0), 0.0));
  check("production symbol maximum is 16/3 at (pi,pi,0)",
        close(production18_spatial_symbol(pi, pi, 0.0), 16.0 / 3.0));
  check("selected C_WAVE fixes production a_max=16/9",
        close(production18_mode_eigenvalue(pi, pi, 0.0), 16.0 / 9.0));

  check("zero mode is rejected as non-elliptic", throws_invalid(0.0));
  check("negative mode is rejected", throws_invalid(-0.1));
  check("band edge is rejected", throws_invalid(4.0));
  check("non-finite mode is rejected",
        throws_invalid(std::numeric_limits<double>::infinity()));

  const std::vector<double> eigenvalues = {
      production18_mode_eigenvalue(0.2, 0.0, 0.0),
      production18_mode_eigenvalue(0.7, 0.4, 0.0),
      production18_mode_eigenvalue(1.1, 0.8, 0.3),
      production18_mode_eigenvalue(pi, pi, 0.0),
  };

  double maximum_rotation_residual = 0.0;
  double maximum_one_tick_action_drift = 0.0;
  double maximum_long_action_drift = 0.0;
  for (double eigenvalue : eigenvalues) {
    NativeModalState state{0.731, -0.284};
    const auto before = native_modal_phase_action(state, eigenvalue);
    const auto next_state = advance_native_modal_tick(state, eigenvalue);
    const auto after = native_modal_phase_action(next_state, eigenvalue);

    const double expected_q =
        before.cos_theta * before.canonical_q
        + before.sin_theta * before.canonical_p;
    const double expected_p =
        -before.sin_theta * before.canonical_q
        + before.cos_theta * before.canonical_p;
    maximum_rotation_residual = std::max(maximum_rotation_residual,
        std::max(std::abs(after.canonical_q - expected_q),
                 std::abs(after.canonical_p - expected_p)));
    maximum_one_tick_action_drift = std::max(maximum_one_tick_action_drift,
        std::abs(after.action - before.action));

    const double phase_step = wrap_native_modal_phase(after.phase - before.phase);
    check("phase advances by minus the source-fixed theta",
          close(phase_step, -before.radians_per_tick, 2e-12));

    state = {0.731, -0.284};
    const double initial_action = before.action;
    for (int tick = 0; tick < 10000; ++tick) {
      state = advance_native_modal_tick(state, eigenvalue);
    }
    const double final_action = native_modal_phase_action(state, eigenvalue).action;
    maximum_long_action_drift = std::max(maximum_long_action_drift,
        std::abs(final_action - initial_action)
            / std::max(1.0, std::abs(initial_action)));
  }

  check("canonical chart conjugates each production mode to a rotation",
        maximum_rotation_residual <= 2e-12);
  check("action is invariant for one primitive tick",
        maximum_one_tick_action_drift <= 2e-12);
  check("action remains bounded over 10000 primitive ticks",
        maximum_long_action_drift <= 2e-11);

  const double eigenvalue = eigenvalues[2];
  const auto unit = native_modal_phase_action({0.4, -0.7}, eigenvalue);
  const auto scaled = native_modal_phase_action({1.2, -2.1}, eigenvalue);
  check("action has the required quadratic amplitude scaling",
        close(scaled.action, 9.0 * unit.action, 2e-12));
  check("rate is fixed in radians per primitive global tick",
        close(unit.radians_per_tick,
              std::acos(1.0 - 0.5 * eigenvalue), 2e-15));

  std::cout.precision(17);
  std::cout << "maximum_rotation_residual=" << maximum_rotation_residual << '\n'
            << "maximum_one_tick_action_drift="
            << maximum_one_tick_action_drift << '\n'
            << "maximum_long_action_drift=" << maximum_long_action_drift << '\n'
            << "scope=NATIVE_MODAL_CARRIER_NOT_LOCAL_PHYSICAL_CLOCK\n"
            << "verdict=TARGET_BLIND_NATIVE_MODAL_PHASE_ACTION_CARRIER\n"
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

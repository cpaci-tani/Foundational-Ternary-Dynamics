/** FTD-0558: native moving-source pole correction. */

#include "ftd/constants.h"
#include "ftd/eft/native_moving_source_pole.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr double identity_gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  if (pass) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const double c2 = ftd::C_WAVE*ftd::C_WAVE;
  const auto result = ftd::eft::analyze_native_moving_source_pole(c2);

  check("observer result valid", result.valid);
  check("exact production discrete-time pole",
      result.production_discrete_time_pole_derived);
  check("seven-point any-speed claim refuted",
      result.seven_point_any_speed_claim_refuted
      && result.seven_point_ratio_floor > 0.0);
  check("positive FULL-stencil speed floor",
      result.full_stencil_positive_speed_floor
      && result.universal_speed_floor > 0.0);
  check("wrapped alias counterexample",
      result.wrapped_alias_counterexample);
  check("integer hop Floquet requirement",
      result.integer_hop_requires_floquet_spectrum);
  check("locked driven-mode cardinality", result.driven_modes.size() == 12);
  check("locked threshold cardinality", result.thresholds.size() == 9);
  check("locked Floquet cardinality", result.floquet_schedules.size() == 12);
  check("identity residual gate",
      result.maximum_identity_residual <= identity_gate);

  double minimum_enumerated_speed = 1e100;
  for (const auto& threshold : result.thresholds) {
    check("threshold arm valid", threshold.valid);
    minimum_enumerated_speed = std::min(
        minimum_enumerated_speed, threshold.minimum_phase_speed);
  }
  double maximum_nonfundamental = 0.0;
  for (const auto& floquet : result.floquet_schedules) {
    check("Floquet arm valid", floquet.valid);
    maximum_nonfundamental = std::max(
        maximum_nonfundamental,
        floquet.maximum_nonfundamental_amplitude);
  }

  const bool passed = failures == 0;
  std::cout << "driven_mode_arms=" << result.driven_modes.size() << '\n'
            << "wrapped_threshold_arms=" << result.thresholds.size() << '\n'
            << "floquet_schedule_arms=" << result.floquet_schedules.size() << '\n'
            << "seven_point_ratio_floor="
            << result.seven_point_ratio_floor << '\n'
            << "universal_production_speed_floor="
            << result.universal_speed_floor << '\n'
            << "minimum_enumerated_phase_speed="
            << minimum_enumerated_speed << '\n'
            << "alias_symbol_residual="
            << result.alias_symbol_residual << '\n'
            << "alias_phase_residual="
            << result.alias_phase_residual << '\n'
            << "wrapped_to_old_alias_ratio="
            << result.old_to_wrapped_alias_ratio << '\n'
            << "maximum_nonfundamental_amplitude="
            << maximum_nonfundamental << '\n'
            << "maximum_identity_residual="
            << result.maximum_identity_residual << '\n'
            << "verdict="
            << (passed
                ? "NATIVE_MOVING_SOURCE_POLE_CORRECTED"
                : "NATIVE_MOVING_SOURCE_CORRECTION_FAILED") << '\n'
            << "native_moving_source_pole failures=" << failures << '\n';
  return passed ? 0 : 1;
}

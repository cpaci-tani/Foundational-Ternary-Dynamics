/**
 * Focused compatibility test for the selected matched-face/native-J scale.
 */

#include "ftd/eft/face_flux_normalization.h"

#include <cmath>
#include <iostream>

int main() {
  constexpr double gate = 1e-15;
  const auto result = ftd::eft::measure_face_flux_normalization();
  const double expected = ftd::G_C / (ftd::C_WAVE * ftd::C_WAVE);
  const bool pass = result.valid
      && std::abs(result.field_scale - expected) <= gate
      && std::abs(result.current_scale - expected) <= gate
      && std::abs(result.susceptibility_residual) <= gate
      && std::abs(result.work_residual) <= gate;
  std::cout.precision(17);
  std::cout << "field_scale=" << result.field_scale << '\n'
            << "native_susceptibility=" << result.native_susceptibility << '\n'
            << "action_work_coefficient="
            << result.native_action_work_coefficient << '\n'
            << "mapped_work_coefficient="
            << result.mapped_field_work_coefficient << '\n'
            << "susceptibility_residual="
            << result.susceptibility_residual << '\n'
            << "work_residual=" << result.work_residual << '\n'
            << "verdict="
            << (pass ? "ONE_SCALE_COMPATIBILITY_EXACT"
                     : "FACE_NORMALIZATION_COMPATIBILITY_FAILS") << '\n';
  return pass ? 0 : 1;
}

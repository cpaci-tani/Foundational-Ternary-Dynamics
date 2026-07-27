/** FTD-0486: exact coefficient no-go for the selected matched action. */

#include "ftd/constants.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/matched_action_normalization.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

}  // namespace

int main() {
  constexpr double gate = 1e-15;
  const auto face = ftd::eft::measure_face_flux_normalization();
  const double kappa = face.native_action_work_coefficient;
  const auto result = ftd::eft::derive_matched_action_normalization(
      ftd::C_SPEED, kappa);

  check("face normalization available", face.valid && kappa > 0.0);
  check("matched action normalization valid", result.valid);
  check("Gauss source coefficient forced to one",
        std::abs(result.gauss_source_coefficient - 1.0) <= gate);
  check("Ampere source coefficient forced to one",
        std::abs(result.ampere_source_coefficient - 1.0) <= gate);
  check("electric force/work coefficient remains kappa",
        std::abs(result.electric_force_coefficient - kappa) <= gate);
  check("field and matter electric power cancel",
        std::abs(result.power_residual) <= gate);
  check("magnetic coefficient is kappa over C_SPEED",
        std::abs(result.magnetic_force_coefficient
                 - kappa / ftd::C_SPEED) <= gate);
  check("magnetic/electric ratio is one over C_SPEED",
        std::abs(result.magnetic_to_electric_ratio
                 - 1.0 / ftd::C_SPEED) <= gate);
  check("frozen equal force coefficients are incompatible",
        !result.equal_force_coefficients_compatible
        && std::abs(result.equal_force_coefficient_residual) > 1e-3);

  const auto unit_cone = ftd::eft::derive_matched_action_normalization(
      1.0, kappa);
  check("unit-speed control restores equal coefficients",
        unit_cone.valid && unit_cone.equal_force_coefficients_compatible);
  check("invalid zero speed fails closed",
        !ftd::eft::derive_matched_action_normalization(0.0, kappa).valid);

  std::cout.precision(17);
  std::cout << "wave_speed=" << result.wave_speed << '\n'
            << "field_work_scale=" << result.field_work_scale << '\n'
            << "connection_coupling=" << result.connection_coupling << '\n'
            << "electric_force_coefficient="
            << result.electric_force_coefficient << '\n'
            << "magnetic_force_coefficient="
            << result.magnetic_force_coefficient << '\n'
            << "magnetic_to_electric_ratio="
            << result.magnetic_to_electric_ratio << '\n'
            << "equal_force_coefficient_residual="
            << result.equal_force_coefficient_residual << '\n'
            << "matched_action_normalization failures=" << failures << '\n'
            << "verdict=MATCHED_SOURCE_AND_EQUAL_FORCE_COEFFICIENTS_INCOMPATIBLE\n";
  return failures == 0 ? 0 : 1;
}

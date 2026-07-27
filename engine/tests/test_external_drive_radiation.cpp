/** FTD-0559: exact external-drive field-energy functional. */

#include "ftd/constants.h"
#include "ftd/eft/external_drive_radiation.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr double identity_gate = 1e-12;
constexpr double response_gate = 1e-10;
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
  const auto result = ftd::eft::analyze_external_drive_radiation(c2);
  check("observer result valid",result.valid);
  check("exact source work identity",result.exact_source_work_identity);
  check("exact retarded response",result.exact_retarded_response);
  check("finite-volume resonance dichotomy",
      result.finite_volume_resonance_dichotomy);
  check("Fejer radiation limit",result.fejer_radiation_limit);
  check("group velocity mismatch Jacobian",
      result.group_velocity_mismatch_jacobian);
  check("integer-hop Floquet weighting",
      result.integer_hop_power_is_floquet_weighted);
  check("locked work cardinality",result.work_arms.size()==12);
  check("locked harmonic cardinality",result.harmonic_arms.size()==24);
  check("locked Fejer cardinality",result.fejer_arms.size()==4);
  check("work identity residual",
      result.maximum_work_identity_residual<=identity_gate);
  check("response residual",
      result.maximum_response_residual<=response_gate);
  check("cumulative work residual",
      result.maximum_cumulative_work_residual<=response_gate);
  check("Fejer residual",
      result.maximum_fejer_residual<=identity_gate);

  double maximum_resonant_coefficient_error = 0.0;
  double minimum_resonant_bound_margin = 1e100;
  double minimum_off_resonant_bound_margin = 1e100;
  for (const auto& arm : result.harmonic_arms) {
    check("harmonic arm valid",arm.valid);
    if (arm.resonant) {
      const double error = std::abs(
          arm.normalized_resonant_energy-0.5);
      maximum_resonant_coefficient_error = std::max(
          maximum_resonant_coefficient_error,error);
      minimum_resonant_bound_margin = std::min(
          minimum_resonant_bound_margin,
          arm.resonant_error_bound-error);
    } else {
      minimum_off_resonant_bound_margin = std::min(
          minimum_off_resonant_bound_margin,
          arm.off_resonant_energy_bound
          -arm.maximum_off_resonant_energy);
    }
  }

  const bool passed = failures==0;
  std::cout << "work_arms=" << result.work_arms.size() << '\n'
            << "harmonic_response_arms=" << result.harmonic_arms.size() << '\n'
            << "fejer_normalization_arms=" << result.fejer_arms.size() << '\n'
            << "maximum_work_identity_residual="
            << result.maximum_work_identity_residual << '\n'
            << "maximum_response_residual="
            << result.maximum_response_residual << '\n'
            << "maximum_cumulative_work_residual="
            << result.maximum_cumulative_work_residual << '\n'
            << "maximum_fejer_residual="
            << result.maximum_fejer_residual << '\n'
            << "maximum_resonant_coefficient_error="
            << maximum_resonant_coefficient_error << '\n'
            << "minimum_resonant_bound_margin="
            << minimum_resonant_bound_margin << '\n'
            << "minimum_off_resonant_bound_margin="
            << minimum_off_resonant_bound_margin << '\n'
            << "verdict="
            << (passed
                ? "EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_DERIVED"
                : "EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_FAILED") << '\n'
            << "external_drive_radiation failures=" << failures << '\n';
  return passed ? 0 : 1;
}

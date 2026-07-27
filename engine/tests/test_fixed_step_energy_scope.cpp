/**
 * FTD-0543: fixed-step action energy is not an automatic consequence.
 */

#include "ftd/eft/fixed_step_energy_scope.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double gate = 1e-14;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

}  // namespace

int main() {
  double worst_midpoint_identity = 0.0;
  double worst_gradient_energy = 0.0;
  const double samples[][3] = {
      {0.0, 1.0, 1.0},
      {-0.4, 0.7, 0.6},
      {0.2, 0.8, 0.3},
      {-0.9, -0.1, 0.7},
      {0.35, 0.35, 0.5}};
  for (const auto& sample : samples) {
    const auto result = ftd::eft::evaluate_fixed_step_energy_scope(
        sample[0], sample[1], sample[2]);
    worst_midpoint_identity = std::max(
        worst_midpoint_identity, result.midpoint_identity_residual);
    worst_gradient_energy = std::max(
        worst_gradient_energy, std::abs(result.gradient_energy_defect));
    check("analytic midpoint defect identity", result.valid
        && result.midpoint_identity_residual <= gate);
    check("discrete-gradient exact energy", result.valid
        && std::abs(result.gradient_energy_defect) <= gate);
  }

  const auto witness = ftd::eft::evaluate_fixed_step_energy_scope(
      0.0, 1.0, 1.0);
  check("rational p0", std::abs(witness.midpoint_p0 - 17.0 / 16.0) <= gate);
  check("rational p1", std::abs(witness.midpoint_p1 - 15.0 / 16.0) <= gate);
  check("rational energy defect",
      std::abs(witness.midpoint_energy_defect - 1.0 / 8.0) <= gate);
  check("rational discrete energy",
      std::abs(witness.discrete_lagrangian_energy - 33.0 / 64.0) <= gate);
  check("endpoint energy is not conserved",
      std::abs(witness.midpoint_energy_defect) > 1e-6);
  check("gradient witness p0",
      std::abs(witness.gradient_p0 - 9.0 / 8.0) <= gate);
  check("gradient witness p1",
      std::abs(witness.gradient_p1 - 7.0 / 8.0) <= gate);
  check("gradient area determinant",
      std::abs(witness.gradient_area_determinant - 9.0 / 11.0) <= gate);
  check("gradient is generically non-area-preserving",
      std::abs(witness.gradient_area_defect) > 1e-6);

  check("zero step fails closed",
      !ftd::eft::evaluate_fixed_step_energy_scope(0.0, 1.0, 0.0).valid);
  check("nonfinite input fails closed",
      !ftd::eft::evaluate_fixed_step_energy_scope(
          0.0, std::numeric_limits<double>::quiet_NaN(), 1.0).valid);

  std::cout.precision(17);
  std::cout << "midpoint_p0=" << witness.midpoint_p0 << '\n'
            << "midpoint_p1=" << witness.midpoint_p1 << '\n'
            << "midpoint_energy_defect="
            << witness.midpoint_energy_defect << '\n'
            << "discrete_lagrangian_energy="
            << witness.discrete_lagrangian_energy << '\n'
            << "gradient_energy_defect="
            << witness.gradient_energy_defect << '\n'
            << "gradient_area_determinant="
            << witness.gradient_area_determinant << '\n'
            << "gradient_area_defect="
            << witness.gradient_area_defect << '\n'
            << "worst_midpoint_identity_residual="
            << worst_midpoint_identity << '\n'
            << "worst_gradient_energy_defect="
            << worst_gradient_energy << '\n'
            << "fixed_step_energy_scope failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

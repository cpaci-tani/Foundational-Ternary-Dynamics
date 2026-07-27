/** FTD-0570: exact-real natural extension and symplectic genesis boundary. */

#include "ftd/eft/genesis_natural_extension.h"

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
  const auto result = ftd::eft::analyze_genesis_natural_extension();

  check("48 generalized-baker arms executed",
        result.baker_arms == 48);
  check("exact-real two-sided Bernoulli natural extension closes",
        result.exact_real_natural_extension
        && result.maximum_baker_inverse_residual <= 1e-12
        && result.maximum_baker_jacobian_residual <= 1e-12);
  check("4,320 branchwise generating-function arms executed",
        result.lift_arms == 4320 && result.accepted_lift_arms == 2160);
  check("raw production genesis is not canonical on the J/W pair",
        result.raw_genesis_is_not_canonical
        && result.minimum_raw_tangential_defect_magnitude > 0.0
        && result.maximum_raw_volume_jacobian < 1.0
        && result.maximum_raw_symplectic_formula_residual <= 1e-12);
  check("added-conjugate branchwise lift is invertible",
        result.branchwise_symplectic_energy_lift
        && result.maximum_lift_inverse_residual <= 1e-11);
  check("added reservoir closes the quadratic energy ledger",
        result.maximum_energy_residual <= 1e-12
        && result.maximum_reservoir_generator_residual <= 1e-12);
  check("all type-2 generating equations close",
        result.maximum_phase_generator_residual <= 1e-12
        && result.maximum_conjugate_generator_residual <= 1e-12);
  check("binary64 histories collide while exact-real history is unbounded",
        result.binary64_history_collision
        && result.exact_real_is_infinite_information);
  check("projected production transition is absolutely irreversible",
        result.projected_kernel_absolutely_irreversible
        && std::isinf(result.projected_log_forward_reverse_ratio));
  check("the exact lift requires additional primitives",
        result.additional_primitives_required
        && !result.production_common_action_recovered);
  check("registered FTD-0570 verdict closes",
        result.valid);

  const ftd::eft::NaturalExtensionPhase invalid{-0.1, 0.5};
  check("invalid phase inputs fail closed",
        !ftd::eft::advance_natural_extension_phase(invalid, 0.5).valid
        && !ftd::eft::advance_natural_extension_phase({0.1, 0.5}, 0.0).valid
        && !ftd::eft::reverse_natural_extension_phase({0.1, 1.0}, 0.5).valid);

  std::cout.precision(17);
  std::cout << "baker_arms=" << result.baker_arms << '\n'
            << "lift_arms=" << result.lift_arms << '\n'
            << "accepted_lift_arms=" << result.accepted_lift_arms << '\n'
            << "maximum_baker_inverse_residual="
            << result.maximum_baker_inverse_residual << '\n'
            << "maximum_lift_inverse_residual="
            << result.maximum_lift_inverse_residual << '\n'
            << "maximum_energy_residual="
            << result.maximum_energy_residual << '\n'
            << "minimum_raw_tangential_defect_magnitude="
            << result.minimum_raw_tangential_defect_magnitude << '\n'
            << "maximum_raw_volume_jacobian="
            << result.maximum_raw_volume_jacobian << '\n'
            << "genesis_natural_extension failures=" << failures << '\n'
            << "verdict=EXACT_REAL_NATURAL_EXTENSION_ADDITIONAL_PRIMITIVES_REQUIRED\n";
  return failures == 0 ? 0 : 1;
}

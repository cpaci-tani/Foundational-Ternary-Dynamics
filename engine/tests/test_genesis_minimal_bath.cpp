/** FTD-0572: minimum bath-rank and prepared-dilation theorem. */

#include "ftd/eft/genesis_minimal_bath.h"

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
  const auto result = ftd::eft::analyze_genesis_minimal_bath();

  check("120 matrix arms execute", result.matrix_arms == 120);
  check("360 canonical-pair arms execute", result.pair_arms == 360);
  check("330 defective pairs require bath coupling",
        result.defective_pair_arms == 330);
  check("zero-drain defect requires two bath pairs",
        result.rank_four_arms == 30
        && result.minimum_bath_pairs_zero_drain == 2);
  check("positive drain requires three bath pairs",
        result.rank_six_arms == 90
        && result.minimum_bath_pairs_positive_drain == 3);
  check("rank lower bound closes", result.rank_lower_bound_proved);
  check("feedback and record ranks saturate the lower bound",
        result.feedback_and_record_ranks_saturate);
  check("minimum pair dilation is symplectic",
        result.maximum_pair_symplectic_residual <= 1e-12);
  check("prepared zero-bath projection reproduces genesis",
        result.maximum_prepared_projection_residual <= 1e-12);
  check("second-step bath feedback matches analytic deviation",
        result.maximum_two_step_formula_residual <= 1e-12
        && result.minimum_nonzero_two_step_deviation > 0.0);
  check("fixed zero-bath section cannot repeat",
        result.fixed_zero_bath_section_cannot_repeat);
  check("passive equal-weight quadratic energy is obstructed",
        result.passive_equal_weight_energy_obstructed
        && result.minimum_passive_commutator > 0.0);
  check("registered FTD-0572 verdict closes",
        result.valid && result.reset_or_active_energy_reservoir_required);

  std::cout.precision(17);
  std::cout << "matrix_arms=" << result.matrix_arms << '\n'
            << "pair_arms=" << result.pair_arms << '\n'
            << "defective_pair_arms=" << result.defective_pair_arms << '\n'
            << "rank_four_arms=" << result.rank_four_arms << '\n'
            << "rank_six_arms=" << result.rank_six_arms << '\n'
            << "maximum_pair_symplectic_residual="
            << result.maximum_pair_symplectic_residual << '\n'
            << "maximum_prepared_projection_residual="
            << result.maximum_prepared_projection_residual << '\n'
            << "maximum_two_step_formula_residual="
            << result.maximum_two_step_formula_residual << '\n'
            << "minimum_nonzero_two_step_deviation="
            << result.minimum_nonzero_two_step_deviation << '\n'
            << "minimum_passive_commutator="
            << result.minimum_passive_commutator << '\n'
            << "genesis_minimal_bath failures=" << failures << '\n'
            << "verdict=MINIMAL_FEEDBACK_DILATION_REQUIRES_RESET_OR_ACTIVE_ENERGY_RESERVOIR\n";
  return failures == 0 ? 0 : 1;
}

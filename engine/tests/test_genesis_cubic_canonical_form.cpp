/** FTD-0573: O_h canonical-form uniqueness and bath-rank price. */

#include "ftd/eft/genesis_cubic_canonical_form.h"

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
  const auto result = ftd::eft::analyze_genesis_cubic_canonical_form();

  check("48-element full cubic group enumerated",
        result.full_cubic_group_elements == 48);
  check("24-element proper cubic group enumerated",
        result.proper_cubic_group_elements == 24);
  check("invariant skew-form constraint rank is 14",
        result.invariant_constraint_rank == 14
        && result.invariant_nullity == 1);
  check("standard J-W pairing is invariant under O_h",
        result.maximum_cubic_invariance_residual <= 1e-12);
  check("standard pairing is unique up to nonzero scale",
        result.standard_pairing_unique_up_to_scale);
  check("120 cubic production arms execute",
        result.production_arms == 120);
  check("zero-drain unconstrained minimum is rank two",
        result.zero_drain_alternative_arms == 30
        && result.zero_drain_unconstrained_minimum_rank_two);
  check("generic positive-drain unconstrained minimum is rank four",
        result.positive_drain_alternative_arms == 90
        && result.generic_unconstrained_minimum_rank_four);
  check("a=t repeated-eigenspace minimum is rank six",
        result.degenerate_a_equals_t_arms == 30
        && result.degenerate_minimum_rank_six);
  check("generic determinant formula closes",
        result.maximum_generic_determinant_formula_residual <= 1e-12
        && result.minimum_generic_alternative_determinant > 0.0);
  check("cubic covariance prices one bath pair in all arms",
        result.symmetry_price_arms == 120
        && result.cubic_covariance_prices_one_bath_pair);
  check("branchwise alternatives are not promoted as one global form",
        result.branchwise_alternatives_are_not_one_global_form
        && !result.native_canonical_action_derived);
  check("registered FTD-0573 verdict closes", result.valid);

  std::cout.precision(17);
  std::cout << "full_cubic_group_elements="
            << result.full_cubic_group_elements << '\n'
            << "proper_cubic_group_elements="
            << result.proper_cubic_group_elements << '\n'
            << "invariant_constraint_rank="
            << result.invariant_constraint_rank << '\n'
            << "invariant_nullity=" << result.invariant_nullity << '\n'
            << "production_arms=" << result.production_arms << '\n'
            << "zero_drain_alternative_arms="
            << result.zero_drain_alternative_arms << '\n'
            << "positive_drain_alternative_arms="
            << result.positive_drain_alternative_arms << '\n'
            << "degenerate_a_equals_t_arms="
            << result.degenerate_a_equals_t_arms << '\n'
            << "symmetry_price_arms=" << result.symmetry_price_arms << '\n'
            << "maximum_cubic_invariance_residual="
            << result.maximum_cubic_invariance_residual << '\n'
            << "maximum_generic_determinant_formula_residual="
            << result.maximum_generic_determinant_formula_residual << '\n'
            << "minimum_generic_alternative_determinant="
            << result.minimum_generic_alternative_determinant << '\n'
            << "genesis_cubic_canonical_form failures=" << failures << '\n'
            << "verdict=CUBIC_COVARIANCE_SELECTS_STANDARD_PAIRING_AND_PRICES_ONE_BATH_PAIR\n";
  return failures == 0 ? 0 : 1;
}

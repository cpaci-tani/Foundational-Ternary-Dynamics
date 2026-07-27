/** FTD-0571: environment-feedback necessity for noncanonical genesis. */

#include "ftd/eft/genesis_environment_feedback.h"

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
  const auto result = ftd::eft::analyze_genesis_environment_feedback();

  check("block-triangular symplectic theorem closes",
        result.block_triangular_symplectic_theorem
        && result.environment_independent_projection_requires_native_symplecticity);
  check("90 raw-genesis matrix arms executed",
        result.matrix_arms == 90);
  check("zero-drain defect has rank four",
        result.rank_four_arms == 30);
  check("positive-drain defect has rank six",
        result.rank_six_arms == 60);
  check("analytic and matrix defects agree",
        result.raw_genesis_defect_has_registered_rank
        && result.maximum_defect_formula_residual <= 1e-12
        && result.maximum_determinant_formula_residual <= 1e-12);
  check("raw branch remains noncanonical and volume contracting",
        result.minimum_nonzero_symplectic_defect > 0.0
        && result.maximum_raw_volume_jacobian < 1.0);
  check("34 existing continuous Voxel components are event spectators",
        result.continuous_spectator_components == 34
        && result.existing_continuous_spectators_are_unchanged);
  check("stateless RNG is not incoming dynamical bath state",
        result.stateless_rng_is_not_dynamical_bath_state);
  check("prepared-bath loophole requires feedback or reset",
        result.prepared_bath_requires_feedback_or_reset);
  check("existing spectators do not close the native action",
        !result.existing_spectators_close_native_action);
  check("registered FTD-0571 verdict closes",
        result.valid && result.environment_feedback_or_reset_required);

  std::cout.precision(17);
  std::cout << "matrix_arms=" << result.matrix_arms << '\n'
            << "rank_four_arms=" << result.rank_four_arms << '\n'
            << "rank_six_arms=" << result.rank_six_arms << '\n'
            << "continuous_spectator_components="
            << result.continuous_spectator_components << '\n'
            << "maximum_defect_formula_residual="
            << result.maximum_defect_formula_residual << '\n'
            << "minimum_nonzero_symplectic_defect="
            << result.minimum_nonzero_symplectic_defect << '\n'
            << "maximum_raw_volume_jacobian="
            << result.maximum_raw_volume_jacobian << '\n'
            << "genesis_environment_feedback failures=" << failures << '\n'
            << "verdict=ENVIRONMENT_FEEDBACK_OR_RESET_REQUIRED\n";
  return failures == 0 ? 0 : 1;
}

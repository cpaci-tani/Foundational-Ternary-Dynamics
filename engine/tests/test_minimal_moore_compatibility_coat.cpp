/** FTD-0577: minimal Moore compatibility-coat audit. */

#include "ftd/eft/minimal_moore_compatibility_coat.h"

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
  const auto result = ftd::eft::analyze_minimal_moore_compatibility_coat();

  check("symmetric normalized radius-one filter is uniquely fixed",
        result.scoped_radius_one_filter_unique
        && result.radius_one_a == 0.25
        && result.radius_one_b == 0.5
        && result.maximum_filter_equation_residual <= 1e-12);
  check("27-site Moore coat is positive, normalized, and centered",
        result.integer_coat_positive_and_normalized
        && result.integer_coat_sites == 27
        && result.center_weight == 1.0 / 8.0
        && result.face_weight == 1.0 / 16.0
        && result.edge_weight == 1.0 / 32.0
        && result.corner_weight == 1.0 / 64.0);
  check("36 path arms preserve polarity partition and first moment",
        result.path_arms == 36
        && result.polarity_arms == 4
        && result.volume_arms == 2
        && result.trilinear_moments_preserved
        && result.maximum_partition_residual <= 1e-12
        && result.maximum_first_moment_residual <= 1e-12
        && result.maximum_wrong_sign_weight_residual <= 1e-12);
  check("face continuity maps exactly to central continuity",
        result.local_central_continuity_exact
        && result.maximum_central_continuity_residual <= 1e-12);
  check("support remains bounded as volume grows",
        result.local_support_volume_independent
        && result.minimum_local_rho_support > 0
        && result.maximum_local_rho_support > 0
        && result.minimum_local_current_support > 0
        && result.maximum_local_current_support > 0);
  check("integer translations commute with the coat and current bridge",
        result.integer_translation_covariant
        && result.translation_arms == 3
        && result.maximum_translation_covariance_residual <= 1e-12);
  check("24 proper-cubic rotations commute with the construction",
        result.proper_cubic_covariant
        && result.proper_cubic_rotation_arms == 24
        && result.maximum_cubic_covariance_residual <= 1e-12);
  check("zero mode survives and central checkerboard nulls are removed",
        result.checkerboard_nulls_removed_from_source
        && result.maximum_zero_mode_residual <= 1e-12
        && result.maximum_checkerboard_response <= 1e-12);
  check("four inherited conditional Hodge energy ledgers close",
        result.conditional_hodge_energy_compatible
        && result.conditional_energy_arms == 4
        && result.maximum_conditional_field_work_residual <= 1e-12
        && result.maximum_conditional_interaction_residual <= 1e-12
        && result.maximum_conditional_total_energy_residual <= 1e-12);
  check("the coupling coat is explicitly non-cardinal",
        !result.coupling_representation_is_cardinal
        && result.integer_center_cardinality_defect >= 7.0 / 8.0 - 1e-12);
  check("no force, Coulomb pole, mobile particle, or production claim is promoted",
        !result.reciprocal_force_derived
        && !result.static_coulomb_pole_recovered
        && !result.mobile_manifested_solution_derived
        && !result.production_changed);
  check("registered FTD-0577 verdict closes", result.valid);

  std::cout.precision(17);
  std::cout << "path_arms=" << result.path_arms << '\n'
            << "polarity_arms=" << result.polarity_arms << '\n'
            << "volume_arms=" << result.volume_arms << '\n'
            << "translation_arms=" << result.translation_arms << '\n'
            << "proper_cubic_rotation_arms="
            << result.proper_cubic_rotation_arms << '\n'
            << "conditional_energy_arms="
            << result.conditional_energy_arms << '\n'
            << "integer_coat_sites=" << result.integer_coat_sites << '\n'
            << "minimum_local_rho_support="
            << result.minimum_local_rho_support << '\n'
            << "maximum_local_rho_support="
            << result.maximum_local_rho_support << '\n'
            << "minimum_local_current_support="
            << result.minimum_local_current_support << '\n'
            << "maximum_local_current_support="
            << result.maximum_local_current_support << '\n'
            << "maximum_filter_equation_residual="
            << result.maximum_filter_equation_residual << '\n'
            << "maximum_partition_residual="
            << result.maximum_partition_residual << '\n'
            << "maximum_first_moment_residual="
            << result.maximum_first_moment_residual << '\n'
            << "maximum_wrong_sign_weight_residual="
            << result.maximum_wrong_sign_weight_residual << '\n'
            << "maximum_central_continuity_residual="
            << result.maximum_central_continuity_residual << '\n'
            << "maximum_translation_covariance_residual="
            << result.maximum_translation_covariance_residual << '\n'
            << "maximum_cubic_covariance_residual="
            << result.maximum_cubic_covariance_residual << '\n'
            << "maximum_zero_mode_residual="
            << result.maximum_zero_mode_residual << '\n'
            << "maximum_checkerboard_response="
            << result.maximum_checkerboard_response << '\n'
            << "maximum_conditional_field_work_residual="
            << result.maximum_conditional_field_work_residual << '\n'
            << "maximum_conditional_interaction_residual="
            << result.maximum_conditional_interaction_residual << '\n'
            << "maximum_conditional_total_energy_residual="
            << result.maximum_conditional_total_energy_residual << '\n'
            << "integer_center_cardinality_defect="
            << result.integer_center_cardinality_defect << '\n'
            << "minimal_moore_compatibility_coat failures=" << failures << '\n'
            << "verdict=MINIMAL_MOORE_COAT_RESTORES_LOCAL_CENTRAL_CONTINUITY_NONCARDINAL_SELECTED\n";
  return failures == 0 ? 0 : 1;
}

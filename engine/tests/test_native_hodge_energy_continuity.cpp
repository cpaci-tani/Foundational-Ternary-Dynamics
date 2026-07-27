/** FTD-0576: native Hodge energy and central-continuity audit. */

#include "ftd/eft/native_hodge_energy_continuity.h"

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
  const auto result = ftd::eft::analyze_native_hodge_energy_continuity();
  check("36 mode work arms close", result.mode_work_arms == 36
      && result.maximum_mode_work_residual <= 1e-12);
  check("four full-field driven work arms close",
        result.full_field_work_arms == 4
        && result.maximum_full_field_work_residual <= 1e-12);
  check("R=J-W/2 is the exact work coordinate",
        result.half_step_coordinate_unique
        && result.maximum_half_step_coordinate_residual <= 1e-12);
  check("constant prescribed source has an affine invariant",
        result.constant_source_affine_invariant_exact);
  check("conditional Hodge total energy closes",
        result.conditional_energy_arms == 4
        && result.conditional_hodge_total_energy_exact
        && result.maximum_conditional_total_energy_residual <= 1e-12);
  check("even cardinal hops violate the central checkerboard nullspace",
        !result.even_cardinal_hop_central_current_exists
        && result.minimum_even_checkerboard_witness >= 2.0 - 1e-12);
  check("odd central-current solutions are box spanning",
        result.odd_cardinal_hop_current_is_box_spanning
        && result.maximum_odd_volume_current_residual <= 1e-12
        && result.minimum_odd_support_fraction >= 1.0 - 1e-12);
  check("no finite-range cardinal-hop central current exists",
        !result.finite_range_cardinal_hop_current_exists);
  check("24 proper-cubic rotations preserve classification",
        result.proper_cubic_rotation_arms == 24
        && result.maximum_cubic_covariance_residual <= 1e-12);
  check("no finite-range face-to-site commuting projection exists",
        !result.finite_range_face_to_site_projection_exists
        && result.face_to_site_checkerboard_defect >= 2.0 - 1e-12);
  check("additional staggered or nonlocal structure is required",
        result.additional_staggered_or_nonlocal_structure_required
        && !result.production_changed);
  check("registered FTD-0576 verdict closes", result.valid);

  std::cout.precision(17);
  std::cout << "mode_work_arms=" << result.mode_work_arms << '\n'
            << "full_field_work_arms=" << result.full_field_work_arms << '\n'
            << "conditional_energy_arms=" << result.conditional_energy_arms << '\n'
            << "axial_cardinal_hop_arms=" << result.axial_cardinal_hop_arms << '\n'
            << "polarity_checks=" << result.polarity_checks << '\n'
            << "proper_cubic_rotation_arms="
            << result.proper_cubic_rotation_arms << '\n'
            << "maximum_mode_work_residual="
            << result.maximum_mode_work_residual << '\n'
            << "maximum_full_field_work_residual="
            << result.maximum_full_field_work_residual << '\n'
            << "maximum_half_step_coordinate_residual="
            << result.maximum_half_step_coordinate_residual << '\n'
            << "maximum_conditional_continuity_residual="
            << result.maximum_conditional_continuity_residual << '\n'
            << "maximum_conditional_field_work_residual="
            << result.maximum_conditional_field_work_residual << '\n'
            << "maximum_conditional_interaction_residual="
            << result.maximum_conditional_interaction_residual << '\n'
            << "maximum_conditional_total_energy_residual="
            << result.maximum_conditional_total_energy_residual << '\n'
            << "maximum_odd_volume_current_residual="
            << result.maximum_odd_volume_current_residual << '\n'
            << "minimum_even_checkerboard_witness="
            << result.minimum_even_checkerboard_witness << '\n'
            << "minimum_odd_support_fraction="
            << result.minimum_odd_support_fraction << '\n'
            << "minimum_odd_support_sites="
            << result.minimum_odd_support_sites << '\n'
            << "maximum_odd_support_sites="
            << result.maximum_odd_support_sites << '\n'
            << "minimum_odd_support_radius="
            << result.minimum_odd_support_radius << '\n'
            << "maximum_odd_support_radius="
            << result.maximum_odd_support_radius << '\n'
            << "maximum_cubic_covariance_residual="
            << result.maximum_cubic_covariance_residual << '\n'
            << "face_to_site_checkerboard_defect="
            << result.face_to_site_checkerboard_defect << '\n'
            << "native_hodge_energy_continuity failures=" << failures << '\n'
            << "verdict=NATIVE_HODGE_ENERGY_IDENTITY_CENTRAL_LOCAL_MOBILE_CURRENT_OBSTRUCTED\n";
  return failures == 0 ? 0 : 1;
}

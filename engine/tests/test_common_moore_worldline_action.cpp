/** FTD-0578: common Moore spacetime/action and self-force audit. */

#include "ftd/eft/common_moore_worldline_action.h"

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
  const auto r = ftd::eft::analyze_common_moore_worldline_action();
  check("104 signed Moore paths obey split and aggregate continuity",
        r.coated_spacetime_continuity_exact
        && r.aggregate_split_arms == 104
        && r.maximum_temporal_partition_residual <= 1e-12
        && r.maximum_current_reconstruction_residual <= 1e-12
        && r.maximum_split_continuity_residual <= 1e-12
        && r.maximum_aggregate_continuity_residual <= 1e-12);
  check("one common action gives deposited source and reciprocal gather",
        r.common_action_deposition_and_gather_adjoint
        && r.reciprocal_path_gather_derived
        && r.action_fixture_arms == 4
        && r.maximum_deposit_orbit_action_residual <= 1e-12
        && r.maximum_endpoint_field_adjoint_residual <= 1e-12);
  check("magnetic exchange performs identically zero scalar work",
        r.magnetic_scalar_work_zero
        && r.maximum_magnetic_scalar_work_residual <= 1e-12);
  check("integer translations and the 24 proper cubic rotations close",
        r.translation_arms == 3
        && r.proper_cubic_rotation_arms == 24
        && r.maximum_translation_covariance_residual <= 1e-12
        && r.maximum_cubic_covariance_residual <= 1e-12);
  check("axial time average equals endpoint midpoint density",
        r.axial_energy_centering_exact
        && r.axial_centering_norm2 <= 1e-12);
  check("edge and body diagonals have the registered rational mismatch",
        r.diagonal_energy_centering_fails
        && r.edge_centering_norm2 > 1e-6
        && r.body_centering_norm2 > 1e-6
        && r.maximum_centering_rational_residual <= 1e-12);
  check("the compact point coat has a polarity-even cubic Peierls barrier",
        r.point_carrier_peierls_pinned
        && r.peierls_arms == 108
        && r.minimum_peierls_coefficient > 1e-8
        && r.minimum_peierls_barrier > 1e-8
        && r.maximum_peierls_law_residual <= 1e-12
        && r.maximum_peierls_polarity_residual <= 1e-12
        && r.maximum_peierls_cubic_residual <= 1e-12);
  check("unmodified point action is not promoted as free mobile matter",
        !r.unmodified_action_is_free_mobile_law && !r.production_changed);
  check("registered FTD-0578 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "aggregate_split_arms=" << r.aggregate_split_arms << '\n'
            << "polarity_arms=" << r.polarity_arms << '\n'
            << "volume_arms=" << r.volume_arms << '\n'
            << "translation_arms=" << r.translation_arms << '\n'
            << "proper_cubic_rotation_arms=" << r.proper_cubic_rotation_arms << '\n'
            << "action_fixture_arms=" << r.action_fixture_arms << '\n'
            << "peierls_arms=" << r.peierls_arms << '\n'
            << "maximum_temporal_partition_residual=" << r.maximum_temporal_partition_residual << '\n'
            << "maximum_current_reconstruction_residual=" << r.maximum_current_reconstruction_residual << '\n'
            << "maximum_split_continuity_residual=" << r.maximum_split_continuity_residual << '\n'
            << "maximum_aggregate_continuity_residual=" << r.maximum_aggregate_continuity_residual << '\n'
            << "maximum_deposit_orbit_action_residual=" << r.maximum_deposit_orbit_action_residual << '\n'
            << "maximum_endpoint_field_adjoint_residual=" << r.maximum_endpoint_field_adjoint_residual << '\n'
            << "maximum_magnetic_scalar_work_residual=" << r.maximum_magnetic_scalar_work_residual << '\n'
            << "maximum_translation_covariance_residual=" << r.maximum_translation_covariance_residual << '\n'
            << "maximum_cubic_covariance_residual=" << r.maximum_cubic_covariance_residual << '\n'
            << "axial_centering_norm2=" << r.axial_centering_norm2 << '\n'
            << "edge_centering_norm2=" << r.edge_centering_norm2 << '\n'
            << "body_centering_norm2=" << r.body_centering_norm2 << '\n'
            << "maximum_centering_rational_residual=" << r.maximum_centering_rational_residual << '\n'
            << "minimum_peierls_coefficient=" << r.minimum_peierls_coefficient << '\n'
            << "minimum_peierls_barrier=" << r.minimum_peierls_barrier << '\n'
            << "maximum_peierls_law_residual=" << r.maximum_peierls_law_residual << '\n'
            << "common_moore_worldline_action failures=" << failures << '\n'
            << "verdict=COMMON_MOORE_WORLDLINE_ACTION_DERIVED_ENERGY_CENTERING_MISMATCH_PEIERLS_PINNED\n";
  return failures == 0 ? 0 : 1;
}

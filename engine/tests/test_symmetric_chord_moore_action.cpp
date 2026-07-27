/** FTD-0580: symmetric chord Moore-action observer. */

#include "ftd/eft/symmetric_chord_moore_action.h"

#include <iostream>
#include <string>

namespace {
int failures = 0;
void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}
}

int main() {
  const auto r = ftd::eft::analyze_symmetric_chord_moore_action();
  check("positive centered chord shape is the registered unique representative",
        r.positive_centered_shape_unique && r.shape_samples == 936
        && r.maximum_partition_residual <= 1e-12
        && r.maximum_first_moment_residual <= 1e-12
        && r.maximum_wrong_sign_residual <= 1e-12);
  check("democratic shortest-path current closes raw and central continuity",
        r.democratic_shortest_route_exact && r.path_arms == 104
        && r.maximum_raw_continuity_residual <= 1e-12
        && r.maximum_central_continuity_residual <= 1e-12
        && r.cubic_rotation_arms == 24
        && r.maximum_cubic_covariance_residual <= 1e-12);
  check("time-exact chord action is endpoint-energy-centered on every Moore path",
        r.common_action_energy_centered
        && r.maximum_temporal_centering_residual <= 1e-12
        && r.maximum_split_continuity_residual <= 1e-12);
  check("all 104 chord Peierls coefficients remain strictly positive",
        r.every_peierls_barrier_positive
        && r.peierls_coefficient_arms == 104
        && r.peierls_potential_samples == 936
        && r.minimum_peierls_coefficient > 1e-14
        && r.minimum_peierls_barrier > 1e-14
        && r.maximum_peierls_law_residual <= 1e-12
        && r.maximum_polarity_residual <= 1e-12);
  check("no gapless particle, production behavior, toggle, or scenario is promoted",
        !r.gapless_mobile_law_derived && !r.production_changed);
  check("registered FTD-0580 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "shape_samples=" << r.shape_samples << '\n'
    << "path_arms=" << r.path_arms << '\n'
    << "peierls_coefficient_arms=" << r.peierls_coefficient_arms << '\n'
    << "peierls_potential_samples=" << r.peierls_potential_samples << '\n'
    << "cubic_rotation_arms=" << r.cubic_rotation_arms << '\n'
    << "maximum_partition_residual=" << r.maximum_partition_residual << '\n'
    << "maximum_first_moment_residual=" << r.maximum_first_moment_residual << '\n'
    << "maximum_wrong_sign_residual=" << r.maximum_wrong_sign_residual << '\n'
    << "maximum_raw_continuity_residual=" << r.maximum_raw_continuity_residual << '\n'
    << "maximum_central_continuity_residual=" << r.maximum_central_continuity_residual << '\n'
    << "maximum_temporal_centering_residual=" << r.maximum_temporal_centering_residual << '\n'
    << "maximum_split_continuity_residual=" << r.maximum_split_continuity_residual << '\n'
    << "maximum_peierls_law_residual=" << r.maximum_peierls_law_residual << '\n'
    << "maximum_polarity_residual=" << r.maximum_polarity_residual << '\n'
    << "maximum_cubic_covariance_residual=" << r.maximum_cubic_covariance_residual << '\n'
    << "minimum_peierls_coefficient=" << r.minimum_peierls_coefficient << '\n'
    << "minimum_peierls_barrier=" << r.minimum_peierls_barrier << '\n'
    << "symmetric_chord_moore_action failures=" << failures << '\n'
    << "verdict=SYMMETRIC_CHORD_CLOSES_MOORE_CENTERING_PEIERLS_PINNING_REMAINS\n";
  return failures == 0 ? 0 : 1;
}


/** FTD-0581: passive-dressing/depinning observer. */

#include "ftd/eft/passive_dressing_depinning_obstruction.h"

#include "ftd/constants.h"

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
  const auto r =
      ftd::eft::analyze_passive_dressing_depinning_obstruction();
  check("all 104 Moore arms have exact positive relativistic thresholds",
        r.exact_relativistic_depinning && r.depinning_arms == 104
        && r.minimum_peierls_coefficient > 1e-14
        && r.minimum_barrier > 1e-14
        && r.minimum_depinning_momentum > 0.0
        && r.minimum_depinning_speed > 0.0
        && r.maximum_depinning_speed < ftd::C_SPEED
        && r.maximum_threshold_energy_residual <= 1e-12
        && r.maximum_inverse_momentum_residual <= 1e-12
        && r.maximum_velocity_identity_residual <= 1e-12);
  check("thresholds are polarity-even and proper-cubic covariant",
        r.maximum_polarity_residual <= 1e-12
        && r.cubic_rotation_arms == 24
        && r.maximum_cubic_covariance_residual <= 1e-12);
  check("positive completed-square passive dressing cannot lower the curve",
        r.passive_completed_square_obstruction
        && r.passive_fixture_arms == 416
        && r.passive_samples == 3744
        && r.maximum_passive_linear_coefficient <= 1e-12
        && r.maximum_passive_negative_excess <= 1e-12);
  check("a Lipschitz passive response cannot cancel the linear Peierls cusp",
        r.passive_cusp_obstruction
        && r.minimum_cusp_slope_gap > 1e-14);
  check("zero-momentum active traversal costs at least one finite barrier",
        r.active_excitation_lower_bound
        && r.active_budget_samples == 2808
        && r.equality_nondifferentiable_arms == 104
        && r.smooth_excited_arms == 208
        && r.maximum_active_budget_residual <= 1e-12
        && r.maximum_equality_midpoint_residual <= 1e-12
        && r.minimum_equality_derivative_jump > 1e-14);
  check("energy budget is not promoted to dynamics or production",
        !r.active_common_action_derived && !r.production_changed);
  check("registered FTD-0581 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "depinning_arms=" << r.depinning_arms << '\n'
    << "passive_fixture_arms=" << r.passive_fixture_arms << '\n'
    << "passive_samples=" << r.passive_samples << '\n'
    << "active_budget_samples=" << r.active_budget_samples << '\n'
    << "equality_nondifferentiable_arms="
    << r.equality_nondifferentiable_arms << '\n'
    << "smooth_excited_arms=" << r.smooth_excited_arms << '\n'
    << "cubic_rotation_arms=" << r.cubic_rotation_arms << '\n'
    << "maximum_threshold_energy_residual="
    << r.maximum_threshold_energy_residual << '\n'
    << "maximum_inverse_momentum_residual="
    << r.maximum_inverse_momentum_residual << '\n'
    << "maximum_velocity_identity_residual="
    << r.maximum_velocity_identity_residual << '\n'
    << "maximum_polarity_residual=" << r.maximum_polarity_residual << '\n'
    << "maximum_cubic_covariance_residual="
    << r.maximum_cubic_covariance_residual << '\n'
    << "maximum_passive_linear_coefficient="
    << r.maximum_passive_linear_coefficient << '\n'
    << "maximum_passive_negative_excess="
    << r.maximum_passive_negative_excess << '\n'
    << "minimum_cusp_slope_gap=" << r.minimum_cusp_slope_gap << '\n'
    << "maximum_active_budget_residual="
    << r.maximum_active_budget_residual << '\n'
    << "maximum_equality_midpoint_residual="
    << r.maximum_equality_midpoint_residual << '\n'
    << "minimum_equality_derivative_jump="
    << r.minimum_equality_derivative_jump << '\n'
    << "minimum_peierls_coefficient=" << r.minimum_peierls_coefficient << '\n'
    << "maximum_peierls_coefficient=" << r.maximum_peierls_coefficient << '\n'
    << "minimum_barrier=" << r.minimum_barrier << '\n'
    << "maximum_barrier=" << r.maximum_barrier << '\n'
    << "minimum_depinning_momentum=" << r.minimum_depinning_momentum << '\n'
    << "maximum_depinning_momentum=" << r.maximum_depinning_momentum << '\n'
    << "minimum_depinning_speed=" << r.minimum_depinning_speed << '\n'
    << "maximum_depinning_speed=" << r.maximum_depinning_speed << '\n'
    << "passive_dressing_depinning_obstruction failures=" << failures << '\n'
    << "verdict=PASSIVE_DRESSING_CANNOT_DEPIN_ACTIVE_TRAVERSAL_COSTS_FINITE_EXCITATION\n";
  return failures == 0 ? 0 : 1;
}

#include "ftd/eft/self_pair_connection_critical_gearbox.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

int failures = 0;

void check(const char* name, bool condition) {
  std::cout << (condition ? "PASS  " : "FAIL  ") << name << '\n';
  if (!condition) ++failures;
}

bool close(double left, double right, double tolerance = 1e-10) {
  return std::abs(left - right)
      <= tolerance * std::max({1.0, std::abs(left), std::abs(right)});
}

}  // namespace

int main() {
  using namespace ftd::eft;

  SelfPairConnectionState state;
  state.common_coordinate = 0.3;
  state.relative_coordinate = 0.7;
  state.relative_momentum = -0.12;

  SelfPairConnectionParameters parameters;
  parameters.common_mass = 1.8;
  parameters.relative_mass = 1.7;
  parameters.bare_quartic_coupling = 0.8;
  parameters.gamma = 0.4;
  parameters.step = 0.025;
  parameters.momentum_scale = 0.01;
  parameters.tolerance = 2e-12;
  parameters.max_iterations = 128;
  parameters.moving_common_momentum_projection_probe = 0.6;

  const auto result = analyze_self_pair_connection_critical_gearbox(
      state, parameters);
  check("self-pair connection critical gearbox is valid", result.valid());
  const double connection = parameters.gamma * parameters.gamma
      / (2.0 * parameters.common_mass);
  check("positive connection energy renormalizes only the quartic coupling",
        result.rest_sector_quartic_fold_exact
            && close(result.connection_quartic_contribution, connection)
            && close(result.effective_quartic_coupling,
                     parameters.bare_quartic_coupling + connection));
  check("self-pair origin has zero Jacobian and zero clock Hessian",
        result.origin_connection_derivative_zero
            && result.self_pair_origin_jacobian == 0.0
            && result.critical_clock_hessian == 0.0
            && result.rest_sector_critical_quartic_exact);
  check("nonzero gamma has a nonzero connection derivative away from origin",
        result.connection_derivative_nonzero_away_for_nonzero_gamma
            && result.connection_derivative_before != 0.0);
  check("mechanical common impulse is minus gamma Delta U",
        result.mechanical_common_impulse_exact
            && std::abs(result.mechanical_impulse_residual) <= 1e-12);
  check("symmetric common endpoint update is exact",
        result.common_endpoint_update_exact
            && close(result.after.common_coordinate - state.common_coordinate,
                     result.common_displacement));
  check("effective quartic child conserves rest-sector energy",
        result.relative_energy_exact
            && std::abs(result.rest_energy_residual) <= 1e-10);
  check("canonical channel impulses remain equal and opposite",
        result.channel_impulses_equal_and_opposite);
  check("reciprocal carry composes with the generated impulse",
        result.reciprocal_carry_composition_exact);
  check("signed-step inverse includes the common coordinate",
        result.signed_step_reversal_exact
            && std::abs(result.reverse_common_coordinate_residual) <= 1e-11);
  check("moving sector exposes the registered quadratic ray coefficient",
        result.moving_sector_has_generic_quadratic_term
            && !result.moving_sector_exact_quartic_generic
            && close(result.moving_quadratic_ray_coefficient,
                     -parameters.gamma
                         * parameters.moving_common_momentum_projection_probe
                         / parameters.common_mass));
  check("polarized symmetric full cycle has zero common drift",
        result.polarized_symmetric_full_cycle_drift_zero
            && result.symmetric_full_cycle_drift_residual == 0.0
            && !result.net_transport_derived);
  const double expected_period = std::sqrt(std::acos(-1.0))
      * std::tgamma(0.25) / std::tgamma(0.75)
      * std::sqrt(parameters.relative_mass
          / (2.0 * result.effective_quartic_coupling));
  check("rest-sector continuum period retains the exact Gstar factor",
        result.continuum_gstar_period_factor_exact
            && close(result.continuum_period_amplitude_product,
                     expected_period, 1e-14));
  check("equal self-dual partition value is conditional, not adopted",
        close(result.conditional_equal_partition_gamma_magnitude,
              std::sqrt(2.0 * parameters.common_mass
                  * parameters.bare_quartic_coupling))
            && !result.equal_self_dual_partition_adopted);
  check("i supplies orientation but not gamma magnitude",
        result.i_supplies_orientation && !result.gamma_derived_from_i);
  check("scale, mass, and finite-tick cadence remain open",
        !result.physical_momentum_scale_derived
            && !result.absolute_mass_derived
            && !result.integer_tick_gstar_cadence_derived);
  check("production and Born remain untouched",
        !result.production_coupling_supplied && !result.born_target_used);
  check("no new selected type is added", !result.new_selected_type_added);

  auto zero_gamma = parameters;
  zero_gamma.gamma = 0.0;
  const auto control = analyze_self_pair_connection_critical_gearbox(
      state, zero_gamma);
  check("gamma-zero control removes the connection quartic and impulse",
        control.valid()
            && control.connection_quartic_contribution == 0.0
            && close(control.effective_quartic_coupling,
                     zero_gamma.bare_quartic_coupling)
            && close(control.mechanical_common_momentum_before, 0.0)
            && close(control.mechanical_common_momentum_after, 0.0));

  auto nonfinite = parameters;
  nonfinite.gamma = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite input fails closed",
        analyze_self_pair_connection_critical_gearbox(state, nonfinite).status
            == SelfPairConnectionStatus::NonFiniteInput);
  auto bad_common_mass = parameters;
  bad_common_mass.common_mass = 0.0;
  check("nonpositive common mass fails closed",
        analyze_self_pair_connection_critical_gearbox(
            state, bad_common_mass).status
            == SelfPairConnectionStatus::InvalidCommonMass);
  auto bad_relative_mass = parameters;
  bad_relative_mass.relative_mass = 0.0;
  check("nonpositive relative mass fails closed",
        analyze_self_pair_connection_critical_gearbox(
            state, bad_relative_mass).status
            == SelfPairConnectionStatus::InvalidRelativeMass);
  auto bad_coupling = parameters;
  bad_coupling.bare_quartic_coupling = 0.0;
  check("nonpositive quartic coupling fails closed",
        analyze_self_pair_connection_critical_gearbox(
            state, bad_coupling).status
            == SelfPairConnectionStatus::InvalidQuarticCoupling);
  auto bad_step = parameters;
  bad_step.step = 0.0;
  check("zero step fails closed",
        analyze_self_pair_connection_critical_gearbox(state, bad_step).status
            == SelfPairConnectionStatus::InvalidStep);
  auto bad_tolerance = parameters;
  bad_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_self_pair_connection_critical_gearbox(
            state, bad_tolerance).status
            == SelfPairConnectionStatus::InvalidTolerance);
  auto bad_scale = parameters;
  bad_scale.momentum_scale = 0.0;
  check("nonpositive momentum scale fails closed",
        analyze_self_pair_connection_critical_gearbox(state, bad_scale).status
            == SelfPairConnectionStatus::InvalidMomentumScale);
  auto bad_iterations = parameters;
  bad_iterations.max_iterations = 0;
  check("zero iteration limit fails closed",
        analyze_self_pair_connection_critical_gearbox(
            state, bad_iterations).status
            == SelfPairConnectionStatus::InvalidIterationLimit);
  auto overflow_gamma = parameters;
  overflow_gamma.gamma = std::numeric_limits<double>::max();
  check("effective quartic overflow fails closed",
        analyze_self_pair_connection_critical_gearbox(
            state, overflow_gamma).status
            == SelfPairConnectionStatus::EffectiveCouplingOverflow);
  auto overflow_pair_state = state;
  overflow_pair_state.relative_coordinate =
      std::numeric_limits<double>::max();
  check("signed-pair overflow fails closed",
        analyze_self_pair_connection_critical_gearbox(
            overflow_pair_state, parameters).status
            == SelfPairConnectionStatus::SignedPairOverflow);

  std::cout << "FTD-0902/0903 self-pair connection critical gearbox: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#include "ftd/eft/common_relative_connection_gearbox.h"

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

bool close(double left, double right, double tolerance = 1e-9) {
  return std::abs(left - right)
      <= tolerance * std::max({1.0, std::abs(left), std::abs(right)});
}

bool close_vector(const ftd::eft::ConnectionVector& left,
                  const ftd::eft::ConnectionVector& right,
                  double tolerance = 1e-9) {
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (!close(left[axis], right[axis], tolerance)) return false;
  }
  return true;
}

ftd::eft::ConnectionVector cubic_transform(
    const ftd::eft::ConnectionVector& value) {
  return {-value[2], value[0], -value[1]};
}

ftd::eft::CommonRelativeConnectionState cubic_transform(
    const ftd::eft::CommonRelativeConnectionState& state) {
  return {
      cubic_transform(state.common_coordinate),
      cubic_transform(state.relative_coordinate),
      cubic_transform(state.canonical_common_momentum),
      cubic_transform(state.relative_momentum),
  };
}

bool any_carry(const ftd::eft::ReciprocalCarryTriplet& value) {
  return std::any_of(value.begin(), value.end(), [](std::int64_t carry) {
    return carry != 0;
  });
}

}  // namespace

int main() {
  using namespace ftd::eft;

  CommonRelativeConnectionState state;
  state.common_coordinate = {0.2, -0.1, 0.4};
  state.relative_coordinate = {0.35, -0.2, 0.15};
  state.canonical_common_momentum = {0.5, -0.3, 0.25};
  state.relative_momentum = {0.1, 0.05, -0.08};

  CommonRelativeConnectionParameters parameters;
  parameters.common_mass = 1.8;
  parameters.relative_mass = 1.3;
  parameters.quartic_coupling = 0.7;
  parameters.gamma = 0.4;
  parameters.step = 0.02;
  parameters.momentum_scale = 0.0002;
  parameters.tolerance = 1e-11;
  parameters.max_iterations = 128;

  const auto result = analyze_common_relative_connection_gearbox(
      state, parameters);
  check("common-relative connection gearbox is valid", result.valid());
  check("registered connection action is explicitly imposed",
        result.imposed_connection_action);
  check("nonzero gamma gives the declared connection curvature",
        result.connection_curvature_nonzero_for_gamma_nonzero
            && close(result.connection_curvature, parameters.gamma, 1e-14));
  check("canonical total momentum is exactly conserved",
        result.canonical_total_momentum_exact
            && result.canonical_momentum_residual <= 1e-12);
  check("mechanical common impulse equals minus gamma Delta D",
        result.mechanical_common_impulse_exact
            && result.mechanical_impulse_residual_norm <= 1e-12);
  check("canonical channel impulses remain equal and opposite",
        result.channel_impulses_equal_and_opposite
            && result.channel_impulse_sum_residual_norm <= 1e-12);
  check("registered common Hamiltonian is conserved",
        result.discrete_common_energy_exact
            && std::abs(result.connection_step.energy_residual) <= 1e-9);
  check("reciprocal carry matches the independent endpoint chart",
        result.reciprocal_carry_compatibility_exact
            && result.chart_endpoint_residual <= 1e-8);
  check("small imposed pstar exposes at least one zone crossing",
        any_carry(result.carry_step.carry_first)
            || any_carry(result.carry_step.carry_second));
  check("signed-step inverse recovers state and reciprocal carry",
        result.signed_step_reversal_exact
            && result.reverse_state_residual <= 1e-8
            && result.reverse_carry_residual <= 1e-8);
  check("canonical total angular momentum is conserved",
        result.canonical_angular_momentum_exact
            && result.angular_momentum_residual_norm <= 1e-8);

  const auto transformed = analyze_common_relative_connection_gearbox(
      cubic_transform(state), parameters);
  check("signed cubic transform remains a valid reference step",
        transformed.valid());
  check("endpoint map is covariant under a signed cubic permutation",
        transformed.cubic_covariant_reference_law
            && close_vector(
                transformed.connection_step.after.common_coordinate,
                cubic_transform(result.connection_step.after.common_coordinate))
            && close_vector(
                transformed.connection_step.after.relative_coordinate,
                cubic_transform(result.connection_step.after.relative_coordinate))
            && close_vector(
                transformed.connection_step.after.relative_momentum,
                cubic_transform(result.connection_step.after.relative_momentum)));

  const ConnectionVector expected_tilt = {
      -parameters.gamma * state.canonical_common_momentum[0]
          / parameters.common_mass,
      -parameters.gamma * state.canonical_common_momentum[1]
          / parameters.common_mass,
      -parameters.gamma * state.canonical_common_momentum[2]
          / parameters.common_mass,
  };
  check("nonzero common momentum tilts the relative origin",
        close_vector(result.clock_origin_tilt, expected_tilt, 1e-14));
  check("continuous connection adds the gamma-squared clock Hessian",
        close(result.critical_clock_hessian,
              parameters.gamma * parameters.gamma / parameters.common_mass,
              1e-14)
            && !result.critical_quartic_preserved
            && !result.continuous_nonzero_connection_preserves_critical_quartic);

  auto control_parameters = parameters;
  control_parameters.gamma = 0.0;
  control_parameters.momentum_scale = 1.0;
  const auto control = analyze_common_relative_connection_gearbox(
      state, control_parameters);
  check("gamma-zero critical-quartic control remains valid",
        control.valid() && control.critical_quartic_preserved
            && control.critical_clock_hessian == 0.0);
  check("gamma-zero control turns off mechanical common impulse",
        close_vector(control.mechanical_common_before,
                     control.mechanical_common_after, 1e-12));

  check("i supplies orientation but does not derive gamma magnitude",
        result.i_supplies_orientation && !result.gamma_derived_from_i);
  check("time reversal remains conditional on channel exchange",
        result.conditional_channel_exchange_time_reversal);
  check("physical coordinate, scale, and absolute mass remain open",
        !result.physical_common_coordinate_identified
            && !result.physical_momentum_scale_derived
            && !result.absolute_mass_derived);
  check("finite-tick Gstar cadence and variational provenance remain open",
        !result.integer_tick_gstar_cadence_derived
            && !result.exact_discrete_variational_action_derived);
  check("production and Born remain untouched",
        !result.production_coupling_supplied && !result.born_target_used);
  check("no new selected type is added", !result.new_selected_type_added);

  auto nonfinite = parameters;
  nonfinite.gamma = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite coefficient fails closed",
        analyze_common_relative_connection_gearbox(state, nonfinite).status
            == CommonRelativeConnectionStatus::NonFiniteInput);
  auto bad_common_mass = parameters;
  bad_common_mass.common_mass = 0.0;
  check("nonpositive common mass fails closed",
        analyze_common_relative_connection_gearbox(state, bad_common_mass).status
            == CommonRelativeConnectionStatus::InvalidCommonMass);
  auto bad_relative_mass = parameters;
  bad_relative_mass.relative_mass = 0.0;
  check("nonpositive relative mass fails closed",
        analyze_common_relative_connection_gearbox(state,
            bad_relative_mass).status
            == CommonRelativeConnectionStatus::InvalidRelativeMass);
  auto bad_coupling = parameters;
  bad_coupling.quartic_coupling = 0.0;
  check("nonpositive quartic coupling fails closed",
        analyze_common_relative_connection_gearbox(state, bad_coupling).status
            == CommonRelativeConnectionStatus::InvalidQuarticCoupling);
  auto bad_step = parameters;
  bad_step.step = 0.0;
  check("zero step fails closed",
        analyze_common_relative_connection_gearbox(state, bad_step).status
            == CommonRelativeConnectionStatus::InvalidStep);
  auto bad_tolerance = parameters;
  bad_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_common_relative_connection_gearbox(state, bad_tolerance).status
            == CommonRelativeConnectionStatus::InvalidTolerance);
  auto bad_scale = parameters;
  bad_scale.momentum_scale = 0.0;
  check("nonpositive momentum scale fails closed",
        analyze_common_relative_connection_gearbox(state, bad_scale).status
            == CommonRelativeConnectionStatus::InvalidMomentumScale);
  auto bad_iterations = parameters;
  bad_iterations.max_iterations = 0;
  check("zero iteration limit fails closed",
        analyze_common_relative_connection_gearbox(state,
            bad_iterations).status
            == CommonRelativeConnectionStatus::InvalidIterationLimit);

  auto huge_chart_state = state;
  huge_chart_state.canonical_common_momentum = {
      std::numeric_limits<double>::max() / 8.0, 0.0, 0.0};
  auto tiny_scale = parameters;
  tiny_scale.momentum_scale = std::numeric_limits<double>::min();
  check("unrepresentable endpoint or chart fails closed",
        !analyze_common_relative_connection_gearbox(
            huge_chart_state, tiny_scale).valid());

  std::cout << "FTD-0899/0901 common-relative connection gearbox: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

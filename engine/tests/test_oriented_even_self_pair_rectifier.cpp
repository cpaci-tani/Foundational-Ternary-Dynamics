#include "ftd/eft/oriented_even_self_pair_rectifier.h"

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

bool close_vector(const ftd::eft::OrientedRectifierVector& left,
                  const ftd::eft::OrientedRectifierVector& right,
                  double tolerance = 1e-10) {
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (!close(left[axis], right[axis], tolerance)) return false;
  }
  return true;
}

ftd::eft::OrientedRectifierVector cubic_transform(
    const ftd::eft::OrientedRectifierVector& value) {
  return {-value[2], value[0], -value[1]};
}

}  // namespace

int main() {
  using namespace ftd::eft;

  OrientedEvenRectifierState state;
  state.common_coordinate = {0.2, -0.1, 0.3};
  state.relative_coordinate = 0.7;
  state.relative_momentum = -0.12;

  OrientedEvenRectifierParameters parameters;
  parameters.common_mass = 1.8;
  parameters.relative_mass = 1.7;
  parameters.bare_quartic_coupling = 0.8;
  parameters.gamma = 0.4;
  parameters.chirality = 1;
  parameters.polar_axis = {0.0, 1.0, 0.0};
  parameters.step = 0.025;
  parameters.momentum_scale = 0.01;
  parameters.tolerance = 2e-12;
  parameters.max_iterations = 128;
  parameters.moving_common_momentum_projection_probe = 0.6;

  const auto result = analyze_oriented_even_self_pair_rectifier(
      state, parameters);
  check("oriented even-self-pair rectifier is valid", result.valid());
  check("retained axis and chirality are explicit reference data",
        result.retained_polar_axis_required
            && result.retained_chirality_required_for_time_reversal
            && !result.polar_axis_substrate_derived
            && !result.chirality_substrate_derived);
  check("even polar rectification from D alone is forbidden",
        result.even_polar_rectifier_from_d_alone_forbidden);
  check("connection is even under clock-sheet exchange",
        result.connection_even_under_clock_sheet_exchange);
  const double expected_connection = parameters.gamma
      * state.relative_coordinate * state.relative_coordinate;
  check("connection is oriented along the retained polar axis",
        close_vector(result.connection_before,
                     {0.0, expected_connection, 0.0}));
  const double connection_coupling = parameters.gamma * parameters.gamma
      / (2.0 * parameters.common_mass);
  check("positive connection energy folds into the rest quartic",
        result.rest_sector_quartic_fold_exact
            && result.rest_sector_critical_quartic_exact
            && close(result.connection_quartic_contribution,
                     connection_coupling)
            && close(result.effective_quartic_coupling,
                     parameters.bare_quartic_coupling
                         + connection_coupling));
  check("mechanical impulse is minus Delta connection",
        result.mechanical_common_impulse_exact
            && result.mechanical_impulse_residual_norm <= 1e-12);
  check("directed common endpoint update is exact",
        result.common_endpoint_update_exact
            && result.directed_common_displacement_exact
            && result.common_displacement[1] < 0.0
            && result.common_endpoint_residual_norm <= 1e-12);
  check("effective quartic energy and channel impulse close",
        result.relative_energy_exact
            && result.channel_impulses_equal_and_opposite
            && std::abs(result.rest_energy_residual) <= 1e-10);
  check("reciprocal carry and signed-step inverse close",
        result.reciprocal_carry_composition_exact
            && result.signed_step_reversal_exact
            && result.reverse_common_residual_norm <= 1e-11);
  check("time reversal pairs opposite chirality branches",
        result.branch_paired_time_reversal_exact
            && !result.naive_fixed_chirality_time_reversal_exact);
  check("generic moving sector retains a quadratic ray term",
        !result.moving_sector_exact_quartic_generic
            && close(result.moving_quadratic_ray_coefficient,
                     -parameters.gamma
                         * parameters.moving_common_momentum_projection_probe
                         / parameters.common_mass));

  const double pi = std::acos(-1.0);
  const double gstar = std::tgamma(0.25) / std::tgamma(0.75);
  const double clock_scale = std::sqrt(parameters.relative_mass
      / (2.0 * result.effective_quartic_coupling));
  const double energy = result.relative_carry_step.relative_step
      .pair_before.hamiltonian_energy;
  const double amplitude = std::pow(
      energy / result.effective_quartic_coupling, 0.25);
  check("turning amplitude is reconstructed from the rest energy",
        close(result.clock_turning_amplitude, amplitude, 1e-14));
  check("continuum period-amplitude product carries Gstar",
        result.continuum_gstar_period_factor_exact
            && close(result.continuum_period_amplitude_product,
                     std::sqrt(pi) * gstar * clock_scale, 1e-14));
  check("cycle displacement carries inverse Gstar",
        result.continuum_inverse_gstar_displacement_exact
            && close(result.continuum_cycle_displacement[1],
                     -4.0 * std::sqrt(pi) * parameters.gamma
                         * amplitude * clock_scale
                         / (parameters.common_mass * gstar), 1e-14));
  check("mean gear ratio carries inverse Gstar squared",
        result.continuum_inverse_gstar_squared_mean_ratio_exact
            && close(result.continuum_mean_gear_ratio[1],
                     -4.0 * parameters.gamma
                         / (parameters.common_mass * gstar * gstar), 1e-14)
            && close(result.continuum_mean_velocity[1],
                     result.continuum_mean_gear_ratio[1]
                         * amplitude * amplitude, 1e-14));

  auto opposite = parameters;
  opposite.chirality = -1;
  const auto reverse_branch = analyze_oriented_even_self_pair_rectifier(
      state, opposite);
  check("chirality reversal reverses discrete and continuum transport",
        reverse_branch.valid()
            && close_vector(reverse_branch.common_displacement,
                            {-result.common_displacement[0],
                             -result.common_displacement[1],
                             -result.common_displacement[2]})
            && close_vector(reverse_branch.continuum_cycle_displacement,
                            {-result.continuum_cycle_displacement[0],
                             -result.continuum_cycle_displacement[1],
                             -result.continuum_cycle_displacement[2]}));

  auto transformed_parameters = parameters;
  transformed_parameters.polar_axis = cubic_transform(parameters.polar_axis);
  auto transformed_state = state;
  transformed_state.common_coordinate = cubic_transform(
      state.common_coordinate);
  const auto transformed = analyze_oriented_even_self_pair_rectifier(
      transformed_state, transformed_parameters);
  check("signed cubic transform preserves the oriented reference law",
        transformed.valid() && transformed.signed_cubic_covariant_given_axis
            && close_vector(transformed.common_displacement,
                            cubic_transform(result.common_displacement))
            && close_vector(transformed.continuum_cycle_displacement,
                            cubic_transform(
                                result.continuum_cycle_displacement)));

  check("gamma scale mass production Born and cadence remain open",
        !result.gamma_derived_from_chi_or_i
            && !result.physical_momentum_scale_derived
            && !result.absolute_mass_derived
            && !result.production_coupling_supplied
            && !result.born_target_used
            && !result.integer_tick_gstar_cadence_derived);
  check("no new selected type is added", !result.new_selected_type_added);

  auto nonfinite = parameters;
  nonfinite.gamma = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite input fails closed",
        analyze_oriented_even_self_pair_rectifier(state, nonfinite).status
            == OrientedEvenRectifierStatus::NonFiniteInput);
  auto bad_common_mass = parameters;
  bad_common_mass.common_mass = 0.0;
  check("nonpositive common mass fails closed",
        analyze_oriented_even_self_pair_rectifier(
            state, bad_common_mass).status
            == OrientedEvenRectifierStatus::InvalidCommonMass);
  auto bad_relative_mass = parameters;
  bad_relative_mass.relative_mass = 0.0;
  check("nonpositive relative mass fails closed",
        analyze_oriented_even_self_pair_rectifier(
            state, bad_relative_mass).status
            == OrientedEvenRectifierStatus::InvalidRelativeMass);
  auto bad_coupling = parameters;
  bad_coupling.bare_quartic_coupling = 0.0;
  check("nonpositive quartic coupling fails closed",
        analyze_oriented_even_self_pair_rectifier(
            state, bad_coupling).status
            == OrientedEvenRectifierStatus::InvalidQuarticCoupling);
  auto bad_step = parameters;
  bad_step.step = 0.0;
  check("zero step fails closed",
        analyze_oriented_even_self_pair_rectifier(state, bad_step).status
            == OrientedEvenRectifierStatus::InvalidStep);
  auto bad_tolerance = parameters;
  bad_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_oriented_even_self_pair_rectifier(
            state, bad_tolerance).status
            == OrientedEvenRectifierStatus::InvalidTolerance);
  auto bad_scale = parameters;
  bad_scale.momentum_scale = 0.0;
  check("nonpositive momentum scale fails closed",
        analyze_oriented_even_self_pair_rectifier(state, bad_scale).status
            == OrientedEvenRectifierStatus::InvalidMomentumScale);
  auto bad_iterations = parameters;
  bad_iterations.max_iterations = 0;
  check("zero iteration limit fails closed",
        analyze_oriented_even_self_pair_rectifier(
            state, bad_iterations).status
            == OrientedEvenRectifierStatus::InvalidIterationLimit);
  auto bad_chirality = parameters;
  bad_chirality.chirality = 0;
  check("zero chirality fails closed",
        analyze_oriented_even_self_pair_rectifier(
            state, bad_chirality).status
            == OrientedEvenRectifierStatus::InvalidChirality);
  auto bad_axis = parameters;
  bad_axis.polar_axis = {2.0, 0.0, 0.0};
  check("nonunit polar axis fails closed",
        analyze_oriented_even_self_pair_rectifier(state, bad_axis).status
            == OrientedEvenRectifierStatus::InvalidPolarAxis);
  auto overflow = parameters;
  overflow.gamma = std::numeric_limits<double>::max();
  check("effective coupling overflow fails closed",
        analyze_oriented_even_self_pair_rectifier(state, overflow).status
            == OrientedEvenRectifierStatus::EffectiveCouplingOverflow);

  std::cout << "FTD-0904 oriented even-self-pair rectifier: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

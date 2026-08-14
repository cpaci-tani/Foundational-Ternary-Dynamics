#include "ftd/eft/native_ternary_dipole_phase_wedge_memory.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

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

bool close_vector(
    const ftd::eft::NativeOrientationVector& left,
    const ftd::eft::NativeOrientationVector& right,
    double tolerance = 1e-10) {
  for (std::size_t axis = 0; axis < 3; ++axis) {
    if (!close(left[axis], right[axis], tolerance)) return false;
  }
  return true;
}

ftd::eft::NativeOrientationVector add(
    const ftd::eft::NativeOrientationVector& value,
    const ftd::eft::NativeOrientationVector& shift) {
  return {value[0] + shift[0], value[1] + shift[1], value[2] + shift[2]};
}

ftd::eft::NativeOrientationVector negate(
    const ftd::eft::NativeOrientationVector& value) {
  return {-value[0], -value[1], -value[2]};
}

ftd::eft::NativeOrientationVector cubic_transform(
    const ftd::eft::NativeOrientationVector& value) {
  return {-value[1], value[2], -value[0]};
}

std::vector<ftd::eft::NativeOrientationMemorySite> reference_sites() {
  using ftd::eft::NativeOrientationMemorySite;
  return {
      NativeOrientationMemorySite{
          {1.0, 0.0, 0.0}, 1,
          {1.0, 0.2, -0.1}, {0.0, 0.3, 0.1}},
      NativeOrientationMemorySite{
          {-1.0, 0.0, 0.0}, -1,
          {0.0, -0.2, 0.4}, {1.0, 0.1, -0.3}},
  };
}

}  // namespace

int main() {
  using namespace ftd::eft;

  NativeOrientationMemoryParameters parameters;
  parameters.memory_mass = 1.7;
  parameters.memory_quartic_coupling = 0.8;
  parameters.tolerance = 2e-11;
  parameters.swept_area_probe_state = {0.7, -0.12};
  parameters.swept_area_probe_parameters.mass = 1.3;
  parameters.swept_area_probe_parameters.coupling = 0.9;
  parameters.swept_area_probe_parameters.step = 0.02;
  parameters.swept_area_probe_parameters.residual_tolerance = 1e-13;
  parameters.swept_area_probe_parameters.max_iterations = 128;

  const auto sites = reference_sites();
  const auto result = analyze_native_ternary_dipole_phase_wedge_memory(
      sites, parameters);
  check("native ternary dipole phase-wedge analyzer is valid", result.valid());
  check("neutral plus-minus pair supplies the polar axis",
        result.neutral_dipole_axis_conditional_exact
            && result.minimum_nonzero_neutral_body_is_plus_minus_pair
            && result.total_ternary_state == 0
            && close_vector(result.ternary_dipole, {2.0, 0.0, 0.0})
            && close_vector(result.polar_axis, {1.0, 0.0, 0.0}));
  check("dipole is origin independent",
        result.origin_independence_exact
            && close_vector(result.origin_independence_residual,
                            {0.0, 0.0, 0.0}));
  check("native fields project to two scalar canonical modes",
        result.projected_modes_spatial_scalars
            && close(result.positive_coordinate, 1.0)
            && close(result.negative_coordinate, 0.0)
            && close(result.positive_momentum, 0.0)
            && close(result.negative_momentum, 1.0));
  check("bilateral wedge supplies positive time-odd chirality",
        result.phase_wedge_spatial_scalar && result.phase_wedge_time_odd
            && close(result.phase_wedge, 1.0)
            && close(result.time_reversed_phase_wedge, -1.0)
            && result.chirality == 1);
  check("Gram data retains only wedge magnitude",
        result.symmetric_gram_loses_wedge_sign
            && close(result.gram_determinant, 1.0)
            && close(result.gram_wedge_square_residual, 0.0));
  check("central quartic memory conserves the wedge",
        result.central_quartic_memory_imposed
            && result.central_memory_conserves_phase_wedge
            && close(result.phase_wedge_derivative_residual, 0.0));
  const double expected_radius = std::pow(
      1.0 / (4.0 * parameters.memory_mass
          * parameters.memory_quartic_coupling),
      1.0 / 6.0);
  check("nonzero wedge has one strict radial memory minimum",
        result.nonzero_wedge_bounded_recursive_memory
            && close(result.radial_minimum, expected_radius)
            && result.radial_minimum_curvature > 0.0
            && result.centrifugal_term_at_current_radius > 0.0);
  check("one-step swept area is not the time-odd branch memory",
        result.swept_area_probe.valid
            && result.swept_area_probe.orientation_sign == -1
            && !result.one_step_swept_area_time_odd_memory
            && close(result.swept_area_full_time_reversal_residual, 0.0));
  check("clock and nonzero-wedge memory are separate modes",
        result.separate_clock_and_chirality_memory_minimum
            && !result.same_mode_nonzero_wedge_retains_pure_gstar_clock);
  check("symmetric square loses the polar and wedge signs",
        result.dipole_symmetric_square_loses_sign
            && result.symmetric_gram_loses_wedge_sign);

  auto translated_sites = sites;
  const NativeOrientationVector shift{3.0, -2.0, 0.5};
  for (auto& site : translated_sites) site.position = add(site.position, shift);
  const auto translated = analyze_native_ternary_dipole_phase_wedge_memory(
      translated_sites, parameters);
  check("global translation leaves the neutral axis and chirality unchanged",
        translated.valid()
            && close_vector(translated.polar_axis, result.polar_axis)
            && translated.chirality == result.chirality);

  auto inverted_sites = sites;
  for (auto& site : inverted_sites) {
    site.position = negate(site.position);
    site.flux = negate(site.flux);
    site.wave_velocity = negate(site.wave_velocity);
  }
  const auto inverted = analyze_native_ternary_dipole_phase_wedge_memory(
      inverted_sites, parameters);
  check("spatial inversion reverses the axis but preserves scalar chirality",
        inverted.valid() && inverted.inversion_reverses_axis_exact
            && close_vector(inverted.polar_axis, {-1.0, 0.0, 0.0})
            && close(inverted.phase_wedge, result.phase_wedge)
            && inverted.chirality == result.chirality);

  auto cubic_sites = sites;
  for (auto& site : cubic_sites) {
    site.position = cubic_transform(site.position);
    site.flux = cubic_transform(site.flux);
    site.wave_velocity = cubic_transform(site.wave_velocity);
  }
  const auto cubic = analyze_native_ternary_dipole_phase_wedge_memory(
      cubic_sites, parameters);
  check("signed cubic transformation preserves the phase-wedge scalar",
        cubic.valid() && cubic.signed_cubic_covariance_exact
            && close_vector(cubic.polar_axis,
                            cubic_transform(result.polar_axis))
            && close(cubic.phase_wedge, result.phase_wedge));

  auto time_reversed_sites = sites;
  for (auto& site : time_reversed_sites) {
    site.wave_velocity = negate(site.wave_velocity);
  }
  const auto time_reversed =
      analyze_native_ternary_dipole_phase_wedge_memory(
          time_reversed_sites, parameters);
  check("time reversal flips the bilateral chirality",
        time_reversed.valid()
            && close(time_reversed.phase_wedge, -result.phase_wedge)
            && time_reversed.chirality == -result.chirality);

  check("formation maintenance production scale mass Born and cadence stay open",
        !result.nonzero_dipole_formation_derived
            && !result.nonzero_phase_wedge_formation_derived
            && !result.production_bilateral_memory_law_present
            && !result.maintenance_erasure_work_closed
            && !result.gamma_magnitude_derived
            && !result.physical_momentum_scale_derived
            && !result.absolute_mass_derived
            && !result.integer_tick_gstar_cadence_derived
            && !result.production_integration_supplied
            && !result.born_target_used);
  check("no new selected type is added", !result.new_selected_type_added);

  check("empty region fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory({}, parameters).status
            == NativeOrientationMemoryStatus::EmptyRegion);
  auto nonfinite = sites;
  nonfinite[0].flux[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite site data fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            nonfinite, parameters).status
            == NativeOrientationMemoryStatus::NonFiniteInput);
  auto nonternary = sites;
  nonternary[0].state = 2;
  check("nonternary state fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            nonternary, parameters).status
            == NativeOrientationMemoryStatus::NonTernaryState);
  auto nonneutral = sites;
  nonneutral[1].state = 0;
  check("nonneutral region fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            nonneutral, parameters).status
            == NativeOrientationMemoryStatus::NonNeutralRegion);
  std::vector<NativeOrientationMemorySite> zeros(2);
  check("missing plus-minus endpoints fail closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            zeros, parameters).status
            == NativeOrientationMemoryStatus::MissingPositiveEndpoint);
  auto duplicate = sites;
  duplicate.push_back(sites[0]);
  duplicate.push_back(sites[1]);
  check("nonunique endpoints fail closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            duplicate, parameters).status
            == NativeOrientationMemoryStatus::NonUniquePositiveEndpoint);
  auto coincident = sites;
  coincident[1].position = coincident[0].position;
  check("coincident endpoints fail closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            coincident, parameters).status
            == NativeOrientationMemoryStatus::CoincidentEndpoints);
  auto zero_wedge = sites;
  zero_wedge[1].wave_velocity = {0.0, 0.0, 0.0};
  check("zero phase wedge fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            zero_wedge, parameters).status
            == NativeOrientationMemoryStatus::ZeroPhaseWedge);
  auto bad_mass = parameters;
  bad_mass.memory_mass = 0.0;
  check("nonpositive memory mass fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            sites, bad_mass).status
            == NativeOrientationMemoryStatus::InvalidMemoryMass);
  auto bad_coupling = parameters;
  bad_coupling.memory_quartic_coupling = 0.0;
  check("nonpositive memory coupling fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            sites, bad_coupling).status
            == NativeOrientationMemoryStatus::InvalidMemoryCoupling);
  auto bad_tolerance = parameters;
  bad_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            sites, bad_tolerance).status
            == NativeOrientationMemoryStatus::InvalidTolerance);
  auto bad_probe = parameters;
  bad_probe.swept_area_probe_parameters.step = 0.0;
  check("invalid swept-area probe fails closed",
        analyze_native_ternary_dipole_phase_wedge_memory(
            sites, bad_probe).status
            == NativeOrientationMemoryStatus::PairProbeFailure);

  std::cout << "FTD-0905/0907 native ternary dipole phase-wedge memory: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

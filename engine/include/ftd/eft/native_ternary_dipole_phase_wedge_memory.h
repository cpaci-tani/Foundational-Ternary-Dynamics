#pragma once
/**
 * @file native_ternary_dipole_phase_wedge_memory.h
 * @brief FTD-0905/0907 isolated native-type orientation-memory analyzer.
 *
 * A neutral actual-layer ternary dipole supplies a conditional polar axis.
 * Projected flux/wave-velocity pairs at its +/- endpoints supply the
 * antisymmetric phase wedge ell=q_+ p_- - q_- p_+, whose sign is a spatial
 * scalar and time odd. The central quartic memory law is imposed; production
 * formation, maintenance, erasure, coupling, and finite-tick cadence remain
 * open.
 */

#include "ftd/eft/native_pair_energy_recursion.h"

#include <array>
#include <cstdint>
#include <vector>

namespace ftd::eft {

using NativeOrientationVector = std::array<double, 3>;

enum class NativeOrientationMemoryStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidMemoryMass,
  InvalidMemoryCoupling,
  InvalidTolerance,
  EmptyRegion,
  NonTernaryState,
  NonNeutralRegion,
  MissingPositiveEndpoint,
  MissingNegativeEndpoint,
  NonUniquePositiveEndpoint,
  NonUniqueNegativeEndpoint,
  CoincidentEndpoints,
  ZeroDipole,
  ZeroPhaseWedge,
  PairProbeFailure,
  InvariantFailure,
};

struct NativeOrientationMemorySite {
  NativeOrientationVector position{};
  int state = 0;
  NativeOrientationVector flux{};
  NativeOrientationVector wave_velocity{};
};

struct NativeOrientationMemoryParameters {
  double memory_mass = 1.0;
  double memory_quartic_coupling = 1.0;
  double tolerance = 1e-11;
  NativePairEnergyState swept_area_probe_state{0.7, -0.12};
  NativePairEnergyParameters swept_area_probe_parameters{};
};

struct NativeOrientationMemoryResult {
  NativeOrientationMemoryStatus status =
      NativeOrientationMemoryStatus::NonFiniteInput;

  NativeOrientationVector ternary_dipole{};
  NativeOrientationVector shifted_origin_dipole{};
  NativeOrientationVector origin_independence_residual{};
  NativeOrientationVector polar_axis{};
  NativePairEnergyStep swept_area_probe{};

  int total_ternary_state = 0;
  int positive_endpoint_index = -1;
  int negative_endpoint_index = -1;
  int chirality = 0;
  double dipole_norm = 0.0;
  double axis_norm_residual = 0.0;
  double positive_coordinate = 0.0;
  double negative_coordinate = 0.0;
  double positive_momentum = 0.0;
  double negative_momentum = 0.0;
  double phase_wedge = 0.0;
  double time_reversed_phase_wedge = 0.0;
  double time_reversal_residual = 0.0;
  double gram_determinant = 0.0;
  double gram_wedge_square_residual = 0.0;
  double memory_radius_squared = 0.0;
  double memory_energy = 0.0;
  double phase_wedge_derivative_residual = 0.0;
  double radial_minimum = 0.0;
  double radial_minimum_equation_residual = 0.0;
  double radial_minimum_curvature = 0.0;
  double centrifugal_term_at_current_radius = 0.0;
  double swept_area_full_time_reversal_residual = 0.0;

  bool neutral_dipole_axis_conditional_exact = false;
  bool origin_independence_exact = false;
  bool minimum_nonzero_neutral_body_is_plus_minus_pair = false;
  bool signed_cubic_covariance_exact = false;
  bool inversion_reverses_axis_exact = false;
  bool dipole_symmetric_square_loses_sign = false;
  bool projected_modes_spatial_scalars = false;
  bool phase_wedge_spatial_scalar = false;
  bool phase_wedge_time_odd = false;
  bool symmetric_gram_loses_wedge_sign = false;
  bool one_step_swept_area_time_odd_memory = false;
  bool central_quartic_memory_imposed = false;
  bool central_memory_conserves_phase_wedge = false;
  bool nonzero_wedge_bounded_recursive_memory = false;
  bool same_mode_nonzero_wedge_retains_pure_gstar_clock = false;
  bool separate_clock_and_chirality_memory_minimum = false;

  bool nonzero_dipole_formation_derived = false;
  bool nonzero_phase_wedge_formation_derived = false;
  bool production_bilateral_memory_law_present = false;
  bool maintenance_erasure_work_closed = false;
  bool gamma_magnitude_derived = false;
  bool physical_momentum_scale_derived = false;
  bool absolute_mass_derived = false;
  bool integer_tick_gstar_cadence_derived = false;
  bool production_integration_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const { return status == NativeOrientationMemoryStatus::Valid; }
};

NativeOrientationMemoryResult analyze_native_ternary_dipole_phase_wedge_memory(
    const std::vector<NativeOrientationMemorySite>& sites,
    const NativeOrientationMemoryParameters& parameters = {});

}  // namespace ftd::eft

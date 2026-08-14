#pragma once
/**
 * @file oriented_even_self_pair_rectifier.h
 * @brief FTD-0904 isolated oriented even-self-pair rectifier witness.
 *
 * A retained polar axis e and time-odd chirality chi orient the imposed
 * connection A=chi*gamma*q^2*e. In the canonical-common rest sector, its
 * positive energy remains exactly quartic while the common coordinate gains
 * directed displacement. This does not derive e, chi, gamma, a physical
 * momentum unit, mass, production coupling, or finite-tick G* cadence.
 */

#include "ftd/eft/quartic_relative_carry_gearbox.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace ftd::eft {

using OrientedRectifierVector = std::array<double, 3>;

enum class OrientedEvenRectifierStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidCommonMass,
  InvalidRelativeMass,
  InvalidQuarticCoupling,
  InvalidStep,
  InvalidTolerance,
  InvalidMomentumScale,
  InvalidIterationLimit,
  InvalidChirality,
  InvalidPolarAxis,
  EffectiveCouplingOverflow,
  RelativeCarryFailure,
  InvariantFailure,
  ReverseFailure,
};

struct OrientedEvenRectifierParameters {
  double common_mass = 1.0;
  double relative_mass = 1.0;
  double bare_quartic_coupling = 1.0;
  /// Imposed magnitude; not derived from chi or i.
  double gamma = 0.0;
  /// Retained clockwise/counterclockwise branch; must be exactly -1 or +1.
  int chirality = 1;
  /// Retained local polar axis; must be unit within tolerance.
  OrientedRectifierVector polar_axis{1.0, 0.0, 0.0};
  double step = 0.01;
  double momentum_scale = 1.0;
  double tolerance = 1e-11;
  std::size_t max_iterations = 96;
  /// Audit probe for P dot e in the generic moving-sector q^2 coefficient.
  double moving_common_momentum_projection_probe = 1.0;
};

struct OrientedEvenRectifierState {
  OrientedRectifierVector common_coordinate{};
  double relative_coordinate = 0.0;
  double relative_momentum = 0.0;
};

struct OrientedEvenRectifierResult {
  OrientedEvenRectifierStatus status =
      OrientedEvenRectifierStatus::NonFiniteInput;

  OrientedEvenRectifierState before{};
  OrientedEvenRectifierState after{};
  QuarticRelativeCarryResult relative_carry_step{};

  OrientedRectifierVector connection_before{};
  OrientedRectifierVector connection_after{};
  OrientedRectifierVector mechanical_common_momentum_before{};
  OrientedRectifierVector mechanical_common_momentum_after{};
  OrientedRectifierVector mechanical_impulse_residual{};
  OrientedRectifierVector common_displacement{};
  OrientedRectifierVector common_endpoint_equation_residual{};
  OrientedRectifierVector reverse_common_coordinate_residual{};
  OrientedRectifierVector continuum_cycle_displacement{};
  OrientedRectifierVector continuum_mean_velocity{};
  OrientedRectifierVector continuum_mean_gear_ratio{};

  double polar_axis_norm = 0.0;
  double polar_axis_norm_residual = 0.0;
  double effective_quartic_coupling = 0.0;
  double connection_quartic_contribution = 0.0;
  double rest_energy_residual = 0.0;
  double mechanical_impulse_residual_norm = 0.0;
  double common_endpoint_residual_norm = 0.0;
  double reverse_common_residual_norm = 0.0;
  double moving_quadratic_ray_coefficient = 0.0;
  double clock_turning_amplitude = 0.0;
  double continuum_period_amplitude_product = 0.0;

  bool imposed_oriented_even_connection = false;
  bool even_polar_rectifier_from_d_alone_forbidden = false;
  bool retained_polar_axis_required = false;
  bool retained_chirality_required_for_time_reversal = false;
  bool signed_cubic_covariant_given_axis = false;
  bool connection_even_under_clock_sheet_exchange = false;
  bool rest_sector_quartic_fold_exact = false;
  bool rest_sector_critical_quartic_exact = false;
  bool mechanical_common_impulse_exact = false;
  bool common_endpoint_update_exact = false;
  bool directed_common_displacement_exact = false;
  bool relative_energy_exact = false;
  bool channel_impulses_equal_and_opposite = false;
  bool reciprocal_carry_composition_exact = false;
  bool signed_step_reversal_exact = false;
  bool branch_paired_time_reversal_exact = false;
  bool naive_fixed_chirality_time_reversal_exact = false;
  bool continuum_gstar_period_factor_exact = false;
  bool continuum_inverse_gstar_displacement_exact = false;
  bool continuum_inverse_gstar_squared_mean_ratio_exact = false;
  bool moving_sector_exact_quartic_generic = false;

  bool polar_axis_substrate_derived = false;
  bool chirality_substrate_derived = false;
  bool gamma_derived_from_chi_or_i = false;
  bool physical_momentum_scale_derived = false;
  bool absolute_mass_derived = false;
  bool integer_tick_gstar_cadence_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const { return status == OrientedEvenRectifierStatus::Valid; }
};

OrientedEvenRectifierResult analyze_oriented_even_self_pair_rectifier(
    const OrientedEvenRectifierState& state,
    const OrientedEvenRectifierParameters& parameters = {});

}  // namespace ftd::eft

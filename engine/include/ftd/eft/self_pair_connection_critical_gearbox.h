#pragma once
/**
 * @file self_pair_connection_critical_gearbox.h
 * @brief FTD-0902/0903 isolated signed-self-pair connection witness.
 *
 * In the canonical-common rest sector P=0, the imposed connection
 * A(D)=gamma*D*|D| folds its positive connection energy exactly into the
 * quartic coupling.  This linearly polarized analyzer composes the existing
 * native quartic recursion and reciprocal-carry witness.  It does not derive
 * gamma, a physical momentum scale, mass, production coupling, or a finite
 * tick G* cadence.
 */

#include "ftd/eft/quartic_relative_carry_gearbox.h"

#include <cstddef>
#include <cstdint>

namespace ftd::eft {

enum class SelfPairConnectionStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidCommonMass,
  InvalidRelativeMass,
  InvalidQuarticCoupling,
  InvalidStep,
  InvalidTolerance,
  InvalidMomentumScale,
  InvalidIterationLimit,
  EffectiveCouplingOverflow,
  SignedPairOverflow,
  RelativeCarryFailure,
  InvariantFailure,
  ReverseFailure,
};

struct SelfPairConnectionParameters {
  double common_mass = 1.0;
  double relative_mass = 1.0;
  double bare_quartic_coupling = 1.0;
  /// Imposed real coefficient. Its magnitude is not derived from i.
  double gamma = 0.0;
  /// Signed nonzero step; negating it applies the endpoint inverse.
  double step = 0.01;
  /// Imposed chart conversion P/p_star=k+2*pi*w.
  double momentum_scale = 1.0;
  double tolerance = 1e-11;
  std::size_t max_iterations = 96;
  /// Audit probe for the coefficient of r^2 in a nonzero-P ray sector.
  double moving_common_momentum_projection_probe = 1.0;
};

struct SelfPairConnectionState {
  double common_coordinate = 0.0;
  double relative_coordinate = 0.0;
  double relative_momentum = 0.0;
};

struct SelfPairConnectionResult {
  SelfPairConnectionStatus status =
      SelfPairConnectionStatus::NonFiniteInput;

  SelfPairConnectionState before{};
  SelfPairConnectionState after{};
  QuarticRelativeCarryResult relative_carry_step{};

  double effective_quartic_coupling = 0.0;
  double connection_quartic_contribution = 0.0;
  double signed_pair_before = 0.0;
  double signed_pair_after = 0.0;
  double connection_derivative_before = 0.0;
  double connection_derivative_after = 0.0;
  double mechanical_common_momentum_before = 0.0;
  double mechanical_common_momentum_after = 0.0;
  double mechanical_impulse_residual = 0.0;
  double common_displacement = 0.0;
  double common_endpoint_equation_residual = 0.0;
  double rest_energy_residual = 0.0;
  double reverse_common_coordinate_residual = 0.0;
  double moving_quadratic_ray_coefficient = 0.0;
  double self_pair_origin_jacobian = 0.0;
  double critical_clock_hessian = 0.0;
  double symmetric_full_cycle_drift_residual = 0.0;
  double continuum_period_amplitude_product = 0.0;
  double conditional_equal_partition_gamma_magnitude = 0.0;

  bool imposed_signed_self_pair_connection = false;
  bool positive_linearized_connection_obstruction_registered = false;
  bool origin_connection_derivative_zero = false;
  bool connection_derivative_nonzero_away_for_nonzero_gamma = false;
  bool rest_sector_quartic_fold_exact = false;
  bool rest_sector_critical_quartic_exact = false;
  bool mechanical_common_impulse_exact = false;
  bool common_endpoint_update_exact = false;
  bool relative_energy_exact = false;
  bool channel_impulses_equal_and_opposite = false;
  bool reciprocal_carry_composition_exact = false;
  bool signed_step_reversal_exact = false;
  bool continuum_gstar_period_factor_exact = false;
  bool moving_sector_has_generic_quadratic_term = false;
  bool moving_sector_exact_quartic_generic = false;
  bool polarized_symmetric_full_cycle_drift_zero = false;
  bool i_supplies_orientation = false;

  bool gamma_derived_from_i = false;
  bool equal_self_dual_partition_adopted = false;
  bool physical_momentum_scale_derived = false;
  bool absolute_mass_derived = false;
  bool integer_tick_gstar_cadence_derived = false;
  bool net_transport_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const { return status == SelfPairConnectionStatus::Valid; }
};

SelfPairConnectionResult analyze_self_pair_connection_critical_gearbox(
    const SelfPairConnectionState& state,
    const SelfPairConnectionParameters& parameters = {});

}  // namespace ftd::eft

#pragma once
/**
 * @file common_relative_connection_gearbox.h
 * @brief FTD-0899/0901 isolated common/relative connection witness.
 *
 * The imposed velocity-linear connection transfers mechanical common
 * momentum to the selected relative quartic sector while conserving the
 * registered canonical Hamiltonian and Noether momentum.  This analyzer is
 * reference mechanics only: gamma, the physical momentum unit, matter/field
 * identification, and finite-tick G* cadence remain open.
 */

#include "ftd/eft/reciprocal_carry_reservoir.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace ftd::eft {

using ConnectionVector = std::array<double, 3>;
using ConnectionWinding = std::array<std::int64_t, 3>;

enum class CommonRelativeConnectionStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidCommonMass,
  InvalidRelativeMass,
  InvalidQuarticCoupling,
  InvalidStep,
  InvalidTolerance,
  InvalidMomentumScale,
  InvalidIterationLimit,
  SolverFailure,
  ChartCarryOutOfRange,
  ChartWindingOverflow,
  CarryTransactionFailure,
  InvariantFailure,
  ReverseFailure,
};

struct CommonRelativeConnectionParameters {
  double common_mass = 1.0;
  double relative_mass = 1.0;
  double quartic_coupling = 1.0;
  /// Imposed real connection coefficient; its magnitude is not derived by i.
  double gamma = 0.0;
  /// Signed nonzero step.  Negating it applies the endpoint inverse.
  double step = 0.01;
  /// Imposed conversion P/p_star=k+2*pi*w.
  double momentum_scale = 1.0;
  double tolerance = 1e-11;
  std::size_t max_iterations = 96;
};

struct CommonRelativeConnectionState {
  ConnectionVector common_coordinate{};
  ConnectionVector relative_coordinate{};
  ConnectionVector canonical_common_momentum{};
  ConnectionVector relative_momentum{};
};

struct CommonRelativeConnectionStep {
  bool valid = false;
  bool converged = false;
  std::size_t solve_iterations = 0;
  CommonRelativeConnectionState before{};
  CommonRelativeConnectionState after{};
  double common_coordinate_equation_residual = 0.0;
  double relative_coordinate_equation_residual = 0.0;
  double relative_momentum_equation_residual = 0.0;
  double energy_before = 0.0;
  double energy_after = 0.0;
  double energy_residual = 0.0;
};

struct CommonRelativeConnectionResult {
  CommonRelativeConnectionStatus status =
      CommonRelativeConnectionStatus::NonFiniteInput;

  CommonRelativeConnectionStep connection_step{};
  ReciprocalCarryResult carry_step{};

  ConnectionVector mechanical_common_before{};
  ConnectionVector mechanical_common_after{};
  ConnectionVector mechanical_impulse_residual{};
  ConnectionVector channel_left_before{};
  ConnectionVector channel_left_after{};
  ConnectionVector channel_right_before{};
  ConnectionVector channel_right_after{};
  ConnectionVector channel_impulse_sum_residual{};
  ConnectionVector principal_left_before{};
  ConnectionVector principal_left_after{};
  ConnectionVector principal_right_before{};
  ConnectionVector principal_right_after{};
  ConnectionWinding winding_left_before{};
  ConnectionWinding winding_left_after{};
  ConnectionWinding winding_right_before{};
  ConnectionWinding winding_right_after{};
  ConnectionVector generated_dimensionless_increment{};
  ConnectionVector angular_momentum_before{};
  ConnectionVector angular_momentum_after{};
  ConnectionVector angular_momentum_residual{};
  ConnectionVector clock_origin_tilt{};

  double connection_curvature = 0.0;
  double critical_clock_hessian = 0.0;
  double canonical_momentum_residual = 0.0;
  double mechanical_impulse_residual_norm = 0.0;
  double channel_impulse_sum_residual_norm = 0.0;
  double angular_momentum_residual_norm = 0.0;
  double chart_endpoint_residual = 0.0;
  double reverse_state_residual = 0.0;
  double reverse_carry_residual = 0.0;

  bool imposed_connection_action = false;
  bool connection_curvature_nonzero_for_gamma_nonzero = false;
  bool canonical_total_momentum_exact = false;
  bool mechanical_common_impulse_exact = false;
  bool channel_impulses_equal_and_opposite = false;
  bool discrete_common_energy_exact = false;
  bool reciprocal_carry_compatibility_exact = false;
  bool signed_step_reversal_exact = false;
  bool cubic_covariant_reference_law = false;
  bool canonical_angular_momentum_exact = false;
  bool i_supplies_orientation = false;
  bool conditional_channel_exchange_time_reversal = false;
  bool critical_quartic_preserved = false;
  bool continuous_nonzero_connection_preserves_critical_quartic = false;

  bool gamma_derived_from_i = false;
  bool physical_common_coordinate_identified = false;
  bool physical_momentum_scale_derived = false;
  bool absolute_mass_derived = false;
  bool integer_tick_gstar_cadence_derived = false;
  bool exact_discrete_variational_action_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const {
    return status == CommonRelativeConnectionStatus::Valid;
  }
};

double common_relative_connection_energy(
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters);

CommonRelativeConnectionResult analyze_common_relative_connection_gearbox(
    const CommonRelativeConnectionState& state,
    const CommonRelativeConnectionParameters& parameters = {});

}  // namespace ftd::eft

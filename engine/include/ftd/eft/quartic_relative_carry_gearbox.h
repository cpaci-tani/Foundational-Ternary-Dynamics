#pragma once
/**
 * @file quartic_relative_carry_gearbox.h
 * @brief FTD-0898 isolated relative-quartic impulse/carry composition.
 *
 * The selected relative quartic recursion generates an equal-and-opposite
 * channel impulse and the FTD-0897 transaction retains every reciprocal-zone
 * crossing. The common mode is deliberately decoupled. This is a reference
 * gearbox, not a production matter-field coupling or a derived momentum unit.
 */

#include "ftd/eft/native_pair_energy_recursion.h"
#include "ftd/eft/reciprocal_carry_reservoir.h"

#include <array>
#include <cstdint>

namespace ftd::eft {

enum class QuarticRelativeCarryStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidTolerance,
  InvalidMomentumScale,
  RelativeStepFailure,
  ChartCarryOutOfRange,
  ChartWindingOverflow,
  CarryTransactionFailure,
  EndpointMismatch,
  ReverseFailure,
};

struct QuarticRelativeCarryInput {
  NativePairEnergyState relative_state{};
  NativePairEnergyParameters relative_parameters{};
  /// Orthogonal common momentum P_C; P_L+P_R=sqrt(2) P_C.
  double common_momentum = 0.0;
  /// Imposed chart conversion P/p_star=k+2*pi*w.
  double momentum_scale = 1.0;
  double tolerance = 1e-11;
};

struct QuarticRelativeCarryResult {
  QuarticRelativeCarryStatus status =
      QuarticRelativeCarryStatus::NonFiniteInput;

  NativePairEnergyStep relative_step{};
  ReciprocalCarryResult carry_step{};

  std::array<double, 2> channel_momentum_before{};
  std::array<double, 2> channel_momentum_after{};
  std::array<double, 2> channel_principal_before{};
  std::array<double, 2> channel_principal_after{};
  std::array<std::int64_t, 2> channel_winding_before{};
  std::array<std::int64_t, 2> channel_winding_after{};

  double generated_dimensionless_increment = 0.0;
  double common_momentum_before = 0.0;
  double common_momentum_after = 0.0;
  double common_momentum_residual = 0.0;
  double relative_energy_residual = 0.0;
  double chart_endpoint_residual = 0.0;
  double reverse_residual = 0.0;
  double continuum_period_amplitude_product = 0.0;

  bool relative_increment_derived_inside_selected_recursion = false;
  bool channel_impulses_equal_and_opposite = false;
  bool relative_energy_exact = false;
  bool reciprocal_carry_composition_exact = false;
  bool full_state_reversal_exact = false;
  bool continuum_gstar_period_factor_exact = false;

  bool common_mode_coupling_derived = false;
  bool matter_field_identification_derived = false;
  bool physical_momentum_scale_derived = false;
  bool integer_tick_gstar_cadence_derived = false;
  bool carry_energy_law_derived = false;
  bool absolute_mass_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const { return status == QuarticRelativeCarryStatus::Valid; }
};

QuarticRelativeCarryResult analyze_quartic_relative_carry_gearbox(
    const QuarticRelativeCarryInput& input);

}  // namespace ftd::eft

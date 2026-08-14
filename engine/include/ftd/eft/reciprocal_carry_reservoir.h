#pragma once
/**
 * @file reciprocal_carry_reservoir.h
 * @brief FTD-0897 isolated reciprocal-carry transaction witness.
 *
 * A supplied equal-and-opposite dimensionless increment is wrapped back to
 * the selected Bloch branch while an integer triplet retains the discarded
 * reciprocal-lattice carry. This is exact momentum bookkeeping. It does not
 * derive the increment, identify the reservoir with substrate hardware, or
 * supply an energy law or physical momentum unit.
 */

#include <array>
#include <cstdint>

namespace ftd::eft {

using ReciprocalTriplet = std::array<double, 3>;
using ReciprocalCarryTriplet = std::array<std::int64_t, 3>;

enum class ReciprocalCarryStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidTolerance,
  InvalidMomentumScale,
  NonPrincipalLabel,
  CarryOutOfRange,
  ReservoirOverflow,
  ConservationFailure,
  ReversalFailure,
};

struct ReciprocalCarryInput {
  ReciprocalTriplet principal_first{};
  ReciprocalTriplet principal_second{};
  /// Applied as +q to the first label and -q to the second label.
  ReciprocalTriplet opposite_increment{};
  ReciprocalCarryTriplet reciprocal_reservoir{};

  /// Imposed conversion in P_candidate = momentum_scale * K_dimensionless.
  double momentum_scale = 1.0;
  double tolerance = 1e-12;
};

struct ReciprocalCarryResult {
  ReciprocalCarryStatus status = ReciprocalCarryStatus::NonFiniteInput;

  ReciprocalTriplet principal_first_after{};
  ReciprocalTriplet principal_second_after{};
  ReciprocalCarryTriplet carry_first{};
  ReciprocalCarryTriplet carry_second{};
  ReciprocalCarryTriplet reciprocal_reservoir_after{};

  ReciprocalTriplet dimensionless_total_before{};
  ReciprocalTriplet dimensionless_total_after{};
  ReciprocalTriplet physical_momentum_before{};
  ReciprocalTriplet physical_momentum_after{};

  double band_energy_before = 0.0;
  double band_energy_after = 0.0;
  double band_energy_change = 0.0;
  double conservation_residual = 0.0;
  double reversal_residual = 0.0;

  bool reciprocal_carry_update_exact = false;
  bool reservoir_increment_unique_given_branch_and_conservation = false;
  bool full_state_reversal_exact = false;
  bool multi_zone_increment_supported = false;
  bool periodic_band_energy_blind_to_reservoir = false;

  bool interaction_increment_derived = false;
  bool reservoir_substrate_identification_derived = false;
  bool reservoir_energy_law_derived = false;
  bool physical_momentum_scale_derived = false;
  bool total_field_matter_momentum_map_derived = false;
  bool absolute_mass_derived = false;
  bool production_coupling_supplied = false;
  bool born_target_used = false;
  bool native_gstar_synchronization_supplied = false;
  bool new_selected_vector_type_added = false;

  bool valid() const { return status == ReciprocalCarryStatus::Valid; }
};

ReciprocalCarryResult apply_reciprocal_carry_transaction(
    const ReciprocalCarryInput& input);

}  // namespace ftd::eft

#pragma once

/**
 * @file canonical_source_centered_gauss_gate.h
 * @brief FTD-0885/0886 canonical Gauss-layer and battery-phase boundary.
 *
 * This isolated EFT reference restores the conjugate coordinates omitted by
 * the FTD-0882 configuration gate. One normalized residual mode and one port
 * mode undergo a positive clocked Hamiltonian quarter-turn. Raw field/port
 * work is balanced by the source-field interaction term. The companion audit
 * shows why the FTD-0884 square-root battery is energy-exact only on its
 * zero-conjugate (Lagrangian) section. No production coupling is supplied.
 */

#include "ftd/eft/oriented_ternary_quarter_turn.h"

#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class CanonicalSourceCenteredGaussStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidClockFrequency,
  InvalidReferenceAction,
  InvalidReferencePhase,
  InvalidTolerance,
  InsufficientReferenceReserve,
};

struct CanonicalSourceCenteredGaussMode {
  double field_normal = 0.0;
  double port = 0.0;
  double field_conjugate = 0.0;
  double port_conjugate = 0.0;
  double source_offset = 0.0;
};

struct CanonicalSourceCenteredGaussInput {
  CanonicalSourceCenteredGaussMode mode;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
  double clock_frequency = 1.0;
  double reference_action = 1.0;
  double reference_phase = 0.0;
  double tolerance = 1e-12;
};

struct CanonicalSourceCenteredGaussResult {
  CanonicalSourceCenteredGaussStatus status =
      CanonicalSourceCenteredGaussStatus::NonFiniteInput;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
  CanonicalSourceCenteredGaussMode before;
  CanonicalSourceCenteredGaussMode after;
  CanonicalSourceCenteredGaussMode recovered;
  double residual_before = 0.0;
  double residual_after = 0.0;
  double cycle_duration = 0.0;
  double pulse_angle = 0.0;
  double carrier_norm_before = 0.0;
  double carrier_norm_after = 0.0;
  double angular_momentum = 0.0;
  double carrier_hamiltonian_lower_bound = 0.0;
  double reference_action_before = 0.0;
  double minimum_reference_action = 0.0;
  double maximum_reference_action = 0.0;
  double reference_action_after = 0.0;
  double maximum_clock_action_excursion = 0.0;
  double raw_energy_before = 0.0;
  double raw_energy_after = 0.0;
  double interaction_energy_before = 0.0;
  double interaction_energy_after = 0.0;
  double raw_source_work = 0.0;
  double interaction_work = 0.0;
  double source_work_residual = 0.0;
  double endpoint_hamiltonian_before = 0.0;
  double endpoint_hamiltonian_after = 0.0;
  double endpoint_hamiltonian_residual = 0.0;
  bool endpoint_symplectic = true;
  bool endpoint_orthogonal = true;
  bool endpoint_orientation_preserving = true;
  bool exact_inverse_verified = false;
  bool positive_source_centered_hamiltonian = false;
  bool zero_conjugate_section = false;
  bool zero_conjugate_section_preserved = false;
  bool frozen_gauss_configuration_gate_reproduced = false;
  bool raw_work_is_interaction_energy_exchange = false;
  bool source_offset_dynamical = false;
  bool autonomous_parity_controller_supplied = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const {
    return status == CanonicalSourceCenteredGaussStatus::Valid;
  }
};

CanonicalSourceCenteredGaussResult
evolve_canonical_source_centered_gauss_cycle(
    const CanonicalSourceCenteredGaussInput& input);

enum class SquareRootBatteryPhaseStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  EmptyAmplitude,
  InvalidTolerance,
  ReserveDepleted,
};

struct SquareRootBatteryPhaseAudit {
  SquareRootBatteryPhaseStatus status =
      SquareRootBatteryPhaseStatus::NonFiniteInput;
  double amplitude_before = 0.0;
  double conjugate_before = 0.0;
  double work = 0.0;
  double amplitude_after = 0.0;
  double conjugate_after = 0.0;
  double oscillator_energy_before = 0.0;
  double oscillator_energy_after = 0.0;
  double oscillator_energy_change = 0.0;
  double target_energy_change = 0.0;
  double energy_change_residual = 0.0;
  double cotangent_jacobian = 0.0;
  double phase_circle_flux = 0.0;
  bool sign_preserved = false;
  bool cotangent_lift_symplectic = false;
  bool lagrangian_section = false;
  bool exact_work_ledger = false;
  bool constant_action_translation_locally_symplectic = true;
  bool constant_action_translation_globally_hamiltonian = false;
  bool phase_blind_state_dependent_drain_symplectic = false;
  bool square_root_law_promoted_to_physical_reservoir = false;

  bool valid() const { return status == SquareRootBatteryPhaseStatus::Valid; }
};

SquareRootBatteryPhaseAudit audit_square_root_battery_phase_completion(
    double amplitude,
    double conjugate,
    double work,
    double tolerance = 1e-12);

struct CanonicalHistoryPort {
  double coordinate = 0.0;
  double conjugate = 0.0;
};

enum class OpenCanonicalHistoryStatus : std::uint8_t {
  Valid = 0,
  EmptyRail,
  NonFiniteInput,
  InvalidTolerance,
};

struct OpenCanonicalHistoryShiftResult {
  OpenCanonicalHistoryStatus status = OpenCanonicalHistoryStatus::EmptyRail;
  std::vector<CanonicalHistoryPort> before;
  std::vector<CanonicalHistoryPort> after;
  std::vector<CanonicalHistoryPort> recovered;
  CanonicalHistoryPort incoming;
  CanonicalHistoryPort outgoing;
  double rail_energy_before = 0.0;
  double rail_energy_after = 0.0;
  double incoming_energy = 0.0;
  double outgoing_energy = 0.0;
  double open_energy_residual = 0.0;
  bool complete_pair_shifted = false;
  bool symplectic_with_boundaries = false;
  bool exact_inverse_verified = false;
  bool scalar_energy_only_export_sufficient = false;
  bool finite_closed_recycler_claimed = false;

  bool valid() const { return status == OpenCanonicalHistoryStatus::Valid; }
};

OpenCanonicalHistoryShiftResult shift_open_canonical_history_right(
    const std::vector<CanonicalHistoryPort>& rail,
    CanonicalHistoryPort incoming,
    double tolerance = 1e-12);

}  // namespace ftd::eft


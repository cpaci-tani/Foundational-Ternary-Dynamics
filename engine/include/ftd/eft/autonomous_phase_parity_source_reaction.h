#pragma once

/**
 * @file autonomous_phase_parity_source_reaction.h
 * @brief FTD-0887/0888 autonomous parity and source-reaction reference.
 *
 * One periodic phase coordinate compiles six non-overlapping Hamiltonian
 * pulses: residual/history, history/reaction, and reaction phase for each of
 * two checkerboard colors. The local endpoint clears a ready Gauss residual
 * while splitting its positive energy between complete history and reaction
 * pairs. This is an isolated EFT witness, not production matter dynamics.
 */

#include <array>
#include <cstdint>

namespace ftd::eft {

enum class AutonomousPhaseParityStatus : std::uint8_t {
  Valid = 0,
  NonFiniteInput,
  InvalidSplitAngle,
  InvalidClockFrequency,
  InvalidReferenceAction,
  InvalidReferencePhase,
  InvalidTolerance,
  InsufficientReferenceReserve,
};

enum class AutonomousPhasePulseKind : std::uint8_t {
  ResidualHistory = 0,
  HistoryReaction,
  ReactionPhase,
};

struct AutonomousPhasePulseDescriptor {
  int checkerboard_color = 0;
  AutonomousPhasePulseKind kind =
      AutonomousPhasePulseKind::ResidualHistory;
  double phase_start = 0.0;
  double phase_end = 0.0;
  double window_integral = 0.0;
  double target_angle = 0.0;
  double pulse_coefficient = 0.0;
};

struct SourceReactionMode {
  double residual = 0.0;
  double history = 0.0;
  double reaction = 0.0;
  double residual_conjugate = 0.0;
  double history_conjugate = 0.0;
  double reaction_conjugate = 0.0;
};

struct AutonomousPhaseParityInput {
  SourceReactionMode mode;
  double source_offset = 0.0;
  double split_angle = 0.785398163397448309615660845819875721;
  double clock_frequency = 1.0;
  double reference_action = 10.0;
  double reference_phase = 0.0;
  double tolerance = 1e-12;
};

struct AutonomousPhaseParityResult {
  AutonomousPhaseParityStatus status =
      AutonomousPhaseParityStatus::NonFiniteInput;
  SourceReactionMode before;
  SourceReactionMode after;
  SourceReactionMode recovered;
  std::array<AutonomousPhasePulseDescriptor, 6> pulses{};
  double source_offset = 0.0;
  double split_angle = 0.0;
  double clock_frequency = 0.0;
  double cycle_duration = 0.0;
  double window_duration = 0.0;
  double base_winding_per_window = 0.0;
  double common_norm_before = 0.0;
  double common_norm_after = 0.0;
  double carrier_hamiltonian_lower_bound = 0.0;
  double maximum_clock_action_excursion_bound = 0.0;
  double reference_action_before = 0.0;
  double reference_action_after = 0.0;
  double residual_energy_before = 0.0;
  double history_energy_after = 0.0;
  double reaction_energy_after = 0.0;
  double raw_energy_before = 0.0;
  double raw_energy_after = 0.0;
  double interaction_energy_before = 0.0;
  double interaction_energy_after = 0.0;
  double old_source_work = 0.0;
  double completed_energy_residual = 0.0;
  double source_reaction_impulse = 0.0;
  bool autonomous_hamiltonian = true;
  bool external_integer_parity_switch_required = false;
  bool phase_windows_c1 = true;
  bool phase_window_interiors_disjoint = true;
  bool action_returns_at_every_boundary = true;
  bool positive_carrier_hamiltonian = false;
  bool endpoint_symplectic = true;
  bool endpoint_orthogonal = true;
  bool endpoint_orientation_preserving = true;
  bool exact_inverse_verified = false;
  bool ready_reaction_slice = false;
  bool gauss_residual_cleared = false;
  bool reaction_displacement_reset = false;
  bool reaction_impulse_generated = false;
  bool exact_history_reaction_split = false;
  bool exact_completed_energy_ledger = false;
  bool history_only_endpoint_energy_saturated = true;
  bool one_canonical_reaction_pair_minimum_in_registered_class = true;
  bool self_dual_channel_symmetry_selected = false;
  bool equal_history_reaction_energy = false;
  bool spatial_ternary_source_recoil_supplied = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const {
    return status == AutonomousPhaseParityStatus::Valid;
  }
};

AutonomousPhaseParityResult
evolve_autonomous_phase_parity_source_reaction_cycle(
    const AutonomousPhaseParityInput& input);

}  // namespace ftd::eft


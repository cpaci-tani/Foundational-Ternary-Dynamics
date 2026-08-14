#pragma once

/**
 * @file hamiltonian_ternary_quarter_turn_actuator.h
 * @brief FTD-0873 isolated Hamiltonian lift of the ternary quarter-turn.
 *
 * One canonical carrier pair (p,q)=a(s,o) and one independent clock pair
 * implement the FTD-0872 hold/R/R^-1 branches over a complete harmonic gate
 * cycle. The API books the transient clock-action and interaction-energy
 * exchange. The harmonic law, amplitude, frequency, and frozen eligibility
 * are reference inputs; no native one-shot scheduler, production coupling,
 * protected transport, or G* synchronization is claimed.
 */

#include "ftd/eft/oriented_ternary_quarter_turn.h"

#include <cstdint>

namespace ftd::eft {

enum class HamiltonianTernaryActuatorStatus : std::uint8_t {
  Valid = 0,
  InvalidLatch,
  InvalidPort,
  InvalidAmplitude,
  InvalidClockFrequency,
  InvalidReferenceAction,
  InvalidReferencePhase,
  InvalidTolerance,
  InsufficientReferenceReserve,
  LogicalQuarterTurnRejected,
  ContinuousLiftMismatch,
  NonFiniteOutput,
};

struct HamiltonianTernaryActuatorInput {
  std::int8_t latch = 0;
  std::int8_t port = 0;
  bool eligible = false;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
  double amplitude = 1.0;
  double clock_frequency = 1.0;
  double reference_action = 1.0;
  double reference_phase = 0.0;
  double tolerance = 1e-12;
};

struct HamiltonianTernaryActuatorResult {
  HamiltonianTernaryActuatorStatus status =
      HamiltonianTernaryActuatorStatus::InvalidLatch;
  TernaryQuarterTurnResult logical_transfer;
  double carrier_p_before = 0.0;
  double carrier_q_before = 0.0;
  double carrier_p_after = 0.0;
  double carrier_q_after = 0.0;
  double carrier_action_before = 0.0;
  double carrier_action_after = 0.0;
  double cycle_duration = 0.0;
  double base_phase = 0.0;
  double gated_phase = 0.0;
  double total_phase = 0.0;
  double imposed_record_energy_scale = 0.0;
  double record_energy_before = 0.0;
  double record_energy_after = 0.0;
  double reference_action_before = 0.0;
  double minimum_reference_action = 0.0;
  double maximum_reference_action = 0.0;
  double reference_action_after = 0.0;
  double conservative_reserve_margin = 0.0;
  double maximum_clock_action_excursion = 0.0;
  double maximum_reference_energy_exchange = 0.0;
  double maximum_interaction_energy_magnitude = 0.0;
  double gate_zero_switch_work = 0.0;
  double antiphase_switch_work_magnitude = 0.0;
  double endpoint_total_energy_before = 0.0;
  double endpoint_total_energy_after = 0.0;
  double endpoint_energy_residual = 0.0;
  bool exact_hamiltonian_lift = false;
  bool exact_hold = false;
  bool exact_forward_quarter_turn = false;
  bool exact_reverse_quarter_turn = false;
  bool continuous_flow_matches_ternary = false;
  bool carrier_action_preserved = false;
  bool controller_exchange_ledger_supplied = false;
  bool gate_zero_switching_booked = false;
  bool complete_cycle_net_work_zero = false;
  bool repeated_active_cycle_is_one_shot = false;
  bool imposed_record_energy_scale_supplied = false;
  bool native_record_energy_scale_derived = false;
  bool dynamic_one_shot_scheduler_supplied = false;
  bool protected_cubic_transport_supplied = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const {
    return status == HamiltonianTernaryActuatorStatus::Valid;
  }
};

/** Evaluate one exact complete cycle of the imposed harmonic actuator. */
HamiltonianTernaryActuatorResult
evolve_hamiltonian_ternary_quarter_turn_cycle(
    const HamiltonianTernaryActuatorInput& input);

}  // namespace ftd::eft


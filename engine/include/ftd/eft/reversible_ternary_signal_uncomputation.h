#pragma once

/**
 * @file reversible_ternary_signal_uncomputation.h
 * @brief FTD-0871 reversible actual-layer latch uncomputation witness.
 *
 * The completed oriented signal is retained while its decoded Z3 label is
 * subtracted from the matching ternary latch. This is an isolated reference
 * contract, not a production controller or continuous reset trajectory.
 */

#include "ftd/eft/catalytic_phase_reference.h"

#include <cstdint>

namespace ftd::eft {

enum class TernarySignalUncomputationStatus {
  Valid,
  InvalidLatch,
  InvalidTolerance,
  InvalidReference,
  InvalidSignal,
  SignalLatchMismatch,
  NonemptyOutputPort,
  DecoderFailed,
  NonFiniteOutput,
};

struct TernarySignalUncomputationInput {
  std::int8_t latch = 0;
  CanonicalCarrierPair orientation_reference{};
  CanonicalCarrierPair completed_signal{};
  CanonicalCarrierPair output_port{};
  double tolerance = 1e-12;
};

struct TernarySignalUncomputationResult {
  TernarySignalUncomputationStatus status =
      TernarySignalUncomputationStatus::InvalidSignal;
  std::int8_t latch_before = 0;
  std::int8_t decoded_signal_sign = 0;
  std::int8_t latch_after_uncomputation = 0;
  std::int8_t inverse_recovered_latch = 0;
  CanonicalCarrierPair signal_before{};
  CanonicalCarrierPair signal_after_uncomputation{};
  CanonicalCarrierPair local_signal_after_handoff{};
  CanonicalCarrierPair exported_signal{};
  double signal_energy_before = 0.0;
  double signal_energy_after = 0.0;
  double decoded_output_energy = 0.0;
  double signal_energy_residual = 0.0;
  double endpoint_latch_storage_energy_difference = 0.0;
  double logical_bath_energy = 0.0;
  bool signal_completion_acknowledged = false;
  bool ternary_group_bijection_verified = false;
  bool reversible_uncomputation_verified = false;
  bool sign_reversal_equivariant = false;
  bool no_extra_acknowledgement_bit = false;
  bool no_reset_history_trit = false;
  bool no_logical_bath_required = false;
  bool output_handoff_reciprocal = false;
  bool local_actual_state_ready = false;
  bool continuous_latch_reset_supplied = false;
  bool controller_work_ledger_supplied = false;
  bool protected_cubic_transport_supplied = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const {
    return status == TernarySignalUncomputationStatus::Valid;
  }
};

/** Execute one registered actual-layer uncomputation and empty-port handoff. */
TernarySignalUncomputationResult execute_reversible_ternary_signal_uncomputation(
    const TernarySignalUncomputationInput& input);

}  // namespace ftd::eft

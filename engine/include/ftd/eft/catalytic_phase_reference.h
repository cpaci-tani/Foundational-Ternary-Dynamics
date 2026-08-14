#pragma once

/**
 * @file catalytic_phase_reference.h
 * @brief FTD-0863 isolated catalytic phase-reference transducer witness.
 *
 * A nonzero canonical reference pair defines an orthonormal phase frame.  The
 * reference-normal component of a separate signal pair is exchanged with a
 * signed matter amplitude by the FTD-0856 identity/swap gate, while the
 * reference-parallel signal component and reference pair remain unchanged.
 * This lets an initially zero signal receive the event energy without loading
 * or spending the reference action.
 *
 * This selected ftd::eft interface is not a production phase, a de Broglie
 * guidance law, a G* cadence, a cost-free controller, or a Voxel consumer.
 */

#include "ftd/eft/phase_referenced_action_rail.h"
#include "ftd/eft/reciprocal_record_port.h"

#include <cstdint>

namespace ftd::eft {

enum class CatalyticPhaseReferenceStatus : std::uint8_t {
  Valid = 0,
  InvalidReference,
  InvalidMatterAmplitude,
  InvalidSignal,
  InvalidTolerance,
  InvalidEligibility,
  NonFiniteOutput,
};

struct PhaseReferenceRotationResult {
  CatalyticPhaseReferenceStatus status =
      CatalyticPhaseReferenceStatus::InvalidReference;
  CanonicalCarrierPair before;
  CanonicalCarrierPair after;
  double phase_advance = 0.0;
  double action_before = 0.0;
  double action_after = 0.0;
  double action_residual = 0.0;
  double jacobian_determinant = 0.0;

  bool valid() const { return status == CatalyticPhaseReferenceStatus::Valid; }
};

struct CatalyticPhaseExchangeInput {
  CanonicalCarrierPair reference;
  double matter_amplitude = 0.0;
  CanonicalCarrierPair signal;
  RecordPortEligibility eligibility = RecordPortEligibility::Hold;
  double tolerance = 1e-12;
};

struct CatalyticPhaseExchangeResult {
  CatalyticPhaseReferenceStatus status =
      CatalyticPhaseReferenceStatus::InvalidReference;
  CanonicalCarrierPair reference_before;
  CanonicalCarrierPair reference_after;
  CanonicalCarrierPair signal_before;
  CanonicalCarrierPair signal_after;
  double reference_action_before = 0.0;
  double reference_action_after = 0.0;
  double matter_before = 0.0;
  double matter_after = 0.0;
  double orthogonal_signal_before = 0.0;
  double orthogonal_signal_after = 0.0;
  double parallel_signal_before = 0.0;
  double parallel_signal_after = 0.0;
  double matter_signal_energy_before = 0.0;
  double matter_signal_energy_after = 0.0;
  double total_energy_before = 0.0;
  double total_energy_after = 0.0;
  double energy_residual = 0.0;
  double signed_content_before = 0.0;
  double signed_content_after = 0.0;
  double signed_content_residual = 0.0;
  double oriented_area_after = 0.0;
  bool gate_exchanged = false;

  bool valid() const { return status == CatalyticPhaseReferenceStatus::Valid; }
};

struct CatalyticSignalReadout {
  CatalyticPhaseReferenceStatus status =
      CatalyticPhaseReferenceStatus::InvalidReference;
  double reference_action = 0.0;
  double signal_energy = 0.0;
  double orthogonal_amplitude = 0.0;
  double parallel_amplitude = 0.0;
  double oriented_area = 0.0;

  bool valid() const { return status == CatalyticPhaseReferenceStatus::Valid; }
};

/** Advance the isolated reference by beta' = R(-phase_advance) beta. */
PhaseReferenceRotationResult rotate_catalytic_phase_reference(
    const CanonicalCarrierPair& reference,
    double phase_advance);

/** Apply the identity/swap gate in the local phase frame of the reference. */
CatalyticPhaseExchangeResult exchange_catalytic_phase_signal(
    const CatalyticPhaseExchangeInput& input);

/** Resolve a signal pair into reference-normal and reference-parallel parts. */
CatalyticSignalReadout read_catalytic_phase_signal(
    const CanonicalCarrierPair& reference,
    const CanonicalCarrierPair& signal);

}  // namespace ftd::eft


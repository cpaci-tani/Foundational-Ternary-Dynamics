#pragma once
/**
 * @file reciprocal_record_port.h
 * @brief FTD-0856 isolated reciprocal record/field boundary witness.
 *
 * This is a selected EFT reference interface.  It is not Voxel state, does
 * not derive the physical eligibility signal, and is not called by any
 * production tick phase.  The caller supplies the hold/exchange eligibility
 * value and the adopted event energy.
 */

#include <cstdint>

namespace ftd::eft {

enum class RecordPortEligibility : std::uint8_t {
  Hold = 0,
  Exchange = 1,
};

enum class RecordPortStatus : std::uint8_t {
  Valid = 0,
  InvalidEnergy,
  InvalidTolerance,
  InvalidRecord,
  InvalidIncomingAmplitude,
  InvalidEligibility,
  InvalidOutputRecord,
};

struct ReciprocalRecordPortInput {
  std::int8_t record = 0;
  double incoming = 0.0;
  double event_energy = 0.0;
  RecordPortEligibility eligibility = RecordPortEligibility::Hold;
  double tolerance = 1e-12;
};

struct ReciprocalRecordPortResult {
  RecordPortStatus status = RecordPortStatus::InvalidEnergy;
  std::int8_t record_before = 0;
  std::int8_t record_after = 0;
  double event_energy = 0.0;
  double event_amplitude = 0.0;
  double matter_before = 0.0;
  double matter_after = 0.0;
  double incoming = 0.0;
  double outgoing = 0.0;
  double energy_before = 0.0;
  double energy_after = 0.0;
  double energy_residual = 0.0;
  double signed_content_before = 0.0;
  double signed_content_after = 0.0;
  double signed_content_residual = 0.0;
  bool gate_exchanged = false;

  bool valid() const { return status == RecordPortStatus::Valid; }
};

/**
 * Apply the FTD-0856 controlled identity/swap on the quantized reference
 * domain record,incoming in {-1,0,+1} x { -sqrt(2B),0,+sqrt(2B) }.
 * Invalid or non-finite inputs fail closed and return valid()==false.
 */
ReciprocalRecordPortResult scatter_reciprocal_record_port(
    const ReciprocalRecordPortInput& input);

}  // namespace ftd::eft

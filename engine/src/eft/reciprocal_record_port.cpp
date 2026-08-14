#include "ftd/eft/reciprocal_record_port.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool close_scaled(double value, double target, double scale, double tolerance) {
  return std::abs(value - target)
      <= tolerance * std::max({1.0, std::abs(scale), std::abs(target)});
}

bool valid_record(std::int8_t record) {
  return record >= -1 && record <= 1;
}

bool classify_amplitude(
    double value,
    double event_amplitude,
    double tolerance,
    std::int8_t& record) {
  if (!std::isfinite(value)) return false;
  if (close_scaled(value, 0.0, event_amplitude, tolerance)) {
    record = 0;
    return true;
  }
  if (close_scaled(value, event_amplitude, event_amplitude, tolerance)) {
    record = 1;
    return true;
  }
  if (close_scaled(value, -event_amplitude, event_amplitude, tolerance)) {
    record = -1;
    return true;
  }
  return false;
}

}  // namespace

ReciprocalRecordPortResult scatter_reciprocal_record_port(
    const ReciprocalRecordPortInput& input) {
  ReciprocalRecordPortResult result;
  result.record_before = input.record;
  result.incoming = input.incoming;
  result.event_energy = input.event_energy;

  if (!(input.event_energy > 0.0) || !std::isfinite(input.event_energy)) {
    result.status = RecordPortStatus::InvalidEnergy;
    return result;
  }
  if (!(input.tolerance >= 0.0) || !std::isfinite(input.tolerance)) {
    result.status = RecordPortStatus::InvalidTolerance;
    return result;
  }
  if (!valid_record(input.record)) {
    result.status = RecordPortStatus::InvalidRecord;
    return result;
  }

  result.event_amplitude = std::sqrt(2.0 * input.event_energy);
  if (!(result.event_amplitude > 0.0)
      || !std::isfinite(result.event_amplitude)) {
    result.status = RecordPortStatus::InvalidEnergy;
    return result;
  }

  std::int8_t incoming_record = 0;
  if (!classify_amplitude(
          input.incoming, result.event_amplitude, input.tolerance,
          incoming_record)) {
    result.status = RecordPortStatus::InvalidIncomingAmplitude;
    return result;
  }

  result.matter_before = result.event_amplitude
      * static_cast<double>(input.record);
  switch (input.eligibility) {
    case RecordPortEligibility::Hold:
      result.matter_after = result.matter_before;
      result.outgoing = input.incoming;
      result.gate_exchanged = false;
      break;
    case RecordPortEligibility::Exchange:
      result.matter_after = input.incoming;
      result.outgoing = result.matter_before;
      result.gate_exchanged = true;
      break;
    default:
      result.status = RecordPortStatus::InvalidEligibility;
      return result;
  }

  if (!classify_amplitude(
          result.matter_after, result.event_amplitude, input.tolerance,
          result.record_after)) {
    result.status = RecordPortStatus::InvalidOutputRecord;
    return result;
  }

  result.energy_before = 0.5 * (
      result.matter_before * result.matter_before
      + result.incoming * result.incoming);
  result.energy_after = 0.5 * (
      result.matter_after * result.matter_after
      + result.outgoing * result.outgoing);
  result.energy_residual = result.energy_after - result.energy_before;
  result.signed_content_before = result.matter_before + result.incoming;
  result.signed_content_after = result.matter_after + result.outgoing;
  result.signed_content_residual =
      result.signed_content_after - result.signed_content_before;
  result.status = RecordPortStatus::Valid;
  return result;
}

}  // namespace ftd::eft

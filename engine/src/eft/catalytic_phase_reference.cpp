#include "ftd/eft/catalytic_phase_reference.h"

#include <cmath>

namespace ftd::eft {
namespace {

bool finite_pair(const CanonicalCarrierPair& pair) {
  return std::isfinite(pair.q) && std::isfinite(pair.p);
}

double pair_action(const CanonicalCarrierPair& pair) {
  const double radius = std::hypot(pair.q, pair.p);
  return 0.5 * radius * radius;
}

struct PhaseFrame {
  bool valid = false;
  double radius = 0.0;
  CanonicalCarrierPair parallel;
  CanonicalCarrierPair orthogonal;
};

PhaseFrame make_phase_frame(const CanonicalCarrierPair& reference) {
  PhaseFrame frame;
  if (!finite_pair(reference)) return frame;
  frame.radius = std::hypot(reference.q, reference.p);
  if (!std::isfinite(frame.radius) || !(frame.radius > 0.0)) return frame;
  frame.parallel.q = reference.q / frame.radius;
  frame.parallel.p = reference.p / frame.radius;
  frame.orthogonal.q = -frame.parallel.p;
  frame.orthogonal.p = frame.parallel.q;
  frame.valid = true;
  return frame;
}

double dot(
    const CanonicalCarrierPair& first,
    const CanonicalCarrierPair& second) {
  return first.q * second.q + first.p * second.p;
}

double wedge(
    const CanonicalCarrierPair& first,
    const CanonicalCarrierPair& second) {
  return first.q * second.p - first.p * second.q;
}

CanonicalCarrierPair compose_signal(
    const PhaseFrame& frame,
    double orthogonal_amplitude,
    double parallel_amplitude) {
  CanonicalCarrierPair signal;
  signal.q = orthogonal_amplitude * frame.orthogonal.q
      + parallel_amplitude * frame.parallel.q;
  signal.p = orthogonal_amplitude * frame.orthogonal.p
      + parallel_amplitude * frame.parallel.p;
  return signal;
}

}  // namespace

PhaseReferenceRotationResult rotate_catalytic_phase_reference(
    const CanonicalCarrierPair& reference,
    double phase_advance) {
  PhaseReferenceRotationResult result;
  result.before = reference;
  result.phase_advance = phase_advance;
  if (!make_phase_frame(reference).valid || !std::isfinite(phase_advance)) {
    result.status = CatalyticPhaseReferenceStatus::InvalidReference;
    return result;
  }
  result.action_before = pair_action(reference);
  const double cosine = std::cos(phase_advance);
  const double sine = std::sin(phase_advance);
  result.after.q = cosine * reference.q + sine * reference.p;
  result.after.p = -sine * reference.q + cosine * reference.p;
  if (!finite_pair(result.after)) {
    result.status = CatalyticPhaseReferenceStatus::NonFiniteOutput;
    return result;
  }
  result.action_after = pair_action(result.after);
  result.action_residual = result.action_after - result.action_before;
  result.jacobian_determinant = 1.0;
  result.status = CatalyticPhaseReferenceStatus::Valid;
  return result;
}

CatalyticSignalReadout read_catalytic_phase_signal(
    const CanonicalCarrierPair& reference,
    const CanonicalCarrierPair& signal) {
  CatalyticSignalReadout result;
  const auto frame = make_phase_frame(reference);
  if (!frame.valid) {
    result.status = CatalyticPhaseReferenceStatus::InvalidReference;
    return result;
  }
  if (!finite_pair(signal)) {
    result.status = CatalyticPhaseReferenceStatus::InvalidSignal;
    return result;
  }
  result.reference_action = pair_action(reference);
  result.signal_energy = pair_action(signal);
  result.orthogonal_amplitude = dot(frame.orthogonal, signal);
  result.parallel_amplitude = dot(frame.parallel, signal);
  result.oriented_area = wedge(reference, signal);
  if (!std::isfinite(result.reference_action)
      || !std::isfinite(result.signal_energy)
      || !std::isfinite(result.orthogonal_amplitude)
      || !std::isfinite(result.parallel_amplitude)
      || !std::isfinite(result.oriented_area)) {
    result.status = CatalyticPhaseReferenceStatus::NonFiniteOutput;
    return result;
  }
  result.status = CatalyticPhaseReferenceStatus::Valid;
  return result;
}

CatalyticPhaseExchangeResult exchange_catalytic_phase_signal(
    const CatalyticPhaseExchangeInput& input) {
  CatalyticPhaseExchangeResult result;
  result.reference_before = input.reference;
  result.reference_after = input.reference;
  result.signal_before = input.signal;
  result.matter_before = input.matter_amplitude;

  const auto frame = make_phase_frame(input.reference);
  if (!frame.valid) {
    result.status = CatalyticPhaseReferenceStatus::InvalidReference;
    return result;
  }
  if (!std::isfinite(input.matter_amplitude)) {
    result.status = CatalyticPhaseReferenceStatus::InvalidMatterAmplitude;
    return result;
  }
  if (!finite_pair(input.signal)) {
    result.status = CatalyticPhaseReferenceStatus::InvalidSignal;
    return result;
  }
  if (!std::isfinite(input.tolerance) || input.tolerance < 0.0) {
    result.status = CatalyticPhaseReferenceStatus::InvalidTolerance;
    return result;
  }

  result.reference_action_before = pair_action(input.reference);
  result.reference_action_after = result.reference_action_before;
  result.orthogonal_signal_before = dot(frame.orthogonal, input.signal);
  result.parallel_signal_before = dot(frame.parallel, input.signal);
  result.parallel_signal_after = result.parallel_signal_before;

  switch (input.eligibility) {
    case RecordPortEligibility::Hold:
      result.matter_after = result.matter_before;
      result.orthogonal_signal_after = result.orthogonal_signal_before;
      result.gate_exchanged = false;
      break;
    case RecordPortEligibility::Exchange:
      result.matter_after = result.orthogonal_signal_before;
      result.orthogonal_signal_after = result.matter_before;
      result.gate_exchanged = true;
      break;
    default:
      result.status = CatalyticPhaseReferenceStatus::InvalidEligibility;
      return result;
  }

  result.signal_after = compose_signal(
      frame,
      result.orthogonal_signal_after,
      result.parallel_signal_after);
  result.matter_signal_energy_before = 0.5 * (
      result.matter_before * result.matter_before
      + input.signal.q * input.signal.q
      + input.signal.p * input.signal.p);
  result.matter_signal_energy_after = 0.5 * (
      result.matter_after * result.matter_after
      + result.signal_after.q * result.signal_after.q
      + result.signal_after.p * result.signal_after.p);
  result.total_energy_before = result.reference_action_before
      + result.matter_signal_energy_before;
  result.total_energy_after = result.reference_action_after
      + result.matter_signal_energy_after;
  result.energy_residual = result.total_energy_after - result.total_energy_before;
  result.signed_content_before =
      result.matter_before + result.orthogonal_signal_before;
  result.signed_content_after =
      result.matter_after + result.orthogonal_signal_after;
  result.signed_content_residual =
      result.signed_content_after - result.signed_content_before;
  result.oriented_area_after = wedge(input.reference, result.signal_after);

  if (!finite_pair(result.signal_after)
      || !std::isfinite(result.matter_after)
      || !std::isfinite(result.total_energy_before)
      || !std::isfinite(result.total_energy_after)
      || !std::isfinite(result.energy_residual)
      || !std::isfinite(result.signed_content_residual)
      || !std::isfinite(result.oriented_area_after)) {
    result.status = CatalyticPhaseReferenceStatus::NonFiniteOutput;
    return result;
  }
  result.status = CatalyticPhaseReferenceStatus::Valid;
  return result;
}

}  // namespace ftd::eft


#include "ftd/eft/reversible_ternary_signal_uncomputation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <utility>

namespace ftd::eft {
namespace {

constexpr std::array<std::int8_t, 3> kTernary{-1, 0, 1};

bool finite_pair(const CanonicalCarrierPair& pair) {
  return std::isfinite(pair.q) && std::isfinite(pair.p);
}

double pair_energy(const CanonicalCarrierPair& pair) {
  const double radius = std::hypot(pair.q, pair.p);
  return 0.5 * radius * radius;
}

bool near_zero_pair(const CanonicalCarrierPair& pair, double tolerance) {
  return std::hypot(pair.q, pair.p) <= tolerance;
}

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

bool ternary(std::int8_t value) {
  return value >= -1 && value <= 1;
}

int encode(std::int8_t value) {
  if (value == -1) return 2;
  return static_cast<int>(value);
}

std::int8_t decode(int residue) {
  const int normalized = (residue % 3 + 3) % 3;
  return normalized == 2 ? std::int8_t{-1}
                         : static_cast<std::int8_t>(normalized);
}

std::int8_t ternary_add(std::int8_t first, std::int8_t second) {
  return decode(encode(first) + encode(second));
}

std::int8_t ternary_subtract(std::int8_t first, std::int8_t second) {
  return decode(encode(first) - encode(second));
}

std::int8_t sign_with_tolerance(double value, double tolerance) {
  if (value > tolerance) return 1;
  if (value < -tolerance) return -1;
  return 0;
}

bool full_group_bijection() {
  for (const std::int8_t control : kTernary) {
    std::set<std::pair<int, int>> outputs;
    for (const std::int8_t latch : kTernary) {
      const auto output = ternary_subtract(latch, control);
      if (ternary_add(output, control) != latch) return false;
      outputs.emplace(static_cast<int>(output), static_cast<int>(control));
    }
    if (outputs.size() != kTernary.size()) return false;
  }
  return true;
}

}  // namespace

TernarySignalUncomputationResult execute_reversible_ternary_signal_uncomputation(
    const TernarySignalUncomputationInput& input) {
  TernarySignalUncomputationResult result;
  result.latch_before = input.latch;
  result.signal_before = input.completed_signal;
  result.signal_after_uncomputation = input.completed_signal;
  result.continuous_latch_reset_supplied = false;
  result.controller_work_ledger_supplied = false;
  result.protected_cubic_transport_supplied = false;
  result.production_coupling_supplied = false;
  result.native_gstar_synchronization_supplied = false;

  if (!ternary(input.latch)) {
    result.status = TernarySignalUncomputationStatus::InvalidLatch;
    return result;
  }
  if (!std::isfinite(input.tolerance) || input.tolerance < 0.0) {
    result.status = TernarySignalUncomputationStatus::InvalidTolerance;
    return result;
  }
  if (!finite_pair(input.orientation_reference)
      || !(pair_energy(input.orientation_reference) > 0.0)) {
    result.status = TernarySignalUncomputationStatus::InvalidReference;
    return result;
  }
  if (!finite_pair(input.completed_signal)) {
    result.status = TernarySignalUncomputationStatus::InvalidSignal;
    return result;
  }
  if (!finite_pair(input.output_port)
      || !near_zero_pair(input.output_port, input.tolerance)) {
    result.status = TernarySignalUncomputationStatus::NonemptyOutputPort;
    return result;
  }

  result.ternary_group_bijection_verified = full_group_bijection();
  result.no_extra_acknowledgement_bit = true;
  result.no_reset_history_trit = true;
  result.no_logical_bath_required = true;
  result.logical_bath_energy = 0.0;
  result.endpoint_latch_storage_energy_difference = 0.0;
  result.sign_reversal_equivariant = true;

  result.signal_energy_before = pair_energy(input.completed_signal);
  if (input.latch == 0) {
    if (!near_zero_pair(input.completed_signal, input.tolerance)) {
      result.status = TernarySignalUncomputationStatus::SignalLatchMismatch;
      return result;
    }
    result.decoded_signal_sign = 0;
    result.latch_after_uncomputation = 0;
    result.inverse_recovered_latch = 0;
    result.signal_after_uncomputation = input.completed_signal;
    result.local_signal_after_handoff = input.completed_signal;
    result.exported_signal = input.output_port;
    result.signal_energy_after = 0.0;
    result.decoded_output_energy = 0.0;
    result.signal_completion_acknowledged = false;
    result.reversible_uncomputation_verified =
        result.ternary_group_bijection_verified;
    result.output_handoff_reciprocal = true;
    result.local_actual_state_ready = true;
    result.status = TernarySignalUncomputationStatus::Valid;
    return result;
  }

  if (!(result.signal_energy_before > input.tolerance * input.tolerance)) {
    result.status = TernarySignalUncomputationStatus::InvalidSignal;
    return result;
  }
  const auto readout = read_catalytic_phase_signal(
      input.orientation_reference,
      input.completed_signal);
  if (!readout.valid()) {
    result.status = TernarySignalUncomputationStatus::DecoderFailed;
    return result;
  }
  const double signal_scale = std::max(
      1.0,
      std::sqrt(2.0 * result.signal_energy_before));
  if (std::abs(readout.parallel_amplitude)
      > input.tolerance * signal_scale) {
    result.status = TernarySignalUncomputationStatus::InvalidSignal;
    return result;
  }
  result.decoded_signal_sign = sign_with_tolerance(
      readout.oriented_area,
      input.tolerance * signal_scale);
  if (result.decoded_signal_sign != input.latch) {
    result.status = TernarySignalUncomputationStatus::SignalLatchMismatch;
    return result;
  }

  result.signal_completion_acknowledged = true;
  result.latch_after_uncomputation = ternary_subtract(
      input.latch,
      result.decoded_signal_sign);
  result.inverse_recovered_latch = ternary_add(
      result.latch_after_uncomputation,
      result.decoded_signal_sign);
  result.reversible_uncomputation_verified =
      result.ternary_group_bijection_verified
      && result.latch_after_uncomputation == 0
      && result.inverse_recovered_latch == input.latch;

  result.signal_energy_after = pair_energy(result.signal_after_uncomputation);
  result.signal_energy_residual =
      result.signal_energy_after - result.signal_energy_before;
  result.local_signal_after_handoff = input.output_port;
  result.exported_signal = result.signal_after_uncomputation;
  result.output_handoff_reciprocal =
      near_zero_pair(result.local_signal_after_handoff, input.tolerance);

  const auto output_readout = read_catalytic_phase_signal(
      input.orientation_reference,
      result.exported_signal);
  if (!output_readout.valid()) {
    result.status = TernarySignalUncomputationStatus::DecoderFailed;
    return result;
  }
  result.decoded_output_energy = output_readout.signal_energy;
  const auto decoded_output_sign = sign_with_tolerance(
      output_readout.oriented_area,
      input.tolerance * signal_scale);
  result.local_actual_state_ready =
      result.reversible_uncomputation_verified
      && result.output_handoff_reciprocal
      && decoded_output_sign == input.latch
      && close(
          result.decoded_output_energy,
          result.signal_energy_before,
          input.tolerance)
      && close(result.signal_energy_residual, 0.0, input.tolerance);

  if (!result.local_actual_state_ready
      || !std::isfinite(result.signal_energy_after)
      || !std::isfinite(result.decoded_output_energy)
      || !std::isfinite(result.signal_energy_residual)) {
    result.status = TernarySignalUncomputationStatus::NonFiniteOutput;
    return result;
  }

  result.status = TernarySignalUncomputationStatus::Valid;
  return result;
}

}  // namespace ftd::eft

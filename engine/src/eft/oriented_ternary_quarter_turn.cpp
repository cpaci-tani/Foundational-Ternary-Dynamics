#include "ftd/eft/oriented_ternary_quarter_turn.h"

#include <array>
#include <set>
#include <utility>

namespace ftd::eft {
namespace {

constexpr std::array<std::int8_t, 3> kTernary{-1, 0, 1};

bool ternary(std::int8_t value) {
  return value >= -1 && value <= 1;
}

std::pair<std::int8_t, std::int8_t> forward(
    std::int8_t latch, std::int8_t port) {
  return {static_cast<std::int8_t>(-port), latch};
}

std::pair<std::int8_t, std::int8_t> reverse(
    std::int8_t latch, std::int8_t port) {
  return {port, static_cast<std::int8_t>(-latch)};
}

std::pair<std::int8_t, std::int8_t> apply_orientation(
    std::int8_t latch,
    std::int8_t port,
    TernaryQuarterTurnOrientation orientation) {
  return orientation == TernaryQuarterTurnOrientation::Forward
      ? forward(latch, port)
      : reverse(latch, port);
}

TernaryQuarterTurnOrientation inverse_orientation(
    TernaryQuarterTurnOrientation orientation) {
  return orientation == TernaryQuarterTurnOrientation::Forward
      ? TernaryQuarterTurnOrientation::Reverse
      : TernaryQuarterTurnOrientation::Forward;
}

int label_norm(std::int8_t latch, std::int8_t port) {
  return static_cast<int>(latch) * static_cast<int>(latch)
      + static_cast<int>(port) * static_cast<int>(port);
}

int oriented_area(
    std::int8_t latch_before,
    std::int8_t port_before,
    std::int8_t latch_after,
    std::int8_t port_after) {
  return static_cast<int>(latch_before) * static_cast<int>(port_after)
      - static_cast<int>(port_before) * static_cast<int>(latch_after);
}

bool full_bijection(TernaryQuarterTurnOrientation orientation) {
  std::set<std::pair<int, int>> outputs;
  for (const auto latch : kTernary) {
    for (const auto port : kTernary) {
      const auto output = apply_orientation(latch, port, orientation);
      outputs.emplace(output.first, output.second);
    }
  }
  return outputs.size() == 9;
}

bool sign_reversal_equivariant(TernaryQuarterTurnOrientation orientation) {
  for (const auto latch : kTernary) {
    for (const auto port : kTernary) {
      const auto direct = apply_orientation(latch, port, orientation);
      const auto reversed = apply_orientation(
          static_cast<std::int8_t>(-latch),
          static_cast<std::int8_t>(-port),
          orientation);
      if (reversed.first != -direct.first
          || reversed.second != -direct.second) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace

TernaryQuarterTurnResult apply_oriented_ternary_quarter_turn(
    const TernaryQuarterTurnInput& input) {
  TernaryQuarterTurnResult result;
  result.latch_before = input.latch;
  result.port_before = input.port;
  result.latch_after = input.latch;
  result.port_after = input.port;
  result.eligible = input.eligible;
  result.orientation = input.orientation;
  result.physical_energy_scale_supplied = false;
  result.controller_work_ledger_supplied = false;
  result.protected_cubic_transport_supplied = false;
  result.production_coupling_supplied = false;
  result.native_gstar_synchronization_supplied = false;

  if (!ternary(input.latch)) {
    result.status = TernaryQuarterTurnStatus::InvalidLatch;
    return result;
  }
  if (!ternary(input.port)) {
    result.status = TernaryQuarterTurnStatus::InvalidPort;
    return result;
  }

  result.label_norm_before = label_norm(input.latch, input.port);
  if (input.eligible) {
    const auto output = apply_orientation(
        input.latch, input.port, input.orientation);
    result.latch_after = output.first;
    result.port_after = output.second;
  }
  result.label_norm_after = label_norm(result.latch_after, result.port_after);
  result.oriented_area = oriented_area(
      result.latch_before,
      result.port_before,
      result.latch_after,
      result.port_after);

  if (input.eligible) {
    const auto recovered = apply_orientation(
        result.latch_after,
        result.port_after,
        inverse_orientation(input.orientation));
    result.recovered_latch = recovered.first;
    result.recovered_port = recovered.second;
  } else {
    result.recovered_latch = result.latch_after;
    result.recovered_port = result.port_after;
  }

  result.exact_inverse_verified =
      result.recovered_latch == input.latch
      && result.recovered_port == input.port;
  result.label_norm_preserved =
      result.label_norm_after == result.label_norm_before;
  result.sign_reversal_equivariant =
      sign_reversal_equivariant(input.orientation);
  result.unique_oriented_isometry = true;
  result.no_logical_erasure =
      full_bijection(TernaryQuarterTurnOrientation::Forward)
      && full_bijection(TernaryQuarterTurnOrientation::Reverse);
  result.ready_emission =
      input.eligible
      && input.orientation == TernaryQuarterTurnOrientation::Forward
      && input.port == 0
      && result.latch_after == 0
      && result.port_after == input.latch;
  result.reciprocal_absorption =
      input.eligible
      && input.orientation == TernaryQuarterTurnOrientation::Reverse
      && input.latch == 0
      && result.latch_after == input.port
      && result.port_after == 0;
  result.nonempty_port_reciprocal_exchange =
      input.eligible && input.port != 0;
  result.naive_empty_port_fail_closed_noninjective = true;

  result.status = TernaryQuarterTurnStatus::Valid;
  return result;
}

}  // namespace ftd::eft


/** FTD-0872 isolated oriented ternary quarter-turn verifier. */

#include "ftd/eft/oriented_ternary_quarter_turn.h"

#include <array>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <utility>

namespace {

using namespace ftd::eft;

int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

TernaryQuarterTurnResult run(
    std::int8_t latch,
    std::int8_t port,
    bool eligible,
    TernaryQuarterTurnOrientation orientation) {
  TernaryQuarterTurnInput input;
  input.latch = latch;
  input.port = port;
  input.eligible = eligible;
  input.orientation = orientation;
  return apply_oriented_ternary_quarter_turn(input);
}

}  // namespace

int main() {
  constexpr std::array<std::int8_t, 3> ternary{-1, 0, 1};

  for (const auto latch : ternary) {
    for (const auto port : ternary) {
      const auto held = run(
          latch, port, false, TernaryQuarterTurnOrientation::Forward);
      check("ineligible gate is valid identity",
          held.valid() && held.latch_after == latch && held.port_after == port);
      check("ineligible gate inverse and norm close",
          held.exact_inverse_verified && held.label_norm_preserved);

      const auto forward = run(
          latch, port, true, TernaryQuarterTurnOrientation::Forward);
      check("forward map is exact quarter-turn",
          forward.valid()
          && forward.latch_after == -port
          && forward.port_after == latch);
      check("forward inverse recovers input",
          forward.exact_inverse_verified
          && forward.recovered_latch == latch
          && forward.recovered_port == port);
      check("forward norm and orientation close",
          forward.label_norm_preserved
          && forward.oriented_area == forward.label_norm_before);

      const auto reverse = run(
          latch, port, true, TernaryQuarterTurnOrientation::Reverse);
      check("reverse map is exact inverse quarter-turn",
          reverse.valid()
          && reverse.latch_after == port
          && reverse.port_after == -latch);
      check("reverse inverse recovers input",
          reverse.exact_inverse_verified
          && reverse.recovered_latch == latch
          && reverse.recovered_port == port);
      check("reverse norm and orientation close",
          reverse.label_norm_preserved
          && reverse.oriented_area == -reverse.label_norm_before);

      check("scope invariants remain explicit",
          forward.unique_oriented_isometry
          && forward.no_logical_erasure
          && forward.sign_reversal_equivariant
          && forward.naive_empty_port_fail_closed_noninjective
          && !forward.physical_energy_scale_supplied
          && !forward.controller_work_ledger_supplied
          && !forward.protected_cubic_transport_supplied
          && !forward.production_coupling_supplied
          && !forward.native_gstar_synchronization_supplied);
    }
  }

  for (const auto sign : ternary) {
    const auto emission = run(
        sign, 0, true, TernaryQuarterTurnOrientation::Forward);
    check("ready emission transfers sign and clears latch",
        emission.ready_emission
        && emission.latch_after == 0
        && emission.port_after == sign);
    const auto absorption = run(
        0, sign, true, TernaryQuarterTurnOrientation::Reverse);
    check("reciprocal absorption restores sign and clears port",
        absorption.reciprocal_absorption
        && absorption.latch_after == sign
        && absorption.port_after == 0);
  }

  const auto occupied = run(
      1, -1, true, TernaryQuarterTurnOrientation::Forward);
  check("nonempty output undergoes reversible exchange",
      occupied.nonempty_port_reciprocal_exchange
      && occupied.latch_after == 1
      && occupied.port_after == 1
      && occupied.exact_inverse_verified);

  std::set<std::pair<int, int>> outputs;
  for (const auto latch : ternary) {
    for (const auto port : ternary) {
      const auto result = run(
          latch, port, true, TernaryQuarterTurnOrientation::Forward);
      outputs.emplace(result.latch_after, result.port_after);
    }
  }
  check("forward quarter-turn is a nine-state permutation", outputs.size() == 9);

  auto invalid_latch = run(
      2, 0, true, TernaryQuarterTurnOrientation::Forward);
  check("invalid latch fails closed",
      invalid_latch.status == TernaryQuarterTurnStatus::InvalidLatch);
  auto invalid_port = run(
      0, -2, true, TernaryQuarterTurnOrientation::Forward);
  check("invalid port fails closed",
      invalid_port.status == TernaryQuarterTurnStatus::InvalidPort);

  std::cout << "FTD-0872 oriented ternary quarter-turn EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "logical_transfer=UNIQUE_ORIENTED_F3_ISOMETRY\n";
  std::cout << "ready_emission=EXACT\n";
  std::cout << "nonempty_port=RECIPROCAL_EXCHANGE_NOT_HIDDEN_HOLD\n";
  std::cout << "physical_energy_controller_transport_production_gstar=OPEN\n";
  return failures == 0 ? 0 : 1;
}


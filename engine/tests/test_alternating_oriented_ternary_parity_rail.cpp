/** FTD-0874 isolated alternating oriented ternary parity-rail verifier. */

#include "ftd/eft/alternating_oriented_ternary_parity_rail.h"
#include "ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <string>
#include <vector>

namespace {

int checks = 0;
int failures = 0;

void check(const std::string& label, bool condition) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cerr << "FAIL  " << label << '\n';
  }
}

std::vector<std::vector<std::int8_t>> ternary_states(std::size_t length) {
  std::vector<std::vector<std::int8_t>> states(1);
  for (std::size_t index = 0; index < length; ++index) {
    std::vector<std::vector<std::int8_t>> next;
    next.reserve(states.size() * 3);
    for (const auto& state : states) {
      for (std::int8_t value = -1; value <= 1; ++value) {
        auto extended = state;
        extended.push_back(value);
        next.push_back(std::move(extended));
      }
    }
    states = std::move(next);
  }
  return states;
}

std::size_t pulse_position(const std::vector<std::int8_t>& rail) {
  for (std::size_t index = 0; index < rail.size(); ++index) {
    if (rail[index] != 0) return index;
  }
  return rail.size();
}

}  // namespace

int main() {
  using namespace ftd::eft;

  for (std::size_t length = 2; length <= 6; ++length) {
    const auto states = ternary_states(length);
    for (std::uint64_t tick = 0; tick < 2; ++tick) {
      std::set<std::vector<std::int8_t>> outputs;
      for (const auto& state : states) {
        const auto forward =
            step_alternating_oriented_ternary_parity_rail(state, tick);
        check("forward finite layer valid", forward.valid());
        check("forward inverse exact", forward.exact_inverse_verified);
        check("forward norm and support preserved",
            forward.label_norm_preserved
            && forward.nonzero_label_count_preserved
            && forward.sign_reversal_equivariant);
        check("forward locality and matching explicit",
            forward.disjoint_matching
            && forward.nearest_neighbor_local
            && forward.unmatched_endpoints_retained);
        check("forward scope remains isolated",
            forward.finite_horizon_only
            && forward.existing_global_tick_parity_used
            && !forward.new_selected_type_added
            && !forward.native_intersite_hamiltonian_supplied
            && !forward.production_coupling_supplied
            && !forward.native_gstar_synchronization_supplied);
        outputs.insert(forward.after);

        const auto reverse =
            reverse_alternating_oriented_ternary_parity_rail(
                forward.after, tick);
        check("public reverse layer recovers input",
            reverse.valid() && reverse.after == state);
      }
      check("finite layer is a full-state permutation",
          outputs.size() == states.size());
    }
  }

  for (std::int8_t sign : {-1, 1}) {
    constexpr std::size_t horizon = 12;
    std::vector<std::int8_t> rail(horizon + 1, 0);
    rail[0] = sign;
    const auto initial = rail;
    for (std::uint64_t tick = 0; tick < horizon; ++tick) {
      const auto step =
          step_alternating_oriented_ternary_parity_rail(rail, tick);
      check("prepared pulse step valid", step.valid());
      rail = step.after;
      check("prepared pulse moves one edge per tick",
          pulse_position(rail) == tick + 1
          && rail[tick + 1] == sign);
      check("prepared pulse clears trail",
          std::all_of(rail.begin(), rail.begin() + tick + 1,
              [](std::int8_t value) { return value == 0; }));
    }
    for (std::uint64_t tick = horizon; tick-- > 0;) {
      const auto reverse =
          reverse_alternating_oriented_ternary_parity_rail(rail, tick);
      check("prepared history reverse step valid", reverse.valid());
      rail = reverse.after;
    }
    check("prepared pulse history recovers exactly", rail == initial);
  }

  std::vector<std::int8_t> fixed{1, 0, 0, 0, 0, 0};
  for (int step = 0; step < 12; ++step) {
    fixed = step_alternating_oriented_ternary_parity_rail(fixed, 0).after;
  }
  check("fixed matching control stays in first bond",
      std::all_of(fixed.begin() + 2, fixed.end(),
          [](std::int8_t value) { return value == 0; }));

  const auto occupied = step_alternating_oriented_ternary_parity_rail(
      {1, -1, 0, 0}, 0);
  check("occupied bond exchanges without erasure",
      occupied.valid()
      && occupied.after[0] == 1
      && occupied.after[1] == 1
      && occupied.occupied_exchange_count == 1
      && occupied.reciprocal_backpressure_retained
      && occupied.no_logical_erasure
      && !occupied.universal_progress_supplied);

  const auto ready = step_alternating_oriented_ternary_parity_rail(
      {-1, 0, 0, 0}, 0);
  check("ready bond transfers and clears upstream",
      ready.valid()
      && ready.after[0] == 0
      && ready.after[1] == -1
      && ready.ready_transfer_count == 1);

  HamiltonianTernaryActuatorInput actuator_input;
  actuator_input.latch = 1;
  actuator_input.eligible = true;
  actuator_input.reference_action = 2.0;
  const auto actuator =
      evolve_hamiltonian_ternary_quarter_turn_cycle(actuator_input);
  const auto handoff = step_alternating_oriented_ternary_parity_rail(
      {actuator.logical_transfer.port_after, 0}, 0);
  check("Hamiltonian actuator composes with first spatial bond",
      actuator.valid()
      && actuator.logical_transfer.latch_after == 0
      && handoff.valid()
      && handoff.after == std::vector<std::int8_t>({0, 1}));

  check("short rail fails closed",
      step_alternating_oriented_ternary_parity_rail({}, 0).status
          == AlternatingTernaryParityRailStatus::InvalidRailLength
      && step_alternating_oriented_ternary_parity_rail({0}, 0).status
          == AlternatingTernaryParityRailStatus::InvalidRailLength);
  check("nonternary label fails closed",
      step_alternating_oriented_ternary_parity_rail({0, 2}, 0).status
          == AlternatingTernaryParityRailStatus::InvalidTernaryLabel);

  std::cout << "FTD-0874 alternating oriented ternary parity rail EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  std::cout << "prepared_pulse=ONE_EDGE_PER_GLOBAL_TICK\n";
  std::cout << "backpressure=RECIPROCAL_RETENTION_NOT_PROGRESS\n";
  std::cout << "production_gstar=OPEN\n";
  return failures == 0 ? 0 : 1;
}

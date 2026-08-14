#pragma once

/**
 * @file alternating_oriented_ternary_parity_rail.h
 * @brief FTD-0874 isolated alternating nearest-neighbour ternary rail.
 *
 * Global tick parity selects disjoint bonds. Each active bond applies the
 * FTD-0872 orientation-preserving map (a,b)->(-b,a). The finite rail retains
 * unmatched endpoints and is exactly reversible. Prepared isolated pulses
 * move one edge per tick; occupied bonds exchange labels without erasure but
 * do not establish universal progress. This is a selected reference witness,
 * not a production Voxel coupling or G* clock.
 */

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace ftd::eft {

enum class AlternatingTernaryParityRailStatus : std::uint8_t {
  Valid = 0,
  InvalidRailLength,
  InvalidTernaryLabel,
};

struct AlternatingTernaryParityRailResult {
  AlternatingTernaryParityRailStatus status =
      AlternatingTernaryParityRailStatus::InvalidRailLength;
  std::uint64_t global_tick = 0;
  std::vector<std::int8_t> before;
  std::vector<std::int8_t> after;
  std::vector<std::int8_t> recovered;
  std::vector<std::pair<std::size_t, std::size_t>> active_bonds;
  int label_norm_before = 0;
  int label_norm_after = 0;
  std::size_t nonzero_labels_before = 0;
  std::size_t nonzero_labels_after = 0;
  std::size_t ready_transfer_count = 0;
  std::size_t occupied_exchange_count = 0;
  bool inverse_step = false;
  bool disjoint_matching = false;
  bool exact_inverse_verified = false;
  bool label_norm_preserved = false;
  bool nonzero_label_count_preserved = false;
  bool sign_reversal_equivariant = false;
  bool nearest_neighbor_local = false;
  bool unmatched_endpoints_retained = false;
  bool no_logical_erasure = false;
  bool reciprocal_backpressure_retained = false;
  bool universal_progress_supplied = false;
  bool finite_horizon_only = true;
  bool existing_global_tick_parity_used = true;
  bool new_selected_type_added = false;
  bool native_intersite_hamiltonian_supplied = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const {
    return status == AlternatingTernaryParityRailStatus::Valid;
  }
};

/** Return the disjoint nearest-neighbour matching selected by tick parity. */
std::vector<std::pair<std::size_t, std::size_t>>
alternating_oriented_ternary_parity_matching(
    std::size_t rail_length,
    std::uint64_t global_tick);

/** Apply one forward parity layer (a,b)->(-b,a). */
AlternatingTernaryParityRailResult
step_alternating_oriented_ternary_parity_rail(
    const std::vector<std::int8_t>& rail,
    std::uint64_t global_tick);

/** Apply the exact inverse layer (a,b)->(b,-a) on the same tick matching. */
AlternatingTernaryParityRailResult
reverse_alternating_oriented_ternary_parity_rail(
    const std::vector<std::int8_t>& rail,
    std::uint64_t global_tick);

}  // namespace ftd::eft

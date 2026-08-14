#include "ftd/eft/alternating_oriented_ternary_parity_rail.h"

#include <algorithm>

namespace ftd::eft {
namespace {

bool ternary(std::int8_t value) {
  return value >= -1 && value <= 1;
}

int label_norm(const std::vector<std::int8_t>& rail) {
  int result = 0;
  for (const auto value : rail) {
    result += static_cast<int>(value) * static_cast<int>(value);
  }
  return result;
}

std::size_t nonzero_count(const std::vector<std::int8_t>& rail) {
  return static_cast<std::size_t>(std::count_if(
      rail.begin(), rail.end(), [](std::int8_t value) { return value != 0; }));
}

bool matching_is_disjoint(
    const std::vector<std::pair<std::size_t, std::size_t>>& bonds,
    std::size_t rail_length) {
  std::vector<bool> used(rail_length, false);
  for (const auto& [left, right] : bonds) {
    if (right != left + 1 || right >= rail_length
        || used[left] || used[right]) {
      return false;
    }
    used[left] = true;
    used[right] = true;
  }
  return true;
}

std::vector<std::int8_t> apply_layer(
    const std::vector<std::int8_t>& rail,
    const std::vector<std::pair<std::size_t, std::size_t>>& bonds,
    bool inverse) {
  auto result = rail;
  for (const auto& [left, right] : bonds) {
    if (inverse) {
      result[left] = rail[right];
      result[right] = static_cast<std::int8_t>(-rail[left]);
    } else {
      result[left] = static_cast<std::int8_t>(-rail[right]);
      result[right] = rail[left];
    }
  }
  return result;
}

AlternatingTernaryParityRailResult evaluate(
    const std::vector<std::int8_t>& rail,
    std::uint64_t global_tick,
    bool inverse) {
  AlternatingTernaryParityRailResult result;
  result.global_tick = global_tick;
  result.before = rail;
  result.inverse_step = inverse;
  result.universal_progress_supplied = false;
  result.finite_horizon_only = true;
  result.existing_global_tick_parity_used = true;
  result.new_selected_type_added = false;
  result.native_intersite_hamiltonian_supplied = false;
  result.production_coupling_supplied = false;
  result.native_gstar_synchronization_supplied = false;

  if (rail.size() < 2) {
    result.status = AlternatingTernaryParityRailStatus::InvalidRailLength;
    return result;
  }
  if (!std::all_of(rail.begin(), rail.end(), ternary)) {
    result.status = AlternatingTernaryParityRailStatus::InvalidTernaryLabel;
    return result;
  }

  result.active_bonds = alternating_oriented_ternary_parity_matching(
      rail.size(), global_tick);
  result.disjoint_matching = matching_is_disjoint(
      result.active_bonds, rail.size());
  result.after = apply_layer(rail, result.active_bonds, inverse);
  result.recovered = apply_layer(result.after, result.active_bonds, !inverse);
  result.label_norm_before = label_norm(result.before);
  result.label_norm_after = label_norm(result.after);
  result.nonzero_labels_before = nonzero_count(result.before);
  result.nonzero_labels_after = nonzero_count(result.after);

  for (const auto& [left, right] : result.active_bonds) {
    if (!inverse && rail[left] != 0 && rail[right] == 0) {
      ++result.ready_transfer_count;
    }
    if (rail[left] != 0 && rail[right] != 0) {
      ++result.occupied_exchange_count;
    }
  }

  result.exact_inverse_verified = result.recovered == result.before;
  result.label_norm_preserved =
      result.label_norm_before == result.label_norm_after;
  result.nonzero_label_count_preserved =
      result.nonzero_labels_before == result.nonzero_labels_after;
  auto reversed_input = rail;
  std::transform(
      reversed_input.begin(), reversed_input.end(), reversed_input.begin(),
      [](std::int8_t value) { return static_cast<std::int8_t>(-value); });
  const auto reversed_output = apply_layer(
      reversed_input, result.active_bonds, inverse);
  result.sign_reversal_equivariant = std::equal(
      reversed_output.begin(), reversed_output.end(), result.after.begin(),
      [](std::int8_t reversed, std::int8_t ordinary) {
        return reversed == static_cast<std::int8_t>(-ordinary);
      });
  result.nearest_neighbor_local = result.disjoint_matching;
  result.unmatched_endpoints_retained = true;
  result.no_logical_erasure = result.exact_inverse_verified
      && result.nonzero_label_count_preserved;
  result.reciprocal_backpressure_retained =
      result.occupied_exchange_count > 0 && result.no_logical_erasure;
  result.status = AlternatingTernaryParityRailStatus::Valid;
  return result;
}

}  // namespace

std::vector<std::pair<std::size_t, std::size_t>>
alternating_oriented_ternary_parity_matching(
    std::size_t rail_length,
    std::uint64_t global_tick) {
  std::vector<std::pair<std::size_t, std::size_t>> bonds;
  const std::size_t start = static_cast<std::size_t>(global_tick & 1U);
  for (std::size_t left = start; left + 1 < rail_length; left += 2) {
    bonds.emplace_back(left, left + 1);
  }
  return bonds;
}

AlternatingTernaryParityRailResult
step_alternating_oriented_ternary_parity_rail(
    const std::vector<std::int8_t>& rail,
    std::uint64_t global_tick) {
  return evaluate(rail, global_tick, false);
}

AlternatingTernaryParityRailResult
reverse_alternating_oriented_ternary_parity_rail(
    const std::vector<std::int8_t>& rail,
    std::uint64_t global_tick) {
  return evaluate(rail, global_tick, true);
}

}  // namespace ftd::eft

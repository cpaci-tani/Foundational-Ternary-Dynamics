#include "ftd/eft/finite_memory_reversible_lift.h"

#include <cmath>
#include <limits>

namespace ftd::eft {

FiniteLiftCount analyze_finite_reversible_lift(
    std::uint64_t preimage_multiplicity,
    std::uint64_t hidden_capacity) {
  FiniteLiftCount result;
  result.preimage_multiplicity = preimage_multiplicity;
  result.hidden_capacity = hidden_capacity;
  if (preimage_multiplicity == 0 || hidden_capacity == 0
      || preimage_multiplicity
          > std::numeric_limits<std::uint64_t>::max() / hidden_capacity) {
    return result;
  }
  result.restricted_domain = preimage_multiplicity * hidden_capacity;
  result.restricted_codomain = hidden_capacity;
  result.cardinality_deficit = result.restricted_domain
      - result.restricted_codomain;
  result.injective_lift_possible = result.restricted_domain
      <= result.restricted_codomain;
  result.valid = true;
  return result;
}

HistoryPushResult push_history_branch(
    std::uint64_t history,
    std::uint64_t branch,
    std::uint64_t radix) {
  HistoryPushResult result;
  result.before = history;
  result.branch = branch;
  result.radix = radix;
  if (radix < 2 || branch >= radix
      || history > (std::numeric_limits<std::uint64_t>::max() - branch)
          / radix) {
    return result;
  }
  result.after = radix * history + branch;
  result.valid = true;
  return result;
}

HistoryPopResult pop_history_branch(
    std::uint64_t history,
    std::uint64_t radix) {
  HistoryPopResult result;
  result.before = history;
  result.radix = radix;
  if (radix < 2) return result;
  result.branch = history % radix;
  result.after = history / radix;
  result.valid = true;
  return result;
}

std::uint64_t minimum_history_bits(
    std::uint64_t radix,
    std::uint64_t events) {
  if (radix < 2 || events == 0) return 0;
  const long double bits = static_cast<long double>(events)
      * std::log2(static_cast<long double>(radix));
  if (!std::isfinite(bits)
      || bits > static_cast<long double>(
          std::numeric_limits<std::uint64_t>::max())) {
    return std::numeric_limits<std::uint64_t>::max();
  }
  return static_cast<std::uint64_t>(std::ceil(bits - 1e-18L));
}

}  // namespace ftd::eft

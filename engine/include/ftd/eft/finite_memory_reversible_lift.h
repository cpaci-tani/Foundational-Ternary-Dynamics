#pragma once
/**
 * @file finite_memory_reversible_lift.h
 * @brief Finite-fiber obstruction and unbounded-history control for lifting
 *        a many-to-one raw matter map (FTD-0499).
 */

#include <cstdint>

namespace ftd::eft {

struct FiniteLiftCount {
  bool valid = false;
  bool injective_lift_possible = false;
  std::uint64_t preimage_multiplicity = 0;
  std::uint64_t hidden_capacity = 0;
  std::uint64_t restricted_domain = 0;
  std::uint64_t restricted_codomain = 0;
  std::uint64_t cardinality_deficit = 0;
};

/// Count the restricted domain/codomain of a projection-preserving lift over
/// one raw output with m preimages and a finite hidden fiber of size H.
FiniteLiftCount analyze_finite_reversible_lift(
    std::uint64_t preimage_multiplicity,
    std::uint64_t hidden_capacity);

struct HistoryPushResult {
  bool valid = false;
  std::uint64_t before = 0;
  std::uint64_t after = 0;
  std::uint64_t branch = 0;
  std::uint64_t radix = 0;
};

struct HistoryPopResult {
  bool valid = false;
  std::uint64_t before = 0;
  std::uint64_t after = 0;
  std::uint64_t branch = 0;
  std::uint64_t radix = 0;
};

/// Push one branch digit: after=radix*before+branch. Fails on invalid branch
/// or uint64 overflow.
HistoryPushResult push_history_branch(
    std::uint64_t history,
    std::uint64_t branch,
    std::uint64_t radix);

/// Pop one branch digit, exactly inverting push_history_branch.
HistoryPopResult pop_history_branch(
    std::uint64_t history,
    std::uint64_t radix);

/// Information capacity required to retain `events` independent radix-way
/// merges. Exact for power-of-two radices; otherwise returns ceil(log2()).
std::uint64_t minimum_history_bits(
    std::uint64_t radix,
    std::uint64_t events);

}  // namespace ftd::eft

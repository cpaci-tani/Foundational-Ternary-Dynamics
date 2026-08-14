#pragma once

/**
 * @file oriented_ternary_quarter_turn.h
 * @brief FTD-0872 isolated reversible ternary source/port quarter-turn.
 *
 * The all-domain forward map is R(s,o)=(-o,s); its inverse is
 * R^-1(s,o)=(o,-s). On a ready output port, forward transfer sends
 * (s,0)->(0,s). A nonempty port is exchanged reversibly rather than hidden
 * behind a noninjective fail-closed wrapper.
 */

#include <cstdint>

namespace ftd::eft {

enum class TernaryQuarterTurnStatus : std::uint8_t {
  Valid = 0,
  InvalidLatch,
  InvalidPort,
};

enum class TernaryQuarterTurnOrientation : std::uint8_t {
  Forward = 0,
  Reverse,
};

struct TernaryQuarterTurnInput {
  std::int8_t latch = 0;
  std::int8_t port = 0;
  bool eligible = false;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
};

struct TernaryQuarterTurnResult {
  TernaryQuarterTurnStatus status =
      TernaryQuarterTurnStatus::InvalidLatch;
  std::int8_t latch_before = 0;
  std::int8_t port_before = 0;
  std::int8_t latch_after = 0;
  std::int8_t port_after = 0;
  std::int8_t recovered_latch = 0;
  std::int8_t recovered_port = 0;
  int label_norm_before = 0;
  int label_norm_after = 0;
  int oriented_area = 0;
  bool eligible = false;
  TernaryQuarterTurnOrientation orientation =
      TernaryQuarterTurnOrientation::Forward;
  bool exact_inverse_verified = false;
  bool label_norm_preserved = false;
  bool sign_reversal_equivariant = false;
  bool unique_oriented_isometry = false;
  bool no_logical_erasure = false;
  bool ready_emission = false;
  bool reciprocal_absorption = false;
  bool nonempty_port_reciprocal_exchange = false;
  bool naive_empty_port_fail_closed_noninjective = false;
  bool physical_energy_scale_supplied = false;
  bool controller_work_ledger_supplied = false;
  bool protected_cubic_transport_supplied = false;
  bool production_coupling_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const { return status == TernaryQuarterTurnStatus::Valid; }
};

/** Apply identity when ineligible, otherwise the selected quarter-turn. */
TernaryQuarterTurnResult apply_oriented_ternary_quarter_turn(
    const TernaryQuarterTurnInput& input);

}  // namespace ftd::eft


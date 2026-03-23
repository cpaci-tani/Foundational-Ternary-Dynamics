#pragma once
// tritium/logic.h — Kleene strong three-valued logic on packed TritWord64
//
// Maps balanced ternary to truth values: Neg = False, Zero = Unknown, Pos = True
// Logic ops: NOT, AND (min), OR (max), CONSENSUS (agree or unknown)

#include "trit.h"

namespace tritium {

// ============================================================================
// NOT: -1 -> +1, 0 -> 0, +1 -> -1 (same as arithmetic negate)
// ============================================================================

inline TritWord64 ternary_not(TritWord64 a) {
    uint64_t low  = a.bits & TritWord64::EVEN_BITS;
    uint64_t high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t nonzero_mask = (low | high);
    nonzero_mask |= (nonzero_mask << 1);
    TritWord64 result;
    result.bits = a.bits ^ (TritWord64::ODD_BITS & nonzero_mask);
    return result;
}

// ============================================================================
// AND: min(a, b) in Kleene logic
// ============================================================================
// Truth table (ordered -1 < 0 < +1):
//   AND | -1   0  +1
//   -1  | -1  -1  -1
//    0  | -1   0   0
//   +1  | -1   0  +1

inline TritWord64 ternary_and(TritWord64 a, TritWord64 b) {
    // Decompose
    uint64_t a_low = a.bits & TritWord64::EVEN_BITS;
    uint64_t a_high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t b_low = b.bits & TritWord64::EVEN_BITS;
    uint64_t b_high = (b.bits >> 1) & TritWord64::EVEN_BITS;

    // Detect specific values
    uint64_t a_neg = a_high & a_low;    // 11 = Neg
    uint64_t b_neg = b_high & b_low;
    uint64_t a_pos = a_low & ~a_high;   // 01 = Pos
    uint64_t b_pos = b_low & ~b_high;
    uint64_t a_nz = a_low | a_high;
    uint64_t b_nz = b_low | b_high;

    // min logic:
    // If either is Neg -> Neg
    uint64_t r_neg = a_neg | b_neg;
    // If both are Pos -> Pos
    uint64_t r_pos = a_pos & b_pos;
    // Otherwise -> Zero (already default)

    TritWord64 result;
    result.bits = r_pos | (r_neg * 3);
    return result;
}

// ============================================================================
// OR: max(a, b) in Kleene logic
// ============================================================================
// Truth table:
//   OR  | -1   0  +1
//   -1  | -1   0  +1
//    0  |  0   0  +1
//   +1  | +1  +1  +1

inline TritWord64 ternary_or(TritWord64 a, TritWord64 b) {
    uint64_t a_low = a.bits & TritWord64::EVEN_BITS;
    uint64_t a_high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t b_low = b.bits & TritWord64::EVEN_BITS;
    uint64_t b_high = (b.bits >> 1) & TritWord64::EVEN_BITS;

    uint64_t a_neg = a_high & a_low;
    uint64_t b_neg = b_high & b_low;
    uint64_t a_pos = a_low & ~a_high;
    uint64_t b_pos = b_low & ~b_high;

    // max logic:
    // If either is Pos -> Pos
    uint64_t r_pos = a_pos | b_pos;
    // If both are Neg -> Neg
    uint64_t r_neg = a_neg & b_neg;
    // Otherwise -> Zero

    TritWord64 result;
    result.bits = r_pos | (r_neg * 3);
    return result;
}

// ============================================================================
// CONSENSUS: returns a where a == b, else Zero
// ============================================================================
// Agreement filter — useful for voting, error correction, confidence masking.

inline TritWord64 consensus(TritWord64 a, TritWord64 b) {
    // Where both words have the same 2-bit pair, keep it; otherwise zero
    uint64_t same = ~(a.bits ^ b.bits); // all-1s where bits match
    // Both bits of a pair must match for the trit to agree
    uint64_t same_low  = same & TritWord64::EVEN_BITS;
    uint64_t same_high = (same >> 1) & TritWord64::EVEN_BITS;
    uint64_t agree = same_low & same_high; // 1 in even position where both bits match
    uint64_t agree_mask = agree | (agree << 1); // expand to full pair

    TritWord64 result;
    result.bits = a.bits & agree_mask;
    return result;
}

} // namespace tritium

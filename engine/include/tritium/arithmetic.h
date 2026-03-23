#pragma once
// tritium/arithmetic.h — Balanced ternary arithmetic on packed TritWord64
//
// All operations process 32 trits in parallel via bitwise ops on uint64_t.

#include "trit.h"

namespace tritium {

// ============================================================================
// Negate: flip sign of every trit
// ============================================================================
// Encoding:  00 (Zero) -> 00,  01 (Pos) -> 11 (Neg),  11 (Neg) -> 01 (Pos)
// XOR with 0xAAAA... flips the high bit:
//   00 ^ 10 = 10 (invalid), 01 ^ 10 = 11 (Neg, correct), 11 ^ 10 = 01 (Pos, correct)
// So we XOR with ODD_BITS, but only for nonzero trits.

inline TritWord64 negate(TritWord64 a) {
    // Identify nonzero trits: a trit is nonzero if either bit is set
    uint64_t low  = a.bits & TritWord64::EVEN_BITS;
    uint64_t high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t nonzero_mask = (low | high); // 1 in low bit position for each nonzero trit
    nonzero_mask |= (nonzero_mask << 1);  // expand to cover both bits of each pair

    // XOR high bits of nonzero trits: swaps 01 <-> 11
    TritWord64 result;
    result.bits = a.bits ^ (TritWord64::ODD_BITS & nonzero_mask);
    return result;
}

// ============================================================================
// Absolute value: -1 -> +1, 0 -> 0, +1 -> +1
// ============================================================================
// Just keep the low bit, clear the high bit for each pair.
// 00 -> 00, 01 -> 01, 11 -> 01

inline TritWord64 abs(TritWord64 a) {
    uint64_t low  = a.bits & TritWord64::EVEN_BITS;
    uint64_t high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    // nonzero trits -> set low bit
    TritWord64 result;
    result.bits = low | high; // keeps only EVEN_BITS positions set
    return result;
}

// ============================================================================
// Element-wise multiply: trit * trit = {-1,0,+1} * {-1,0,+1}
// ============================================================================
// Rules: 0*x = 0, x*0 = 0, (+1)*(+1) = +1, (-1)*(-1) = +1,
//        (+1)*(-1) = -1, (-1)*(+1) = -1
//
// If either is zero, result is zero.
// Otherwise, result sign = XOR of the sign bits.
// Result magnitude is always 1 (for nonzero inputs).

inline TritWord64 multiply(TritWord64 a, TritWord64 b) {
    // Detect nonzero trits
    uint64_t a_low  = a.bits & TritWord64::EVEN_BITS;
    uint64_t a_high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t a_nz   = a_low | a_high;  // 1 per nonzero trit (in even positions)

    uint64_t b_low  = b.bits & TritWord64::EVEN_BITS;
    uint64_t b_high = (b.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t b_nz   = b_low | b_high;

    // Both nonzero?
    uint64_t both_nz = a_nz & b_nz;  // 1 in even position where both are nonzero

    // Sign bits (high bit of each pair): Neg=1, Pos=0 (since Neg=11, Pos=01)
    // For nonzero trits: high bit IS the sign (1=negative, 0=positive)
    uint64_t a_sign = a_high & a_nz;  // only look at nonzero trits
    uint64_t b_sign = b_high & b_nz;
    uint64_t result_sign = (a_sign ^ b_sign) & both_nz;  // XOR: same signs -> 0 (Pos), diff -> 1 (Neg)

    // Build result: low bit is 1 where both nonzero, high bit is result_sign
    TritWord64 result;
    result.bits = both_nz | (result_sign << 1);
    return result;
}

// ============================================================================
// Element-wise addition (no carry): trit + trit, clamped to {-1,0,+1}
// ============================================================================
// This is "saturating" add for single-trit precision.
// True balanced ternary addition with carry is below.
//
// Truth table (a+b clamped):
//   -1 + -1 = -1 (saturate)   -1 + 0 = -1    -1 + 1 = 0
//    0 + -1 = -1               0 + 0 =  0      0 + 1 = 1
//   +1 + -1 = 0               +1 + 0 = +1     +1 + 1 = +1 (saturate)

inline TritWord64 add_saturate(TritWord64 a, TritWord64 b) {
    // Convert to signed representation for parallel add:
    // Map: 00->0, 01->+1, 11->-1
    // We compute per-trit integer sum and re-encode.

    // Decompose into sign and magnitude
    uint64_t a_low = a.bits & TritWord64::EVEN_BITS;
    uint64_t a_high = (a.bits >> 1) & TritWord64::EVEN_BITS;
    uint64_t b_low = b.bits & TritWord64::EVEN_BITS;
    uint64_t b_high = (b.bits >> 1) & TritWord64::EVEN_BITS;

    // Nonzero masks
    uint64_t a_nz = a_low | a_high;
    uint64_t b_nz = b_low | b_high;

    // Signs (1 = negative for nonzero trits)
    uint64_t a_neg = a_high & a_low; // 11 pattern = Neg
    uint64_t b_neg = b_high & b_low;
    uint64_t a_pos = a_low & ~a_high; // 01 pattern = Pos
    uint64_t b_pos = b_low & ~b_high;

    // Cases where they cancel: Pos + Neg or Neg + Pos -> Zero
    uint64_t cancel = (a_pos & b_neg) | (a_neg & b_pos);

    // Cases where both are same sign -> saturate to that sign
    uint64_t both_pos = a_pos & b_pos;
    uint64_t both_neg = a_neg & b_neg;

    // Cases where one is zero -> take the other
    uint64_t a_zero = ~a_nz & TritWord64::EVEN_BITS;
    uint64_t b_zero = ~b_nz & TritWord64::EVEN_BITS;

    // Result positives: both_pos, or (a_pos & b_zero), or (b_pos & a_zero)
    uint64_t r_pos = both_pos | (a_pos & b_zero) | (b_pos & a_zero);
    // Result negatives: both_neg, or (a_neg & b_zero), or (b_neg & a_zero)
    uint64_t r_neg = both_neg | (a_neg & b_zero) | (b_neg & a_zero);
    // Cancellations -> zero (no bits set)

    // Encode: Pos = 01, Neg = 11
    TritWord64 result;
    result.bits = r_pos | (r_neg * 3); // r_neg * 3 = r_neg | (r_neg << 1)
    return result;
}

// ============================================================================
// Element-wise addition with carry (full balanced ternary add)
// ============================================================================
// Returns {sum_word, carry_word} where carry propagates one position left.
// In balanced ternary: (-1)+(-1) = +1 carry -1, (+1)+(+1) = -1 carry +1

struct TritAddResult {
    TritWord64 sum;
    TritWord64 carry;
};

inline TritAddResult add_with_carry(TritWord64 a, TritWord64 b) {
    // Process each trit individually via lookup
    // This is the simple reference implementation; SIMD version in simd.h
    TritAddResult result;
    result.sum.bits = 0;
    result.carry.bits = 0;

    for (int i = 0; i < TritWord64::CAPACITY; ++i) {
        int va = to_int(a.get(i));
        int vb = to_int(b.get(i));
        int sum = va + vb;

        Trit s, c;
        if (sum >= 2)       { s = to_trit(sum - 3); c = Trit::Pos; }
        else if (sum <= -2) { s = to_trit(sum + 3); c = Trit::Neg; }
        else                { s = to_trit(sum);     c = Trit::Zero; }

        result.sum.set(i, s);
        if (i + 1 < TritWord64::CAPACITY)
            result.carry.set(i + 1, c); // carry goes to next position
    }

    return result;
}

} // namespace tritium

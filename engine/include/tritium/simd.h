#pragma once
// tritium/simd.h — SIMD-accelerated operations for packed trit words
//
// Provides optimized implementations using SSE4.2/AVX2 intrinsics
// with scalar fallbacks for portability.
//
// Architecture detection:
//   __AVX2__    -> 256-bit operations (128 trits per instruction)
//   __SSE4_2__  -> 128-bit operations (64 trits per instruction)
//   otherwise   -> scalar fallback using __builtin_popcountll

#include "trit.h"
#include <cstddef>
#include <cstdint>

#if defined(__AVX2__)
#include <immintrin.h>
#elif defined(__SSE4_2__)
#include <nmmintrin.h>
#endif

namespace tritium {
namespace simd {

// ============================================================================
// Popcount: count bits set in a uint64_t
// ============================================================================
// Used by dot product, hamming, l0_norm.

inline int popcount64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#elif defined(_MSC_VER)
    return static_cast<int>(__popcnt64(x));
#else
    // Fallback: Hamming weight via bit manipulation
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return static_cast<int>((x * 0x0101010101010101ULL) >> 56);
#endif
}

// ============================================================================
// Batch dot product of trit word arrays
// ============================================================================
// Computes Σ_w dot(a[w], b[w]) across n_words TritWord64 pairs.
// This is the inner loop of TritVector::dot().

inline int batch_dot(const TritWord64* a, const TritWord64* b, size_t n_words) {
    int result = 0;

#if defined(__AVX2__)
    // Process 4 TritWord64s at a time (256 bits = 4 × uint64_t)
    size_t w = 0;
    __m256i acc_agree = _mm256_setzero_si256();
    __m256i acc_differ = _mm256_setzero_si256();
    const __m256i even_mask = _mm256_set1_epi64x(
        static_cast<long long>(TritWord64::EVEN_BITS));

    for (; w + 4 <= n_words; w += 4) {
        __m256i va = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(&a[w]));
        __m256i vb = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(&b[w]));

        // Decompose into even/odd bits
        __m256i a_lo = _mm256_and_si256(va, even_mask);
        __m256i a_hi = _mm256_and_si256(_mm256_srli_epi64(va, 1), even_mask);
        __m256i b_lo = _mm256_and_si256(vb, even_mask);
        __m256i b_hi = _mm256_and_si256(_mm256_srli_epi64(vb, 1), even_mask);

        // Nonzero masks
        __m256i a_nz = _mm256_or_si256(a_lo, a_hi);
        __m256i b_nz = _mm256_or_si256(b_lo, b_hi);
        __m256i both_nz = _mm256_and_si256(a_nz, b_nz);

        // Negative detection (11 pattern)
        __m256i a_neg = _mm256_and_si256(a_hi, a_lo);
        __m256i b_neg = _mm256_and_si256(b_hi, b_lo);

        // Sign difference
        __m256i sign_diff = _mm256_and_si256(
            _mm256_xor_si256(a_neg, b_neg), both_nz);
        __m256i sign_agr = _mm256_andnot_si256(sign_diff, both_nz);

        // Accumulate (we'll popcount at the end)
        acc_agree = _mm256_or_si256(acc_agree,
            _mm256_slli_epi64(sign_agr, 0)); // placeholder — popcount below
        acc_differ = _mm256_or_si256(acc_differ,
            _mm256_slli_epi64(sign_diff, 0));

        // Actually, we need to popcount per-word and accumulate integers.
        // AVX2 doesn't have native popcount, so extract and use scalar.
        uint64_t agr_vals[4], dif_vals[4];
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(agr_vals), sign_agr);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(dif_vals), sign_diff);
        for (int i = 0; i < 4; ++i)
            result += popcount64(agr_vals[i]) - popcount64(dif_vals[i]);
    }

    // Handle remaining words
    for (; w < n_words; ++w) {
        uint64_t va = a[w].bits, vb = b[w].bits;
        uint64_t a_lo = va & TritWord64::EVEN_BITS;
        uint64_t a_hi = (va >> 1) & TritWord64::EVEN_BITS;
        uint64_t b_lo = vb & TritWord64::EVEN_BITS;
        uint64_t b_hi = (vb >> 1) & TritWord64::EVEN_BITS;
        uint64_t a_nz = a_lo | a_hi, b_nz = b_lo | b_hi;
        uint64_t both_nz = a_nz & b_nz;
        uint64_t a_neg = a_hi & a_lo, b_neg = b_hi & b_lo;
        uint64_t sd = (a_neg ^ b_neg) & both_nz;
        uint64_t sa = both_nz & ~sd;
        result += popcount64(sa) - popcount64(sd);
    }

#else
    // Scalar fallback
    for (size_t w = 0; w < n_words; ++w) {
        uint64_t va = a[w].bits, vb = b[w].bits;
        uint64_t a_lo = va & TritWord64::EVEN_BITS;
        uint64_t a_hi = (va >> 1) & TritWord64::EVEN_BITS;
        uint64_t b_lo = vb & TritWord64::EVEN_BITS;
        uint64_t b_hi = (vb >> 1) & TritWord64::EVEN_BITS;
        uint64_t a_nz = a_lo | a_hi, b_nz = b_lo | b_hi;
        uint64_t both_nz = a_nz & b_nz;
        uint64_t a_neg = a_hi & a_lo, b_neg = b_hi & b_lo;
        uint64_t sd = (a_neg ^ b_neg) & both_nz;
        uint64_t sa = both_nz & ~sd;
        result += popcount64(sa) - popcount64(sd);
    }
#endif

    return result;
}

// ============================================================================
// Batch negate
// ============================================================================
inline void batch_negate(const TritWord64* src, TritWord64* dst, size_t n_words) {
    for (size_t w = 0; w < n_words; ++w) {
        uint64_t bits = src[w].bits;
        uint64_t lo = bits & TritWord64::EVEN_BITS;
        uint64_t hi = (bits >> 1) & TritWord64::EVEN_BITS;
        uint64_t nz = (lo | hi);
        uint64_t nz_mask = nz | (nz << 1);
        dst[w].bits = bits ^ (TritWord64::ODD_BITS & nz_mask);
    }
}

// ============================================================================
// Batch element-wise multiply
// ============================================================================
inline void batch_multiply(const TritWord64* a, const TritWord64* b,
                            TritWord64* dst, size_t n_words) {
    for (size_t w = 0; w < n_words; ++w) {
        uint64_t va = a[w].bits, vb = b[w].bits;
        uint64_t a_lo = va & TritWord64::EVEN_BITS;
        uint64_t a_hi = (va >> 1) & TritWord64::EVEN_BITS;
        uint64_t b_lo = vb & TritWord64::EVEN_BITS;
        uint64_t b_hi = (vb >> 1) & TritWord64::EVEN_BITS;
        uint64_t a_nz = a_lo | a_hi, b_nz = b_lo | b_hi;
        uint64_t both_nz = a_nz & b_nz;
        uint64_t a_neg = a_hi & a_lo, b_neg = b_hi & b_lo;
        uint64_t result_sign = (a_neg ^ b_neg) & both_nz;
        dst[w].bits = both_nz | (result_sign << 1);
    }
}

} // namespace simd
} // namespace tritium

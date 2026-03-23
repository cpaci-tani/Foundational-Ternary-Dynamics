#pragma once
// tritium/threshold.h — Convert continuous values to/from balanced ternary
//
// Hard quantization (deterministic) and stochastic quantization (FTD Born rule).

#include "trit.h"
#include <cmath>
#include <random>
#include <cstddef>

namespace tritium {

// ============================================================================
// Hard quantization: x > threshold -> Pos, x < -threshold -> Neg, else Zero
// ============================================================================

inline Trit hard_quantize(double x, double threshold) {
    if (x > threshold)  return Trit::Pos;
    if (x < -threshold) return Trit::Neg;
    return Trit::Zero;
}

// Batch: quantize n doubles into TritWord64 array
inline void batch_quantize(const double* src, TritWord64* dst, size_t n, double threshold) {
    size_t n_words = (n + TritWord64::CAPACITY - 1) / TritWord64::CAPACITY;
    for (size_t w = 0; w < n_words; ++w)
        dst[w].bits = 0;

    for (size_t i = 0; i < n; ++i) {
        Trit t = hard_quantize(src[i], threshold);
        size_t word_i = i / TritWord64::CAPACITY;
        int bit_i = static_cast<int>(i % TritWord64::CAPACITY);
        dst[word_i].set(bit_i, t);
    }
}

// ============================================================================
// Stochastic quantization (FTD-style Born rule)
// ============================================================================
// Probability of manifestation: p = 1 - exp(-|x| / k_b)
// If manifested, sign follows sign(x).

template<typename RNG>
inline Trit stochastic_quantize(double x, double k_b, RNG& rng) {
    double abs_x = std::abs(x);
    if (abs_x < 1e-15) return Trit::Zero;

    double p = 1.0 - std::exp(-abs_x / k_b);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < p) {
        return (x > 0) ? Trit::Pos : Trit::Neg;
    }
    return Trit::Zero;
}

// Batch stochastic quantization
template<typename RNG>
inline void batch_stochastic_quantize(const double* src, TritWord64* dst,
                                       size_t n, double k_b, RNG& rng) {
    size_t n_words = (n + TritWord64::CAPACITY - 1) / TritWord64::CAPACITY;
    for (size_t w = 0; w < n_words; ++w)
        dst[w].bits = 0;

    for (size_t i = 0; i < n; ++i) {
        Trit t = stochastic_quantize(src[i], k_b, rng);
        size_t word_i = i / TritWord64::CAPACITY;
        int bit_i = static_cast<int>(i % TritWord64::CAPACITY);
        dst[word_i].set(bit_i, t);
    }
}

// ============================================================================
// Dequantize: trit -> double (for reconstruction)
// ============================================================================

inline double dequantize(Trit t, double scale = 1.0) {
    return static_cast<double>(static_cast<int8_t>(t)) * scale;
}

// Batch dequantize
inline void batch_dequantize(const TritWord64* src, double* dst,
                              size_t n, double scale = 1.0) {
    for (size_t i = 0; i < n; ++i) {
        size_t word_i = i / TritWord64::CAPACITY;
        int bit_i = static_cast<int>(i % TritWord64::CAPACITY);
        dst[i] = dequantize(src[word_i].get(bit_i), scale);
    }
}

} // namespace tritium

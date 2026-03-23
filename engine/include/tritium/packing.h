#pragma once
// tritium/packing.h — Conversion between compute (TritWord64) and storage (TritPack) formats
//
// Uses precomputed lookup tables for zero-branch encode/decode.

#include "trit.h"
#include <cstddef>

namespace tritium {

// ============================================================================
// Lookup tables (constexpr, compiled into .rodata)
// ============================================================================

namespace detail {

// Decode table: byte value -> array of 5 trits
struct DecodeEntry { Trit trits[5]; };

constexpr DecodeEntry make_decode_entry(uint8_t byte) {
    DecodeEntry e{};
    int v = byte;
    for (int i = 0; i < 5; ++i) {
        int r = v % 3;
        e.trits[i] = static_cast<Trit>(r - 1);
        v /= 3;
    }
    return e;
}

// Generate all 243 decode entries at compile time
struct DecodeLUT {
    DecodeEntry entries[243];
    constexpr DecodeLUT() : entries{} {
        for (int i = 0; i < 243; ++i)
            entries[i] = make_decode_entry(static_cast<uint8_t>(i));
    }
};

inline constexpr DecodeLUT DECODE_LUT{};

// Encode: convert 5 trit values to a byte
// Each trit is offset by +1 to get {0,1,2}, then packed as mixed-radix
inline uint8_t encode_5trits(Trit t0, Trit t1, Trit t2, Trit t3, Trit t4) {
    int v = (static_cast<int>(t0) + 1)
          + (static_cast<int>(t1) + 1) * 3
          + (static_cast<int>(t2) + 1) * 9
          + (static_cast<int>(t3) + 1) * 27
          + (static_cast<int>(t4) + 1) * 81;
    return static_cast<uint8_t>(v);
}

} // namespace detail

// ============================================================================
// Decode: TritPack byte -> individual trits (via LUT)
// ============================================================================

inline void decode_pack(uint8_t byte, Trit out[5]) {
    const auto& e = detail::DECODE_LUT.entries[byte];
    for (int i = 0; i < 5; ++i)
        out[i] = e.trits[i];
}

// ============================================================================
// Batch conversion: TritWord64[] <-> TritPack[]
// ============================================================================

// Convert n_trits from compute format (TritWord64 array) to storage format (TritPack array).
// src must contain ceil(n_trits/32) TritWord64s.
// dst must have space for ceil(n_trits/5) TritPacks.
inline void pack(const TritWord64* src, TritPack* dst, size_t n_trits) {
    size_t trit_idx = 0;
    size_t pack_idx = 0;

    while (trit_idx + 5 <= n_trits) {
        // Extract 5 trits from the compute words
        Trit t[5];
        for (int k = 0; k < 5; ++k) {
            size_t ti = trit_idx + k;
            t[k] = src[ti / TritWord64::CAPACITY].get(static_cast<int>(ti % TritWord64::CAPACITY));
        }
        dst[pack_idx].byte = detail::encode_5trits(t[0], t[1], t[2], t[3], t[4]);
        pack_idx++;
        trit_idx += 5;
    }

    // Handle remaining trits (< 5)
    if (trit_idx < n_trits) {
        Trit t[5] = {Trit::Zero, Trit::Zero, Trit::Zero, Trit::Zero, Trit::Zero};
        for (size_t k = 0; trit_idx + k < n_trits; ++k) {
            size_t ti = trit_idx + k;
            t[k] = src[ti / TritWord64::CAPACITY].get(static_cast<int>(ti % TritWord64::CAPACITY));
        }
        dst[pack_idx].byte = detail::encode_5trits(t[0], t[1], t[2], t[3], t[4]);
    }
}

// Convert n_trits from storage format (TritPack array) to compute format (TritWord64 array).
// src must contain ceil(n_trits/5) TritPacks.
// dst must have space for ceil(n_trits/32) TritWord64s (will be zeroed first).
inline void unpack(const TritPack* src, TritWord64* dst, size_t n_trits) {
    // Zero all destination words
    size_t n_words = (n_trits + TritWord64::CAPACITY - 1) / TritWord64::CAPACITY;
    for (size_t i = 0; i < n_words; ++i)
        dst[i].bits = 0;

    size_t trit_idx = 0;
    size_t pack_idx = 0;
    size_t n_packs = (n_trits + TritPack::CAPACITY - 1) / TritPack::CAPACITY;

    while (pack_idx < n_packs) {
        Trit t[5];
        decode_pack(src[pack_idx].byte, t);

        for (int k = 0; k < 5 && trit_idx < n_trits; ++k, ++trit_idx) {
            size_t word_i = trit_idx / TritWord64::CAPACITY;
            int bit_i = static_cast<int>(trit_idx % TritWord64::CAPACITY);
            dst[word_i].set(bit_i, t[k]);
        }
        pack_idx++;
    }
}

// ============================================================================
// Utility: required buffer sizes
// ============================================================================

inline size_t packed_size(size_t n_trits) {
    return (n_trits + TritPack::CAPACITY - 1) / TritPack::CAPACITY;
}

inline size_t word_count(size_t n_trits) {
    return (n_trits + TritWord64::CAPACITY - 1) / TritWord64::CAPACITY;
}

} // namespace tritium

#pragma once
// [SELECTION] Finite research candidates only. No production physics adoption,
// continuum temperature, or gas recovery is claimed. The compile-time v2 law
// has an independently generated cubic-equivariant pair involution; v1 does not.
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace ftd::thermal {

#if defined(FTD_THERMAL_EQUIVARIANT)
inline constexpr char LAW_ID[] = "phi-thermal-equivariant-candidate-2";
#else
inline constexpr char LAW_ID[] = "phi-thermal-pair-candidate-1";
#endif
inline constexpr char ENCODING_ID[] = "ftd.thermal.checkpoint-le64-bank6-v1";
inline constexpr std::uint32_t SCHEMA_VERSION = 1;
inline constexpr std::size_t CHANNELS = 343;
inline constexpr std::size_t WORDS_PER_SITE = 6;
inline constexpr std::size_t PAIRS = CHANNELS * (CHANNELS - 1) / 2;
inline constexpr std::size_t HEADER_BYTES = 96;
inline constexpr std::size_t BYTES_PER_SITE = 48;
inline constexpr std::size_t CHECKSUM_BYTES = 32;
inline constexpr std::uint64_t LAST_WORD_MASK = (std::uint64_t{1} << 23) - 1;

using Bank = std::array<std::uint64_t, WORDS_PER_SITE>;
using Velocity = std::array<std::int8_t, 3>;
using Pair = std::array<std::uint16_t, 2>;

// The sole dynamical record. Spatial index is x + L * (y + L * z).
// A microtick is one real radius-one tick; phase is never a separate clock.
struct Records {
    std::uint32_t L = 0;
    std::uint64_t microtick = 0;
    std::vector<Bank> bank;
    explicit Records(std::uint32_t size = 0);
    unsigned phase() const noexcept { return unsigned(microtick % 4); }
};

// Transaction output only: no hidden clock, retained physical state, or owner.
struct Scratch {
    std::vector<Bank> output;
};

// Exact integer observations. P and E2 use integer channel labels v, not v/4.
// Actual cycle displacement is v over four microticks. These are not Kelvin,
// SI energy, a fitted equation of state, or a temperature readout.
struct Totals {
    std::uint64_t N = 0;
    std::uint64_t E2 = 0;
    std::array<std::int64_t, 3> P{};
};

Velocity velocity(std::size_t channel);
std::size_t channel_index(int vx, int vy, int vz);
bool occupied(const Bank& bank, std::size_t channel);
void set_occupied(Bank& bank, std::size_t channel, bool value = true);
std::int8_t manifestation(const Bank& bank);
Pair collision_successor(std::size_t a, std::size_t b);
// SHA256 over every ascending input pair's LE uint16 (a,b,next_a,next_b).
std::array<std::uint8_t, 32> collision_table_hash();

void validate(const Records& records);
Totals totals(const Records& records);
// Strong exception guarantee for records. Scratch is disposable on rejection.
void step(Records& records, Scratch& scratch);
void advance(Records& records, Scratch& scratch, std::uint64_t microticks);

// Canonical LE checkpoint:
// 0: magic FTDTH01\0; 8: schema u32; 12: L u32; 16: microtick u64;
// 24: SHA256(LAW_ID); 56: SHA256(ENCODING_ID); 88: payload bytes u64;
// 96: x-fast site-major banks, six LE u64 words/site; final 32 bytes:
// SHA256 of the complete header and payload. No padding or scratch is encoded.
std::vector<std::uint8_t> encode(const Records& records);
Records decode(const std::vector<std::uint8_t>& bytes);

}  // namespace ftd::thermal

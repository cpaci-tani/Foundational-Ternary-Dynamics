#include "finite_thermal.h"
#if defined(FTD_THERMAL_EQUIVARIANT)
#include "equivariant_table.h"
#endif
#include "../hydro/hydro_sha256.h"  // Hash utility only; no hydro state or law.
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace ftd::thermal {
namespace {
using Byte = std::uint8_t;
constexpr Byte MAGIC[8] = {'F', 'T', 'D', 'T', 'H', '0', '1', 0};

std::size_t sites(std::uint32_t L) {
    static_assert(std::numeric_limits<std::size_t>::digits >= 32,
                  "thermal candidate requires a 32-bit or larger host");
    if (L < 3 || L > 128)
        throw std::invalid_argument("thermal L must be in [3,128]");
    return std::size_t(L) * L * L;
}

unsigned bit_count(std::uint64_t word) noexcept {
    unsigned count = 0;
    while (word != 0) { word &= word - 1; ++count; }
    return count;
}

unsigned bank_count(const Bank& bank) noexcept {
    unsigned count = 0;
    for (const auto word : bank) count += bit_count(word);
    return count;
}

// Called only with a nonzero word. The fallback keeps C++17 portability;
// no BMI/POPCNT instruction-set requirement is imposed on the build.
unsigned first_bit(std::uint64_t word) noexcept {
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    unsigned long index = 0;
    _BitScanForward64(&index, word);
    return unsigned(index);
#elif defined(_MSC_VER)
    unsigned long index = 0;
    const auto low = static_cast<unsigned long>(word);
    if (low != 0) { _BitScanForward(&index, low); return unsigned(index); }
    _BitScanForward(&index, static_cast<unsigned long>(word >> 32));
    return 32u + unsigned(index);
#elif defined(__GNUC__) || defined(__clang__)
    return unsigned(__builtin_ctzll(static_cast<unsigned long long>(word)));
#else
    unsigned index = 0;
    while ((word & 1u) == 0) { word >>= 1; ++index; }
    return index;
#endif
}

bool exactly_two(const Bank& bank, Pair& pair) noexcept {
    unsigned count = 0;
    for (std::size_t w = 0; w < WORDS_PER_SITE; ++w) {
        auto bits = bank[w];
        while (bits != 0) {
            if (count == 2) return false;
            pair[count++] = std::uint16_t(w * 64 + first_bit(bits));
            bits &= bits - 1;
        }
    }
    return count == 2;
}

bool get_bit(const Bank& bank, std::size_t channel) noexcept {
    return (bank[channel / 64] & (std::uint64_t{1} << (channel % 64))) != 0;
}

void put_bit(Bank& bank, std::size_t channel) noexcept {
    bank[channel / 64] |= std::uint64_t{1} << (channel % 64);
}

Velocity raw_velocity(std::size_t channel) noexcept {
    return {std::int8_t(int(channel / 49) - 3),
            std::int8_t(int((channel / 7) % 7) - 3),
            std::int8_t(int(channel % 7) - 3)};
}

std::size_t pair_index(std::size_t a, std::size_t b) noexcept {
    return a * (2 * CHANNELS - a - 1) / 2 + b - a - 1;
}

#if !defined(FTD_THERMAL_EQUIVARIANT)
struct KeyedPair {
    std::array<int, 4> key;
    Pair pair;
};
#endif

// Each separately linked executable has exactly one immutable law. There is
// no runtime table selector and no second record owner.
const std::array<Pair, PAIRS>& collision_table() {
#if defined(FTD_THERMAL_EQUIVARIANT)
    static const auto table = [] {
        std::array<Pair, PAIRS> result{};
        for (std::size_t i = 0; i < PAIRS; ++i)
            result[i] = {generated::EQUIVARIANT_SUCCESSORS[i][0],
                         generated::EQUIVARIANT_SUCCESSORS[i][1]};
        return result;
    }();
    return table;
#else
    // V1's exact lexicographic cycle selects an orientation. It is not
    // symmetry averaging or a cubic-covariance claim.
    static const auto table = [] {
        std::vector<KeyedPair> pairs;
        pairs.reserve(PAIRS);
        for (std::size_t a = 0; a < CHANNELS; ++a) {
            const auto va = raw_velocity(a);
            for (std::size_t b = a + 1; b < CHANNELS; ++b) {
                const auto vb = raw_velocity(b);
                std::array<int, 4> key{};
                for (unsigned axis = 0; axis < 3; ++axis) {
                    key[axis] = int(va[axis]) + int(vb[axis]);
                    key[3] += int(va[axis]) * int(va[axis])
                            + int(vb[axis]) * int(vb[axis]);
                }
                pairs.push_back({key, {std::uint16_t(a), std::uint16_t(b)}});
            }
        }
        std::sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
            return a.key != b.key ? a.key < b.key : a.pair < b.pair;
        });
        std::array<Pair, PAIRS> result{};
        for (std::size_t begin = 0; begin < pairs.size();) {
            std::size_t end = begin + 1;
            while (end < pairs.size() && pairs[end].key == pairs[begin].key) ++end;
            for (std::size_t i = begin; i < end; ++i) {
                const auto source = pairs[i].pair;
                result[pair_index(source[0], source[1])] =
                    pairs[i + 1 == end ? begin : i + 1].pair;
            }
            begin = end;
        }
        return result;
    }();
    return table;
#endif
}

unsigned wrap_hop(unsigned coordinate, int hop, unsigned L) noexcept {
    if (hop > 0) return coordinate + 1 == L ? 0 : coordinate + 1;
    if (hop < 0) return coordinate == 0 ? L - 1 : coordinate - 1;
    return coordinate;
}

void append_le(std::vector<Byte>& bytes, std::uint64_t value, unsigned count) {
    for (unsigned i = 0; i < count; ++i)
        bytes.push_back(Byte(value >> (8 * i)));
}

std::uint64_t read_le(const std::vector<Byte>& bytes, std::size_t offset,
                      unsigned count) noexcept {
    std::uint64_t value = 0;
    for (unsigned i = 0; i < count; ++i)
        value |= std::uint64_t(bytes[offset + i]) << (8 * i);
    return value;
}

const std::array<unsigned char, 32>& law_hash() {
    static const auto hash = hydro::sha256(
        reinterpret_cast<const Byte*>(LAW_ID), sizeof(LAW_ID) - 1);
    return hash;
}

const std::array<unsigned char, 32>& encoding_hash() {
    static const auto hash = hydro::sha256(
        reinterpret_cast<const Byte*>(ENCODING_ID), sizeof(ENCODING_ID) - 1);
    return hash;
}
}  // namespace

Records::Records(std::uint32_t size) : L(size) {
    if (size != 0) bank.resize(sites(size));
}

Velocity velocity(std::size_t channel) {
    if (channel >= CHANNELS) throw std::out_of_range("thermal channel index");
    return raw_velocity(channel);
}

std::size_t channel_index(int vx, int vy, int vz) {
    if (vx < -3 || vx > 3 || vy < -3 || vy > 3 || vz < -3 || vz > 3)
        throw std::out_of_range("thermal velocity component");
    return std::size_t(vx + 3) * 49 + std::size_t(vy + 3) * 7 + std::size_t(vz + 3);
}

bool occupied(const Bank& bank, std::size_t channel) {
    if (channel >= CHANNELS) throw std::out_of_range("thermal channel index");
    return get_bit(bank, channel);
}

void set_occupied(Bank& bank, std::size_t channel, bool value) {
    if (channel >= CHANNELS) throw std::out_of_range("thermal channel index");
    const auto mask = std::uint64_t{1} << (channel % 64);
    if (value) bank[channel / 64] |= mask;
    else bank[channel / 64] &= ~mask;
}

std::int8_t manifestation(const Bank& bank) {
    if ((bank.back() & ~LAST_WORD_MASK) != 0)
        throw std::invalid_argument("thermal unused channel bits must be zero");
    const auto count = bank_count(bank);
    return count == 0 ? std::int8_t{0}
                     : count % 2 ? std::int8_t{1} : std::int8_t{-1};
}

Pair collision_successor(std::size_t a, std::size_t b) {
    if (a >= b || b >= CHANNELS)
        throw std::invalid_argument("thermal pair requires 0 <= a < b < 343");
    return collision_table()[pair_index(a, b)];
}

std::array<std::uint8_t, 32> collision_table_hash() {
    static const auto hash = [] {
        const auto& table = collision_table();
        std::vector<Byte> bytes;
        bytes.reserve(PAIRS * 8);
        for (std::size_t a = 0; a < CHANNELS; ++a)
            for (std::size_t b = a + 1; b < CHANNELS; ++b) {
                const auto successor = table[pair_index(a, b)];
                append_le(bytes, a, 2);
                append_le(bytes, b, 2);
                append_le(bytes, successor[0], 2);
                append_le(bytes, successor[1], 2);
            }
        return hydro::sha256(bytes.data(), bytes.size());
    }();
    return hash;
}

void validate(const Records& records) {
    if (records.bank.size() != sites(records.L))
        throw std::invalid_argument("thermal bank shape mismatch");
    for (const auto& bank : records.bank)
        if ((bank.back() & ~LAST_WORD_MASK) != 0)
            throw std::invalid_argument("thermal unused channel bits must be zero");
}

Totals totals(const Records& records) {
    validate(records);
    Totals result;
    for (const auto& bank : records.bank) {
        for (std::size_t w = 0; w < WORDS_PER_SITE; ++w) {
            auto bits = bank[w];
            while (bits != 0) {
                const auto c = w * 64 + first_bit(bits);
                bits &= bits - 1;
                ++result.N;
                const auto v = raw_velocity(c);
                for (unsigned axis = 0; axis < 3; ++axis) {
                    result.P[axis] += v[axis];
                    result.E2 += std::uint64_t(int(v[axis]) * int(v[axis]));
                }
            }
        }
    }
    return result;
}

void step(Records& records, Scratch& scratch) {
    validate(records);
    if (records.microtick == std::numeric_limits<std::uint64_t>::max())
        throw std::overflow_error("thermal physical tick counter exhausted");
    const auto phase = records.phase();
    if (phase == 0) {
        const auto& table = collision_table();
        scratch.output = records.bank;
        for (std::size_t x = 0; x < records.bank.size(); ++x) {
            const auto& bank = records.bank[x];
            Pair input{};
            if (!exactly_two(bank, input)) continue;
            const auto output = table[pair_index(input[0], input[1])];
            scratch.output[x] = {};
            put_bit(scratch.output[x], output[0]);
            put_bit(scratch.output[x], output[1]);
        }
    } else {
        scratch.output.assign(records.bank.size(), Bank{});
        const auto L = records.L;
        for (std::size_t i = 0; i < records.bank.size(); ++i) {
            const unsigned x = unsigned(i % L);
            const unsigned y = unsigned((i / L) % L);
            const unsigned z = unsigned(i / (std::size_t(L) * L));
            for (std::size_t w = 0; w < WORDS_PER_SITE; ++w) {
                auto bits = records.bank[i][w];
                while (bits != 0) {
                    const auto c = w * 64 + first_bit(bits);
                    bits &= bits - 1;
                    const auto v = raw_velocity(c);
                    const auto hop = [phase](int component) {
                        return std::abs(component) >= int(phase)
                            ? (component > 0 ? 1 : -1) : 0;
                    };
                    const auto tx = wrap_hop(x, hop(v[0]), L);
                    const auto ty = wrap_hop(y, hop(v[1]), L);
                    const auto tz = wrap_hop(z, hop(v[2]), L);
                    const std::size_t target = tx + std::size_t(L) * (ty + std::size_t(L) * tz);
                    put_bit(scratch.output[target], c);
                }
            }
        }
    }
    // Every potentially throwing operation finished before this commit.
    records.bank.swap(scratch.output);
    ++records.microtick;
}

void advance(Records& records, Scratch& scratch, std::uint64_t microticks) {
    validate(records);
    if (microticks > std::numeric_limits<std::uint64_t>::max() - records.microtick)
        throw std::overflow_error("thermal batch tick counter exhausted");
    if (microticks == 0) return;
    Records candidate = records;
    for (std::uint64_t i = 0; i < microticks; ++i) step(candidate, scratch);
    records.bank.swap(candidate.bank);
    records.microtick = candidate.microtick;
}

std::vector<Byte> encode(const Records& records) {
    validate(records);
    const auto payload_bytes = records.bank.size() * BYTES_PER_SITE;
    std::vector<Byte> bytes;
    bytes.reserve(HEADER_BYTES + payload_bytes + CHECKSUM_BYTES);
    bytes.insert(bytes.end(), std::begin(MAGIC), std::end(MAGIC));
    append_le(bytes, SCHEMA_VERSION, 4);
    append_le(bytes, records.L, 4);
    append_le(bytes, records.microtick, 8);
    for (const auto* hash : {&law_hash(), &encoding_hash()})
        bytes.insert(bytes.end(), hash->begin(), hash->end());
    append_le(bytes, payload_bytes, 8);
    for (const auto& bank : records.bank)
        for (const auto word : bank) append_le(bytes, word, 8);
    const auto checksum = hydro::sha256(bytes.data(), bytes.size());
    bytes.insert(bytes.end(), checksum.begin(), checksum.end());
    return bytes;
}

Records decode(const std::vector<Byte>& bytes) {
    if (bytes.size() < HEADER_BYTES + CHECKSUM_BYTES
        || !std::equal(std::begin(MAGIC), std::end(MAGIC), bytes.begin()))
        throw std::invalid_argument("thermal checkpoint magic/length mismatch");
    if (read_le(bytes, 8, 4) != SCHEMA_VERSION)
        throw std::invalid_argument("thermal checkpoint schema mismatch");
    if (!std::equal(law_hash().begin(), law_hash().end(), bytes.begin() + 24)
        || !std::equal(encoding_hash().begin(), encoding_hash().end(), bytes.begin() + 56))
        throw std::invalid_argument("thermal checkpoint law/encoding mismatch");
    const auto L = std::uint32_t(read_le(bytes, 12, 4));
    const auto n = sites(L);
    const auto payload_bytes = n * BYTES_PER_SITE;
    if (read_le(bytes, 88, 8) != payload_bytes
        || bytes.size() != HEADER_BYTES + payload_bytes + CHECKSUM_BYTES)
        throw std::invalid_argument("thermal checkpoint exact payload length mismatch");
    const auto checksum = hydro::sha256(bytes.data(), bytes.size() - CHECKSUM_BYTES);
    if (!std::equal(checksum.begin(), checksum.end(), bytes.end() - CHECKSUM_BYTES))
        throw std::invalid_argument("thermal checkpoint checksum mismatch");
    Records records(L);
    records.microtick = read_le(bytes, 16, 8);
    std::size_t offset = HEADER_BYTES;
    for (auto& bank : records.bank)
        for (auto& word : bank) { word = read_le(bytes, offset, 8); offset += 8; }
    validate(records);
    return records;
}

}  // namespace ftd::thermal

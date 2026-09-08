#pragma once
// Versioned observation transport. These records describe the production
// reference engine; they are not part of its evolution or a strict checkpoint.
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ftd {
inline constexpr std::uint32_t kNativeObservationMagic = 0x334e5446u; // FTN3
inline constexpr std::size_t kNativeObservationHeaderBytes = 88;
inline constexpr std::uint64_t kWireSafeInteger = 9007199254740991ull;

// Preserve existing numeric consumers in the exact interval. Never put a
// rounded identity onto the JSON/JavaScript boundary outside that interval.
inline std::string json_exact_uint64(std::uint64_t value) {
    const auto decimal = std::to_string(value);
    return value <= kWireSafeInteger ? decimal : '"' + decimal + '"';
}

// External process namespace only: randomness never enters engine state or its
// RNG. Source epochs alone are not unique across a restarted server process.
inline const std::array<std::uint64_t, 2>& native_instance_nonce() {
    static const auto nonce = [] {
        std::random_device source;
        std::array<std::uint64_t, 2> result{};
        for (auto& word : result)
            word = (std::uint64_t(static_cast<std::uint32_t>(source())) << 32)
                 | static_cast<std::uint32_t>(source());
        if (result[0] == 0 && result[1] == 0)
            throw std::runtime_error("unavailable native transport instance namespace");
        return result;
    }();
    return nonce;
}
inline std::string native_instance_id() {
    const auto& nonce = native_instance_nonce();
    std::ostringstream out;
    out << std::hex << std::setfill('0') << std::setw(16) << nonce[1]
        << std::setw(16) << nonce[0];
    return out.str();
}

struct NativeObservation {
    std::uint64_t request_id = 0;
    std::uint64_t sample_tick = 0;
    std::uint64_t source_epoch = 0;
    std::uint64_t epoch = 0; // publisher state boundary, not backend state hash
    double physical_time = 0;
    double dt = 0;
    std::uint32_t lattice_size = 0;
    std::array<std::uint64_t, 2> instance_nonce{};
};

inline void write_native_observation_header(std::vector<std::uint8_t>& frame,
                                            const NativeObservation& meta) {
    if (frame.size() < kNativeObservationHeaderBytes + 4
        || meta.request_id == 0 || meta.request_id > kWireSafeInteger
        || meta.source_epoch == 0 || meta.epoch == 0 || meta.lattice_size == 0
        || (meta.instance_nonce[0] == 0 && meta.instance_nonce[1] == 0))
        throw std::invalid_argument("invalid native observation envelope");
    const auto put = [&](std::size_t offset, std::uint64_t value, unsigned bytes) {
        for (unsigned i = 0; i < bytes; ++i)
            frame[offset + i] = static_cast<std::uint8_t>(value >> (8 * i));
    };
    const auto floating = [&](std::size_t offset, double value) {
        static_assert(sizeof(double) == sizeof(std::uint64_t), "IEEE binary64 transport");
        std::uint64_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        put(offset, bits, 8);
    };
    put(0, kNativeObservationMagic, 4);
    put(4, kNativeObservationHeaderBytes, 4);
    put(8, meta.request_id, 8);
    put(16, meta.sample_tick, 8);
    put(24, meta.source_epoch, 8);
    put(32, meta.epoch, 8);
    put(40, frame.size() - kNativeObservationHeaderBytes, 8);
    floating(48, meta.physical_time);
    floating(56, meta.dt);
    put(64, meta.lattice_size, 4);
    put(68, 1, 4); // sampled floating reference observation
    put(72, meta.instance_nonce[0], 8);
    put(80, meta.instance_nonce[1], 8);
}
} // namespace ftd

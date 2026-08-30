#pragma once
/**
 * @file dynamical_state_digest.h
 * @brief Canonical, parallel-reducible digest of Scale-0 dynamical state.
 *
 * Schema 1 hashes named scalar values only. It never reads Voxel object
 * bytes, so ABI padding cannot enter the result. IEEE signed zero is
 * normalized to one representation. The schema includes state, the primary,
 * dual, strong, and weak field/velocity channels, particle velocity and
 * remainder, latency, lock/spin/color/flavor, acceleration magnitude, the
 * persistent dJ conjugate-velocity buffer, and the Coulomb/latency potentials.
 * It excludes global/per-voxel clocks, identity bookkeeping, RNG state,
 * counters, telemetry/ledger values, force diagnostics, phi (Gauss scratch),
 * and every delta_j read-phase scratch buffer.
 *
 * Parallel combination law
 * ------------------------
 * Each logical scalar is tagged by (schema, field, component, lattice index),
 * canonicalized, and mapped independently into two SplitMix64 lanes. A chunk
 * accumulator is the component-wise sum modulo 2^64 of those lanes and of the
 * exact non-finite/non-default counters. Therefore
 *
 *   digest(A union B) = digest(A) + digest(B)  (component-wise mod 2^64)
 *
 * for disjoint tagged chunks. Reduction-tree shape and GPU block scheduling
 * cannot change the result. Lattice index remains in every token, so the
 * commutative reduction does not discard spatial position.
 */

#include "ftd/voxel.h"

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#if defined(__CUDACC__)
#define FTD_DIGEST_HD __host__ __device__
#else
#define FTD_DIGEST_HD
#endif

namespace ftd {

inline constexpr std::uint32_t DYNAMICAL_STATE_DIGEST_SCHEMA = 1;
inline constexpr std::uint64_t DYNAMICAL_STATE_DIGEST_DOMAIN =
    0x4654443044594e41ULL;  // "FTD0DYNA"
inline constexpr std::uint64_t DYNAMICAL_STATE_VALUES_PER_SITE = 54;

enum class DynamicalStateField : std::uint32_t {
    State = 1,
    Flux = 2,
    WaveVelocity = 3,
    FluxLeft = 4,
    FluxRight = 5,
    WaveVelocityLeft = 6,
    WaveVelocityRight = 7,
    Velocity = 8,
    Remainder = 9,
    Latency = 10,
    Locked = 11,
    Spin = 12,
    Color = 13,
    Flavor = 14,
    AccelerationMagnitude = 15,
    StrongFlux = 16,
    StrongWaveVelocity = 17,
    WeakFlux = 18,
    WeakWaveVelocity = 19,
    ConjugateVelocity = 20,
    CoulombPotential = 21,
    LatencyPotential = 22,
};

struct DynamicalStateDigestAccumulator {
    std::uint64_t hash_lo = 0;
    std::uint64_t hash_hi = 0;
    std::uint64_t nonfinite_value_count = 0;
    std::uint64_t nondefault_value_count = 0;
};

struct DynamicalStateDigest {
    std::uint32_t schema_version = DYNAMICAL_STATE_DIGEST_SCHEMA;
    std::int32_t lattice_size = 0;
    std::uint64_t site_count = 0;
    std::int64_t tick = 0;
    std::uint64_t state_version = 0;
    std::uint64_t hash_lo = 0;
    std::uint64_t hash_hi = 0;
    std::uint64_t nonfinite_value_count = 0;
    std::uint64_t nondefault_value_count = 0;
    // Bytes copied device->host to answer this request. CPU captures report 0.
    std::uint64_t device_to_host_bytes = 0;

    bool exact_default_record() const {
        return nonfinite_value_count == 0 && nondefault_value_count == 0;
    }
};

namespace digest_detail {

FTD_DIGEST_HD inline std::uint64_t splitmix64(std::uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

FTD_DIGEST_HD inline std::uint64_t canonical_double_bits(double value) {
    if (value == 0.0) return 0;  // normalize +0 and -0
#if defined(__CUDA_ARCH__)
    return static_cast<std::uint64_t>(__double_as_longlong(value));
#else
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
#endif
}

FTD_DIGEST_HD inline bool double_bits_are_nonfinite(std::uint64_t bits) {
    return (bits & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL;
}

FTD_DIGEST_HD inline DynamicalStateDigestAccumulator combine(
    const DynamicalStateDigestAccumulator& lhs,
    const DynamicalStateDigestAccumulator& rhs) {
    return {
        lhs.hash_lo + rhs.hash_lo,
        lhs.hash_hi + rhs.hash_hi,
        lhs.nonfinite_value_count + rhs.nonfinite_value_count,
        lhs.nondefault_value_count + rhs.nondefault_value_count,
    };
}

FTD_DIGEST_HD inline void accumulate_bits(
    DynamicalStateDigestAccumulator& accumulator,
    DynamicalStateField field,
    std::uint32_t component,
    std::uint64_t lattice_index,
    std::uint64_t canonical_bits,
    bool nonfinite,
    bool nondefault) {
    const std::uint64_t field_component =
        (static_cast<std::uint64_t>(field) << 3) | component;
    const std::uint64_t position_key = splitmix64(
        DYNAMICAL_STATE_DIGEST_DOMAIN
        ^ (field_component * 0xd6e8feb86659fd93ULL)
        ^ (lattice_index * 0xa0761d6478bd642fULL));
    accumulator.hash_lo += splitmix64(position_key ^ canonical_bits);
    accumulator.hash_hi += splitmix64(
        (position_key ^ 0xe7037ed1a0b428dbULL)
        + (canonical_bits ^ (canonical_bits >> 29)));
    accumulator.nonfinite_value_count += nonfinite ? 1ULL : 0ULL;
    accumulator.nondefault_value_count += nondefault ? 1ULL : 0ULL;
}

FTD_DIGEST_HD inline void accumulate_double(
    DynamicalStateDigestAccumulator& accumulator,
    DynamicalStateField field,
    std::uint32_t component,
    std::uint64_t lattice_index,
    double value) {
    const std::uint64_t bits = canonical_double_bits(value);
    accumulate_bits(accumulator, field, component, lattice_index, bits,
                    double_bits_are_nonfinite(bits), bits != 0);
}

template <typename Integer>
FTD_DIGEST_HD inline void accumulate_integer(
    DynamicalStateDigestAccumulator& accumulator,
    DynamicalStateField field,
    std::uint64_t lattice_index,
    Integer value) {
    const std::uint64_t bits = static_cast<std::uint64_t>(
        static_cast<std::int64_t>(value));
    accumulate_bits(accumulator, field, 0, lattice_index, bits, false,
                    value != static_cast<Integer>(0));
}

FTD_DIGEST_HD inline void accumulate_vec3(
    DynamicalStateDigestAccumulator& accumulator,
    DynamicalStateField field,
    std::uint64_t lattice_index,
    const Vec3& value) {
    accumulate_double(accumulator, field, 0, lattice_index, value.x);
    accumulate_double(accumulator, field, 1, lattice_index, value.y);
    accumulate_double(accumulator, field, 2, lattice_index, value.z);
}

inline DynamicalStateDigest finalize(
    DynamicalStateDigestAccumulator accumulator,
    int lattice_size,
    std::uint64_t site_count,
    std::int64_t tick,
    std::uint64_t state_version,
    std::uint64_t device_to_host_bytes) {
    // Metadata is domain separation, not a dynamical value, and therefore
    // does not affect the exact nonfinite/nondefault counters.
    accumulator.hash_lo += splitmix64(
        DYNAMICAL_STATE_DIGEST_DOMAIN ^ DYNAMICAL_STATE_DIGEST_SCHEMA);
    accumulator.hash_hi += splitmix64(
        (static_cast<std::uint64_t>(lattice_size) << 32) ^ site_count
        ^ 0x8ebc6af09c88c6e3ULL);
    return {
        DYNAMICAL_STATE_DIGEST_SCHEMA,
        static_cast<std::int32_t>(lattice_size),
        site_count,
        tick,
        state_version,
        accumulator.hash_lo,
        accumulator.hash_hi,
        accumulator.nonfinite_value_count,
        accumulator.nondefault_value_count,
        device_to_host_bytes,
    };
}

}  // namespace digest_detail

inline DynamicalStateDigestAccumulator accumulate_dynamical_state_host(
    const std::vector<Voxel>& voxels,
    const std::vector<Vec3>& conjugate_velocity,
    const std::vector<double>& phi_coulomb,
    const std::vector<double>& phi_latency) {
    const std::size_t count = voxels.size();
    if (conjugate_velocity.size() != count || phi_coulomb.size() != count
        || phi_latency.size() != count) {
        throw std::invalid_argument(
            "canonical dynamical-state buffers must have identical extents");
    }

    DynamicalStateDigestAccumulator accumulator{};
    for (std::size_t i = 0; i < count; ++i) {
        const Voxel& voxel = voxels[i];
        const auto index = static_cast<std::uint64_t>(i);
        digest_detail::accumulate_integer(
            accumulator, DynamicalStateField::State, index, voxel.state);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::Flux, index, voxel.flux);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::WaveVelocity, index, voxel.wave_vel);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::FluxLeft, index, voxel.flux_L);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::FluxRight, index, voxel.flux_R);
        digest_detail::accumulate_vec3(accumulator,
            DynamicalStateField::WaveVelocityLeft, index, voxel.wave_vel_L);
        digest_detail::accumulate_vec3(accumulator,
            DynamicalStateField::WaveVelocityRight, index, voxel.wave_vel_R);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::Velocity, index, voxel.velocity);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::Remainder, index, voxel.remainder);
        digest_detail::accumulate_double(
            accumulator, DynamicalStateField::Latency, 0, index, voxel.latency);
        digest_detail::accumulate_integer(accumulator,
            DynamicalStateField::Locked, index,
            static_cast<std::uint8_t>(voxel.locked ? 1 : 0));
        digest_detail::accumulate_integer(
            accumulator, DynamicalStateField::Spin, index, voxel.spin);
        digest_detail::accumulate_integer(
            accumulator, DynamicalStateField::Color, index, voxel.color);
        digest_detail::accumulate_integer(
            accumulator, DynamicalStateField::Flavor, index, voxel.flavor);
        digest_detail::accumulate_double(accumulator,
            DynamicalStateField::AccelerationMagnitude, 0, index,
            voxel.accel_mag);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::StrongFlux, index,
            voxel.flux_strong);
        digest_detail::accumulate_vec3(accumulator,
            DynamicalStateField::StrongWaveVelocity, index,
            voxel.wave_vel_strong);
        digest_detail::accumulate_vec3(
            accumulator, DynamicalStateField::WeakFlux, index,
            voxel.flux_weak);
        digest_detail::accumulate_vec3(accumulator,
            DynamicalStateField::WeakWaveVelocity, index,
            voxel.wave_vel_weak);
        digest_detail::accumulate_vec3(accumulator,
            DynamicalStateField::ConjugateVelocity, index,
            conjugate_velocity[i]);
        digest_detail::accumulate_double(accumulator,
            DynamicalStateField::CoulombPotential, 0, index, phi_coulomb[i]);
        digest_detail::accumulate_double(accumulator,
            DynamicalStateField::LatencyPotential, 0, index, phi_latency[i]);
    }
    return accumulator;
}

}  // namespace ftd

#undef FTD_DIGEST_HD

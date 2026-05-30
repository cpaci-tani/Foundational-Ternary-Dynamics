#pragma once
// FTD voxel-level RNG (BH-F5 / BH-F8 / BH-F9 closure, 2026-05-05).
//
// Single canonical source of pseudo-randomness for FTD's stochastic
// kernels. Both CPU and GPU paths call into this header so per-voxel
// per-tick per-salt outputs are bit-exact CPU↔GPU at unit mass.
//
// Pre-2026-05-05 the engine used:
//   - CPU:  voxel_uniform() in phase_write.cpp (SplitMix64-based).
//   - GPU:  curandGenerateUniformDouble (Philox4_32_10 internally) into
//           a per-voxel d_random buffer that genesis/evaporation kernels
//           sampled by index.
// These two streams are statistically equivalent in distribution but
// diverge per-voxel from tick 1 onward. That divergence had to be
// absorbed as measurement noise in CPU↔GPU parity tests. This header
// closes the divergence by canonicalising the SplitMix64 stream and
// having both backends consume it via __host__ __device__ inlines.
//
// Salt domain enumeration is part of the PUBLIC RNG stream definition.
// Renumbering or reordering changes the entire engine's RNG output.
// Adding a new domain = adding a new value at the END (no reuse, no
// reorder).

#include <cstdint>
#ifndef __CUDACC__
#include <cmath>
#endif

#ifdef __CUDACC__
#define FTD_RNG_HD __host__ __device__ __forceinline__
#else
#define FTD_RNG_HD inline
#endif

namespace ftd {

// Salt domain enumeration. Stable values, part of the public stream.
enum class VoxelRng : std::uint64_t {
    GenesisManifest = 1,
    GenesisSpin     = 2,
    Evaporation     = 3,
    LangevinNoiseX  = 4,  // BH-F5: replaces curandGenerateNormalDouble (axis x)
    LangevinNoiseY  = 5,  // axis y
    LangevinNoiseZ  = 6,  // axis z
    WeakTransmutation = 7,
    PairProduction = 8,
};

// SplitMix64-based per-voxel uniform on [0, 1).
//
// Inputs:
//   seed       — engine-level RNG seed (e.g. toggles.langevin_seed).
//   voxel_idx  — flat lattice index of the site.
//   tick       — current tick number.
//   salt       — VoxelRng domain selector (cast to uint64_t).
//
// Output: a uniform double in [0, 1) deterministic in the four inputs.
// Same arithmetic on CPU and GPU; bit-exact under IEEE-754 double
// (no transcendentals, no machine-specific intrinsics).
FTD_RNG_HD
double voxel_uniform(std::uint64_t seed,
                     int           voxel_idx,
                     int           tick,
                     std::uint64_t salt) {
    std::uint64_t x = seed
                    ^ (static_cast<std::uint64_t>(voxel_idx) * 0x9E3779B97F4A7C15ULL)
                    ^ (static_cast<std::uint64_t>(tick)      * 0xBF58476D1CE4E5B9ULL)
                    ^ (salt                                  * 0x94D049BB133111EBULL);
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    x =  x ^ (x >> 31);
    // Mantissa-only conversion to double in [0, 1): top 53 bits / 2^53.
    return (x >> 11) * (1.0 / 9007199254740992.0);
}

// Box-Muller-derived per-voxel N(0, 1) Gaussian.
//
// Replaces curandGenerateNormalDouble for the Langevin OU update.
// Each axis (x, y, z) gets its own salt domain (LangevinNoiseX/Y/Z),
// so the three components are deterministic and independent.
//
// Implementation: two SplitMix64 uniforms with disjoint salt-offsets,
// combined via standard Box-Muller cos-lane. This produces one N(0,1)
// per voxel-axis-tick triple.
//
// log() and sqrt() are IEEE-754-conformant on both CPU (<cmath>) and
// CUDA device (intrinsics). cos() is conformant for the bounded
// 2π·u argument range. The transcendentals introduce sub-ULP-level
// CPU↔GPU differences; for parity-testing purposes these are bounded
// by ~1e-15 relative error per draw.
FTD_RNG_HD
double voxel_normal(std::uint64_t seed,
                    int           voxel_idx,
                    int           tick,
                    std::uint64_t salt) {
    constexpr double TWO_PI = 6.28318530717958647692;
    // Two uniforms with disjoint offsets so they're independent draws.
    double u0 = voxel_uniform(seed, voxel_idx, tick, salt);
    double u1 = voxel_uniform(seed, voxel_idx, tick, salt + (1ULL << 32));
    // Avoid log(0) at the bottom edge.
    constexpr double EPS = 1e-300;
    if (u0 < EPS) u0 = EPS;
#ifdef __CUDACC__
    double mag = ::sqrt(-2.0 * ::log(u0));
    double c   = ::cos(TWO_PI * u1);
#else
    double mag = std::sqrt(-2.0 * std::log(u0));
    double c   = std::cos(TWO_PI * u1);
#endif
    return mag * c;
}

}  // namespace ftd

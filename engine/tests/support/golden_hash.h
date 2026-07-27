#pragma once
// ============================================================================
// golden_hash.h — shared golden-gate state-hash harness (revision 0.5).
//
// Extracted VERBATIM from test_render_bridge_golden.cpp so multi-profile
// goldens (default-profile, boundary-mode, L=9, GPU) fold state identically.
// The extraction commit reproduced GOLDEN_HASH 0xb604d81a3d79366e bit-exact.
//
// Hash design (see ADR-0012): xor-fold of the bit representations of every
// double we care about, mixed through the FNV-1a 64-bit prime so each
// contribution is permuted before xoring — pure XOR is order-independent,
// which would mask voxel-permutation bugs.
//
// Two folds:
//   compute_state_hash(rb)      — the ORIGINAL fold: per-voxel state / flux /
//                                 wave_vel / velocity + energy audit +
//                                 manifested-particle list. Any change to its
//                                 mixing order or field set changes every
//                                 pinned golden — do not touch.
//   compute_state_hash_ext(rb)  — the original fold PLUS per-voxel dual-
//                                 substrate fields (flux_L/R, wave_vel_L/R)
//                                 and latency, for goldens whose profile
//                                 exercises those sectors (default-profile
//                                 golden and later variants). Separate
//                                 function so the original constant stays
//                                 valid forever.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cstdint>
#include <cstring>
#include <cmath>

namespace ftd { namespace test {

// ---------------------------------------------------------------------------
// Hash mixer — FNV-1a 64-bit prime. Each contribution is multiplied through
// the mixer before being xored into the running hash, so reordering inputs
// changes the hash (catches voxel-permutation regressions).
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_FNV_PRIME  = 0x100000001b3ULL;
static constexpr std::uint64_t GOLDEN_FNV_OFFSET = 0xcbf29ce484222325ULL;

inline std::uint64_t mix_u64(std::uint64_t h, std::uint64_t v) {
    h ^= v;
    h *= GOLDEN_FNV_PRIME;
    return h;
}

inline std::uint64_t mix_double(std::uint64_t h, double d) {
    // Bit-cast double -> uint64. NaNs collapse to a sentinel so a NaN bug
    // still gives a stable hash (failure is then visible in the audit, not
    // in the hash diff).
    if (std::isnan(d)) return mix_u64(h, 0x7ff8000000000000ULL);
    std::uint64_t u;
    std::memcpy(&u, &d, sizeof(u));
    return mix_u64(h, u);
}

inline std::uint64_t mix_vec3(std::uint64_t h, const Vec3& v) {
    h = mix_double(h, v.x);
    h = mix_double(h, v.y);
    h = mix_double(h, v.z);
    return h;
}

inline std::uint64_t mix_i64(std::uint64_t h, std::int64_t i) {
    return mix_u64(h, static_cast<std::uint64_t>(i));
}

namespace golden_detail {

// Energy-audit fold — the REPORTED DIAGNOSTIC SCALARS.
//
// Split out from mix_audit_and_manifested on 2026-07-27 so a change to a
// reported number can be distinguished from a change to the simulation
// trajectory. Folding both into one constant meant a diagnostic correction and
// a physics regression produced the same failure, which is how a gate stops
// being informative. The combined fold below is unchanged bit-for-bit.
inline std::uint64_t mix_audit(std::uint64_t h, const RenderBridge& rb) {
    // 2. Energy audit — 22 doubles + 2 ints + Vec3 poynting.
    auto a = rb.energy_audit();
    h = mix_double(h, a.field_energy);
    h = mix_double(h, a.wave_energy);
    h = mix_double(h, a.particle_ke);
    h = mix_double(h, a.total_energy);
    h = mix_double(h, a.gauss_violation);
    h = mix_double(h, a.max_gauss_error);
    h = mix_double(h, a.self_field_injection);
    h = mix_double(h, a.coulomb_pe);
    h = mix_double(h, a.E_field_energy);
    h = mix_double(h, a.B_field_energy);
    h = mix_i64(h, static_cast<std::int64_t>(a.charge_total));
    h = mix_i64(h, static_cast<std::int64_t>(a.manifested_count));
    h = mix_vec3(h, a.total_poynting);
    h = mix_double(h, a.E_L_total);
    h = mix_double(h, a.E_R_total);
    h = mix_double(h, a.wv_L_total);
    h = mix_double(h, a.wv_R_total);
    h = mix_double(h, a.chirality_total);
    h = mix_double(h, a.strong_energy);
    h = mix_double(h, a.weak_energy);

    return h;
}

// Manifested-particle-list fold — part of the TRAJECTORY, not the diagnostics.
inline std::uint64_t mix_manifested(std::uint64_t h, const RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    const int N = static_cast<int>(voxels.size());

    // 3. Manifested-particle list — (idx, state, velocity) per manifested site.
    int n_manifested = 0;
    for (int idx = 0; idx < N; ++idx) {
        if (voxels[idx].state != 0) {
            h = mix_i64(h, idx);
            h = mix_i64(h, static_cast<std::int64_t>(voxels[idx].state));
            h = mix_vec3(h, voxels[idx].velocity);
            ++n_manifested;
        }
    }
    h = mix_i64(h, n_manifested);

    return h;
}

// Combined fold, preserved BIT-FOR-BIT (audit then manifested, same order as
// before the split) so every historical pinned constant keeps its meaning.
inline std::uint64_t mix_audit_and_manifested(std::uint64_t h, const RenderBridge& rb) {
    h = mix_audit(h, rb);
    h = mix_manifested(h, rb);
    return h;
}

// Per-voxel trajectory fold (original field set).
inline std::uint64_t mix_voxels(std::uint64_t h, const RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    const int N = static_cast<int>(voxels.size());
    h = mix_i64(h, N);
    for (int idx = 0; idx < N; ++idx) {
        const auto& v = voxels[idx];
        h = mix_i64(h, static_cast<std::int64_t>(v.state));
        h = mix_vec3(h, v.flux);
        h = mix_vec3(h, v.wave_vel);
        h = mix_vec3(h, v.velocity);
    }
    return h;
}

} // namespace golden_detail

// ---------------------------------------------------------------------------
// Compute the byte-hash of the current engine state (ORIGINAL fold — the
// pinned GOLDEN_HASH constants of the minimal-profile golden depend on this
// exact field set and order).
// ---------------------------------------------------------------------------
inline std::uint64_t compute_state_hash(const RenderBridge& rb) {
    std::uint64_t h = GOLDEN_FNV_OFFSET;

    // 1. Voxel fields — every site, in linear index order.
    const auto& voxels = rb.voxels();
    const int N = static_cast<int>(voxels.size());
    h = mix_i64(h, N);
    for (int idx = 0; idx < N; ++idx) {
        const auto& v = voxels[idx];
        h = mix_i64(h, static_cast<std::int64_t>(v.state));
        h = mix_vec3(h, v.flux);
        h = mix_vec3(h, v.wave_vel);
        h = mix_vec3(h, v.velocity);
    }

    return golden_detail::mix_audit_and_manifested(h, rb);
}

// ---------------------------------------------------------------------------
// Extended fold: original per-voxel fields PLUS dual-substrate flux_L/R,
// wave_vel_L/R and the latency scalar. For goldens whose toggle profile
// exercises those sectors (default-profile golden etc.). Do NOT retrofit
// onto the minimal-profile golden — its pinned constant uses the original
// fold above.
// ---------------------------------------------------------------------------
inline std::uint64_t compute_state_hash_ext(const RenderBridge& rb) {
    std::uint64_t h = GOLDEN_FNV_OFFSET;

    // 1. Voxel fields — every site, in linear index order.
    const auto& voxels = rb.voxels();
    const int N = static_cast<int>(voxels.size());
    h = mix_i64(h, N);
    for (int idx = 0; idx < N; ++idx) {
        const auto& v = voxels[idx];
        h = mix_i64(h, static_cast<std::int64_t>(v.state));
        h = mix_vec3(h, v.flux);
        h = mix_vec3(h, v.wave_vel);
        h = mix_vec3(h, v.velocity);
        h = mix_vec3(h, v.flux_L);
        h = mix_vec3(h, v.flux_R);
        h = mix_vec3(h, v.wave_vel_L);
        h = mix_vec3(h, v.wave_vel_R);
        h = mix_double(h, v.latency);
    }

    return golden_detail::mix_audit_and_manifested(h, rb);
}

// ---------------------------------------------------------------------------
// SPLIT GATE (2026-07-27).
//
// `compute_state_hash` above folds the simulation trajectory AND the reported
// diagnostic scalars into one number, so it cannot distinguish
//   "the physics changed"            (serious)
// from
//   "a reported number was corrected" (often intended).
// On 2026-07-27 a correction that restored the missing c^2 to the magnetic
// energy and Poynting flux moved four golden constants while provably not
// touching a single voxel field -- and the gate reported it identically to a
// physics regression. A detector that fires the same way for both teaches you
// to ignore it.
//
// These two folds let a test report WHICH half moved. Use them together; their
// concatenation covers exactly the same fields as compute_state_hash.
// ---------------------------------------------------------------------------

/** Trajectory only: per-voxel fields + the manifested-particle list. */
inline std::uint64_t compute_state_only_hash(const RenderBridge& rb) {
    std::uint64_t h = GOLDEN_FNV_OFFSET;
    h = golden_detail::mix_voxels(h, rb);
    h = golden_detail::mix_manifested(h, rb);
    return h;
}

/** Reported diagnostics only: the energy-audit scalars. */
inline std::uint64_t compute_audit_only_hash(const RenderBridge& rb) {
    std::uint64_t h = GOLDEN_FNV_OFFSET;
    return golden_detail::mix_audit(h, rb);
}

}}  // namespace ftd::test

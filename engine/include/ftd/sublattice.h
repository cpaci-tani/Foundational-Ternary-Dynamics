#pragma once
/**
 * Sublattice projection — SC / FCC / BCC sub-stencils of the Moore-26
 * neighborhood. Required infrastructure for the Cluster A campaign
 * (Mechanism C for g_c on σ_BCC, L2 calibration-invariant ratio test).
 *
 * Background. The current engine's 18-pt isotropic Laplacian
 *   L₁₈ = (1/3)·face_sum + (1/6)·edge_sum − 4·center
 * is structurally (σ_SC + σ_FCC)/2 — orthogonal to the BCC sub-stencil
 * where the master quadratic's polynomial roots live (FTD-0050 closure;
 * see docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md).
 *
 * This header introduces three sublattice-projected Laplacians, runnable
 * on the same field, so a campaign can measure spectra on any of the
 * three sub-stencils with no other engine path in scope. Pre-registered
 * predictions for all three live in
 *   docs/theory/10_eft_program/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md.
 *
 * Mathematical form (per ontic/lemniscate.h:130-147):
 *   SC  (6 face nbrs):       Lap_sc  = (1/6) Σ nbrs - center
 *                            Watson normalization: I₃ ≈ 0.506
 *   FCC (12 edge nbrs):      Lap_fcc = (1/12) Σ nbrs - center
 *                            Watson normalization: I₂ ≈ 0.446
 *   BCC (8 corner nbrs):     Lap_bcc = (1/8) Σ nbrs - center
 *                            Watson normalization: I₁ = G*²/(2π) (THEOREM)
 *   FULL (legacy 18-pt):     existing laplacian_field<F> in field_operators.h
 *
 * Each is a *consistent* discrete Laplacian (sum of weights = 0).
 * Leading-order Taylor coefficients differ:
 *   SC:  ∇²f h² + O(h⁴ anisotropic)
 *   FCC: 2∇²f h² (edge sites at distance √2)
 *   BCC: 3∇²f h² (corner sites at distance √3)
 *   FULL: ∇²f h² + O(h⁶) (the 2:1 face:edge ratio cancels O(h⁴))
 * Spectrum-extraction code must divide by the per-stencil scaling so
 * x₊/x₋ ratios are scaling-invariant.
 *
 * Parity classification (BCC body-center voxel convention).
 * The "BCC sublattice" in standard crystallography = SC ∪ SC+(½,½,½).
 * On the integer cubic lattice we encode this as parity classes:
 *   SC sites:  (even, even, even)              — N/8 of voxels
 *   BCC sites: (odd,  odd,  odd)               — N/8 of voxels
 *   FCC sites: remainder (mixed parities)      — 6N/8 of voxels
 * Note: this is a property of *voxel coordinates*, distinct from the
 * neighbor-set property (which 8/12/6 neighbors enter the stencil).
 * Every voxel — regardless of parity class — has 8 corner neighbors,
 * 12 edge neighbors, and 6 face neighbors. The Laplacian variants below
 * apply uniformly site-by-site; sublattice CLASSIFICATION is used by
 * the Langevin filter (term_toggles.h::LangevinSiteFilter) and by
 * sublattice-filtered correlators (correlations.h).
 *
 * THESE MUST STAY INLINE — called per-voxel per-tick from phase kernels
 * and from correlator inner loops.
 */

#include <array>
#include <cstdint>
#include <vector>
#include "lattice.h"
#include "voxel.h"
#include "ontic/master_quadratic.h"   // D_SPATIAL — single source of truth

namespace ftd {

// Stencil-mode selector for phase_read sublattice projection.
// Controls which neighbor set's Laplacian drives wave/coupling updates.
// FULL preserves the legacy 18-pt path; the other three select a
// single sub-stencil.
enum class BccStencilMode : uint8_t {
    FULL = 0,  // legacy (1/3 face + 1/6 edge − 4 center) — DEFAULT
    SC   = 1,  // 6 face neighbors only
    FCC  = 2,  // 12 edge neighbors only
    BCC  = 3   // 8 corner neighbors only
};

// Voxel-parity site class. Used to identify which sublattice a voxel
// SITS ON (vs. which neighbor-set its Laplacian uses, which is uniform).
enum class SiteClass : uint8_t {
    SC_SITES  = 0,  // (even, even, even)
    BCC_SITES = 1,  // (odd,  odd,  odd)
    FCC_SITES = 2,  // remainder
    ALL_SITES = 3
};

// Classify a voxel by its (x,y,z) parity. BCC = all odd; SC = all even.
inline SiteClass classify_voxel(int x, int y, int z) {
    const int px = x & 1;
    const int py = y & 1;
    const int pz = z & 1;
    if (px == 0 && py == 0 && pz == 0) return SiteClass::SC_SITES;
    if (px == 1 && py == 1 && pz == 1) return SiteClass::BCC_SITES;
    return SiteClass::FCC_SITES;
}

inline SiteClass classify_voxel(const Lattice& lat, int idx) {
    auto c = lat.coord(idx);
    return classify_voxel(c.x, c.y, c.z);
}

// Predicate: does the given voxel match the requested filter?
// ALL_SITES always returns true; site-class match returns class equality.
inline bool site_matches_filter(SiteClass site, SiteClass filter) {
    return filter == SiteClass::ALL_SITES || site == filter;
}

inline bool site_matches_filter(const Lattice& lat, int idx, SiteClass filter) {
    if (filter == SiteClass::ALL_SITES) return true;
    return classify_voxel(lat, idx) == filter;
}

// === Neighbor counts on the D-dimensional cubic lattice ===
// Derived from cubic-lattice geometry (D = ftd::ontic::D_SPATIAL = 3).
//   N_FACE   = 2D                  — axial neighbors (±1 along one axis)
//   N_EDGE   = 4·C(D,2) = 2D(D-1)  — face-diagonal neighbors (two axes ±1)
//   N_CORNER = 2^D                 — body-diagonal neighbors (all axes ±1)
// These are the sizes of Lattice::neighbors_6/12/8_corner respectively.
// At D=3: (N_FACE, N_EDGE, N_CORNER) = (6, 12, 8). The ratio sum to the
// 26-neighbor Moore shell: N_FACE + N_EDGE + N_CORNER = 26 = 3^D − 1 [THEOREM].
inline constexpr int N_FACE   = 2 * ftd::ontic::D_SPATIAL;                            // 6
inline constexpr int N_EDGE   = 2 * ftd::ontic::D_SPATIAL * (ftd::ontic::D_SPATIAL - 1); // 12
inline constexpr int N_CORNER = 1 << ftd::ontic::D_SPATIAL;                           // 8
static_assert(N_FACE + N_EDGE + N_CORNER == 26,
              "Moore-26 shell decomposition broken: N_FACE+N_EDGE+N_CORNER must equal 3^D−1 at D=3");

// Sub-stencil weights derived from neighbor counts. Single source of truth
// for correlation extractors and tests for analytic eigenvalue checks.
// Each is a *consistent* discrete Laplacian (sum of weights minus center = 0).
inline constexpr double W_SC_FACE   = 1.0 / static_cast<double>(N_FACE);    // 1/6
inline constexpr double W_FCC_EDGE  = 1.0 / static_cast<double>(N_EDGE);    // 1/12
inline constexpr double W_BCC_CORNER = 1.0 / static_cast<double>(N_CORNER); // 1/8

// === Sublattice-projected Laplacians ===
// Templated on Voxel field pointer, mirroring laplacian_field<F> in
// field_operators.h. Each accumulates ONLY the relevant neighbor set.

// SC sub-stencil: 6 face neighbors weighted 1/6 minus center.
template <Vec3 Voxel::*F>
inline Vec3 laplacian_sc(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
    const auto& face = lattice.neighbors_6(idx);
    Vec3 lap;
    for (int n : face) lap += voxels[n].*F * W_SC_FACE;
    lap -= voxels[idx].*F;
    return lap;
}

// FCC sub-stencil: 12 edge neighbors weighted 1/12 minus center.
template <Vec3 Voxel::*F>
inline Vec3 laplacian_fcc(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
    const auto& edge = lattice.neighbors_12(idx);
    Vec3 lap;
    for (int n : edge) lap += voxels[n].*F * W_FCC_EDGE;
    lap -= voxels[idx].*F;
    return lap;
}

// BCC sub-stencil: 8 corner neighbors weighted 1/8 minus center.
// This is the load-bearing kernel for Cluster A — the master quadratic
// lives on this sub-stencil (Watson I₁ = G*²/(2π); ontic/lemniscate.h:147).
template <Vec3 Voxel::*F>
inline Vec3 laplacian_bcc(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
    const auto& corner = lattice.neighbors_8_corner(idx);
    Vec3 lap;
    for (int n : corner) lap += voxels[n].*F * W_BCC_CORNER;
    lap -= voxels[idx].*F;
    return lap;
}

// Single-source-of-truth dispatch wrapper. Branches on stencil mode at
// the call site. CPU-side only; GPU kernels select via separate kernel
// launches (see kernels_sublattice.cu, hedge step E9).
template <Vec3 Voxel::*F>
inline Vec3 laplacian_sublattice(BccStencilMode mode,
                                  const std::vector<Voxel>& voxels,
                                  const Lattice& lattice, int idx) {
    switch (mode) {
        case BccStencilMode::SC:  return laplacian_sc<F>(voxels, lattice, idx);
        case BccStencilMode::FCC: return laplacian_fcc<F>(voxels, lattice, idx);
        case BccStencilMode::BCC: return laplacian_bcc<F>(voxels, lattice, idx);
        case BccStencilMode::FULL:
        default: {
            // Inline the legacy 18-pt form rather than depending on
            // field_operators.h — keeps this header standalone.
            const auto& face = lattice.neighbors_6(idx);
            const auto& edge = lattice.neighbors_12(idx);
            Vec3 lap;
            for (int n : face) lap += voxels[n].*F * (1.0/3.0);
            for (int n : edge) lap += voxels[n].*F * (1.0/6.0);
            lap -= voxels[idx].*F * 4.0;
            return lap;
        }
    }
}

}  // namespace ftd

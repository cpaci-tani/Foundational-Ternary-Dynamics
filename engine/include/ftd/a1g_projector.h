#pragma once
/**
 * A_{1g} projector for the 27-voxel Moore block.
 *
 * Bridge-I empirical instrument (FTD-0110, Option A).
 * See docs/theory/03_derivations/DERIV_FTD0110_NONLINEAR_BRIDGE.md §5.2.
 *
 * Under the cubic point group O_h, the natural permutation rep on a 3³
 * block decomposes as
 *
 *     ρ_27 ≅ 4·A_{1g} ⊕ 2·E_g ⊕ 2·T_{2g} ⊕ A_{2u} ⊕ 3·T_{1u} ⊕ T_{2u}.
 *
 * The 4-dim A_{1g}-isotypic subspace is spanned by the four orbit-sum
 * vectors (one per O_h-orbit of the 27 voxels):
 *
 *     ê_center = δ_{(0,0,0)}                               (orbit size 1)
 *     ê_face   = (1/√6) · Σ_{v ∈ SC face}   δ_v             (orbit size 6)
 *     ê_edge   = (1/√12) · Σ_{v ∈ FCC edge} δ_v             (orbit size 12)
 *     ê_corner = (1/√8) · Σ_{v ∈ BCC corner} δ_v            (orbit size 8)
 *
 * For a scalar function f : 27-block → R, the A_{1g} energy is
 *
 *     E_{A_{1g}} = f_center²
 *                + (1/6) (Σ_face f)²
 *                + (1/12) (Σ_edge f)²
 *                + (1/8) (Σ_corner f)².
 *
 * The total energy is ‖f‖² = Σ f_v² over all 27 voxels.
 * The A_{1g} fraction is E_{A_{1g}} / ‖f‖² ∈ [0, 1].
 *
 * Sanity: for f = δ_center → fraction = 1; for f ≡ const → fraction = 1;
 * for a uniformly random f → fraction = dim(A_{1g}) / 27 = 4/27 ≈ 0.148.
 *
 * Bridge-I claim: starting from an A_{1g}-pure IC (e.g. δ_center · A) and
 * evolving under the FTD pipeline, the per-component A_{1g} fraction of φ
 * remains 1 in expectation. Empirical verification of this is the queued
 * cross-check from DERIV_FTD0110_NONLINEAR_BRIDGE.md §5.2.
 */

#include "voxel.h"

#include <cstddef>
#include <vector>

namespace ftd {

struct A1gFraction {
    double mean = 0.0;          // ⅓ Σ_α f_α (component-averaged)
    double f_x = 0.0;           // per-component fractions
    double f_y = 0.0;
    double f_z = 0.0;
    double total_energy = 0.0;  // ‖φ‖² over 27 block voxels (all 3 components)
    double a1g_energy = 0.0;    // A_{1g} energy summed over 3 components
};

inline int wrap_idx(int x, int L) {
    return ((x % L) + L) % L;
}

inline int lattice_idx(int x, int y, int z, int L) {
    return wrap_idx(x, L) * L * L + wrap_idx(y, L) * L + wrap_idx(z, L);
}

// 26-voxel Moore neighborhood offsets, partitioned by O_h orbit.
// Center orbit is {(0,0,0)} — handled separately.
struct MooreOrbits {
    static constexpr int FACE_OFFSETS[6][3] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1}
    };
    static constexpr int EDGE_OFFSETS[12][3] = {
        { 1, 1, 0}, { 1,-1, 0}, {-1, 1, 0}, {-1,-1, 0},
        { 1, 0, 1}, { 1, 0,-1}, {-1, 0, 1}, {-1, 0,-1},
        { 0, 1, 1}, { 0, 1,-1}, { 0,-1, 1}, { 0,-1,-1}
    };
    static constexpr int CORNER_OFFSETS[8][3] = {
        { 1, 1, 1}, { 1, 1,-1}, { 1,-1, 1}, { 1,-1,-1},
        {-1, 1, 1}, {-1, 1,-1}, {-1,-1, 1}, {-1,-1,-1}
    };
};

/**
 * Compute the A_{1g} fraction of the flux field on the 27-block centred
 * at (cx, cy, cz). Uses periodic wrap on the lattice of side L.
 *
 * Returns per-component fractions f_x, f_y, f_z and the component-averaged
 * mean. Each f_α ∈ [0, 1] (or 1 by convention if the component has zero
 * total energy).
 *
 * The total `mean` is computed by summing A_{1g} energies and total
 * energies across all three components, then taking the ratio. This is
 * the energy-weighted aggregate fraction (a single number ≤ 1 measuring
 * "what fraction of the flux's L² energy at this block is A_{1g}").
 */
inline A1gFraction compute_a1g_fraction(
    const std::vector<Voxel>& voxels, int L,
    int cx, int cy, int cz)
{
    A1gFraction out;

    auto component_at = [&](int x, int y, int z, int alpha) -> double {
        const int i = lattice_idx(x, y, z, L);
        const auto& v = voxels[i];
        switch (alpha) {
            case 0: return v.flux.x;
            case 1: return v.flux.y;
            case 2: return v.flux.z;
        }
        return 0.0;
    };

    double per_alpha_a1g[3] = {0.0, 0.0, 0.0};
    double per_alpha_tot[3] = {0.0, 0.0, 0.0};

    for (int alpha = 0; alpha < 3; ++alpha) {
        const double f_center = component_at(cx, cy, cz, alpha);

        double sum_face = 0.0, sum_edge = 0.0, sum_corner = 0.0;
        double tot = f_center * f_center;

        for (int k = 0; k < 6; ++k) {
            const auto& d = MooreOrbits::FACE_OFFSETS[k];
            const double f = component_at(cx + d[0], cy + d[1], cz + d[2], alpha);
            sum_face += f;
            tot += f * f;
        }
        for (int k = 0; k < 12; ++k) {
            const auto& d = MooreOrbits::EDGE_OFFSETS[k];
            const double f = component_at(cx + d[0], cy + d[1], cz + d[2], alpha);
            sum_edge += f;
            tot += f * f;
        }
        for (int k = 0; k < 8; ++k) {
            const auto& d = MooreOrbits::CORNER_OFFSETS[k];
            const double f = component_at(cx + d[0], cy + d[1], cz + d[2], alpha);
            sum_corner += f;
            tot += f * f;
        }

        const double a1g = f_center * f_center
                         + (sum_face * sum_face) / 6.0
                         + (sum_edge * sum_edge) / 12.0
                         + (sum_corner * sum_corner) / 8.0;

        per_alpha_a1g[alpha] = a1g;
        per_alpha_tot[alpha] = tot;
    }

    auto safe_ratio = [](double num, double den) -> double {
        // If a component has identically zero energy on the block, treat
        // it as trivially A_{1g}-pure (the zero vector lies in every
        // subspace).
        return (den > 0.0) ? (num / den) : 1.0;
    };

    out.f_x = safe_ratio(per_alpha_a1g[0], per_alpha_tot[0]);
    out.f_y = safe_ratio(per_alpha_a1g[1], per_alpha_tot[1]);
    out.f_z = safe_ratio(per_alpha_a1g[2], per_alpha_tot[2]);

    out.a1g_energy   = per_alpha_a1g[0] + per_alpha_a1g[1] + per_alpha_a1g[2];
    out.total_energy = per_alpha_tot[0] + per_alpha_tot[1] + per_alpha_tot[2];
    out.mean = safe_ratio(out.a1g_energy, out.total_energy);

    return out;
}

}  // namespace ftd

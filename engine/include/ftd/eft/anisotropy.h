#pragma once
/**
 * @file ftd/eft/anisotropy.h
 * @brief Rotational-anisotropy measurement for the EFT Recovery Program (Phase 1A).
 *
 * Physics motivation
 * ------------------
 * A cubic lattice breaks the continuous rotational symmetry O(3) down to the
 * octahedral subgroup O_h. At the lattice scale this is unavoidable; a
 * Wilsonian EFT requires the broken symmetry to be *recovered* in the IR. That
 * recovery is quantified by how rapidly direction-class correlators coalesce
 * as r grows past a few lattice spacings.
 *
 * This module computes the flux-flux two-point function separately along the
 * three inequivalent direction classes on the cubic lattice:
 *
 *     face     = (1, 0, 0)       · 3 cubic axes, shortest displacement family
 *     edge     = (1, 1, 0)       · 6 face-diagonal directions
 *     diagonal = (1, 1, 1)       · 4 body-diagonal directions (∋ BCC substructure)
 *
 * From each direction-class correlator C_dir(r) we report:
 *   - screening length ξ_dir from C(r) = A · exp(−r/ξ) + B · r^(−η) fit;
 *   - anisotropy coefficient δ(r) = (ξ_face − ξ_diag) / ξ̄;
 *   - residual amplitude |C_face(r) − C_diag(r)| / |C̄(r)|.
 *
 * Pre-registered expectations are in
 * docs/theory/10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md §4.1. No expectation
 * is adjusted here after the fact.
 *
 * Epistemic status
 * ----------------
 * All measurement; no new physics inserted. The functions here do not *claim*
 * rotational invariance — they *quantify* its recovery. Tag: [MEASUREMENT].
 *
 * Complexity
 * ----------
 * O(L³ · max_r). For L = 64, max_r = L/2 = 32 this is ~8.4 M dot products per
 * direction class; three classes total. Acceptable for offline measurement
 * (seconds), not a hot-path computation.
 *
 * Coordinate convention
 * ---------------------
 * Displacement `r` along direction (ℓ, m, n) means the sampled voxel is at
 * (x + r·ℓ, y + r·m, z + r·n) with periodic wrap via `lattice::wrap`. For the
 * edge and diagonal classes the Euclidean distance is r·√2 and r·√3
 * respectively; consumers that want "same Euclidean distance" must rescale r
 * themselves.
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// Per-direction-class correlator result.
struct DirectionalCorrelator {
    std::vector<double> face;      ///< C(r) along ±(1,0,0), averaged over 3 cubic axes
    std::vector<double> edge;      ///< C(r) along ±(1,±1,0), averaged over 6 face-diagonals
    std::vector<double> diagonal;  ///< C(r) along ±(1,±1,±1), averaged over 4 body-diagonals
    int max_r = 0;                 ///< upper bound on index r (exclusive)
};

/// Compute flux-flux correlator C(r) = ⟨J(x)·J(x+r)⟩ separately along the
/// three inequivalent cubic direction classes. Each class averages over its
/// symmetry-equivalent directions and all lattice sites. Periodic wrap.
///
/// @param rb      bridge providing voxels() and lattice()
/// @param max_r   upper bound; defaults to L/2 (half the lattice, symmetry
///                limit for periodic correlators). For diagonal class the
///                effective Euclidean reach is max_r·√3, so consumers may want
///                a smaller max_r to keep Euclidean scales matched.
inline DirectionalCorrelator directional_flux_correlation(
    const RenderBridge& rb, int max_r = -1)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int L = lat.size();
    if (max_r < 0 || max_r > L / 2) max_r = L / 2;

    DirectionalCorrelator out;
    out.max_r = max_r;
    out.face.assign(max_r, 0.0);
    out.edge.assign(max_r, 0.0);
    out.diagonal.assign(max_r, 0.0);

    // 3 face directions: (±1,0,0) is equivalent to (1,0,0) under averaging,
    // so we sample the 3 positive cubic axes only.
    const int face_dirs[3][3] = {
        {1, 0, 0}, {0, 1, 0}, {0, 0, 1}
    };
    // 6 edge directions: (1,1,0), (1,-1,0), (1,0,1), (1,0,-1), (0,1,1), (0,1,-1).
    // The negatives (-1,-1,0), etc. are equivalent to these under translation.
    const int edge_dirs[6][3] = {
        {1, 1, 0}, {1, -1, 0},
        {1, 0, 1}, {1, 0, -1},
        {0, 1, 1}, {0, 1, -1}
    };
    // 4 body-diagonal directions: (1,1,1), (1,1,-1), (1,-1,1), (1,-1,-1).
    const int diag_dirs[4][3] = {
        {1, 1, 1}, {1, 1, -1}, {1, -1, 1}, {1, -1, -1}
    };

    auto accumulate = [&](const int (*dirs)[3], int n_dirs, std::vector<double>& C) {
        std::vector<long long> counts(max_r, 0);
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    const int idx0 = lat.index(x, y, z);
                    const Vec3& J0 = vox[idx0].flux;

                    for (int r = 0; r < max_r; ++r) {
                        for (int d = 0; d < n_dirs; ++d) {
                            const int dx = dirs[d][0], dy = dirs[d][1], dz = dirs[d][2];
                            const int xr = lat.wrap(x + r * dx);
                            const int yr = lat.wrap(y + r * dy);
                            const int zr = lat.wrap(z + r * dz);
                            const int idx_r = lat.index(xr, yr, zr);
                            C[r] += J0.dot(vox[idx_r].flux);
                            counts[r]++;
                        }
                    }
                }
            }
        }
        for (int r = 0; r < max_r; ++r) {
            if (counts[r] > 0) C[r] /= static_cast<double>(counts[r]);
        }
    };

    accumulate(face_dirs, 3, out.face);
    accumulate(edge_dirs, 6, out.edge);
    accumulate(diag_dirs, 4, out.diagonal);
    return out;
}

/// Fit result from a single direction-class correlator.
struct CorrelatorFit {
    double xi = 0.0;       ///< screening length (exponential-decay constant)
    double amplitude = 0.0;///< C(0) estimate (or best-fit A)
    double r2 = 0.0;       ///< goodness-of-fit (Pearson R² over log-residuals)
    int    n_points = 0;   ///< points actually used in fit
    bool   valid = false;  ///< true iff fit converged and ξ is finite positive
};

/// Fit C(r) = A · exp(−r/ξ) over a range [r_min, r_max). Uses linear
/// regression on ln|C(r)| vs r. Samples with C(r) ≤ 0 or very small (below
/// 1e-12·|C(0)|) are dropped to stabilise the log. Requires ≥ 3 usable
/// samples to return valid=true.
inline CorrelatorFit fit_exponential(
    const std::vector<double>& C, int r_min = 2, int r_max = -1)
{
    CorrelatorFit fit;
    const int N = static_cast<int>(C.size());
    if (r_max < 0 || r_max > N) r_max = N;
    if (r_min < 0) r_min = 0;
    if (r_max - r_min < 3) return fit;

    const double C0 = std::abs(C.empty() ? 0.0 : C[0]);
    if (C0 < 1e-30) return fit;

    // Accumulate linear regression of ln|C(r)| = a - r/ξ  =>  slope = -1/ξ
    double sum_r = 0.0, sum_y = 0.0, sum_rr = 0.0, sum_ry = 0.0, sum_yy = 0.0;
    int n = 0;
    for (int r = r_min; r < r_max; ++r) {
        const double val = C[r];
        if (val <= 0.0) continue;
        if (val < 1e-12 * C0) continue;
        const double y = std::log(val);
        const double rd = static_cast<double>(r);
        sum_r += rd;
        sum_y += y;
        sum_rr += rd * rd;
        sum_ry += rd * y;
        sum_yy += y * y;
        ++n;
    }
    if (n < 3) return fit;

    const double denom = n * sum_rr - sum_r * sum_r;
    if (std::abs(denom) < 1e-30) return fit;
    const double slope = (n * sum_ry - sum_r * sum_y) / denom;
    const double intercept = (sum_y - slope * sum_r) / n;

    if (slope >= 0.0) return fit;  // correlator should decay for a screening length to exist
    const double xi = -1.0 / slope;
    if (!std::isfinite(xi) || xi <= 0.0) return fit;

    // Pearson R² on the log-linear fit
    const double ybar = sum_y / n;
    double ss_tot = 0.0, ss_res = 0.0;
    for (int r = r_min; r < r_max; ++r) {
        const double val = C[r];
        if (val <= 0.0 || val < 1e-12 * C0) continue;
        const double y = std::log(val);
        const double yhat = intercept + slope * static_cast<double>(r);
        ss_tot += (y - ybar) * (y - ybar);
        ss_res += (y - yhat) * (y - yhat);
    }
    const double r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;

    fit.xi = xi;
    fit.amplitude = std::exp(intercept);
    fit.r2 = r2;
    fit.n_points = n;
    fit.valid = true;
    return fit;
}

/// Anisotropy diagnostics extracted from a DirectionalCorrelator at a given
/// reference scale.
struct AnisotropyDiagnostic {
    CorrelatorFit fit_face;
    CorrelatorFit fit_edge;
    CorrelatorFit fit_diagonal;
    /// δ = (ξ_face − ξ_diag) / ξ̄ where ξ̄ = mean of face and diagonal.
    /// Zero means isotropic; nonzero means the correlator's decay scale
    /// depends on which lattice direction it is measured along.
    double delta = 0.0;
    /// Pointwise amplitude residual at r = r_ref (defined by caller):
    /// residual = |C_face(r) − C_diag(r)| / max(|C̄(r)|, 1e-30)
    double pointwise_residual = 0.0;
    bool valid = false;
};

/// Compute δ and the pointwise residual at r_ref from a DirectionalCorrelator.
/// The fit windows default to [r_min = 2, r_max = max_r] which drops the
/// r = 0, 1 points (contact + nearest-neighbor dominated by shape of the
/// initial configuration, not by screening).
inline AnisotropyDiagnostic diagnose_anisotropy(
    const DirectionalCorrelator& dc,
    int r_ref,
    int r_min = 2, int r_max = -1)
{
    AnisotropyDiagnostic out;
    out.fit_face     = fit_exponential(dc.face,     r_min, r_max);
    out.fit_edge     = fit_exponential(dc.edge,     r_min, r_max);
    out.fit_diagonal = fit_exponential(dc.diagonal, r_min, r_max);

    if (out.fit_face.valid && out.fit_diagonal.valid) {
        const double xibar = 0.5 * (out.fit_face.xi + out.fit_diagonal.xi);
        if (xibar > 0.0) {
            out.delta = (out.fit_face.xi - out.fit_diagonal.xi) / xibar;
            out.valid = true;
        }
    }

    if (r_ref >= 0 && r_ref < dc.max_r) {
        const double cf = dc.face[r_ref];
        const double cd = dc.diagonal[r_ref];
        const double cbar = 0.5 * (cf + cd);
        out.pointwise_residual = std::abs(cf - cd) / std::max(std::abs(cbar), 1e-30);
    }

    return out;
}

}  // namespace eft
}  // namespace ftd

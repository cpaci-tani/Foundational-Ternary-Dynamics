#pragma once
/**
 * @file ftd/eft/lorentz_recovery.h
 * @brief Lorentz-covariance recovery diagnostic (EFT Recovery Program, Phase 1B).
 *
 * Physics motivation
 * ------------------
 * A Lorentz-covariant continuum theory has a temporal two-point function and a
 * spatial two-point function related by Wick rotation / rescaling of time:
 *
 *     C_t(τ) = ⟨J(τ, 0) · J(0, 0)⟩
 *     C_s(r) = ⟨J(0, r) · J(0, 0)⟩
 *
 * Under the substitution τ → c · τ (with c the speed limit; c = 1/√D on a
 * D-dimensional cubic lattice via CFL stability), Lorentz covariance requires
 * C_t(cτ) ≈ C_s(τ) in the IR.
 *
 * This module:
 *   1. Samples a *vector* time series of flux at a fixed voxel (the existing
 *      `temporal_autocorrelation` works on scalars only).
 *   2. Computes the temporal flux-flux correlator C_t(τ).
 *   3. Rescales τ → c·τ and compares to a spatial correlator C_s(r).
 *   4. Fits the residual |C_t(cτ) − C_s(τ)| / |C_s(τ)| to a power law
 *      residual ∝ (a/r)^q, extracting the Lorentz-recovery exponent q.
 *
 * Pre-registered expectations live in SPEC_EFT_RECOVERY_PROGRAM.md §4.2.
 *
 * Epistemic status
 * ----------------
 * Pure measurement. The rescaling constant c is imported from the CFL-stability
 * theorem (C_SPEED = 1/√3 in ftd/constants.h) and is [DERIVED] from the lattice
 * update rule. The residual exponent q is [MEASURED].
 *
 * Design
 * ------
 * The temporal correlator requires snapshots taken at successive ticks. To
 * keep correlations.h lean we hold time series in a caller-owned
 * std::vector<Vec3>; utilities in this header only consume that container.
 * The sampling loop (caller runs `rb.tick()` and `sample_flux_at(rb, pos)`)
 * stays in the caller so this header has no dependency on engine dynamics.
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// Sample the flux vector at voxel (x,y,z). Small helper so callers avoid
/// reaching into rb.lattice()/rb.voxels() themselves.
inline Vec3 sample_flux_at(const RenderBridge& rb, int x, int y, int z) {
    return rb.voxels()[rb.lattice().index(x, y, z)].flux;
}

/// C_t(τ) = ⟨J(t+τ, 0)·J(t, 0)⟩_t, connected (subtract ⟨J⟩ · ⟨J⟩).
/// Input: time series of flux vectors sampled at successive ticks.
///
/// @param series    vector time series (one Vec3 per tick)
/// @param max_tau   upper bound on τ (default T/2)
inline std::vector<double> temporal_flux_correlation(
    const std::vector<Vec3>& series, int max_tau = -1)
{
    const int T = static_cast<int>(series.size());
    if (max_tau < 0 || max_tau > T / 2) max_tau = T / 2;

    // Mean
    Vec3 mean{0.0, 0.0, 0.0};
    for (const auto& v : series) { mean.x += v.x; mean.y += v.y; mean.z += v.z; }
    mean.x /= T; mean.y /= T; mean.z /= T;

    std::vector<double> Ct(max_tau, 0.0);
    for (int tau = 0; tau < max_tau; ++tau) {
        double sum = 0.0;
        const int count = T - tau;
        for (int t = 0; t < count; ++t) {
            const Vec3& a = series[t];
            const Vec3& b = series[t + tau];
            sum += (a.x - mean.x) * (b.x - mean.x)
                 + (a.y - mean.y) * (b.y - mean.y)
                 + (a.z - mean.z) * (b.z - mean.z);
        }
        Ct[tau] = sum / static_cast<double>(count);
    }
    return Ct;
}

/// Result of comparing temporal C_t(cτ) against spatial C_s(r):
/// the rescaled residual at each r and, optionally, a power-law fit of
/// residual decay.
struct LorentzCollapse {
    /// For each integer r ∈ [0, r_max): linearly-interpolated C_t at τ = r/c,
    /// so both series are evaluated at the same "Euclidean distance" r.
    std::vector<double> C_t_rescaled;
    /// Direct copy of the spatial correlator on [0, r_max).
    std::vector<double> C_s;
    /// residual[r] = |C_t(cr) − C_s(r)| / max(|C_s(r)|, eps)
    std::vector<double> residual;
    /// Fit of residual(r) = B · r^(−q) over [r_min, r_max). Only valid if
    /// residuals are strictly positive and decay monotonically.
    double q = 0.0;
    double r2 = 0.0;
    bool   fit_valid = false;
};

/// Resample the temporal correlator at times τ = r/c for integer r, using
/// linear interpolation, and produce a pointwise residual against C_s(r).
///
/// @param C_t        temporal correlator (indexed by tick τ)
/// @param C_s        spatial correlator  (indexed by lattice distance r)
/// @param c          rescaling speed (default 1/√3, the CFL lattice speed limit)
/// @param r_min,r_max range over which to fit residual power law
/// @param normalize  if true, divide each correlator by its own C(0) before
///                   comparing. This isolates the *shape* of the correlator
///                   (the Lorentz-covariance test proper) from the overall
///                   amplitude, which for a standing-wave plane-wave
///                   initial condition oscillates with time while the
///                   temporal correlator averages over it. Defaults to true.
inline LorentzCollapse compare_correlators(
    const std::vector<double>& C_t,
    const std::vector<double>& C_s,
    double c = 1.0 / 1.7320508075688772,  // 1/√3
    int r_min = 4, int r_max = -1,
    bool normalize = true)
{
    LorentzCollapse out;
    const int R = static_cast<int>(C_s.size());
    const int Tt = static_cast<int>(C_t.size());
    const int rmax = static_cast<int>(static_cast<double>(Tt - 1) * c);
    const int R_eff = std::min(R, rmax);
    if (r_max < 0 || r_max > R_eff) r_max = R_eff;

    const double Cs0 = (normalize && !C_s.empty() && std::abs(C_s[0]) > 1e-30)
                       ? C_s[0] : 1.0;
    const double Ct0 = (normalize && !C_t.empty() && std::abs(C_t[0]) > 1e-30)
                       ? C_t[0] : 1.0;

    out.C_s.assign(R_eff, 0.0);
    out.C_t_rescaled.assign(R_eff, 0.0);
    out.residual.assign(R_eff, 0.0);

    for (int r = 0; r < R_eff; ++r) {
        const double tau = static_cast<double>(r) / c;
        const int ti = static_cast<int>(std::floor(tau));
        const double frac = tau - static_cast<double>(ti);
        double interp;
        if (ti + 1 < Tt) {
            interp = (1.0 - frac) * C_t[ti] + frac * C_t[ti + 1];
        } else if (ti < Tt) {
            interp = C_t[ti];
        } else {
            interp = 0.0;
        }
        const double cs_norm = C_s[r] / Cs0;
        const double ct_norm = interp  / Ct0;
        out.C_s[r]          = cs_norm;
        out.C_t_rescaled[r] = ct_norm;

        // Use a denominator of max(|C_s|, small) so residuals are bounded
        // near zero-crossings; supplement with |C_s(0)|=1 (after norm) as
        // a natural floor. When both correlators have been normalised to
        // 1 at r=0, a meaningful scale is ~1, so the floor is 0.01 = 1%.
        const double denom = std::max(std::abs(cs_norm), 1e-2);
        out.residual[r] = std::abs(ct_norm - cs_norm) / denom;
    }

    // Fit residual(r) = B · r^(−q)  ⇒  ln residual = ln B − q · ln r
    if (r_max - r_min >= 3) {
        double sum_x = 0.0, sum_y = 0.0, sum_xx = 0.0, sum_xy = 0.0, sum_yy = 0.0;
        int n = 0;
        for (int r = r_min; r < r_max; ++r) {
            const double res = out.residual[r];
            if (!(res > 0.0) || !(std::isfinite(res))) continue;
            if (res > 1e6) continue;
            const double x = std::log(static_cast<double>(r));
            const double y = std::log(res);
            sum_x += x; sum_y += y; sum_xx += x * x; sum_xy += x * y; sum_yy += y * y;
            ++n;
        }
        if (n >= 3) {
            const double denom = n * sum_xx - sum_x * sum_x;
            if (std::abs(denom) > 1e-30) {
                const double slope = (n * sum_xy - sum_x * sum_y) / denom;
                const double intercept = (sum_y - slope * sum_x) / n;
                const double ybar = sum_y / n;
                double ss_tot = 0.0, ss_res = 0.0;
                for (int r = r_min; r < r_max; ++r) {
                    const double res = out.residual[r];
                    if (!(res > 0.0) || !(std::isfinite(res)) || res > 1e6) continue;
                    const double y = std::log(res);
                    const double yhat = intercept + slope * std::log(static_cast<double>(r));
                    ss_tot += (y - ybar) * (y - ybar);
                    ss_res += (y - yhat) * (y - yhat);
                }
                out.q = -slope;           // residual ∝ r^(−q) so slope = −q
                out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
                out.fit_valid = std::isfinite(out.q) && out.q > 0.0;
            }
        }
    }

    return out;
}

}  // namespace eft
}  // namespace ftd

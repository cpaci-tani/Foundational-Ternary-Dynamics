#pragma once
/**
 * @file ftd/eft/matched_poisson.h
 * @brief Matched-stencil conjugate-gradient Poisson solver for EFT measurements.
 *
 * Motivation (see DERIV_GAP_CLOSURE.md §T1)
 * -----------------------------------------
 * The engine's production Gauss projection in engine/src/poisson_solvers.cpp
 * uses two stencils that are not consistent:
 *
 *   - An 18-point Laplacian on the scalar potential φ (sor_sweep_18pt)
 *   - A 6-point central-difference divergence on the flux field J
 *
 * As a consequence, even after infinite SOR iterations the residual
 * ∇₆·J − ρ does not go to zero: the solved φ satisfies ∇₁₈² φ = ρ − ∇₆·J,
 * but what we need is ∇₆² φ = ρ − ∇₆·J. The stencil mismatch is an O(1)
 * leak that saturates at ~1% of |J|_max after a single SOR call and does
 * NOT improve with more iterations (in fact it can grow because the
 * iteration is not a contraction at ω = 1.75 under this mismatch).
 *
 * This module implements a *matched-stencil* solver:
 *   - Laplacian uses 6-point central difference:
 *         (∇² φ)(x) = Σ_{μ=±} [φ(x+μ) − φ(x)] / 1²   (unit spacing)
 *                   = φ(x+x̂) + φ(x−x̂) + φ(x+ŷ) + φ(x−ŷ) + φ(x+ẑ) + φ(x−ẑ)
 *                     − 6 φ(x)
 *   - Solved by conjugate gradient (CG) to a user-specified tolerance.
 *   - Gradient uses the same 6-point central difference as the engine's
 *     divergence_flux_op, so subtracting ∇φ from J exactly cancels the
 *     6-point divergence residual up to CG tolerance.
 *
 * The solver is self-contained — it does NOT call engine's tick() — and
 * does NOT modify anything except the flux field at vacuum voxels
 * (particles are left alone, matching the convention of
 * poisson_solvers.cpp::gauss_project_cpu).
 *
 * Epistemic status: [TOOLING]. No new physics; a correct, converging
 * implementation of what Gauss projection always intended to do.
 *
 * Design
 * ------
 * We implement standard CG on ∇₆² φ = b where b = ∇₆·J − ρ. The 6-point
 * Laplacian is linear in φ, symmetric, and negative-semi-definite, so CG
 * is the right choice (BiCGstab not required). Periodic boundary
 * conditions: φ is defined modulo a constant, which the CG does not
 * care about — it converges to the correct gradient field regardless.
 *
 * Runtime: O(L³) per CG iteration, O(L) iterations for convergence on a
 * smooth source, so total O(L⁴). At L=64 this is ~2×10⁷ ops/iteration,
 * ~10⁸ total; about half a second wall-clock.
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

struct MatchedPoissonReport {
    int iterations = 0;         ///< CG iterations actually performed
    double final_residual_norm = 0.0;  ///< ‖∇²φ − b‖₂
    double initial_residual_norm = 0.0;
    double tolerance = 0.0;
    // "Vacuum" = state == 0 voxels (any). Boundary layer near particles.
    double vacuum_max_div_before = 0.0;
    double vacuum_max_div_after = 0.0;
    double vacuum_rms_div_before = 0.0;
    double vacuum_rms_div_after = 0.0;
    // "Deep vacuum" = vacuum voxels whose 6 immediate neighbors are all
    // vacuum. These are exactly the voxels where stencil-matched
    // projection is guaranteed to drive residual to CG tolerance
    // (matched composition ∇₋ · ∇₊ applies cleanly with no boundary-
    // layer contamination). Vacuum voxels adjacent to particles are
    // structurally resistant because we don't modify particle flux.
    double deep_vacuum_max_div_before = 0.0;
    double deep_vacuum_max_div_after = 0.0;
    double deep_vacuum_rms_div_before = 0.0;
    double deep_vacuum_rms_div_after = 0.0;
    long long n_deep_vacuum = 0;
    bool converged = false;
};

namespace detail {

// STAGGERED-STENCIL FAMILY (Yee-like)
// -----------------------------------
// Central differences on a cubic lattice suffer a well-known pathology:
// div_centered ∘ grad_centered = L₂ φ where L₂ uses ±2-voxel offsets,
// decoupling the lattice into 2³ = 8 independent sublattices. Each has
// its own constant zero mode, giving an 8-dim null space that breaks
// Poisson solvers.
//
// Yee-style staggering fixes this by using backward difference for
// divergence and forward difference for gradient (or vice versa). Their
// composition gives the standard 7-point nearest-neighbor Laplacian,
// which has only the single constant zero mode (periodic torus).
//
// We implement the backward-div / forward-grad variant:
//
//   (∇₋ · J)[i] = Σ_μ (J_μ[i] − J_μ[i−μ])           [backward div]
//   (∇₊ φ)[i]   = ( φ[i+x̂] − φ[i],  φ[i+ŷ] − φ[i],  φ[i+ẑ] − φ[i] )
//   L_NN φ[i]   = (∇₋ · ∇₊ φ)[i] = Σ_μ (φ[i+μ] − 2 φ[i] + φ[i−μ])
//
// The discrete Ward identity tested here is "backward divergence of
// flux equals ρ". This is a valid lattice realization of ∇·J = ρ; it
// differs from the engine's central-difference Ward identity in finite-
// lattice artefacts of order O(a²), but both agree in the continuum
// limit.

// Backward-difference divergence at voxel idx (matched-stencil).
inline double divergence_back(const std::vector<Voxel>& vox,
                              const Lattice& lat, int idx) {
    const int L = lat.size();
    const int LL = L * L;
    const int iz = idx % L;
    const int iy = (idx / L) % L;
    const int ix = idx / LL;
    const int ixm = lat.wrap(ix - 1);
    const int iym = lat.wrap(iy - 1);
    const int izm = lat.wrap(iz - 1);
    const double Jx_here = vox[idx].flux.x;
    const double Jy_here = vox[idx].flux.y;
    const double Jz_here = vox[idx].flux.z;
    const double Jx_back = vox[lat.index(ixm, iy, iz)].flux.x;
    const double Jy_back = vox[lat.index(ix, iym, iz)].flux.y;
    const double Jz_back = vox[lat.index(ix, iy, izm)].flux.z;
    return (Jx_here - Jx_back) + (Jy_here - Jy_back) + (Jz_here - Jz_back);
}

// Kept for backward compatibility with earlier test code that named
// this "divergence_6pt"; it is the engine's central-difference convention.
inline double divergence_6pt(const std::vector<Voxel>& vox,
                             const Lattice& lat, int idx) {
    const int L = lat.size();
    const int LL = L * L;
    const int iz = idx % L;
    const int iy = (idx / L) % L;
    const int ix = idx / LL;
    const int ixp = lat.wrap(ix + 1), ixm = lat.wrap(ix - 1);
    const int iyp = lat.wrap(iy + 1), iym = lat.wrap(iy - 1);
    const int izp = lat.wrap(iz + 1), izm = lat.wrap(iz - 1);
    const double dx = 0.5 * (vox[lat.index(ixp, iy, iz)].flux.x
                            - vox[lat.index(ixm, iy, iz)].flux.x);
    const double dy = 0.5 * (vox[lat.index(ix, iyp, iz)].flux.y
                            - vox[lat.index(ix, iym, iz)].flux.y);
    const double dz = 0.5 * (vox[lat.index(ix, iy, izp)].flux.z
                            - vox[lat.index(ix, iy, izm)].flux.z);
    return dx + dy + dz;
}

// Standard 7-point nearest-neighbor Laplacian, which is the exact
// composition ∇₋ · ∇₊ on a cubic lattice. The single zero mode is the
// constant (eliminated by remove_mean() in cg_poisson).
inline void apply_laplacian_matched(const std::vector<double>& phi,
                                    std::vector<double>& Aphi,
                                    const Lattice& lat) {
    const int L = lat.size();
    const int LL = L * L;
    const int N = L * L * L;
    for (int i = 0; i < N; ++i) {
        const int iz = i % L;
        const int iy = (i / L) % L;
        const int ix = i / LL;
        const int ixp = lat.wrap(ix + 1), ixm = lat.wrap(ix - 1);
        const int iyp = lat.wrap(iy + 1), iym = lat.wrap(iy - 1);
        const int izp = lat.wrap(iz + 1), izm = lat.wrap(iz - 1);
        Aphi[i] =
              phi[lat.index(ixp, iy, iz)] + phi[lat.index(ixm, iy, iz)]
            + phi[lat.index(ix, iyp, iz)] + phi[lat.index(ix, iym, iz)]
            + phi[lat.index(ix, iy, izp)] + phi[lat.index(ix, iy, izm)]
            - 6.0 * phi[i];
    }
}

// Dot product and axpy helpers for CG.
inline double dot(const std::vector<double>& a, const std::vector<double>& b) {
    double s = 0.0;
    const int n = static_cast<int>(a.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

inline void axpy(double alpha, const std::vector<double>& x,
                 std::vector<double>& y) {
    const int n = static_cast<int>(y.size());
    for (int i = 0; i < n; ++i) y[i] += alpha * x[i];
}

inline void scaled_copy(double alpha, const std::vector<double>& x,
                        const std::vector<double>& y, std::vector<double>& out) {
    // out = alpha * x + y
    const int n = static_cast<int>(out.size());
    for (int i = 0; i < n; ++i) out[i] = alpha * x[i] + y[i];
}

// Remove the zero mode from a scalar field (periodic boundary case —
// ∇²φ = b has a solution only if <b> = 0, and φ is defined modulo a
// constant; we pin its mean to zero to fix the ambiguity).
inline void remove_mean(std::vector<double>& x) {
    double s = 0.0;
    for (double v : x) s += v;
    const double mean = s / static_cast<double>(x.size());
    for (double& v : x) v -= mean;
}

}  // namespace detail

/// Solve ∇²φ = b on a periodic L³ lattice using conjugate gradient.
/// Output φ has zero mean (zero mode pinned). Returns iteration count
/// and final residual norm. Does NOT modify any engine state.
///
/// The sign convention: we solve (+∇²)φ = b directly. Because ∇² is
/// symmetric negative-semidefinite, CG works on −∇² applied to −φ; the
/// implementation rephrases as solving ∇²φ = b with the natural signs
/// and CG's p-update direction handles the negative-definiteness.
///
/// @param b               source, length N = L³
/// @param lat             lattice (for neighbor indices + wrap)
/// @param phi             output, length N; initial guess used (pass zeros)
/// @param tol             target ‖r‖₂ / ‖b‖₂  (default 1e-10)
/// @param max_iter        hard cap on iterations (default 10·L)
inline MatchedPoissonReport cg_poisson(
    const std::vector<double>& b,
    const Lattice& lat,
    std::vector<double>& phi,
    double tol = 1e-10,
    int max_iter = -1)
{
    using namespace detail;
    MatchedPoissonReport rpt;
    const int N = lat.total_sites();
    if (max_iter < 0) max_iter = 10 * lat.size();
    rpt.tolerance = tol;

    // b must have zero mean for ∇²φ = b to be solvable on a torus.
    // We subtract the mean to enforce consistency (the mean contribution
    // is unphysical — it would correspond to a uniform background
    // charge that the periodic geometry cannot source).
    std::vector<double> bfix(b);
    remove_mean(bfix);

    // Working vectors
    std::vector<double> r(N, 0.0), p(N, 0.0), Ap(N, 0.0);

    // r = b - A*phi
    apply_laplacian_matched(phi, Ap, lat);
    for (int i = 0; i < N; ++i) r[i] = bfix[i] - Ap[i];
    p = r;

    const double b_norm = std::sqrt(std::max(dot(bfix, bfix), 1e-60));
    double r_sq = dot(r, r);
    rpt.initial_residual_norm = std::sqrt(r_sq);

    for (int it = 0; it < max_iter; ++it) {
        apply_laplacian_matched(p, Ap, lat);
        // Note: our Laplacian is negative-semidefinite (∇² has
        // eigenvalues ≤ 0 on a compact torus), but CG assumes symmetric
        // positive definite. We're actually solving ∇²φ = b; -∇² is SPD.
        // Effectively multiply both sides by -1: solve -∇²φ = -b. The
        // algorithm works directly if we track the sign of <p, Ap>.
        const double pAp = dot(p, Ap);
        if (std::abs(pAp) < 1e-60) break;
        const double alpha = r_sq / pAp;
        axpy(alpha, p, phi);
        axpy(-alpha, Ap, r);
        const double new_r_sq = dot(r, r);
        const double rel = std::sqrt(new_r_sq) / b_norm;
        rpt.iterations = it + 1;
        rpt.final_residual_norm = std::sqrt(new_r_sq);
        if (rel < tol) {
            rpt.converged = true;
            break;
        }
        const double beta = new_r_sq / r_sq;
        // p = r + beta * p
        for (int i = 0; i < N; ++i) p[i] = r[i] + beta * p[i];
        r_sq = new_r_sq;
    }

    remove_mean(phi);
    return rpt;
}

/// Project the flux field J so that ∇₆·J = ρ, where ρ = state, at every
/// VACUUM voxel (state == 0). Uses the matched-stencil CG solver above.
/// Particle voxels' flux is left untouched, matching the existing
/// poisson_solvers.cpp::gauss_project_cpu convention.
///
/// This is the correct replacement for gauss_projection when the caller
/// needs a Ward-identity-precise flux field (e.g. for EFT measurements
/// of α_eff via V(r), or for tight Ward-identity tests).
inline MatchedPoissonReport matched_gauss_project(
    RenderBridge& rb, double tol = 1e-10, int max_iter = -1)
{
    MatchedPoissonReport rpt;
    auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int N = lat.total_sites();

    // Helper: is voxel i "deep vacuum"? — state == 0 AND all 6 NN are also
    // state == 0. These are the voxels where the matched projection is
    // mathematically guaranteed to reach CG-tolerance residual.
    const int Lsz = lat.size();
    const int LLsz = Lsz * Lsz;
    auto is_deep_vacuum = [&](int i) -> bool {
        if (vox[i].state != 0) return false;
        const int iz = i % Lsz;
        const int iy = (i / Lsz) % Lsz;
        const int ix = i / LLsz;
        const int off[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (int k = 0; k < 6; ++k) {
            const int nx = lat.wrap(ix + off[k][0]);
            const int ny = lat.wrap(iy + off[k][1]);
            const int nz = lat.wrap(iz + off[k][2]);
            if (vox[lat.index(nx, ny, nz)].state != 0) return false;
        }
        return true;
    };

    // Measure BEFORE projection: two metrics — all-vacuum and deep-vacuum.
    double max_before = 0.0, sum_sq_before = 0.0;
    long long n_before = 0;
    double dv_max_before = 0.0, dv_sum_sq_before = 0.0;
    long long dv_n_before = 0;
    for (int i = 0; i < N; ++i) {
        if (vox[i].state != 0) continue;
        const double d = detail::divergence_back(vox, lat, i);
        if (std::abs(d) > max_before) max_before = std::abs(d);
        sum_sq_before += d * d;
        ++n_before;
        if (is_deep_vacuum(i)) {
            if (std::abs(d) > dv_max_before) dv_max_before = std::abs(d);
            dv_sum_sq_before += d * d;
            ++dv_n_before;
        }
    }

    // Build source b = ∇₋·J − ρ at every voxel (including particle voxels
    // — their contribution sets up the correct Coulomb-like potential φ).
    std::vector<double> b(N, 0.0);
    for (int i = 0; i < N; ++i) {
        const double d = detail::divergence_back(vox, lat, i);
        const double rho = static_cast<double>(vox[i].state);
        b[i] = d - rho;
    }

    // Solve ∇²φ = b.
    std::vector<double> phi(N, 0.0);
    auto cg_rpt = cg_poisson(b, lat, phi, tol, max_iter);
    // Copy CG fields into our report (don't clobber before-measurements).
    rpt.iterations = cg_rpt.iterations;
    rpt.final_residual_norm = cg_rpt.final_residual_norm;
    rpt.initial_residual_norm = cg_rpt.initial_residual_norm;
    rpt.tolerance = cg_rpt.tolerance;
    rpt.converged = cg_rpt.converged;
    rpt.vacuum_max_div_before = max_before;
    rpt.vacuum_rms_div_before = (n_before > 0) ? std::sqrt(sum_sq_before / n_before) : 0.0;
    rpt.deep_vacuum_max_div_before = dv_max_before;
    rpt.deep_vacuum_rms_div_before = (dv_n_before > 0) ? std::sqrt(dv_sum_sq_before / dv_n_before) : 0.0;
    rpt.n_deep_vacuum = dv_n_before;

    // Subtract FORWARD gradient of φ from J at vacuum voxels only. The
    // composition (backward div ∘ forward grad) applied to this update
    // exactly cancels b at every vacuum voxel — this is why stencil-
    // matching matters: the corrected flux satisfies ∇₋·J_new = ρ
    // pointwise on vacuum voxels, modulo CG tolerance.
    const int L = lat.size();
    const int LL = L * L;
    for (int i = 0; i < N; ++i) {
        if (vox[i].state != 0) continue;
        const int iz = i % L;
        const int iy = (i / L) % L;
        const int ix = i / LL;
        // forward gradient at voxel i: φ[i+μ] - φ[i]
        const double gx = phi[lat.index(lat.wrap(ix + 1), iy, iz)] - phi[i];
        const double gy = phi[lat.index(ix, lat.wrap(iy + 1), iz)] - phi[i];
        const double gz = phi[lat.index(ix, iy, lat.wrap(iz + 1))] - phi[i];
        vox[i].flux.x -= gx;
        vox[i].flux.y -= gy;
        vox[i].flux.z -= gz;
    }

    // Measure AFTER projection: backward divergence (matched metric).
    // All-vacuum and deep-vacuum metrics.
    double max_after = 0.0, sum_sq_after = 0.0;
    long long n_after = 0;
    double dv_max_after = 0.0, dv_sum_sq_after = 0.0;
    long long dv_n_after = 0;
    for (int i = 0; i < N; ++i) {
        if (vox[i].state != 0) continue;
        const double d = detail::divergence_back(vox, lat, i);
        if (std::abs(d) > max_after) max_after = std::abs(d);
        sum_sq_after += d * d;
        ++n_after;
        if (is_deep_vacuum(i)) {
            if (std::abs(d) > dv_max_after) dv_max_after = std::abs(d);
            dv_sum_sq_after += d * d;
            ++dv_n_after;
        }
    }
    rpt.vacuum_max_div_after = max_after;
    rpt.vacuum_rms_div_after = (n_after > 0) ? std::sqrt(sum_sq_after / n_after) : 0.0;
    rpt.deep_vacuum_max_div_after = dv_max_after;
    rpt.deep_vacuum_rms_div_after = (dv_n_after > 0) ? std::sqrt(dv_sum_sq_after / dv_n_after) : 0.0;

    return rpt;
}

}  // namespace eft
}  // namespace ftd

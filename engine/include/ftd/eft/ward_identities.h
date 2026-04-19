#pragma once
/**
 * @file ftd/eft/ward_identities.h
 * @brief Ward-identity diagnostics for the EFT Recovery Program (Phase 1C).
 *
 * Physics motivation
 * ------------------
 * In any theory with a conserved current J^μ, the Ward identity
 *
 *     ∂_μ ⟨J^μ(x)⟩ = ⟨ρ(x)⟩
 *
 * is a consequence of gauge invariance (continuity of charge). On the FTD
 * lattice the Gauss constraint ∇·J = ρ is *enforced* by the
 * `gauss_projection` phase — so the zeroth-order identity holds to machine
 * precision by construction. The real EFT question is whether *composite
 * operators* also satisfy Ward identities at the permille level. If they do,
 * gauge invariance lifts from "imposed on the flux field" to "a consequence
 * of the measurement algebra."
 *
 * This module tests three Ward identities across four configurations:
 *
 *   I1 (Gauss):           max_x |∇·J(x) − ρ(x)|                < ε_gauss (≲ 10⁻⁸)
 *   I2 (continuity):      max_x |∂_t ρ(x) + ∇·J(x)|             < ε_cont  (≲ 10⁻⁶)
 *   I3 (composite JJ):    |⟨(∇·J)(x) · J^ν(y)⟩ − ⟨ρ(x) · J^ν(y)⟩| / scale  < ε_comp (≲ 10⁻³)
 *
 * Vertex-level Γ_μ(p,p) = ∂Σ/∂p^μ is explicitly outside scope — it requires
 * lattice fermion propagators that do not yet exist in the engine.
 *
 * Pre-registered expectations: SPEC_EFT_RECOVERY_PROGRAM.md §4.3.
 *
 * Epistemic status
 * ----------------
 * All measurement. The identities are imported from the standard
 * gauge-theory Ward identity; FTD's claim is only that the *lattice*
 * realises them. Tag: [MEASUREMENT].
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// Result of one Ward-identity measurement.
struct WardResult {
    double max_abs_violation = 0.0;  ///< sup over all x of |identity lhs − rhs|
    double rms_violation = 0.0;      ///< √⟨violation²⟩ over all voxels
    double scale = 0.0;              ///< reference scale |J|_max or |ρ|_max used for reporting
    long long n_samples = 0;         ///< number of voxels contributing to the average
};

/// Gauss identity test:  ∇·J(x)  ?=  ρ(x)  for every VACUUM voxel (s = 0).
/// On the FTD lattice, ρ(x) = s(x) (the ternary state acts as the charge
/// density per the gauge-emergence derivation). The `gauss_projection`
/// toggle's SOR solver enforces this by modifying only vacuum flux —
/// particle voxels (s ≠ 0) are intentionally left alone, so their local
/// ∇·J need not equal s (see engine/src/poisson_solvers.cpp, the
/// `if (voxels[i].state != 0) continue;` on the grad-φ writeback loop).
///
/// Hence the correct EFT Ward test restricts to vacuum voxels. A report on
/// particle-voxel violation is physically meaningful too — it quantifies
/// the "charge concentration" that would have to be smeared to restore the
/// pointwise identity — but is not a Ward violation of the projected EFT.
inline WardResult gauss_identity(const RenderBridge& rb, bool vacuum_only = true) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int N = lat.total_sites();

    WardResult out;
    double max_abs = 0.0, sum_sq = 0.0, scale = 0.0;
    long long n = 0;
    for (int i = 0; i < N; ++i) {
        if (vacuum_only && vox[i].state != 0) continue;
        const double div = rb.divergence_flux(i);
        const double rho = static_cast<double>(vox[i].state);
        const double v = div - rho;
        const double a = std::abs(v);
        if (a > max_abs) max_abs = a;
        sum_sq += v * v;
        const double fm = std::sqrt(vox[i].flux.dot(vox[i].flux));
        if (fm > scale) scale = fm;
        ++n;
    }
    out.max_abs_violation = max_abs;
    out.rms_violation = (n > 0) ? std::sqrt(sum_sq / static_cast<double>(n)) : 0.0;
    out.scale = scale;
    out.n_samples = n;
    return out;
}

/// Continuity-equation test: ∂_t ρ(x) + ∇·J(x) = 0 between two successive
/// snapshots. Requires the caller to supply ρ(t) and ρ(t+dt) side-by-side.
/// Here we accept ρ as pre-sampled int8 states at the two ticks.
///
/// @param rho_now        length-N state array at current tick
/// @param rho_next       length-N state array at next tick
/// @param divJ_now       length-N sampled divergence at current tick
/// @param dt             tick interval
inline WardResult continuity_identity(
    const std::vector<int8_t>& rho_now,
    const std::vector<int8_t>& rho_next,
    const std::vector<double>& divJ_now,
    double dt)
{
    WardResult out;
    const int N = static_cast<int>(rho_now.size());
    if (N == 0 || static_cast<int>(rho_next.size()) != N ||
        static_cast<int>(divJ_now.size()) != N || dt <= 0.0) {
        return out;
    }
    double max_abs = 0.0, sum_sq = 0.0;
    for (int i = 0; i < N; ++i) {
        const double drho_dt = (static_cast<double>(rho_next[i])
                              - static_cast<double>(rho_now[i])) / dt;
        const double v = drho_dt + divJ_now[i];
        const double a = std::abs(v);
        if (a > max_abs) max_abs = a;
        sum_sq += v * v;
    }
    out.max_abs_violation = max_abs;
    out.rms_violation = std::sqrt(sum_sq / static_cast<double>(N));
    out.n_samples = N;
    return out;
}

/// Composite Ward identity for the correlator ⟨∇·J(x)·J^ν(y)⟩ vs
/// ⟨ρ(x)·J^ν(y)⟩. Averages over all pairs (x, y) at fixed displacement r
/// along the +x axis, then over all ν = x, y, z.
///
/// @param rb      bridge
/// @param r_max   maximum displacement r along +x (default L/2)
inline WardResult composite_ward_identity(const RenderBridge& rb, int r_max = -1) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int L = lat.size();
    if (r_max < 0 || r_max > L / 2) r_max = L / 2;

    WardResult out;
    double max_abs = 0.0;
    double sum_sq = 0.0;
    long long n = 0;
    double scale = 0.0;

    for (int r = 0; r < r_max; ++r) {
        double sum_divJ_J[3] = {0, 0, 0};  // ⟨(∇·J)(x)·J^ν(x+r)⟩ accumulated
        double sum_rho_J[3]  = {0, 0, 0};  // ⟨ρ(x)·J^ν(x+r)⟩ accumulated
        long long npair = 0;
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    const int i0 = lat.index(x, y, z);
                    const int ir = lat.index(lat.wrap(x + r), y, z);
                    const double div_i0 = rb.divergence_flux(i0);
                    const double rho_i0 = static_cast<double>(vox[i0].state);
                    const Vec3& Jr = vox[ir].flux;
                    sum_divJ_J[0] += div_i0 * Jr.x;
                    sum_divJ_J[1] += div_i0 * Jr.y;
                    sum_divJ_J[2] += div_i0 * Jr.z;
                    sum_rho_J[0]  += rho_i0 * Jr.x;
                    sum_rho_J[1]  += rho_i0 * Jr.y;
                    sum_rho_J[2]  += rho_i0 * Jr.z;
                    const double jmag = std::sqrt(Jr.dot(Jr));
                    if (jmag > scale) scale = jmag;
                    ++npair;
                }
            }
        }
        if (npair > 0) {
            for (int nu = 0; nu < 3; ++nu) {
                const double diff = std::abs(sum_divJ_J[nu] - sum_rho_J[nu])
                                  / static_cast<double>(npair);
                if (diff > max_abs) max_abs = diff;
                sum_sq += diff * diff;
                ++n;
            }
        }
    }

    out.max_abs_violation = max_abs;
    out.rms_violation = (n > 0) ? std::sqrt(sum_sq / static_cast<double>(n)) : 0.0;
    out.scale = scale;
    out.n_samples = n;
    return out;
}

/// Utility: snapshot the state field to an int8 vector for continuity tests.
inline std::vector<int8_t> snapshot_state(const RenderBridge& rb) {
    const auto& vox = rb.voxels();
    std::vector<int8_t> s(vox.size());
    for (std::size_t i = 0; i < vox.size(); ++i) s[i] = vox[i].state;
    return s;
}

/// Utility: snapshot ∇·J to a double vector for continuity tests.
inline std::vector<double> snapshot_divergence(const RenderBridge& rb) {
    const auto& vox = rb.voxels();
    std::vector<double> d(vox.size());
    for (std::size_t i = 0; i < vox.size(); ++i)
        d[i] = rb.divergence_flux(static_cast<int>(i));
    return d;
}

}  // namespace eft
}  // namespace ftd

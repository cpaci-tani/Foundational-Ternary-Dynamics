#pragma once
/**
 * @file ftd/eft/operator_spectrum.h
 * @brief Operator-basis and scaling-dimension extraction (EFT Phase 3).
 *
 * Implements the six-operator basis pre-registered in
 * docs/theory/10_eft_program/SPEC_OPERATOR_BASIS.md §2. Each operator is
 * evaluated per voxel using central-difference field operators (already
 * defined in ftd/field_operators.h); the two-point correlator
 * C_O(r) = ⟨O(x) · O(x+r)⟩ − ⟨O⟩² is computed as a function of axis-
 * averaged displacement r (the same convention used in
 * ftd/correlations.h::spatial_flux_correlation).
 *
 * Scaling dimension Δ is extracted by regressing ln|C_O(r)| vs ln r over
 * [r_min, r_max] (reuses the proven regression form from
 * ftd/eft/anisotropy.h::fit_exponential — but there the fit target was
 * exponential exp(-r/ξ), here it is power-law r^(-2Δ), so we reimplement
 * the regression in log-log form rather than log-linear).
 *
 * Epistemic status: pure measurement. No new fit forms, no new physics.
 * Tag: [MEASUREMENT].
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// Canonical operator IDs matching SPEC_OPERATOR_BASIS.md §2.
enum class OpId {
    JJ          = 0,  ///< J · J                        (naive Δ = 2)
    divJ2       = 1,  ///< (∇·J)²                        (naive Δ = 4)
    curlJ2      = 2,  ///< (∇×J) · (∇×J)                (naive Δ = 4)
    JdotDivJ    = 3,  ///< J · ∇(∇·J)                    (naive Δ = 5)
    J4          = 4,  ///< (J · J)²                      (naive Δ = 4)
    stateSq     = 5,  ///< s · s                         (naive Δ = 2)
};

static constexpr int kNumOps = 6;
static constexpr const char* kOpNames[kNumOps] = {
    "JJ", "divJ2", "curlJ2", "JdotDivJ", "J4", "stateSq"
};
static constexpr double kNaiveDim[kNumOps] = { 2.0, 4.0, 4.0, 5.0, 4.0, 2.0 };

/// Evaluate operator `op` at voxel index `idx`. All six operators are
/// scalar-valued at each voxel, so the return type is double.
inline double evaluate_operator(const RenderBridge& rb, int idx, OpId op) {
    const auto& vox = rb.voxels();
    const auto& J = vox[idx].flux;
    const double s = static_cast<double>(vox[idx].state);
    const auto& lat = rb.lattice();
    switch (op) {
        case OpId::JJ:
            return J.dot(J);
        case OpId::divJ2: {
            const double d = ::ftd::divergence_flux_op(vox, lat, idx);
            return d * d;
        }
        case OpId::curlJ2: {
            const Vec3 c = ::ftd::curl_flux_op(vox, lat, idx);
            return c.dot(c);
        }
        case OpId::JdotDivJ: {
            // J · ∇(∇·J). Use existing gradient_divergence_op for ∇(∇·J).
            const Vec3 g = ::ftd::gradient_divergence_op(vox, lat, idx);
            return J.dot(g);
        }
        case OpId::J4: {
            const double j2 = J.dot(J);
            return j2 * j2;
        }
        case OpId::stateSq:
            return s * s;
    }
    return 0.0;
}

/// Two-point correlator for operator `op`:
///   C_O(r) = ⟨O(x) · O(x+r)⟩ - ⟨O⟩²
/// Averaged over all voxels and the three cubic axes (same convention as
/// ftd::spatial_flux_correlation). Returns a vector indexed by r.
inline std::vector<double> operator_correlator(
    const RenderBridge& rb, OpId op, int max_r = -1)
{
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int N = lat.total_sites();
    if (max_r < 0 || max_r > L / 2) max_r = L / 2;

    // Pre-compute O(x) at every voxel once; cheap for simple scalar
    // operators, necessary because the correlator needs each site as
    // both "x" and "x+r" in the inner loop.
    std::vector<double> Ofield(N, 0.0);
    double O_sum = 0.0;
    for (int i = 0; i < N; ++i) {
        const double v = evaluate_operator(rb, i, op);
        Ofield[i] = v;
        O_sum += v;
    }
    const double O_mean = O_sum / static_cast<double>(N);

    std::vector<double> C(max_r, 0.0);
    std::vector<long long> counts(max_r, 0);

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int i0 = lat.index(x, y, z);
                const double O0 = Ofield[i0] - O_mean;
                for (int r = 0; r < max_r; ++r) {
                    const int ix = lat.index(lat.wrap(x + r), y, z);
                    const int iy = lat.index(x, lat.wrap(y + r), z);
                    const int iz = lat.index(x, y, lat.wrap(z + r));
                    C[r] += O0 * (Ofield[ix] - O_mean);
                    C[r] += O0 * (Ofield[iy] - O_mean);
                    C[r] += O0 * (Ofield[iz] - O_mean);
                    counts[r] += 3;
                }
            }
        }
    }
    for (int r = 0; r < max_r; ++r)
        if (counts[r] > 0) C[r] /= static_cast<double>(counts[r]);
    return C;
}

/// Scaling-dimension fit result.
struct ScalingFit {
    double delta = 0.0;     ///< Extracted Δ from C ∝ r^(-2Δ) → slope = -2Δ
    double r2 = 0.0;        ///< Pearson R² of ln|C| vs ln r
    int n_points = 0;
    bool valid = false;     ///< true iff slope < 0 and fit ran
};

/// Fit C(r) = A · r^(-2Δ) by linear regression of ln|C| vs ln r.
/// Points with |C| ≤ 0 or negative are skipped; if fewer than 3 points
/// survive, returns valid=false.
inline ScalingFit fit_power_law(const std::vector<double>& C,
                                int r_min = 2, int r_max = -1)
{
    ScalingFit fit;
    const int N = static_cast<int>(C.size());
    if (r_max < 0 || r_max > N) r_max = N;
    if (r_min < 1) r_min = 1;  // ln r undefined at r=0
    if (r_max - r_min < 3) return fit;

    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    int n = 0;
    for (int r = r_min; r < r_max; ++r) {
        const double absC = std::abs(C[r]);
        if (!(absC > 0.0) || !std::isfinite(absC)) continue;
        const double x = std::log(static_cast<double>(r));
        const double y = std::log(absC);
        sx += x; sy += y; sxx += x * x; sxy += x * y;
        ++n;
    }
    if (n < 3) return fit;

    const double denom = n * sxx - sx * sx;
    if (std::abs(denom) < 1e-30) return fit;

    const double slope = (n * sxy - sx * sy) / denom;
    const double intercept = (sy - slope * sx) / n;

    // Scaling dimension: C(r) ∝ r^(-2Δ) ⇒ slope = -2Δ ⇒ Δ = -slope/2
    if (slope >= 0.0) return fit;  // correlator not decaying → no Δ

    // Pearson R²
    const double ybar = sy / n;
    double ss_tot = 0.0, ss_res = 0.0;
    for (int r = r_min; r < r_max; ++r) {
        const double absC = std::abs(C[r]);
        if (!(absC > 0.0) || !std::isfinite(absC)) continue;
        const double y = std::log(absC);
        const double yhat = intercept + slope * std::log(static_cast<double>(r));
        ss_tot += (y - ybar) * (y - ybar);
        ss_res += (y - yhat) * (y - yhat);
    }
    fit.delta = -0.5 * slope;
    fit.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
    fit.n_points = n;
    fit.valid = std::isfinite(fit.delta);
    return fit;
}

/// Convenience: evaluate and fit for all six operators, returning fits
/// indexed by the OpId numerical value.
inline std::array<ScalingFit, kNumOps> measure_operator_spectrum(
    const RenderBridge& rb, int r_min = 2, int r_max = -1)
{
    std::array<ScalingFit, kNumOps> out;
    for (int i = 0; i < kNumOps; ++i) {
        auto op = static_cast<OpId>(i);
        auto C = operator_correlator(rb, op);
        out[i] = fit_power_law(C, r_min, r_max);
    }
    return out;
}

}  // namespace eft
}  // namespace ftd

#pragma once
/**
 * @file ftd/sim/fit_scaling_dimension.h
 * @brief Power-law fit C(r) ∝ r^(−2Δ) → extract scaling dimension Δ.
 *
 * Takes a correlator vector (result of FluxCorrelator::result_host()
 * or any Observable<Backend, std::vector<double>>) and fits
 *
 *     ln |C(r)|  =  ln A  −  2 Δ · ln r
 *
 * by linear regression over [r_min, r_max). Returns Δ, amplitude A,
 * and Pearson R².
 *
 * This is a pure analysis utility — not an Observable, not backend-
 * specific. Ships as a free function because it operates on
 * already-host-resident data.
 *
 * Mirrors ftd::eft::fit_power_law in operator_spectrum.h (both exist
 * during the transition; pipeline users can pick either).
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd {
namespace sim {

struct ScalingDimensionFit {
    double delta = 0.0;  ///< scaling dimension Δ from C(r) ∝ r^(−2Δ)
    double amplitude = 0.0;
    double r2 = 0.0;
    int n_points = 0;
    bool valid = false;
};

/// Fit C(r) to a power law and extract Δ.
/// @param C     correlator values indexed by integer r
/// @param r_min first r to include (skip contact + nearest-neighbour noise)
/// @param r_max one past the last r to include (default = C.size())
inline ScalingDimensionFit fit_scaling_dimension(
    const std::vector<double>& C, int r_min = 2, int r_max = -1)
{
    ScalingDimensionFit fit;
    const int N = static_cast<int>(C.size());
    if (r_max < 0 || r_max > N) r_max = N;
    if (r_min < 1) r_min = 1;  // ln r undefined at r = 0
    if (r_max - r_min < 3) return fit;

    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    int n = 0;
    for (int r = r_min; r < r_max; ++r) {
        const double absC = std::abs(C[r]);
        if (!(absC > 0.0) || !std::isfinite(absC)) continue;
        const double x = std::log(static_cast<double>(r));
        const double y = std::log(absC);
        sx += x; sy += y; sxx += x*x; sxy += x*y;
        ++n;
    }
    if (n < 3) return fit;

    const double denom = n*sxx - sx*sx;
    if (std::abs(denom) < 1e-30) return fit;

    const double slope = (n*sxy - sx*sy) / denom;
    const double intercept = (sy - slope*sx) / n;
    if (slope >= 0.0) return fit;  // non-decaying → no Δ

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
    fit.amplitude = std::exp(intercept);
    fit.r2 = (ss_tot > 0.0) ? 1.0 - ss_res/ss_tot : 0.0;
    fit.n_points = n;
    fit.valid = std::isfinite(fit.delta) && fit.delta > 0.0;
    return fit;
}

/// Exponential fit: C(r) = A · exp(−r/ξ). Useful for mass-gap
/// extraction on condensate correlators (slope is −1/ξ; mass m = 1/ξ).
struct ExponentialFit {
    double xi = 0.0;         ///< decay length
    double mass = 0.0;       ///< 1/ξ (equivalent mass gap)
    double amplitude = 0.0;
    double r2 = 0.0;
    int n_points = 0;
    bool valid = false;
};

inline ExponentialFit fit_exponential_decay(
    const std::vector<double>& C, int r_min = 2, int r_max = -1)
{
    ExponentialFit fit;
    const int N = static_cast<int>(C.size());
    if (r_max < 0 || r_max > N) r_max = N;
    if (r_min < 0) r_min = 0;
    if (r_max - r_min < 3) return fit;

    const double C0 = C.empty() ? 0.0 : std::abs(C[0]);
    if (C0 < 1e-30) return fit;

    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    int n = 0;
    for (int r = r_min; r < r_max; ++r) {
        const double val = std::abs(C[r]);
        if (!(val > 0.0) || !std::isfinite(val)) continue;
        if (val < 1e-12 * C0) continue;
        const double y = std::log(val);
        const double rd = static_cast<double>(r);
        sx += rd; sy += y; sxx += rd*rd; sxy += rd*y;
        ++n;
    }
    if (n < 3) return fit;

    const double denom = n*sxx - sx*sx;
    if (std::abs(denom) < 1e-30) return fit;
    const double slope = (n*sxy - sx*sy) / denom;
    const double intercept = (sy - slope*sx) / n;
    if (slope >= 0.0) return fit;

    const double xi = -1.0 / slope;
    if (!std::isfinite(xi) || xi <= 0.0) return fit;

    const double ybar = sy / n;
    double ss_tot = 0.0, ss_res = 0.0;
    for (int r = r_min; r < r_max; ++r) {
        const double val = std::abs(C[r]);
        if (!(val > 0.0) || val < 1e-12 * C0) continue;
        const double y = std::log(val);
        const double yhat = intercept + slope * r;
        ss_tot += (y - ybar) * (y - ybar);
        ss_res += (y - yhat) * (y - yhat);
    }
    fit.xi = xi;
    fit.mass = 1.0 / xi;
    fit.amplitude = std::exp(intercept);
    fit.r2 = (ss_tot > 0.0) ? 1.0 - ss_res/ss_tot : 0.0;
    fit.n_points = n;
    fit.valid = true;
    return fit;
}

}  // namespace sim
}  // namespace ftd

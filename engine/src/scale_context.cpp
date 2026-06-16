/**
 * @file engine/src/scale_context.cpp
 * @purpose Implementation of the read-only scale-context readout admissibility
 *          gate (C_scale). See scale_context.h for the contract, and
 *          docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md for the
 *          canonical definitions.
 *
 * α-BLINDNESS: this TU references no coupling / α / Koopman symbol. Its inputs
 * are lattice geometry, |J|², and the observation-only event counters.
 *
 * GOLDEN-NEUTRAL: only const RenderBridge accessors are used; never called from
 * tick().
 */

#include "ftd/scale_context.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "ftd/render_bridge.h"   // const accessors: lattice(), voxels(), event counters
#include "ftd/constants.h"       // PI

namespace ftd {

namespace {

// Priority-ordered classifier (SPEC_SCALE_CONTEXT_READOUT §2/§3). Reads the
// already-measured fields of `d` plus the config and writes regime + status.
void classify(ScaleContextDiagnostics& d, const ScaleContextConfig& cfg) {
    const double eps = 1e-12;

    if (d.support_count == 0 || d.cloud_energy <= eps) {
        d.regime = ScaleRegime::Evaporating;
        d.status = ReadoutStatus::RejectedScaleContext;
    } else if (d.active_fraction < cfg.f_active_evap_min || d.R_eff < eps) {
        d.regime = ScaleRegime::Evaporating;
        d.status = ReadoutStatus::RejectedScaleContext;
    } else if (d.kappa < cfg.kappa_min) {
        d.regime = ScaleRegime::UVLocked;
        d.status = ReadoutStatus::RejectedScaleContext;
    } else if (d.zeta > cfg.zeta_max || d.active_fraction > cfg.f_active_max ||
               !d.center_well_defined) {
        d.regime = ScaleRegime::Percolating;
        d.status = ReadoutStatus::RejectedScaleContext;
    } else if (d.beta > cfg.beta_max) {
        d.regime = ScaleRegime::ShellDominated;
        d.status = ReadoutStatus::RejectedScaleContext;
    } else if (!d.confinement_fixed_point) {
        d.regime = ScaleRegime::BoundedAdmissible;
        d.status = ReadoutStatus::RejectedSelfConfinement;
    } else if (!d.stationary) {
        d.regime = ScaleRegime::BoundedAdmissible;
        d.status = ReadoutStatus::RejectedNonStationary;
    } else {
        d.regime = ScaleRegime::BoundedAdmissible;
        d.status = ReadoutStatus::Admissible;
    }

    // Observe-only override: report the regime but never claim a verdict unless
    // the gate is explicitly armed.
    if (!cfg.gate_active) d.status = ReadoutStatus::DiagnosticOnly;
}

// Least-squares slope of y vs x over the window (robust to per-tick noise).
double lsq_slope(const std::deque<double>& x, const std::deque<double>& y) {
    const std::size_t n = x.size();
    if (n < 2) return 0.0;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (std::size_t i = 0; i < n; ++i) {
        sx += x[i];
        sy += y[i];
        sxx += x[i] * x[i];
        sxy += x[i] * y[i];
    }
    const double dn = static_cast<double>(n);
    const double denom = dn * sxx - sx * sx;
    if (std::abs(denom) < 1e-12) return 0.0;
    return (dn * sxy - sx * sy) / denom;
}

double mean_of(const std::deque<double>& v) {
    if (v.empty()) return 0.0;
    double s = 0.0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}

// Single-exponential relaxation time from the lag-1 autocorrelation of the
// fluctuations. Advisory only (noisy; never gates) — see scale_context.h.
double estimate_tau(const std::deque<double>& v) {
    const std::size_t n = v.size();
    if (n < 3) return 0.0;
    const double m = mean_of(v);
    double var = 0.0, c1 = 0.0;
    for (std::size_t i = 0; i < n; ++i) var += (v[i] - m) * (v[i] - m);
    for (std::size_t i = 0; i + 1 < n; ++i) c1 += (v[i] - m) * (v[i + 1] - m);
    if (var < 1e-15) return 0.0;
    const double rho1 = c1 / var;
    if (rho1 <= 0.0 || rho1 >= 1.0) return 0.0;
    return -1.0 / std::log(rho1);
}

}  // namespace

ScaleContextDiagnostics measure_scale_context(const RenderBridge& rb,
                                              const ScaleContextConfig& cfg) {
    ScaleContextDiagnostics d;

    const Lattice& lat = rb.lattice();
    const int L = lat.size();
    const std::int64_t N = lat.total_sites();
    const std::vector<Voxel>& vox = rb.voxels();  // const overload — no host-dirty flag

    d.tick = rb.current_tick();
    d.L = L;
    d.a = 1.0;

    const double two_pi = 2.0 * PI;

    // ---- Pass 1: support, energy, circular-mean accumulators, peak ----
    double sumw = 0.0;  // Σ|J|² over support
    double Cx = 0, Sx = 0, Cy = 0, Sy = 0, Cz = 0, Sz = 0;
    int support_count = 0;
    double peak = 0.0;

    for (std::int64_t i = 0; i < N; ++i) {
        const Voxel& v = vox[static_cast<std::size_t>(i)];
        const double w = v.flux.mag2();
        bool in_support = (w >= cfg.energy_threshold);
        if (!in_support && cfg.union_with_state && v.state != 0) in_support = true;
        if (!in_support) continue;

        ++support_count;
        sumw += w;
        const double mag = std::sqrt(w);
        if (mag > peak) peak = mag;

        const Coord c = lat.coord(static_cast<int>(i));
        const double ax = two_pi * c.x / L;
        const double ay = two_pi * c.y / L;
        const double az = two_pi * c.z / L;
        Cx += w * std::cos(ax);  Sx += w * std::sin(ax);
        Cy += w * std::cos(ay);  Sy += w * std::sin(ay);
        Cz += w * std::cos(az);  Sz += w * std::sin(az);
    }

    d.support_count  = support_count;
    d.active_fraction = (N > 0) ? static_cast<double>(support_count) / static_cast<double>(N) : 0.0;
    d.J2_total       = sumw;
    d.cloud_energy   = 0.5 * sumw;
    d.peak_density   = peak;

    if (support_count == 0 || sumw <= 1e-30) {
        // Empty / no energy: geometry stays zero; classify ⇒ Evaporating.
        d.stationary = true;  // vacuous; classifier rejects on support first anyway
        classify(d, cfg);
        return d;
    }

    // ---- Circular-mean center (PBC) ----
    auto circ = [&](double C, double S) -> std::pair<double, double> {
        const double rbar = std::sqrt(C * C + S * S) / sumw;
        double ang = std::atan2(S, C);
        if (ang < 0.0) ang += two_pi;
        return {ang * L / two_pi, rbar};
    };
    const auto [cx, rbx] = circ(Cx, Sx);
    const auto [cy, rby] = circ(Cy, Sy);
    const auto [cz, rbz] = circ(Cz, Sz);
    d.center_x = cx;
    d.center_y = cy;
    d.center_z = cz;
    d.center_concentration = std::min({rbx, rby, rbz});
    d.center_well_defined  = d.center_concentration > 0.2;  // [IMPOSED] floor

    const Vec3 center(cx, cy, cz);

    // ---- Pass 2: PBC R_eff, radial energy histogram, Φ shells ----
    const int n_shells = (cfg.n_shells > 0) ? cfg.n_shells : 1;
    const double sw = (cfg.shell_width > 0.0) ? cfg.shell_width : 1.0;
    std::vector<double> Ebin(static_cast<std::size_t>(n_shells), 0.0);
    std::vector<double> Sout(static_cast<std::size_t>(n_shells), 0.0);
    std::vector<double> Sret(static_cast<std::size_t>(n_shells), 0.0);
    double sum_w_r2 = 0.0;

    for (std::int64_t i = 0; i < N; ++i) {
        const Voxel& v = vox[static_cast<std::size_t>(i)];
        const double w = v.flux.mag2();
        bool in_support = (w >= cfg.energy_threshold);
        if (!in_support && cfg.union_with_state && v.state != 0) in_support = true;
        if (!in_support) continue;

        const Coord c = lat.coord(static_cast<int>(i));
        const Vec3 p(static_cast<double>(c.x), static_cast<double>(c.y), static_cast<double>(c.z));
        const Vec3 dr = min_image_disp(p, center, L);
        const double r = dr.mag();
        sum_w_r2 += w * r * r;

        int k = static_cast<int>(std::floor(r / sw));
        if (k < 0) k = 0;
        if (k >= n_shells) k = n_shells - 1;
        Ebin[static_cast<std::size_t>(k)] += w;

        if (r > 1e-9) {
            const Vec3 rhat = dr * (1.0 / r);
            const double jr = v.flux.dot(rhat);
            if (jr > 0.0) Sout[static_cast<std::size_t>(k)] += jr;
            else          Sret[static_cast<std::size_t>(k)] += -jr;
        }
    }

    d.R_eff = std::sqrt(sum_w_r2 / sumw);
    d.kappa = d.R_eff / d.a;
    d.zeta  = d.R_eff / static_cast<double>(L);

    // ---- Radial quantiles r50, r90 from the energy CDF ----
    double Etot = 0.0;
    for (double e : Ebin) Etot += e;
    auto quantile = [&](double q) -> double {
        if (Etot <= 0.0) return 0.0;
        const double target = q * Etot;
        double cum = 0.0;
        for (int k = 0; k < n_shells; ++k) {
            const double prev = cum;
            cum += Ebin[static_cast<std::size_t>(k)];
            if (cum >= target) {
                const double binE = Ebin[static_cast<std::size_t>(k)];
                const double frac = (binE > 0.0) ? (target - prev) / binE : 0.0;
                return (k + frac) * sw;
            }
        }
        return n_shells * sw;
    };
    d.r50 = quantile(0.5);
    d.r90 = quantile(0.9);
    d.delta_shell = d.r90 - d.r50;
    d.beta = d.delta_shell / std::max(d.R_eff, 1e-9);

    // ---- Self-confinement (flux balance at the R_eff shell) ----
    int ks = static_cast<int>(std::floor(d.R_eff / sw));
    if (ks < 0) ks = 0;
    if (ks >= n_shells) ks = n_shells - 1;
    d.phi_outward = Sout[static_cast<std::size_t>(ks)];
    d.phi_return  = Sret[static_cast<std::size_t>(ks)];
    d.phi_balance = d.phi_outward - d.phi_return;
    d.phi_balance_norm = std::abs(d.phi_balance) / (d.phi_outward + d.phi_return + 1e-12);

    auto g = [&](int k) { return Sout[static_cast<std::size_t>(k)] - Sret[static_cast<std::size_t>(k)]; };
    if (ks >= 1 && ks <= n_shells - 2) {
        d.dPhi_dR = (g(ks + 1) - g(ks - 1)) / (2.0 * sw);
    } else if (ks == 0 && n_shells >= 2) {
        d.dPhi_dR = (g(1) - g(0)) / sw;
    } else if (ks == n_shells - 1 && n_shells >= 2) {
        d.dPhi_dR = (g(ks) - g(ks - 1)) / sw;
    } else {
        d.dPhi_dR = 0.0;
    }
    d.confinement_fixed_point =
        (d.phi_balance_norm <= cfg.phi_balance_tol) && (d.dPhi_dR < cfg.dPhi_dR_max);

    // ---- Boundary susceptibility (this tick; pure telemetry) ----
    d.genesis_events     = rb.genesis_events_this_tick();
    d.evaporation_events = rb.evaporation_events_this_tick();
    d.B_t = static_cast<double>(d.genesis_events - d.evaporation_events);

    // ---- Rolling fields are 0 in single-shot ⇒ stationary trivially true ----
    d.dR_dt = 0.0;
    d.dJ2_dt = 0.0;
    d.tau_cloud = 0.0;
    d.Theta = 0.0;
    d.stationary = true;

    classify(d, cfg);
    return d;
}

ScaleContextDiagnostics ScaleContextTracker::ingest(const RenderBridge& rb) {
    ScaleContextDiagnostics d = measure_scale_context(rb, cfg_);

    // Push this tick's samples into the rolling window. The slope x-axis is an
    // internal monotonic counter, not d.tick (which is rb.current_tick() and
    // may be constant if the caller re-measures a frozen state without ticking).
    idxv_.push_back(static_cast<double>(n_ingested_));
    ++n_ingested_;
    reff_.push_back(d.R_eff);
    j2_.push_back(d.J2_total);
    bt_.push_back(d.B_t);
    const std::size_t cap = (cfg_.window > 0) ? static_cast<std::size_t>(cfg_.window) : 1;
    while (reff_.size() > cap) {
        idxv_.pop_front();
        reff_.pop_front();
        j2_.pop_front();
        bt_.pop_front();
    }

    if (warmed_up()) {
        d.dR_dt = lsq_slope(idxv_, reff_);
        const double meanj2 = mean_of(j2_);
        const double slopej2 = lsq_slope(idxv_, j2_);
        d.dJ2_dt = (std::abs(meanj2) > 1e-12) ? slopej2 / meanj2 : 0.0;
        d.tau_cloud = estimate_tau(reff_);
        d.Theta = (cfg_.tau_bath > 0.0) ? d.tau_cloud / cfg_.tau_bath : 0.0;
        const double meanB = mean_of(bt_);
        d.stationary = (std::abs(d.dR_dt) <= cfg_.dR_dt_tol) &&
                       (std::abs(d.dJ2_dt) <= cfg_.dJ2_dt_tol) &&
                       (std::abs(meanB) <= cfg_.B_t_tol);
    } else {
        // Not enough history to judge stationarity — conservatively not stationary.
        d.stationary = false;
    }

    classify(d, cfg_);  // re-classify with rolling fields filled
    latest_ = d;
    return d;
}

}  // namespace ftd

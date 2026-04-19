#pragma once
/**
 * @file ftd/sim/measure_v_of_r.h
 * @brief Two-charge interaction potential V(r) — composite measurement.
 *
 * V(r) is not a single-pipeline observable. It requires multiple
 * independent simulations:
 *   1. one +1 self-energy simulation (E_self_plus)
 *   2. one −1 self-energy simulation (E_self_minus)
 *   3. N pair-energy simulations, one per r, (E_pair(r))
 *   V(r) = E_pair(r) − (E_self_plus + E_self_minus)
 *
 * Returning V(r) = −α/r + const on the continuum Coulomb potential.
 * Linear regression of V against 1/r then yields α_fit.
 *
 * This helper lives outside the Observable<T> template because it
 * runs multiple pipelines itself. It's a utility that *uses* the
 * pipeline + FieldEnergyAudit observable.
 *
 * This replaces engine/include/ftd/eft/coupling_measurement.h for the
 * pipeline path (both APIs coexist during the transition).
 *
 * Usage:
 *
 *     auto result = measure_v_of_r<BackendGpu>(
 *         64,                                  // L
 *         std::vector<int>{4, 8, 12, 16, 20},  // r_values
 *         300);                                // n_ticks
 *     for (const auto& p : result.data)
 *         std::printf("r=%d V=%.6f alpha_r=%.6f\n", p.r, p.V, p.alpha_r);
 *     std::printf("alpha_fit = %.6f (R^2=%.4f)\n", result.alpha_fit, result.r2);
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "ftd/render_bridge.h"          // for EnergyAudit, TermToggles, K_B
#include "ftd/sim/backend_cpu.h"
#include "ftd/sim/pipeline.h"
#include "ftd/sim/observables/field_energy_audit.h"

#ifdef FTD_ENABLE_CUDA
#  include "ftd/sim/backend_gpu.h"
#endif

namespace ftd {
namespace sim {

/// One V(r) sample.
struct VofRPoint {
    int r = 0;
    double V = 0.0;        ///< E_pair(r) − 2·E_self
    double alpha_r = 0.0;  ///< −V · r   (constant = α on pure Coulomb)
};

struct VofRResult {
    int L = 0;
    int n_ticks = 0;
    double e_self_pos = 0.0;
    double e_self_neg = 0.0;
    std::vector<VofRPoint> data;
    double alpha_fit = 0.0;  ///< slope fit of V vs 1/r → −slope
    double r2 = 0.0;         ///< Pearson R² of the fit
    bool valid = false;
};

/// Canonical "bare lattice" toggles for coupling-constant measurement.
/// Matches the toggle set used by benchmark_emergent_alpha and by the
/// EFT-program coupling_measurement module.
inline TermToggles bare_lattice_toggles_for_coupling() {
    TermToggles t{};
    t.wave_propagation = true;
    t.coupling = true;
    t.gauss_projection = true;
    t.genesis = false;
    t.damping = false;
    t.selective_damping = false;
    t.larmor_radiation = false;
    t.forces = false;
    t.poisson_coulomb = false;
    t.gravity = false;
    t.movement = false;
    t.lorentz_force = false;
    t.color_forces = false;
    t.dual_substrate = false;
    t.weak_transmutation = false;
    t.latency_field = false;
    return t;
}

namespace detail {

/// Run one self-energy simulation and return field_energy at the final tick.
template <typename Backend>
double measure_self_energy(int L, int8_t sign, int n_ticks,
                           double initial_flux_z = 0.05) {
    const int mid = L / 2;
    Pipeline<Backend> p(L);
    p.set_toggles(bare_lattice_toggles_for_coupling());
    p.inject_particle(mid, mid, mid, sign,
                      {0.0, 0.0, static_cast<double>(sign) * initial_flux_z});
    // Note: we don't lock on the GPU backend because lock() is a no-op there.
    //       On CPU the lock keeps the particle stationary; on GPU the
    //       engine treats state!=0 as immovable by default.
    p.lock(mid, mid, mid);

    auto audit = std::make_shared<FieldEnergyAudit<Backend>>();
    p.observe_at(n_ticks, audit);
    p.run(n_ticks);
    return audit->result_host().field_energy;
}

/// Run one pair-energy simulation (+1 at centre, −1 at offset r along x).
template <typename Backend>
double measure_pair_energy(int L, int r, int n_ticks,
                           double initial_flux_z = 0.05) {
    const int mid = L / 2;
    Pipeline<Backend> p(L);
    p.set_toggles(bare_lattice_toggles_for_coupling());
    p.inject_particle(mid, mid, mid, +1,
                      {0.0, 0.0, initial_flux_z});
    p.lock(mid, mid, mid);
    p.inject_particle(mid + r, mid, mid, -1,
                      {0.0, 0.0, -initial_flux_z});
    p.lock(mid + r, mid, mid);

    auto audit = std::make_shared<FieldEnergyAudit<Backend>>();
    p.observe_at(n_ticks, audit);
    p.run(n_ticks);
    return audit->result_host().field_energy;
}

}  // namespace detail

/// Measure α_eff via V(r) on an L cubed lattice using the specified backend.
///
/// @param L                lattice size (>= 16 for meaningful r range)
/// @param r_values         separations to sample along x-axis
/// @param n_ticks          ticks per configuration (300 recommended for L >= 64)
/// @param initial_flux_z   per-particle initial flux magnitude
template <typename Backend>
VofRResult measure_v_of_r(int L,
                          const std::vector<int>& r_values,
                          int n_ticks = 300,
                          double initial_flux_z = 0.05) {
    VofRResult out;
    out.L = L;
    out.n_ticks = n_ticks;
    if (L < 8 || r_values.empty()) return out;

    out.e_self_pos = detail::measure_self_energy<Backend>(L, +1, n_ticks, initial_flux_z);
    out.e_self_neg = detail::measure_self_energy<Backend>(L, -1, n_ticks, initial_flux_z);
    const double E_2self = out.e_self_pos + out.e_self_neg;

    for (int r : r_values) {
        const double E_pair = detail::measure_pair_energy<Backend>(L, r, n_ticks, initial_flux_z);
        const double V = E_pair - E_2self;
        VofRPoint pt;
        pt.r = r;
        pt.V = V;
        pt.alpha_r = -V * static_cast<double>(r);
        out.data.push_back(pt);
    }

    // Linear regression of V(r) vs 1/r.
    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (const auto& p : out.data) {
            const double x = 1.0 / p.r;
            const double y = p.V;
            sx += x; sy += y; sxx += x*x; sxy += x*y;
        }
        const double denom = n*sxx - sx*sx;
        if (std::abs(denom) > 1e-30) {
            const double slope = (n*sxy - sx*sy) / denom;
            const double intercept = (sy - slope*sx) / n;
            out.alpha_fit = -slope;
            const double ybar = sy / n;
            double ss_tot = 0.0, ss_res = 0.0;
            for (const auto& p : out.data) {
                const double x = 1.0 / p.r;
                const double yhat = intercept + slope*x;
                ss_tot += (p.V - ybar) * (p.V - ybar);
                ss_res += (p.V - yhat) * (p.V - yhat);
            }
            out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res/ss_tot : 0.0;
            out.valid = std::isfinite(out.alpha_fit);
        }
    }
    return out;
}

}  // namespace sim
}  // namespace ftd

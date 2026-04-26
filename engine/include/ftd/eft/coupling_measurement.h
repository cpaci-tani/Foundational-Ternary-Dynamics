#pragma once
/**
 * @file ftd/eft/coupling_measurement.h
 * @brief Lattice-coupling measurement for the EFT Recovery Program (Phase 2B).
 *
 * Physics motivation
 * ------------------
 * The EFT Recovery Program's β-function measurement (Phase 2C) needs a
 * scale-dependent coupling α_eff(L) extracted from engine output at
 * multiple blocking stages. This module defines the canonical extraction:
 *
 *     α_eff = − slope of  V(r)  vs  1/r
 *
 * where V(r) = E_pair(r) − (E_self(+) + E_self(−)) is the interaction
 * energy of a +1/−1 pair at separation r. For a Coulomb potential
 * V(r) = −α/r + const, a linear regression of V vs 1/r recovers α from
 * the slope.
 *
 * The extraction reuses the existing `benchmark_emergent_alpha.cpp`
 * experiment-E2 pattern but exposes it as a reusable function so:
 *   - it can be called on blocked lattices (Phase 2C trajectory);
 *   - it returns both the fitted α and the per-r data for goodness-of-fit
 *     reporting;
 *   - it documents the canonical `configure_bare_lattice` toggles in one
 *     place.
 *
 * Epistemic status
 * ----------------
 * The fit form V = −α/r + const is imported from continuum Coulomb physics.
 * FTD's claim is only that a *lattice-scale-dependent* α_eff(L) exists and
 * is extractable; whether it matches the QED β running is the Phase 2C
 * question. Tag: [MEASUREMENT]; the fit form itself is [IMPOSED from
 * standard EM], the α values are [MEASURED].
 *
 * Threading & cost
 * ----------------
 * Each α_eff measurement runs 1 self-energy + K pair-energy simulations,
 * each K ticks of engine evolution with gauss_projection active. On L = 64
 * at K = 300 ticks, total wall-time is O(60 s) single-threaded. Callers
 * that need multiple seeds can parallelise at the outer loop.
 */

#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// One V(r) data point from the two-charge interaction-energy probe.
struct VofR {
    int    r        = 0;    ///< lattice-unit separation
    double V        = 0.0;  ///< E_pair − 2·E_self
    double alpha_r  = 0.0;  ///< −V · r (should be ~constant = α on Coulomb fit)
};

/// Result of a full α_eff extraction.
struct CouplingMeasurement {
    std::vector<VofR> data;   ///< V(r) samples used in the fit
    double alpha_fit = 0.0;   ///< slope-fit α from regressing V against 1/r
    double r2        = 0.0;   ///< R² of the V vs 1/r linear fit
    double e_self_pos = 0.0;
    double e_self_neg = 0.0;
    int    L         = 0;
    int    n_ticks   = 0;
    bool   valid     = false;
};

/// Optional Langevin thermalization applied before α_eff probes.
struct LangevinOptions {
    bool enabled = false;
    double T = 0.0;
    double gamma = 0.01;
    int burn_in_ticks = 0;
    unsigned int seed = 1;
};

/// Canonical "bare lattice" toggle configuration for α_eff measurement.
/// Leaves: wave_propagation, coupling, gauss_projection ON; genesis,
/// damping, selective_damping, larmor_radiation, forces, poisson_coulomb,
/// gravity, movement, lorentz_force, color_forces, dual_substrate,
/// weak_transmutation, latency_field OFF. Matches the configuration used
/// by `benchmark_emergent_alpha.cpp::configure_bare_lattice`.
inline void configure_bare_lattice_for_coupling(RenderBridge& rb) {
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = false;
    rb.toggles.damping = false;
    rb.toggles.forces = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.gravity = false;
    rb.toggles.movement = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.color_forces = false;
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field = false;
    rb.toggles.langevin = false;
}

inline void apply_langevin_options(RenderBridge& rb,
                                   const LangevinOptions& options) {
    rb.toggles.langevin = options.enabled;
    rb.toggles.langevin_T = options.T;
    rb.toggles.langevin_gamma = options.gamma;
    rb.toggles.langevin_seed = options.seed;
    rb.seed_rng(options.seed);
}

inline void copy_flux_and_wave_vel_for_coupling(const RenderBridge& src,
                                                RenderBridge& dst) {
    const auto& sv = src.voxels();
    auto& dv = dst.voxels();
    const size_t N = sv.size();
    for (size_t i = 0; i < N; ++i) {
        dv[i].flux = sv[i].flux;
        dv[i].wave_vel = sv[i].wave_vel;
        dv[i].flux_L = sv[i].flux_L;
        dv[i].flux_R = sv[i].flux_R;
        dv[i].wave_vel_L = sv[i].wave_vel_L;
        dv[i].wave_vel_R = sv[i].wave_vel_R;
    }
}

inline void place_test_charge_on_bg(RenderBridge& rb,
                                    int x, int y, int z,
                                    int8_t sign,
                                    double initial_flux_z = 0.05) {
    if (sign == 0) return;
    rb.inject_particle(x, y, z, sign,
                       {0, 0, static_cast<double>(sign) * initial_flux_z});
    rb.voxels()[rb.lattice().index(x, y, z)].locked = true;
}

/// Measure the field self-energy of a single charge s ∈ {+1, −1} placed at
/// the lattice centre. Returns the steady-state energy_audit.field_energy
/// after running for `n_ticks` ticks. Uses the canonical bare-lattice
/// toggle configuration.
///
/// `initial_flux_z` is the amplitude of the seed flux aligned with the
/// charge sign. Varying this amplitude across runs gives an ensemble of
/// independent configurations for multi-seed statistics (see
/// `measure_alpha_eff_ensemble`).
inline double measure_self_energy(int L, int8_t sign, int n_ticks,
                                  double initial_flux_z = 0.05) {
    const int mid = L / 2;
    RenderBridge rb(L);
    configure_bare_lattice_for_coupling(rb);
    rb.inject_particle(mid, mid, mid, sign,
                       {0, 0, static_cast<double>(sign) * initial_flux_z});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

inline double measure_self_energy(int L, int8_t sign, int n_ticks,
                                  double initial_flux_z,
                                  const LangevinOptions& options) {
    const int mid = L / 2;
    RenderBridge rb(L);
    configure_bare_lattice_for_coupling(rb);
    apply_langevin_options(rb, options);
    if (options.enabled && options.burn_in_ticks > 0) {
        rb.run(options.burn_in_ticks);
    }
    place_test_charge_on_bg(rb, mid, mid, mid, sign, initial_flux_z);
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

/// Measure the field energy of a +1/−1 pair at separation r along the
/// x-axis, with the +1 at the lattice centre. Pair is locked so dynamics
/// are limited to field evolution, not particle motion. Returns
/// steady-state field_energy.
inline double measure_pair_energy(int L, int r, int n_ticks,
                                  double initial_flux_z = 0.05) {
    const int mid = L / 2;
    RenderBridge rb(L);
    configure_bare_lattice_for_coupling(rb);
    rb.inject_particle(mid, mid, mid, +1, {0, 0, initial_flux_z});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -initial_flux_z});
    rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

inline double measure_pair_energy(int L, int r, int n_ticks,
                                  double initial_flux_z,
                                  const LangevinOptions& options) {
    const int mid = L / 2;
    RenderBridge rb(L);
    configure_bare_lattice_for_coupling(rb);
    apply_langevin_options(rb, options);
    if (options.enabled && options.burn_in_ticks > 0) {
        rb.run(options.burn_in_ticks);
    }
    place_test_charge_on_bg(rb, mid, mid, mid, +1, initial_flux_z);
    place_test_charge_on_bg(rb, mid + r, mid, mid, -1, initial_flux_z);
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

/// Full α_eff extraction via V(r) fit.
///
/// @param L              lattice size (must be ≥ 16 for meaningful r range)
/// @param n_ticks        ticks per configuration (300 recommended for L = 64)
/// @param r_min          first pair separation to sample (default 4)
/// @param r_max          last pair separation (default L/3)
/// @param r_step         step between separations (default 2)
/// @param initial_flux_z seed amplitude (default 0.05; varying this across
///                       runs produces an ensemble of independent
///                       configurations for multi-seed statistics)
inline CouplingMeasurement measure_alpha_eff(
    int L, int n_ticks = 300,
    int r_min = 4, int r_max = -1, int r_step = 2,
    double initial_flux_z = 0.05)
{
    CouplingMeasurement out;
    out.L = L;
    out.n_ticks = n_ticks;
    if (r_max < 0) r_max = L / 3;
    if (r_max <= r_min) return out;
    if (L < 8) return out;

    out.e_self_pos = measure_self_energy(L, +1, n_ticks, initial_flux_z);
    out.e_self_neg = measure_self_energy(L, -1, n_ticks, initial_flux_z);
    const double E_2self = out.e_self_pos + out.e_self_neg;

    for (int r = r_min; r <= r_max; r += r_step) {
        const double E_pair = measure_pair_energy(L, r, n_ticks, initial_flux_z);
        const double V = E_pair - E_2self;
        VofR pt;
        pt.r = r;
        pt.V = V;
        pt.alpha_r = -V * static_cast<double>(r);
        out.data.push_back(pt);
    }

    // Linear regression of V(r) vs 1/r — slope = −α.
    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
        for (const auto& p : out.data) {
            const double x = 1.0 / static_cast<double>(p.r);
            const double y = p.V;
            sx += x; sy += y; sxx += x * x; sxy += x * y; syy += y * y;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            const double slope = (n * sxy - sx * sy) / denom;
            const double intercept = (sy - slope * sx) / n;
            out.alpha_fit = -slope;
            // R² = 1 − SS_res/SS_tot
            const double ybar = sy / n;
            double ss_tot = 0.0, ss_res = 0.0;
            for (const auto& p : out.data) {
                const double x = 1.0 / static_cast<double>(p.r);
                const double y = p.V;
                const double yhat = intercept + slope * x;
                ss_tot += (y - ybar) * (y - ybar);
                ss_res += (y - yhat) * (y - yhat);
            }
            out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
            out.valid = std::isfinite(out.alpha_fit);
        }
    }

    return out;
}

inline CouplingMeasurement measure_alpha_eff(
    int L, int n_ticks,
    int r_min, int r_max, int r_step,
    double initial_flux_z,
    const LangevinOptions& options)
{
    CouplingMeasurement out;
    out.L = L;
    out.n_ticks = n_ticks;
    if (r_max < 0) r_max = L / 3;
    if (r_max <= r_min) return out;
    if (L < 8) return out;

    out.e_self_pos = measure_self_energy(L, +1, n_ticks, initial_flux_z, options);
    out.e_self_neg = measure_self_energy(L, -1, n_ticks, initial_flux_z, options);
    const double E_2self = out.e_self_pos + out.e_self_neg;

    for (int r = r_min; r <= r_max; r += r_step) {
        const double E_pair =
            measure_pair_energy(L, r, n_ticks, initial_flux_z, options);
        const double V = E_pair - E_2self;
        VofR pt;
        pt.r = r;
        pt.V = V;
        pt.alpha_r = -V * static_cast<double>(r);
        out.data.push_back(pt);
    }

    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
        for (const auto& p : out.data) {
            const double x = 1.0 / static_cast<double>(p.r);
            const double y = p.V;
            sx += x; sy += y; sxx += x * x; sxy += x * y; syy += y * y;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            const double slope = (n * sxy - sx * sy) / denom;
            const double intercept = (sy - slope * sx) / n;
            out.alpha_fit = -slope;
            const double ybar = sy / n;
            double ss_tot = 0.0, ss_res = 0.0;
            for (const auto& p : out.data) {
                const double x = 1.0 / static_cast<double>(p.r);
                const double y = p.V;
                const double yhat = intercept + slope * x;
                ss_tot += (y - ybar) * (y - ybar);
                ss_res += (y - yhat) * (y - yhat);
            }
            out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
            out.valid = std::isfinite(out.alpha_fit);
        }
    }

    return out;
}

inline std::unique_ptr<RenderBridge> prepare_thermal_background(
    int L, double T, double gamma, int burn_in_ticks,
    unsigned int seed = 1)
{
    auto rb = std::make_unique<RenderBridge>(L);
    configure_bare_lattice_for_coupling(*rb);
    LangevinOptions options;
    options.enabled = true;
    options.T = T;
    options.gamma = gamma;
    options.seed = seed;
    apply_langevin_options(*rb, options);
    if (burn_in_ticks > 0) rb->run(burn_in_ticks);
    return rb;
}

inline double measure_self_energy_on_bg(const RenderBridge& bg,
                                        int8_t sign,
                                        int n_ticks,
                                        double initial_flux_z = 0.05) {
    const int L = bg.lattice().size();
    const int mid = L / 2;
    RenderBridge rb(L);
    configure_bare_lattice_for_coupling(rb);
    copy_flux_and_wave_vel_for_coupling(bg, rb);
    place_test_charge_on_bg(rb, mid, mid, mid, sign, initial_flux_z);
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

inline double measure_pair_energy_on_bg(const RenderBridge& bg,
                                        int r,
                                        int n_ticks,
                                        double initial_flux_z = 0.05) {
    const int L = bg.lattice().size();
    const int mid = L / 2;
    RenderBridge rb(L);
    configure_bare_lattice_for_coupling(rb);
    copy_flux_and_wave_vel_for_coupling(bg, rb);
    place_test_charge_on_bg(rb, mid, mid, mid, +1, initial_flux_z);
    place_test_charge_on_bg(rb, mid + r, mid, mid, -1, initial_flux_z);
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

inline CouplingMeasurement measure_alpha_eff_on_bg(
    const RenderBridge& bg, int n_ticks = 300,
    int r_min = 4, int r_max = -1, int r_step = 2,
    double initial_flux_z = 0.05)
{
    CouplingMeasurement out;
    const int L = bg.lattice().size();
    out.L = L;
    out.n_ticks = n_ticks;
    if (r_max < 0) r_max = L / 3;
    if (r_max <= r_min) return out;
    if (L < 8) return out;

    out.e_self_pos = measure_self_energy_on_bg(bg, +1, n_ticks, initial_flux_z);
    out.e_self_neg = measure_self_energy_on_bg(bg, -1, n_ticks, initial_flux_z);
    const double E_2self = out.e_self_pos + out.e_self_neg;

    for (int r = r_min; r <= r_max; r += r_step) {
        const double E_pair = measure_pair_energy_on_bg(bg, r, n_ticks, initial_flux_z);
        const double V = E_pair - E_2self;
        VofR pt;
        pt.r = r;
        pt.V = V;
        pt.alpha_r = -V * static_cast<double>(r);
        out.data.push_back(pt);
    }

    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0, syy = 0;
        for (const auto& p : out.data) {
            const double x = 1.0 / static_cast<double>(p.r);
            const double y = p.V;
            sx += x; sy += y; sxx += x * x; sxy += x * y; syy += y * y;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            const double slope = (n * sxy - sx * sy) / denom;
            const double intercept = (sy - slope * sx) / n;
            out.alpha_fit = -slope;
            const double ybar = sy / n;
            double ss_tot = 0.0, ss_res = 0.0;
            for (const auto& p : out.data) {
                const double x = 1.0 / static_cast<double>(p.r);
                const double y = p.V;
                const double yhat = intercept + slope * x;
                ss_tot += (y - ybar) * (y - ybar);
                ss_res += (y - yhat) * (y - yhat);
            }
            out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
            out.valid = std::isfinite(out.alpha_fit);
        }
    }

    return out;
}

}  // namespace eft
}  // namespace ftd

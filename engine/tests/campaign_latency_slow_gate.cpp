/**
 * @file campaign_latency_slow_gate.cpp
 * @brief T3 slow-gate candidacy, Stage A: is the latency sector SLOW
 *        relative to the flux band, under native matter activity?
 *
 * Motivation (temporal-interior front T3; FOUND_SPA_CHAIN_RELATIVITY_
 * EXTENSION_v1 §13/§14): the Born campaigns established that occupation
 * (Born) weighting of threshold statistics requires a stochastic element
 * slower than the flux band, and that flux-borne thermal noise cannot
 * supply it (signal and noise share the band; PREREG_BORN_REGIME_MAP_
 * ENGINE_v1, Outcome N). The latency sector — the Poisson gravity proxy
 * grad^2 L = 4 pi G rho, sourced by manifested matter — is the registered
 * [OPEN CANDIDATE — UNSCORED] native slow channel. This campaign scores
 * its NECESSARY condition: the correlation time of latency fluctuations
 * under native churn, measured against the flux band in the SAME run.
 *
 * Profile (native activity): wave + gauss_projection + langevin (drive)
 * + genesis master (manifestation + evaporation) + movement +
 * latency_field. field_energy_gravity OFF (matter-sourced latency only).
 * The thermostat drives |J| across K_GENESIS at a modest calibrated rate;
 * matter appears, moves, evaporates; latency tracks the matter
 * configuration through the per-tick Poisson solve.
 *
 * Measured per run: probe series of phi_latency and of flux J_z; matter
 * population N_m(t). Outputs: sigma_lat, tau_lat (integrated
 * autocorrelation, mean-removed), tau_flux (same estimator, same run),
 * the ratio tau_lat/tau_flux, and Omega*tau_lat against the band top
 * (2 asin(C) = 1.2310) and a mid-band mode (lambda = 8: Omega = 0.4456),
 * with the Born-regime design target Omega*tau >= 30 for reference.
 *
 * STATUS: SHAKEDOWN INSTRUMENT until PREREG_LATENCY_SLOW_GATE_v1 freezes
 * parameters. Read-mostly: the harness writes no engine state; the
 * activity is the engine's own. No claim, no tag.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

namespace {

constexpr int    L        = 32;
constexpr int    N_BURN   = 4000;
constexpr int    N_MEAS   = 20000;
constexpr double T_BATH   = 0.13;    // provisional; shakedown calibrates
constexpr unsigned SEED   = 20260807;
constexpr int    N_PROBES = 6;
constexpr int    AC_MAX   = 6000;    // latency can be very slow
constexpr double OM_TOP   = 1.230959445;   // 2 asin(C_WAVE)
constexpr double OM_MID   = 0.445649;      // lambda = 8 axis mode

double integrated_tau(const std::vector<std::vector<double>>& series,
                      int ac_max) {
    double tau_sum = 0.0;
    int used = 0;
    for (const auto& s : series) {
        double mean = 0.0;
        for (double v : s) mean += v;
        mean /= s.size();
        double c0 = 0.0;
        for (double v : s) c0 += (v - mean) * (v - mean);
        c0 /= s.size();
        if (c0 <= 0.0) continue;
        double tau = 0.5;
        for (int lag = 1; lag <= ac_max; ++lag) {
            double c = 0.0;
            const int nn = (int)s.size() - lag;
            if (nn < 100) break;
            for (int t = 0; t < nn; ++t)
                c += (s[t] - mean) * (s[t + lag] - mean);
            c /= nn;
            const double r = c / c0;
            if (r <= 0.0) break;
            tau += r;
        }
        tau_sum += tau;
        ++used;
    }
    return used ? tau_sum / used : -1.0;
}

}  // namespace

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    const double T_bath = (argc > 1) ? std::atof(argv[1]) : T_BATH;
    const unsigned seed_run = (argc > 2)
        ? (unsigned)std::strtoul(argv[2], nullptr, 10) : SEED;
    std::printf("==============================================================\n");
    std::printf(" Latency slow-gate candidacy — Stage A (native activity)\n");
    std::printf(" L=%d burn=%d meas=%d T=%.3f K_GENESIS=%.6f\n",
                L, N_BURN, N_MEAS, T_bath, ftd::K_GENESIS);
    std::printf("==============================================================\n");

    ftd::RenderBridge rb(L);
    // Validated cluster-thermodynamics skeleton (campaign_cluster_
    // relaxation): disable_all + wave/gauss/genesis/langevin at cool T
    // and gamma = 0.02, CPU, blob-seeded — a regime with STABLE bounded
    // clusters (the thermostat-driven bulk profile floods autocatalytically;
    // two shakedown cells proved it). Added here: gravity + latency_field
    // (the measured sector). Movement stays OFF as in the validated
    // skeleton; churn = genesis/evaporation reconfiguration at the
    // cluster boundary.
    // v5 protocol — NO THERMOSTAT anywhere: every thermostatted profile
    // eventually floods via the unpinned DC zero mode walking |J| up to
    // K_GENESIS (three shakedown cells, three timescales, one cause).
    // The matter is seeded directly as the corpus's own bound phase — a
    // checkerboard SC crystal ball (opposite-polarity nearest neighbours
    // at the compact-bond minimum) — and the measurement is of the
    // latency field of a self-gravitating, surface-evaporating crystal
    // under fully native dynamics. Zero [IMPOSED] drive in-measurement.
    (void)T_bath;   // retained in the banner for provenance; unused in v5
    // v6: genesis master OFF — the no-thermostat cell proved the minimal
    // profile is supercritical to matter proliferation (evaporation dumps
    // super-threshold flux; pair genesis doubles it; flood in 1,000
    // ticks). With genesis off, N is exactly conserved and the churn is
    // pure self-gravitating dynamics: the latency field of a virializing
    // cluster. tau_lat = the gravitational dynamical time — the native
    // timescale of the sector under test.
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = false;  // N conserved; flood-impossible
    rb.toggles.movement         = true;   // self-gravitating churn
    rb.toggles.forces           = true;   // phase_forces master — gravity
                                          // acceleration lives inside it
                                          // (v6 lesson: without it the
                                          // cluster is exactly static)
    rb.toggles.gravity          = true;   // latency_field requires it
    rb.toggles.latency_field    = true;
    rb.toggles.field_energy_gravity = false;
    rb.seed_rng(seed_run);

    const int N = L * L * L;
    const ftd::RenderBridge& crb = rb;

    // Seed: checkerboard crystal ball, radius R_SEED, centre polarity by
    // site parity (the SC bound phase); modest sub-threshold flux.
    const int c = L / 2;
    const int R_SEED = 5;
    int n_seeded = 0;
    for (int dz = -R_SEED; dz <= R_SEED; ++dz)
        for (int dy = -R_SEED; dy <= R_SEED; ++dy)
            for (int dx = -R_SEED; dx <= R_SEED; ++dx) {
                if (dx * dx + dy * dy + dz * dz > R_SEED * R_SEED) continue;
                const int par = ((dx + dy + dz) % 2 + 2) % 2;
                const int8_t s = par ? int8_t(-1) : int8_t(1);
                rb.inject_particle(c + dx, c + dy, c + dz, s,
                                   {0.0, 0.0, 0.0});
                ++n_seeded;
            }
    std::printf("  seeded checkerboard crystal: %d bodies, R = %d\n",
                n_seeded, R_SEED);

    std::printf("  equilibrating cluster (%d ticks, guard at 50%%)...\n",
                N_BURN);
    for (int t = 0; t < N_BURN; ++t) {
        rb.run(1);
        if (t % 500 == 499) {
            const auto& vox = crb.voxels();
            int nm = 0;
            for (int i = 0; i < N; ++i)
                if (vox[i].state != 0) ++nm;
            std::printf("    equil tick %d: N_matter = %d (%.2f%%)\n",
                        t + 1, nm, 100.0 * nm / N);
            if (nm > N / 2) {
                std::printf("  SATURATION GUARD: population > 50%% — "
                            "cell INVALID at T=%.4f (lower T)\n", T_bath);
                return 1;
            }
        }
    }

    std::vector<int> probes;
    for (int p = 0; p < N_PROBES; ++p) {
        int off = 3 + p * (L - 7) / (N_PROBES - 1);
        probes.push_back(rb.lattice().index(off, (off * 2) % L, (off * 3) % L));
    }
    std::vector<std::vector<double>> lat_series(
        N_PROBES, std::vector<double>(N_MEAS, 0.0));
    std::vector<std::vector<double>> flux_series(
        N_PROBES, std::vector<double>(N_MEAS, 0.0));
    std::vector<double> pop(N_MEAS, 0.0);
    double sum_z2 = 0.0;

    for (int t = 0; t < N_MEAS; ++t) {
        rb.run(1);
        const auto& vox = crb.voxels();
        const auto& phi = crb.phi_latency();
        int nm = 0;
        double z2 = 0.0;
        for (int i = 0; i < N; ++i) {
            if (vox[i].state != 0) ++nm;
            z2 += vox[i].flux.z * vox[i].flux.z;
        }
        pop[t] = nm;
        sum_z2 += z2 / N;
        for (int p = 0; p < N_PROBES; ++p) {
            lat_series[p][t] = phi[probes[p]];
            flux_series[p][t] = vox[probes[p]].flux.z;
        }
        if (nm > N / 2) {
            std::printf("  SATURATION GUARD (measurement): N_matter=%d — "
                        "cell INVALID at T=%.4f\n", nm, T_bath);
            return 1;
        }
        if ((t + 1) % 5000 == 0)
            std::printf("    tick %d/%d  N_matter=%d\n", t + 1, N_MEAS, nm);
    }

    double pop_mean = 0, pop_min = 1e18, pop_max = -1e18;
    for (double v : pop) {
        pop_mean += v;
        if (v < pop_min) pop_min = v;
        if (v > pop_max) pop_max = v;
    }
    pop_mean /= pop.size();
    const double sigma_z = std::sqrt(sum_z2 / N_MEAS);

    // sigma_lat across probes
    double sig_lat = 0.0;
    for (const auto& s : lat_series) {
        double m = 0, c0 = 0;
        for (double v : s) m += v;
        m /= s.size();
        for (double v : s) c0 += (v - m) * (v - m);
        sig_lat += std::sqrt(c0 / s.size());
    }
    sig_lat /= N_PROBES;

    const double tau_lat  = integrated_tau(lat_series, AC_MAX);
    const double tau_flux = integrated_tau(flux_series, 600);
    const double tau_pop  = integrated_tau({pop}, AC_MAX);

    std::printf("\n  --- results ---\n");
    std::printf("  matter population: mean %.1f  range [%.0f, %.0f]  "
                "(%.2f%% of sites)  tau_pop=%.1f\n",
                pop_mean, pop_min, pop_max, 100.0 * pop_mean / N, tau_pop);
    std::printf("  flux sigma_z = %.4f;  tau_flux = %.2f ticks\n",
                sigma_z, tau_flux);
    std::printf("  latency: sigma_lat = %.3e;  tau_lat = %.1f ticks\n",
                sig_lat, tau_lat);
    std::printf("  SLOWNESS RATIO tau_lat/tau_flux = %.1f\n",
                (tau_flux > 0) ? tau_lat / tau_flux : -1.0);
    std::printf("  Om_top*tau_lat = %.1f   Om_mid*tau_lat = %.1f   "
                "(Born-regime design target >= 30)\n",
                OM_TOP * tau_lat, OM_MID * tau_lat);
    std::printf("\nSHAKEDOWN ONLY — no claim, no tag, prereg lock pending.\n");
    return 0;
}

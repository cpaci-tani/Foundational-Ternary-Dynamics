/**
 * @file campaign_born_regime_map.cpp
 * @brief Engine-side Born regime map (temporal-interior front T3, FTD-0200
 *        path): does the mechanism-level regime law — Born-fraction rising
 *        with Omega*tau — transfer to the NATIVE thermal field?
 *
 * Design (analytic-overlay, fully read-only):
 *   The engine evolves ONLY the native thermal field: wave_propagation +
 *   langevin (CPU OU thermostat), everything else OFF. The two-mode
 *   coherent field is a deterministic ANALYTIC OVERLAY added in this
 *   harness at counting time:
 *       J_coh_z(x,y,t) = A1 cos(k1 x) cos(O1 t + th1)
 *                      + A2 cos(k2 y) cos(O2 t + th2),
 *   equal occupation (A_i ~ 1/sqrt(Omega_i)). Threshold upcrossings of
 *   |J_thermal + J_coh zhat| vs K_GENESIS are counted per site; the SAME
 *   run with overlay off gives the exact same-noise control. No injection,
 *   no signal decay (the thermostat never sees the overlay), no state
 *   modification: the engine field is never written.
 *
 * Per cell (gamma, mode pair): per-site excess counts regressed on
 * {1, cos 2 k1 x, cos 2 k2 y}; Born-fraction BF = (R - R_amp)/(1 - R_amp);
 * the effective noise correlation time tau_eff is MEASURED from probe-site
 * J_z autocorrelation (integrated |C(t)|/C(0) up to first zero + envelope).
 * Output: one CSV row per cell to engine/results/born_regime_map/.
 *
 * Dispersion (M18, face 1/3 edge 1/6, axis-aligned): omega(k) = 2 C sin(k/2),
 * Omega(k) = 2 asin(C_WAVE sin(k/2)) — band top 1.230959.
 *
 * STATUS: SHAKEDOWN INSTRUMENT — parameters below are provisional until
 * the preregistration (PREREG_BORN_REGIME_MAP_ENGINE_v1) freezes them;
 * per the C12 discipline (and the v1 lesson) the instrument must validate
 * on synthetics/calibration before any lock. This binary makes no claim
 * and moves no tag.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

namespace {

constexpr double PI = 3.141592653589793238462643383279502884;

constexpr int    L        = 32;
constexpr int    N_MEAS   = 20000;
constexpr double T_INIT   = 0.15;    // starting bath T; auto-calibrated
constexpr double A_FRAC   = 0.50;    // A1 = A_FRAC * sigma_z (weak field)
constexpr unsigned SEED0  = 20260807;
constexpr int    N_PROBES = 6;
constexpr int    AC_MAX   = 600;     // autocorrelation max lag (ticks)

// sigma_target is a per-cell knob: lower sigma -> rarer crossings -> a
// slower crossing clock tau_cross = 1/nu_ctl (the PRIMARY regime axis).
struct CellSpec { double gamma; double sig; int lam1; int lam2; };
const CellSpec CELLS[] = {
    {0.01, 0.47, 16, 8},                     // DC-fix validation cell
    {0.10, 0.55, 16, 8}, {0.10, 0.47, 16, 8},
    {0.10, 0.40, 16, 8}, {0.10, 0.34, 16, 8},
    {0.50, 0.47, 16, 8}, {0.50, 0.37, 16, 8},
    {0.10, 0.47,  8, 4}, {0.10, 0.37,  8, 4},
    {0.50, 0.40,  8, 4},
};

double Om_of_lam(int lam) {
    const double k = 2.0 * PI / lam;
    const double w = ftd::C_WAVE * std::sin(k / 2.0);   // omega/2
    return 2.0 * std::asin(w);
}

struct CellResult {
    double gamma, O1, O2, sigma_z, tau_eff, tau_cross, bf, R, c1, c2;
    long long net_excess, ctl_counts;
    int lam1, lam2;
    bool valid;
};

CellResult run_cell(const CellSpec& cs, unsigned seed) {
    const double K = ftd::K_GENESIS;
    const double O1 = Om_of_lam(cs.lam1);
    const double O2 = Om_of_lam(cs.lam2);
    const double k1 = 2.0 * PI / cs.lam1;
    const double k2 = 2.0 * PI / cs.lam2;

    ftd::RenderBridge rb(L);
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling         = false;
    rb.toggles.damping          = false;
    rb.toggles.genesis          = false;      // read-only observer design
    // Gauss projection ON: natively regularizes the thermostat's zero mode
    // (the frozen uniform-J offset the first shakedown exposed); both
    // overlay modes are transverse (J_z varying along x/y), so the
    // projection leaves the coherent field untouched.
    rb.toggles.gauss_projection = true;
    rb.toggles.forces           = false;
    rb.toggles.gravity          = false;
    rb.toggles.poisson_coulomb  = false;
    rb.toggles.movement         = false;
    rb.toggles.lorentz_force    = false;
    rb.toggles.selective_damping= false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate   = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field    = false;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T_INIT;
    rb.toggles.langevin_gamma   = cs.gamma;
    rb.toggles.langevin_seed    = seed;

    const int N = L * L * L;
    const ftd::RenderBridge& crb = rb;

    // Burn-in scaled to the slowest relaxation (soft modes ~ 1/gamma).
    const int n_burn = std::min(30000, std::max(4000, (int)(30.0 / cs.gamma)));
    std::printf("  [cell g=%.2f lam=(%d,%d)] burn-in %d ticks...\n",
                cs.gamma, cs.lam1, cs.lam2, n_burn);
    rb.run(n_burn);

    // --- auto-calibration: tune T until sigma_z hits SIGMA_TARGET ---
    // (deterministic protocol: <=5 iterations of measure -> rescale ->
    //  re-burn; declared in the preregistration)
    // All observer statistics use the MEAN-SUBTRACTED field: the unpinned
    // thermostat zero mode is divergence-free, so Gauss projection cannot
    // remove it (rev-3 lesson); subtracting the instantaneous spatial mean
    // J_bar(t) is the declared observer convention.
    auto measure_sigma = [&](int ticks) {
        double sum_z2 = 0.0;
        for (int t = 0; t < ticks; ++t) {
            rb.run(1);
            const auto& vox = crb.voxels();
            double mz = 0.0;
            for (int i = 0; i < N; ++i) mz += vox[i].flux.z;
            mz /= N;
            double s = 0.0;
            for (int i = 0; i < N; ++i) {
                const double d = vox[i].flux.z - mz;
                s += d * d;
            }
            sum_z2 += s / N;
        }
        return std::sqrt(sum_z2 / ticks);
    };
    double sigma_z = measure_sigma(300);
    bool calib_ok = true;
    for (int it = 0; it < 5 && std::fabs(sigma_z - cs.sig) >
                                0.05 * cs.sig; ++it) {
        const double scale = (cs.sig / sigma_z) * (cs.sig / sigma_z);
        rb.toggles.langevin_T *= scale;
        if (rb.toggles.langevin_T < 1e-6 || rb.toggles.langevin_T > 100.0) {
            std::printf("    CALIBRATION GUARD: T=%.3g out of bounds — "
                        "cell INVALID (unresponsive variance)\n",
                        rb.toggles.langevin_T);
            calib_ok = false;
            break;
        }
        const int reburn = std::min(15000, std::max(2000, (int)(10.0 / cs.gamma)));
        std::printf("    calib it%d: sigma_z=%.4f -> T=%.5f, re-burn %d\n",
                    it, sigma_z, rb.toggles.langevin_T, reburn);
        rb.run(reburn);
        sigma_z = measure_sigma(300);
    }
    std::printf("    calibrated: T=%.5f sigma_z=%.4f (target %.3f)%s\n",
                rb.toggles.langevin_T, sigma_z, cs.sig,
                calib_ok ? "" : "  [INVALID]");
    if (!calib_ok) {
        CellResult bad{};
        bad.gamma = cs.gamma; bad.O1 = O1; bad.O2 = O2;
        bad.sigma_z = sigma_z; bad.lam1 = cs.lam1; bad.lam2 = cs.lam2;
        bad.valid = false;
        return bad;
    }
    const double A1 = A_FRAC * sigma_z;
    const double A2 = A1 * std::sqrt(O1 / O2);          // equal occupation
    const double th1 = 0.7391, th2 = 2.1147;            // fixed phases

    // precompute spatial profiles
    std::vector<double> c1x(L), c2y(L), b1x(L), b2y(L);
    for (int i = 0; i < L; ++i) {
        c1x[i] = std::cos(k1 * i);
        c2y[i] = std::cos(k2 * i);
        b1x[i] = std::cos(2.0 * k1 * i);
        b2y[i] = std::cos(2.0 * k2 * i);
    }

    // probe sites for tau_eff
    std::vector<int> probes;
    for (int p = 0; p < N_PROBES; ++p) {
        int off = 3 + p * (L - 7) / (N_PROBES - 1);
        probes.push_back(rb.lattice().index(off, (off * 2) % L, (off * 3) % L));
    }
    std::vector<std::vector<double>> probe_series(
        N_PROBES, std::vector<double>(N_MEAS, 0.0));
    std::vector<std::vector<double>> probe_mag(
        N_PROBES, std::vector<double>(N_MEAS, 0.0));

    // --- measurement: paired signal/control counting on one run ---
    std::vector<double> prev_sig(N, 0.0), prev_ctl(N, 0.0);
    std::vector<long long> cnt_sig(N, 0), cnt_ctl(N, 0);
    bool first = true;
    for (int t = 0; t < N_MEAS; ++t) {
        rb.run(1);
        const auto& vox = crb.voxels();
        const double f1 = std::cos(O1 * t + th1);
        const double f2 = std::cos(O2 * t + th2);
        double mx = 0.0, my = 0.0, mz = 0.0;
        for (int i = 0; i < N; ++i) {
            mx += vox[i].flux.x;
            my += vox[i].flux.y;
            mz += vox[i].flux.z;
        }
        mx /= N; my /= N; mz /= N;
        for (int i = 0; i < N; ++i) {
            const int x =  i % L;
            const int y = (i / L) % L;
            const auto& J = vox[i].flux;
            const double jx = J.x - mx, jy = J.y - my, jz = J.z - mz;
            const double coh = A1 * c1x[x] * f1 + A2 * c2y[y] * f2;
            const double zs  = jz + coh;
            const double msig = std::sqrt(jx * jx + jy * jy + zs * zs);
            const double mctl = std::sqrt(jx * jx + jy * jy + jz * jz);
            if (!first) {
                if (prev_sig[i] < K && msig >= K) ++cnt_sig[i];
                if (prev_ctl[i] < K && mctl >= K) ++cnt_ctl[i];
            }
            prev_sig[i] = msig;
            prev_ctl[i] = mctl;
        }
        for (int p = 0; p < N_PROBES; ++p) {
            const auto& Jp = vox[probes[p]].flux;
            const double px = Jp.x - mx, py = Jp.y - my, pz = Jp.z - mz;
            probe_series[p][t] = pz;
            probe_mag[p][t] = std::sqrt(px * px + py * py + pz * pz);
        }
        first = false;
        if ((t + 1) % 5000 == 0)
            std::printf("    tick %d/%d\n", t + 1, N_MEAS);
    }

    // --- tau estimators: integrated autocorrelation to first zero ---
    // tau_z: of J_z (soft-mode-dominated, reported for context);
    // tau_mag: of |J| (threshold-relevant magnitude, PRIMARY regime axis).
    auto integrated_tau = [](const std::vector<std::vector<double>>& series) {
        double tau_sum = 0.0;
        for (const auto& s : series) {
            double mean = 0.0;
            for (double v : s) mean += v;
            mean /= s.size();
            double c0 = 0.0;
            for (double v : s) c0 += (v - mean) * (v - mean);
            c0 /= s.size();
            double tau = 0.5;
            for (int lag = 1; lag <= AC_MAX; ++lag) {
                double c = 0.0;
                const int nn = (int)s.size() - lag;
                for (int t = 0; t < nn; ++t)
                    c += (s[t] - mean) * (s[t + lag] - mean);
                c /= nn;
                const double r = (c0 > 0) ? c / c0 : 0.0;
                if (r <= 0.0) break;
                tau += r;
            }
            tau_sum += tau;
        }
        return tau_sum / series.size();
    };
    const double tau_z = integrated_tau(probe_series);
    const double tau_eff = integrated_tau(probe_mag);   // PRIMARY

    // --- regression of per-site excess on {1, cos2k1x, cos2k2y} ---
    double S11 = 0, S1b = 0, S1c = 0, Sbb = 0, Sbc = 0, Scc = 0;
    double Sy1 = 0, Syb = 0, Syc = 0;
    long long tot_sig = 0, tot_ctl = 0;
    for (int i = 0; i < N; ++i) {
        const int x =  i % L;
        const int y = (i / L) % L;
        const double ex = double(cnt_sig[i] - cnt_ctl[i]);
        const double b = b1x[x], c = b2y[y];
        S11 += 1;   S1b += b;     S1c += c;
        Sbb += b*b; Sbc += b*c;   Scc += c*c;
        Sy1 += ex;  Syb += ex*b;  Syc += ex*c;
        tot_sig += cnt_sig[i];
        tot_ctl += cnt_ctl[i];
    }
    // solve 3x3 normal equations
    double M[3][4] = {{S11, S1b, S1c, Sy1},
                      {S1b, Sbb, Sbc, Syb},
                      {S1c, Sbc, Scc, Syc}};
    for (int r = 0; r < 3; ++r) {
        int piv = r;
        for (int q = r + 1; q < 3; ++q)
            if (std::fabs(M[q][r]) > std::fabs(M[piv][r])) piv = q;
        for (int cc2 = 0; cc2 < 4; ++cc2) std::swap(M[r][cc2], M[piv][cc2]);
        for (int q = 0; q < 3; ++q) {
            if (q == r) continue;
            const double f = M[q][r] / M[r][r];
            for (int cc2 = r; cc2 < 4; ++cc2) M[q][cc2] -= f * M[r][cc2];
        }
    }
    const double coef1 = M[1][3] / M[1][1];
    const double coef2 = M[2][3] / M[2][2];
    const double R = coef2 / coef1;
    const double R_amp = O1 / O2;
    const double bf = (R - R_amp) / (1.0 - R_amp);

    // PRIMARY regime axis: the crossing clock tau_cross = 1/nu_ctl,
    // the mean interval between control upcrossings per site.
    const double nu_ctl = double(tot_ctl) /
                          (double(N) * double(N_MEAS - 1));
    const double tau_cross = (nu_ctl > 0) ? 1.0 / nu_ctl : -1.0;

    CellResult res;
    res.gamma = cs.gamma; res.O1 = O1; res.O2 = O2; res.sigma_z = sigma_z;
    res.tau_eff = tau_eff; res.tau_cross = tau_cross;
    res.bf = bf; res.R = R;
    res.c1 = coef1; res.c2 = coef2;
    res.net_excess = tot_sig - tot_ctl; res.ctl_counts = tot_ctl;
    res.lam1 = cs.lam1; res.lam2 = cs.lam2;
    res.valid = (tot_ctl > 0 && coef1 > 0);
    std::printf("  [cell g=%.2f sig=%.2f lam=(%d,%d)] sigma_z=%.4f A1=%.4f\n"
                "    tau_cross=%.2f (PRIMARY; nu_ctl=%.4f) tau_mag=%.1f "
                "tau_z=%.1f  Om*tau_cross=(%.2f,%.2f)\n",
                cs.gamma, cs.sig, cs.lam1, cs.lam2, sigma_z, A1,
                tau_cross, nu_ctl, tau_eff, tau_z,
                O1 * tau_cross, O2 * tau_cross);
    std::printf("    counts: ctl %lld, net excess %lld;  c=(%.4g, %.4g)  "
                "R=%.4f (R_amp=%.4f)  BF=%.4f\n",
                res.ctl_counts, res.net_excess, coef1, coef2, R, R_amp, bf);
    return res;
}

}  // namespace

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("==============================================================\n");
    std::printf(" Born regime map — engine-side shakedown (read-only overlay)\n");
    std::printf(" L=%d meas=%d T_init=%.3f A_frac=%.2f K=%.10f "
                "(sigma_target per cell)\n",
                L, N_MEAS, T_INIT, A_FRAC, ftd::K_GENESIS);
    std::printf("==============================================================\n");
    const bool quick = (argc > 1 && std::strcmp(argv[1], "--quick") == 0);
    std::vector<CellResult> results;
    int ci = 0;
    for (const auto& cs : CELLS) {
        results.push_back(run_cell(cs, SEED0 + 977 * ci));
        ++ci;
        if (quick && ci >= 1) break;   // rev-4 quick: DC-fix cell only
    }
    std::printf("\n%6s %9s %8s %9s %10s %10s %12s %8s %6s\n",
                "gamma", "lam", "sigma_z", "tau_cross", "Om1*tauc",
                "Om2*tauc", "net_excess", "BF", "valid");
    for (const auto& r : results)
        std::printf("%6.2f  (%2d,%2d) %8.4f %9.2f %10.2f %10.2f %12lld "
                    "%8.4f %6s\n",
                    r.gamma, r.lam1, r.lam2, r.sigma_z, r.tau_cross,
                    r.O1 * r.tau_cross, r.O2 * r.tau_cross, r.net_excess,
                    r.bf, r.valid ? "yes" : "NO");
    std::printf("\nSHAKEDOWN ONLY — no claim, no tag, prereg lock pending.\n");
    return 0;
}

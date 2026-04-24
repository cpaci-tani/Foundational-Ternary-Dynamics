/**
 * @file benchmark_alpha_convergence.cpp
 * @brief α_eff continuum-limit convergence study at high SOR precision.
 *
 * Physics question
 * ----------------
 * Does the native Coulomb response converge to the geometric lattice
 * normalization α_geom = 1/(2π) as L → ∞?
 *
 * Phase G established that α_r(r, L) = 2·r·G_L(r), where G_L is the lattice
 * Poisson Green's function at separation r on an L³ periodic lattice. The
 * coupling IS the geometric Green's function — no free parameter.
 *
 * This benchmark validates that finding quantitatively by measuring the
 * geometric coupling via V(r) = E_pair - 2·E_self at multiple L, with SOR=100
 * precision. The continuum extrapolation uses 1/L → 0 scaling.
 *
 * Method
 * ------
 * For each L ∈ {16, 24, 32, 48, 64}:
 *   1. Measure self-energy of +1 and −1 charges (locked, 300 ticks)
 *   2. Measure pair energy at separations r ∈ [3, L/3]
 *   3. Extract V(r) = E_pair(r) − E_self(+) − E_self(−)
 *   4. Fit V(r) = −α_eff/r + const via linear regression on (1/r, V)
 *   5. Compare the measured geometric coupling to α_geom = 1/(2π)
 *   6. Also compare to Phase G prediction: α_G(L) = mean_r[−V(r)·r]
 *
 * Epistemic status: [MEASUREMENT]. The V(r) ~ 1/r fit form is imported
 * from continuum Coulomb; FTD's claim is that the lattice produces the
 * geometric normalization.
 *
 * Expected output: the geometric coupling approaches α_geom as L grows,
 * with finite-size corrections scaling as O(1/L).
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <string>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/eft/coupling_measurement.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

/// Measure field energy for a single locked charge at lattice center.
/// Uses SOR=30 Gauss projection + Coulomb for high-precision fields.
double measure_self_energy(int L, int8_t sign, int n_ticks) {
    ftd::RenderBridge rb(L);
    ftd::eft::configure_bare_lattice_for_coupling(rb);
    rb.set_sor_iterations(100);

    const int mid = L / 2;
    rb.inject_particle(mid, mid, mid, sign, {0, 0, static_cast<double>(sign) * 0.05});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

/// Measure field energy for a locked +1/−1 pair at separation r.
double measure_pair_energy(int L, int r, int n_ticks) {
    ftd::RenderBridge rb(L);
    ftd::eft::configure_bare_lattice_for_coupling(rb);
    rb.set_sor_iterations(100);

    const int mid = L / 2;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, 0.05});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -0.05});
    rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

struct VofR {
    int r;
    double V;
    double alpha_r;   // −V·r
};

struct AlphaResult {
    int L;
    double alpha_fit;
    double r2;
    double alpha_mean;  // mean of −V·r across all r
    double e_self_pos;
    double e_self_neg;
    double wall_sec;
    std::vector<VofR> data;
};

struct WindowSummary {
    int L = 0;
    const char* name = "";
    int r_min = 0;
    int r_max = 0;
    int n = 0;
    double alpha_mean = 0.0;
    double alpha_G_mean = 0.0;
    double mean_err_vs_G = 0.0;
};

AlphaResult measure_alpha_at_L(int L, int n_ticks,
                               const std::vector<int>& r_values = {}) {
    AlphaResult out;
    out.L = L;

    const auto t0 = std::chrono::high_resolution_clock::now();

    // Step 1: Self-energies
    out.e_self_pos = measure_self_energy(L, +1, n_ticks);
    out.e_self_neg = measure_self_energy(L, -1, n_ticks);
    const double E_2self = out.e_self_pos + out.e_self_neg;

    // Step 2: Pair energies at multiple r
    std::vector<int> samples = r_values;
    if (samples.empty()) {
        const int r_min = 3;
        const int r_max = L / 3;
        const int r_step = (L <= 24) ? 1 : 2;
        for (int r = r_min; r <= r_max; r += r_step) samples.push_back(r);
    }

    for (int r : samples) {
        double E_pair = measure_pair_energy(L, r, n_ticks);
        double V = E_pair - E_2self;
        VofR pt;
        pt.r = r;
        pt.V = V;
        pt.alpha_r = -V * static_cast<double>(r);
        out.data.push_back(pt);
    }

    // Step 3: Linear regression V(r) vs 1/r → slope = −α
    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (const auto& p : out.data) {
            double x = 1.0 / static_cast<double>(p.r);
            double y = p.V;
            sx += x; sy += y; sxx += x * x; sxy += x * y;
        }
        double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            double slope = (n * sxy - sx * sy) / denom;
            double intercept = (sy - slope * sx) / n;
            out.alpha_fit = -slope;

            double ybar = sy / n;
            double ss_tot = 0, ss_res = 0;
            for (const auto& p : out.data) {
                double x = 1.0 / static_cast<double>(p.r);
                double y = p.V;
                double yhat = intercept + slope * x;
                ss_tot += (y - ybar) * (y - ybar);
                ss_res += (y - yhat) * (y - yhat);
            }
            out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
        }
    }

    // Step 4: Mean of −V·r (model-free α extraction)
    if (!out.data.empty()) {
        double sum = 0;
        for (const auto& p : out.data) sum += p.alpha_r;
        out.alpha_mean = sum / out.data.size();
    }

    const auto t1 = std::chrono::high_resolution_clock::now();
    out.wall_sec = std::chrono::duration<double>(t1 - t0).count();
    return out;
}

/// Compute the lattice Poisson Green's function G_L(r) at separation r
/// on an L³ periodic lattice (analytical sum in Fourier space).
double lattice_greens_function(int L, int r) {
    double G = 0.0;
    const double twopi_L = 2.0 * M_PI / L;
    for (int kx = 0; kx < L; ++kx)
        for (int ky = 0; ky < L; ++ky)
            for (int kz = 0; kz < L; ++kz) {
                if (kx == 0 && ky == 0 && kz == 0) continue;  // skip zero mode
                double sx = std::sin(twopi_L * kx * 0.5);
                double sy = std::sin(twopi_L * ky * 0.5);
                double sz = std::sin(twopi_L * kz * 0.5);
                double lambda = 4.0 * (sx*sx + sy*sy + sz*sz);
                // G(r) = (1/L³) Σ_k cos(k·r) / λ_k
                // For displacement along x-axis: k·r = (2π/L)·kx·r
                double phase = twopi_L * kx * r;
                G += std::cos(phase) / lambda;
            }
    return G / (L * L * L);
}

WindowSummary summarize_window(const AlphaResult& ar, const char* name,
                               int r_min, int r_max) {
    WindowSummary ws;
    ws.L = ar.L;
    ws.name = name;
    ws.r_min = r_min;
    ws.r_max = r_max;

    double sum_meas = 0.0;
    double sum_G = 0.0;
    double sum_err = 0.0;
    for (const auto& pt : ar.data) {
        if (pt.r < r_min || pt.r > r_max) continue;
        const double alpha_G = 2.0 * pt.r * lattice_greens_function(ar.L, pt.r);
        const double err = (std::abs(alpha_G) > 1e-30)
            ? 100.0 * std::abs(pt.alpha_r - alpha_G) / std::abs(alpha_G)
            : 0.0;
        sum_meas += pt.alpha_r;
        sum_G += alpha_G;
        sum_err += err;
        ++ws.n;
    }

    if (ws.n > 0) {
        ws.alpha_mean = sum_meas / ws.n;
        ws.alpha_G_mean = sum_G / ws.n;
        ws.mean_err_vs_G = sum_err / ws.n;
    }
    return ws;
}

void print_window_summary(const WindowSummary& ws, double alpha_geom) {
    const double err_geom = (ws.n > 0)
        ? 100.0 * std::abs(ws.alpha_mean - alpha_geom) / alpha_geom
        : 0.0;
    std::printf("  %-18s L=%-3d r=[%d,%d] n=%-2d "
                "mean(-V*r)=%.8f mean(2rG)=%.8f "
                "err_vs_geom=%7.3f%% mean_err_vs_G=%7.3f%%\n",
                ws.name, ws.L, ws.r_min, ws.r_max, ws.n,
                ws.alpha_mean, ws.alpha_G_mean, err_geom, ws.mean_err_vs_G);
}

void print_linear_extrapolation(const std::vector<WindowSummary>& windows,
                                const char* label,
                                double alpha_geom) {
    std::vector<WindowSummary> usable;
    for (const auto& ws : windows) {
        if (ws.n > 0) usable.push_back(ws);
    }
    if (usable.size() < 3) {
        std::printf("  %-18s not enough points for 1/L extrapolation\n", label);
        return;
    }

    const int n = static_cast<int>(usable.size());
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    for (const auto& ws : usable) {
        const double x = 1.0 / ws.L;
        const double y = ws.alpha_mean;
        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
    }
    const double denom = n * sxx - sx * sx;
    if (std::abs(denom) <= 1e-30) return;
    const double slope = (n * sxy - sx * sy) / denom;
    const double alpha_inf = (sy - slope * sx) / n;
    const double err = 100.0 * std::abs(alpha_inf - alpha_geom) / alpha_geom;
    std::printf("  %-18s alpha_inf=%.8f slope=%+.6f err_vs_1/(2pi)=%.4f%%\n",
                label, alpha_inf, slope, err);
}

}  // namespace

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  α_eff CONTINUUM-LIMIT CONVERGENCE STUDY (SOR=100)\n");
    const double alpha_geom = 1.0 / (2.0 * M_PI);
    std::printf("  Reference: α_geom = 1/(2π) = %.8f\n", alpha_geom);
    std::printf("  Phase G prediction: α_r = 2·r·G_L(r)\n");
    std::printf("================================================================\n\n");

    // Lattice sizes. Pass one or more integer L values on the command line to
    // isolate expensive points, e.g. `benchmark_alpha_convergence 256`.
    // Add `--fixed-window` to sample only r={5,7,9}.
    std::vector<int> Ls = {32, 48, 64, 96, 128, 192, 256};
    bool fixed_window_only = false;
    if (argc > 1) {
        Ls.clear();
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--fixed-window" || arg == "--fixed-r") {
                fixed_window_only = true;
                continue;
            }
            int L = std::atoi(arg.c_str());
            if (L > 0) Ls.push_back(L);
        }
        if (Ls.empty()) Ls = {32, 48, 64, 96, 128, 192, 256};
    }
    const int n_ticks = 400;

    std::printf("%-5s  %12s  %12s  %12s  %10s  %7s  %5s\n",
                "L", "α_fit", "α_mean", "err_fit(%)", "err_mean(%)", "R²", "wall");
    std::printf("-----  ------------  ------------  ------------  ----------  -------  -----\n");

    std::vector<AlphaResult> results;
    std::vector<WindowSummary> fixed_near_windows;
    std::vector<WindowSummary> intermediate_windows;

    for (int L : Ls) {
        std::printf("  Running L=%d...\n", L);
        std::fflush(stdout);

        const std::vector<int> fixed_r = {5, 7, 9};
        AlphaResult ar = fixed_window_only
            ? measure_alpha_at_L(L, n_ticks, fixed_r)
            : measure_alpha_at_L(L, n_ticks);
        results.push_back(ar);

        double err_fit  = 100.0 * std::abs(ar.alpha_fit  - alpha_geom) / alpha_geom;
        double err_mean = 100.0 * std::abs(ar.alpha_mean - alpha_geom) / alpha_geom;

        std::printf("%-5d  %12.8f  %12.8f  %12.4f  %10.4f  %.5f  %.1fs\n",
                    L, ar.alpha_fit, ar.alpha_mean, err_fit, err_mean, ar.r2, ar.wall_sec);

        // Print V(r) detail
        std::printf("  V(r):");
        for (const auto& p : ar.data) {
            std::printf("  r=%d V=%+.3e α_r=%.5f", p.r, p.V, p.alpha_r);
        }
        std::printf("\n");

        // Fixed-r near-continuum window: excludes the r=3 contact/core point
        // while keeping r/L -> 0 as L grows.
        auto fixed_near = summarize_window(ar, "fixed-r[5,9]", 5, 9);
        fixed_near_windows.push_back(fixed_near);
        print_window_summary(fixed_near, alpha_geom);

        // Scaled intermediate window: avoids the source core and the torus
        // edge regime. This is primarily a finite-lattice Green's-function
        // alignment check, not a direct continuum-normalization estimator.
        const int r_mid_min = std::max(7, ar.L / 8);
        const int r_mid_max = ar.L / 4;
        auto intermediate = summarize_window(ar, "mid[L/8,L/4]",
                                             r_mid_min, r_mid_max);
        intermediate_windows.push_back(intermediate);
        print_window_summary(intermediate, alpha_geom);
    }

    std::printf("\n================================================================\n");
    std::printf("  WINDOWED SUMMARY\n");
    std::printf("================================================================\n\n");
    std::printf("Fixed-r window tests the continuum normalization because r/L -> 0.\n");
    std::printf("Intermediate window tests finite-torus Green's-function alignment.\n\n");
    print_linear_extrapolation(fixed_near_windows, "fixed-r[5,9]", alpha_geom);
    print_linear_extrapolation(intermediate_windows, "mid[L/8,L/4]", alpha_geom);

    // === Phase G comparison ===
    std::printf("\n================================================================\n");
    std::printf("  PHASE G COMPARISON: α_G(r) = 2·r·G_L(r) vs measured −V·r\n");
    std::printf("================================================================\n\n");

    for (const auto& ar : results) {
        std::printf("L=%d:\n", ar.L);
        std::printf("  %5s  %12s  %12s  %10s\n", "r", "meas −V·r", "2·r·G_L(r)", "err(%)");
        for (const auto& pt : ar.data) {
            double GL = lattice_greens_function(ar.L, pt.r);
            double alpha_G = 2.0 * pt.r * GL;
            double err = 100.0 * std::abs(pt.alpha_r - alpha_G) / alpha_G;
            std::printf("  %5d  %12.8f  %12.8f  %10.4f\n",
                        pt.r, pt.alpha_r, alpha_G, err);
        }
        std::printf("\n");
    }

    // === Continuum extrapolation ===
    std::printf("================================================================\n");
    std::printf("  CONTINUUM EXTRAPOLATION: α_eff(L) = α_∞ + c/L\n");
    std::printf("================================================================\n\n");

    if (results.size() >= 3) {
        // Linear fit: α_mean(L) = α_∞ + c · (1/L)
        int n = static_cast<int>(results.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (const auto& ar : results) {
            double x = 1.0 / ar.L;
            double y = ar.alpha_mean;
            sx += x; sy += y; sxx += x * x; sxy += x * y;
        }
        double denom = n * sxx - sx * sx;
        double slope = (n * sxy - sx * sy) / denom;
        double alpha_inf = (sy - slope * sx) / n;
        double err_inf = 100.0 * std::abs(alpha_inf - alpha_geom) / alpha_geom;

        std::printf("  Linear fit: α_∞ = %.8f  (err: %.4f%%)\n", alpha_inf, err_inf);
        std::printf("  Slope c = %.6f  (finite-size correction coefficient)\n", slope);
        std::printf("  Geometric Theory: α_geom = 1/(2π) = %.8f\n", alpha_geom);
        std::printf("  Match:     %s\n",
                    err_inf < 5.0 ? "WITHIN 5%% — CONVERGENT TO 1/(2π)" :
                    err_inf < 10.0 ? "WITHIN 10%%" : "NOT YET CONVERGED");
    }

    std::printf("\n================================================================\n");
    std::printf("  Done.\n");
    std::printf("================================================================\n");

    return 0;
}

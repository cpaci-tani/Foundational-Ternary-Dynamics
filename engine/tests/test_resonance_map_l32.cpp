/**
 * Phase B.3 (resonance investigation): fine resonance map at L=32.
 *
 * Hypothesis: cluster stability tracks resonance windows in (A, L) space —
 * stable amplitudes correspond to cluster sizes that match stable geometric/
 * eigenmode configurations of the lattice. Stability is a resonance
 * phenomenon, and the resonance positions depend on L.
 *
 * This test scans amplitude finely at L=32 to map the full resonance
 * structure: which amplitudes produce stable clusters (and at what cluster
 * size n_obs), and which are short-lived metastable transients (with what
 * τ_bind).
 *
 * Output: a (A, n_obs, τ_bind, regime, RMS) resonance map suitable for
 * comparison with same maps at L=48, L=64 to identify L-scaling.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static const double TWOPI = 2.0 * 3.14159265358979323846;

struct R {
    double A;
    int n_init;
    int n_max;
    int n_final;
    int t_first_growth;
    int t_full_flood;
    double rms_final;
    std::string regime;
};

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

static double compute_rms(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int64_t total = lat.total_sites();
    int n = 0;
    double sx_x=0, cx_x=0, sx_y=0, cx_y=0, sx_z=0, cx_z=0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        ++n;
        auto c = lat.coord(static_cast<int>(i));
        sx_x += std::sin(TWOPI*c.x/L); cx_x += std::cos(TWOPI*c.x/L);
        sx_y += std::sin(TWOPI*c.y/L); cx_y += std::cos(TWOPI*c.y/L);
        sx_z += std::sin(TWOPI*c.z/L); cx_z += std::cos(TWOPI*c.z/L);
    }
    if (n == 0) return 0.0;
    double cx = std::atan2(sx_x, cx_x) * L / TWOPI; if (cx < 0) cx += L;
    double cy = std::atan2(sx_y, cx_y) * L / TWOPI; if (cy < 0) cy += L;
    double cz = std::atan2(sx_z, cx_z) * L / TWOPI; if (cz < 0) cz += L;
    auto wrap = [L](double d) { if (d>L/2.0) d-=L; if (d<-L/2.0) d+=L; return d; };
    double rms = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        auto c = lat.coord(static_cast<int>(i));
        double dx = wrap(c.x - cx), dy = wrap(c.y - cy), dz = wrap(c.z - cz);
        rms += dx*dx + dy*dy + dz*dz;
    }
    return std::sqrt(rms / n);
}

static R run_one(double A, int L, int N_TICKS) {
    const int N_WARMUP = 50;
    const int SAMPLE = 25;
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    R r;
    r.A = A;
    r.n_init = count_manifested(rb);
    r.n_max = r.n_init;
    r.t_first_growth = -1;
    r.t_full_flood = -1;
    for (int t = 1; t <= N_TICKS; ++t) {
        rb.tick();
        if (t % SAMPLE == 0) {
            int n = count_manifested(rb);
            if (n > r.n_max) r.n_max = n;
            if (r.t_first_growth < 0 && n > r.n_init * 1.5 && n > 5) r.t_first_growth = t;
            if (r.t_full_flood < 0 && n > 1000) r.t_full_flood = t;
        }
    }
    r.n_final = count_manifested(rb);
    r.rms_final = compute_rms(rb);

    if (r.n_init == 0) r.regime = "none";
    else if (r.t_full_flood > 0) r.regime = "FLOODED";
    else if (r.t_first_growth > 0) r.regime = "GROWING";
    else if (r.n_final == 0) r.regime = "DECAY";
    else if (r.n_init == 1) r.regime = "BOUND-trivial";
    else r.regime = "STABLE";
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  RESONANCE MAP at L=32, +color+triad, fine A-scan\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_TICKS = 600;

    std::cout << "Configuration: L=" << L << ", N_TICKS=" << N_TICKS
              << ", +color+triad, single seed\n";
    std::cout << "Scan: A ∈ [3.0, 8.0] in 0.25 steps (21 amplitudes)\n\n";

    std::vector<double> A_vals;
    for (double A = 3.0; A <= 8.01; A += 0.25) A_vals.push_back(A);

    std::cout << "  A/K_G    n_init   n_max   n_final   t_growth   t_flood   rms_f    regime\n";
    std::cout << "  -----    ------   -----   -------   --------   -------   -----    ------\n";
    std::vector<R> all;
    for (double A : A_vals) {
        R r = run_one(A, L, N_TICKS);
        all.push_back(r);
        std::cout << std::fixed << std::setprecision(2)
                  << "  " << std::setw(5) << r.A << "    "
                  << std::setw(6) << r.n_init << "   "
                  << std::setw(5) << r.n_max << "   "
                  << std::setw(7) << r.n_final << "   "
                  << std::setw(8) << (r.t_first_growth < 0 ? std::string("none") : std::to_string(r.t_first_growth)) << "   "
                  << std::setw(7) << (r.t_full_flood < 0 ? std::string("none") : std::to_string(r.t_full_flood)) << "   "
                  << std::setw(5) << std::setprecision(2) << r.rms_final << "    "
                  << r.regime << "\n";
    }

    // Identify resonance windows
    std::cout << "\n--- Resonance windows (consecutive STABLE amplitudes) ---\n";
    int win_start = -1;
    int last_n = -2;
    for (size_t i = 0; i < all.size(); ++i) {
        bool is_stable = (all[i].regime == "STABLE" || all[i].regime == "BOUND-trivial");
        if (is_stable) {
            if (win_start < 0 || all[i].n_final != last_n) {
                if (win_start >= 0) {
                    std::cout << "  Window: A ∈ [" << std::fixed << std::setprecision(2)
                              << all[win_start].A << ", " << all[i-1].A << "] → n = "
                              << last_n << "\n";
                }
                win_start = static_cast<int>(i);
                last_n = all[i].n_final;
            }
        } else {
            if (win_start >= 0) {
                std::cout << "  Window: A ∈ [" << std::fixed << std::setprecision(2)
                          << all[win_start].A << ", " << all[i-1].A << "] → n = "
                          << last_n << "\n";
                win_start = -1;
            }
        }
    }
    if (win_start >= 0) {
        std::cout << "  Window: A ∈ [" << std::fixed << std::setprecision(2)
                  << all[win_start].A << ", " << all.back().A << "] → n = "
                  << last_n << "\n";
    }

    // Also print τ_bind for unstable amps to see resonance structure in flood timing
    std::cout << "\n--- τ_bind for unstable amplitudes (looking for resonant longer τ) ---\n";
    for (const auto& r : all) {
        if (r.regime == "GROWING" || r.regime == "FLOODED") {
            int tb = (r.t_first_growth > 0) ? r.t_first_growth : -1;
            std::cout << "  A=" << std::fixed << std::setprecision(2) << r.A
                      << "  τ_bind=" << (tb < 0 ? "?" : std::to_string(tb)) << " ticks\n";
        }
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (L=32 resonance map)\n";
    std::cout << "================================================================\n";
    return 0;
}

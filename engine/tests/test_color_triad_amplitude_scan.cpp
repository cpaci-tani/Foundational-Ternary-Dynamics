/**
 * Phase B.3 (δ''') Test 3: amplitude scan with +color+triad at L=32.
 *
 * The +color+triad A=7 configuration was the closest approach to a true
 * bound state in the bound-state search. This test scans amplitudes
 * A ∈ {4, 5, 6, 7, 8, 9, 10, 11, 12, 14}·K_GENESIS with the same
 * +color+triad config to determine: is A=7 a special amplitude, or is
 * the whole A range quasi-bound?
 *
 * 2 seeds per amplitude. Triplet-metric classification with refined
 * thresholds (drift < 2.0, rms < L/3 → BOUND-CANDIDATE).
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static const double TWOPI = 2.0 * 3.14159265358979323846;

struct Result {
    double A;
    int seed;
    int n_init, n_mid, n_final;
    double drift;
    double rms_final;
    std::string regime;
};

static void compute_state(const ftd::RenderBridge& rb, int& n_total,
                           double& cx, double& cy, double& cz, double& rms) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int64_t total = lat.total_sites();
    n_total = 0;
    double sx_x=0, cx_x=0, sx_y=0, cx_y=0, sx_z=0, cx_z=0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        ++n_total;
        auto c = lat.coord(static_cast<int>(i));
        sx_x += std::sin(TWOPI * c.x / L); cx_x += std::cos(TWOPI * c.x / L);
        sx_y += std::sin(TWOPI * c.y / L); cx_y += std::cos(TWOPI * c.y / L);
        sx_z += std::sin(TWOPI * c.z / L); cx_z += std::cos(TWOPI * c.z / L);
    }
    if (n_total == 0) { cx = cy = cz = rms = 0; return; }
    cx = std::atan2(sx_x, cx_x) * L / TWOPI; if (cx < 0) cx += L;
    cy = std::atan2(sx_y, cx_y) * L / TWOPI; if (cy < 0) cy += L;
    cz = std::atan2(sx_z, cx_z) * L / TWOPI; if (cz < 0) cz += L;
    auto wrap = [L](double d) { if (d > L/2.0) d -= L; if (d < -L/2.0) d += L; return d; };
    rms = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        auto c = lat.coord(static_cast<int>(i));
        double dx = wrap(c.x - cx), dy = wrap(c.y - cy), dz = wrap(c.z - cz);
        rms += dx*dx + dy*dy + dz*dz;
    }
    rms = std::sqrt(rms / n_total);
}

static Result run_one(double A, int seed, int L, int n_warmup, int n_trace) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < n_warmup; ++t) rb.tick();

    Result r;
    r.A = A; r.seed = seed;
    int n; double cx, cy, cz, rms;
    compute_state(rb, n, cx, cy, cz, rms);
    r.n_init = n;
    double cx0=cx, cy0=cy, cz0=cz;

    for (int t = 0; t < n_trace / 2; ++t) rb.tick();
    compute_state(rb, n, cx, cy, cz, rms);
    r.n_mid = n;

    for (int t = 0; t < n_trace / 2; ++t) rb.tick();
    compute_state(rb, n, cx, cy, cz, rms);
    r.n_final = n;
    r.rms_final = rms;
    auto wrap = [L](double d) { if (d > L/2.0) d -= L; if (d < -L/2.0) d += L; return d; };
    double dx = wrap(cx - cx0), dy = wrap(cy - cy0), dz = wrap(cz - cz0);
    r.drift = std::sqrt(dx*dx + dy*dy + dz*dz);

    double bound_drift_thresh = 2.0;
    double bound_rms_thresh = L / 3.0;
    if (r.n_final == 0) r.regime = "FULL DECAY";
    else if (r.n_final > 3 * r.n_init) r.regime = "FLOODING";
    else if (r.n_final < r.n_init / 3) r.regime = "DECAYING";
    else if (r.drift > 3.0) r.regime = "SOLITON";
    else if (r.rms_final > L / 2.5) r.regime = "DIFFUSING";
    else if (r.drift < bound_drift_thresh && r.rms_final < bound_rms_thresh)
        r.regime = "BOUND-CANDIDATE";
    else r.regime = "QUASI-BOUND";
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (δ''') Test 3: +color+triad amplitude scan at L=32\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 300;
    std::vector<double> A_vals = {4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 14.0};
    std::vector<int> seeds = {1, 2};

    std::cout << "Configuration: L=" << L << ", warmup=" << N_WARMUP
              << ", trace=" << N_TRACE
              << ", toggle: defaults + color_forces + triad_binding\n\n";

    std::cout << "  A/K_G  seed  n_init  n_mid  n_final  drift  rms_f  regime\n";
    std::cout << "  -----  ----  ------  -----  -------  -----  -----  ------\n";

    std::vector<Result> all;
    for (double A : A_vals) {
        for (int seed : seeds) {
            Result r = run_one(A, seed, L, N_WARMUP, N_TRACE);
            all.push_back(r);
            std::cout << std::fixed << std::setprecision(1)
                      << "  " << std::setw(5) << r.A << "  "
                      << std::setw(4) << r.seed << "  "
                      << std::setw(6) << r.n_init << "  "
                      << std::setw(5) << r.n_mid << "  "
                      << std::setw(7) << r.n_final << "  "
                      << std::setw(5) << std::setprecision(2) << r.drift << "  "
                      << std::setw(5) << r.rms_final << "  "
                      << r.regime << "\n";
        }
    }

    std::cout << "\n--- Per-amplitude regime tally ---\n";
    for (double A : A_vals) {
        std::vector<std::string> regimes;
        for (const auto& r : all) if (std::abs(r.A - A) < 0.01) regimes.push_back(r.regime);
        std::cout << "  A=" << std::fixed << std::setprecision(1) << A << ": ";
        for (const auto& r : regimes) std::cout << r << " ";
        std::cout << "\n";
    }

    int n_quasi_bound_amps = 0;
    for (double A : A_vals) {
        bool all_quasi = true;
        for (const auto& r : all) {
            if (std::abs(r.A - A) < 0.01 &&
                r.regime != "BOUND-CANDIDATE" && r.regime != "QUASI-BOUND") {
                all_quasi = false; break;
            }
        }
        if (all_quasi) ++n_quasi_bound_amps;
    }

    std::cout << "\n--- Verdict ---\n";
    std::cout << "  Amplitudes with ALL seeds quasi-bound or bound-candidate: "
              << n_quasi_bound_amps << " / " << A_vals.size() << "\n";

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}

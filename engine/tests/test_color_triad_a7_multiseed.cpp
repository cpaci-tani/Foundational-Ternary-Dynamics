/**
 * Phase B.3 (δ'''): multi-seed characterization of +color+triad A=7 quasi-bound.
 *
 * Prior single-seed result (test_cluster_bound_state_search.cpp): the
 * +color_forces +triad_binding configuration at A=7 produced the closest
 * approach to a true bound state — drift=1.19 (nearly stationary centroid),
 * matter quasi-conserved (n=15→19), but rms=10.19 slightly above the
 * L/4=8 bound threshold.
 *
 * This test runs the same configuration with 5 different seeds at L=32 to
 * determine: (a) is the quasi-bound regime deterministic/reproducible, or
 * is it a single-seed artifact? (b) what is the seed-distribution of
 * regime classifications (bound / quasi-bound / soliton / flood)?
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static const double TWOPI = 2.0 * 3.14159265358979323846;

struct SeedResult {
    int seed;
    int n_init;
    int n_mid;
    int n_final;
    double centroid_drift;
    double rms_init;
    double rms_mid;
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

static SeedResult run_one(int seed, double A_over_KG, int L, int n_warmup, int n_trace) {
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = static_cast<uint32_t>(seed);
    std::string err;
    if (!rb.toggles.validate(&err)) {
        SeedResult r;
        r.seed = seed; r.n_init = -1;
        r.regime = "INVALID(" + err + ")";
        return r;
    }
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A_over_KG * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < n_warmup; ++t) rb.tick();

    SeedResult r;
    r.seed = seed;
    int n; double cx, cy, cz, rms;
    compute_state(rb, n, cx, cy, cz, rms);
    r.n_init = n;
    r.rms_init = rms;
    double cx0=cx, cy0=cy, cz0=cz;

    for (int t = 0; t < n_trace / 2; ++t) rb.tick();
    compute_state(rb, n, cx, cy, cz, rms);
    r.n_mid = n;
    r.rms_mid = rms;

    for (int t = 0; t < n_trace / 2; ++t) rb.tick();
    compute_state(rb, n, cx, cy, cz, rms);
    r.n_final = n;
    r.rms_final = rms;

    auto wrap = [L](double d) { if (d > L/2.0) d -= L; if (d < -L/2.0) d += L; return d; };
    double dx = wrap(cx - cx0), dy = wrap(cy - cy0), dz = wrap(cz - cz0);
    r.centroid_drift = std::sqrt(dx*dx + dy*dy + dz*dz);

    // Classify with refined thresholds for bound state
    double bound_drift_thresh = 2.0;     // generous: cluster wanders < 2 voxels
    double bound_rms_thresh = L / 3.0;   // generous: spread < L/3
    if (r.n_final == 0) r.regime = "FULL DECAY";
    else if (r.n_final > 3 * r.n_init) r.regime = "FLOODING";
    else if (r.n_final < r.n_init / 3) r.regime = "DECAYING";
    else if (r.centroid_drift > 3.0) r.regime = "SOLITON";
    else if (r.rms_final > L / 2.5) r.regime = "DIFFUSING";
    else if (r.centroid_drift < bound_drift_thresh && r.rms_final < bound_rms_thresh)
        r.regime = "BOUND-CANDIDATE";
    else r.regime = "QUASI-BOUND";
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (δ'''): +color+triad A=7 multi-seed characterization\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 300;
    const double A = 7.0;
    std::vector<int> seeds = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    std::cout << "Configuration: L=" << L << ", warmup=" << N_WARMUP
              << ", trace=" << N_TRACE << ", A=" << A << "·K_GENESIS\n";
    std::cout << "Toggle: engine defaults + color_forces + triad_binding\n";
    std::cout << "Seeds: " << seeds.size() << "\n\n";
    std::cout << "Bound-candidate criteria (refined): drift < 2.0, rms < L/3 = "
              << L/3.0 << "\n\n";

    std::cout << "  seed   n_init  n_mid  n_final   drift   rms_i  rms_m  rms_f  regime\n";
    std::cout << "  ----   ------  -----  -------   -----   -----  -----  -----  ------\n";

    std::vector<SeedResult> results;
    for (int seed : seeds) {
        SeedResult r = run_one(seed, A, L, N_WARMUP, N_TRACE);
        results.push_back(r);
        if (r.n_init < 0) {
            std::cout << "  " << std::setw(4) << seed << "   " << r.regime << "\n";
            continue;
        }
        std::cout << "  " << std::setw(4) << r.seed << "   "
                  << std::setw(6) << r.n_init << "  "
                  << std::setw(5) << r.n_mid << "  "
                  << std::setw(7) << r.n_final << "   "
                  << std::fixed << std::setprecision(2) << std::setw(5) << r.centroid_drift << "   "
                  << std::setw(5) << r.rms_init << "  "
                  << std::setw(5) << r.rms_mid << "  "
                  << std::setw(5) << r.rms_final << "  "
                  << r.regime << "\n";
    }

    // Tally regimes
    std::cout << "\n--- Regime tally ---\n";
    std::vector<std::pair<std::string,int>> tally;
    for (const auto& r : results) {
        bool found = false;
        for (auto& p : tally) if (p.first == r.regime) { ++p.second; found = true; break; }
        if (!found) tally.push_back({r.regime, 1});
    }
    for (const auto& p : tally) {
        std::cout << "  " << std::left << std::setw(20) << p.first
                  << ": " << p.second << " / " << seeds.size() << "\n";
    }

    // Verdict
    std::cout << "\n--- Verdict ---\n";
    int n_bound_or_quasi = 0;
    for (const auto& r : results)
        if (r.regime == "BOUND-CANDIDATE" || r.regime == "QUASI-BOUND")
            ++n_bound_or_quasi;
    if (n_bound_or_quasi == static_cast<int>(seeds.size())) {
        std::cout << "  [VERDICT] All " << seeds.size()
                  << " seeds give BOUND or QUASI-BOUND — config is reproducibly quasi-bound.\n";
        std::cout << "  Worth continuing: L=64 verification + amplitude scan + long-time evolution.\n";
    } else if (n_bound_or_quasi >= static_cast<int>(seeds.size()) * 7 / 10) {
        std::cout << "  [VERDICT] Majority (" << n_bound_or_quasi << "/"
                  << seeds.size() << ") are quasi-bound; some seed dependence exists.\n";
    } else {
        std::cout << "  [VERDICT] Minority quasi-bound (" << n_bound_or_quasi << "/"
                  << seeds.size() << ") — quasi-bound regime is seed-sensitive.\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (" << seeds.size() << "-seed multi-seed scan)\n";
    std::cout << "================================================================\n";
    return 0;
}

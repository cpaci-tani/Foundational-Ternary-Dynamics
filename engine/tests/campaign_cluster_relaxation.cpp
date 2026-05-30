/**
 * campaign_cluster_relaxation — Exp-B of the cluster-thermodynamics
 * EXPLORATORY pass (P4 N_internal + P1 cost<->N).
 *
 * EXPLORATORY, NOT pre-registered. See .claude/plans/lazy-conjuring-marble.md.
 * Results are signal-detection only and are NOT evidence.
 *
 * P4 (N_internal): does a cluster's reconstitution/relaxation time grow with
 * scrambling temperature T and size N, HOLDING KINEMATICS FIXED? Kinematics are
 * pinned by construction: movement=OFF (centroid fixed, v~0) and latency_field=OFF
 * (so the engine's kinematic proper-time v.tau never accumulates -> the
 * kinematic-null is satisfied trivially, recorded as mean_tau_kin). Any
 * T-dependence of tau_relax is therefore the INTERNAL (non-kinematic) effect.
 *
 * Protocol per (T, A, seed): inject one flux blob, equilibrate, snapshot the
 * cluster mask M0 + baseline org_eq / N_eq / E_wave_eq; PERTURB by deleting a
 * fixed half of M0 (state=flux=wave_vel=0 for x >= centroid_x); then measure
 * org(t)/org_eq recovery. tau_relax = first tick recovering 1-1/e of the deficit
 * (org reaches q_min + (1-1/e)(1-q_min)); censored at N_REC if never reached.
 *
 * P1 piggyback: steady-state cluster E_wave (maintenance-cost proxy) vs N_eq,
 * swept over A -> cost<->N correlation.
 *
 * Usage: campaign_cluster_relaxation [--L 32] [--seeds 8] [--output-dir DIR] [--smoke]
 */
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/cluster_observables.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace ftd;

static int N_EQ  = 150;
static int N_REC = 300;

// Largest sign-grouped Moore-connected manifested component (>= min_size); empty if none.
static std::vector<int> largest_component(const RenderBridge& rb, int min_size) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<int> label(total, -1);
    std::vector<int> best;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0 || label[i] != -1) continue;
        int8_t sign = vox[i].state;
        int cid = i;  // unique
        std::vector<int> comp, q; q.push_back(static_cast<int>(i)); label[i] = cid;
        while (!q.empty()) {
            int idx = q.back(); q.pop_back();
            comp.push_back(idx);
            for (int n : lat.neighbors_26(idx))
                if (label[n] == -1 && vox[n].state == sign) { label[n] = cid; q.push_back(n); }
        }
        if (static_cast<int>(comp.size()) >= min_size && comp.size() > best.size()) best = std::move(comp);
    }
    return best;
}

static double mean_tau(const RenderBridge& rb, const std::vector<int>& idxs) {
    if (idxs.empty()) return 0.0;
    const auto& vox = rb.voxels();
    double s = 0.0; for (int i : idxs) s += vox[i].tau;
    return s / idxs.size();
}

struct RelaxRow {
    double T, A; unsigned seed;
    int N_eq; double org_eq, E_wave_eq, q_min;
    int tau_relax; bool censored; double mean_tau_kin;
};

static RelaxRow run_relaxation(int L, double T, double A, unsigned seed) {
    RenderBridge rb(L);
    rb.force_cpu();
    rb.toggles.disable_all();         // movement OFF, latency_field OFF, dual_substrate OFF
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.langevin_seed    = seed;
    rb.seed_rng(seed);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    rb.inject_flux(cx, cy, cz, {A * K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_EQ; ++t) rb.tick();

    RelaxRow r; r.T = T; r.A = A; r.seed = seed;
    std::vector<int> C0 = largest_component(rb, 4);
    ClusterMeasure base = measure_cluster(rb, C0);
    r.N_eq = base.size; r.org_eq = base.org; r.E_wave_eq = base.E_wave;
    r.mean_tau_kin = mean_tau(rb, C0);
    r.tau_relax = N_REC; r.censored = true; r.q_min = 1.0;

    if (C0.empty() || base.org <= 1e-9) {  // no cluster formed
        r.censored = true; return r;
    }

    // PERTURB: delete the half of M0 with x >= centroid_x.
    {
        auto& vox = rb.voxels();
        const auto& lat = rb.lattice();
        double cxm = base.centroid.x;
        for (int i : C0) {
            if (lat.coord(i).x >= cxm) { vox[i].state = 0; vox[i].flux = Vec3(); vox[i].wave_vel = Vec3(); }
        }
    }
    {
        std::vector<int> Cp = largest_component(rb, 1);
        double org_post = Cp.empty() ? 0.0 : measure_cluster(rb, Cp).org;
        r.q_min = org_post / r.org_eq;
    }
    const double recover_target = r.q_min + (1.0 - 1.0 / std::exp(1.0)) * (1.0 - r.q_min);

    for (int t = 1; t <= N_REC; ++t) {
        rb.tick();
        std::vector<int> Ct = largest_component(rb, 1);
        double org_t = Ct.empty() ? 0.0 : measure_cluster(rb, Ct).org;
        double q = org_t / r.org_eq;
        if (q >= recover_target) { r.tau_relax = t; r.censored = false; break; }
    }
    return r;
}

int main(int argc, char** argv) {
    std::string outdir = ".";
    int L = 32, seeds = 8; bool smoke = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--output-dir" && i + 1 < argc) outdir = argv[++i];
        else if (a == "--L" && i + 1 < argc) L = std::atoi(argv[++i]);
        else if (a == "--seeds" && i + 1 < argc) seeds = std::atoi(argv[++i]);
        else if (a == "--smoke") smoke = true;
    }
    if (smoke) { L = 16; seeds = 1; N_EQ = 40; N_REC = 60; }

    std::cout << "================================================================\n";
    std::cout << "  EXPLORATORY campaign_cluster_relaxation  L=" << L << " seeds=" << seeds
              << (smoke ? "  [SMOKE]" : "") << "\n";
    std::cout << "  P4 N_internal: tau_relax vs (T, N) at v~0, latency OFF.\n";
    std::cout << "  (NOT pre-registered; signal-detection only.)\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed;

    std::vector<double> Ts = smoke ? std::vector<double>{0.005} : std::vector<double>{0.005, 0.01, 0.02, 0.05};
    // amplitudes chosen to form clusters in the quiescent single-inject config
    // (feasibility check 2026-05-30: A<=8 -> N_eq=0; clusters form at A>=11).
    std::vector<double> As = smoke ? std::vector<double>{14}    : std::vector<double>{11, 13, 14, 16, 18};

    std::ofstream f(outdir + "/relaxation_summary.csv");
    f << "T,A,seed,N_eq,org_eq,E_wave_eq,q_min,tau_relax,censored,mean_tau_kin\n";
    for (double T : Ts) for (double A : As) for (int s = 0; s < seeds; ++s) {
        unsigned sd = 0x5EED0u + static_cast<unsigned>(s);
        auto r = run_relaxation(L, T, A, sd);
        f << r.T << "," << r.A << "," << r.seed << "," << r.N_eq << "," << r.org_eq << ","
          << r.E_wave_eq << "," << r.q_min << "," << r.tau_relax << "," << (r.censored ? 1 : 0) << ","
          << r.mean_tau_kin << "\n";
        std::cout << "  T=" << T << " A=" << A << " seed=" << s
                  << " -> N_eq=" << r.N_eq << " org_eq=" << r.org_eq
                  << " q_min=" << r.q_min << " tau_relax=" << r.tau_relax
                  << (r.censored ? " [censored]" : "") << " tau_kin=" << r.mean_tau_kin << "\n";
    }
    std::cout << "\n  wrote " << outdir << "/relaxation_summary.csv\n";
    std::cout << "================================================================\n";
    std::cout << "  DONE (exploratory; analyze with scripts/exploration/explore_cluster_relaxation.py)\n";
    std::cout << "================================================================\n";
    return 0;
}

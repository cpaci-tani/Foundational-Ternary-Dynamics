/**
 * campaign_cluster_fission_fusion — Exp-A of the cluster-thermodynamics
 * EXPLORATORY pass (P2 fission/fusion asymmetry + P3 fusion-is-lossy).
 *
 * EXPLORATORY, NOT pre-registered. See .claude/plans/lazy-conjuring-marble.md.
 * Results are signal-detection only and are NOT evidence.
 *
 * Two arms (--arm=fusion|fission):
 *  - fusion : nucleate two clusters (amplitude signs = compatibility axis:
 *             (+,+) same manifested sign = compatible; (+,-) opposite =
 *             incompatible), then drive them together (forces+movement+v_rel)
 *             and classify merge / scatter / annihilate via genealogy. Records
 *             N_merged vs N1+N2 and org (P3 lossiness) + detuning (P2 gate).
 *  - fission: nucleate one cluster, optionally apply a quadrupole dent
 *             (delta>0 = driver; delta=0 = spontaneity control), classify
 *             Fission events + delta_fis = (Nchild-Nparent)/Nparent (P2
 *             conservativeness). Death-valley A in {10,11}.
 *
 * Usage: campaign_cluster_fission_fusion --arm fusion|fission
 *        [--L 32] [--seeds 8] [--output-dir DIR] [--smoke] [--no-forces]
 */
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/cluster_genealogy.h"
#include "ftd/cluster_observables.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace ftd;

// ---- exploratory run parameters (coarse; for-record pre-reg uses finer) ----
static int WARMUP = 40;
static int RUN    = 300;
static int STRIDE = 4;
static const int STABLE_THRESHOLD = 8;   // min size to count as a real cluster

// All connected (sign-grouped, Moore) manifested components >= min_size.
static std::vector<std::vector<int>> components(const RenderBridge& rb, int min_size) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<int> label(total, -1);
    std::vector<std::vector<int>> comps;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0 || label[i] != -1) continue;
        int8_t sign = vox[i].state;
        int cid = static_cast<int>(comps.size());
        comps.emplace_back();
        std::vector<int> q; q.push_back(static_cast<int>(i)); label[i] = cid;
        while (!q.empty()) {
            int idx = q.back(); q.pop_back();
            comps[cid].push_back(idx);
            for (int n : lat.neighbors_26(idx))
                if (label[n] == -1 && vox[n].state == sign) { label[n] = cid; q.push_back(n); }
        }
    }
    std::vector<std::vector<int>> out;
    for (auto& c : comps) if (static_cast<int>(c.size()) >= min_size) out.push_back(std::move(c));
    std::sort(out.begin(), out.end(), [](const std::vector<int>& a, const std::vector<int>& b) {
        return a.size() > b.size();
    });
    return out;
}

static void quiescent_config(RenderBridge& rb, double T, unsigned seed) {
    rb.force_cpu();
    rb.toggles.disable_all();           // also clears dual_substrate, movement, latency_field, forces
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.langevin_seed    = seed;
    rb.seed_rng(seed);
}

// ----------------------------- fusion arm -----------------------------------
struct FusionOutcome {
    double r0, v_rel, b, A1, A2; int s1, s2; unsigned seed;
    int N1, N2; double org1, org2, detuning;
    std::string outcome; int N_merged; double org_merged; int n_fusion; int final_alive;
};

static FusionOutcome run_fusion(int L, double r0, double v_rel, double b,
                                double A1, double A2, int s1, int s2,
                                double T, unsigned seed, bool no_forces) {
    RenderBridge rb(L);
    quiescent_config(rb, T, seed);

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    const int x1 = cx - static_cast<int>(std::round(r0 / 2));
    const int x2 = cx + static_cast<int>(std::round(r0 / 2));
    const int y1 = cy - static_cast<int>(std::round(b / 2));
    const int y2 = cy + static_cast<int>(std::round(b / 2));
    rb.inject_flux(x1, y1, cz, {A1 * s1 * K_GENESIS, 0.0, 0.0});
    rb.inject_flux(x2, y2, cz, {A2 * s2 * K_GENESIS, 0.0, 0.0});

    for (int t = 0; t < WARMUP; ++t) rb.tick();

    // pre-contact: two largest clusters
    auto pre = components(rb, 4);
    ClusterMeasure m1, m2;
    if (pre.size() >= 1) m1 = measure_cluster(rb, pre[0]);
    if (pre.size() >= 2) m2 = measure_cluster(rb, pre[1]);

    // turn on forces + movement; drive the two clusters together
    if (!no_forces) { rb.toggles.forces = true; rb.toggles.poisson_coulomb = true; rb.toggles.gravity = false; }
    rb.toggles.movement = true;
    {
        auto& vox = rb.voxels();
        const auto& lat = rb.lattice();
        for (int64_t i = 0; i < lat.total_sites(); ++i) {
            if (vox[i].state == 0) continue;
            Coord c = lat.coord(i);
            double vx = (c.x < cx) ? +v_rel / 2.0 : -v_rel / 2.0;
            vox[i].velocity = Vec3(vx, 0.0, 0.0);
        }
    }

    ClusterGenealogyTracker g;
    g.record(rb);                                 // establish the 2 pre-contact clusters
    for (int t = 1; t <= RUN; ++t) { rb.tick(); if (t % STRIDE == 0) g.record(rb); }

    auto finals = components(rb, STABLE_THRESHOLD);
    FusionOutcome o;
    o.r0 = r0; o.v_rel = v_rel; o.b = b; o.A1 = A1; o.A2 = A2; o.s1 = s1; o.s2 = s2; o.seed = seed;
    o.N1 = m1.size; o.N2 = m2.size; o.org1 = m1.org; o.org2 = m2.org;
    o.detuning = detuning_proxy(m1.flux_sum, m2.flux_sum);
    o.n_fusion = g.count(EventType::Fusion);
    o.final_alive = static_cast<int>(finals.size());
    o.N_merged = 0; o.org_merged = 0.0;
    if (o.n_fusion >= 1 && o.final_alive == 1) {
        o.outcome = "merge"; o.N_merged = static_cast<int>(finals[0].size());
        o.org_merged = measure_cluster(rb, finals[0]).org;
    } else if (o.final_alive == 0) {
        o.outcome = "annihilate";
    } else if (o.final_alive >= 2) {
        o.outcome = "scatter";
    } else {
        o.outcome = "other";
        if (!finals.empty()) { o.N_merged = static_cast<int>(finals[0].size()); o.org_merged = measure_cluster(rb, finals[0]).org; }
    }
    return o;
}

// ----------------------------- fission arm ----------------------------------
struct FissionRow {
    double A, delta; unsigned seed; int n_fission, n_death; double delta_fis_mean; bool driven;
};

static FissionRow run_fission(int L, double A, double delta, double T, unsigned seed) {
    RenderBridge rb(L);
    quiescent_config(rb, T, seed);
    const int cx = L / 2, cy = L / 2, cz = L / 2;
    rb.inject_flux(cx, cy, cz, {A * K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < WARMUP; ++t) rb.tick();
    if (delta > 0.0) {  // quadrupole dent (driver)
        rb.inject_flux_add(cx + 2, cy, cz, {+delta * K_GENESIS, 0.0, 0.0});
        rb.inject_flux_add(cx - 2, cy, cz, {-delta * K_GENESIS, 0.0, 0.0});
    }
    ClusterGenealogyTracker g;
    g.record(rb);
    for (int t = 1; t <= RUN; ++t) { rb.tick(); if (t % STRIDE == 0) g.record(rb); }
    auto fis = g.fissions();
    double sumdf = 0.0; int nf = 0;
    for (auto& e : fis)
        if (e.sum_parent_size > 0) { sumdf += double(e.sum_child_size - e.sum_parent_size) / e.sum_parent_size; ++nf; }
    FissionRow r;
    r.A = A; r.delta = delta; r.seed = seed; r.driven = (delta > 0.0);
    r.n_fission = g.count(EventType::Fission);
    r.n_death = g.count(EventType::Death);
    r.delta_fis_mean = (nf > 0) ? sumdf / nf : 0.0;
    return r;
}

int main(int argc, char** argv) {
    std::string arm = "fusion", outdir = ".";
    int L = 32, seeds = 8; bool smoke = false, no_forces = false;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--arm" && i + 1 < argc) arm = argv[++i];
        else if (a == "--output-dir" && i + 1 < argc) outdir = argv[++i];
        else if (a == "--L" && i + 1 < argc) L = std::atoi(argv[++i]);
        else if (a == "--seeds" && i + 1 < argc) seeds = std::atoi(argv[++i]);
        else if (a == "--smoke") smoke = true;
        else if (a == "--no-forces") no_forces = true;
    }
    if (smoke) { L = 16; seeds = 1; WARMUP = 20; RUN = 60; }

    std::cout << "================================================================\n";
    std::cout << "  EXPLORATORY campaign_cluster_fission_fusion  arm=" << arm
              << " L=" << L << " seeds=" << seeds << (smoke ? "  [SMOKE]" : "") << "\n";
    std::cout << "  (NOT pre-registered; signal-detection only.)\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed;

    const double T = 0.005;  // low-T quiescent

    if (arm == "fusion") {
        std::vector<double> r0s   = smoke ? std::vector<double>{8}    : std::vector<double>{8, 12};
        std::vector<double> vrels = smoke ? std::vector<double>{0.1}  : std::vector<double>{0.0, 0.1};
        std::vector<double> bs    = smoke ? std::vector<double>{0}    : std::vector<double>{0, 3};
        // amplitudes that form clusters in this config (feasibility 2026-05-30: A>=11)
        std::vector<std::pair<double,double>> amps = smoke ? std::vector<std::pair<double,double>>{{14,14}}
                                                           : std::vector<std::pair<double,double>>{{14,14},{14,16}};
        std::vector<std::pair<int,int>> signs = smoke ? std::vector<std::pair<int,int>>{{+1,+1}}
                                                      : std::vector<std::pair<int,int>>{{+1,+1},{+1,-1}};
        std::ofstream f(outdir + "/fusion_outcomes.csv");
        f << "r0,v_rel,b,A1,A2,s1,s2,seed,N1,N2,org1,org2,detuning,outcome,N_merged,org_merged,n_fusion,final_alive\n";
        int total = 0, merges = 0;
        for (double r0 : r0s) for (double v : vrels) for (double b : bs)
        for (auto& ap : amps) for (auto& sg : signs) for (int s = 0; s < seeds; ++s) {
            unsigned seed = 0xF15510u + static_cast<unsigned>(s);
            auto o = run_fusion(L, r0, v, b, ap.first, ap.second, sg.first, sg.second, T, seed, no_forces);
            f << o.r0 << "," << o.v_rel << "," << o.b << "," << o.A1 << "," << o.A2 << ","
              << o.s1 << "," << o.s2 << "," << o.seed << "," << o.N1 << "," << o.N2 << ","
              << o.org1 << "," << o.org2 << "," << o.detuning << "," << o.outcome << ","
              << o.N_merged << "," << o.org_merged << "," << o.n_fusion << "," << o.final_alive << "\n";
            ++total; if (o.outcome == "merge") ++merges;
            std::cout << "  r0=" << r0 << " v=" << v << " b=" << b << " A=(" << ap.first << "," << ap.second
                      << ") sign=(" << sg.first << "," << sg.second << ") seed=" << s
                      << " -> " << o.outcome << " (N1+N2=" << o.N1 + o.N2 << " N_merged=" << o.N_merged << ")\n";
        }
        std::cout << "\n  fusion runs=" << total << " merges=" << merges << "\n";
        std::cout << "  wrote " << outdir << "/fusion_outcomes.csv\n";
    } else if (arm == "fission") {
        // death-valley/large amplitudes that form a fissionable cluster (A>=11 forms clusters)
        std::vector<double> As     = smoke ? std::vector<double>{14}  : std::vector<double>{14, 16, 18};
        std::vector<double> deltas = smoke ? std::vector<double>{0}   : std::vector<double>{0.0, 0.5};
        std::ofstream f(outdir + "/fission_events.csv");
        f << "A,delta,seed,driven,n_fission,n_death,delta_fis_mean\n";
        for (double A : As) for (double d : deltas) for (int s = 0; s < seeds; ++s) {
            unsigned seed = 0xF15510u + static_cast<unsigned>(s);
            auto r = run_fission(L, A, d, T, seed);
            f << r.A << "," << r.delta << "," << r.seed << "," << (r.driven ? 1 : 0) << ","
              << r.n_fission << "," << r.n_death << "," << r.delta_fis_mean << "\n";
            std::cout << "  A=" << A << " delta=" << d << " seed=" << s
                      << " -> n_fission=" << r.n_fission << " n_death=" << r.n_death
                      << " delta_fis=" << r.delta_fis_mean << "\n";
        }
        std::cout << "\n  wrote " << outdir << "/fission_events.csv\n";
    } else {
        std::cerr << "unknown --arm " << arm << " (use fusion|fission)\n";
        return 2;
    }
    std::cout << "================================================================\n";
    std::cout << "  DONE (exploratory; analyze with scripts/exploration/explore_cluster_fission_fusion.py)\n";
    std::cout << "================================================================\n";
    
    // Cleanup artifacts if run in the default working directory (e.g. by CTest)
    if (outdir == ".") {
        std::remove("fusion_outcomes.csv");
        std::remove("fission_events.csv");
    }
    return 0;
}

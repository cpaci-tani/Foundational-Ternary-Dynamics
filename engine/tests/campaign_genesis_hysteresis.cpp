/**
 * @file campaign_genesis_hysteresis.cpp
 * @brief First-order confirmation for the FTD genesis transition: HYSTERESIS.
 *
 * Companion to campaign_genesis_criticality (the pre-registered FSS order test,
 * PREREG_GENESIS_CRITICALITY_v1, which trended first-order). The gold-standard
 * proof of a FIRST-ORDER transition is a HYSTERESIS LOOP: the heating branch
 * (start from void s=0, ramp Langevin T UP) jumps to the active phase at T_up,
 * but the cooling branch (carry the manifested state, ramp T DOWN) stays active
 * until a LOWER T_down < T_up before collapsing. A 2nd-order/continuous
 * transition has NO loop (the branches coincide). The loop area / (T_up - T_down)
 * gap is the discontinuity signature; pairs with the negative Binder cumulant
 * (U4 < 0 = phase coexistence) already seen in the criticality scout.
 *
 * ONE persistent bridge, state carried across the whole T-ramp (this is the
 * point -- re-initializing per T, as the FSS campaign does, only ever samples
 * the heating branch). Canonical stack, CPU, NO injection (pure thermal drive):
 *   wave_propagation + gauss_projection + genesis + coupling + langevin.
 *
 * Output (one row per T-step): branch(0=up,1=down),step,T,m,manifested
 *
 * Usage:
 *   campaign_genesis_hysteresis --L=24 --Tlo=0.04 --Thi=0.20 --steps=24 \
 *       --dwell=300 --equil=600 --cpu --output-dir=PATH --tag=hyst
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
int manifested_total(const ftd::RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    int total = 0;
    for (const auto& v : voxels) if (v.state != 0) ++total;
    return total;
}
} // namespace

int main(int argc, char** argv) {
    int L = 24;
    double Tlo = 0.04, Thi = 0.20;
    int steps = 24;
    int dwell = 300;
    int equil = 600;
    double gamma = 0.02;
    bool force_cpu = false;
    std::uint32_t seed = 0x6E157000u;
    std::string tag = "hyst";
    std::string output_dir = "engine/results/genesis_hysteresis_default/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--Tlo=", 0) == 0)        Tlo = std::atof(a.c_str() + 6);
        else if (a.rfind("--Thi=", 0) == 0)        Thi = std::atof(a.c_str() + 6);
        else if (a.rfind("--steps=", 0) == 0)      steps = std::atoi(a.c_str() + 8);
        else if (a.rfind("--dwell=", 0) == 0)      dwell = std::atoi(a.c_str() + 8);
        else if (a.rfind("--equil=", 0) == 0)      equil = std::atoi(a.c_str() + 8);
        else if (a.rfind("--gamma=", 0) == 0)      gamma = std::atof(a.c_str() + 8);
        else if (a == "--cpu")                     force_cpu = true;
        else if (a.rfind("--seed=", 0) == 0)       seed = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const double N = static_cast<double>(L) * L * L;

    ftd::RenderBridge rb(L);
    if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(150); }
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.coupling         = true;
    rb.toggles.dual_substrate   = false;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_gamma   = gamma;
    rb.toggles.langevin_T       = Tlo;
    rb.seed_rng(seed);

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("hyst_" + tag + "_L" + std::to_string(L) + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "branch,step,T,m,manifested\n");

    std::printf("genesis_hysteresis: L=%d Tlo=%.4f Thi=%.4f steps=%d dwell=%d equil=%d gamma=%.4f backend=%s\n",
                L, Tlo, Thi, steps, dwell, equil, gamma, force_cpu ? "cpu" : "default");
    std::fflush(stdout);

    // Equilibrate at Tlo, starting from void (absorbing phase).
    for (int t = 0; t < equil; ++t) rb.tick();

    auto run_branch = [&](int branch) {
        for (int s = 0; s <= steps; ++s) {
            // up branch: Tlo -> Thi; down branch: Thi -> Tlo (state carried over).
            const double frac = static_cast<double>(s) / steps;
            const double T = (branch == 0) ? (Tlo + frac * (Thi - Tlo))
                                           : (Thi - frac * (Thi - Tlo));
            rb.toggles.langevin_T = T;
            double m_acc = 0.0; int m_n = 0;
            for (int t = 0; t < dwell; ++t) {
                rb.tick();
                if (t >= 3 * dwell / 4) { m_acc += manifested_total(rb) / N; ++m_n; }  // tail avg
            }
            const double m = m_n ? m_acc / m_n : manifested_total(rb) / N;
            std::fprintf(f, "%d,%d,%.6f,%.8f,%d\n", branch, s, T, m, manifested_total(rb));
            std::printf("  %s step=%2d T=%.4f m=%.5f\n", branch == 0 ? "UP  " : "DOWN", s, T, m);
            std::fflush(stdout);
        }
    };

    run_branch(0);   // heating: void -> active
    run_branch(1);   // cooling: active -> void (state carried)

    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

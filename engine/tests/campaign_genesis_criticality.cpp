/**
 * @file campaign_genesis_criticality.cpp
 * @brief Order of the FTD genesis/manifestation transition (RG-spectrum probe).
 *
 * NARROW TARGET: is the genesis threshold a CRITICAL POINT (2nd-order, with a
 * scaling spectrum -> genesis is a RELEVANT operator, the cluster-mass ladder
 * has RG content) or FIRST-ORDER/trivial (no RG-derived spectrum)?
 *
 * Genesis is an ABSORBING-STATE transition: the void (s=0) is the quiescent
 * phase, manifestation (s=+/-1) is "activity". We drive it purely by Langevin
 * temperature T (no injection) and measure the steady-state order parameter
 *     m = N_manifested / L^3
 * and its full fluctuation distribution, across lattice sizes L. Finite-size
 * scaling of m(T), the susceptibility chi = L^3 * Var(m), the Binder cumulant
 * U4 = 1 - <m^4>/(3<m^2>^2), and the histogram P(m) decide the ORDER:
 *   - continuous m(T), chi peak growing with L, U4 CROSSING, unimodal P(m_c)
 *       => GENESIS-CRITICAL (relevant operator, scaling spectrum)
 *   - discontinuous m(T), BIMODAL P(m) at coexistence, no U4 crossing (dip),
 *     chi ~ L^D (not L^{gamma/nu})
 *       => GENESIS-FIRST-ORDER (no RG-derived spectrum; energy-budget ladder)
 *
 * Canonical stack (FTD-0261/0267): wave_propagation + gauss_projection +
 * genesis + langevin [+ coupling], CPU (Langevin is the CPU OU thermostat).
 * NO point injection -- pure thermal manifestation (the FTD-0107 ic2 regime).
 *
 * Output (one row per sampled tick):
 *   L,T,seed,tick,manifested,m
 *
 * Usage:
 *   campaign_genesis_criticality --L=24 --Ts=0.02,0.04,0.06,0.08,0.10 \
 *       --seeds=8 --equil=2000 --sample=1000 --cpu --output-dir=PATH --tag=run
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

// Total manifested voxels (state != 0).
int manifested_total(const ftd::RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    int total = 0;
    for (const auto& v : voxels) if (v.state != 0) ++total;
    return total;
}

std::vector<double> parse_list(const std::string& s) {
    std::vector<double> out;
    std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i);
        if (j == std::string::npos) j = s.size();
        out.push_back(std::atof(s.substr(i, j - i).c_str()));
        i = j + 1;
    }
    return out;
}

} // namespace

int main(int argc, char** argv) {
    int L = 24;
    std::string Ts_str = "0.02,0.04,0.06,0.08,0.10,0.12";
    int seeds = 8;
    int equil = 2000;
    int sample = 1000;
    double gamma = 0.02;
    bool coupling_on = true;
    bool force_cpu = false;
    std::uint32_t seed_base = 0x6E150000u;
    std::string tag = "crit";
    std::string output_dir = "engine/results/genesis_criticality_default/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--Ts=", 0) == 0)         Ts_str = a.substr(5);
        else if (a.rfind("--seeds=", 0) == 0)      seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--equil=", 0) == 0)      equil = std::atoi(a.c_str() + 8);
        else if (a.rfind("--sample=", 0) == 0)     sample = std::atoi(a.c_str() + 9);
        else if (a.rfind("--gamma=", 0) == 0)      gamma = std::atof(a.c_str() + 8);
        else if (a == "--coupling=off")            coupling_on = false;
        else if (a == "--coupling=on")             coupling_on = true;
        else if (a == "--cpu")                     force_cpu = true;
        else if (a.rfind("--seed-base=", 0) == 0)  seed_base = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 12, nullptr, 0));
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const std::vector<double> Ts = parse_list(Ts_str);
    const double N = static_cast<double>(L) * L * L;

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("crit_" + tag + "_L" + std::to_string(L) + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "L,T,seed,tick,manifested,m\n");

    std::printf("genesis_criticality: L=%d Ts=%s seeds=%d equil=%d sample=%d gamma=%.4f coupling=%s backend=%s\n",
                L, Ts_str.c_str(), seeds, equil, sample, gamma,
                coupling_on ? "on" : "off", force_cpu ? "cpu" : "default");
    std::fflush(stdout);

    for (double T : Ts) {
        for (int s = 0; s < seeds; ++s) {
            ftd::RenderBridge rb(L);
            if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(150); }
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.genesis          = true;
            rb.toggles.coupling         = coupling_on;
            rb.toggles.dual_substrate   = false;
            rb.toggles.langevin         = true;
            rb.toggles.langevin_T       = T;
            rb.toggles.langevin_gamma   = gamma;
            rb.seed_rng(seed_base + static_cast<std::uint32_t>(s)
                        + static_cast<std::uint32_t>(T * 1e6));  // decorrelate T-rows
            // NO injection: pure thermal manifestation (absorbing-state drive).

            for (int t = 0; t < equil; ++t) rb.tick();          // equilibrate
            for (int t = 0; t < sample; ++t) {
                rb.tick();
                const int man = manifested_total(rb);
                std::fprintf(f, "%d,%.6f,%d,%d,%d,%.8f\n",
                             L, T, s, t, man, man / N);
            }
            std::printf("  T=%.4f seed=%d  done (last m=%.5f)\n",
                        T, s, manifested_total(rb) / N);
            std::fflush(stdout);
        }
    }

    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

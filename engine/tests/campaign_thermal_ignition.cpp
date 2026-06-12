/**
 * @file campaign_thermal_ignition.cpp
 * @brief FTD-0274 scout — min/max temperature + ignition/detonation map of the lattice.
 *
 * Establishes the thermodynamic axis of the FTD substrate and the explosive
 * (first-order, FTD-0272) character of the genesis transition:
 *
 *   THERMAL mode (langevin bath, NO injection): heat the void by ramping the bath
 *     temperature langevin_T, then cool back. Per T measure manifestation fraction
 *     m = N/L³, the KINETIC temperature T_kin = ⟨½|wave_vel|²⟩/(3/2) (equipartition,
 *     k_B≡1), and a stability flag. Yields:
 *       T_up   — heating condensation onset (the void's metastability limit / spinodal)
 *       T_down — cooling onset (hysteresis ⇒ first-order; FTD-0272: self-sustaining
 *                condensate pinned to T=0)
 *       T_max  — where T_kin stops tracking T_set / the leapfrog destabilizes: the
 *                CFL/causality ceiling (thermal velocity → max signal speed c=1/√3).
 *                The discreteness-imposed MAXIMUM temperature (a continuum has none).
 *
 *   IGNITION mode (langevin OFF, deterministic): at each L, sweep the injection
 *     amplitude A and classify the settled state BOUNDED (controlled nucleation, a
 *     "flame") vs FLOODED (the autocatalytic condensate runs away — a "detonation").
 *     Yields A*(L), the ignition threshold, testing whether a tighter (smaller) box
 *     ignites at lower drive (energy confinement: radiation can't vent).
 *
 * All temperatures are reported in lattice units and in units of c² (c=1/√3 ⇒
 * c²=1/3) and K_GENESIS². Golden-neutral / read-only.
 *
 * Output: thermal_ignition_<tag>.csv
 *   mode,L,phase,drive,m,T_kin,wave_e,total_e,stable,outcome
 *
 * Usage:
 *   campaign_thermal_ignition --mode=all --Ls=24,32 --Tmax=0.5 --dT=0.01 \
 *       --As=2,6,10,14,18,22,28,36,46,60 --settle=400 --seeds=2 --tag=scout
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/voxel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

template <typename T>
std::vector<T> parse_list(const std::string& s) {
    std::vector<T> out; std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(static_cast<T>(std::atof(s.substr(i, j - i).c_str()))); i = j + 1;
    }
    return out;
}

void base_toggles(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.set_sor_iterations(150);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.dual_substrate   = false;
}

struct Probe { double m, T_kin, wave_e, total_e; bool stable; };

Probe probe(ftd::RenderBridge& rb, int L) {
    const ftd::EnergyAudit ea = rb.energy_audit();
    const double Nvox = (double)L * L * L;
    Probe p;
    p.m = ea.manifested_count / Nvox;
    // equipartition: ⟨½|wave_vel|²⟩ = (3/2)·T  ⇒  T_kin = wave_energy / ((3/2)·Nvox)
    p.T_kin = ea.wave_energy / (1.5 * Nvox);
    p.wave_e = ea.wave_energy;
    p.total_e = ea.total_energy;
    p.stable = std::isfinite(ea.total_energy) && ea.total_energy < 1e8;
    return p;
}

} // namespace

int main(int argc, char** argv) {
    std::string mode = "all", Ls_str = "24,32";
    std::string As_str = "2,6,10,14,18,22,28,36,46,60";
    double Tmax = 0.5, dT = 0.01, flood_frac = 0.25;
    int settle = 400, seeds = 2;
    std::string tag = "scout";
    std::string output_dir = "engine/results/thermal_ignition/";
    std::uint32_t seed_base = 0x73E12000u;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--mode=", 0) == 0)        mode = a.substr(7);
        else if (a.rfind("--Ls=", 0) == 0)          Ls_str = a.substr(5);
        else if (a.rfind("--As=", 0) == 0)          As_str = a.substr(5);
        else if (a.rfind("--Tmax=", 0) == 0)        Tmax = std::atof(a.c_str() + 7);
        else if (a.rfind("--dT=", 0) == 0)          dT = std::atof(a.c_str() + 5);
        else if (a.rfind("--settle=", 0) == 0)      settle = std::atoi(a.c_str() + 9);
        else if (a.rfind("--seeds=", 0) == 0)       seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--flood-frac=", 0) == 0)  flood_frac = std::atof(a.c_str() + 13);
        else if (a.rfind("--tag=", 0) == 0)         tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0)  output_dir = a.substr(13);
    }

    const std::vector<int> Ls = parse_list<int>(Ls_str);
    const std::vector<double> As = parse_list<double>(As_str);
    const double C2 = 1.0 / 3.0;                          // c² (c = 1/√3)
    const double KG2 = ftd::K_GENESIS * ftd::K_GENESIS;

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("thermal_ignition_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "mode,L,phase,drive,m,T_kin,wave_e,total_e,stable,outcome\n");

    std::printf("thermal_ignition: mode=%s Ls=%s Tmax=%.3f dT=%.3f settle=%d  "
                "c2=%.4f K_GENESIS2=%.4f\n", mode.c_str(), Ls_str.c_str(), Tmax, dT,
                settle, C2, KG2);
    std::fflush(stdout);

    const bool do_thermal  = (mode == "all" || mode == "thermal");
    const bool do_ignition = (mode == "all" || mode == "ignition");

    // ---- THERMAL: heat-then-cool hysteresis + T_kin + T_max ----
    if (do_thermal) {
        for (int L : Ls) {
            const long flood_thresh = (long)(flood_frac * (double)L * L * L);
            (void)flood_thresh;
            ftd::RenderBridge rb(L);
            base_toggles(rb);
            rb.toggles.langevin       = true;
            rb.toggles.langevin_gamma = 0.02;
            rb.seed_rng(seed_base);
            double T_up = -1, T_max = -1;
            // HEATING from void
            for (double T = 0.0; T <= Tmax + 1e-9; T += dT) {
                rb.toggles.langevin_T = T;
                for (int t = 0; t < settle; ++t) rb.tick();
                const Probe p = probe(rb, L);
                if (T_up < 0 && p.m > 0.5) T_up = T;
                if (T_max < 0 && !p.stable) T_max = T;
                const char* oc = !p.stable ? "UNSTABLE" : (p.m > 0.5 ? "CONDENSED" : "VACUUM");
                std::fprintf(f, "thermal,%d,heat,%.4f,%.5f,%.6f,%.6g,%.6g,%d,%s\n",
                             L, T, p.m, p.T_kin, p.wave_e, p.total_e, p.stable ? 1 : 0, oc);
                std::printf("  [heat] L=%d T=%.3f  m=%.3f  T_kin=%.4f  %s\n",
                            L, T, p.m, p.T_kin, oc);
                std::fflush(stdout);
                if (!p.stable) break;          // past the CFL ceiling
            }
            // COOLING back down (hysteresis)
            double T_down = -1;
            for (double T = Tmax; T >= -1e-9; T -= dT) {
                rb.toggles.langevin_T = (T < 0 ? 0 : T);
                for (int t = 0; t < settle; ++t) rb.tick();
                const Probe p = probe(rb, L);
                if (p.m < 0.5 && T_down < 0) T_down = T;   // first time it falls back
                const char* oc = (p.m > 0.5 ? "CONDENSED" : "VACUUM");
                std::fprintf(f, "thermal,%d,cool,%.4f,%.5f,%.6f,%.6g,%.6g,%d,%s\n",
                             L, T, p.m, p.T_kin, p.wave_e, p.total_e, p.stable ? 1 : 0, oc);
                std::fflush(stdout);
            }
            std::printf("  L=%d  T_up=%.3f (%.2f c²)  T_down=%.3f  T_max=%.3f (%.2f c²)\n",
                        L, T_up, T_up / C2, T_down, T_max, T_max / C2);
            std::fflush(stdout);
        }
    }

    // ---- IGNITION: amplitude sweep, bounded vs detonation, vs L ----
    if (do_ignition) {
        for (int L : Ls) {
            const long flood_thresh = (long)(flood_frac * (double)L * L * L);
            const int c = L / 2;
            double A_star = -1;
            for (double A : As) {
                int floods = 0; double m_last = 0.0;
                for (int s = 0; s < seeds; ++s) {
                    ftd::RenderBridge rb(L);
                    base_toggles(rb);                       // langevin OFF (deterministic)
                    rb.seed_rng(seed_base + (std::uint32_t)s * 2654435761u);
                    rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0, 0});
                    for (int t = 0; t < settle; ++t) rb.tick();
                    const Probe p = probe(rb, L);
                    m_last = p.m;
                    const long n = (long)std::lround(p.m * (double)L * L * L);
                    if (n > flood_thresh) ++floods;
                }
                const bool flooded = floods > seeds / 2;
                const char* oc = flooded ? "DETONATION"
                               : (m_last < 1e-9 ? "EVAPORATED" : "BOUNDED");
                if (flooded && A_star < 0) A_star = A;
                std::fprintf(f, "ignition,%d,inject,%.4f,%.5f,0,0,0,1,%s\n", L, A, m_last, oc);
                std::printf("  [ignite] L=%d A=%.0f  floods=%d/%d  %s\n",
                            L, A, floods, seeds, oc);
                std::fflush(stdout);
            }
            std::printf("  L=%d  ignition threshold A*=%.0f\n", L, A_star);
            std::fflush(stdout);
        }
    }

    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

/**
 * @file campaign_proton_stability.cpp
 * @brief FTD-0301: is the proton (uud triad) dynamically stable, or does it
 *        DECAY (evaporate / transmute) under FTD's native dynamics?
 *
 * VERIFIED SOURCE FACTS (the audit premise):
 *  - triad_binding_cpu locks ONLY three SAME-STATE particles
 *    (transmutation_phases.cpp:148,153: `vb.state != va.state` / `vc.state != va.state` -> skip).
 *  - the proton is uud = (+1, +1, -1) — MIXED sign (constructors_bulk_matter.cpp:53-61).
 *  => the triad lock CANNOT fire on a real proton, so the "locked triad exempt
 *     from evaporation" stability argument (proof_complete_sm.py:462-463) NEVER
 *     applies to it. The proton is, from t=0, an UNLOCKED configuration subject to
 *     evaporation AND weak transmutation.
 *  - weak_transmutation_cpu flips state -> -state on ANY manifested voxel once
 *    stress > WEAK_THRESHOLD=K_GENESIS (transmutation_phases.cpp:29-33), locked
 *    or not — the B-violation handle FTD's own baryogenesis sector invokes
 *    (CHECKLIST_PHYSICS.md:534). Cold seeding leaves stress below threshold, so
 *    the HEATED arms below drive it across: `--heat=inject` (a deterministic flux
 *    pulse at the d-quark, the campaign_weak_transmutation recipe) and
 *    `--heat=langevin` (the OU thermostat, the requested physical heating).
 *
 * This campaign seeds a uud proton and measures whether its three quark cores
 * persist; it contrasts with a (LOCKABLE) uuu same-sign triple to show the lock
 * protects an artificial object, not the proton. The heated arms then test the
 * [THEOREM]'s strongest sub-claim — "weak transmutation flips polarity but
 * PRESERVES triad structure" — by firing weak on a proton core and recording
 * whether the proton (multiset {+1,+1,-1}, charge Σs=1) is preserved or
 * transmuted (a flipped quark => different multiset + different Σs => the proton
 * is gone and Σs is not conserved).
 *
 * Observable per tick: the 3 core states, manifested-core count, total charge
 * Σs, triad locked? — and tau_persist = first tick the core-state multiset
 * changes from the seed (evaporation: a core -> 0; transmutation: a core flips
 * sign).
 *
 * OBSERVATION-ONLY: new TU, no edits to any phase_*.cpp/kernel/constant.
 *
 * Output (engine/results/proton_stability/):
 *   proton_stability_<tag>.csv         — one row per (species,weak,seed)
 *   proton_stability_traj_<tag>.csv    — per-tick trajectory (with --traj)
 *
 * Usage:
 *   campaign_proton_stability --L=32 --ticks=2000 --species=proton,samesign \
 *       --weak=on --triad=on --movement=off --radius=2 --seeds=8 --cpu --tag=v1
 *   campaign_proton_stability --species=proton --weak=on --heat=inject \
 *       --warmup=200 --heat-dwell=20 --heat-amp=10 --seeds=8 --traj --tag=heat_inj
 *   campaign_proton_stability --species=proton --weak=on --heat=langevin \
 *       --heat-T=0.03 --seeds=8 --traj --tag=heat_lan
 */

#include "ftd/constants.h"
#include "ftd/constructors.h"
#include "ftd/render_bridge.h"
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

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> out; std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(s.substr(i, j - i)); i = j + 1;
    }
    return out;
}

// the three equilateral-triangle vertices used by ctor::proton (radius r)
struct Core { int x, y, z; };
std::vector<Core> triad_vertices(int C, int r) {
    std::vector<Core> v;
    const double ang[3] = {0.0, 2.0 * ftd::PI / 3.0, 4.0 * ftd::PI / 3.0};
    for (double a : ang)
        v.push_back({C + (int)std::lround(r * std::cos(a)),
                     C + (int)std::lround(r * std::sin(a)), C});
    return v;
}

} // namespace

int main(int argc, char** argv) {
    int L = 32, ticks = 2000, seeds = 4, radius = 2;
    std::string species_str = "proton,samesign", weak_str = "on,off";
    std::string triad_str = "on", movement_str = "off", genesis_str = "on", dual_str = "on";
    std::string heat_str = "none";          // none | inject | langevin
    double heat_amp = 10.0, heat_T = 0.03;  // inject pulse (×K_B) / langevin temperature
    int heat_dwell = 20, warmup = 200;      // inject pulse window (ticks after warmup)
    bool force_cpu = false, traj = false;
    std::string tag = "v1", output_dir = "engine/results/proton_stability/";
    std::uint32_t seed_base = 0x9701D000u;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--ticks=", 0) == 0)      ticks = std::atoi(a.c_str() + 8);
        else if (a.rfind("--species=", 0) == 0)    species_str = a.substr(10);
        else if (a.rfind("--weak=", 0) == 0)       weak_str = a.substr(7);
        else if (a.rfind("--triad=", 0) == 0)      triad_str = a.substr(8);
        else if (a.rfind("--movement=", 0) == 0)   movement_str = a.substr(11);
        else if (a.rfind("--genesis=", 0) == 0)    genesis_str = a.substr(10);
        else if (a.rfind("--dual=", 0) == 0)       dual_str = a.substr(7);
        else if (a.rfind("--heat=", 0) == 0)       heat_str = a.substr(7);
        else if (a.rfind("--heat-amp=", 0) == 0)   heat_amp = std::atof(a.c_str() + 11);
        else if (a.rfind("--heat-T=", 0) == 0)     heat_T = std::atof(a.c_str() + 9);
        else if (a.rfind("--heat-dwell=", 0) == 0) heat_dwell = std::atoi(a.c_str() + 13);
        else if (a.rfind("--warmup=", 0) == 0)     warmup = std::atoi(a.c_str() + 9);
        else if (a.rfind("--radius=", 0) == 0)     radius = std::atoi(a.c_str() + 9);
        else if (a.rfind("--seeds=", 0) == 0)      seeds = std::atoi(a.c_str() + 8);
        else if (a == "--cpu")                     force_cpu = true;
        else if (a == "--traj")                    traj = true;
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const auto species = split(species_str);
    const auto weaks   = split(weak_str);
    const bool triad_on    = (triad_str == "on");
    const bool movement_on = (movement_str == "on");
    const bool genesis_on  = (genesis_str != "off");
    const bool dual_on     = (dual_str != "off");
    const bool heat_inject  = (heat_str == "inject");
    const bool heat_langevin = (heat_str == "langevin");

    fs::create_directories(output_dir);
    std::FILE* fs_ = std::fopen((fs::path(output_dir) / ("proton_stability_" + tag + ".csv")).string().c_str(), "w");
    std::FILE* ft_ = traj ? std::fopen((fs::path(output_dir) / ("proton_stability_traj_" + tag + ".csv")).string().c_str(), "w") : nullptr;
    if (!fs_) { std::fprintf(stderr, "cannot open output\n"); return 1; }
    std::fprintf(fs_, "species,weak,triad,movement,genesis,dual,heat,heat_T,radius,L,ticks,seed,"
                      "tau_persist,fail_mode,c0_seed,c1_seed,c2_seed,"
                      "c0_final,c1_final,c2_final,q_init,q_final,manifested_final,locked_final\n");
    if (ft_) std::fprintf(ft_, "species,weak,heat,seed,tick,c0,c1,c2,n_manifested,charge_sum,n_locked\n");
    std::fflush(fs_);

    std::printf("proton_stability: L=%d ticks=%d species=%s weak=%s triad=%s movement=%s genesis=%s heat=%s seeds=%d backend=%s\n",
                L, ticks, species_str.c_str(), weak_str.c_str(), triad_str.c_str(),
                movement_str.c_str(), genesis_str.c_str(), heat_str.c_str(), seeds, force_cpu ? "cpu" : "default");
    if (heat_inject)
        std::printf("  heat=inject: pulse ±%.3g (=%.2g·K_B) at d-quark, ticks (%d,%d]\n",
                    heat_amp * ftd::K_B, heat_amp, warmup, warmup + heat_dwell);
    if (heat_langevin)
        std::printf("  heat=langevin: T=%.4g gamma=0.02 (WEAK_THRESHOLD=%.4g)\n", heat_T, ftd::WEAK_THRESHOLD);
    std::fflush(stdout);

    const int C = L / 2;
    const auto verts = triad_vertices(C, radius);
    auto vidx = [&](const Core& c) { return c.x * L * L + c.y * L + c.z; };
    const int ci[3] = { vidx(verts[0]), vidx(verts[1]), vidx(verts[2]) };

    for (const std::string& sp : species) {
        const bool is_proton = (sp == "proton");
        // seed core states: proton uud = (+1,+1,-1); samesign uuu = (+1,+1,+1)
        const int8_t seed_s[3] = { +1, +1, (int8_t)(is_proton ? -1 : +1) };
        for (const std::string& w : weaks) {
            const bool weak_on = (w == "on");
            for (int s = 0; s < seeds; ++s) {
                ftd::RenderBridge rb(L);
                if (force_cpu) { rb.force_cpu(); rb.set_sor_iterations(60); }
                rb.toggles.disable_all();
                rb.toggles.wave_propagation = true;
                rb.toggles.coupling         = true;
                rb.toggles.damping          = true;
                rb.toggles.gauss_projection = true;
                rb.toggles.dual_substrate   = dual_on;   // OU thermostat only reaches weak's stress with dual=off
                rb.toggles.genesis          = genesis_on;   // the evaporation channel (off => isolate weak)
                rb.toggles.color_forces     = true;   // required by triad_binding
                rb.toggles.triad_binding    = triad_on;
                rb.toggles.weak_transmutation = weak_on;
                rb.toggles.forces           = movement_on;  // forces only matter with movement
                rb.toggles.movement         = movement_on;
                // seed_rng sets toggles.langevin_seed = seed (render_bridge.cpp:256),
                // which keys BOTH the OU noise AND the weak-flip RNG — so each seed
                // is a distinct, bit-reproducible stream. Set langevin physics AFTER.
                rb.seed_rng(seed_base + (std::uint32_t)s * 2654435761u);
                if (heat_langevin) {
                    rb.toggles.langevin       = true;
                    rb.toggles.langevin_T     = heat_T;
                    rb.toggles.langevin_gamma = 0.02;
                }

                if (is_proton) {
                    ftd::ctor::proton(rb, ftd::Coord{C, C, C}, radius);
                } else {  // uuu: same positions, all +1
                    ftd::ctor::quark(rb, ftd::Coord{verts[0].x, verts[0].y, verts[0].z}, +1, 1, +1);
                    ftd::ctor::quark(rb, ftd::Coord{verts[1].x, verts[1].y, verts[1].z}, +1, 2, -1);
                    ftd::ctor::quark(rb, ftd::Coord{verts[2].x, verts[2].y, verts[2].z}, +1, 3, +1);
                }
                const long long q_init = rb.charge_sum();

                int tau = -1; std::string fail = "intact";
                for (int t = 1; t <= ticks; ++t) {
                    // heated arm — inject route: a sustained flux pulse at the
                    // d-quark core (verts[2]) over ticks (warmup, warmup+dwell],
                    // matching the campaign_weak_transmutation recipe.
                    if (heat_inject && t > warmup && t <= warmup + heat_dwell) {
                        const double A = heat_amp * ftd::K_B;
                        const Core& d = verts[2];
                        rb.inject_flux(d.x + 1, d.y,     d.z,     { A, 0, 0});
                        rb.inject_flux(d.x - 1, d.y,     d.z,     {-A, 0, 0});
                        rb.inject_flux(d.x,     d.y + 1, d.z,     { 0, A, 0});
                        rb.inject_flux(d.x,     d.y - 1, d.z,     { 0,-A, 0});
                    }
                    rb.run(1);
                    const auto& v = rb.voxels();
                    int8_t cs[3]; int nman = 0, nlock = 0;
                    for (int k = 0; k < 3; ++k) {
                        cs[k] = v[ci[k]].state;
                        if (cs[k] != 0) ++nman;
                        if (v[ci[k]].locked) ++nlock;
                    }
                    if (ft_ && (t % 25 == 0 || t <= 5 ||
                                (heat_inject && t >= warmup && t <= warmup + heat_dwell + 10))) {
                        std::fprintf(ft_, "%s,%s,%s,%d,%d,%d,%d,%d,%d,%lld,%d\n",
                                     sp.c_str(), w.c_str(), heat_str.c_str(), s, t,
                                     cs[0], cs[1], cs[2], nman, rb.charge_sum(), nlock);
                    }
                    if (tau < 0) {
                        // multiset compare to seed (order-independent over 3 cores)
                        int seed_sum = seed_s[0] + seed_s[1] + seed_s[2];
                        int cur_sum  = cs[0] + cs[1] + cs[2];
                        bool evaporated  = (nman < 3);
                        bool transmuted  = (!evaporated && cur_sum != seed_sum);
                        if (evaporated || transmuted) {
                            tau = t;
                            fail = evaporated ? (transmuted ? "evap+transmute" : "evaporation")
                                              : "transmutation";
                        }
                    }
                }
                const auto& vf = rb.voxels();
                int8_t cf[3]; int nman = 0, nlock = 0;
                for (int k = 0; k < 3; ++k) { cf[k] = vf[ci[k]].state; if (cf[k]!=0)++nman; if (vf[ci[k]].locked)++nlock; }
                const long long q_final = rb.charge_sum();

                std::fprintf(fs_, "%s,%s,%d,%d,%d,%d,%s,%.4g,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%lld,%lld,%d,%d\n",
                             sp.c_str(), w.c_str(), triad_on, movement_on, genesis_on, dual_on,
                             heat_str.c_str(), (heat_langevin ? heat_T : 0.0), radius, L, ticks, s,
                             tau, fail.c_str(), seed_s[0], seed_s[1], seed_s[2],
                             cf[0], cf[1], cf[2], q_init, q_final, nman, nlock);
                std::fflush(fs_);
                std::printf("  %-8s weak=%-3s heat=%-8s seed=%d  tau_persist=%-6d %-14s  "
                            "cores %d%d%d -> %d%d%d  Q %lld->%lld  manif=%d lock=%d\n",
                            sp.c_str(), w.c_str(), heat_str.c_str(), s, tau, fail.c_str(),
                            seed_s[0], seed_s[1], seed_s[2], cf[0], cf[1], cf[2],
                            q_init, q_final, nman, nlock);
                std::fflush(stdout);
            }
        }
    }
    std::fclose(fs_); if (ft_) std::fclose(ft_);
    std::printf("wrote proton_stability_%s.csv\n", tag.c_str());
    return 0;
}

/**
 * @file campaign_genesis_geometry.cpp
 * @brief FTD-0110 nonlinear bridge: per-fired-voxel FIRING GEOMETRY in the engine.
 *
 * Sibling of campaign_genesis_trajectory.cpp (FTD-0267). Where that runner logs
 * per-tick aggregate counts, this one logs the GEOMETRY of the one-shot genesis
 * burst: for every voxel that newly manifests, which radial O_h shell it sits in
 * (center / SC face / FCC edge / BCC corner / 2nd-SC / outer), at which tick, with
 * what |J|. This lets the forward model (scripts/exploration/genesis_na_law_forward.py)
 * be compared against the engine on the firing SET / shell profile -- not just the
 * total count -- which is the load-bearing FTD-0267 structural signature
 * ("12 FCC edges do NOT fire while 8 BCC corners do at A=14").
 *
 * GOLDEN-NEUTRAL BY CONSTRUCTION: this is a read-only POST-TICK STATE DIFF. After
 * each rb.run(1) it compares state[i] against the previous tick's snapshot and
 * records newly-nonzero sites. It edits NO engine .cpp, draws NO RNG, and changes
 * NO control flow -- it cannot touch GOLDEN_HASH. (Verify: ctest -R
 * test_render_bridge_golden prints the current pin unchanged.)
 *
 * Canonical ic1 stack, identical to campaign_genesis_trajectory.cpp:
 *   wave_propagation + gauss_projection + genesis + coupling [+ langevin].
 *   x-axial point injection A*K_GENESIS at center.
 *
 * CLI:
 *   --L=N --A=X --ticks=N --seed=0xHEX|N
 *   --thermostat=on|off --coupling=on|off --dir=axial|diag --gamma=X --T=X
 *   --cpu  --tag=S  --output-dir=PATH
 *
 * Output: {output-dir}/geom_A{A}.csv with one row per newly-fired voxel:
 *   tick,idx,x,y,z,dx,dy,dz,shell,jmag,state
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// Minimum-image displacement component on a periodic L-lattice, in (-L/2, L/2].
inline int min_image(int x, int c, int L) {
    int d = ((x - c + L / 2) % L + L) % L - L / 2;
    return d;
}

const char* shell_of(int dx, int dy, int dz) {
    const int r2 = dx * dx + dy * dy + dz * dz;
    if (r2 == 0) return "center";
    if (r2 == 1) return "SC";     // face,   r=1
    if (r2 == 2) return "FCC";    // edge,   r=sqrt2
    if (r2 == 3) return "BCC";    // corner, r=sqrt3
    if (r2 == 4) return "SC2";    // 2nd face shell, r=2
    return "outer";
}

} // namespace

int main(int argc, char** argv) {
    int L = 32;
    double A = 14.0;
    int ticks = 60;
    std::uint32_t seed = 0xE0102000u;
    bool thermostat_on = true;
    bool coupling_on = true;   // canonical FTD-0261/0263 stack default
    bool diag = false;
    double gamma = 0.02;
    double T = 0.005;
    bool force_cpu = false;
    std::string tag = "geom";
    std::string output_dir = "engine/results/genesis_geometry_default/";

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--A=", 0) == 0)          A = std::atof(a.c_str() + 4);
        else if (a.rfind("--ticks=", 0) == 0)      ticks = std::atoi(a.c_str() + 8);
        else if (a.rfind("--seed=", 0) == 0)       seed = static_cast<std::uint32_t>(std::strtoul(a.c_str() + 7, nullptr, 0));
        else if (a == "--thermostat=off")          thermostat_on = false;
        else if (a == "--thermostat=on")           thermostat_on = true;
        else if (a == "--coupling=off")            coupling_on = false;
        else if (a == "--coupling=on")             coupling_on = true;
        else if (a == "--dir=diag")                diag = true;
        else if (a == "--dir=axial")               diag = false;
        else if (a == "--cpu")                     force_cpu = true;
        else if (a.rfind("--gamma=", 0) == 0)      gamma = std::atof(a.c_str() + 8);
        else if (a.rfind("--T=", 0) == 0)          T = std::atof(a.c_str() + 4);
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    ftd::RenderBridge rb(L);
    if (force_cpu) {
        rb.force_cpu();
        rb.set_sor_iterations(150);
    }
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.coupling         = coupling_on;
    rb.toggles.dual_substrate   = false;
    if (thermostat_on) {
        rb.toggles.langevin       = true;
        rb.toggles.langevin_T     = T;
        rb.toggles.langevin_gamma = gamma;
    } else {
        rb.toggles.langevin = false;
    }
    rb.seed_rng(seed);

    const int cx = L / 2;
    if (diag) {
        const double c3 = A * ftd::K_GENESIS / std::sqrt(3.0);
        rb.inject_flux(cx, cx, cx, {c3, c3, c3});
    } else {
        rb.inject_flux(cx, cx, cx, {A * ftd::K_GENESIS, 0, 0});
    }

    fs::create_directories(output_dir);
    char abuf[32];
    std::snprintf(abuf, sizeof(abuf), "%.0f", A);
    const fs::path out_csv = fs::path(output_dir) / ("geom_A" + std::string(abuf) + ".csv");

    std::printf("genesis_geometry: tag=%s L=%d A=%.2f ticks=%d dir=%s thermostat=%s coupling=%s "
                "seed=0x%X backend=%s\n",
                tag.c_str(), L, A, ticks, diag ? "diag" : "axial",
                thermostat_on ? "on" : "off", coupling_on ? "on" : "off",
                seed, force_cpu ? "cpu" : "default");
    std::fflush(stdout);

    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) {
        std::fprintf(stderr, "ERROR: cannot open %s\n", out_csv.string().c_str());
        return 1;
    }
    std::fprintf(f, "# L=%d A=%.4f dir=%s coupling=%s thermostat=%s seed=0x%X backend=%s\n",
                 L, A, diag ? "diag" : "axial", coupling_on ? "on" : "off",
                 thermostat_on ? "on" : "off", seed, force_cpu ? "cpu" : "default");
    std::fprintf(f, "tick,idx,x,y,z,dx,dy,dz,shell,jmag,state\n");

    const int N = L * L * L;
    std::vector<std::int8_t> prev(N, 0);   // previous-tick state snapshot
    int total_fired = 0;

    for (int t = 0; t < ticks; ++t) {
        rb.run(1);
        const auto& voxels = rb.voxels();
        int fired_this_tick = 0;
        for (int i = 0; i < N; ++i) {
            const std::int8_t s = voxels[i].state;
            if (s != 0 && prev[i] == 0) {
                // newly manifested this tick
                const int x = i / (L * L);
                const int y = (i / L) % L;
                const int z = i % L;
                const int dx = min_image(x, cx, L);
                const int dy = min_image(y, cx, L);
                const int dz = min_image(z, cx, L);
                const double jmag = std::sqrt(voxels[i].flux.mag2());
                std::fprintf(f, "%d,%d,%d,%d,%d,%d,%d,%d,%s,%.5f,%d\n",
                             t, i, x, y, z, dx, dy, dz, shell_of(dx, dy, dz),
                             jmag, static_cast<int>(s));
                ++fired_this_tick;
                ++total_fired;
            }
            prev[i] = s;
        }
        if (fired_this_tick > 0)
            std::printf("  t=%3d: newly_fired=%d (cumulative=%d)\n",
                        t, fired_this_tick, total_fired);
        std::fflush(stdout);
    }
    std::fclose(f);
    std::printf("DONE A=%.2f: total_fired=%d -> %s\n", A, total_fired, out_csv.string().c_str());
    return 0;
}

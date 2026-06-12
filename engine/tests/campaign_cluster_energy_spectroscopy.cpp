/**
 * @file campaign_cluster_energy_spectroscopy.cpp
 * @brief FTD-0273 Phase 1 — mass as flux-energy in flip-quanta.
 *
 * Re-expresses the FTD-0110/0269 cluster law N(A)≈k·A² in ENERGY units. The
 * owner's reframe: flux J just carries energy (½|J|²); manifestation is a
 * per-voxel THRESHOLD flip (|J|>K_GENESIS), so the natural quantum is the
 * single-voxel flip energy ε = ½·K_GENESIS². A cluster's "mass" is then its
 * settled flux energy ABOVE the vacuum floor, in ε-units — a dimensionless
 * quanta count, DECOUPLED from m_e (no 0.511 anchor anywhere).
 *
 * Two row classes (column `kind`):
 *   emergent       — GENUINE: axial flux A·K_GENESIS injected at center, genesis
 *                    grows a self-consistent cluster (langevin OFF ⇒ bit-
 *                    deterministic per Phase-0 gate). The settled field energy
 *                    is NOT the injected energy (genesis self-field + dynamics
 *                    add/remove), so M_quanta is a real emergent number.
 *   control_ohseed — CONTROL ONLY: the frozen O_h geometric seeds (octa=7,
 *                    cubocta=13, stella=9, moore=27 manifested STATE voxels,
 *                    genesis OFF). N is pinned to the geometric count; field
 *                    energy is the gauss-induced electrostatic self-energy. The
 *                    structure is IMPOSED ⇒ NOT a mass; the analyzer quotes no
 *                    M_quanta for these. Used to exercise the ledger read and
 *                    confirm genesis-OFF pins N.
 *
 * First-order failure modes (FTD-0272) are detected and reported, never hidden:
 *   EVAPORATED (N=0), FLOODED (N>flood_frac·L³ or still growing), UNSETTLED
 *   (energy drift>tol), BOUNDED (the only rows eligible for a quoted M_quanta).
 *
 * Output: cluster_energy_spectroscopy_<tag>.csv
 *
 * Usage:
 *   campaign_cluster_energy_spectroscopy --Ls=24,32 --As=2,4,6,8,10,12,14,16,20,28,40 \
 *       --seeds=3 --settle=300 --survive=200 --flood-frac=0.25 \
 *       --output-dir=PATH --tag=spec
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/scenarios.h"
#include "ftd/voxel.h"

#include <algorithm>
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

void harness(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.set_sor_iterations(150);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = false;
    rb.toggles.dual_substrate   = false;
}

struct Sampled {
    int n_settle = 0, n_min = 0, n_max = 0;
    double field_avg = 0.0, wave_avg = 0.0, total_avg = 0.0;
    double local_avg = 0.0;                   // ½Σ|J|² over cluster support + R-shell
    double field_min = 0.0, field_max = 0.0;  // whole-lattice slosh amplitude
    double local_min = 0.0, local_max = 0.0;  // cluster-LOCAL slosh (the L-test)
};

// CLUSTER-LOCAL flux energy: ½Σ|J|² over every voxel that is manifested OR within
// Chebyshev radius R of a manifested voxel (periodic). Isolates the cluster's own
// energy from the radiated/un-condensed flux halo that dominates the whole-lattice
// field_energy — the direct test of the owner's "energy IN the cluster" mass model.
double local_flux_energy(const ftd::RenderBridge& rb, int L, int R) {
    const auto& vox = rb.voxels();
    const int N = L * L * L;
    std::vector<char> near(N, 0);
    for (int i = 0; i < N; ++i) {
        if (vox[i].state == 0) continue;
        const int x = i / (L * L), y = (i / L) % L, z = i % L;
        for (int dz = -R; dz <= R; ++dz)
            for (int dy = -R; dy <= R; ++dy)
                for (int dx = -R; dx <= R; ++dx) {
                    const int xx = ((x + dx) % L + L) % L;
                    const int yy = ((y + dy) % L + L) % L;
                    const int zz = ((z + dz) % L + L) % L;
                    near[(xx * L + yy) * L + zz] = 1;
                }
    }
    double e = 0.0;
    for (int i = 0; i < N; ++i) if (near[i]) e += 0.5 * vox[i].flux.mag2();
    return e;
}

// Tick `window` more ticks, sampling field/wave/total + cluster-local energy and
// manifested count each tick. Returns time-averaged energies (sloshing averaged
// out) and the N envelope (n_min/n_max — the cluster-stability signal).
Sampled sample_window(ftd::RenderBridge& rb, int window, int L, int R_local) {
    Sampled s;
    s.n_settle = rb.energy_audit().manifested_count;
    s.n_min = s.n_settle; s.n_max = s.n_settle;
    double sum_f = 0.0, sum_w = 0.0, sum_t = 0.0, sum_l = 0.0;
    bool first = true;
    for (int t = 0; t < window; ++t) {
        rb.tick();
        const ftd::EnergyAudit ea = rb.energy_audit();
        const double eloc = local_flux_energy(rb, L, R_local);
        sum_f += ea.field_energy; sum_w += ea.wave_energy; sum_t += ea.total_energy;
        sum_l += eloc;
        const int n = ea.manifested_count;
        if (n < s.n_min) s.n_min = n;
        if (n > s.n_max) s.n_max = n;
        if (first || ea.field_energy < s.field_min) s.field_min = ea.field_energy;
        if (first || ea.field_energy > s.field_max) s.field_max = ea.field_energy;
        if (first || eloc < s.local_min) s.local_min = eloc;
        if (first || eloc > s.local_max) s.local_max = eloc;
        first = false;
    }
    const double inv = (window > 0) ? 1.0 / window : 1.0;
    s.field_avg = sum_f * inv; s.wave_avg = sum_w * inv;
    s.total_avg = sum_t * inv;  s.local_avg = sum_l * inv;
    return s;
}

} // namespace

int main(int argc, char** argv) {
    std::string Ls_str = "24,32";
    std::string As_str = "2,4,6,8,10,12,14,16,20,28,40";
    int seeds = 3, settle = 300, survive = 200;
    double flood_frac = 0.25;
    std::string tag = "spec";
    std::string output_dir = "engine/results/cluster_energy_spectroscopy/";
    std::uint32_t seed_base = 0xC1057E40u;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--Ls=", 0) == 0)          Ls_str = a.substr(5);
        else if (a.rfind("--As=", 0) == 0)          As_str = a.substr(5);
        else if (a.rfind("--seeds=", 0) == 0)       seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--settle=", 0) == 0)      settle = std::atoi(a.c_str() + 9);
        else if (a.rfind("--survive=", 0) == 0)     survive = std::atoi(a.c_str() + 10);
        else if (a.rfind("--flood-frac=", 0) == 0)  flood_frac = std::atof(a.c_str() + 13);
        else if (a.rfind("--tag=", 0) == 0)         tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0)  output_dir = a.substr(13);
    }

    const std::vector<int> Ls = parse_list<int>(Ls_str);
    const std::vector<double> As = parse_list<double>(As_str);
    const double EPSILON = 0.5 * ftd::K_GENESIS * ftd::K_GENESIS;

    // After `settle` ticks of equilibration we sample over a `window` (== survive)
    // because the undamped flux field does not settle to a CONSTANT energy — it
    // SLOSHES between flux (½|J|²) and wave_vel (½|wave|²) every tick. The cluster
    // identity (manifested count N) IS stable; the instantaneous flux energy is
    // not. So the mass proxy is the TIME-AVERAGED field energy over the window,
    // and BOUNDED is judged by N-stability, not by energy drift. (total_energy is
    // dominated by spurious wave_vel pumped by the non-variational Gauss operator
    // — kept only as a diagnostic, NOT the mass.)
    const int window = survive;

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("cluster_energy_spectroscopy_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "kind,A,seed,L,settle,n_settle,n_min,n_max,field_avg,wave_avg,"
                    "total_avg,local_avg,E_floor,E_above_floor,M_quanta,M_local,"
                    "M_per_voxel,slosh_amp,slosh_local,outcome\n");
    const int R_local = 2;

    std::printf("cluster_energy_spectroscopy: Ls=%s As=%s seeds=%d settle=%d survive=%d "
                "flood_frac=%.2f EPSILON=%.6f\n",
                Ls_str.c_str(), As_str.c_str(), seeds, settle, survive, flood_frac, EPSILON);
    std::fflush(stdout);

    const char* oh_seeds[] = {"s0-seed-octahedron", "s0-seed-cuboctahedron",
                              "s0-seed-stella-octangula", "s0-seed-moore-cell"};

    for (int L : Ls) {
        const long flood_thresh = static_cast<long>(flood_frac * (double)L * L * L);
        const int c = L / 2;

        // ---- vacuum floor: empty lattice, no injection, same harness/ticks ----
        double E_floor = 0.0;
        {
            ftd::RenderBridge rb(L);
            harness(rb);
            rb.seed_rng(seed_base);
            for (int t = 0; t < settle; ++t) rb.tick();
            E_floor = rb.energy_audit().field_energy;
        }
        std::printf("  L=%d  E_floor=%.6g  flood_thresh=%ld voxels\n", L, E_floor, flood_thresh);
        std::fflush(stdout);

        // N-stability tolerance: |n_max - n_min| allowed before calling UNSTABLE.
        auto n_stable = [](const Sampled& s) {
            const double tol = std::max(3.0, 0.30 * (double)s.n_max);
            return (double)(s.n_max - s.n_min) <= tol;
        };
        auto classify = [&](const Sampled& s) -> const char* {
            if (s.n_max == 0)               return "EVAPORATED";
            if ((long)s.n_max > flood_thresh) return "FLOODED";
            if (!n_stable(s))               return "UNSTABLE";
            return "BOUNDED";
        };

        // ---- emergent rows (genuine) ----
        for (double A : As) {
            for (int s = 0; s < seeds; ++s) {
                const std::uint32_t seed = seed_base + static_cast<std::uint32_t>(s) * 2654435761u;
                ftd::RenderBridge rb(L);
                harness(rb);
                rb.seed_rng(seed);
                rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
                for (int t = 0; t < settle; ++t) rb.tick();   // equilibrate
                const Sampled smp = sample_window(rb, window, L, R_local);

                const char* outcome = classify(smp);
                const double E_above = smp.field_avg - E_floor;
                const double M_quanta = E_above / EPSILON;          // whole-lattice flux
                const double M_local  = smp.local_avg / EPSILON;    // cluster-local flux
                const double M_pv = (smp.n_settle > 0) ? M_local / smp.n_settle : 0.0;
                const double slosh = (smp.field_avg > 1e-12)
                                   ? (smp.field_max - smp.field_min) / smp.field_avg : 0.0;
                const double slosh_l = (smp.local_avg > 1e-12)
                                   ? (smp.local_max - smp.local_min) / smp.local_avg : 0.0;

                std::fprintf(f, "emergent,%.0f,%u,%d,%d,%d,%d,%d,%.10g,%.10g,%.10g,%.10g,"
                                "%.10g,%.10g,%.6f,%.6f,%.6f,%.6f,%.6f,%s\n",
                             A, seed, L, settle, smp.n_settle, smp.n_min, smp.n_max,
                             smp.field_avg, smp.wave_avg, smp.total_avg, smp.local_avg,
                             E_floor, E_above, M_quanta, M_local, M_pv, slosh, slosh_l, outcome);
                std::printf("  L=%d A=%-3.0f s=%d  N=%d[%d-%d]  M_loc=%-8.2f  M/vox=%-6.2f  "
                            "slosh_tot=%.2f  slosh_loc=%.2f  %s\n",
                            L, A, s, smp.n_settle, smp.n_min, smp.n_max,
                            M_local, M_pv, slosh, slosh_l, outcome);
                std::fflush(stdout);
            }
        }

        // ---- control: frozen O_h seeds (NOT a mass; ledger/invariant check) ----
        for (const char* name : oh_seeds) {
            ftd::RenderBridge rb(L);
            harness(rb);
            rb.toggles.genesis = false;            // freeze: N pinned to geometry
            rb.seed_rng(seed_base);
            ftd::setup_s0_seed_scenario(rb, name);
            rb.toggles.genesis = false;            // re-assert (moore-cell sets it; others don't)
            for (int t = 0; t < settle; ++t) rb.tick();
            const Sampled smp = sample_window(rb, window, L, R_local);
            const double slosh = (smp.field_avg > 1e-12)
                               ? (smp.field_max - smp.field_min) / smp.field_avg : 0.0;

            const double slosh_l = (smp.local_avg > 1e-12)
                               ? (smp.local_max - smp.local_min) / smp.local_avg : 0.0;
            // M_* sentinel -1: this is a frozen IMPOSED structure, not a mass.
            std::fprintf(f, "control_ohseed,0,0,%d,%d,%d,%d,%d,%.10g,%.10g,%.10g,%.10g,"
                            "%.10g,%.10g,%.6f,%.6f,%.6f,%.6f,%.6f,%s\n",
                         L, settle, smp.n_settle, smp.n_min, smp.n_max, smp.field_avg,
                         smp.wave_avg, smp.total_avg, smp.local_avg, E_floor,
                         smp.field_avg - E_floor, -1.0, -1.0, -1.0, slosh, slosh_l,
                         name + 8 /* strip "s0-seed-" */);
            std::printf("  [control] L=%d %-26s  N=%-3d  <Eflux>=%-8.3f  <Eloc>=%-8.3f  slosh=%.2f\n",
                        L, name, smp.n_settle, smp.field_avg, smp.local_avg, slosh);
            std::fflush(stdout);
        }
    }

    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

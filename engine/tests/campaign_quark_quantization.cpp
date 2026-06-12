/**
 * @file campaign_quark_quantization.cpp
 * @brief FTD-0273 Phase 2 — quantize a colored "quark" with voxels; observe its
 *        phenomena. Companion to campaign_cluster_energy_spectroscopy (Phase 1).
 *
 * The owner: "Assuming quarks are a real thing, we should be able to quantize
 * those with voxels and see what kind of phenomena they present." This is an
 * OBSERVATIONAL instrument — it injects colored clusters and reads back energy
 * (in flip-quanta ε=½·K_GENESIS²) + the engine's color phenomena. NO anchoring
 * to hadron masses; report whatever the engine does.
 *
 * Three experiments (column `expt`), each golden-neutral / read-only:
 *   single  — the 6 seeded quark scenarios (s0-seed-{up..top}-quark). SEEDED ⇒
 *             partly circular (energy ≈ injection); the genuine signal is whether
 *             a colored seed GROWS a bounded cluster under genesis.
 *   confine — TWO opposite/same-color manifested charges at separation r; with
 *             only color_forces on, the realized color force shows up as the
 *             charge's velocity after one tick (∝ |F_color|). Maps the 3-regime
 *             profile (Coulomb r<3, transition 3≤r<8, "linear" r≥8). NOTE: the
 *             r≥8 force is F∝r (harmonic V∝r²), NOT constant string tension —
 *             measured and reported as-is.
 *   triad   — color-singlet binding: triad_binding requires color_forces AND
 *             dual_substrate (term_toggles.h). 3 manifested charges, colors
 *             {1,2,3}, equilateral; count locked voxels + bound energy, with the
 *             binding toggle ON vs OFF. Dual-substrate energies are a DIFFERENT
 *             branch — do NOT compare their ε-scale to Phase 1.
 *
 * Output: quark_quantization_<tag>.csv  (heterogeneous; `expt` selects columns)
 *
 * Usage:
 *   campaign_quark_quantization --L=24 --settle=200 --window=50 \
 *       --rs=2,3,4,5,6,8,10,12,14 --output-dir=PATH --tag=qq
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/scenarios.h"
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

std::vector<int> parse_ints(const std::string& s) {
    std::vector<int> out; std::size_t i = 0;
    while (i < s.size()) {
        std::size_t j = s.find(',', i); if (j == std::string::npos) j = s.size();
        out.push_back(std::atoi(s.substr(i, j - i).c_str())); i = j + 1;
    }
    return out;
}

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

} // namespace

int main(int argc, char** argv) {
    int L = 24, settle = 200, window = 50;
    std::string rs_str = "2,3,4,5,6,8,10,12,14";
    std::string tag = "qq";
    std::string output_dir = "engine/results/quark_quantization/";
    const std::uint32_t seed = 0x90A12C00u;
    const double EPSILON = 0.5 * ftd::K_GENESIS * ftd::K_GENESIS;
    const int R_local = 2;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--settle=", 0) == 0)     settle = std::atoi(a.c_str() + 9);
        else if (a.rfind("--window=", 0) == 0)     window = std::atoi(a.c_str() + 9);
        else if (a.rfind("--rs=", 0) == 0)         rs_str = a.substr(5);
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const std::vector<int> rs = parse_ints(rs_str);
    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("quark_quantization_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "expt,label,r,pair,binding,manifested,color,locked,field_energy,"
                    "local_energy,M_local,force_proxy\n");

    std::printf("quark_quantization: L=%d settle=%d window=%d rs=%s EPSILON=%.6f\n",
                L, settle, window, rs_str.c_str(), EPSILON);
    std::fflush(stdout);

    const int c = L / 2;

    auto harness_base = [&](ftd::RenderBridge& rb) {
        rb.force_cpu();
        rb.set_sor_iterations(150);
        rb.toggles.disable_all();
    };

    // ---- 2a single seeded quark ----
    const char* quarks[] = {"s0-seed-up-quark", "s0-seed-down-quark",
                            "s0-seed-strange-quark", "s0-seed-charm-quark",
                            "s0-seed-bottom-quark", "s0-seed-top-quark"};
    for (const char* qn : quarks) {
        ftd::RenderBridge rb(L);
        harness_base(rb);
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.forces           = true;   // master gate for phase_forces
        rb.toggles.color_forces     = true;
        rb.toggles.langevin         = false;
        rb.seed_rng(seed);
        ftd::setup_s0_seed_scenario(rb, qn);
        for (int t = 0; t < settle; ++t) rb.tick();
        double sum_local = 0.0; long sum_n = 0;
        for (int t = 0; t < window; ++t) {
            rb.tick();
            sum_local += local_flux_energy(rb, L, R_local);
            sum_n += rb.energy_audit().manifested_count;
        }
        const double Eloc = sum_local / window;
        const int n_avg = (int)(sum_n / window);
        const int center_color = rb.voxels()[(c * L + c) * L + c].color;
        std::fprintf(f, "single,%s,0,,,%d,%d,0,%.10g,%.10g,%.6f,0\n",
                     qn + 8, n_avg, center_color, rb.energy_audit().field_energy,
                     Eloc, Eloc / EPSILON);
        std::printf("  [single] %-12s  N=%-4d  color=%d  M_local=%.2f\n",
                    qn + 8, n_avg, center_color, Eloc / EPSILON);
        std::fflush(stdout);
    }

    // ---- 2b confinement: F(r) via velocity after 1 tick, color_forces only ----
    for (int pair = 0; pair < 2; ++pair) {            // 0 = diff color, 1 = same color
        const int c1 = 1, c2 = (pair == 0) ? 2 : 1;
        for (int r : rs) {
            if (c + r >= L) continue;
            ftd::RenderBridge rb(L);
            harness_base(rb);
            // wave+gauss populate the active-index set so phase_forces processes
            // the charges (color force only iterates ordered_active_indices()).
            // genesis + movement OFF: the two charges stay fixed at separation r.
            rb.toggles.wave_propagation = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces           = true;   // master gate for phase_forces
            rb.toggles.color_forces     = true;
            rb.seed_rng(seed);
            rb.inject_particle(c, c, c, +1, {0, 0, 0}, +1, (int8_t)c1);
            rb.inject_particle(c + r, c, c, +1, {0, 0, 0}, +1, (int8_t)c2);
            rb.tick();                                 // phase_forces populates force_diag_
            // Read the color force directly (movement OFF, so velocity stays 0;
            // the realized |F_color| lives in the force diagnostic's f_strong).
            const double force_proxy = rb.force_diag_at(c, c, c).f_strong.mag();
            const double fe = rb.energy_audit().field_energy;
            std::fprintf(f, "confine,,%d,%s,,2,%d,0,%.10g,0,0,%.10g\n",
                         r, (pair == 0) ? "diff" : "same", c1, fe, force_proxy);
            std::printf("  [confine] pair=%s r=%-2d  |F|=%.6g\n",
                        (pair == 0) ? "diff" : "same", r, force_proxy);
            std::fflush(stdout);
        }
    }

    // ---- 2c triad: color-singlet binding (dual_substrate + triad_binding) ----
    // triad_binding (transmutation_phases.cpp) locks 3 same-state particles whose
    // pairwise distances are all <= TRIAD_RADIUS (3.0) and near-equilateral
    // (min/max >= 0.8). It is purely GEOMETRIC — it does NOT check color, so the
    // "color singlet" is our colors {1,2,3} label, not an engine requirement.
    // Place a compact integer triad: pairwise {2, 2.24, 2.24}, ratio 0.89.
    for (int binding = 0; binding < 2; ++binding) {
        ftd::RenderBridge rb(L);
        harness_base(rb);
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.dual_substrate   = true;
        rb.toggles.forces           = true;   // master gate for phase_forces
        rb.toggles.color_forces     = true;
        rb.toggles.triad_binding    = (binding == 1);
        rb.seed_rng(seed);
        const int tx[3] = {c, c + 2, c + 1};
        const int ty[3] = {c, c,     c + 2};
        for (int k = 0; k < 3; ++k)
            rb.inject_particle(tx[k], ty[k], c, +1, {0, 0, 0}, +1, (int8_t)(k + 1));
        for (int t = 0; t < settle; ++t) rb.tick();
        double sum_local = 0.0, sum_field = 0.0; long sum_n = 0; long sum_locked = 0;
        for (int t = 0; t < window; ++t) {
            rb.tick();
            sum_local += local_flux_energy(rb, L, R_local);
            const ftd::EnergyAudit ea = rb.energy_audit();
            sum_field += ea.field_energy; sum_n += ea.manifested_count;
            int locked = 0;
            const auto& vox = rb.voxels();
            for (int i = 0; i < L * L * L; ++i) if (vox[i].locked) ++locked;
            sum_locked += locked;
        }
        const double Eloc = sum_local / window, Efield = sum_field / window;
        const int n_avg = (int)(sum_n / window), locked_avg = (int)(sum_locked / window);
        std::fprintf(f, "triad,,0,,%d,%d,0,%d,%.10g,%.10g,%.6f,0\n",
                     binding, n_avg, locked_avg, Efield, Eloc, Eloc / EPSILON);
        std::printf("  [triad] binding=%d  N=%-3d  locked=%-3d  M_local=%.2f  Efield=%.3f\n",
                    binding, n_avg, locked_avg, Eloc / EPSILON, Efield);
        std::fflush(stdout);
    }

    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

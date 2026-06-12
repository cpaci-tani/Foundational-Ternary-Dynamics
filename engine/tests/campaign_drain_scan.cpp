/**
 * @file campaign_drain_scan.cpp
 * @brief FTD-0276 Leg A: does the cluster-efficiency k_eff scale as drain²?
 *
 * The kinetic drain (fraction of wave_vel consumed at a genesis manifestation,
 * `v.wave_vel *= (1 − kinetic_drain)`) is the engine-tuning constant FTD-0269
 * found decisively load-bearing for the N(A) law's calibration (knee shifts 16
 * grid-units across drain ∈ {0.25, 0.5, 0.75}). `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md`
 * §12 floats the untested hypothesis k = drain² (0.5² = 0.25 = 1/N_base). This
 * runner measures the sub-knee cluster efficiency k_eff(drain) on a fine drain
 * grid to test that power law.
 *
 * Method (canonical ic1 stack, identical to campaign_genesis_geometry / FTD-0261):
 *   wave_propagation + gauss_projection + genesis + coupling + langevin (γ=0.02,
 *   T=0.005). x-axial point injection A·K_GENESIS at the center voxel. Settle
 *   `--settle` ticks, then report the settled manifested count N = energy_audit()
 *   .manifested_count (the L-invariant cluster identity per FTD-0273). Sweeps
 *   --drains × --As × --seeds; sets rb.toggles.kinetic_drain per drain.
 *
 * GOLDEN-NEUTRAL: read-only campaign; at the default drain 0.5 the engine is
 * bit-identical to the legacy constexpr path (verify: test_render_bridge_golden
 * prints 0x56fa28acb5b9fe88). Uses the FTD-0276 runtime kinetic_drain toggle.
 *
 * Output: drain_scan_<tag>.csv
 *   drain,A,seed,N,settle,L
 *
 * Usage:
 *   campaign_drain_scan --L=32 --drains=0.125,0.25,0.375,0.5,0.625,0.75 \
 *       --As=10,12,14,16,20,25,30,40 --seeds=5 --settle=300 --cpu --tag=v1
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

} // namespace

int main(int argc, char** argv) {
    int L = 32, settle = 300, seeds = 5;
    std::string drains_str = "0.125,0.25,0.375,0.5,0.625,0.75";
    std::string As_str = "10,12,14,16,20,25,30,40";
    bool force_cpu = false;
    double gamma = 0.02, T = 0.005;
    std::string tag = "v1";
    std::string output_dir = "engine/results/drain_scan/";
    std::uint32_t seed_base = 0xD3A10000u;

    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if      (a.rfind("--L=", 0) == 0)          L = std::atoi(a.c_str() + 4);
        else if (a.rfind("--drains=", 0) == 0)     drains_str = a.substr(9);
        else if (a.rfind("--As=", 0) == 0)         As_str = a.substr(5);
        else if (a.rfind("--seeds=", 0) == 0)      seeds = std::atoi(a.c_str() + 8);
        else if (a.rfind("--settle=", 0) == 0)     settle = std::atoi(a.c_str() + 9);
        else if (a.rfind("--gamma=", 0) == 0)      gamma = std::atof(a.c_str() + 8);
        else if (a.rfind("--T=", 0) == 0)          T = std::atof(a.c_str() + 4);
        else if (a == "--cpu")                     force_cpu = true;
        else if (a.rfind("--tag=", 0) == 0)        tag = a.substr(6);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }

    const std::vector<double> drains = parse_list<double>(drains_str);
    const std::vector<double> As = parse_list<double>(As_str);

    fs::create_directories(output_dir);
    const fs::path out_csv = fs::path(output_dir) / ("drain_scan_" + tag + ".csv");
    std::FILE* f = std::fopen(out_csv.string().c_str(), "w");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", out_csv.string().c_str()); return 1; }
    std::fprintf(f, "drain,A,seed,N,settle,L\n");
    std::fflush(f);   // per-row fflush below survives a mid-run kill (MSVC _IOLBF is full-buffered)

    std::printf("drain_scan: L=%d drains=%s As=%s seeds=%d settle=%d backend=%s\n",
                L, drains_str.c_str(), As_str.c_str(), seeds, settle,
                force_cpu ? "cpu" : "default");
    std::fflush(stdout);

    const int cx = L / 2;
    for (double drain : drains) {
        for (double A : As) {
            double nsum = 0.0; int nseen = 0;
            for (int s = 0; s < seeds; ++s) {
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
                rb.toggles.langevin_T       = T;
                rb.toggles.kinetic_drain    = drain;   // FTD-0276 runtime knob
                rb.seed_rng(seed_base + (std::uint32_t)s * 2654435761u);
                rb.inject_flux(cx, cx, cx, {A * ftd::K_GENESIS, 0, 0});
                rb.run(settle);
                const ftd::EnergyAudit ea = rb.energy_audit();
                const long N = (long)ea.manifested_count;
                std::fprintf(f, "%.4f,%.1f,%d,%ld,%d,%d\n", drain, A, s, N, settle, L);
                std::fflush(f);
                nsum += (double)N; ++nseen;
            }
            const double nbar = nseen ? nsum / nseen : 0.0;
            const double k_eff = (A > 0 ? nbar / (A * A) : 0.0);
            std::printf("  drain=%.3f A=%5.1f  N̄=%7.2f  k_eff=N̄/A²=%.4f\n",
                        drain, A, nbar, k_eff);
            std::fflush(stdout);
        }
    }
    std::fclose(f);
    std::printf("wrote %s\n", out_csv.string().c_str());
    return 0;
}

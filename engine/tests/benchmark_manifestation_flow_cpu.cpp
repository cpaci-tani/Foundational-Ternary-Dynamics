/**
 * @file benchmark_manifestation_flow_cpu.cpp
 * @brief Single-seed CPU measurement of the FTD-native b=2 flow on a
 *        manifestation-dressed background. Emits one JSON row to stdout.
 *
 * Plan B reroute, Task 6'.
 * Plan: docs/superpowers/plans/2026-04-23-manifestation-scale-flow.md
 *
 * Pipeline:
 *   1. prepare_manifestation_background(L, n, seed, settle)
 *   2. render_bridge_to_dual_cell_fields(bg)
 *   3. measure_native_b2_flow(fine)
 *   4. emit JSON with flux_energy_{fine,coarse,ratio}, Gauss residuals,
 *      source counts, wall time.
 *
 * Primary observable: flux_energy_ratio. At the bare Gaussian fixed point
 * this equals 1 exactly (analytical theorem). Deviation from 1 at density
 * n > 0 is the non-linear manifestation signal.
 *
 * Usage:
 *   benchmark_manifestation_flow_cpu --L=32 --density=0.01 --seed=42 \
 *       --settle=200
 */
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "ftd/eft/coupling_measurement.h"       // configure_bare_lattice_for_coupling
#include "ftd/eft/dual_cell_blocking.h"         // render_bridge_to_dual_cell_fields
#include "ftd/eft/dual_cell_flow.h"             // measure_native_b2_flow
#include "ftd/eft/manifestation_background.h"   // prepare_manifestation_background
#include "ftd/render_bridge.h"

namespace {
struct Args {
    int L = 32;
    double density = 0.01;
    uint64_t seed = 42;
    int settle_ticks = 200;
    bool force_cpu = true;
};

Args parse(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        const char* s = argv[i];
        auto eat = [&](const char* k, auto& dst) {
            const size_t n = std::strlen(k);
            if (std::strncmp(s, k, n) == 0 && s[n] == '=') {
                dst = std::stod(s + n + 1);
                return true;
            }
            return false;
        };
        double tmp = 0.0;
        if (eat("--L", tmp)) a.L = static_cast<int>(tmp);
        else if (eat("--density", tmp)) a.density = tmp;
        else if (eat("--seed", tmp)) a.seed = static_cast<uint64_t>(tmp);
        else if (eat("--settle", tmp)) a.settle_ticks = static_cast<int>(tmp);
        else if (std::strcmp(s, "--gpu") == 0) a.force_cpu = false;
    }
    return a;
}
}  // namespace

int main(int argc, char** argv) {
    const Args a = parse(argc, argv);
    using clock = std::chrono::steady_clock;
    const auto t0 = clock::now();

    // Prepare manifestation background.
    auto bg = ftd::eft::prepare_manifestation_background(
        a.L, a.density, a.seed, a.settle_ticks);
    if (a.force_cpu) bg->force_cpu();
    const int n_placed = ftd::eft::count_manifested_sites(*bg);

    // Adapt to DualCellFields.
    auto fine = ftd::eft::render_bridge_to_dual_cell_fields(*bg);

    // Run the b=2 flow measurement.
    const ftd::eft::NativeB2FlowReport rep =
        ftd::eft::measure_native_b2_flow(fine);

    const auto t1 = clock::now();
    const double wall = std::chrono::duration<double>(t1 - t0).count();

    // Adapter consistency: source count must be preserved under b=2 blocking
    // (integer-exact). gauss_preserved is a theorem-level check that only
    // holds for analytically constructed fields — the engine's SOR projection
    // leaves O(1e-3) residuals, so we REPORT gauss_residual_{fine,coarse}
    // for diagnostic value but do NOT gate the pipeline on it.
    const char* status = rep.source_conserved ? "ok" : "pipeline_error";

    std::printf(
        "[{\"L\": %d, \"n\": %.6g, \"level\": 0, \"seed\": %llu, "
        "\"n_placed\": %d, "
        "\"total_source_fine\": %d, \"total_source_coarse\": %d, "
        "\"flux_energy_fine\": %.10g, \"flux_energy_coarse\": %.10g, "
        "\"flux_energy_ratio\": %.10g, "
        "\"gauss_residual_fine\": %.6g, \"gauss_residual_coarse\": %.6g, "
        "\"source_conserved\": %s, \"gauss_preserved\": %s, "
        "\"wall_seconds\": %.3f, \"status\": \"%s\"}]\n",
        a.L, a.density, static_cast<unsigned long long>(a.seed),
        n_placed,
        rep.total_source_fine, rep.total_source_coarse,
        rep.flux_energy_fine, rep.flux_energy_coarse, rep.flux_energy_ratio,
        rep.gauss_residual_fine, rep.gauss_residual_coarse,
        rep.source_conserved ? "true" : "false",
        rep.gauss_preserved  ? "true" : "false",
        wall, status);

    return rep.source_conserved ? 0 : 1;
}

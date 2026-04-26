/**
 * @file test_cpu_warnings.cpp
 * @brief Verify CPU-build runtime warnings fire for GPU-only toggles.
 *
 * The engine has toggles whose implementation lives only on the GPU path
 * (strong_force, exchange_force per cpu_runtime_warnings()). On a CPU build
 * these toggles are silent no-ops. The audit (test-orchestrator, 2026-04-25)
 * flagged that no test verifies the warning fires AND that the observable is
 * actually invariant when the GPU-only toggle is set on CPU.
 *
 * This test:
 *   W1. Captures stderr around RenderBridge::tick() with strong_force=true
 *       on CPU; asserts a non-empty warning string is emitted.
 *   W2. Asserts that voxel state with vs without strong_force on CPU is
 *       identical (toggle is genuinely a no-op, no silent partial-effect).
 *   W3. Same for exchange_force.
 *
 * On a GPU-enabled build, the test still constructs RenderBridge with
 * force_cpu() so the warning path is exercised consistently.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

namespace {

bool voxels_byte_equal(const std::vector<ftd::Voxel>& a, const std::vector<ftd::Voxel>& b) {
    if (a.size() != b.size()) return false;
    return std::memcmp(a.data(), b.data(), a.size() * sizeof(ftd::Voxel)) == 0;
}

// Run a brief simulation with the given toggle set, capture stderr.
struct Run {
    std::vector<ftd::Voxel> voxels;
    std::string stderr_text;
};

Run run_capture(int L, bool strong, bool exchange) {
    Run r;
    // Redirect stderr to a stringstream
    std::stringstream sink;
    std::streambuf* old_cerr = std::cerr.rdbuf(sink.rdbuf());

    {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.poisson_coulomb  = true;
        rb.toggles.forces           = true;
        rb.toggles.movement         = true;
        rb.toggles.strong_force     = strong;
        rb.toggles.exchange_force   = exchange;
        rb.force_cpu();
        rb.seed_rng(0x12345678u);
        // Stamp a charged pair so forces have something to act on
        rb.inject_particle(L/2 - 2, L/2, L/2, +1, ftd::Vec3{0,0,0}, 0, 1);
        rb.inject_particle(L/2 + 2, L/2, L/2, -1, ftd::Vec3{0,0,0}, 0, 2);
        rb.run(2);
        r.voxels = rb.voxels();
    }

    std::cerr.rdbuf(old_cerr);
    r.stderr_text = sink.str();
    return r;
}

bool contains(const std::string& haystack, const char* needle) {
    return haystack.find(needle) != std::string::npos;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  CPU-Only Runtime Warnings (GPU-only toggles)\n");
    std::printf("================================================================\n");
    std::printf("  Asserts: strong_force / exchange_force toggles emit a stderr\n");
    std::printf("           warning AND are genuine no-ops on CPU builds.\n\n");

#ifdef _OPENMP
    omp_set_num_threads(1);
#endif

    const int L = 8;
    int failures = 0;

    // Baseline: neither GPU-only toggle set
    Run baseline   = run_capture(L, /*strong=*/false, /*exchange=*/false);
    Run with_strong   = run_capture(L, /*strong=*/true,  /*exchange=*/false);
    Run with_exchange = run_capture(L, /*strong=*/false, /*exchange=*/true);
    Run with_both     = run_capture(L, /*strong=*/true,  /*exchange=*/true);

    std::printf("  W1 strong_force=true on CPU build:\n");
    std::printf("    stderr length: %zu chars\n", with_strong.stderr_text.size());
    if (!contains(with_strong.stderr_text, "strong_force")) {
        std::printf("    FAIL: expected 'strong_force' warning in stderr, not found.\n");
        ++failures;
    } else {
        std::printf("    PASS: warning fired with 'strong_force' substring\n");
    }
    if (!voxels_byte_equal(baseline.voxels, with_strong.voxels)) {
        std::printf("    FAIL: strong_force=true changed voxel state on CPU "
                    "(should be a no-op).\n");
        ++failures;
    } else {
        std::printf("    PASS: voxel state byte-identical to baseline (no-op confirmed)\n");
    }

    std::printf("\n  W2 exchange_force=true on CPU build:\n");
    std::printf("    stderr length: %zu chars\n", with_exchange.stderr_text.size());
    // exchange_force requires poisson_coulomb=true (validated). We've enabled it.
    if (!contains(with_exchange.stderr_text, "exchange_force")) {
        std::printf("    FAIL: expected 'exchange_force' warning in stderr.\n");
        ++failures;
    } else {
        std::printf("    PASS: warning fired with 'exchange_force' substring\n");
    }
    if (!voxels_byte_equal(baseline.voxels, with_exchange.voxels)) {
        std::printf("    FAIL: exchange_force=true changed voxel state on CPU.\n");
        ++failures;
    } else {
        std::printf("    PASS: voxel state byte-identical to baseline\n");
    }

    std::printf("\n  W3 both GPU-only toggles set:\n");
    std::printf("    stderr length: %zu chars\n", with_both.stderr_text.size());
    const bool both_warned = contains(with_both.stderr_text, "strong_force")
                          && contains(with_both.stderr_text, "exchange_force");
    if (!both_warned) {
        std::printf("    FAIL: expected both warnings in stderr.\n");
        ++failures;
    } else {
        std::printf("    PASS: both warnings present\n");
    }

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: CPU-only warning contract intact (PASS)\n");
    } else {
        std::printf("  RESULT: %d failure(s) — GPU-only toggles silently misbehave on CPU\n",
                    failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}

/**
 * @file test_cpu_warnings.cpp
 * @brief CPU Yukawa / exchange are live pairwise channels, not GPU-only no-ops.
 *
 * Historically these toggles were CUDA-only and this file pinned the
 * advertised no-op + stderr warning. Both channels now share
 * yukawa_pair_force_mag / exchange_pair_force_mag with the GPU kernels.
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/term_toggles.h"

namespace {

bool voxels_byte_equal(const std::vector<ftd::Voxel>& a, const std::vector<ftd::Voxel>& b) {
    if (a.size() != b.size()) return false;
    return std::memcmp(a.data(), b.data(), a.size() * sizeof(ftd::Voxel)) == 0;
}

std::vector<ftd::Voxel> run_pair(int L, bool strong, bool exchange, int8_t spin) {
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
    rb.inject_particle(L/2 - 2, L/2, L/2, +1, ftd::Vec3{0,0,0}, spin, 1);
    rb.inject_particle(L/2 + 2, L/2, L/2, -1, ftd::Vec3{0,0,0}, spin, 2);
    rb.run(2);
    return rb.voxels();
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  CPU pairwise force channels (Yukawa / exchange)\n");
    std::printf("================================================================\n");

#ifdef _OPENMP
    omp_set_num_threads(1);
#endif

    const int L = 8;
    int failures = 0;

    ftd::TermToggles strong_on;
    strong_on.disable_all();
    strong_on.strong_force = true;
    if (!strong_on.cpu_runtime_warnings().empty()) {
        std::printf("  FAIL: strong_force still carries a gpu_only_warning\n");
        ++failures;
    } else {
        std::printf("  PASS: strong_force has no GPU-only warning\n");
    }

    ftd::TermToggles exchange_on;
    exchange_on.disable_all();
    exchange_on.poisson_coulomb = true;
    exchange_on.exchange_force = true;
    if (!exchange_on.cpu_runtime_warnings().empty()) {
        std::printf("  FAIL: exchange_force still carries a gpu_only_warning\n");
        ++failures;
    } else {
        std::printf("  PASS: exchange_force has no GPU-only warning\n");
    }

    auto baseline = run_pair(L, false, false, 0);
    auto with_strong = run_pair(L, true, false, 0);
    if (voxels_byte_equal(baseline, with_strong)) {
        std::printf("  FAIL: strong_force=true did not change CPU voxel state\n");
        ++failures;
    } else {
        std::printf("  PASS: Yukawa changes CPU state vs baseline\n");
    }

    auto base_spin = run_pair(L, false, false, +1);
    auto with_exchange = run_pair(L, false, true, +1);
    if (voxels_byte_equal(base_spin, with_exchange)) {
        std::printf("  FAIL: exchange_force=true did not change CPU voxel state\n");
        ++failures;
    } else {
        std::printf("  PASS: exchange changes CPU state vs same-spin baseline\n");
    }

    std::printf("\n================================================================\n");
    if (failures == 0) {
        std::printf("  RESULT: CPU Yukawa/exchange are live (PASS)\n");
    } else {
        std::printf("  RESULT: %d failure(s)\n", failures);
    }
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}

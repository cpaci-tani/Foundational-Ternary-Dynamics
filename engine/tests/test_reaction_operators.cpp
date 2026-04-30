/**
 * @file test_reaction_operators.cpp
 * @brief Unit tests for the FTD-0112 reaction-sector operators (O7-O10).
 *
 * Sanity checks:
 *   1. Identical snapshots → all 4 operators return 0 (no reactions).
 *   2. Single from-vacuum genesis event → O7 > 0, O8 > 0, O9 = 0.
 *   3. Single to-vacuum evap event → O7 > 0, O8 = 0, O9 > 0.
 *   4. Pure flux (s ≡ 0 always) → all 4 operators return 0.
 */

#include "ftd/eft/dual_cell_blocking.h"
#include "ftd/eft/reaction_operators.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

using ftd::eft::DualCellFields;
using ftd::eft::SnapshotPair;
using ftd::eft::accumulate_reaction_means;
using ftd::eft::evaluate_reaction_op;
using ftd::eft::kNumReactionOps;
using ftd::eft::kReactionOpNames;

constexpr double kEps = 1e-10;

// Build a "zero" field at given L.
DualCellFields make_zero(int L) {
    DualCellFields f(L);
    for (auto& v : f.rho_cell) v = 0;
    for (auto& v : f.phi_x) v = 0.0;
    for (auto& v : f.phi_y) v = 0.0;
    for (auto& v : f.phi_z) v = 0.0;
    return f;
}

bool nearly(double a, double b, double tol = kEps) {
    return std::abs(a - b) < tol;
}

// Test 1: identical snapshots → all 4 ops return 0.
bool test_identical_snapshots() {
    const int L = 4;
    auto a = make_zero(L);
    auto b = make_zero(L);
    // Add some flux but keep state identical.
    a.phi_x[a.index(1, 1, 1)] = 0.5;
    a.phi_y[a.index(2, 2, 2)] = 0.7;
    b = a;  // identical
    SnapshotPair p{a, b};
    std::array<double, kNumReactionOps> means{};
    accumulate_reaction_means(p, means);
    bool ok = true;
    for (int i = 0; i < kNumReactionOps; ++i) {
        if (!nearly(means[i], 0.0)) {
            std::fprintf(stderr, "[FAIL] identical snapshots: %s = %.6e (expected 0)\n",
                         kReactionOpNames[i], means[i]);
            ok = false;
        }
    }
    return ok;
}

// Test 2: single from-vacuum genesis at one cell → O7 > 0, O8 > 0, O9 = 0.
bool test_single_genesis() {
    const int L = 4;
    auto a = make_zero(L);
    auto b = make_zero(L);
    // Set flux in 'before' so that |J_before| > 0 at the genesis cell.
    a.phi_x[a.index(2, 2, 2)] = 1.0;
    b.phi_x[b.index(2, 2, 2)] = 1.0;  // doesn't matter — we read from "before"
    // Single genesis at (2,2,2): s_before = 0, s_after = +1.
    a.rho_cell[a.index(2, 2, 2)] = 0;
    b.rho_cell[b.index(2, 2, 2)] = 1;
    SnapshotPair p{a, b};
    std::array<double, kNumReactionOps> means{};
    accumulate_reaction_means(p, means);

    bool ok = true;
    if (!(means[0] > 0.0)) {
        std::fprintf(stderr, "[FAIL] genesis: reactionDensity = %.6e (expected > 0)\n", means[0]);
        ok = false;
    }
    if (!(means[1] > 0.0)) {
        std::fprintf(stderr, "[FAIL] genesis: genesisFlux = %.6e (expected > 0)\n", means[1]);
        ok = false;
    }
    if (!nearly(means[2], 0.0)) {
        std::fprintf(stderr, "[FAIL] genesis: evapFlux = %.6e (expected 0)\n", means[2]);
        ok = false;
    }

    // Verify quantitative value of O7: 1/L^3 (single cell with δs² = 1).
    const double expected_O7 = 1.0 / (L * L * L);
    if (!nearly(means[0], expected_O7)) {
        std::fprintf(stderr, "[FAIL] genesis: reactionDensity = %.6e (expected %.6e)\n",
                     means[0], expected_O7);
        ok = false;
    }
    return ok;
}

// Test 3: single to-vacuum evap at one cell → O7 > 0, O8 = 0, O9 > 0.
bool test_single_evap() {
    const int L = 4;
    auto a = make_zero(L);
    auto b = make_zero(L);
    a.phi_x[a.index(1, 1, 1)] = 0.8;
    b.phi_x[b.index(1, 1, 1)] = 0.0;
    a.rho_cell[a.index(1, 1, 1)] = -1;  // s_before = -1
    b.rho_cell[b.index(1, 1, 1)] = 0;   // s_after = 0
    SnapshotPair p{a, b};
    std::array<double, kNumReactionOps> means{};
    accumulate_reaction_means(p, means);

    bool ok = true;
    if (!(means[0] > 0.0)) {
        std::fprintf(stderr, "[FAIL] evap: reactionDensity = %.6e (expected > 0)\n", means[0]);
        ok = false;
    }
    if (!nearly(means[1], 0.0)) {
        std::fprintf(stderr, "[FAIL] evap: genesisFlux = %.6e (expected 0)\n", means[1]);
        ok = false;
    }
    if (!(means[2] > 0.0)) {
        std::fprintf(stderr, "[FAIL] evap: evapFlux = %.6e (expected > 0)\n", means[2]);
        ok = false;
    }
    return ok;
}

// Test 4: pure flux (s ≡ 0 always) → all 4 ops return 0.
bool test_pure_flux_no_reactions() {
    const int L = 4;
    auto a = make_zero(L);
    auto b = make_zero(L);
    // Different flux values, but state stays 0 in both.
    a.phi_x[a.index(1, 1, 1)] = 0.5;
    a.phi_y[a.index(2, 2, 2)] = 0.7;
    b.phi_x[b.index(1, 1, 1)] = 0.3;
    b.phi_y[b.index(2, 2, 2)] = 0.9;
    SnapshotPair p{a, b};
    std::array<double, kNumReactionOps> means{};
    accumulate_reaction_means(p, means);
    bool ok = true;
    for (int i = 0; i < kNumReactionOps; ++i) {
        if (!nearly(means[i], 0.0)) {
            std::fprintf(stderr, "[FAIL] pure flux: %s = %.6e (expected 0)\n",
                         kReactionOpNames[i], means[i]);
            ok = false;
        }
    }
    return ok;
}

}  // namespace

int main() {
    std::printf("FTD-0112 reaction operators unit tests\n");
    std::printf("--------------------------------------\n");
    int total = 0, passed = 0;

    auto run = [&](const char* name, bool (*test)()) {
        ++total;
        std::printf("  %-40s ... ", name);
        if (test()) {
            std::printf("PASS\n");
            ++passed;
        } else {
            std::printf("FAIL\n");
        }
    };

    run("identical snapshots → all ops zero", test_identical_snapshots);
    run("single genesis event", test_single_genesis);
    run("single evap event", test_single_evap);
    run("pure flux (s ≡ 0) → all ops zero", test_pure_flux_no_reactions);

    std::printf("--------------------------------------\n");
    std::printf("Result: %d / %d passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}

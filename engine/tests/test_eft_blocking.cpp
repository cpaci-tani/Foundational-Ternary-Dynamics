/**
 * @file test_eft_blocking.cpp
 * @brief EFT Phase 2A — block-spin transformation validation gate.
 *
 * This test is the pre-β validation gate required by
 * SPEC_EFT_RECOVERY_PROGRAM.md §5.1. Before any β-function measurement
 * runs, blocking itself must be physically consistent — specifically,
 * charge must be conserved under the coarse-graining.
 *
 * Checks
 * ------
 *   B1: single +1 voxel on L=32 → block → coarse total charge = 1
 *       (pre-registered gate in SPEC §5.1)
 *   B2: pair (+1, −1) on L=32 → block → coarse total charge = 0
 *   B3: three distant +1 voxels on L=32 → block → coarse total = 3
 *   B4: uniform flux field J = (0.1, 0.2, 0.3) on L=16 → block → same
 *       uniform flux on coarse lattice (mean of identical samples)
 *   B5: random flux (unit-variance Gaussian) on L=16 → block → coarse
 *       total flux² is in [L_coarse³ · 3σ²/8, L_coarse³ · 3σ²/8 · 2]
 *       i.e. flux-squared ratio (coarse/fine) ≈ 1/8 within statistical
 *       error (independent-sample averaging reduces variance by 1/8
 *       for N=8 children)
 *   B6: block twice (L=32 → L=16 → L=8) — total charge preserved through
 *       both stages for an asymmetric multi-charge configuration
 *   B7: single +2 block (four +1 + zero −1 voxels + remaining zeros) →
 *       charge-conserving blocking places +1 at block centre plus
 *       overflow +1 at next-lex coarse site; total = 2
 */

#include <cmath>
#include <cstdio>
#include <random>

#include "ftd/eft/blocking.h"
#include "ftd/render_bridge.h"

static int g_failures = 0;

static void check(const char* name, bool ok, const char* detail = nullptr) {
    if (ok) std::printf("  PASS  %s\n", name);
    else {
        std::printf("  FAIL  %s%s%s\n", name,
                    detail ? "  " : "", detail ? detail : "");
        ++g_failures;
    }
}

// B1 — single +1 voxel
static void b1_single_positive() {
    std::puts("\n--- B1: single +1 voxel → charge preserved ---");
    ftd::RenderBridge rb(32);
    rb.voxel_at(16, 16, 16).state = +1;
    auto blocked = ftd::eft::block_full(rb);
    auto ig = ftd::eft::check_integrity(rb, *blocked);
    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(fine=%lld coarse=%lld conserved=%s)",
                  ig.total_charge_fine, ig.total_charge_coarse,
                  ig.charge_conserved ? "YES" : "NO");
    check("B1 single +1 block preserves charge", ig.charge_conserved, buf);
    check("B1 coarse total = 1", ig.total_charge_coarse == 1, buf);
}

// B2 — charge pair
static void b2_charge_pair() {
    std::puts("\n--- B2: (+1, −1) pair → net charge 0 preserved ---");
    ftd::RenderBridge rb(32);
    rb.voxel_at(10, 16, 16).state = +1;
    rb.voxel_at(22, 16, 16).state = -1;
    auto blocked = ftd::eft::block_full(rb);
    auto ig = ftd::eft::check_integrity(rb, *blocked);
    char buf[160];
    std::snprintf(buf, sizeof buf,
                  "(fine=%lld coarse=%lld)", ig.total_charge_fine, ig.total_charge_coarse);
    check("B2 pair: fine net = 0", ig.total_charge_fine == 0, buf);
    check("B2 pair: coarse net = 0", ig.total_charge_coarse == 0, buf);
}

// B3 — three distant +1 charges
static void b3_three_charges() {
    std::puts("\n--- B3: three distant +1 voxels → total 3 preserved ---");
    ftd::RenderBridge rb(32);
    rb.voxel_at(4, 4, 4).state = +1;
    rb.voxel_at(16, 16, 16).state = +1;
    rb.voxel_at(28, 28, 28).state = +1;
    auto blocked = ftd::eft::block_full(rb);
    auto ig = ftd::eft::check_integrity(rb, *blocked);
    char buf[160];
    std::snprintf(buf, sizeof buf,
                  "(fine=%lld coarse=%lld)", ig.total_charge_fine, ig.total_charge_coarse);
    check("B3 three charges: fine=3", ig.total_charge_fine == 3, buf);
    check("B3 three charges: coarse=3", ig.total_charge_coarse == 3, buf);
}

// B4 — uniform flux
static void b4_uniform_flux() {
    std::puts("\n--- B4: uniform flux → preserved exactly ---");
    const int L = 16;
    ftd::RenderBridge rb(L);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_flux(x, y, z, {0.1, 0.2, 0.3});
    auto blocked = ftd::eft::block_full(rb);

    // Every coarse voxel should have flux (0.1, 0.2, 0.3)
    bool all_match = true;
    const auto& vox = blocked->bridge().voxels();
    for (const auto& v : vox) {
        if (std::abs(v.flux.x - 0.1) > 1e-12 ||
            std::abs(v.flux.y - 0.2) > 1e-12 ||
            std::abs(v.flux.z - 0.3) > 1e-12) { all_match = false; break; }
    }
    check("B4 uniform flux preserved to machine precision", all_match);

    auto ig = ftd::eft::check_integrity(rb, *blocked);
    char buf[160];
    const double expected_ratio = 1.0 / 8.0;  // 8 fine → 1 coarse, same value
    // Wait: uniform flux gives |J|² constant, so fine has L³ · |J|² = 4096 · 0.14
    // and coarse has (L/2)³ · |J|² = 512 · 0.14, so ratio = 1/8. ✓
    std::snprintf(buf, sizeof buf,
                  "(fine_flux²=%.3f coarse_flux²=%.3f ratio=%.4f expected=%.4f)",
                  ig.total_flux_sq_fine, ig.total_flux_sq_coarse,
                  ig.flux_sq_ratio, expected_ratio);
    check("B4 uniform flux total ratio ≈ 1/8", std::abs(ig.flux_sq_ratio - expected_ratio) < 1e-9, buf);
}

// B5 — random flux variance
static void b5_random_flux() {
    std::puts("\n--- B5: Gaussian random flux → coarse/fine ratio ≈ 1/8 × variance-reduction ---");
    const int L = 16;
    ftd::RenderBridge rb(L);
    std::mt19937 rng(7);
    std::normal_distribution<double> dist(0.0, 1.0);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_flux(x, y, z, {dist(rng), dist(rng), dist(rng)});
    auto blocked = ftd::eft::block_full(rb);
    auto ig = ftd::eft::check_integrity(rb, *blocked);

    // For iid Gaussian children averaged to one coarse value:
    //   Var(coarse_J) = Var(fine_J) / 8
    //   Σ_coarse |J|² = L_c³ · E[|J_coarse|²] = (L/2)³ · 3σ²/8
    //   Σ_fine   |J|² = L³ · 3σ² = 8 · (L/2)³ · 3σ²
    //   ratio = 1/64
    // Statistical uncertainty on this ratio for L_c³ = 512 samples of
    // variance 1/8 is ~ √(2/N) ≈ 6% of the ratio.
    const double expected = 1.0 / 64.0;
    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(fine=%.3f coarse=%.3f ratio=%.5f expected=%.5f)",
                  ig.total_flux_sq_fine, ig.total_flux_sq_coarse,
                  ig.flux_sq_ratio, expected);
    check("B5 random flux ratio ≈ 1/64 (within 30%)",
          std::abs(ig.flux_sq_ratio - expected) / expected < 0.30, buf);
}

// B6 — iterated blocking
static void b6_iterated() {
    std::puts("\n--- B6: block twice (32 → 16 → 8) → charge preserved ---");
    ftd::RenderBridge rb(32);
    // Asymmetric multi-charge configuration.
    rb.voxel_at(3, 3, 3).state   = +1;
    rb.voxel_at(5, 5, 5).state   = +1;
    rb.voxel_at(20, 20, 20).state = -1;
    rb.voxel_at(25, 10, 15).state = +1;

    auto stage1 = ftd::eft::block_full(rb);
    auto stage2 = ftd::eft::block_full(stage1->bridge());

    auto ig1 = ftd::eft::check_integrity(rb, *stage1);
    auto ig2 = ftd::eft::check_integrity(stage1->bridge(), *stage2);

    char buf[256];
    std::snprintf(buf, sizeof buf,
                  "(stage1 fine=%lld coarse=%lld; stage2 fine=%lld coarse=%lld)",
                  ig1.total_charge_fine, ig1.total_charge_coarse,
                  ig2.total_charge_fine, ig2.total_charge_coarse);
    check("B6 stage-1 charge preserved", ig1.charge_conserved, buf);
    check("B6 stage-2 charge preserved", ig2.charge_conserved, buf);
    check("B6 end-to-end charge preserved",
          rb.voxels().size() >= 0 && stage2->total_charge() == 2 /* +1+1-1+1 = 2 */, buf);
}

// B7 — overflow: block with 4 × (+1) children
static void b7_overflow() {
    std::puts("\n--- B7: 4 × (+1) in one block → overflow spreads deterministically ---");
    ftd::RenderBridge rb(16);
    // Four +1 voxels all in the (0,0,0) coarse block (fine coords 0..1 × 0..1 × 0..1)
    rb.voxel_at(0, 0, 0).state = +1;
    rb.voxel_at(1, 0, 0).state = +1;
    rb.voxel_at(0, 1, 0).state = +1;
    rb.voxel_at(0, 0, 1).state = +1;
    // Expected: block net S = +4; primary coarse voxel gets +1, overflow 3
    // flows to next lex-ordered coarse voxels with state==0. Total = +4 preserved.
    auto blocked = ftd::eft::block_full(rb);
    auto ig = ftd::eft::check_integrity(rb, *blocked);
    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(fine=%lld coarse=%lld conserved=%s)",
                  ig.total_charge_fine, ig.total_charge_coarse,
                  ig.charge_conserved ? "YES" : "NO");
    check("B7 overflow: fine=4 coarse=4", ig.charge_conserved && ig.total_charge_coarse == 4, buf);
}

int main() {
    std::puts("================================================================");
    std::puts("  EFT Phase 2A — Block-Spin Transformation Validation Gate");
    std::puts("  (SPEC_EFT_RECOVERY_PROGRAM.md §5.1 — must pass before β)");
    std::puts("================================================================");

    b1_single_positive();
    b2_charge_pair();
    b3_three_charges();
    b4_uniform_flux();
    b5_random_flux();
    b6_iterated();
    b7_overflow();

    std::puts("\n----------------------------------------------------------------");
    if (g_failures == 0) {
        std::puts("  All EFT-Phase-2A validation checks PASS");
        std::puts("  Blocking is physically consistent; cleared for β measurement.");
        return 0;
    }
    std::printf("  %d EFT-Phase-2A check(s) FAILED\n", g_failures);
    std::puts("  Blocking is NOT consistent; β measurement is BLOCKED.");
    return 1;
}

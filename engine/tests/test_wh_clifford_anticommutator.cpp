/**
 * @file test_wh_clifford_anticommutator.cpp
 * @brief Measure the anticommutator of the engine-induced product on the three
 *        weight-1 Walsh–Hadamard modes of a 2^3 block.
 *
 * Context:
 *   Conjecture (PDF draft §1, 2026-04-24): the 1+3+3+1 Walsh–Hadamard
 *   decomposition of a 2^3 FTD block instantiates Cl(3,0), with flux
 *   transitions giving Dirac γ-matrices.
 *
 *   Structural no-go (DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md, 2026-04-24):
 *   the WH pointwise product is the abelian group algebra ℝ[(ℤ/2)^3] ≅ ℝ^8,
 *   NOT isomorphic to Cl(3,0) ≅ M_2(ℝ) ⊕ M_2(ℝ). The spontaneous version is
 *   closed. The dynamical version — that non-linear engine ingredients
 *   (ternary clamp + movement) induce a Clifford product on the 8-dim WH
 *   space — is OPEN.
 *
 * Protocol:
 *   For each ordered pair (f, g) of the three weight-1 WH modes:
 *     1. Fresh RenderBridge, non-linear ingredients enabled (genesis + movement)
 *     2. Inject flux ∝ χ_f along the matching axis on a 2^3 corner block
 *     3. Run 1 tick
 *     4. Inject flux ∝ χ_g
 *     5. Run 1 tick
 *     6. Read state on the 2^3 block, WH-decompose
 *   Form anticommutator T_{fg} + T_{gf}.
 *   Clifford criterion: {e_i, e_j} = 2 δ_{ij} · 1  (coefficient on v=000 only).
 *
 * Outcome:
 *   - PASS = all 9 anticommutators within tolerance of Clifford structure
 *           → fermion emergence conjecture upgraded to THEOREM.
 *   - FAIL = anticommutators have structure but not Clifford
 *           → conjecture falsified on the 2^3 block; seek origin elsewhere
 *             (e.g. Moore-26 decomposition SC + FCC + BCC).
 *
 *   This test is a measurement. It returns 0 regardless of the Clifford
 *   verdict so long as the protocol ran cleanly; the verdict is reported
 *   in stdout for inspection.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>

#include "ftd/render_bridge.h"

// ---------------- Walsh–Hadamard helpers ----------------

// χ_v(x,y,z) = (-1)^{v_1 x + v_2 y + v_3 z} on the 2^3 block.
// v is a 3-bit mask: bit0=v_1 (x), bit1=v_2 (y), bit2=v_3 (z).
static inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

// Block coordinates: 2^3 = 8 sites at (x,y,z) ∈ {0,1}^3.
// Normalized inner product  <f, χ_v>  over the block.
static double wh_coef_on_block(const std::vector<ftd::Voxel>& vox,
                               int L, int v_mask) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        sum += static_cast<double>(vox[i].state) * static_cast<double>(chi(v_mask, x, y, z));
    }
    return sum / 8.0;
}

// ---------------- Injection ----------------

// Inject flux ∝ A·χ_v on the 2^3 corner block, aligned along |axis| ∈ {0,1,2}.
static void inject_wh_mode(ftd::RenderBridge& rb, int v_mask, int axis, double A) {
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const double s = static_cast<double>(chi(v_mask, x, y, z));
        ftd::Vec3 dF{0, 0, 0};
        if (axis == 0) dF.x = A * s;
        if (axis == 1) dF.y = A * s;
        if (axis == 2) dF.z = A * s;
        rb.inject_flux_add(x, y, z, dF);
    }
}

// ---------------- Main protocol ----------------

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // Lattice: L = 8 gives room around the 2^3 block so propagation doesn't
    // self-wrap before readout. 8^3 = 512 voxels, trivial on GPU.
    const int L = 8;

    // Three weight-1 WH modes (Hamming weight 1 in 3-bit mask).
    // Mode 0: v=001 (only x bit) → axis 0
    // Mode 1: v=010 (only y bit) → axis 1
    // Mode 2: v=100 (only z bit) → axis 2
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    const std::array<const char*, 3> w1_name = {"e_1 (χ_100)", "e_2 (χ_010)", "e_3 (χ_001)"};

    // Amplitude: genesis fires when |J| > 3·K_B ≈ 1.53 with probability
    //   p = 1 - exp(-(|J| - 3·K_B)/K_B).
    // At A=3, p ≈ 0.94 (stochastic miss ~6% — contaminates the signal).
    // At A=10, p ≈ 1 − 4×10⁻⁸ (effectively deterministic).
    // We use A=10 so the order-dependence we measure is non-stochastic.
    const double A = 10.0;

    // T[f][g][v] = WH coefficient at mode v after applying mode f then g.
    std::array<std::array<std::array<double, 8>, 3>, 3> T{};

    std::printf("================================================================\n");
    std::printf("  Walsh–Hadamard / Clifford anticommutator measurement\n");
    std::printf("================================================================\n");
    std::printf("  Lattice L=%d, block 2^3 at origin, injection amplitude A=%.2f\n",
                L, A);
    std::printf("  Toggles: wave_propagation + gauss_projection + genesis + movement\n");
    std::printf("  (genesis + movement = the two non-linear engine ingredients)\n\n");

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);

        // Minimal single-substrate ternary dynamics with the two non-linearities.
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling         = false;
        rb.toggles.damping          = false;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.forces           = false;
        rb.toggles.gravity          = false;
        rb.toggles.poisson_coulomb  = false;
        rb.toggles.movement         = true;
        rb.toggles.lorentz_force    = false;
        rb.toggles.selective_damping= false;
        rb.toggles.larmor_radiation = false;
        rb.toggles.dual_substrate   = false;
        rb.toggles.weak_transmutation = false;
        rb.toggles.latency_field    = false;
        rb.toggles.langevin         = false;
        // Seed by unordered pair {fi,gi}, NOT by ordered pair. This way the
        // two orderings (f→g) and (g→f) share the same random stream, so any
        // anticommutator asymmetry reflects dynamical non-commutativity and
        // not RNG differences.
        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0xC14F01D + 100 * lo + hi);

        // Inject mode f, run 1 tick
        inject_wh_mode(rb, w1_mask[fi], fi, A);
        rb.run(1);

        // Inject mode g, run 1 tick
        inject_wh_mode(rb, w1_mask[gi], gi, A);
        rb.run(1);

        // WH-decompose the resulting state on the 2^3 block
        const auto& vox = rb.voxels();
        for (int v = 0; v < 8; ++v) {
            T[fi][gi][v] = wh_coef_on_block(vox, L, v);
        }
    }

    // Anticommutator matrix {e_f, e_g} := T[f][g] + T[g][f]
    std::printf("--- Anticommutator {e_f, e_g} projected onto the 8 WH basis modes ---\n");
    std::printf("  basis index: v=0 (000 ident)  v=1 (001 x)  v=2 (010 y)  v=3 (011 xy)\n");
    std::printf("               v=4 (100 z)      v=5 (101 xz) v=6 (110 yz) v=7 (111 xyz)\n\n");
    std::printf("  pair    | ident  |   x    |   y    |  xy    |   z    |  xz    |  yz    |  xyz   \n");
    std::printf("  --------+--------+--------+--------+--------+--------+--------+--------+--------\n");
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        std::printf("  {%d,%d}   |", fi + 1, gi + 1);
        for (int v = 0; v < 8; ++v) {
            const double ac = T[fi][gi][v] + T[gi][fi][v];
            std::printf(" %+6.3f |", ac);
        }
        std::printf("\n");
    }
    std::printf("\n");

    // Clifford check: {e_i, e_j} = 2 δ_{ij} · e_0
    //   (i,j) = (i,i) → coefficient on v=0 equals 2, all other v equal 0
    //   (i,j) ≠      → all 8 coefficients equal 0
    const double tol = 0.2;  // tolerance in WH-coefficient units
    int pairs_ok = 0;
    std::printf("--- Clifford verdict (tol=%.2f) ---\n", tol);
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        const double ac_ident = T[fi][gi][0] + T[gi][fi][0];
        double ac_other = 0.0;
        for (int v = 1; v < 8; ++v) {
            ac_other += std::abs(T[fi][gi][v] + T[gi][fi][v]);
        }
        const double expected_ident = (fi == gi) ? 2.0 : 0.0;
        const bool ident_ok = std::abs(ac_ident - expected_ident) < tol;
        const bool other_ok = ac_other < tol * 7;
        const bool pair_ok  = ident_ok && other_ok;
        if (pair_ok) ++pairs_ok;
        std::printf("  {%d,%d}: ident=%+6.3f (exp %+.1f) %s ; |others|=%5.3f %s → %s\n",
                    fi + 1, gi + 1, ac_ident, expected_ident,
                    ident_ok ? "✓" : "✗",
                    ac_other,
                    other_ok ? "✓" : "✗",
                    pair_ok ? "CLIFFORD-CONSISTENT" : "NOT CLIFFORD");
    }

    std::printf("\n  Clifford-consistent pairs: %d / 6 (3 diag + 3 off-diag)\n", pairs_ok);

    std::printf("\n================================================================\n");
    if (pairs_ok == 6) {
        std::printf("  RESULT: engine-induced product on the 2^3 block satisfies\n");
        std::printf("          Clifford anticommutation {e_i,e_j} = 2δ_{ij}·1 within\n");
        std::printf("          tolerance %.2f. Fermion-emergence conjecture UPGRADED.\n", tol);
    } else {
        std::printf("  RESULT: engine-induced product on the 2^3 block does NOT satisfy\n");
        std::printf("          Clifford anticommutation within tolerance %.2f.\n", tol);
        std::printf("          Fermion emergence from the b=2 block is FALSIFIED at\n");
        std::printf("          this order. Seek origin in a different structure\n");
        std::printf("          (e.g. Moore-26 SC+FCC+BCC layer decomposition).\n");
    }
    std::printf("================================================================\n");

    // Return 0 regardless — this is a measurement, not a pass/fail assertion.
    return 0;
}

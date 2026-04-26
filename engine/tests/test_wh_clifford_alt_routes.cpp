/**
 * @file test_wh_clifford_alt_routes.cpp
 * @brief Phase-4 fermion-emergence alt-route measurements (FTD-0061 extension).
 *
 * The original test_wh_clifford_anticommutator established that genesis +
 * movement collapse every weight-1 WH mode to a uniform scalar state, giving
 * {e_i, e_j} = 2 · 1 for all (i,j) (not Clifford). The present test varies
 * the non-linear ingredient to see whether any alternative route preserves
 * mode distinction long enough for the anticommutator to become
 * 2 · δ_{ij} · 1 (Clifford) or some other structured algebra.
 *
 * Routes tested:
 *   4a. Pair production only (no genesis)     — local ±1 pair creation
 *   4b. Weak transmutation on dual substrate  — polarity flip under stress
 *   4d. Velocity-driven movement + forces     — transport + annihilation
 *
 * Route 4c (Moore-26 grade structure at 3³ block) requires a different
 * basis (Z/3 Fourier, not Walsh-Hadamard) and is filed separately.
 *
 * Each route replaces the genesis+movement pair of the original test with
 * its alternative non-linearity, using the SAME injection protocol:
 *   inject mode f on axis fi → run 1 tick → inject mode g on axis gi → run 1 tick
 *   → WH-decompose the state field on the 2³ corner block.
 *
 * Clifford criterion: {e_f, e_g} = 2 δ_{fg} · 1
 *   (diagonal: +2 on ident coef, 0 elsewhere; off-diagonal: all 0)
 *
 * PASS = 6/6 pairs Clifford-consistent
 * FAIL = at least one pair violates Clifford (expected outcome for 4a, 4b, 4d)
 *
 * This is a measurement, not a unit test: return 0 regardless of verdict.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <string>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

// ---------- WH helpers (identical to the original test) ----------

static inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

static double wh_coef_on_block(const std::vector<ftd::Voxel>& vox,
                               int L, int v_mask) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        sum += static_cast<double>(vox[i].state)
             * static_cast<double>(chi(v_mask, x, y, z));
    }
    return sum / 8.0;
}

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

// ---------- Common protocol per route ----------

struct RouteResult {
    std::string name;
    int clifford_pairs = 0;
    std::array<std::array<std::array<double, 8>, 3>, 3> T{};
};

static RouteResult run_route(const std::string& route_name,
                             void (*configure)(ftd::RenderBridge&),
                             double amplitude,
                             bool preseed_state) {
    const int L = 8;
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};

    RouteResult result;
    result.name = route_name;

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);
        configure(rb);

        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0xA17E01D + 100 * lo + hi);

        // Optional pre-seeding: give non-linearities a state field to act on.
        // Routes 4a (pair) and 4b (weak) otherwise have nothing to flip/pair.
        if (preseed_state) {
            // Seed every block site with +1 (mode 0 injection) so subsequent
            // mode f/g injections land on an existing state field.
            for (int x = 0; x < 2; ++x)
            for (int y = 0; y < 2; ++y)
            for (int z = 0; z < 2; ++z) {
                rb.inject_particle(x, y, z, +1, {0, 0, 0});
            }
        }

        inject_wh_mode(rb, w1_mask[fi], fi, amplitude);
        rb.run(1);
        inject_wh_mode(rb, w1_mask[gi], gi, amplitude);
        rb.run(1);

        const auto& vox = rb.voxels();
        for (int v = 0; v < 8; ++v) {
            result.T[fi][gi][v] = wh_coef_on_block(vox, L, v);
        }
    }

    // Clifford check
    const double tol = 0.2;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        const double ac_ident = result.T[fi][gi][0] + result.T[gi][fi][0];
        double ac_other = 0.0;
        for (int v = 1; v < 8; ++v) {
            ac_other += std::abs(result.T[fi][gi][v] + result.T[gi][fi][v]);
        }
        const double expected_ident = (fi == gi) ? 2.0 : 0.0;
        const bool ident_ok = std::abs(ac_ident - expected_ident) < tol;
        const bool other_ok = ac_other < tol * 7;
        if (ident_ok && other_ok) ++result.clifford_pairs;
    }
    return result;
}

static void print_route(const RouteResult& r) {
    std::printf("\n================================================================\n");
    std::printf("  ROUTE: %s\n", r.name.c_str());
    std::printf("================================================================\n");
    std::printf("  pair    | ident  |   x    |   y    |  xy    |   z    |  xz    |  yz    |  xyz   \n");
    std::printf("  --------+--------+--------+--------+--------+--------+--------+--------+--------\n");
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        std::printf("  {%d,%d}   |", fi + 1, gi + 1);
        for (int v = 0; v < 8; ++v) {
            const double ac = r.T[fi][gi][v] + r.T[gi][fi][v];
            std::printf(" %+6.3f |", ac);
        }
        std::printf("\n");
    }
    std::printf("  Clifford-consistent pairs: %d / 6\n", r.clifford_pairs);
    if (r.clifford_pairs == 6) {
        std::printf("  → CLIFFORD EMERGENCE UPGRADED from this route\n");
    } else {
        std::printf("  → Clifford FALSIFIED on this route\n");
    }
}

// ---------- Route configurations ----------

static void configure_4a_pair_production(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.pair_production  = true;
    // No genesis, no movement, no forces
}

static void configure_4b_weak_transmutation(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation   = true;
    rb.toggles.gauss_projection   = true;
    rb.toggles.weak_transmutation = true;
    rb.toggles.dual_substrate     = true;  // required by weak
    // Seed some manifested state for weak to flip (otherwise no-op)
    // Done by caller via inject_wh_mode which only seeds flux;
    // we add a particle at centre to give weak something to flip.
}

static void configure_4d_velocity_driven(ftd::RenderBridge& rb) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.forces           = true;
    rb.toggles.poisson_coulomb  = true;
    rb.toggles.movement         = true;
    rb.toggles.genesis          = true;  // need state for forces to act on
}

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Phase-4 WH/Clifford Alt-Route Measurements\n");
    std::printf("  (FTD-0061 extension — do any non-genesis route give Clifford?)\n");
    std::printf("================================================================\n");

    // Large amplitude — drives all routes into saturation / deterministic regime.
    const double A = 10.0;

    // 4a and 4b need pre-seeded state (pair_production and weak_transmutation
    // are no-ops on void blocks). 4d already seeds state via genesis.
    const auto r4a = run_route("4a. Pair production only (pre-seeded state)",
                               configure_4a_pair_production, A, /*preseed=*/true);
    print_route(r4a);

    const auto r4b = run_route("4b. Weak transmutation (dual, pre-seeded state)",
                               configure_4b_weak_transmutation, A, /*preseed=*/true);
    print_route(r4b);

    const auto r4d = run_route("4d. Velocity-driven movement + forces + genesis",
                               configure_4d_velocity_driven, A, /*preseed=*/false);
    print_route(r4d);

    // Summary
    std::printf("\n================================================================\n");
    std::printf("  PHASE-4 SUMMARY: Clifford-consistent pair counts (out of 6)\n");
    std::printf("================================================================\n");
    std::printf("    Original (genesis+movement, FTD-0061):  3 / 6  FALSIFIED\n");
    std::printf("    4a Pair production only:                %d / 6  %s\n",
                r4a.clifford_pairs,
                r4a.clifford_pairs == 6 ? "PASS" : "FALSIFIED");
    std::printf("    4b Weak transmutation (dual):           %d / 6  %s\n",
                r4b.clifford_pairs,
                r4b.clifford_pairs == 6 ? "PASS" : "FALSIFIED");
    std::printf("    4d Velocity+forces+genesis:             %d / 6  %s\n",
                r4d.clifford_pairs,
                r4d.clifford_pairs == 6 ? "PASS" : "FALSIFIED");

    const int any_clifford = (r4a.clifford_pairs == 6)
                           || (r4b.clifford_pairs == 6)
                           || (r4d.clifford_pairs == 6);

    std::printf("\n  VERDICT: ");
    if (any_clifford) {
        std::printf("At least one route produces Clifford anticommutation.\n");
        std::printf("  Fermion emergence from the 2³ block is UPGRADED via that route.\n");
    } else {
        std::printf("NO route tested produces Clifford {e_i,e_j} = 2δ_{ij}·1.\n");
        std::printf("  Fermion emergence from the 2³ block is FURTHER FALSIFIED under\n");
        std::printf("  all four non-linearity choices (genesis, pair, weak, velocity+).\n");
        std::printf("  Seek fermion origin in a structure OTHER than the 2³ block.\n");
    }
    std::printf("================================================================\n");

    return 0;  // measurement, not a pass/fail
}

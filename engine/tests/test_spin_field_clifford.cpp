/**
 * @file test_spin_field_clifford.cpp
 * @brief Phase-4e: fermion-emergence test on the SPIN field, not the state field.
 *
 * The universal collapse across FTD-0061, 0071, 0072 was driven by the
 * state field being a 0-form that gets set to a uniform sign(div J) value
 * under every tested dynamics. The spin field, by contrast, is assigned
 * from the dominant CURL component — a different geometric signal.
 *
 * Hypothesis: injections with non-trivial curl structure may produce
 * spin-field patterns that preserve WH mode information, allowing
 * non-trivial anticommutator structure.
 *
 * Protocol differs from FTD-0061 in two ways:
 *   1. Injections are OFF-AXIS: mode along x-bit injected into flux_y (not
 *      flux_x), so ∂_x(flux_y) ≠ 0 and curl_z is generated.
 *   2. Read the spin field, not the state field, and WH-decompose.
 *
 * Same Clifford criterion: {e_i, e_j} = 2 δ_ij · 1.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

static inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

// Read the spin field and WH-decompose on the 2³ block.
static double wh_coef_of_spin(const std::vector<ftd::Voxel>& vox,
                              int L, int v_mask) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        sum += static_cast<double>(vox[i].spin)
             * static_cast<double>(chi(v_mask, x, y, z));
    }
    return sum / 8.0;
}

// Off-axis injection: mode v_mask's spatial pattern, but stored in the
// flux component ORTHOGONAL to its natural axis. This creates a non-zero
// curl that drives spin assignment during genesis.
static void inject_wh_mode_offaxis(ftd::RenderBridge& rb,
                                   int v_mask,
                                   int storage_axis,
                                   double A) {
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const double s = static_cast<double>(chi(v_mask, x, y, z));
        ftd::Vec3 dF{0, 0, 0};
        if (storage_axis == 0) dF.x = A * s;
        if (storage_axis == 1) dF.y = A * s;
        if (storage_axis == 2) dF.z = A * s;
        rb.inject_flux_add(x, y, z, dF);
    }
}

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Phase-4e: Spin-field WH/Clifford Anticommutator Test\n");
    std::printf("================================================================\n");
    std::printf("  Modes injected OFF-AXIS (mode-bit i, stored in axis (i+1) mod 3)\n");
    std::printf("  to produce non-zero curl → non-zero spin assignment.\n");
    std::printf("  Anticommutator measured on the SPIN field, not the state field.\n\n");

    const int L = 8;
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    const double A = 10.0;

    std::array<std::array<std::array<double, 8>, 3>, 3> T{};

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis          = true;
        rb.toggles.movement         = true;
        rb.toggles.dual_substrate   = false;

        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0x571F0001 + 100 * lo + hi);

        // Mode f stored on axis (f+1) mod 3 — creates curl perpendicular to f.
        inject_wh_mode_offaxis(rb, w1_mask[fi], (fi + 1) % 3, A);
        rb.run(1);
        inject_wh_mode_offaxis(rb, w1_mask[gi], (gi + 1) % 3, A);
        rb.run(1);

        const auto& vox = rb.voxels();
        for (int v = 0; v < 8; ++v) {
            T[fi][gi][v] = wh_coef_of_spin(vox, L, v);
        }
    }

    std::printf("--- Anticommutator {e_f, e_g} on spin-field WH modes ---\n");
    std::printf("  pair    | ident  |   x    |   y    |  xy    |   z    |  xz    |  yz    |  xyz   \n");
    std::printf("  --------+--------+--------+--------+--------+--------+--------+--------+--------\n");
    int clifford_pairs = 0;
    const double tol = 0.2;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        std::printf("  {%d,%d}   |", fi + 1, gi + 1);
        for (int v = 0; v < 8; ++v) {
            const double ac = T[fi][gi][v] + T[gi][fi][v];
            std::printf(" %+6.3f |", ac);
        }
        std::printf("\n");
        const double ac_ident = T[fi][gi][0] + T[gi][fi][0];
        double ac_other = 0.0;
        for (int v = 1; v < 8; ++v) {
            ac_other += std::abs(T[fi][gi][v] + T[gi][fi][v]);
        }
        const double expected_ident = (fi == gi) ? 2.0 : 0.0;
        const bool ident_ok = std::abs(ac_ident - expected_ident) < tol;
        const bool other_ok = ac_other < tol * 7;
        if (ident_ok && other_ok) ++clifford_pairs;
    }

    // Also report the full T matrix to see whether there's *any* structure
    // even if it's not Clifford.
    std::printf("\n--- Full T[f][g] table (spin-field readout) ---\n");
    std::printf("  (looking for non-zero, mode-distinguishing structure)\n");
    for (int fi = 0; fi < 3; ++fi) {
        for (int gi = 0; gi < 3; ++gi) {
            double norm2 = 0;
            for (int v = 0; v < 8; ++v) norm2 += T[fi][gi][v] * T[fi][gi][v];
            std::printf("  T[%d,%d]: ||T||=%.3f", fi + 1, gi + 1, std::sqrt(norm2));
        }
        std::printf("\n");
    }

    std::printf("\n================================================================\n");
    std::printf("  Clifford-consistent pairs (spin field): %d / 6\n", clifford_pairs);
    if (clifford_pairs == 6) {
        std::printf("  RESULT: Spin-field readout SUPPORTS Clifford anticommutation.\n");
        std::printf("  Fermion emergence UPGRADED via spin-field probe.\n");
    } else {
        std::printf("  RESULT: Spin-field readout does NOT support Clifford.\n");
        std::printf("  Together with FTD-0061, 0071, 0072: fermion emergence from\n");
        std::printf("  single-tick site-local probes is broadly falsified on finite\n");
        std::printf("  blocks. Seek structure in edge/worldline/propagator constructions.\n");
    }
    std::printf("================================================================\n");

    return 0;
}

/**
 * @file test_flux_link_clifford.cpp
 * @brief Phase-4f: fermion-emergence test on the FLUX 1-form (link-like
 *        degrees of freedom), not the state 0-form.
 *
 * Following the mode-erasure theorem (DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md)
 * which proves site-local state-field readout cannot support Clifford, this
 * test probes the flux field, which is a 1-form (vector) living on sites with
 * three components.
 *
 * The natural "link" operator for axis i is
 *   L_i := WH_coefficient of χ_{weight-1 along axis i} on flux component i.
 * This is the "how much does flux_i look like its corresponding weight-1
 * WH mode pattern" — the 1-form analog of the weight-1 WH state readout.
 *
 * Protocol (same as FTD-0061):
 *   inject mode f on axis f → tick → inject mode g on axis g → tick → read
 *   all three flux components → WH-decompose each → 24 coefs total.
 *
 * Report:
 *   - The diagonal L_i after 2 injections: {L_f,L_f} value
 *   - The off-diagonal {L_f, L_g} for f ≠ g
 *   - Clifford: {L_i, L_j} = 2 δ_ij · (identity 1-form, e.g. χ_{000} on flux_i)
 *
 * If this has non-trivial Clifford structure: fermion emergence candidate
 * upgraded to the flux 1-form, not state 0-form.
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

// WH coefficient of a scalar field f(x,y,z) on the 2³ block.
// Here f is one component of flux (flux_x, flux_y, or flux_z).
static double wh_coef_flux_component(const std::vector<ftd::Voxel>& vox,
                                     int L, int component, int v_mask) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        double fc = 0.0;
        if (component == 0) fc = vox[i].flux.x;
        if (component == 1) fc = vox[i].flux.y;
        if (component == 2) fc = vox[i].flux.z;
        sum += fc * static_cast<double>(chi(v_mask, x, y, z));
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

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Phase-4f: Flux 1-form (link) WH/Clifford Anticommutator Test\n");
    std::printf("================================================================\n");
    std::printf("  Read flux_x, flux_y, flux_z separately after 2-injection protocol.\n");
    std::printf("  Measure WH decomposition of each component (24 coefs total).\n");
    std::printf("  Test anticommutator on axial link operators L_i = WH[flux_i, χ_{axis_i}].\n\n");

    const int L = 8;
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    const double A = 10.0;

    // T[f][g][axis][v]: WH coefficient of flux_axis on mode v after
    // sequential injection of mode f then mode g.
    std::array<std::array<std::array<std::array<double, 8>, 3>, 3>, 3> T{};

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
        rb.seed_rng(0xF10517D0 + 100 * lo + hi);

        inject_wh_mode(rb, w1_mask[fi], fi, A);
        rb.run(1);
        inject_wh_mode(rb, w1_mask[gi], gi, A);
        rb.run(1);

        const auto& vox = rb.voxels();
        for (int axis = 0; axis < 3; ++axis)
        for (int v = 0; v < 8; ++v) {
            T[fi][gi][axis][v] = wh_coef_flux_component(vox, L, axis, v);
        }
    }

    // The link operator L_i is the WH coefficient on flux_i at its natural
    // weight-1 mode χ_{2^i}. Clifford anticommutation on these 3:
    std::printf("--- Link operator anticommutator on natural axial 1-form modes ---\n");
    std::printf("    L_i := WH_flux_i[χ_{natural weight-1 mode along axis i}]\n");
    std::printf("    Clifford expects: {L_i, L_j} = 2 δ_ij · 1_flux\n\n");
    std::printf("  pair    | {L_f, L_g} (diagonal of 3-axis block)\n");
    std::printf("  --------+------------------------------------------\n");
    int clifford_pairs = 0;
    const double tol = 0.5;  // looser tolerance for continuous flux field
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        // {L_f, L_g}_{axis=i} = T[f][g][i][χ_i] + T[g][f][i][χ_i]
        double ac_diag[3];
        double ac_off_sum = 0.0;
        for (int axis = 0; axis < 3; ++axis) {
            const int nat_mode = w1_mask[axis];
            ac_diag[axis] = T[fi][gi][axis][nat_mode] + T[gi][fi][axis][nat_mode];
            // "off-diagonal axes" = how much does flux_i have WH mass on a mode
            // other than its natural one?
            for (int v = 0; v < 8; ++v) {
                if (v == nat_mode) continue;
                ac_off_sum += std::abs(T[fi][gi][axis][v] + T[gi][fi][axis][v]);
            }
        }
        std::printf("  {%d,%d}   | L_x=%+7.3f  L_y=%+7.3f  L_z=%+7.3f   |off|=%.3f\n",
                    fi + 1, gi + 1, ac_diag[0], ac_diag[1], ac_diag[2], ac_off_sum);

        // Clifford check: for i==j, exactly one L_i should be +2 * (natural amp).
        // For i!=j, all L_i should be ≈ 0.
        if (fi == gi) {
            const bool active_matches = std::abs(ac_diag[fi]) > 1.0
                                      && std::abs(ac_diag[(fi + 1) % 3]) < tol
                                      && std::abs(ac_diag[(fi + 2) % 3]) < tol;
            if (active_matches) ++clifford_pairs;
        } else {
            const bool all_small = std::abs(ac_diag[0]) < tol
                                 && std::abs(ac_diag[1]) < tol
                                 && std::abs(ac_diag[2]) < tol;
            if (all_small) ++clifford_pairs;
        }
    }

    std::printf("\n  Clifford-consistent pairs: %d / 6\n", clifford_pairs);

    // Also report full WH spectrum of each flux component after each injection
    // pair, to see what the flux 1-form looks like structurally.
    std::printf("\n--- Sample: full flux WH spectrum after (f=1, g=2) ---\n");
    std::printf("    (mode-distinguishing structure in flux 1-form?)\n");
    for (int axis = 0; axis < 3; ++axis) {
        std::printf("  flux_%c: ", "xyz"[axis]);
        for (int v = 0; v < 8; ++v) {
            std::printf(" %+6.2f", T[0][1][axis][v]);
        }
        std::printf("\n");
    }

    std::printf("\n================================================================\n");
    if (clifford_pairs == 6) {
        std::printf("  RESULT: Flux 1-form (link) SUPPORTS Clifford anticommutation.\n");
        std::printf("  Fermion emergence UPGRADED via flux 1-form probe.\n");
    } else {
        std::printf("  RESULT: Flux 1-form does NOT support Clifford on axial modes\n");
        std::printf("  under genesis + movement. Pairs consistent: %d/6.\n",
                    clifford_pairs);
        std::printf("  Non-trivial flux WH structure is present (see spectrum above)\n");
        std::printf("  but not in Clifford form. Next: propagator Dirac-fit test\n");
        std::printf("  on Langevin ensemble (Phase-4 followup / option 2).\n");
    }
    std::printf("================================================================\n");
    return 0;
}

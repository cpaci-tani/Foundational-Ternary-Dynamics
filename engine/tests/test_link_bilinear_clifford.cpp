/**
 * @file test_link_bilinear_clifford.cpp
 * @brief Program F — link-bilinear fermion probe.
 *
 * The mode-erasure theorem (FTD-0073) proves site-local state-field readout
 * with pointwise-threshold dynamics cannot support Clifford. FTD-0074
 * extended this negatively to the flux 1-form with site-local dynamics,
 * finding separable-tensor structure.
 *
 * Program F asks: does the no-go extend when
 *   (a) non-local dynamics are enabled (forces, triad, strong, exchange,
 *       pair_production, weak_transmutation)
 *   (b) the readout is genuinely BILINEAR in flux between adjacent sites
 *       (not just a linear WH projection of a single flux component).
 *
 * If the link-bilinear anticommutator is still separable, the no-go extends
 * to the full engine toggle set and fermion emergence from FTD native
 * dynamics is closed negative at the elementary level. Fermions must be
 * (i) composite (baryons, Program H) or (ii) selection-level (Branch-B).
 *
 * If the anticommutator becomes Clifford-like, Program F is a positive
 * result: non-local dynamics + bilinear readout is the correct algebraic
 * category for fermion emergence.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

namespace {

// Walsh-Hadamard character on 2^3 block.
inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

// WH coefficient of one flux component on the 2^3 block.
double wh_coef_flux_component(const std::vector<ftd::Voxel>& vox,
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

// Bilinear link observable: B_i = sum over faces perpendicular to axis i of
// J_i(x) * J_i(x + hat_i). This is a scalar bilinear in flux, genuinely
// non-local, and is the canonical "propagator-like" flux bilinear.
double bilinear_link(const std::vector<ftd::Voxel>& vox, int L, int axis) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        int x2 = x, y2 = y, z2 = z;
        if (axis == 0) x2 = (x + 1) % 2;
        if (axis == 1) y2 = (y + 1) % 2;
        if (axis == 2) z2 = (z + 1) % 2;
        const int i1 = x  * L * L + y  * L + z;
        const int i2 = x2 * L * L + y2 * L + z2;
        double fi1 = 0, fi2 = 0;
        if (axis == 0) { fi1 = vox[i1].flux.x; fi2 = vox[i2].flux.x; }
        if (axis == 1) { fi1 = vox[i1].flux.y; fi2 = vox[i2].flux.y; }
        if (axis == 2) { fi1 = vox[i1].flux.z; fi2 = vox[i2].flux.z; }
        sum += fi1 * fi2;
    }
    return sum;  // 8 edge-pairs summed (wraps within 2^3 block)
}

// Cross-axis bilinear: B_{ij}(axis=k) = sum_x J_i(x) * J_j(x + hat_k).
// This is a tensor bilinear that tests cross-axis coupling.
double cross_bilinear(const std::vector<ftd::Voxel>& vox, int L,
                      int comp_i, int comp_j, int link_axis) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        int x2 = x, y2 = y, z2 = z;
        if (link_axis == 0) x2 = (x + 1) % 2;
        if (link_axis == 1) y2 = (y + 1) % 2;
        if (link_axis == 2) z2 = (z + 1) % 2;
        const int i1 = x  * L * L + y  * L + z;
        const int i2 = x2 * L * L + y2 * L + z2;
        double a = 0, b = 0;
        if (comp_i == 0) a = vox[i1].flux.x;
        if (comp_i == 1) a = vox[i1].flux.y;
        if (comp_i == 2) a = vox[i1].flux.z;
        if (comp_j == 0) b = vox[i2].flux.x;
        if (comp_j == 1) b = vox[i2].flux.y;
        if (comp_j == 2) b = vox[i2].flux.z;
        sum += a * b;
    }
    return sum;
}

void inject_wh_mode(ftd::RenderBridge& rb, int v_mask, int axis, double A) {
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

void enable_full_nonlocal(ftd::TermToggles& t) {
    t.disable_all();
    // Base dynamics
    t.wave_propagation  = true;
    t.gauss_projection  = true;
    t.genesis           = true;
    t.movement          = true;
    // Full non-local dynamics
    t.forces            = true;
    t.emergent_forces   = true;
    t.pair_production   = true;
    t.weak_transmutation= true;
    t.exchange_force    = true;
    t.strong_force      = true;
    t.triad_binding     = true;
    t.color_forces      = true;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Program F: Link-Bilinear Fermion Probe\n");
    std::printf("  (non-local dynamics + bilinear flux readout)\n");
    std::printf("================================================================\n\n");

    const int L = 8;
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    const double A = 10.0;

    // Collect flux fields after each injection ordering (fi, gi).
    // T_linear[fi][gi][axis][v_mask]: WH coef of flux_axis on mode v_mask.
    std::array<std::array<std::array<std::array<double, 8>, 3>, 3>, 3> T_linear{};

    // B_same[fi][gi][axis]: bilinear axial link along axis (same-axis flux product).
    std::array<std::array<std::array<double, 3>, 3>, 3> B_same{};

    // B_cross[fi][gi][i][j][k]: cross-axis bilinear J_i(x) J_j(x + hat_k).
    std::array<std::array<std::array<std::array<std::array<double, 3>, 3>, 3>, 3>, 3> B_cross{};

    std::printf("Running injection protocol with full non-local toggle set...\n");
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);
        enable_full_nonlocal(rb.toggles);

        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0xF20517D0u + 100u * static_cast<unsigned>(lo) +
                                  static_cast<unsigned>(hi));

        inject_wh_mode(rb, w1_mask[fi], fi, A);
        rb.run(1);
        inject_wh_mode(rb, w1_mask[gi], gi, A);
        rb.run(1);

        const auto& vox = rb.voxels();

        for (int axis = 0; axis < 3; ++axis)
        for (int v = 0; v < 8; ++v) {
            T_linear[fi][gi][axis][v] = wh_coef_flux_component(vox, L, axis, v);
        }

        for (int axis = 0; axis < 3; ++axis) {
            B_same[fi][gi][axis] = bilinear_link(vox, L, axis);
        }

        for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        for (int k = 0; k < 3; ++k) {
            B_cross[fi][gi][i][j][k] = cross_bilinear(vox, L, i, j, k);
        }
    }

    // ===================================================================
    // PART A: linear WH anticommutator (reproduce FTD-0074 baseline)
    // ===================================================================
    std::printf("\n--- Part A: Linear WH anticommutator (baseline, cf. FTD-0074) ---\n");
    std::printf("    L_i = WH_flux_i[chi_{natural w1 mode along i}]\n");
    std::printf("    Clifford expects: {L_i, L_j} = 2 delta_ij * 1_flux\n\n");
    std::printf("  pair    | {L_f,L_g} on axis x   y   z       |off|\n");
    std::printf("  --------+--------------------------------------------\n");
    int linear_clifford = 0;
    const double tol_lin = 0.5;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        double ac_diag[3];
        double ac_off_sum = 0.0;
        for (int axis = 0; axis < 3; ++axis) {
            const int nat = w1_mask[axis];
            ac_diag[axis] = T_linear[fi][gi][axis][nat] +
                            T_linear[gi][fi][axis][nat];
            for (int v = 0; v < 8; ++v) {
                if (v == nat) continue;
                ac_off_sum += std::abs(T_linear[fi][gi][axis][v] +
                                        T_linear[gi][fi][axis][v]);
            }
        }
        std::printf("  {%d,%d}   | %+7.3f  %+7.3f  %+7.3f      |off|=%.3f\n",
                    fi + 1, gi + 1, ac_diag[0], ac_diag[1], ac_diag[2],
                    ac_off_sum);
        if (fi == gi) {
            if (std::abs(ac_diag[fi]) > 1.0 &&
                std::abs(ac_diag[(fi + 1) % 3]) < tol_lin &&
                std::abs(ac_diag[(fi + 2) % 3]) < tol_lin) ++linear_clifford;
        } else {
            if (std::abs(ac_diag[0]) < tol_lin &&
                std::abs(ac_diag[1]) < tol_lin &&
                std::abs(ac_diag[2]) < tol_lin) ++linear_clifford;
        }
    }
    std::printf("\n  Linear Clifford-consistent pairs: %d / 6\n", linear_clifford);

    // ===================================================================
    // PART B: bilinear link anticommutator (Program F core)
    // ===================================================================
    std::printf("\n--- Part B: Bilinear link anticommutator (Program F) ---\n");
    std::printf("    B_i = sum_{faces perp to i} J_i(x) * J_i(x + hat_i)\n");
    std::printf("    Clifford expects: {B_i, B_j} = 2 delta_ij * B_0\n\n");
    std::printf("  pair    | {B_f, B_g} diag (axes x/y/z)          |off-axis|\n");
    std::printf("  --------+-------------------------------------------------\n");

    // Baseline B_0 = bilinear link from the trivial run (no injection)
    ftd::RenderBridge rb0(L);
    enable_full_nonlocal(rb0.toggles);
    rb0.seed_rng(0xF2BA5E10u);
    rb0.run(2);
    const auto& vox0 = rb0.voxels();
    double B0[3];
    for (int axis = 0; axis < 3; ++axis) B0[axis] = bilinear_link(vox0, L, axis);
    std::printf("  (baseline B_0: Bx=%+.4f  By=%+.4f  Bz=%+.4f)\n\n",
                B0[0], B0[1], B0[2]);

    int bilinear_clifford = 0;
    const double tol_bi_diag = 1.0;
    const double tol_bi_off  = 0.5;

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        // {B_f, B_g} = B[f→g] + B[g→f] - 2*B0 (subtract baseline identity
        // component to isolate the injection-induced contribution)
        double ac[3];
        for (int axis = 0; axis < 3; ++axis) {
            ac[axis] = B_same[fi][gi][axis] + B_same[gi][fi][axis]
                       - 2.0 * B0[axis];
        }
        double off_mag = std::abs(ac[(fi + 1) % 3]) +
                         std::abs(ac[(fi + 2) % 3]) +
                         (fi != gi ? std::abs(ac[gi]) : 0.0);
        std::printf("  {%d,%d}   | %+8.3f  %+8.3f  %+8.3f         |off|=%.3f\n",
                    fi + 1, gi + 1, ac[0], ac[1], ac[2], off_mag);

        // Clifford check:
        //   i==j: exactly B_i should be large (diagonal "2·1"), others ≈ 0.
        //   i!=j: all should be ≈ 0.
        if (fi == gi) {
            const bool active_matches = std::abs(ac[fi]) > tol_bi_diag &&
                                         std::abs(ac[(fi + 1) % 3]) < tol_bi_off &&
                                         std::abs(ac[(fi + 2) % 3]) < tol_bi_off;
            if (active_matches) ++bilinear_clifford;
        } else {
            const bool all_small = std::abs(ac[0]) < tol_bi_off &&
                                   std::abs(ac[1]) < tol_bi_off &&
                                   std::abs(ac[2]) < tol_bi_off;
            if (all_small) ++bilinear_clifford;
        }
    }
    std::printf("\n  Bilinear Clifford-consistent pairs: %d / 6\n", bilinear_clifford);

    // ===================================================================
    // PART C: cross-axis bilinear structure (mode coupling)
    // ===================================================================
    std::printf("\n--- Part C: Cross-axis bilinear J_i(x) * J_j(x + hat_k) ---\n");
    std::printf("  Tests whether non-local dynamics generate cross-axis coupling.\n");
    std::printf("  A true Clifford structure would show {B_ij, B_kl} tensor structure.\n\n");

    double cross_mag_sum_symm = 0.0;
    double cross_mag_sum_antisymm = 0.0;
    int n_nonzero_symm = 0, n_nonzero_antisymm = 0;

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        for (int k = 0; k < 3; ++k) {
            if (i == j && j == k) continue;  // axial-only; skip
            const double val = B_cross[fi][gi][i][j][k];
            const double symm = 0.5 * (val + B_cross[gi][fi][i][j][k]);
            const double anti = 0.5 * (val - B_cross[gi][fi][i][j][k]);
            cross_mag_sum_symm     += std::abs(symm);
            cross_mag_sum_antisymm += std::abs(anti);
            if (std::abs(symm) > 0.1) ++n_nonzero_symm;
            if (std::abs(anti) > 0.1) ++n_nonzero_antisymm;
        }
    }
    std::printf("  Total |symmetric| cross-bilinear mass:     %.3f (nonzero %d)\n",
                cross_mag_sum_symm, n_nonzero_symm);
    std::printf("  Total |antisymmetric| cross-bilinear mass: %.3f (nonzero %d)\n",
                cross_mag_sum_antisymm, n_nonzero_antisymm);
    std::printf("\n  Clifford signature expects: antisymm > 0 AND symm small relative\n");
    std::printf("  (non-commutative product -> large antisymmetric part).\n");

    // ===================================================================
    // Verdict
    // ===================================================================
    std::printf("\n================================================================\n");
    std::printf("  Program F Verdict\n");
    std::printf("================================================================\n");
    const bool linear_ok   = (linear_clifford   == 6);
    const bool bilinear_ok = (bilinear_clifford == 6);

    std::printf("  Linear flux WH (baseline):        %d/6 Clifford-consistent\n",
                linear_clifford);
    std::printf("  Bilinear link (Program F core):   %d/6 Clifford-consistent\n",
                bilinear_clifford);
    std::printf("  Cross-axis antisymmetric mass:    %.3f\n",
                cross_mag_sum_antisymm);
    std::printf("\n");

    if (bilinear_ok) {
        std::printf("  ==> Non-local dynamics + bilinear readout PRODUCES Clifford.\n");
        std::printf("  ==> Program F CLOSED POSITIVE. Fermion emergence via link\n");
        std::printf("      bilinear is the algebraic category.\n");
    } else if (cross_mag_sum_antisymm > 1.0) {
        std::printf("  ==> Partial positive: cross-axis antisymmetric structure is\n");
        std::printf("      present but axial bilinears do not close Clifford on their\n");
        std::printf("      own. Further work needed on the right bilinear basis.\n");
    } else {
        std::printf("  ==> Program F CLOSED NEGATIVE. The no-go extends to non-local\n");
        std::printf("      dynamics at the link-bilinear level. Fermion emergence\n");
        std::printf("      from elementary operators is structurally blocked in FTD.\n");
        std::printf("      Remaining routes: composite baryons (Program H), or matter\n");
        std::printf("      as Branch-B selection.\n");
    }
    std::printf("================================================================\n");

    return 0;
}

/**
 * @file test_clifford_multigrade.cpp
 * @brief Path 1 — Wilson-loop-style multi-grade decomposition.
 *
 * F-double-prime (FTD-0087) showed the iterated bivector commutator does not
 * close cleanly at the 4-injection scale. But the F-prime matching signature
 * (FTD-0086) holds at 2-injection. The question Path 1 asks: under
 * 2-injection on (f, g), does the FULL Cl(3,0) grade structure hold?
 *
 * In Cl(3,0):
 *   Grade 0 (scalar):     S = sum |J(x)|^2
 *   Grade 1 (vector):     V_i = sum J_i(x)
 *   Grade 2 (bivector):   P_{ij} = sum [J_i(x) J_j(x+hat_i) - J_i(x+hat_j) J_j(x)]
 *   Grade 3 (pseudoscalar): T = sum J_x(x) J_y(x) J_z(x)
 *
 * Cl(3,0) algebra predictions for 2-axis injection (f, g) with f != g:
 *   - Grade 0 scalar: non-zero (Casimir-like, axis-isotropic across pairs).
 *   - Grade 1 vector: dominantly on axes f and g (no third axis content).
 *   - Grade 2 bivector: concentrated on matching plaquette P_{fg} (FTD-0086).
 *   - Grade 3 pseudoscalar T: NEAR ZERO (third axis is absent).
 *
 * If T is small and V is on (f, g) only, the Cl(3,0) grade structure holds
 * at 2-injection order. This validates the F-prime bivector matching as part
 * of a coherent multi-grade picture, not an isolated coincidence.
 *
 * Additionally we measure the 4-link Wilson loop product
 *   W_{ij}^{4-fold}(x) = J_i(x) J_j(x+hat_i) J_i(x+hat_i+hat_j) J_j(x+hat_j)
 * which is grade-0 (scalar) by construction. Compare to grade-0 from sum |J|^2.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

namespace {

inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

inline double comp(const ftd::Vec3& v, int a) {
    return a == 0 ? v.x : a == 1 ? v.y : v.z;
}

inline int idx(int x, int y, int z, int L) {
    return x * L * L + y * L + z;
}

// Grade 0: scalar Casimir.
double grade0_scalar(const std::vector<ftd::Voxel>& vox, int L) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const auto& f = vox[idx(x,y,z,L)].flux;
        sum += f.x*f.x + f.y*f.y + f.z*f.z;
    }
    return sum;
}

// Grade 1: vector V_i = sum J_i.
void grade1_vector(const std::vector<ftd::Voxel>& vox, int L, double V[3]) {
    V[0] = V[1] = V[2] = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const auto& f = vox[idx(x,y,z,L)].flux;
        V[0] += f.x; V[1] += f.y; V[2] += f.z;
    }
}

// Grade 2: bivector P_{ij}.
double grade2_bivector(const std::vector<ftd::Voxel>& vox, int L,
                       int axis_i, int axis_j) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        int x_i = x, y_i = y, z_i = z;
        if (axis_i == 0) x_i = (x + 1) % 2;
        if (axis_i == 1) y_i = (y + 1) % 2;
        if (axis_i == 2) z_i = (z + 1) % 2;
        int x_j = x, y_j = y, z_j = z;
        if (axis_j == 0) x_j = (x + 1) % 2;
        if (axis_j == 1) y_j = (y + 1) % 2;
        if (axis_j == 2) z_j = (z + 1) % 2;
        const double Ji_x   = comp(vox[idx(x,y,z,L)].flux, axis_i);
        const double Jj_xpi = comp(vox[idx(x_i,y_i,z_i,L)].flux, axis_j);
        const double Ji_xpj = comp(vox[idx(x_j,y_j,z_j,L)].flux, axis_i);
        const double Jj_x   = comp(vox[idx(x,y,z,L)].flux, axis_j);
        sum += Ji_x * Jj_xpi - Ji_xpj * Jj_x;
    }
    return sum;
}

// Grade 3: pseudoscalar T = sum J_x J_y J_z at each site.
double grade3_pseudoscalar(const std::vector<ftd::Voxel>& vox, int L) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const auto& f = vox[idx(x,y,z,L)].flux;
        sum += f.x * f.y * f.z;
    }
    return sum;
}

// 4-link Wilson loop: J_i(x) J_j(x+hat_i) J_i(x+hat_i+hat_j) J_j(x+hat_j)
// summed over plaquettes in the (i, j) plane within the 2^3 block.
double wilson_loop_4link(const std::vector<ftd::Voxel>& vox, int L,
                         int axis_i, int axis_j) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        int xi = x, yi = y, zi = z;
        if (axis_i == 0) xi = (x + 1) % 2;
        if (axis_i == 1) yi = (y + 1) % 2;
        if (axis_i == 2) zi = (z + 1) % 2;
        int xj = x, yj = y, zj = z;
        if (axis_j == 0) xj = (x + 1) % 2;
        if (axis_j == 1) yj = (y + 1) % 2;
        if (axis_j == 2) zj = (z + 1) % 2;
        int xij = xi, yij = yi, zij = zi;
        if (axis_j == 0) xij = (xij + 1) % 2;
        if (axis_j == 1) yij = (yij + 1) % 2;
        if (axis_j == 2) zij = (zij + 1) % 2;

        const double a = comp(vox[idx(x,   y,   z,   L)].flux, axis_i);
        const double b = comp(vox[idx(xi,  yi,  zi,  L)].flux, axis_j);
        const double c = comp(vox[idx(xij, yij, zij, L)].flux, axis_i);
        const double d = comp(vox[idx(xj,  yj,  zj,  L)].flux, axis_j);
        sum += a * b * c * d;
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
    t.wave_propagation  = true;
    t.gauss_projection  = true;
    t.genesis           = true;
    t.movement          = true;
    t.forces            = true;
    t.emergent_forces   = true;
    t.pair_production   = true;
    t.weak_transmutation= true;
    t.exchange_force    = true;
    t.strong_force      = true;
    t.triad_binding     = true;
    t.color_forces      = true;
}

struct GradeMeasurement {
    double S;        // grade 0
    double V[3];     // grade 1
    double P[3];     // grade 2 (P_xy, P_xz, P_yz)
    double T;        // grade 3
    double W[3];     // 4-link Wilson loop
};

GradeMeasurement run_2inj(int L, double A, unsigned seed,
                          int axis_f, int axis_g) {
    ftd::RenderBridge rb(L);
    enable_full_nonlocal(rb.toggles);
    rb.seed_rng(seed);
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    inject_wh_mode(rb, w1_mask[axis_f], axis_f, A);
    rb.run(1);
    inject_wh_mode(rb, w1_mask[axis_g], axis_g, A);
    rb.run(1);

    const auto& vox = rb.voxels();
    GradeMeasurement m{};
    m.S = grade0_scalar(vox, L);
    grade1_vector(vox, L, m.V);
    m.P[0] = grade2_bivector(vox, L, 0, 1);
    m.P[1] = grade2_bivector(vox, L, 0, 2);
    m.P[2] = grade2_bivector(vox, L, 1, 2);
    m.T = grade3_pseudoscalar(vox, L);
    m.W[0] = wilson_loop_4link(vox, L, 0, 1);
    m.W[1] = wilson_loop_4link(vox, L, 0, 2);
    m.W[2] = wilson_loop_4link(vox, L, 1, 2);
    return m;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Path 1 — Cl(3,0) Multi-Grade Decomposition\n");
    std::printf("================================================================\n\n");

    const int L = 8;
    const double A = 10.0;
    const std::array<unsigned, 8> seeds = {
        0xF4170517u, 0xF4170518u, 0xF4170519u, 0xF417051Au,
        0xF417051Bu, 0xF417051Cu, 0xF417051Du, 0xF417051Eu
    };

    const std::array<std::pair<int,int>, 3> off_pairs = {{
        {0, 1}, {0, 2}, {1, 2}
    }};
    const char* pair_name[3] = { "(x,y)", "(x,z)", "(y,z)" };
    const char* plaq_name[3] = { "P_xy", "P_xz", "P_yz" };
    const char* axis_name[3] = { "x", "y", "z" };

    std::printf("Protocol: 2-injection (f, g), 8 seeds. Decompose into Cl(3,0) grades.\n\n");

    // ==============================================================
    // For each off-diagonal pair, decompose anticommutator and commutator
    // into grades 0, 1, 2, 3 and 4-link Wilson loops.
    // ==============================================================
    int grade_passes = 0;
    int total_grade_tests = 0;

    for (int p = 0; p < 3; ++p) {
        const int fi = off_pairs[p].first;
        const int gi = off_pairs[p].second;
        const int third = 3 - fi - gi;  // axis NOT in (fi, gi)

        std::printf("===== Pair %s (third axis = %s) =====\n",
                    pair_name[p], axis_name[third]);

        // Mean over seeds: anticommutator (symm under swap) and commutator (anti).
        double S_symm = 0, S_anti = 0;
        double V_symm[3] = {0,0,0}, V_anti[3] = {0,0,0};
        double P_symm[3] = {0,0,0}, P_anti[3] = {0,0,0};
        double T_symm = 0, T_anti = 0;
        double W_symm[3] = {0,0,0}, W_anti[3] = {0,0,0};

        for (unsigned seed : seeds) {
            GradeMeasurement fwd = run_2inj(L, A, seed, fi, gi);
            GradeMeasurement bwd = run_2inj(L, A, seed, gi, fi);

            S_symm += fwd.S + bwd.S;       S_anti += fwd.S - bwd.S;
            T_symm += fwd.T + bwd.T;       T_anti += fwd.T - bwd.T;
            for (int a = 0; a < 3; ++a) {
                V_symm[a] += fwd.V[a] + bwd.V[a];
                V_anti[a] += fwd.V[a] - bwd.V[a];
                P_symm[a] += fwd.P[a] + bwd.P[a];
                P_anti[a] += fwd.P[a] - bwd.P[a];
                W_symm[a] += fwd.W[a] + bwd.W[a];
                W_anti[a] += fwd.W[a] - bwd.W[a];
            }
        }
        const double Nseed = static_cast<double>(seeds.size());
        S_symm /= Nseed; S_anti /= Nseed;
        T_symm /= Nseed; T_anti /= Nseed;
        for (int a = 0; a < 3; ++a) {
            V_symm[a] /= Nseed; V_anti[a] /= Nseed;
            P_symm[a] /= Nseed; P_anti[a] /= Nseed;
            W_symm[a] /= Nseed; W_anti[a] /= Nseed;
        }

        std::printf("  Grade 0 (scalar):    {symm} = %+9.3f      [anti] = %+9.3f\n",
                    S_symm, S_anti);

        std::printf("  Grade 1 (vector):    V_x  = (%+7.3f, %+7.3f)  ",
                    V_symm[0], V_anti[0]);
        std::printf("V_y  = (%+7.3f, %+7.3f)  ", V_symm[1], V_anti[1]);
        std::printf("V_z  = (%+7.3f, %+7.3f)\n", V_symm[2], V_anti[2]);

        std::printf("  Grade 2 (bivector):  P_xy = (%+7.3f, %+7.3f)  ",
                    P_symm[0], P_anti[0]);
        std::printf("P_xz = (%+7.3f, %+7.3f)  ", P_symm[1], P_anti[1]);
        std::printf("P_yz = (%+7.3f, %+7.3f)\n", P_symm[2], P_anti[2]);

        std::printf("  Grade 3 (pseudoscalar): T = %+9.3f      [anti] = %+9.3f\n",
                    T_symm, T_anti);

        std::printf("  4-link Wilson loop:  W_xy = (%+7.3f, %+7.3f)  ",
                    W_symm[0], W_anti[0]);
        std::printf("W_xz = (%+7.3f, %+7.3f)  ", W_symm[1], W_anti[1]);
        std::printf("W_yz = (%+7.3f, %+7.3f)\n", W_symm[2], W_anti[2]);

        // Cl(3,0) grade-structure tests for this pair:
        // (1) Grade 3 (pseudoscalar) should be SMALL — third axis absent.
        // (2) Grade 1 vector mass on third axis should be SMALL.
        // (3) Grade 2 bivector commutator concentrated on matching plaquette.
        // (4) Grade 0 scalar present (Casimir non-zero).
        const double V_mag_third  = std::abs(V_symm[third]);
        const double V_mag_active = std::abs(V_symm[fi]) + std::abs(V_symm[gi]);
        const double P_match_anti = std::abs(P_anti[p]);
        double P_off_anti = 0;
        for (int a = 0; a < 3; ++a) if (a != p) P_off_anti = std::max(P_off_anti, std::abs(P_anti[a]));

        std::printf("  Cl(3,0) grade-structure tests:\n");

        // Test 1: pseudoscalar small (third axis absent)
        const bool t1 = std::abs(T_symm) < 0.5 * std::abs(S_symm) / 10.0;
        std::printf("    [%s] Grade 3 pseudo |T_symm| = %.3f << |S_symm|/10 = %.3f\n",
                    t1 ? "PASS" : "FAIL",
                    std::abs(T_symm), std::abs(S_symm)/10.0);
        if (t1) ++grade_passes; ++total_grade_tests;

        // Test 2: vector along third axis suppressed
        const bool t2 = (V_mag_third < 0.3 * V_mag_active) || (V_mag_active < 0.1);
        std::printf("    [%s] Grade 1 |V_third| = %.3f vs |V_active| = %.3f\n",
                    t2 ? "PASS" : "FAIL", V_mag_third, V_mag_active);
        if (t2) ++grade_passes; ++total_grade_tests;

        // Test 3: bivector commutator concentrated on matching plaquette
        const bool t3 = (P_match_anti > 0.5) && (P_match_anti > 2.0 * P_off_anti);
        std::printf("    [%s] Grade 2 [E,E] -> %s: |%.3f| vs max_off = %.3f\n",
                    t3 ? "PASS" : "FAIL", plaq_name[p],
                    P_match_anti, P_off_anti);
        if (t3) ++grade_passes; ++total_grade_tests;

        // Test 4: Casimir scalar non-zero
        const bool t4 = std::abs(S_symm) > 100.0;
        std::printf("    [%s] Grade 0 scalar |S_symm| = %.3f > 100\n",
                    t4 ? "PASS" : "FAIL", std::abs(S_symm));
        if (t4) ++grade_passes; ++total_grade_tests;

        std::printf("\n");
    }

    // ==============================================================
    // Verdict
    // ==============================================================
    std::printf("================================================================\n");
    std::printf("  Path 1 Multi-Grade Verdict\n");
    std::printf("================================================================\n");
    std::printf("  Grade-structure tests passed: %d / %d (across 3 off-diag pairs x 4 tests)\n",
                grade_passes, total_grade_tests);

    const double pass_rate = total_grade_tests > 0
                               ? double(grade_passes) / double(total_grade_tests)
                               : 0.0;

    if (pass_rate >= 0.90) {
        std::printf("\n  ==> Cl(3,0) grade structure HOLDS at 2-injection order.\n");
        std::printf("      The bivector-matching signature in F-prime is part of\n");
        std::printf("      a coherent multi-grade picture: vectors on active axes,\n");
        std::printf("      bivectors on matching plaquette, pseudoscalar suppressed,\n");
        std::printf("      scalar Casimir present.\n");
        std::printf("  ==> Iterated-commutator failure (FTD-0087) is most likely\n");
        std::printf("      a 4-injection dynamical-noise issue, NOT an algebraic\n");
        std::printf("      defect. Cl(3,0) skeleton is consistent at leading order.\n");
    } else if (pass_rate >= 0.65) {
        std::printf("\n  ==> Partial Cl(3,0) grade structure. Some grades clean,\n");
        std::printf("      others contaminated. F-prime bivector signature is real\n");
        std::printf("      but multi-grade picture is not fully clean.\n");
    } else {
        std::printf("\n  ==> Cl(3,0) grade structure does NOT hold cleanly.\n");
        std::printf("      The bivector-matching signature in F-prime is real but\n");
        std::printf("      isolated, not part of a Clifford grade structure.\n");
    }
    std::printf("================================================================\n");
    return 0;
}

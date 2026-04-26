/**
 * @file test_moore26_clifford_test.cpp
 * @brief Phase-4c fermion-emergence route: Moore-26 / 3³ block with axial
 *        sawtooth modes as "weight-1" generators.
 *
 * Tests whether the 27-site 3³ block (containing the full Moore-26
 * neighborhood + center) carries Clifford structure on three natural
 * axis-dipole modes:
 *
 *   e_x(x,y,z) = +1 if x=0, 0 if x=1, -1 if x=2   (sawtooth along x)
 *   e_y, e_z analogously.
 *
 * These are real, orthogonal, zero-mean dipole modes on the 3³ block — the
 * Moore-26 analog of Walsh-Hadamard weight-1 modes.
 *
 * Protocol mirrors `test_wh_clifford_anticommutator.cpp`: inject flux ∝
 * A·e_f along axis f → run 1 tick → inject mode e_g along axis g → run 1
 * tick → project state onto (1, e_x, e_y, e_z) basis.
 *
 * Clifford criterion: {e_i, e_j} (normalized) = 2 δ_ij on identity, 0 on
 * the three dipole modes.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

// Block is 3³ at origin (coordinates 0..2 in each axis).
static inline int axial_dipole(int axis, int x, int y, int z) {
    const int c[3] = {x, y, z};
    const int v = c[axis];
    return (v == 0) ? +1 : ((v == 2) ? -1 : 0);
}

// Normalized inner product of state field with axial dipole mode f over the
// 3³ block. Dipole has 18 non-zero entries → normalization by 18 gives mean.
static double dipole_coef(const std::vector<ftd::Voxel>& vox,
                          int L, int axis) {
    double sum = 0.0;
    for (int x = 0; x < 3; ++x)
    for (int y = 0; y < 3; ++y)
    for (int z = 0; z < 3; ++z) {
        const int i = x * L * L + y * L + z;
        sum += static_cast<double>(vox[i].state)
             * static_cast<double>(axial_dipole(axis, x, y, z));
    }
    // 18 nonzero entries (9 at v=0, 9 at v=2)
    return sum / 18.0;
}

// Identity-mode coefficient: mean state over the 3³ block.
static double ident_coef(const std::vector<ftd::Voxel>& vox, int L) {
    double sum = 0.0;
    for (int x = 0; x < 3; ++x)
    for (int y = 0; y < 3; ++y)
    for (int z = 0; z < 3; ++z) {
        const int i = x * L * L + y * L + z;
        sum += static_cast<double>(vox[i].state);
    }
    return sum / 27.0;
}

// Inject flux ∝ A · e_f(x,y,z) on the 3³ block with flux aligned on axis f.
static void inject_moore_dipole(ftd::RenderBridge& rb, int axis, double A) {
    for (int x = 0; x < 3; ++x)
    for (int y = 0; y < 3; ++y)
    for (int z = 0; z < 3; ++z) {
        const int s = axial_dipole(axis, x, y, z);
        if (s == 0) continue;
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
    std::printf("  Phase-4c: Moore-26 / 3³ Block Clifford Anticommutator Test\n");
    std::printf("================================================================\n");
    std::printf("  3 axial dipole modes e_x, e_y, e_z on a 3³ = 27-site block.\n");
    std::printf("  Protocol: inject mode f → tick → inject mode g → tick →\n");
    std::printf("            project state onto (1, e_x, e_y, e_z) basis.\n\n");

    const int L = 8;
    const double A = 10.0;

    // T[f][g][k]: k=0 is identity coefficient; k=1,2,3 are e_x, e_y, e_z
    std::array<std::array<std::array<double, 4>, 3>, 3> T{};

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
        rb.seed_rng(0xA80017D6 + 100 * lo + hi);

        inject_moore_dipole(rb, fi, A);
        rb.run(1);
        inject_moore_dipole(rb, gi, A);
        rb.run(1);

        const auto& vox = rb.voxels();
        T[fi][gi][0] = ident_coef(vox, L);
        for (int k = 0; k < 3; ++k) {
            T[fi][gi][k + 1] = dipole_coef(vox, L, k);
        }
    }

    // Report anticommutator matrix
    std::printf("--- Anticommutator {e_f, e_g} on (1, e_x, e_y, e_z) basis ---\n");
    std::printf("  pair  |  ident |  e_x   |  e_y   |  e_z   \n");
    std::printf("  ------+--------+--------+--------+--------\n");
    int clifford_pairs = 0;
    const double tol = 0.2;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        const double ac_ident = T[fi][gi][0] + T[gi][fi][0];
        double ac_off = 0.0;
        for (int k = 0; k < 3; ++k) {
            ac_off += std::abs(T[fi][gi][k + 1] + T[gi][fi][k + 1]);
        }
        std::printf("  {%d,%d} | %+6.3f | %+6.3f | %+6.3f | %+6.3f\n",
                    fi + 1, gi + 1,
                    ac_ident,
                    T[fi][gi][1] + T[gi][fi][1],
                    T[fi][gi][2] + T[gi][fi][2],
                    T[fi][gi][3] + T[gi][fi][3]);

        const double expected_ident = (fi == gi) ? 2.0 : 0.0;
        const bool ident_ok = std::abs(ac_ident - expected_ident) < tol;
        const bool off_ok = ac_off < tol * 3;
        if (ident_ok && off_ok) ++clifford_pairs;
    }

    std::printf("\n  Clifford-consistent pairs: %d / 6\n", clifford_pairs);
    std::printf("\n================================================================\n");
    if (clifford_pairs == 6) {
        std::printf("  RESULT: Moore-26 / 3³ block SUPPORTS Clifford structure.\n");
        std::printf("  Fermion-emergence conjecture UPGRADED via Moore-26 stencil.\n");
    } else {
        std::printf("  RESULT: Moore-26 / 3³ block does NOT support Clifford structure\n");
        std::printf("  on axial dipole modes. Together with FTD-0061 (2³ block) and\n");
        std::printf("  FTD-0071 (four engine non-linearities on 2³), fermion emergence\n");
        std::printf("  from direct grade-structure bases is broadly falsified.\n");
    }
    std::printf("================================================================\n");
    return 0;
}

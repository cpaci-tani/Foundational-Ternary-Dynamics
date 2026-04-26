/**
 * @file test_plaquette_bivector_clifford.cpp
 * @brief Program F-prime — plaquette bivector probe.
 *
 * Program F (FTD-0085) detected non-commutative algebraic structure in the
 * cross-axis bilinear sector but axial bilinears didn't close Clifford.
 * The natural Clifford-bivector basis on a 3D lattice is the plaquette
 * 2-form
 *
 *   P_{ij}(x) = J_i(x) J_j(x + hat_i) - J_i(x + hat_j) J_j(x)        [i < j]
 *
 * which is antisymmetric in (i, j) by construction and lives on the 6
 * oriented plaquettes per site (3 unordered axis pairs * 2 orientations).
 *
 * In Cl(3,0) the bivector subalgebra is isomorphic to su(2) / so(3):
 *   b_1 = e_2 e_3,  b_2 = e_3 e_1,  b_3 = e_1 e_2
 *   {b_i, b_j} = -2 delta_ij                  (anticommutator: diagonal, NEGATIVE)
 *   [b_i, b_j] = -2 epsilon_{ijk} b_k         (commutator: third bivector)
 *
 * The probe: inject (f_i, g_i) WH modes, run with full non-local dynamics,
 * read out three plaquette totals P_xy, P_yz, P_zx. Compare:
 *
 *   {hat_E_f, hat_E_g}[P_a] = P_a[f→g] + P_a[g→f]   (symmetric under inj-swap)
 *   [hat_E_f, hat_E_g][P_a] = P_a[f→g] - P_a[g→f]   (antisymmetric under inj-swap)
 *
 * Clifford-bivector prediction:
 *   - For pair (f, g) with f != g, the COMMUTATOR should be concentrated on
 *     the plaquette P_a where a corresponds to the unordered pair (f, g).
 *     i.e., [E_x, E_y] gives mass on P_xy, not on P_yz or P_zx.
 *   - The anticommutator (off-diagonal in (f, g)) should vanish.
 *   - Diagonal anticommutator (f = g) is the "scalar grade" - non-trivial.
 *
 * If this signature appears: bivector emergence is positive and the
 * fermion-emergence no-go (mode-erasure theorem) is broken on bivector basis.
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

// Plaquette bivector total over the 2^3 block on the (i, j) axis pair.
// P_{ij}^total = sum_{x in 2^3} [J_i(x) J_j(x + hat_i) - J_i(x + hat_j) J_j(x)]
double plaquette_bivector(const std::vector<ftd::Voxel>& vox, int L,
                          int axis_i, int axis_j) {
    auto comp = [](const ftd::Vec3& v, int a) -> double {
        if (a == 0) return v.x;
        if (a == 1) return v.y;
        return v.z;
    };
    auto idx = [L](int x, int y, int z) {
        return x * L * L + y * L + z;
    };
    auto step = [](int v, int axis, int dir) {
        int xyz[3] = {0, 0, 0};
        xyz[axis] = dir;
        return xyz;
    };

    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        // Site x
        const int s0 = idx(x, y, z);
        // Site x + hat_i
        int x_i = x, y_i = y, z_i = z;
        if (axis_i == 0) x_i = (x + 1) % 2;
        if (axis_i == 1) y_i = (y + 1) % 2;
        if (axis_i == 2) z_i = (z + 1) % 2;
        const int s_i = idx(x_i, y_i, z_i);
        // Site x + hat_j
        int x_j = x, y_j = y, z_j = z;
        if (axis_j == 0) x_j = (x + 1) % 2;
        if (axis_j == 1) y_j = (y + 1) % 2;
        if (axis_j == 2) z_j = (z + 1) % 2;
        const int s_j = idx(x_j, y_j, z_j);

        const double Ji_x   = comp(vox[s0].flux, axis_i);
        const double Jj_xpi = comp(vox[s_i].flux, axis_j);
        const double Ji_xpj = comp(vox[s_j].flux, axis_i);
        const double Jj_x   = comp(vox[s0].flux, axis_j);

        sum += Ji_x * Jj_xpi - Ji_xpj * Jj_x;
        (void) step;  // silence unused
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

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Program F-prime: Plaquette Bivector Probe\n");
    std::printf("  (natural Cl(3,0) bivector basis: 2-form on oriented faces)\n");
    std::printf("================================================================\n\n");

    const int L = 8;
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    const double A = 10.0;

    // Three unordered axis pairs (i < j): (0,1), (0,2), (1,2)
    // Bivector index a: 0 = P_{xy} (plaq 0,1), 1 = P_{xz} (plaq 0,2), 2 = P_{yz} (plaq 1,2)
    const std::array<std::pair<int,int>, 3> bivec_pair = {{
        {0, 1}, {0, 2}, {1, 2}
    }};
    const char* bivec_name[3] = { "P_xy", "P_xz", "P_yz" };

    // R[f][g][a]: plaquette a after injection sequence (f, g)
    std::array<std::array<std::array<double, 3>, 3>, 3> R{};

    std::printf("Running injection protocol with full non-local dynamics...\n");
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = 0; gi < 3; ++gi) {
        ftd::RenderBridge rb(L);
        enable_full_nonlocal(rb.toggles);

        const int lo = (fi < gi) ? fi : gi;
        const int hi = (fi < gi) ? gi : fi;
        rb.seed_rng(0xF3170517u + 100u * static_cast<unsigned>(lo) +
                                  static_cast<unsigned>(hi));

        inject_wh_mode(rb, w1_mask[fi], fi, A);
        rb.run(1);
        inject_wh_mode(rb, w1_mask[gi], gi, A);
        rb.run(1);

        const auto& vox = rb.voxels();
        for (int a = 0; a < 3; ++a) {
            R[fi][gi][a] = plaquette_bivector(vox, L,
                                               bivec_pair[a].first,
                                               bivec_pair[a].second);
        }
    }

    // ===================================================================
    // PART A: anticommutator (symmetric under injection swap)
    // ===================================================================
    std::printf("\n--- Part A: Anticommutator {hat_E_f, hat_E_g}[P_a] ---\n");
    std::printf("    = R[f][g][a] + R[g][f][a]\n");
    std::printf("    Cl(3,0) bivector expects diagonal {P_a, P_a} = -2*1, off-diag = 0\n\n");
    std::printf("  injection pair | P_xy           P_xz           P_yz\n");
    std::printf("  ---------------+-------------------------------------------------\n");
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        double symm[3];
        for (int a = 0; a < 3; ++a) {
            symm[a] = R[fi][gi][a] + R[gi][fi][a];
        }
        std::printf("  {%d,%d}          | %+13.4f  %+13.4f  %+13.4f\n",
                    fi + 1, gi + 1, symm[0], symm[1], symm[2]);
    }

    // ===================================================================
    // PART B: commutator (antisymmetric under injection swap) — KEY TEST
    // ===================================================================
    std::printf("\n--- Part B: Commutator [hat_E_f, hat_E_g][P_a] (KEY TEST) ---\n");
    std::printf("    = R[f][g][a] - R[g][f][a]\n");
    std::printf("    Cl(3,0) prediction: [E_i, E_j] proportional to bivector e_i e_j.\n");
    std::printf("    Expect mass concentrated on the plaquette matching (f, g).\n\n");
    std::printf("  injection pair | P_xy           P_xz           P_yz       expected dominant\n");
    std::printf("  ---------------+----------------------------------------------------------\n");

    int correct_concentration = 0;
    int total_offdiag_pairs = 0;
    double signal_to_offaxis = 0.0;
    int s2o_count = 0;

    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi + 1; gi < 3; ++gi) {  // strict ordering for off-diagonal pairs
        double anti[3];
        for (int a = 0; a < 3; ++a) {
            anti[a] = R[fi][gi][a] - R[gi][fi][a];
        }

        // The plaquette index a* matching the pair (fi, gi):
        //   pair (0,1) -> a* = 0 (P_xy)
        //   pair (0,2) -> a* = 1 (P_xz)
        //   pair (1,2) -> a* = 2 (P_yz)
        int a_star = -1;
        for (int a = 0; a < 3; ++a) {
            if (bivec_pair[a].first == fi && bivec_pair[a].second == gi) {
                a_star = a;
                break;
            }
        }

        const double signal = std::abs(anti[a_star]);
        double off_max = 0.0;
        for (int a = 0; a < 3; ++a) {
            if (a == a_star) continue;
            off_max = std::max(off_max, std::abs(anti[a]));
        }

        std::printf("  {%d,%d}          | %+13.4f  %+13.4f  %+13.4f       %s\n",
                    fi + 1, gi + 1, anti[0], anti[1], anti[2], bivec_name[a_star]);

        ++total_offdiag_pairs;
        if (signal > 1.0 && signal > 2.0 * off_max) ++correct_concentration;
        if (off_max > 1e-9) {
            signal_to_offaxis += signal / off_max;
            ++s2o_count;
        }
    }

    const double mean_s2o = s2o_count > 0 ? signal_to_offaxis / s2o_count : 0.0;
    std::printf("\n  Signal/off-axis (geom mean over off-diagonal pairs): %.2f\n", mean_s2o);
    std::printf("  Pairs with correct concentration: %d / %d\n",
                correct_concentration, total_offdiag_pairs);

    // ===================================================================
    // PART C: full bivector anticommutator structure (off-diagonal in pair)
    // ===================================================================
    std::printf("\n--- Part C: Off-diagonal anticommutator vs commutator masses ---\n");
    double anti_mass = 0.0, symm_mass = 0.0;
    int n_anti_nonzero = 0, n_symm_nonzero = 0;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi; gi < 3; ++gi) {
        for (int a = 0; a < 3; ++a) {
            const double s = R[fi][gi][a] + R[gi][fi][a];
            const double c = R[fi][gi][a] - R[gi][fi][a];
            symm_mass += std::abs(s);
            anti_mass += std::abs(c);
            if (std::abs(s) > 0.5) ++n_symm_nonzero;
            if (std::abs(c) > 0.5) ++n_anti_nonzero;
        }
    }
    std::printf("  Total |anticommutator (symm)| mass: %.3f  (nonzero: %d)\n",
                symm_mass, n_symm_nonzero);
    std::printf("  Total |commutator    (anti)| mass: %.3f  (nonzero: %d)\n",
                anti_mass, n_anti_nonzero);
    if (anti_mass + symm_mass > 1e-9) {
        std::printf("  Ratio anti/(anti+symm) = %.3f\n",
                    anti_mass / (anti_mass + symm_mass));
        std::printf("  (Cl(3,0) bivector subalgebra: symm dominates on diag,\n");
        std::printf("   anti dominates on off-diag)\n");
    }

    // ===================================================================
    // PART D: bivector basis Frobenius distance from su(2) structure
    // ===================================================================
    // su(2) requires [b_i, b_j] = ±2*epsilon_{ijk}*b_k (proportional to third).
    // Test: for off-diagonal injection pairs, is anti[a*] proportional to a
    // single plaquette and the others suppressed?
    std::printf("\n--- Part D: SU(2)/SO(3) bivector signature ---\n");
    std::printf("  For [E_i, E_j] to give bivector e_i e_j:\n");
    std::printf("    inj (0,1) → mass on P_xy only\n");
    std::printf("    inj (0,2) → mass on P_xz only\n");
    std::printf("    inj (1,2) → mass on P_yz only\n\n");

    bool su2_signature = true;
    for (int fi = 0; fi < 3; ++fi)
    for (int gi = fi + 1; gi < 3; ++gi) {
        double anti[3];
        for (int a = 0; a < 3; ++a) {
            anti[a] = R[fi][gi][a] - R[gi][fi][a];
        }
        int a_star = -1;
        for (int a = 0; a < 3; ++a) {
            if (bivec_pair[a].first == fi && bivec_pair[a].second == gi) {
                a_star = a;
                break;
            }
        }
        const double signal = std::abs(anti[a_star]);
        double off_max = 0.0;
        for (int a = 0; a < 3; ++a) {
            if (a == a_star) continue;
            off_max = std::max(off_max, std::abs(anti[a]));
        }
        const bool ok = (signal > 1.0) && (signal > 3.0 * off_max);
        if (!ok) su2_signature = false;
        std::printf("    {%d,%d}: signal_on_%s = %.3f, max_off = %.3f  -- %s\n",
                    fi + 1, gi + 1, bivec_name[a_star], signal, off_max,
                    ok ? "match" : "no match");
    }

    // ===================================================================
    // Verdict
    // ===================================================================
    std::printf("\n================================================================\n");
    std::printf("  Program F-prime Verdict\n");
    std::printf("================================================================\n");
    std::printf("  Correct-concentration off-diagonal pairs: %d / %d\n",
                correct_concentration, total_offdiag_pairs);
    std::printf("  Signal/off-axis geom mean: %.2f\n", mean_s2o);
    std::printf("  SU(2)/SO(3) bivector signature: %s\n\n",
                su2_signature ? "YES" : "NO");

    if (su2_signature && correct_concentration == total_offdiag_pairs) {
        std::printf("  ==> Plaquette bivectors close SU(2)-like algebra.\n");
        std::printf("  ==> Program F-prime CLOSED POSITIVE.\n");
        std::printf("  ==> Fermion emergence shifts from Branch-B selection to\n");
        std::printf("      Branch-A derivation on the bivector basis.\n");
    } else if (correct_concentration >= 2) {
        std::printf("  ==> Partial positive: most pairs concentrate correctly\n");
        std::printf("      but full SU(2) signature is borderline. Algebra is\n");
        std::printf("      non-abelian and bivector-like but with significant\n");
        std::printf("      contamination.\n");
    } else if (anti_mass > 1.0) {
        std::printf("  ==> Non-commutativity present (commutator mass = %.2f)\n",
                    anti_mass);
        std::printf("      but plaquette bivectors are NOT the correct Clifford\n");
        std::printf("      basis. Algebra is non-abelian on a different generator\n");
        std::printf("      set. Further work needed.\n");
    } else {
        std::printf("  ==> NULL — plaquette bivectors do not show non-commutative\n");
        std::printf("      structure. Re-evaluate the link-bilinear positive\n");
        std::printf("      result from Program F (FTD-0085).\n");
    }
    std::printf("================================================================\n");

    return 0;
}

/**
 * @file test_bivector_closure.cpp
 * @brief Program F-double-prime — closure tests for the plaquette bivector
 *        algebra detected in Program F-prime (FTD-0086).
 *
 * F-prime measured the commutator [E_f, E_g] concentrated on the matching
 * plaquette P_fg (3/3 pairs, 40x signal/off-axis). To upgrade from
 * "matching-bivector signature" to "Cl(3,0) bivector closure", three
 * additional tests:
 *
 *   PART A — Multi-seed robustness:
 *     Re-run F-prime's three off-diagonal pairs with N_seeds=8 different
 *     RNG seeds. Report mean ± stdev of the matching-plaquette commutator
 *     and the off-axis leakage. Confirms the signal is structural, not a
 *     seed artifact.
 *
 *   PART B — Casimir uniformity (e_i^2 = 1 scalar grade test):
 *     For each axis i, inject (i, i) sequence and measure scalar observable
 *     S = sum_x |J(x)|^2. In Cl(3,0): e_i e_i = +1 (scalar grade only).
 *     Test: S_x, S_y, S_z should be approximately equal (axis-isotropic
 *     Casimir).
 *
 *   PART D — Bivector commutator closure (the SU(2) test):
 *     [B_xy, B_yz] should be proportional to B_xz (the third bivector)
 *     under su(2) closure with structure constants epsilon_{abc}.
 *     Operationally: B_a = (1/2)[E_i, E_j] is a 2-injection difference, so
 *     [B_xy, B_yz] is an 8-sequence 4-injection linear combination. Each
 *     sequence: 4 WH injections in specified order, 1 tick between each.
 *     Project final state on the three plaquette bivectors. SU(2) closure
 *     predicts mass concentrated on P_xz with sign ε_{xy,yz,xz} ~ -1.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <array>
#include <vector>
#include <algorithm>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

namespace {

inline int chi(int v_mask, int x, int y, int z) {
    int dot = ((v_mask >> 0) & 1) * x
            + ((v_mask >> 1) & 1) * y
            + ((v_mask >> 2) & 1) * z;
    return (dot & 1) ? -1 : +1;
}

double plaquette_bivector(const std::vector<ftd::Voxel>& vox, int L,
                          int axis_i, int axis_j) {
    auto comp = [](const ftd::Vec3& v, int a) -> double {
        return a == 0 ? v.x : a == 1 ? v.y : v.z;
    };
    auto idx = [L](int x, int y, int z) {
        return x * L * L + y * L + z;
    };
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int s0 = idx(x, y, z);
        int x_i = x, y_i = y, z_i = z;
        if (axis_i == 0) x_i = (x + 1) % 2;
        if (axis_i == 1) y_i = (y + 1) % 2;
        if (axis_i == 2) z_i = (z + 1) % 2;
        const int s_i = idx(x_i, y_i, z_i);
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
    }
    return sum;
}

double scalar_flux_squared(const std::vector<ftd::Voxel>& vox, int L) {
    double sum = 0.0;
    for (int x = 0; x < 2; ++x)
    for (int y = 0; y < 2; ++y)
    for (int z = 0; z < 2; ++z) {
        const int i = x * L * L + y * L + z;
        const auto& f = vox[i].flux;
        sum += f.x * f.x + f.y * f.y + f.z * f.z;
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
    t.dual_substrate    = true;
    t.triad_binding     = true;
    t.color_forces      = true;
}

// Run a multi-injection sequence and return final flux configuration.
struct Outcome {
    double P_xy;
    double P_xz;
    double P_yz;
    double S;  // scalar |J|^2
};

Outcome run_sequence(int L, double A, unsigned seed,
                     const std::vector<int>& axes) {
    ftd::RenderBridge rb(L);
    enable_full_nonlocal(rb.toggles);
    rb.seed_rng(seed);
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    for (int axis : axes) {
        inject_wh_mode(rb, w1_mask[axis], axis, A);
        rb.run(1);
    }
    const auto& vox = rb.voxels();
    Outcome out;
    out.P_xy = plaquette_bivector(vox, L, 0, 1);
    out.P_xz = plaquette_bivector(vox, L, 0, 2);
    out.P_yz = plaquette_bivector(vox, L, 1, 2);
    out.S    = scalar_flux_squared(vox, L);
    return out;
}

double mean(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return v.empty() ? 0.0 : s / static_cast<double>(v.size());
}

double stdev(const std::vector<double>& v) {
    const double m = mean(v);
    double ss = 0.0;
    for (double x : v) ss += (x - m) * (x - m);
    return v.size() > 1 ? std::sqrt(ss / static_cast<double>(v.size() - 1)) : 0.0;
}

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Program F-double-prime: Bivector Algebra Closure Tests\n");
    std::printf("================================================================\n\n");

    const int    L = 8;
    const double A = 10.0;
    const std::array<unsigned, 8> seeds = {
        0xF3170517u, 0xF3170518u, 0xF3170519u, 0xF317051Au,
        0xF317051Bu, 0xF317051Cu, 0xF317051Du, 0xF317051Eu
    };

    // ==============================================================
    // PART A: Multi-seed robustness on F-prime commutators
    // ==============================================================
    std::printf("--- Part A: Multi-seed robustness (N_seeds=%zu) ---\n",
                seeds.size());
    std::printf("  Pair (i,j) | mean(matching) +/- stdev   |   mean(off-axis max) +/- stdev\n");
    std::printf("  -----------+------------------------------+------------------------------\n");

    struct PairKey { int i, j, ax_match; };
    const std::array<PairKey, 3> pairs = {{
        {0, 1, 0},  // (x, y) -> P_xy
        {0, 2, 1},  // (x, z) -> P_xz
        {1, 2, 2}   // (y, z) -> P_yz
    }};
    const char* match_name[3] = {"P_xy", "P_xz", "P_yz"};

    int robust_signal_pairs = 0;
    for (const auto& pk : pairs) {
        std::vector<double> match_vals, off_vals;
        for (unsigned seed : seeds) {
            Outcome fwd = run_sequence(L, A, seed,
                                        {pk.i, pk.j});
            Outcome bwd = run_sequence(L, A, seed,
                                        {pk.j, pk.i});
            const double pl[3] = {
                fwd.P_xy - bwd.P_xy,
                fwd.P_xz - bwd.P_xz,
                fwd.P_yz - bwd.P_yz
            };
            const double matched = pl[pk.ax_match];
            double off_max = 0.0;
            for (int a = 0; a < 3; ++a) {
                if (a == pk.ax_match) continue;
                off_max = std::max(off_max, std::abs(pl[a]));
            }
            match_vals.push_back(matched);
            off_vals.push_back(off_max);
        }
        const double m_mean = mean(match_vals);
        const double m_std  = stdev(match_vals);
        const double o_mean = mean(off_vals);
        const double o_std  = stdev(off_vals);
        std::printf("  (%d,%d)->%-3s | %+8.3f +/- %6.3f       |   %+8.3f +/- %6.3f\n",
                    pk.i + 1, pk.j + 1, match_name[pk.ax_match],
                    m_mean, m_std, o_mean, o_std);
        if (std::abs(m_mean) > 1.0 && std::abs(m_mean) > 3.0 * o_mean) {
            ++robust_signal_pairs;
        }
    }
    std::printf("\n  Pairs with robust matching > 3 x off-axis (across seeds): %d / 3\n",
                robust_signal_pairs);

    // ==============================================================
    // PART B: Casimir uniformity (diagonal scalar grade)
    // ==============================================================
    std::printf("\n--- Part B: Casimir uniformity test (e_i^2 = scalar grade) ---\n");
    std::printf("    Inject (i, i), measure scalar S = sum_x |J(x)|^2\n");
    std::printf("    Cl(3,0) e_i^2 = +1 implies S_x = S_y = S_z (axis-isotropic).\n\n");

    std::vector<double> S_x, S_y, S_z;
    for (unsigned seed : seeds) {
        S_x.push_back(run_sequence(L, A, seed, {0, 0}).S);
        S_y.push_back(run_sequence(L, A, seed, {1, 1}).S);
        S_z.push_back(run_sequence(L, A, seed, {2, 2}).S);
    }
    const double sx_m = mean(S_x), sx_s = stdev(S_x);
    const double sy_m = mean(S_y), sy_s = stdev(S_y);
    const double sz_m = mean(S_z), sz_s = stdev(S_z);
    std::printf("  S_x = %.3f +/- %.3f\n", sx_m, sx_s);
    std::printf("  S_y = %.3f +/- %.3f\n", sy_m, sy_s);
    std::printf("  S_z = %.3f +/- %.3f\n", sz_m, sz_s);
    const double s_avg = (sx_m + sy_m + sz_m) / 3.0;
    const double s_max_dev = std::max({
        std::abs(sx_m - s_avg), std::abs(sy_m - s_avg), std::abs(sz_m - s_avg)
    });
    const double s_isotropy = s_avg > 0 ? s_max_dev / s_avg : 1.0;
    std::printf("  Mean S = %.3f, max |S_i - <S>| / <S> = %.3f%s\n",
                s_avg, s_isotropy,
                (s_isotropy < 0.1) ? "  (PASS: axis-isotropic)"
                                    : "  (FAIL: anisotropic)");

    // ==============================================================
    // PART D: Bivector commutator [B_xy, B_yz] -- closure test
    // ==============================================================
    std::printf("\n--- Part D: Bivector commutator closure ---\n");
    std::printf("  Test: [B_xy, B_yz] proportional to B_xz?\n");
    std::printf("    B_xy = (1/2)[E_x, E_y] = (1/2)(E_x E_y - E_y E_x)\n");
    std::printf("    [B_xy, B_yz] = sum over 8 four-injection sequences:\n");
    std::printf("      +(x,y,y,z) -(x,y,z,y) -(y,x,y,z) +(y,x,z,y)\n");
    std::printf("      -(y,z,x,y) +(y,z,y,x) +(z,y,x,y) -(z,y,y,x)\n");
    std::printf("    Cl(3,0) closure: result proportional to B_xz, on plaquette P_xz.\n\n");

    // Full set of 4-axis sequences and their signs, matched by triple commutator
    // structure for [B_a, B_b] = (1/4) * sum (combinations).
    // All cyclic triples (a,b,c) of bivectors: B_xy(0,1), B_yz(1,2), B_zx(2,0)
    // We test [B_a, B_b] for each cyclic triple and compare to B_c.

    struct ClosureTest {
        const char* name;
        int a_i, a_j;  // first bivector axes
        int b_i, b_j;  // second bivector axes
        int c_i, c_j;  // expected output bivector axes
    };
    // [B_xy, B_yz] = ? B_xz   (axes: a=(0,1), b=(1,2), c=(0,2))
    // [B_yz, B_zx] = ? B_yx = -B_xy   (axes: a=(1,2), b=(0,2), c=(0,1))
    // [B_zx, B_xy] = ? B_zy = -B_yz   (axes: a=(0,2), b=(0,1), c=(1,2))
    const std::array<ClosureTest, 3> ctests = {{
        { "[B_xy, B_yz]", 0, 1, 1, 2, 0, 2 },
        { "[B_xz, B_xy]", 0, 2, 0, 1, 1, 2 },
        { "[B_yz, B_xz]", 1, 2, 0, 2, 0, 1 }
    }};

    int closure_pairs_pass = 0;

    for (const auto& ct : ctests) {
        // Build the 8 sequences for [B_{a_i, a_j}, B_{b_i, b_j}].
        // Using B_a = (1/2)(E_{a_i} E_{a_j} - E_{a_j} E_{a_i}):
        // [B_a, B_b] = B_a B_b - B_b B_a
        //   = (1/4)[ (E_ai E_aj - E_aj E_ai)(E_bi E_bj - E_bj E_bi)
        //          - (E_bi E_bj - E_bj E_bi)(E_ai E_aj - E_aj E_ai) ]
        struct Seq {
            std::array<int, 4> axes;
            int sign;
        };
        const Seq seq[8] = {
            // +B_a B_b expansion (4 terms, signs +,-,-,+)
            { { ct.a_i, ct.a_j, ct.b_i, ct.b_j }, +1 },
            { { ct.a_i, ct.a_j, ct.b_j, ct.b_i }, -1 },
            { { ct.a_j, ct.a_i, ct.b_i, ct.b_j }, -1 },
            { { ct.a_j, ct.a_i, ct.b_j, ct.b_i }, +1 },
            // -B_b B_a expansion (4 terms, signs -,+,+,-)
            { { ct.b_i, ct.b_j, ct.a_i, ct.a_j }, -1 },
            { { ct.b_i, ct.b_j, ct.a_j, ct.a_i }, +1 },
            { { ct.b_j, ct.b_i, ct.a_i, ct.a_j }, +1 },
            { { ct.b_j, ct.b_i, ct.a_j, ct.a_i }, -1 }
        };

        // Per-seed accumulator for the linear combination on each plaquette.
        std::vector<double> commutator_vals_xy, commutator_vals_xz, commutator_vals_yz;
        for (unsigned seed : seeds) {
            double acc[3] = {0, 0, 0};  // P_xy, P_xz, P_yz
            for (const auto& s : seq) {
                Outcome o = run_sequence(L, A, seed,
                                          { s.axes[0], s.axes[1],
                                            s.axes[2], s.axes[3] });
                acc[0] += s.sign * o.P_xy;
                acc[1] += s.sign * o.P_xz;
                acc[2] += s.sign * o.P_yz;
            }
            const double scale = 0.25;  // 1/4 prefactor
            commutator_vals_xy.push_back(scale * acc[0]);
            commutator_vals_xz.push_back(scale * acc[1]);
            commutator_vals_yz.push_back(scale * acc[2]);
        }
        const double m_xy = mean(commutator_vals_xy), s_xy = stdev(commutator_vals_xy);
        const double m_xz = mean(commutator_vals_xz), s_xz = stdev(commutator_vals_xz);
        const double m_yz = mean(commutator_vals_yz), s_yz = stdev(commutator_vals_yz);

        // Determine the "expected" plaquette index from c_i, c_j
        int c_idx = -1;
        if (ct.c_i == 0 && ct.c_j == 1) c_idx = 0;  // P_xy
        if (ct.c_i == 0 && ct.c_j == 2) c_idx = 1;  // P_xz
        if (ct.c_i == 1 && ct.c_j == 2) c_idx = 2;  // P_yz

        const double mvals[3] = {m_xy, m_xz, m_yz};
        const double svals[3] = {s_xy, s_xz, s_yz};
        const char* names[3]  = {"P_xy", "P_xz", "P_yz"};
        std::printf("  %s   (expected dominant: %s)\n", ct.name, names[c_idx]);
        for (int a = 0; a < 3; ++a) {
            std::printf("     %s = %+8.3f +/- %.3f%s\n",
                        names[a], mvals[a], svals[a],
                        a == c_idx ? "   <-- expected dominant" : "");
        }
        const double signal = std::abs(mvals[c_idx]);
        double off = 0.0;
        for (int a = 0; a < 3; ++a) {
            if (a == c_idx) continue;
            off = std::max(off, std::abs(mvals[a]));
        }
        const bool ok = (signal > 1.0) && (signal > 2.0 * off);
        std::printf("     signal = %.3f, max_off = %.3f  --> %s\n\n",
                    signal, off, ok ? "CLOSES" : "no closure");
        if (ok) ++closure_pairs_pass;
    }

    std::printf("  Bivector closure pairs (signal > 2*off): %d / 3\n",
                closure_pairs_pass);

    // ==============================================================
    // Verdict
    // ==============================================================
    std::printf("\n================================================================\n");
    std::printf("  Program F-double-prime Verdict\n");
    std::printf("================================================================\n");
    std::printf("  Part A: matching-bivector signal robust across %zu seeds: %d/3\n",
                seeds.size(), robust_signal_pairs);
    std::printf("  Part B: scalar Casimir axis-isotropy: %.3f%%  (PASS if < 10%%)\n",
                100.0 * s_isotropy);
    std::printf("  Part D: bivector commutator [B_a, B_b] closes on B_c: %d/3\n",
                closure_pairs_pass);

    const bool A_ok = (robust_signal_pairs == 3);
    const bool B_ok = (s_isotropy < 0.10);
    const bool D_ok = (closure_pairs_pass >= 2);
    const int  passes = (int)A_ok + (int)B_ok + (int)D_ok;

    std::printf("\n  %d/3 closure tests PASS\n\n", passes);
    if (passes == 3) {
        std::printf("  ==> Plaquette bivectors close su(2)/so(3) algebra.\n");
        std::printf("  ==> Cl(3,0) bivector subalgebra confirmed in FTD native dynamics.\n");
        std::printf("  ==> Fermion emergence: BRANCH-A DERIVATION on bivector basis.\n");
    } else if (passes == 2) {
        std::printf("  ==> Strong partial closure. Two of three properties verified.\n");
        std::printf("  ==> Algebra is su(2)-like up to caveats. Branch-A path open.\n");
    } else if (passes == 1) {
        std::printf("  ==> Weak partial closure. F-prime matching signature stands\n");
        std::printf("      but full su(2) closure not verified.\n");
    } else {
        std::printf("  ==> Closure tests FAIL. F-prime signal may not extend to a Lie\n");
        std::printf("      algebra structure. Re-examine the bivector identification.\n");
    }
    std::printf("================================================================\n");
    return 0;
}

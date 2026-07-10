/**
 * @file test_bivector_closure_v2.cpp
 * @brief M2 (FTD-0380) — noise-controlled bivector closure re-test.
 *
 * Pre-registered in PREREG_VERTEX_DK_CLOSURE_v1.md §3 — expectations locked
 * before this runner produced any output.
 *
 * Program F-double-prime (FTD-0087) found the iterated bivector commutator
 * [B_a, B_b] does not concentrate on B_c at the 4-injection scale. Path 1
 * (FTD-0088) reinterpreted that failure as 4-injection dynamical noise and
 * named the decisive discriminator (§3.5.3): re-run with time-averaged
 * readouts, larger L, lower A. This runner executes exactly that sweep:
 *
 *   L ∈ {8, 16, 32}  ×  A ∈ {1, 3, 10}  ×  readout ∈ {instant, time-avg}
 *   16 seeds (the 8 FTD-0087 seeds + 8 new), 3 cyclic triples,
 *   8 signed 4-injection sequences per triple, prefactor 1/4.
 *
 * The Part D machinery (sequence table, signs, block-toroidal 2^3 plaquette
 * readout, dual_substrate toggle set) replicates test_bivector_closure.cpp
 * verbatim for comparability with FTD-0087.
 *
 * Outcomes (pre-registered): CLOSURE-RECOVERED / CLOSURE-TREND /
 * CLOSURE-ROBUST-FAIL / UNDETERMINED. Includes the baseline replication
 * check: the (L=8, A=10, instant) cell restricted to the original 8 seeds
 * must qualitatively reproduce FTD-0087's no-closure result, else engine
 * drift is reported before any interpretation.
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

// Block-toroidal plaquette readout on the 2^3 corner block — replicates
// test_bivector_closure.cpp exactly (shifts are mod 2 within the block).
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

// FTD-0087 toggle set — replicated verbatim (incl. dual_substrate; the
// exchange_force/strong_force variant belongs to FTD-0088's test, disclosed
// in PREREG §2.4).
void enable_f2prime_toggles(ftd::TermToggles& t) {
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

constexpr int N_EXTRA = 4;   // extra ticks for the time-averaged readout

// Run one 4-injection sequence; return plaquette readouts at sequence end
// (snapshot 0) and after each of N_EXTRA further ticks (snapshots 1..4).
struct Snapshots {
    double P[N_EXTRA + 1][3];   // [snapshot][plaquette 0=xy,1=xz,2=yz]
};
Snapshots run_sequence(int L, double A, unsigned seed,
                       const std::array<int, 4>& axes) {
    ftd::RenderBridge rb(L);
    enable_f2prime_toggles(rb.toggles);
    rb.seed_rng(seed);
    const std::array<int, 3> w1_mask = {0b001, 0b010, 0b100};
    for (int axis : axes) {
        inject_wh_mode(rb, w1_mask[axis], axis, A);
        rb.run(1);
    }
    Snapshots out{};
    auto read = [&](int snap) {
        const auto& vox = rb.voxels();
        out.P[snap][0] = plaquette_bivector(vox, L, 0, 1);
        out.P[snap][1] = plaquette_bivector(vox, L, 0, 2);
        out.P[snap][2] = plaquette_bivector(vox, L, 1, 2);
    };
    read(0);
    for (int e = 1; e <= N_EXTRA; ++e) {
        rb.run(1);
        read(e);
    }
    return out;
}

double mean(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return v.empty() ? 0.0 : s / static_cast<double>(v.size());
}
double sem(const std::vector<double>& v) {
    if (v.size() < 2) return 0.0;
    const double m = mean(v);
    double ss = 0.0;
    for (double x : v) ss += (x - m) * (x - m);
    return std::sqrt(ss / static_cast<double>(v.size() - 1))
         / std::sqrt(static_cast<double>(v.size()));
}

struct ClosureTest {
    const char* name;
    int a_i, a_j;   // first bivector axes
    int b_i, b_j;   // second bivector axes
    int c_idx;      // expected output plaquette index (0=xy,1=xz,2=yz)
};

struct TripleStat {
    double m_exp;      // mean on expected plaquette
    double sem_exp;    // stderr on expected plaquette
    double off;        // max |mean| on the other two
    double r;          // concentration ratio
    bool   closes;
};

} // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  M2 (FTD-0380) - noise-controlled bivector closure re-test\n");
    std::printf("  PREREG_VERTEX_DK_CLOSURE_v1.md S3 - locked before first run\n");
    std::printf("================================================================\n\n");

    const std::array<int, 3>    Ls = {8, 16, 32};
    const std::array<double, 3> As = {1.0, 3.0, 10.0};
    const std::array<unsigned, 16> seeds = {
        // the 8 FTD-0087 seeds
        0xF3170517u, 0xF3170518u, 0xF3170519u, 0xF317051Au,
        0xF317051Bu, 0xF317051Cu, 0xF317051Du, 0xF317051Eu,
        // 8 new seeds
        0xF3170617u, 0xF3170618u, 0xF3170619u, 0xF317061Au,
        0xF317061Bu, 0xF317061Cu, 0xF317061Du, 0xF317061Eu
    };

    // [B_a, B_b] expansion into 8 signed 4-injection sequences — verbatim
    // from test_bivector_closure.cpp Part D.
    const std::array<ClosureTest, 3> ctests = {{
        { "[B_xy, B_yz]", 0, 1, 1, 2, 1 },   // expected P_xz
        { "[B_xz, B_xy]", 0, 2, 0, 1, 2 },   // expected P_yz
        { "[B_yz, B_xz]", 1, 2, 0, 2, 0 }    // expected P_xy
    }};
    const char* plaq_name[3] = {"P_xy", "P_xz", "P_yz"};
    const char* ro_name[2]   = {"instant", "time-avg"};

    // stats[L][A][readout][triple]
    TripleStat stats[3][3][2][3];
    // per-seed instant values for the baseline replication check
    std::vector<double> base_exp[3], base_off[3];   // per triple, first 8 seeds

    for (int li = 0; li < 3; ++li) {
        for (int ai = 0; ai < 3; ++ai) {
            const int    L = Ls[li];
            const double A = As[ai];
            std::printf("--- Cell L=%d, A=%.0f (%zu seeds x 3 triples x 8 sequences) ---\n",
                        L, A, seeds.size());

            for (int t = 0; t < 3; ++t) {
                const ClosureTest& ct = ctests[t];
                struct Seq { std::array<int, 4> axes; int sign; };
                const Seq seq[8] = {
                    { { ct.a_i, ct.a_j, ct.b_i, ct.b_j }, +1 },
                    { { ct.a_i, ct.a_j, ct.b_j, ct.b_i }, -1 },
                    { { ct.a_j, ct.a_i, ct.b_i, ct.b_j }, -1 },
                    { { ct.a_j, ct.a_i, ct.b_j, ct.b_i }, +1 },
                    { { ct.b_i, ct.b_j, ct.a_i, ct.a_j }, -1 },
                    { { ct.b_i, ct.b_j, ct.a_j, ct.a_i }, +1 },
                    { { ct.b_j, ct.b_i, ct.a_i, ct.a_j }, +1 },
                    { { ct.b_j, ct.b_i, ct.a_j, ct.a_i }, -1 }
                };

                // per-seed commutator projections, per readout mode
                std::vector<double> vals[2][3];   // [readout][plaquette]

                for (size_t si = 0; si < seeds.size(); ++si) {
                    double acc_inst[3] = {0, 0, 0};
                    double acc_avg [3] = {0, 0, 0};
                    for (const auto& s : seq) {
                        Snapshots sn = run_sequence(L, A, seeds[si], s.axes);
                        for (int a = 0; a < 3; ++a) {
                            acc_inst[a] += s.sign * sn.P[0][a];
                            double avg = 0.0;
                            for (int e = 0; e <= N_EXTRA; ++e) avg += sn.P[e][a];
                            acc_avg[a] += s.sign * (avg / (N_EXTRA + 1));
                        }
                    }
                    for (int a = 0; a < 3; ++a) {
                        vals[0][a].push_back(0.25 * acc_inst[a]);
                        vals[1][a].push_back(0.25 * acc_avg [a]);
                    }
                    // baseline replication capture: L=8, A=10, instant, first 8 seeds
                    if (L == 8 && A == 10.0 && si < 8) {
                        const double e = 0.25 * acc_inst[ct.c_idx];
                        double omax = 0.0;
                        for (int a = 0; a < 3; ++a) {
                            if (a == ct.c_idx) continue;
                            omax = std::max(omax, std::abs(0.25 * acc_inst[a]));
                        }
                        base_exp[t].push_back(e);
                        base_off[t].push_back(omax);
                    }
                }

                for (int ro = 0; ro < 2; ++ro) {
                    TripleStat st{};
                    st.m_exp   = mean(vals[ro][ct.c_idx]);
                    st.sem_exp = sem (vals[ro][ct.c_idx]);
                    st.off = 0.0;
                    for (int a = 0; a < 3; ++a) {
                        if (a == ct.c_idx) continue;
                        st.off = std::max(st.off, std::abs(mean(vals[ro][a])));
                    }
                    const double sig = std::abs(st.m_exp);
                    st.closes = (sig > 2.0 * st.off) && (sig > 2.0 * st.sem_exp);
                    st.r = sig / std::max(std::max(st.off, st.sem_exp), 1e-12);
                    stats[li][ai][ro][t] = st;

                    std::printf("  %-13s %-8s -> %s = %+9.4f +/- %7.4f  off=%8.4f  r=%6.2f  %s\n",
                                ct.name, ro_name[ro], plaq_name[ct.c_idx],
                                st.m_exp, st.sem_exp, st.off, st.r,
                                st.closes ? "CLOSES" : "no");
                }
            }
            std::printf("\n");
        }
    }

    // ------------------------------------------- baseline replication check
    std::printf("--- Baseline replication check (L=8, A=10, instant, 8 FTD-0087 seeds) ---\n");
    int base_closures = 0;
    for (int t = 0; t < 3; ++t) {
        const double m  = mean(base_exp[t]);
        const double se = sem (base_exp[t]);
        const double of = mean(base_off[t]);
        const bool closes = (std::abs(m) > 2.0 * of) && (std::abs(m) > 2.0 * se);
        std::printf("  %s: expected = %+8.3f +/- %6.3f, mean off = %8.3f  -> %s\n",
                    ctests[t].name, m, se, of, closes ? "CLOSES" : "no closure");
        if (closes) ++base_closures;
    }
    const bool baseline_ok = (base_closures <= 1);
    std::printf("  FTD-0087 reported 0/3 closure at this cell. Reproduced closure count: %d/3 -> %s\n\n",
                base_closures,
                baseline_ok ? "consistent (no drift)"
                            : "** ENGINE DRIFT since FTD-0087 - re-base before interpreting **");

    // ------------------------------------------------------------- verdict
    // 1. Any cell closing 3/3?
    struct ClosedCell { int li, ai, ro; };
    std::vector<ClosedCell> closed_cells;
    for (int li = 0; li < 3; ++li)
    for (int ai = 0; ai < 3; ++ai)
    for (int ro = 0; ro < 2; ++ro) {
        int n = 0;
        for (int t = 0; t < 3; ++t) if (stats[li][ai][ro][t].closes) ++n;
        if (n == 3) closed_cells.push_back({li, ai, ro});
    }

    // 2. Monotone improvement with decreasing A (columns: L x readout)
    int a_trend_cols = 0;
    for (int li = 0; li < 3; ++li)
    for (int ro = 0; ro < 2; ++ro) {
        int triples_ordered = 0;
        for (int t = 0; t < 3; ++t) {
            const double r1 = stats[li][0][ro][t].r;   // A=1
            const double r3 = stats[li][1][ro][t].r;   // A=3
            const double r10 = stats[li][2][ro][t].r;  // A=10
            if (r1 > r3 && r3 > r10) ++triples_ordered;
        }
        if (triples_ordered >= 2) ++a_trend_cols;
    }
    // 3. Monotone improvement with increasing L (columns: A x readout)
    int l_trend_cols = 0;
    for (int ai = 0; ai < 3; ++ai)
    for (int ro = 0; ro < 2; ++ro) {
        int triples_ordered = 0;
        for (int t = 0; t < 3; ++t) {
            const double r8  = stats[0][ai][ro][t].r;
            const double r16 = stats[1][ai][ro][t].r;
            const double r32 = stats[2][ai][ro][t].r;
            if (r32 > r16 && r16 > r8) ++triples_ordered;
        }
        if (triples_ordered >= 2) ++l_trend_cols;
    }

    std::printf("================================================================\n");
    std::printf("  M2 Verdict (pre-registered criteria, PREREG S3.3)\n");
    std::printf("================================================================\n");
    std::printf("  Cells closing 3/3: %zu / 18\n", closed_cells.size());
    for (const auto& c : closed_cells) {
        std::printf("    - L=%d, A=%.0f, %s\n", Ls[c.li], As[c.ai], ro_name[c.ro]);
    }
    std::printf("  A-monotone columns (r improves as A drops, >=2/3 triples): %d / 6\n",
                a_trend_cols);
    std::printf("  L-monotone columns (r improves as L grows, >=2/3 triples): %d / 6\n",
                l_trend_cols);

    bool recovered = false;
    for (const auto& c : closed_cells) {
        const bool noise_controlled = (As[c.ai] < 10.0) || (Ls[c.li] > 8) || (c.ro == 1);
        if (noise_controlled) recovered = true;
    }
    const bool trend = (a_trend_cols >= 4) || (l_trend_cols >= 4);

    const char* verdict;
    if (recovered) {
        verdict = "CLOSURE-RECOVERED";
        std::printf("\n  ==> CLOSURE-RECOVERED: su(2) bivector closure holds in a\n");
        std::printf("      noise-controlled regime. FTD-0088's dynamical-noise\n");
        std::printf("      interpretation is CONFIRMED. [MEASURED]\n");
    } else if (trend) {
        verdict = "CLOSURE-TREND";
        std::printf("\n  ==> CLOSURE-TREND: no cell closes 3/3 but concentration\n");
        std::printf("      improves monotonically under noise control. Approximate\n");
        std::printf("      algebra with controllable deviation (FTD-0087 Path 2).\n");
    } else if (closed_cells.empty()) {
        verdict = "CLOSURE-ROBUST-FAIL";
        std::printf("\n  ==> CLOSURE-ROBUST-FAIL: no closure and no monotone\n");
        std::printf("      improvement under any control. The 4-injection failure is\n");
        std::printf("      structural; FTD-0088's noise reinterpretation is REFUTED.\n");
    } else {
        verdict = "UNDETERMINED";
        std::printf("\n  ==> UNDETERMINED: pattern fits none of the pre-registered\n");
        std::printf("      outcomes cleanly (e.g. only the baseline cell closes).\n");
        std::printf("      Full per-cell table above; no tag movement.\n");
    }
    if (!baseline_ok) {
        std::printf("\n  NOTE: baseline replication FAILED - engine drift since\n");
        std::printf("  FTD-0087. The verdict above must be re-based before use.\n");
    }
    std::printf("\n  VERDICT: %s\n", verdict);
    std::printf("================================================================\n");
    return 0;
}

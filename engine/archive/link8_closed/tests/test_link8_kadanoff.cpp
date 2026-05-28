/**
 * @file test_link8_kadanoff.cpp
 * @brief Link 8 Candidate 1 — Kadanoff blocking vs master-quadratic recurrence.
 *
 * ============================================================================
 *  [CLOSED NEGATIVE 2026-04-20] — DO NOT LAND IN CI
 * ----------------------------------------------------------------------------
 *  The hypothesis this test probes — "the master quadratic is the
 *  characteristic polynomial of a Kadanoff RG step on the FTD engine" — was
 *  closed NEGATIVE on 2026-04-20. Three independent tests agreed for a
 *  structural reason: the engine's 18-point coupling stencil is (SC+FCC)/2,
 *  orthogonal to the BCC sub-stencil where 16 G*^2 = 2 pi 16 W_BCC lives.
 *
 *  The master quadratic itself remains a THEOREM at the number-theoretic
 *  layer (Gamma(1/4)^4 + CM-curve uniqueness). Only the *additional* RG-flow
 *  reading is retired. FTD-0001/0013/0014 are UNAFFECTED.
 *
 *  Authoritative closure memo:
 *    docs/theory/10_eft_program/AUDIT_LINK8_CLOSURE.md
 *  Ledger entry:  FTD-0050
 *
 *  This file is archived rather than deleted because the Kadanoff-blocking
 *  infrastructure it contains (ftd/eft/blocking.h plumbing, self+pair
 *  V(r) extraction on blocked lattices) is reusable for future matched-
 *  stencil beta-function work. The *framing* is dead; the *scaffolding* is not.
 * ============================================================================
 *
 * Target (from C:\Users\cpaci\Downloads\link8_candidate1_instructions.md):
 *     y_{n+1} = A * y_n + B * y_{n-1}
 *     A = 16 * G*^2 =  140.06013537449450489
 *     B = -16 * G*^3 = -414.39243772270942760
 *
 * Procedure:
 *   1. Run bare-lattice engine with charge configurations on a FINE lattice L_0.
 *   2. Block the steady-state J-field N_LEVELS times using a run-specific rule.
 *   3. At each level n, extract y_n = -slope(V(r), 1/r) from a linear fit of
 *      V_n(r) = E_pair_n(r) - E_self+_n - E_self-_n  vs  1/r.
 *   4. Report y_n values + recurrence residual / fitted coefficients.
 *
 * Output is RAW DATA. No pass/fail judgement. The analysis happens after the
 * run returns.
 *
 * Compile: part of the standard CMake target (test_link8_kadanoff).
 * Run:     $ engine\build\Release\test_link8_kadanoff.exe
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "ftd/eft/blocking.h"
#include "ftd/eft/coupling_measurement.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

namespace {

// ---- targets ---------------------------------------------------------------
constexpr double G_STAR = 2.958675119188638889;
constexpr double A_TARGET =  16.0 * G_STAR * G_STAR;           // 140.06013537449450489
constexpr double B_TARGET = -16.0 * G_STAR * G_STAR * G_STAR;  // -414.39243772270942760

// ---- knobs ----------------------------------------------------------------
constexpr int    FINE_L     = 64;   // starting lattice size
constexpr int    N_LEVELS   = 4;    // levels 0..3 -> L in {64, 32, 16, 8}
constexpr int    N_TICKS    = 300;  // engine ticks per fine simulation
constexpr double SEED_FLUX  = 0.05;

// ---- helpers ---------------------------------------------------------------

// Compute (1/2) sum |J|^2 over all voxels of a RenderBridge. This matches
// the self-consistent definition used by the existing measure_alpha_eff.
double field_energy_of(const ftd::RenderBridge& rb) {
    double s = 0.0;
    for (const auto& v : rb.voxels()) {
        s += 0.5 * (v.flux.x * v.flux.x + v.flux.y * v.flux.y + v.flux.z * v.flux.z);
    }
    return s;
}

// Run a bare-lattice simulation on L_fine with a single charge at the centre.
// Returns the final RenderBridge (heap-allocated) for subsequent blocking.
std::unique_ptr<ftd::RenderBridge> run_self_charge(int L_fine, int8_t sign) {
    auto rb = std::make_unique<ftd::RenderBridge>(L_fine);
    ftd::eft::configure_bare_lattice_for_coupling(*rb);
    const int mid = L_fine / 2;
    rb->inject_particle(mid, mid, mid, sign,
                        {0.0, 0.0, static_cast<double>(sign) * SEED_FLUX});
    rb->voxels()[rb->lattice().index(mid, mid, mid)].locked = true;
    rb->run(N_TICKS);
    return rb;
}

// Run a bare-lattice simulation with a +1/-1 pair along +x at separation r_f.
std::unique_ptr<ftd::RenderBridge> run_pair(int L_fine, int r_f) {
    auto rb = std::make_unique<ftd::RenderBridge>(L_fine);
    ftd::eft::configure_bare_lattice_for_coupling(*rb);
    const int mid = L_fine / 2;
    rb->inject_particle(mid, mid, mid, +1, {0.0, 0.0, SEED_FLUX});
    rb->voxels()[rb->lattice().index(mid, mid, mid)].locked = true;
    rb->inject_particle(mid + r_f, mid, mid, -1, {0.0, 0.0, -SEED_FLUX});
    rb->voxels()[rb->lattice().index(mid + r_f, mid, mid)].locked = true;
    rb->run(N_TICKS);
    return rb;
}

// Linear fit of V vs 1/r. Returns (slope, intercept, R^2, n).
struct LinFit { double slope = 0, intercept = 0, r2 = 0; int n = 0; };
LinFit fit_V_over_inv_r(const std::vector<std::pair<int, double>>& pts) {
    LinFit f;
    f.n = static_cast<int>(pts.size());
    if (f.n < 2) return f;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (const auto& p : pts) {
        const double x = 1.0 / static_cast<double>(p.first);
        const double y = p.second;
        sx += x; sy += y; sxx += x * x; sxy += x * y;
    }
    const double denom = f.n * sxx - sx * sx;
    if (std::abs(denom) < 1e-30) return f;
    f.slope = (f.n * sxy - sx * sy) / denom;
    f.intercept = (sy - f.slope * sx) / f.n;
    const double ybar = sy / f.n;
    double ss_tot = 0, ss_res = 0;
    for (const auto& p : pts) {
        const double x = 1.0 / static_cast<double>(p.first);
        const double y = p.second;
        const double yhat = f.intercept + f.slope * x;
        ss_tot += (y - ybar) * (y - ybar);
        ss_res += (y - yhat) * (y - yhat);
    }
    f.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
    return f;
}

// ---- blocking adaptors (one per run) --------------------------------------

// block_factor_2_Javg — Run 1: flux-average + charge-conserving state.
// Uses existing ftd::eft::block_full.
std::unique_ptr<ftd::RenderBridge> block_run1(const ftd::RenderBridge& src) {
    auto blocked = ftd::eft::block_full(src);
    // Transfer ownership of the inner RenderBridge out of BlockedRenderBridge.
    // We do this by moving voxels into a fresh RB of the coarse size.
    const int Lc = blocked->coarse_size();
    auto out = std::make_unique<ftd::RenderBridge>(Lc);
    // Deep-copy voxel state (flux + state) from blocked -> out.
    const auto& src_vox = blocked->bridge().voxels();
    auto& dst_vox = out->voxels();
    for (size_t i = 0; i < src_vox.size() && i < dst_vox.size(); ++i) {
        dst_vox[i].flux = src_vox[i].flux;
        dst_vox[i].state = src_vox[i].state;
    }
    return out;
}

// ---- one complete run ------------------------------------------------------

struct LevelData {
    int L = 0;
    std::vector<std::pair<int, double>> V_vs_r;  // (r_coarse, V) samples
    double alpha = 0.0;
    double r2 = 0.0;
    double E_self_pos = 0.0;
    double E_self_neg = 0.0;
};

struct RunResult {
    std::string tag;
    std::vector<LevelData> levels;
};

// Execute Run 1: J-field local averaging + charge-conserving state blocking.
// Strategy:
//   (a) One fine simulation per (config, r_f). Configs = self+, self-, pair@r_f.
//   (b) Blocking is applied IN-MEMORY to each steady-state bridge N_LEVELS-1
//       times. Energies are read at every level.
//   (c) At each level, V_n(r_c) = E_pair(r_f -> r_c at level n)
//                                 - E_self+(level n) - E_self-(level n).
RunResult run_kadanoff_Javg(const std::string& tag,
                            int L_fine, int n_levels,
                            const std::vector<int>& r_f_list)
{
    std::printf("\n========== RUN: %s ==========\n", tag.c_str());
    std::printf("FIELD: J (3-component flux)\n");
    std::printf("BLOCK_RULE: local 2x2x2 average (flux) + charge-conserving state\n");
    std::printf("BLOCK_FACTOR: 2\n");
    std::printf("COUPLING_EXTRACTION: V(r) = E_pair - E_self+ - E_self-, slope fit vs 1/r\n");
    std::printf("FINE_L: %d   N_LEVELS: %d   N_TICKS: %d   SEED_FLUX: %.3f\n",
                L_fine, n_levels, N_TICKS, SEED_FLUX);
    std::printf("r_f_list (fine-lattice separations):");
    for (int r : r_f_list) std::printf(" %d", r);
    std::printf("\n\n");

    RunResult out;
    out.tag = tag;
    out.levels.resize(n_levels);
    for (int n = 0; n < n_levels; ++n) out.levels[n].L = L_fine >> n;

    // Self-energy simulations (run once at fine L, block down).
    std::printf("  -- self-energy (+): ");
    std::fflush(stdout);
    {
        auto rb = run_self_charge(L_fine, +1);
        out.levels[0].E_self_pos = field_energy_of(*rb);
        std::printf("L=%d E=%.6e", L_fine, out.levels[0].E_self_pos);
        auto cur = std::move(rb);
        for (int n = 1; n < n_levels; ++n) {
            auto nxt = block_run1(*cur);
            out.levels[n].E_self_pos = field_energy_of(*nxt);
            std::printf("  |  L=%d E=%.6e", out.levels[n].L, out.levels[n].E_self_pos);
            cur = std::move(nxt);
        }
        std::printf("\n");
    }

    std::printf("  -- self-energy (-): ");
    std::fflush(stdout);
    {
        auto rb = run_self_charge(L_fine, -1);
        out.levels[0].E_self_neg = field_energy_of(*rb);
        std::printf("L=%d E=%.6e", L_fine, out.levels[0].E_self_neg);
        auto cur = std::move(rb);
        for (int n = 1; n < n_levels; ++n) {
            auto nxt = block_run1(*cur);
            out.levels[n].E_self_neg = field_energy_of(*nxt);
            std::printf("  |  L=%d E=%.6e", out.levels[n].L, out.levels[n].E_self_neg);
            cur = std::move(nxt);
        }
        std::printf("\n");
    }

    // Pair simulations for each r_f.
    for (int r_f : r_f_list) {
        std::printf("  -- pair r_f=%d: ", r_f);
        std::fflush(stdout);
        auto rb = run_pair(L_fine, r_f);
        auto cur = std::move(rb);
        for (int n = 0; n < n_levels; ++n) {
            const int r_c = r_f >> n;  // integer divide; tracks charge centre
            if (n > 0) {
                // block_run1 already applied to cur before we measured; but first
                // iteration (n=0) uses cur as-is. So block AFTER recording n=0.
            }
            const double E_pair = field_energy_of(*cur);
            const double V = E_pair - out.levels[n].E_self_pos
                                    - out.levels[n].E_self_neg;
            if (r_c >= 1) {
                out.levels[n].V_vs_r.emplace_back(r_c, V);
            }
            std::printf("  n=%d L=%d r=%d V=%.6e", n, out.levels[n].L, r_c, V);
            if (n + 1 < n_levels) {
                cur = block_run1(*cur);
            }
        }
        std::printf("\n");
    }

    // Fit alpha at each level.
    std::printf("\n  Linear fits V(r) vs 1/r (alpha = -slope):\n");
    for (int n = 0; n < n_levels; ++n) {
        // Consolidate identical r_c by averaging (r_c might repeat when r_f
        // differ by factor 2^n producing same r_c after integer division).
        std::vector<std::pair<int, double>> pts;
        std::vector<double> sums(L_fine, 0.0);
        std::vector<int>    cnts(L_fine, 0);
        for (const auto& p : out.levels[n].V_vs_r) {
            if (p.first >= 1 && p.first < L_fine) {
                sums[p.first] += p.second;
                cnts[p.first] += 1;
            }
        }
        for (int r = 1; r < L_fine; ++r) {
            if (cnts[r] > 0) pts.emplace_back(r, sums[r] / cnts[r]);
        }
        LinFit f = fit_V_over_inv_r(pts);
        out.levels[n].alpha = -f.slope;
        out.levels[n].r2    = f.r2;
        std::printf("    n=%d  L=%2d  n_pts=%d  alpha=%.6e  R^2=%.5f  y=1/alpha=%.6e\n",
                    n, out.levels[n].L, f.n, out.levels[n].alpha, f.r2,
                    (out.levels[n].alpha != 0.0 ? 1.0 / out.levels[n].alpha : 0.0));
    }
    std::fflush(stdout);
    return out;
}

// Report recurrence analysis on a RunResult.
void report_recurrence(const RunResult& res) {
    std::printf("\n  Recurrence analysis (target A=%.6f  B=%.6f):\n",
                A_TARGET, B_TARGET);

    // Extract y_n = 1/alpha_n (if alpha_n > 0). Also try y_n = alpha_n for
    // completeness — we don't know a priori which variable closes.
    std::vector<double> y_inv, y_raw;
    for (const auto& lv : res.levels) {
        if (lv.alpha != 0.0) {
            y_inv.push_back(1.0 / lv.alpha);
            y_raw.push_back(lv.alpha);
        } else {
            y_inv.push_back(0.0);
            y_raw.push_back(0.0);
        }
    }

    auto print_y = [](const char* name, const std::vector<double>& y) {
        std::printf("    y_n (%s):", name);
        for (size_t i = 0; i < y.size(); ++i) std::printf("  y_%zu=%.6e", i, y[i]);
        std::printf("\n");
    };
    print_y("1/alpha", y_inv);
    print_y("alpha  ", y_raw);

    auto residual_report = [](const std::vector<double>& y, const char* name) {
        std::printf("    === residual check (%s):\n", name);
        for (size_t n = 2; n < y.size(); ++n) {
            const double lhs = y[n];
            const double rhs = A_TARGET * y[n-1] + B_TARGET * y[n-2];
            const double abs_err = lhs - rhs;
            const double rel_err = (std::abs(rhs) > 1e-30) ? abs_err / rhs : 0.0;
            std::printf("      triple (y_%zu,y_%zu,y_%zu): y_n=%.6e  rhs=%.6e  "
                        "residual=%.6e  rel=%.2e\n",
                        n-2, n-1, n, lhs, rhs, abs_err, rel_err);
        }
    };
    residual_report(y_inv, "y = 1/alpha");
    residual_report(y_raw, "y = alpha  ");

    // Fit (A, B) if we have >= 4 values (2 equations, 2 unknowns).
    auto fit_AB = [](const std::vector<double>& y, const char* name) {
        if (y.size() < 4) return;
        // Equations: for k = 2..N-1:  y[k] = A*y[k-1] + B*y[k-2]
        // Take the first two equations for a direct 2x2 solve (k=2, k=3).
        // M = [[y[1], y[0]], [y[2], y[1]]],  rhs = [y[2], y[3]]
        const double m11 = y[1], m12 = y[0], m21 = y[2], m22 = y[1];
        const double det = m11 * m22 - m12 * m21;
        const double r1 = y[2], r2 = y[3];
        std::printf("    === 2-eq fit (%s)  det(M) = %.6e\n", name, det);
        if (std::abs(det) < 1e-30) {
            std::printf("      (singular; cannot fit)\n");
            return;
        }
        const double A_fit = ( m22 * r1 - m12 * r2) / det;
        const double B_fit = (-m21 * r1 + m11 * r2) / det;
        const double dA = 100.0 * (A_fit - A_TARGET) / A_TARGET;
        const double dB = 100.0 * (B_fit - B_TARGET) / B_TARGET;
        std::printf("      A_fit = %.6f  (target %.6f, dev = %.2f%%)\n",
                    A_fit, A_TARGET, dA);
        std::printf("      B_fit = %.6f  (target %.6f, dev = %.2f%%)\n",
                    B_fit, B_TARGET, dB);
    };
    fit_AB(y_inv, "y = 1/alpha");
    fit_AB(y_raw, "y = alpha  ");
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);  // line buffering so printf appears live
    std::printf("================================================================\n");
    std::printf("  Link 8 Candidate 1 -- Kadanoff blocking recurrence test\n");
    std::printf("  Target: y_{n+1} = %.6f * y_n + %.6f * y_{n-1}\n",
                A_TARGET, B_TARGET);
    std::printf("  Harness: true real-space Kadanoff blocking on fine engine state\n");
    std::printf("================================================================\n");

    // Run 1 literal (instructions §"Run 1"): L_fine = 8, 3 levels -> {8,4,2}.
    // r_f ∈ {2} only: r_f=4 is L/2 = boundary. r_c per level: {2,1,0}.
    // r_c=0 drops, so level-2 has no points.
    auto run1_lit = run_kadanoff_Javg("Run 1 (literal): L_fine=8, J-avg",
                                      8, 3, {2, 4});
    report_recurrence(run1_lit);

    // Run 6 literal (instructions §"Run 6"): L_fine=16 -> {16,8,4,2}, 4 pts.
    // r_f ∈ {4, 8}: r_c per level: {4,8} {2,4} {1,2} {0,1}.
    auto run6_lit = run_kadanoff_Javg("Run 6 (literal): L_fine=16, J-avg",
                                      16, 4, {4, 8});
    report_recurrence(run6_lit);

    // Run 1 extended: L_fine=64, 4 levels -> {64,32,16,8}. Beyond spec but
    // gives better separation of scales for the recurrence fit.
    auto run1_ext = run_kadanoff_Javg("Run 1 (extended): L_fine=64, J-avg",
                                      FINE_L, N_LEVELS, {8, 16, 24});
    report_recurrence(run1_ext);

    std::printf("\n");
    std::printf("================================================================\n");
    std::printf("  DONE. Raw data above. No pass/fail judgement issued.\n");
    std::printf("================================================================\n");
    return 0;
}

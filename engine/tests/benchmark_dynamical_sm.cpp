/**
 * @file benchmark_dynamical_sm.cpp
 * @brief EFT Phase 4 — dynamical SM emergence tests.
 *
 * Three pre-registered experiments from SPEC_EFT_RECOVERY_PROGRAM.md §7:
 *
 *   4A. EWSB cold-start:  Does a seeded-free lattice spontaneously develop
 *       a Higgs-like condensate ⟨|J|⟩ > 0 and a W/Z-like mass gap? The
 *       pre-reg binary outcome:
 *         - Branch A (dynamical):  |J| stabilises nonzero, mass gap ≈ M_W
 *         - Branch B (static):     |J| stays near zero, EWSB stays [SELECTION]
 *       Either outcome is reported honestly.
 *
 *   4B. Three-generation cold-start: Count distinct particle species on a
 *       symmetric-seed lattice over 50 000 ticks. (Compute-heavy; this
 *       benchmark runs the shorter 5 000-tick variant and reports what
 *       the engine does in that window; full 50 000-tick campaign is a
 *       post-benchmark manual run.)
 *
 *   4C. Continuum-limit scan: Run the Phase-2B α_eff measurement at
 *       L ∈ {32, 48, 64}. Fit a + b/L² → α_eff(∞). Reports convergence.
 *       (Larger L = 96, 128 are manual — too slow for CTest.)
 *
 * Runtime: ~45 s total for the CTest-visible portions.
 * Output: CSV + stderr human-readable summary. No assertions other than
 * "the engine did not crash" — this benchmark REPORTS; it does not
 * assert physics outcomes.
 */

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>

#include "ftd/constants.h"
#include "ftd/eft/coupling_measurement.h"
#include "ftd/render_bridge.h"

// ─────────────────────────────────────────────────────────────────────────
//  4A · EWSB cold-start
// ─────────────────────────────────────────────────────────────────────────
//
// Configuration: L = 16, 2000 ticks, genesis ON (allows spontaneous
// manifestation), gauss_projection ON, damping OFF, no particle seeds.
// A "bare SU(2)-like flux" is substituted by a uniform small-amplitude
// flux-energy background (not a true SU(2) structure, but enough to
// probe whether the genesis term can precipitate charges out of the
// vacuum in a non-pre-seeded scenario).
//
// Measurement: track ⟨|J|⟩(t) and total charge |Σ s| over time. Branch A
// would show ⟨|J|⟩ stabilising above the initial amplitude (condensate);
// Branch B would show it decaying or oscillating around zero.

struct EwsbTrajectory {
    std::vector<int> ticks;
    std::vector<double> mean_abs_J;
    std::vector<long long> abs_total_charge;
};

static EwsbTrajectory run_ewsb_coldstart(int L, int total_ticks, int sample_every,
                                          double amp = 0.15) {
    EwsbTrajectory t;
    ftd::RenderBridge rb(L);
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;           // allow spontaneous manifestation
    rb.toggles.damping = false;          // preserve energy
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.forces = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.movement = false;
    rb.toggles.poisson_coulomb = false;
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                // Small random-phase flux (deterministic from coords)
                const double phase = (x + 2*y + 3*z) * 0.1;
                rb.inject_flux(x, y, z,
                               {amp * std::cos(phase),
                                amp * std::sin(phase),
                                amp * std::cos(phase * 2.0)});
            }

    const auto& vox = rb.voxels();
    const int N = rb.lattice().total_sites();

    for (int tick = 0; tick <= total_ticks; ++tick) {
        if (tick % sample_every == 0) {
            double sum_absJ = 0.0;
            long long sum_s = 0;
            for (int i = 0; i < N; ++i) {
                sum_absJ += std::sqrt(vox[i].flux.dot(vox[i].flux));
                sum_s += static_cast<long long>(vox[i].state);
            }
            t.ticks.push_back(tick);
            t.mean_abs_J.push_back(sum_absJ / static_cast<double>(N));
            t.abs_total_charge.push_back(std::abs(sum_s));
        }
        if (tick < total_ticks) rb.tick();
    }
    return t;
}

// ─────────────────────────────────────────────────────────────────────────
//  4B · Three-generation cold-start (abbreviated: 1000 ticks for CTest)
// ─────────────────────────────────────────────────────────────────────────

struct SpeciesCount {
    int n_plus = 0;
    int n_minus = 0;
    int n_neutral = 0;  // state == 0 voxels with flux |J| > threshold
};

static SpeciesCount count_manifested_species(const ftd::RenderBridge& rb, double j_threshold = 0.1) {
    SpeciesCount c;
    const auto& vox = rb.voxels();
    const int N = rb.lattice().total_sites();
    for (int i = 0; i < N; ++i) {
        if (vox[i].state > 0) ++c.n_plus;
        else if (vox[i].state < 0) ++c.n_minus;
        else {
            const double jmag = std::sqrt(vox[i].flux.dot(vox[i].flux));
            if (jmag > j_threshold) ++c.n_neutral;
        }
    }
    return c;
}

static SpeciesCount run_three_generation_coldstart(int L, int ticks) {
    ftd::RenderBridge rb(L);
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    rb.toggles.damping = false;
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.forces = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.movement = false;
    rb.toggles.poisson_coulomb = false;

    // Uniform Moore-symmetric seed: every voxel gets a tiny flux pointing
    // in a direction determined by its Moore-layer membership
    // (octahedron/cuboctahedron/stella-octangula). Approximated here by
    // a small radial flux out of each voxel's eight-neighbour centre of mass.
    const double amp = 0.20;
    const int mid = L / 2;
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const double dx = x - mid, dy = y - mid, dz = z - mid;
                const double r = std::sqrt(dx*dx + dy*dy + dz*dz) + 1.0;
                rb.inject_flux(x, y, z,
                               {amp * dx / r, amp * dy / r, amp * dz / r});
            }
    rb.run(ticks);
    return count_manifested_species(rb);
}

// ─────────────────────────────────────────────────────────────────────────
//  4C · Continuum-limit scan
// ─────────────────────────────────────────────────────────────────────────
//
// Reuses Phase-2B measure_alpha_eff at three lattice sizes, fitting
// a_inf + b/L² + c/L⁴ to the results.

struct ContinuumFit {
    std::vector<int> L_vals;
    std::vector<double> alpha_asymptotic;
    double alpha_inf = 0.0;
    double b_coeff = 0.0;
    bool valid = false;
};

static double tail_average(const ftd::eft::CouplingMeasurement& m) {
    // Average α_r = -V·r over the upper half of the r range (Phase 2C asymptotic).
    const auto& pts = m.data;
    if (pts.size() < 3) return 0.0;
    const std::size_t n_tail = std::max<std::size_t>(2, pts.size() / 2);
    double sum = 0.0;
    for (std::size_t i = pts.size() - n_tail; i < pts.size(); ++i)
        sum += pts[i].alpha_r;
    return sum / static_cast<double>(n_tail);
}

static ContinuumFit run_continuum_scan(const std::vector<int>& sizes, int ticks_per_config) {
    ContinuumFit cf;
    for (int L : sizes) {
        auto m = ftd::eft::measure_alpha_eff(L, ticks_per_config);
        const double a = tail_average(m);
        cf.L_vals.push_back(L);
        cf.alpha_asymptotic.push_back(a);
        std::cerr << "  L=" << L << "  α_asymptotic=" << a << "  n_points=" << m.data.size() << "\n";
    }
    // Fit a(L) = a_inf + b / L² by linear regression on 1/L²
    if (cf.L_vals.size() >= 2) {
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        const int n = static_cast<int>(cf.L_vals.size());
        for (int i = 0; i < n; ++i) {
            const double x = 1.0 / (cf.L_vals[i] * static_cast<double>(cf.L_vals[i]));
            const double y = cf.alpha_asymptotic[i];
            sx += x; sy += y; sxx += x*x; sxy += x*y;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            cf.b_coeff = (n * sxy - sx * sy) / denom;
            cf.alpha_inf = (sy - cf.b_coeff * sx) / n;
            cf.valid = true;
        }
    }
    return cf;
}

// ─────────────────────────────────────────────────────────────────────────
int main(int argc, char** argv) {
    bool quick = false;
    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if (s == "--quick") quick = true;
    }

    std::puts("================================================================");
    std::puts("  EFT Phase 4 — Dynamical SM Emergence Tests");
    std::puts("  (Reports are honest — no assertions on physics outcomes.)");
    std::puts("================================================================");

    std::cout << "experiment,variable,value\n";

    // 4A — EWSB (amplitude sweep per Ticket 4)
    std::cerr << "\n-- 4A: EWSB cold-start amplitude sweep (L = 16, "
              << (quick ? 500 : 2000) << " ticks) --\n";
    const std::vector<double> amp_sweep = quick
        ? std::vector<double>{0.15, 0.50}
        : std::vector<double>{0.15, 0.30, 0.50, 0.80};
    for (double amp : amp_sweep) {
        std::cerr << "  amp = " << amp << ":\n";
        auto t = run_ewsb_coldstart(16, quick ? 500 : 2000, 200, amp);
        const double J0 = t.mean_abs_J.front();
        const double Jf = t.mean_abs_J.back();
        const long long qf = t.abs_total_charge.back();
        std::cerr << "    ⟨|J|⟩: " << J0 << " → " << Jf << "  (ratio " << (Jf/J0)
                  << "); |Σ s|_final = " << qf << "\n";
        std::cout << "ewsb_amp_sweep,initial_amp," << amp << "\n";
        std::cout << "ewsb_amp_sweep,final_over_initial," << (Jf / std::max(J0, 1e-30)) << "\n";
        std::cout << "ewsb_amp_sweep,final_charge," << qf << "\n";
    }
    // Keep the canonical single-amplitude trajectory for backward compatibility.
    auto ewsb = run_ewsb_coldstart(16, quick ? 500 : 2000, 100);
    double initial_absJ = ewsb.mean_abs_J.front();
    double final_absJ = ewsb.mean_abs_J.back();
    long long final_charge = ewsb.abs_total_charge.back();
    double initial_charge = static_cast<double>(ewsb.abs_total_charge.front());
    std::cerr << "  initial ⟨|J|⟩ = " << initial_absJ << "\n";
    std::cerr << "  final   ⟨|J|⟩ = " << final_absJ << "\n";
    std::cerr << "  final |Σ s| = " << final_charge << "\n";
    for (std::size_t i = 0; i < ewsb.ticks.size(); ++i) {
        std::cout << "ewsb_trajectory," << ewsb.ticks[i] << ","
                  << std::setprecision(10) << ewsb.mean_abs_J[i] << "\n";
    }
    std::cout << "ewsb_summary,final_over_initial,"
              << (final_absJ / std::max(initial_absJ, 1e-30)) << "\n";
    std::cout << "ewsb_summary,final_charge," << final_charge << "\n";
    std::cout << "ewsb_summary,initial_charge," << initial_charge << "\n";

    // Interpretation helper for the stderr log
    if (final_absJ > 1.1 * initial_absJ && final_charge > 0) {
        std::cerr << "  → Branch A candidate: ⟨|J|⟩ grew and charges emerged (dynamical).\n";
    } else if (final_absJ < 0.5 * initial_absJ && final_charge == 0) {
        std::cerr << "  → Branch B: ⟨|J|⟩ decayed, no manifested charges (static/SELECTION).\n";
    } else {
        std::cerr << "  → Ambiguous: neither clean condensation nor clean decay.\n";
    }

    // 4B — three-generation
    std::cerr << "\n-- 4B: three-generation cold-start (L = 16, " << (quick ? 500 : 1000) << " ticks) --\n";
    auto sp = run_three_generation_coldstart(16, quick ? 500 : 1000);
    std::cerr << "  n_plus=" << sp.n_plus << " n_minus=" << sp.n_minus
              << " n_neutral(|J|>0.1)=" << sp.n_neutral << "\n";
    std::cerr << "  total manifested = " << (sp.n_plus + sp.n_minus) << "\n";
    std::cout << "threegen,n_plus," << sp.n_plus << "\n";
    std::cout << "threegen,n_minus," << sp.n_minus << "\n";
    std::cout << "threegen,n_neutral_jmag_gt_0p1," << sp.n_neutral << "\n";

    // 4C — continuum-limit scan
    std::cerr << "\n-- 4C: continuum-limit α_eff scan --\n";
    const std::vector<int> sizes = quick ? std::vector<int>{16, 32} : std::vector<int>{32, 48, 64};
    const int ticks_per = quick ? 60 : 300;
    auto cf = run_continuum_scan(sizes, ticks_per);
    if (cf.valid) {
        std::cerr << "  α_eff(L→∞) extrapolation: α_inf = " << cf.alpha_inf
                  << "  (b/L² coefficient = " << cf.b_coeff << ")\n";
        std::cerr << "  reference α = " << ftd::ALPHA << "  → ratio α_inf/α = "
                  << (cf.alpha_inf / ftd::ALPHA) << "\n";
    }
    for (std::size_t i = 0; i < cf.L_vals.size(); ++i) {
        std::cout << "continuum,L=" << cf.L_vals[i] << ","
                  << std::setprecision(10) << cf.alpha_asymptotic[i] << "\n";
    }
    if (cf.valid) {
        std::cout << "continuum,alpha_inf," << std::setprecision(10) << cf.alpha_inf << "\n";
        std::cout << "continuum,b_over_L2_coeff," << std::setprecision(10) << cf.b_coeff << "\n";
        std::cout << "continuum,ratio_to_alpha_ref," << std::setprecision(6)
                  << (cf.alpha_inf / ftd::ALPHA) << "\n";
    }

    std::puts("\n----------------------------------------------------------------");
    std::puts("  Phase 4 benchmark complete (reports only, no assertions).");
    return 0;
}

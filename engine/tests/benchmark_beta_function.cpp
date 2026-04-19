/**
 * @file benchmark_beta_function.cpp
 * @brief EFT Phase 2C — lattice-measured β(g) via multi-scale α_eff extraction.
 *
 * This benchmark is the engine-side half of the β-function measurement.
 * The Python-side analysis (`scripts/benchmarks/measure_beta_function.py`)
 * consumes the CSV output this executable produces.
 *
 * Procedure
 * ---------
 * For each of three scales — L = 64 (canonical fine), L = 32, L = 16 —
 * we measure α_eff via the V(r) two-charge-potential probe (Phase 2B).
 * The three α_eff values at three scales give the coarse-grained RG
 * trajectory; β(g) = [g(scale/2) − g(scale)] / ln 2 evaluated at each
 * intermediate pair of points.
 *
 * IMPORTANT CAVEAT
 * ----------------
 * The canonical "blocking" procedure is `block_full(rb)`, which operates
 * on an *instantaneous snapshot* of the fine lattice (rb's voxels at a
 * given tick). But α_eff extraction needs to *run engine dynamics on
 * the blocked lattice*. Running engine ticks on a coarse-grained
 * configuration is a simplification — the coarse theory has a different
 * effective action and evolving it with the fine engine's rules is only
 * approximately correct.
 *
 * The practical interpretation of a β measured this way:
 *   - Fine (L=64):   α_eff(a)           measured directly at the fine scale
 *   - Coarse (L=32): α_eff(2a)          measured on an L=32 lattice run
 *                                        from the same canonical regime
 *   - Coarser (L=16): α_eff(4a)          measured on an L=16 lattice
 *
 * This is the "Monte Carlo renormalization group" approach in its
 * simplest form: we measure the coupling at three different UV cutoffs
 * and interpret scale dependence as β. True block-spin RG would run the
 * fine theory, block the configuration, and measure the blocked coupling
 * on the blocked configuration directly — which requires a variational
 * action match that the FTD engine does not yet support. For now we
 * report both approaches where they differ.
 *
 * Output (stdout, CSV)
 * --------------------
 *   col,  value
 *   method, (mcrg | blocked)          method tag
 *   L,     lattice size
 *   ticks, engine ticks per config
 *   r,     pair separation
 *   V,     V(r) data point
 *   alpha_r, -V*r (constant = alpha on clean Coulomb)
 *   alpha_fit, slope-fit alpha (one row per L with r="fit")
 *   r2,    fit R²
 *
 * Runtime: ~90 s for all 3 scales at default n_ticks = 300.
 */

#include <cmath>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>

#include "ftd/eft/blocking.h"
#include "ftd/eft/coupling_measurement.h"
#include "ftd/constants.h"

static void emit_measurement(const char* method, const ftd::eft::CouplingMeasurement& m) {
    for (const auto& p : m.data) {
        std::cout << method << "," << m.L << "," << m.n_ticks << ","
                  << p.r << ","
                  << std::setprecision(10) << p.V << ","
                  << std::setprecision(10) << p.alpha_r << ","
                  << "\n";
    }
    std::cout << method << "," << m.L << "," << m.n_ticks << ","
              << "fit" << ","
              << std::setprecision(10) << m.alpha_fit << ","
              << std::setprecision(10) << m.r2 << ","
              << (m.valid ? "valid" : "invalid") << "\n";
}

int main(int argc, char** argv) {
    int n_ticks = 300;
    bool quick  = false;
    bool extended = false;       // Ticket 3: add L = 128
    bool day2 = false;           // Day-2 Thread 1a: also add L = 256
    bool multi_seed = false;     // Ticket 2: 4 seeds per scale
    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if (s == "--quick") quick = true;
        else if (s == "--extended") extended = true;
        else if (s == "--day2") day2 = true;
        else if (s == "--multi-seed") multi_seed = true;
        else if (s.rfind("--ticks=", 0) == 0) n_ticks = std::atoi(s.c_str() + 8);
    }
    if (quick) n_ticks = 80;

    std::cerr << "================================================================\n";
    std::cerr << "  EFT Phase 2C — β(g) via multi-scale α_eff measurement\n";
    std::cerr << "  Reference α = 1/" << (1.0/ftd::ALPHA) << " = " << ftd::ALPHA << "\n";
    std::cerr << "  n_ticks = " << n_ticks << (quick ? " (quick)" : "")
              << (extended ? " + L=128 (extended)" : "")
              << (multi_seed ? " + 4 seeds per L (multi-seed)" : "") << "\n";
    std::cerr << "================================================================\n";

    std::cout << "method,L,ticks,r,V_or_alpha_fit,alpha_r_or_r2,flag\n";

    // Ticket 2: multi-seed ensemble, varying initial_flux_z as a
    // deterministic seed parameter (the engine dynamics are otherwise
    // deterministic). Four well-separated seeds cover amplitude regimes
    // where transient behaviour is slightly different.
    const std::vector<double> seeds = multi_seed
        ? std::vector<double>{0.030, 0.050, 0.070, 0.100}
        : std::vector<double>{0.050};

    // Ticket 3: Include L = 128 when --extended; L = 256 when --day2
    std::vector<int> sizes = {16, 32, 64};
    if (quick) sizes = {16, 32};           // skip L=64+ in quick mode
    if (extended && !quick) sizes.push_back(128);
    if (day2 && !quick) {
        if (!extended) sizes.push_back(128);   // day2 implies extended
        sizes.push_back(256);
    }

    for (int L : sizes) {
        // At L >= 128 keep the r-step coarser to cap runtime — the extra
        // r-points mainly narrow the fit uncertainty, not change α_eff.
        // At L = 256 r_step=6 to stay within runtime budget (~8 min).
        const int r_step = (L >= 256) ? 6 : (L >= 128 ? 4 : 2);
        for (size_t si = 0; si < seeds.size(); ++si) {
            const double seed_amp = seeds[si];
            std::cerr << "\n-- MCRG: L = " << L
                      << "  seed[" << si << "] = " << seed_amp
                      << "  r_step = " << r_step << " --\n";
            auto m = ftd::eft::measure_alpha_eff(L, n_ticks, /*r_min=*/4,
                                                 /*r_max=*/-1, r_step,
                                                 /*initial_flux_z=*/seed_amp);
            std::cerr << "   α_fit = " << std::setprecision(10) << m.alpha_fit
                      << "   R² = " << m.r2
                      << "   (vs reference " << ftd::ALPHA << ")\n";
            // Tag rows with the seed index so the Python analyzer can group.
            std::string method = "mcrg_seed" + std::to_string(si);
            emit_measurement(method.c_str(), m);
        }
    }

    return 0;
}

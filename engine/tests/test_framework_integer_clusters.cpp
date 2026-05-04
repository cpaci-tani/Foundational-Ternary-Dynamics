/**
 * Phase B.3 follow-up: search for stable clusters at OTHER FTD framework integers.
 *
 * The A=5 +color+triad SOLITON breakthrough (§5.6.10) has matter content
 * n=4 = N_base = mult(A_{1g}). This raises the question: are there stable
 * clusters at the OTHER FTD framework integers?
 *
 *   N_c = 3   (color number)
 *   N_base = 4   (lightest stable, A=5 +color+triad — confirmed)
 *   b_3 = 7   (master quadratic discriminant integer)
 *   N_eff = 13   (effective dimension; appears in mass formulas)
 *
 * This test scans amplitudes A ∈ {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}
 * with +color+triad config and checks whether ANY amplitude produces a
 * stable cluster with matter content n ∈ {3, 7, 13} (the other FTD integers).
 *
 * If yes: the engine produces stable clusters at multiple framework
 * integers — strong evidence that FTD-0110's algebraic spine determines
 * the engine's stable-cluster spectrum.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

struct R {
    double A;
    int n_init;
    int n_final;
    int t_first_growth;
    bool stable;
};

static R run_one(double A, int N_TICKS) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int SAMPLE = 25;
    ftd::RenderBridge rb(L);
    rb.toggles.color_forces = true;
    rb.toggles.triad_binding = true;
    rb.toggles.langevin_seed = 1;
    const int inj = L / 2;
    rb.inject_flux(inj, inj, inj, {A * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    R r;
    r.A = A;
    r.n_init = count_manifested(rb);
    r.t_first_growth = -1;
    for (int t = 1; t <= N_TICKS; ++t) {
        rb.tick();
        if (t % SAMPLE == 0) {
            int n = count_manifested(rb);
            if (r.t_first_growth < 0 && n > r.n_init * 1.5) r.t_first_growth = t;
        }
    }
    r.n_final = count_manifested(rb);
    r.stable = (r.t_first_growth < 0 && r.n_final > 0);
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  Phase B.3: stable-cluster search at FTD framework integers\n";
    std::cout << "================================================================\n\n";
    std::cout << "FTD framework integers: N_c=3, N_base=4, b_3=7, N_eff=13\n";
    std::cout << "Question: does the engine produce STABLE clusters at these matter contents?\n\n";

    const int N_TICKS = 1000;
    std::vector<double> A_vals = {3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5,
                                   7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0,
                                   11.0, 12.0, 13.0, 14.0, 15.0, 16.0};

    std::cout << "  A/K_G    n_init    n_final    t_growth    stable    matches FTD integer?\n";
    std::cout << "  -----    ------    -------    --------    ------    --------------------\n";

    std::vector<R> results;
    for (double A : A_vals) {
        R r = run_one(A, N_TICKS);
        results.push_back(r);
        std::string fti_match;
        if (r.stable) {
            if (r.n_final == 3) fti_match = "← N_c = 3";
            else if (r.n_final == 4) fti_match = "← N_base = 4";
            else if (r.n_final == 7) fti_match = "← b_3 = 7";
            else if (r.n_final == 13) fti_match = "← N_eff = 13";
        }
        std::cout << std::fixed << std::setprecision(1)
                  << "  " << std::setw(5) << r.A << "    "
                  << std::setw(6) << r.n_init << "    "
                  << std::setw(7) << r.n_final << "    "
                  << std::setw(8) << (r.t_first_growth < 0 ? std::string("none") : std::to_string(r.t_first_growth)) << "    "
                  << std::setw(6) << (r.stable ? "YES" : "no") << "    "
                  << fti_match << "\n";
    }

    // Tally
    std::cout << "\n--- Summary ---\n";
    std::vector<int> stable_sizes;
    for (const auto& r : results) {
        if (r.stable) stable_sizes.push_back(r.n_final);
    }
    std::cout << "  Total stable amplitudes tested: " << stable_sizes.size() << " / " << A_vals.size() << "\n";
    std::cout << "  Stable cluster matter contents observed: ";
    for (int n : stable_sizes) std::cout << n << " ";
    std::cout << "\n";

    int n3=0, n4=0, n7=0, n13=0;
    for (int n : stable_sizes) {
        if (n == 3) ++n3;
        if (n == 4) ++n4;
        if (n == 7) ++n7;
        if (n == 13) ++n13;
    }

    std::cout << "  Coincidences with FTD framework integers:\n";
    std::cout << "    n = N_c = 3   : " << n3 << " amplitude(s)\n";
    std::cout << "    n = N_base = 4: " << n4 << " amplitude(s)\n";
    std::cout << "    n = b_3 = 7   : " << n7 << " amplitude(s)\n";
    std::cout << "    n = N_eff = 13: " << n13 << " amplitude(s)\n";

    std::cout << "\n--- Verdict ---\n";
    int n_fti_matches = n3 + n4 + n7 + n13;
    if (n_fti_matches >= 3) {
        std::cout << "  [VERDICT] STRONG EVIDENCE: engine produces stable clusters at MULTIPLE\n";
        std::cout << "  FTD framework integers (n=3, 4, 7, 13). The engine's stable-cluster\n";
        std::cout << "  spectrum aligns with the FTD-0110 algebraic-spine integers.\n";
    } else if (n_fti_matches == 2) {
        std::cout << "  [VERDICT] MODERATE evidence: engine produces stable clusters at TWO\n";
        std::cout << "  FTD framework integers. Suggestive but not conclusive.\n";
    } else if (n_fti_matches == 1) {
        std::cout << "  [VERDICT] WEAK evidence: only n = N_base = 4 stable; could be coincidence.\n";
    } else {
        std::cout << "  [VERDICT] NO stable clusters at FTD framework integers (other than possibly N_base=4).\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED\n";
    std::cout << "================================================================\n";
    return 0;
}

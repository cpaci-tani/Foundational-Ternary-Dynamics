/**
 * @file campaign_bcc_band_spectrum.cpp
 * @brief BCC sub-stencil two-state spectrum campaign — smoke test.
 *
 * Implements the protocol of `docs/theory/10_eft_program/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md`.
 *
 * Pre-registered prediction (Mechanism C / FTD-0093).
 *   The thermalized BCC sub-stencil correlator
 *      C(τ) = <ψ(t)·ψ(t+τ)>_t,    ψ(t) = Σ_{i ∈ BCC sites} |J(i, t)|²
 *   is expected to admit a two-exponential decomposition whose decay rates
 *   stand in the master-quadratic ratio  x_+ / x_-  ≈  45.31  (= 137.04 / 3.024).
 *   The SUM x_+ + x_- on the BCC sub-stencil is expected to track 16 G*² ≈ 140.06
 *   in the dimensionless lattice eigenvalue convention.
 *
 *   Pre-registered control conditions:
 *     - On the SC sub-stencil the ratio should NOT be 45.31 (different
 *       eigenvalue scale; SC has Watson I_3, not I_1).
 *     - On the FCC sub-stencil the ratio should NOT be 45.31.
 *     - On the FULL stencil the ratio likewise should NOT be 45.31
 *       (legacy 18-pt is the (σ_SC + σ_FCC)/2 mix, BCC-orthogonal per FTD-0050).
 *
 * Smoke-test parameters
 * ---------------------
 *   L = 16, n_seeds = 2, T = 0.005, gamma = 0.05.
 *   N_BURN = 500, N_MEASURE = 800. Wall budget < 60 s on CPU.
 *
 * Smoke output is exploratory — it verifies the harness produces valid
 * spectrum extraction. The publication-grade falsifier (D2 §3 thresholds)
 * runs at L ∈ {24, 32, 48}, n_seeds ≥ 8, with all-stencil predictions
 * locked before run via git tag preregister-cluster-A-vN.
 *
 * Output
 * ------
 *   stdout: CSV per-(stencil, seed) row. Columns:
 *     stencil, seed, L, T, gamma, N_burn, N_measure, x_plus, x_minus,
 *     ratio, sum, valid_prony, valid_gevp, prony_failure
 *   stderr: human-readable progress + summary table.
 *
 * Epistemic status
 * ----------------
 *   [PROTOCOL DESIGN] — the harness runs end-to-end. Numerical values
 *   produced by the smoke test are NOT publishable. See D2 §7 resource
 *   budget.
 */

#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ftd/correlations.h"
#include "ftd/render_bridge.h"
#include "ftd/spectrum_extraction.h"
#include "ftd/sublattice.h"

namespace {

const char* stencil_name(ftd::BccStencilMode m) {
    switch (m) {
        case ftd::BccStencilMode::SC:   return "SC";
        case ftd::BccStencilMode::FCC:  return "FCC";
        case ftd::BccStencilMode::BCC:  return "BCC";
        case ftd::BccStencilMode::FULL: return "FULL";
    }
    return "?";
}

ftd::SiteClass class_for_stencil(ftd::BccStencilMode m) {
    // Match the Langevin parity filter to the stencil:
    //   BCC stencil → BCC sites
    //   SC  stencil → SC sites
    //   FCC stencil → FCC sites
    //   FULL stencil → all sites (legacy)
    switch (m) {
        case ftd::BccStencilMode::SC:   return ftd::SiteClass::SC_SITES;
        case ftd::BccStencilMode::FCC:  return ftd::SiteClass::FCC_SITES;
        case ftd::BccStencilMode::BCC:  return ftd::SiteClass::BCC_SITES;
        case ftd::BccStencilMode::FULL: return ftd::SiteClass::ALL_SITES;
    }
    return ftd::SiteClass::ALL_SITES;
}

struct StencilResult {
    ftd::BccStencilMode mode;
    unsigned int        seed;
    int                 L;
    double              T;
    double              gamma;
    int                 N_burn;
    int                 N_measure;
    ftd::TwoStateSpectrum prony;
    double              sum   = 0.0;
    double              ratio = 0.0;
};

StencilResult run_one(ftd::BccStencilMode mode, unsigned int seed,
                       int L, double T, double gamma,
                       int N_burn, int N_measure)
{
    StencilResult r;
    r.mode = mode; r.seed = seed; r.L = L; r.T = T; r.gamma = gamma;
    r.N_burn = N_burn; r.N_measure = N_measure;

    ftd::RenderBridge rb(L);
    // GPU port complete 2026-04-26:
    //   - Single-substrate Langevin verified working (test_langevin_equipartition).
    //   - Sublattice site filter ported (kernels_stencil.cu::langevin_site_match).
    //   - BCC sub-stencil ported (kernels_stencil.cu::phase_read_kernel branches on
    //     bcc_stencil_mode parameter).
    // No force_cpu() — campaign runs on GPU when CUDA is enabled.

    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;     // Laplacian IS the dynamical operator
    rb.toggles.gauss_projection = true;
    rb.toggles.langevin = true;
    rb.toggles.langevin_T = T;
    rb.toggles.langevin_gamma = gamma;
    rb.toggles.langevin_seed = seed;
    rb.toggles.bcc_stencil = mode;
    rb.toggles.langevin_site_filter = class_for_stencil(mode);

    std::string err;
    if (!rb.toggles.validate(&err)) {
        std::cerr << "[FAIL] toggle validation for " << stencil_name(mode) << ": " << err;
        r.prony.failure_reason = "toggle_validation";
        return r;
    }

    // Burn-in
    if (N_burn > 0) rb.run(N_burn);

    // Measurement: per-tick, sample sum_flux_energy on the matching parity class.
    const ftd::SiteClass filter = class_for_stencil(mode);
    std::vector<double> psi(N_measure, 0.0);
    const ftd::RenderBridge& crb = rb;   // const overload of voxels()
    for (int t = 0; t < N_measure; ++t) {
        rb.run(1);
        psi[t] = ftd::sum_flux_energy_sublattice(crb, filter);
    }

    // Build temporal autocorrelation, then extract two-state spectrum.
    auto C = ftd::temporal_autocorrelation(psi, /*max_tau=*/std::min(60, N_measure / 2));
    if (C.size() < 6) {
        r.prony.failure_reason = "autocorrelation: too few samples";
        return r;
    }

    r.prony = ftd::extract_two_state_prony(C, /*tau0=*/2);
    if (r.prony.valid) {
        r.sum   = r.prony.x_plus + r.prony.x_minus;
        r.ratio = (r.prony.x_minus > 0.0)
                    ? (r.prony.x_plus / r.prony.x_minus)
                    : 0.0;
    }
    return r;
}

void emit_csv_header() {
    std::printf("stencil,seed,L,T,gamma,N_burn,N_measure,x_plus,x_minus,sum,ratio,valid_prony,prony_failure\n");
}

void emit_csv_row(const StencilResult& r) {
    std::printf("%s,%u,%d,%.6f,%.6f,%d,%d,%.6e,%.6e,%.6e,%.6e,%d,\"%s\"\n",
                stencil_name(r.mode), r.seed, r.L, r.T, r.gamma,
                r.N_burn, r.N_measure,
                r.prony.x_plus, r.prony.x_minus, r.sum, r.ratio,
                r.prony.valid ? 1 : 0,
                r.prony.failure_reason ? r.prony.failure_reason : "");
    std::fflush(stdout);
}

}  // anonymous namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // Smoke parameters.
    const int L = 16;
    const double T = 0.005;
    const double gamma = 0.05;
    const int N_BURN = 500;
    const int N_MEASURE = 800;
    const std::vector<unsigned int> seeds = {1u, 2u};

    std::cerr << "================================================================\n";
    std::cerr << "  BCC band-spectrum campaign (smoke test)\n";
    std::cerr << "================================================================\n";
    std::cerr << "  L=" << L << "  T=" << T << "  gamma=" << gamma << "\n";
    std::cerr << "  N_burn=" << N_BURN << "  N_measure=" << N_MEASURE
              << "  n_seeds=" << seeds.size() << "\n";
    std::cerr << "  Stencil modes scanned: SC, FCC, BCC, FULL\n";
    std::cerr << "  Pre-registered prediction (D2 §3): BCC ratio x_+/x_- ≈ 45.31;\n";
    std::cerr << "    publication-grade falsifier needs L>=24 and n_seeds>=8.\n";
    std::cerr << "    Smoke results are exploratory only.\n\n";

    emit_csv_header();

    const std::vector<ftd::BccStencilMode> modes = {
        ftd::BccStencilMode::SC,
        ftd::BccStencilMode::FCC,
        ftd::BccStencilMode::BCC,
        ftd::BccStencilMode::FULL,
    };

    int total_runs = 0;
    int valid_runs = 0;
    for (auto m : modes) {
        std::cerr << "  --- Stencil " << stencil_name(m) << " ---\n";
        for (unsigned int s : seeds) {
            StencilResult r = run_one(m, s, L, T, gamma, N_BURN, N_MEASURE);
            emit_csv_row(r);
            ++total_runs;
            if (r.prony.valid) ++valid_runs;
            if (r.prony.valid) {
                std::cerr << "    seed=" << s
                          << "  x_+=" << r.prony.x_plus
                          << "  x_-=" << r.prony.x_minus
                          << "  ratio=" << r.ratio
                          << "  sum=" << r.sum << "\n";
            } else {
                std::cerr << "    seed=" << s << "  Prony invalid: "
                          << (r.prony.failure_reason ? r.prony.failure_reason : "")
                          << "\n";
            }
        }
    }

    std::cerr << "\n  Summary: " << valid_runs << "/" << total_runs
              << " runs produced a valid two-state Prony spectrum.\n";
    std::cerr << "  Smoke acceptance: at least one run extracted cleanly.\n";
    std::cerr << "  Publication-grade run (L>=24, n_seeds>=8, longer N_MEASURE)\n";
    std::cerr << "  is what tests the D2 §3 falsification thresholds.\n";

    if (valid_runs == 0) {
        std::cerr << "  [FAIL] Zero valid Prony extractions across all stencils.\n";
        return 1;
    }
    std::cerr << "  [PASS] Harness produced at least one valid two-state spectrum.\n";
    return 0;
}

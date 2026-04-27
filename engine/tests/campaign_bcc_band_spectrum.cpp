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
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ftd/correlations.h"
#include "ftd/render_bridge.h"
#include "ftd/spectrum_extraction.h"
#include "ftd/sublattice.h"

namespace fs = std::filesystem;

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

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // Default: smoke parameters (L=16, n_seeds=2). Production: pass CLI
    // overrides for L, seeds, burn, measure, output-dir per
    // PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md §D2 (FTD-0093 / Campaign A).
    int L = 16;
    double T = 0.005;
    double gamma = 0.05;
    int N_BURN = 500;
    int N_MEASURE = 800;
    int n_seeds = 2;
    std::string output_dir;  // empty → stdout-only (smoke); set → write meta.json + CSV
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a.rfind("--L=", 0) == 0)            L          = std::atoi(a.c_str() + 4);
        else if (a.rfind("--seeds=", 0) == 0)   n_seeds    = std::atoi(a.c_str() + 8);
        else if (a.rfind("--burn=", 0) == 0)    N_BURN     = std::atoi(a.c_str() + 7);
        else if (a.rfind("--measure=", 0) == 0) N_MEASURE  = std::atoi(a.c_str() + 10);
        else if (a.rfind("--T=", 0) == 0)       T          = std::atof(a.c_str() + 4);
        else if (a.rfind("--gamma=", 0) == 0)   gamma      = std::atof(a.c_str() + 8);
        else if (a.rfind("--output-dir=", 0) == 0) output_dir = a.substr(13);
    }
    std::vector<unsigned int> seeds;
    seeds.reserve(n_seeds);
    for (int i = 0; i < n_seeds; ++i) seeds.push_back(static_cast<unsigned int>(i + 1));

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
    std::vector<StencilResult> all_results;
    all_results.reserve(modes.size() * seeds.size());
    for (auto m : modes) {
        std::cerr << "  --- Stencil " << stencil_name(m) << " ---\n";
        for (unsigned int s : seeds) {
            StencilResult r = run_one(m, s, L, T, gamma, N_BURN, N_MEASURE);
            emit_csv_row(r);
            all_results.push_back(r);
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
    // (CSV rows already emitted to stdout; in production runs we tee them
    // into output_dir/spectrum.csv and write a meta.json summary.)
    if (!output_dir.empty()) {
        std::error_code ec;
        fs::create_directories(output_dir, ec);

        // Persist all CSV rows to spectrum.csv (stdout was used for tee
        // but ephemeral; this is the durable artifact).
        {
            std::ofstream csv(fs::path(output_dir) / "spectrum.csv");
            if (csv) {
                csv << "stencil,seed,L,T,gamma,N_burn,N_measure,x_plus,x_minus,sum,ratio,valid_prony,prony_failure\n";
                for (const auto& r : all_results) {
                    csv << stencil_name(r.mode) << "," << r.seed << "," << r.L
                        << "," << r.T << "," << r.gamma << "," << r.N_burn
                        << "," << r.N_measure << "," << r.prony.x_plus
                        << "," << r.prony.x_minus << "," << r.sum << "," << r.ratio
                        << "," << (r.prony.valid ? 1 : 0) << ",\""
                        << (r.prony.failure_reason ? r.prony.failure_reason : "") << "\"\n";
                }
            }
        }

        // Per-stencil aggregates: mean ratio, mean sum, sample stderr, valid count.
        // Prediction (FTD-0093 §3): BCC ratio = 45.31, sum = 16·G*² ≈ 140.06;
        // SC/FCC/FULL controls should NOT match.
        struct Agg {
            double ratio_mean = 0.0, ratio_stderr = 0.0;
            double sum_mean = 0.0,   sum_stderr = 0.0;
            int valid_n = 0;
        };
        auto agg = [](const std::vector<StencilResult>& rs, ftd::BccStencilMode m) -> Agg {
            Agg out;
            std::vector<double> ratios, sums;
            for (const auto& r : rs) {
                if (r.mode != m) continue;
                if (!r.prony.valid) continue;
                if (r.ratio == 0.0) continue;  // skip negative-x_- artifacts
                ratios.push_back(r.ratio);
                sums.push_back(r.sum);
            }
            out.valid_n = static_cast<int>(ratios.size());
            if (out.valid_n == 0) return out;
            for (double v : ratios) out.ratio_mean += v;
            for (double v : sums)   out.sum_mean += v;
            out.ratio_mean /= out.valid_n;
            out.sum_mean   /= out.valid_n;
            if (out.valid_n < 2) return out;
            double sr2 = 0.0, ss2 = 0.0;
            for (double v : ratios) sr2 += (v - out.ratio_mean) * (v - out.ratio_mean);
            for (double v : sums)   ss2 += (v - out.sum_mean)   * (v - out.sum_mean);
            out.ratio_stderr = std::sqrt(sr2 / (out.valid_n - 1) / out.valid_n);
            out.sum_stderr   = std::sqrt(ss2 / (out.valid_n - 1) / out.valid_n);
            return out;
        };
        const Agg sc   = agg(all_results, ftd::BccStencilMode::SC);
        const Agg fcc  = agg(all_results, ftd::BccStencilMode::FCC);
        const Agg bcc  = agg(all_results, ftd::BccStencilMode::BCC);
        const Agg full_ = agg(all_results, ftd::BccStencilMode::FULL);

        // Emit a stencil-aggregate.csv for cross-L plotting.
        {
            std::ofstream a(fs::path(output_dir) / "stencil_aggregate.csv");
            if (a) {
                a << "stencil,valid_n,ratio_mean,ratio_stderr,sum_mean,sum_stderr\n";
                auto row = [&a](const char* name, const Agg& g) {
                    a << name << "," << g.valid_n << "," << g.ratio_mean << ","
                      << g.ratio_stderr << "," << g.sum_mean << "," << g.sum_stderr << "\n";
                };
                row("SC",   sc);
                row("FCC",  fcc);
                row("BCC",  bcc);
                row("FULL", full_);
            }
        }

        // meta.json with full aggregates and the headline falsifier check.
        std::ofstream meta(fs::path(output_dir) / "meta.json");
        if (meta) {
            meta << "{\n";
            meta << "  \"campaign\": \"bcc_spectrum_2026-04-27\",\n";
            meta << "  \"ledger_row\": \"FTD-0093\",\n";
            meta << "  \"protocol\": \"docs/theory/10_eft_program/PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md\",\n";
            meta << "  \"L\": " << L << ",\n";
            meta << "  \"T\": " << T << ",\n";
            meta << "  \"gamma\": " << gamma << ",\n";
            meta << "  \"N_BURN\": " << N_BURN << ",\n";
            meta << "  \"N_MEASURE\": " << N_MEASURE << ",\n";
            meta << "  \"n_seeds\": " << seeds.size() << ",\n";
            meta << "  \"total_runs\": " << total_runs << ",\n";
            meta << "  \"valid_runs\": " << valid_runs << ",\n";
            meta << "  \"predicted_bcc_ratio\": 45.31,\n";
            meta << "  \"predicted_bcc_sum\": 140.06,\n";
            auto emit = [&meta](const char* name, const Agg& g) {
                meta << "  \"" << name << "\": {\n";
                meta << "    \"valid_n\": " << g.valid_n << ",\n";
                meta << "    \"ratio_mean\": " << g.ratio_mean << ",\n";
                meta << "    \"ratio_stderr\": " << g.ratio_stderr << ",\n";
                meta << "    \"sum_mean\": " << g.sum_mean << ",\n";
                meta << "    \"sum_stderr\": " << g.sum_stderr << "\n";
                meta << "  }";
            };
            emit("SC",   sc);   meta << ",\n";
            emit("FCC",  fcc);  meta << ",\n";
            emit("BCC",  bcc);  meta << ",\n";
            emit("FULL", full_); meta << ",\n";
            // Falsifier checks (PROTOCOL §5):
            // (a) BCC ratio matches 45.31 within stderr
            // (b) SC, FCC, FULL ratios do NOT match 45.31 (basis specificity)
            const double pred_ratio = 45.31;
            const bool bcc_matches  = (bcc.valid_n >= 2) &&
                std::abs(bcc.ratio_mean - pred_ratio) < 3.0 * std::max(bcc.ratio_stderr, 0.1);
            auto control_matches = [pred_ratio](const Agg& g) {
                if (g.valid_n < 2) return false;
                return std::abs(g.ratio_mean - pred_ratio) < 3.0 * std::max(g.ratio_stderr, 0.1);
            };
            const bool sc_control_ok   = !control_matches(sc);
            const bool fcc_control_ok  = !control_matches(fcc);
            const bool full_control_ok = !control_matches(full_);
            const bool falsifier_pass  = bcc_matches && sc_control_ok && fcc_control_ok && full_control_ok;
            meta << "  \"falsifier_bcc_matches_45.31\": " << (bcc_matches ? "true" : "false") << ",\n";
            meta << "  \"falsifier_sc_control_distinct\": " << (sc_control_ok ? "true" : "false") << ",\n";
            meta << "  \"falsifier_fcc_control_distinct\": " << (fcc_control_ok ? "true" : "false") << ",\n";
            meta << "  \"falsifier_full_control_distinct\": " << (full_control_ok ? "true" : "false") << ",\n";
            meta << "  \"falsifier_overall\": \"" << (falsifier_pass ? "PASS" : "FAIL") << "\"\n";
            meta << "}\n";
        }
        std::cerr << "  artifacts → " << output_dir << "\n";
        std::cerr << "  Falsifier verdict (PROTOCOL §5):\n";
        std::cerr << "    BCC ratio mean = " << bcc.ratio_mean
                  << " ± " << bcc.ratio_stderr << " (target 45.31)\n";
        std::cerr << "    SC ratio mean  = " << sc.ratio_mean   << " ± " << sc.ratio_stderr   << "\n";
        std::cerr << "    FCC ratio mean = " << fcc.ratio_mean  << " ± " << fcc.ratio_stderr  << "\n";
        std::cerr << "    FULL ratio mean= " << full_.ratio_mean << " ± " << full_.ratio_stderr << "\n";
    }
    return 0;
}

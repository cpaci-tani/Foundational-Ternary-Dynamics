/**
 * @file campaign_beta_measurement.cpp
 * @brief β-function measurement at non-zero temperature — smoke test.
 *
 * Implements the protocol of `docs/theory/10_eft_program/PROTOCOL_BETA_MEASUREMENT.md`.
 *
 * Smoke test parameters
 * ---------------------
 *   L_fine        = 32
 *   L_mid         = 16    (blocking factor b = 2)
 *   T             = 0.005 (Langevin temperature, sub-K_B to avoid genesis)
 *   gamma         = 0.01
 *   burn_in       = 5000  ticks per seed
 *   n_seeds       = 4     (publication target: 100; smoke: 4 keeps wall time
 *                          below the 1800-s budget on CPU single-threaded)
 *   n_ticks_probe = 200   (reduced from publication 600)
 *   r range       = {4, 6, 8, 10} on L=32; {4} on L=16 (degenerate fit)
 *
 * Output
 * ------
 *   stdout: CSV per-(stage, L, seed, r) measurement plus per-seed alpha_fit
 *           rows (r = "fit") plus a final beta row.
 *   stderr: human-readable progress + summary.
 *
 * Phase-G note
 * ------------
 * `AUDIT_ALPHA_EXTRACTION.md` showed that V(r) on the bare engine is the
 * lattice Poisson Green's function — geometric, fixed under blocking on
 * the propagator side. Any non-zero β under this protocol must come from
 * the source/manifestation side: thermal dressing of the test charge
 * plus the block-spin charge-conserving rule's overflow spreading. We
 * pre-commit that β ≈ 0 within statistical error is the
 * Phase-G-consistent expected outcome at the smoke-test sample size.
 *
 * Epistemic status
 * ----------------
 * [PROTOCOL DESIGN] — implementation is complete and runs end-to-end.
 * The numerical β value reported by the smoke test is NOT a publishable
 * result. See PROTOCOL_BETA_MEASUREMENT.md §8 for the path to the
 * publishable measurement on WSL2.
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

#include "ftd/eft/blocking.h"
#include "ftd/eft/coupling_measurement.h"
#include "ftd/render_bridge.h"

namespace {

// Local CPU-only versions of the prepare_thermal_background +
// measure_alpha_eff_on_bg helpers from coupling_measurement.h.
//
// Why: FTD-0051 ships the Langevin thermostat on the CPU single-substrate
// path only; on GPU-enabled builds RenderBridge auto-selects the GPU
// backend, on which `langevin = true` is a silent no-op (the thermal
// background ends up at <|v|²> = 0). force_cpu() switches to the CPU
// backend, which honours the Langevin update via phase_write.
//
// These wrappers exist because ftd::eft::prepare_thermal_background and
// the *_on_bg helpers do not expose a force_cpu() hook. Once GPU
// Langevin lands, these wrappers can be deleted in favour of the upstream
// helpers.

std::unique_ptr<ftd::RenderBridge> prepare_thermal_bg_cpu(
    int L, double T, double gamma, int burn_in_ticks, unsigned int seed)
{
    auto rb = std::make_unique<ftd::RenderBridge>(L);
    rb->force_cpu();
    ftd::eft::configure_bare_lattice_for_coupling(*rb);
    ftd::eft::LangevinOptions opts;
    opts.enabled = true;
    opts.T = T;
    opts.gamma = gamma;
    opts.seed = seed;
    ftd::eft::apply_langevin_options(*rb, opts);
    if (burn_in_ticks > 0) rb->run(burn_in_ticks);
    return rb;
}

double self_energy_on_bg_cpu(const ftd::RenderBridge& bg, int8_t sign,
                             int n_ticks, double initial_flux_z) {
    const int L = bg.lattice().size();
    const int mid = L / 2;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    ftd::eft::configure_bare_lattice_for_coupling(rb);
    ftd::eft::copy_flux_and_wave_vel_for_coupling(bg, rb);
    ftd::eft::place_test_charge_on_bg(rb, mid, mid, mid, sign, initial_flux_z);
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

double pair_energy_on_bg_cpu(const ftd::RenderBridge& bg, int r,
                             int n_ticks, double initial_flux_z) {
    const int L = bg.lattice().size();
    const int mid = L / 2;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    ftd::eft::configure_bare_lattice_for_coupling(rb);
    ftd::eft::copy_flux_and_wave_vel_for_coupling(bg, rb);
    ftd::eft::place_test_charge_on_bg(rb, mid, mid, mid, +1, initial_flux_z);
    ftd::eft::place_test_charge_on_bg(rb, mid + r, mid, mid, -1, initial_flux_z);
    rb.run(n_ticks);
    return rb.energy_audit().field_energy;
}

ftd::eft::CouplingMeasurement alpha_eff_on_bg_cpu(
    const ftd::RenderBridge& bg, int n_ticks,
    int r_min, int r_max, int r_step, double initial_flux_z)
{
    ftd::eft::CouplingMeasurement out;
    const int L = bg.lattice().size();
    out.L = L;
    out.n_ticks = n_ticks;
    if (r_max < 0) r_max = L / 3;
    if (r_max <= r_min) return out;
    if (L < 8) return out;

    out.e_self_pos = self_energy_on_bg_cpu(bg, +1, n_ticks, initial_flux_z);
    out.e_self_neg = self_energy_on_bg_cpu(bg, -1, n_ticks, initial_flux_z);
    const double E_2self = out.e_self_pos + out.e_self_neg;

    for (int r = r_min; r <= r_max; r += r_step) {
        const double E_pair = pair_energy_on_bg_cpu(bg, r, n_ticks, initial_flux_z);
        const double V = E_pair - E_2self;
        ftd::eft::VofR pt;
        pt.r = r;
        pt.V = V;
        pt.alpha_r = -V * static_cast<double>(r);
        out.data.push_back(pt);
    }

    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (const auto& p : out.data) {
            const double x = 1.0 / static_cast<double>(p.r);
            const double y = p.V;
            sx += x; sy += y; sxx += x * x; sxy += x * y;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            const double slope = (n * sxy - sx * sy) / denom;
            const double intercept = (sy - slope * sx) / n;
            out.alpha_fit = -slope;
            const double ybar = sy / n;
            double ss_tot = 0.0, ss_res = 0.0;
            for (const auto& p : out.data) {
                const double x = 1.0 / static_cast<double>(p.r);
                const double y = p.V;
                const double yhat = intercept + slope * x;
                ss_tot += (y - ybar) * (y - ybar);
                ss_res += (y - yhat) * (y - yhat);
            }
            out.r2 = (ss_tot > 0.0) ? 1.0 - ss_res / ss_tot : 0.0;
            out.valid = std::isfinite(out.alpha_fit);
        }
    }
    return out;
}


struct StageResult {
    int    L = 0;
    double T = 0.0;
    double gamma = 0.0;
    int    burn_in = 0;
    int    n_ticks_probe = 0;
    std::vector<double> alpha_per_seed;
    double alpha_mean = 0.0;
    double alpha_sem  = 0.0;   // standard error of the mean
};

// Run one stage: prepare a Langevin background, then measure α_eff_on_bg.
// Returns per-seed alphas and their seed-to-seed mean and SEM.
StageResult run_stage(int L, double T, double gamma,
                      int burn_in, int n_ticks_probe,
                      const std::vector<unsigned int>& seeds,
                      int r_min, int r_max, int r_step,
                      double initial_flux_z,
                      const char* stage_label) {
    StageResult st;
    st.L = L;
    st.T = T;
    st.gamma = gamma;
    st.burn_in = burn_in;
    st.n_ticks_probe = n_ticks_probe;

    std::cerr << "  --- Stage " << stage_label << " (L=" << L << ", T=" << T
              << ", " << seeds.size() << " seeds) ---\n";

    for (size_t k = 0; k < seeds.size(); ++k) {
        const unsigned int seed = seeds[k];
        std::cerr << "    seed " << seed << " (" << (k+1) << "/"
                  << seeds.size() << "): preparing thermal bg...\n";

        auto bg = prepare_thermal_bg_cpu(L, T, gamma, burn_in, seed);

        // Sanity check on equipartition of the background.
        const auto& vox = bg->voxels();
        double v2_sum = 0.0;
        for (const auto& v : vox) v2_sum += v.wave_vel.mag2();
        const double v2_mean = v2_sum / static_cast<double>(vox.size());
        std::cerr << "      <|v|²>_voxel = " << v2_mean
                  << "  (target 3T = " << 3.0*T << ", ratio = "
                  << v2_mean / (3.0 * T) << ")\n";

        std::cerr << "      measuring alpha_eff on thermal bg "
                  << "(probe ticks=" << n_ticks_probe << ")...\n";

        auto m = alpha_eff_on_bg_cpu(
            *bg, n_ticks_probe, r_min, r_max, r_step, initial_flux_z);

        // Emit per-r CSV rows.
        for (const auto& p : m.data) {
            std::cout << "data," << stage_label << "," << L << ","
                      << seed << "," << T << "," << gamma << ","
                      << p.r << ","
                      << std::setprecision(10) << p.V << ","
                      << std::setprecision(10) << p.alpha_r << ",,,,"
                      << n_ticks_probe << "\n";
        }
        // Emit per-seed fit row.
        std::cout << "fit," << stage_label << "," << L << ","
                  << seed << "," << T << "," << gamma << ",fit,,,"
                  << std::setprecision(10) << m.alpha_fit << ","
                  << std::setprecision(6) << m.r2 << ","
                  << (m.valid ? "valid" : "invalid") << ","
                  << n_ticks_probe << "\n";

        st.alpha_per_seed.push_back(m.alpha_fit);
        std::cerr << "      alpha_fit = " << m.alpha_fit
                  << "  (R²=" << m.r2 << ", "
                  << (m.valid ? "valid" : "INVALID") << ")\n";
    }

    // Aggregate across seeds.
    if (!st.alpha_per_seed.empty()) {
        double s = 0.0;
        for (double a : st.alpha_per_seed) s += a;
        st.alpha_mean = s / static_cast<double>(st.alpha_per_seed.size());
        if (st.alpha_per_seed.size() >= 2) {
            double s2 = 0.0;
            for (double a : st.alpha_per_seed) {
                const double d = a - st.alpha_mean;
                s2 += d * d;
            }
            const double var =
                s2 / static_cast<double>(st.alpha_per_seed.size() - 1);
            st.alpha_sem = std::sqrt(var /
                static_cast<double>(st.alpha_per_seed.size()));
        }
    }
    std::cerr << "    => mean alpha = " << st.alpha_mean
              << " ± " << st.alpha_sem << " (SEM, n="
              << st.alpha_per_seed.size() << ")\n";
    return st;
}

}  // namespace

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);

    // Smoke-test defaults. CLI lets the WSL2 path crank these up.
    int    L_fine        = 32;
    int    L_mid         = 16;
    double T             = 0.005;
    double gamma_lang    = 0.01;
    int    burn_in       = 5000;
    int    n_ticks_probe = 200;
    int    n_seeds       = 4;

    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);
        if      (s.rfind("--L_fine=",   0) == 0) L_fine        = std::atoi(s.c_str() + 9);
        else if (s.rfind("--L_mid=",    0) == 0) L_mid         = std::atoi(s.c_str() + 8);
        else if (s.rfind("--T=",        0) == 0) T             = std::atof(s.c_str() + 4);
        else if (s.rfind("--gamma=",    0) == 0) gamma_lang    = std::atof(s.c_str() + 8);
        else if (s.rfind("--burn=",     0) == 0) burn_in       = std::atoi(s.c_str() + 7);
        else if (s.rfind("--ticks=",    0) == 0) n_ticks_probe = std::atoi(s.c_str() + 8);
        else if (s.rfind("--seeds=",    0) == 0) n_seeds       = std::atoi(s.c_str() + 8);
    }

    std::cerr << "================================================================\n";
    std::cerr << "  Campaign — β-function measurement at non-zero T (smoke)\n";
    std::cerr << "  Protocol: docs/theory/10_eft_program/PROTOCOL_BETA_MEASUREMENT.md\n";
    std::cerr << "================================================================\n";
    std::cerr << "  L_fine = " << L_fine << ", L_mid = " << L_mid
              << " (b = " << double(L_fine) / double(L_mid) << ")\n";
    std::cerr << "  T = " << T << ", gamma = " << gamma_lang
              << ", burn = " << burn_in << " ticks\n";
    std::cerr << "  n_seeds = " << n_seeds
              << ", probe ticks = " << n_ticks_probe << "\n";
    std::cerr << "================================================================\n";

    // CSV header.
    std::cout << "kind,stage,L,seed,T,gamma,r,V,alpha_r,alpha_fit,r2,valid,n_ticks_probe\n";

    std::vector<unsigned int> seeds;
    for (int i = 0; i < n_seeds; ++i) {
        seeds.push_back(static_cast<unsigned int>(101 + 17 * i));
    }

    // Stage 1: fine lattice.
    StageResult fine = run_stage(L_fine, T, gamma_lang, burn_in,
                                 n_ticks_probe, seeds,
                                 /*r_min=*/4, /*r_max=*/L_fine / 3,
                                 /*r_step=*/2, /*initial_flux_z=*/0.05,
                                 "fine");

    // Stage 2: blocked / mid lattice.
    // Fewer r-points at small L; tolerate r_max = L/3 with r_step = 2.
    StageResult mid  = run_stage(L_mid,  T, gamma_lang, burn_in,
                                 n_ticks_probe, seeds,
                                 /*r_min=*/2, /*r_max=*/L_mid / 3,
                                 /*r_step=*/1, /*initial_flux_z=*/0.05,
                                 "mid");

    // β extraction: g = sqrt(alpha); β ≈ (g_mid - g_fine) / log(b).
    const double b = static_cast<double>(L_fine) / static_cast<double>(L_mid);
    const double logb = std::log(b);

    auto safe_sqrt = [](double a) {
        return (a > 0.0) ? std::sqrt(a) : -std::sqrt(-a);  // signed root
    };

    const double g_fine = safe_sqrt(fine.alpha_mean);
    const double g_mid  = safe_sqrt(mid.alpha_mean);
    const double dg     = g_mid - g_fine;
    const double beta   = (logb > 0.0) ? dg / logb : 0.0;

    // Propagate errors:  σ_g(L) = (1/(2|g|)) σ_alpha(L)  (for alpha > 0).
    auto sigma_g = [&](double a, double s) {
        if (a > 0.0 && std::abs(g_fine) > 0.0)
            return s / (2.0 * std::sqrt(a));
        return std::abs(s);  // fallback when alpha < 0
    };
    const double sg_fine = sigma_g(fine.alpha_mean, fine.alpha_sem);
    const double sg_mid  = sigma_g(mid.alpha_mean,  mid.alpha_sem);
    const double sigma_beta = (logb > 0.0)
        ? std::sqrt(sg_fine * sg_fine + sg_mid * sg_mid) / logb
        : 0.0;

    std::cerr << "\n----------------------------------------------------------------\n";
    std::cerr << "  alpha_fine (L=" << L_fine << ") = " << fine.alpha_mean
              << " ± " << fine.alpha_sem << "\n";
    std::cerr << "  alpha_mid  (L=" << L_mid  << ") = " << mid.alpha_mean
              << " ± " << mid.alpha_sem  << "\n";
    std::cerr << "  g_fine = " << g_fine << ",  g_mid = " << g_mid
              << ",  Δg = " << dg << "\n";
    std::cerr << "  β = (g_mid − g_fine) / log(" << b << ") = "
              << beta << " ± " << sigma_beta << "\n";
    std::cerr << "----------------------------------------------------------------\n";

    std::cout << "beta,L_fine=" << L_fine << ",L_mid=" << L_mid
              << ",b=" << b << ",T=" << T << ",gamma=" << gamma_lang
              << ",alpha_fine=" << fine.alpha_mean
              << ",alpha_fine_sem=" << fine.alpha_sem
              << ",alpha_mid=" << mid.alpha_mean
              << ",alpha_mid_sem=" << mid.alpha_sem
              << ",beta=" << beta
              << ",sigma_beta=" << sigma_beta << "\n";

    // Smoke-test PASS: campaign produced a finite β with finite error bar
    // at both stages. We do NOT compare to QED/QCD reference β — the
    // smoke test does not have the statistical power for that comparison.
    const bool finite_beta = std::isfinite(beta) && std::isfinite(sigma_beta);
    const bool fine_valid  = fine.alpha_per_seed.size() == seeds.size();
    const bool mid_valid   = mid.alpha_per_seed.size()  == seeds.size();
    const bool nonempty    = !fine.alpha_per_seed.empty()
                          && !mid.alpha_per_seed.empty();

    if (finite_beta && fine_valid && mid_valid && nonempty) {
        std::cerr << "\n  PASS  β finite at " << seeds.size()
                  << " seeds × 2 scales — smoke test ran end-to-end.\n";
        std::cerr << "  NOTE  Numerical β value is NOT publishable; see\n"
                     "        PROTOCOL_BETA_MEASUREMENT.md §8 for the\n"
                     "        WSL2 path to a publishable measurement.\n";
        return 0;
    }
    std::cerr << "\n  FAIL  β extraction produced non-finite or missing data.\n";
    return 1;
}

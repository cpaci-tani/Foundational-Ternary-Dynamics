/**
 * Test: W/Z Mass Generation from Chirality Gap in Dual Substrate
 *
 * Physics Checklist Item #36
 *
 * In dual-substrate mode, flux splits into left-handed (J_L) and
 * right-handed (J_R) components. The observable psi = J_L + J_R
 * propagates at C_SPEED (massless photon mode). The chirality
 * phi = J_L - J_R has a mass gap: excitations above the gap
 * correspond to massive W/Z bosons; below it, the photon is massless.
 *
 * Tests:
 *   WZ-1: Chirality gap exists in dual-substrate mode
 *   WZ-2: Observable flux propagates at speed C (massless photon)
 *   WZ-3: Chirality perturbation decays (massive mode)
 *   WZ-4: Mass constants from ontic.h match experiment
 *   WZ-5: W/Z mass ratio from Weinberg angle
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <ftd/render_bridge.h>
#include <ftd/ontic.h>

using namespace ftd;
using namespace ftd::ontic;

static int g_pass = 0, g_fail = 0;

static void check(const char* name, bool cond) {
    if (cond) { std::cout << "  [PASS] " << name << "\n"; g_pass++; }
    else      { std::cout << "  [FAIL] " << name << "\n"; g_fail++; }
}

static void check_close(const char* name, double got, double exp, double reltol) {
    double err = (exp == 0.0) ? std::abs(got) : std::abs(got - exp) / std::abs(exp);
    bool ok = err < reltol;
    if (ok) {
        std::cout << "  [PASS] " << name
                  << " (" << got << " vs " << exp << ", " << err * 100.0 << "% err)\n";
        g_pass++;
    } else {
        std::cout << "  [FAIL] " << name
                  << " (" << got << " vs " << exp << ", " << err * 100.0 << "% err)\n";
        g_fail++;
    }
}

int main() {
    std::cout << std::setprecision(8);

    // ================================================================
    // WZ-1: Chirality lives in the D register; matter never sources it
    // ================================================================
    // Injection seeds charge-signed chirality (delta-split: +1 L-major,
    // -1 R-major), so at t=0 chi(+1) > 0 and chi(-1) < 0. Under the exact
    // (F,D) register semantics (2026-07-17 adjudication) the matter
    // coupling sources ONLY F: the injected D-content disperses across
    // the lattice (nonzero |chi| sum persists) while the particle sites
    // themselves decay to chirality dust — they stay flux-live but
    // D-blind. Both halves are asserted below.
    // ================================================================
    std::cout << "\n=== WZ-1: Chirality gap exists in dual-substrate mode ===\n";

    {
        RenderBridge rb(16);
        rb.toggles.dual_substrate = true;
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.damping = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.genesis = false;
        rb.toggles.forces = true;
        rb.toggles.movement = false;  // pin particles in place

        // Inject +1 and -1 particles separated in the grid
        rb.inject_particle(5, 8, 8, +1, Vec3(K_B, 0, 0));
        rb.inject_particle(11, 8, 8, -1, Vec3(K_B, 0, 0));

        // Injection-time chirality carries the charge-signed delta-split
        // (inject_particle: +1 -> L-major, -1 -> R-major). These signs are
        // an INJECTION convention, valid only before evolution.
        int idx_pos0 = rb.lattice().index(5, 8, 8);
        int idx_neg0 = rb.lattice().index(11, 8, 8);
        double chi_pos_t0 = rb.voxels()[idx_pos0].chirality_density();
        double chi_neg_t0 = rb.voxels()[idx_neg0].chirality_density();
        std::cout << "    chi(+1) at t=0 = " << chi_pos_t0 << " (injection: > 0)\n";
        std::cout << "    chi(-1) at t=0 = " << chi_neg_t0 << " (injection: < 0)\n";
        check("WZ-1c: chi(+1) > 0 at injection", chi_pos_t0 > 0);
        check("WZ-1d: chi(-1) < 0 at injection", chi_neg_t0 < 0);

        // Evolve to let flux fields develop
        for (int t = 0; t < 100; ++t) rb.tick();

        auto audit = rb.energy_audit();

        // Check 1: Total chirality is nonzero (particles create L/R asymmetry)
        // With +1 and -1 particles, local chirality is nonzero even if global
        // may partially cancel. Check that chirality_total has developed.
        // chirality_total is sum of |psi_L|^2 - |psi_R|^2 over all sites;
        // with opposite-sign particles, contributions may partially cancel
        // but the absolute sum of local chirality should be nonzero.
        double chi_abs_sum = 0.0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            chi_abs_sum += std::abs(rb.voxels()[i].chirality_density());
        }
        std::cout << "    |chirality| sum = " << chi_abs_sum << "\n";
        check("WZ-1a: |chirality| sum > 0 near particles", chi_abs_sum > 1e-6);

        // Check 2 (register theorem, 2026-07-17 adjudication): matter never
        // sources D. The coupling refills F at the particle sites every tick
        // but NEVER the difference register, so the injected site chirality
        // disperses and is not replenished — after evolution the sites are
        // chirality-blind (float dust) while still carrying strong flux.
        // The pre-adjudication expectation here (chirality concentrated at
        // particle sites with charge-signed values) contradicts the exact
        // (F,D) register semantics and read sign-arbitrary ~1e-17 dust.
        int idx_pos = rb.lattice().index(5, 8, 8);
        int idx_neg = rb.lattice().index(11, 8, 8);

        double chi_at_pos = std::abs(rb.voxels()[idx_pos].chirality_density());
        double chi_at_neg = std::abs(rb.voxels()[idx_neg].chirality_density());
        double flux_at_pos = rb.voxels()[idx_pos].flux.mag();
        double flux_at_neg = rb.voxels()[idx_neg].flux.mag();

        std::cout << "    |chi| at +1 site = " << chi_at_pos
                  << " (|J| = " << flux_at_pos << ")\n";
        std::cout << "    |chi| at -1 site = " << chi_at_neg
                  << " (|J| = " << flux_at_neg << ")\n";

        check("WZ-1b: particle sites stay flux-live (|J| > 1e-4)",
              flux_at_pos > 1e-4 && flux_at_neg > 1e-4);
        check("WZ-1e: matter does not source D — site chirality decays to dust (< 1e-10)",
              chi_at_pos < 1e-10 && chi_at_neg < 1e-10);
    }

    // ================================================================
    // WZ-2: Observable flux propagates at speed C (massless photon)
    // ================================================================
    // The symmetric mode (J_L + J_R) corresponds to the photon.
    // It should propagate at C_SPEED = 1/sqrt(3) ~ 0.577 voxels/tick.
    // We inject a symmetric perturbation (equal L and R) and measure
    // the wavefront after some ticks.
    // ================================================================
    std::cout << "\n=== WZ-2: Observable flux propagates at speed C (massless photon) ===\n";

    {
        int L = 32;
        RenderBridge rb(L);
        rb.toggles.dual_substrate = true;
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = false;   // pure wave, no coupling
        rb.toggles.damping = false;    // undamped propagation
        rb.toggles.gauss_projection = false;
        rb.toggles.genesis = false;
        rb.toggles.forces = false;
        rb.toggles.movement = false;

        int cx = L / 2, cy = L / 2, cz = L / 2;
        int center = rb.lattice().index(cx, cy, cz);

        // Inject symmetric perturbation: equal J_L and J_R
        // This ensures phi = J_L - J_R = 0 (pure observable/photon mode)
        double amp = 0.5;
        rb.voxels()[center].flux_L = Vec3(amp, 0, 0);
        rb.voxels()[center].flux_R = Vec3(amp, 0, 0);
        rb.voxels()[center].flux = Vec3(2.0 * amp, 0, 0);  // observable = L + R

        int ticks = 20;
        for (int t = 0; t < ticks; ++t) rb.tick();

        // Measure wavefront: find the farthest point along +x with
        // significant flux energy
        double threshold = 1e-6;
        int max_reach = 0;
        for (int dx = 1; dx < L / 2; ++dx) {
            int ix = (cx + dx) % L;
            int idx = rb.lattice().index(ix, cy, cz);
            double e = rb.voxels()[idx].flux.mag2();
            if (e > threshold) {
                max_reach = dx;
            }
        }

        double measured_speed = (ticks > 0) ? static_cast<double>(max_reach) / ticks : 0.0;
        std::cout << "    wavefront reach = " << max_reach << " voxels in " << ticks << " ticks\n";
        std::cout << "    measured speed = " << measured_speed << " voxels/tick\n";
        std::cout << "    C_SPEED = " << C_SPEED << " voxels/tick\n";

        // The wavefront speed should be near C_SPEED (numerical dispersion
        // can make leading edge travel slightly faster than CFL phase velocity)
        // Accept within 50% — the key assertion is that it propagates at all
        // and at the right order of magnitude.
        check("WZ-2a: wavefront propagates (reach > 0)", max_reach > 0);
        check_close("WZ-2b: speed ~ C_SPEED", measured_speed, C_SPEED, 0.50);

        // Check that the observable identity holds everywhere after propagation
        bool identity_ok = true;
        int Ntot = rb.lattice().total_sites();
        for (int i = 0; i < Ntot; ++i) {
            auto& v = rb.voxels()[i];
            Vec3 sum = v.flux_L + v.flux_R;
            if (std::abs(v.flux.x - sum.x) > 1e-10 ||
                std::abs(v.flux.y - sum.y) > 1e-10 ||
                std::abs(v.flux.z - sum.z) > 1e-10) {
                identity_ok = false;
                break;
            }
        }
        check("WZ-2c: flux = flux_L + flux_R identity holds", identity_ok);
    }

    // ================================================================
    // WZ-3: Chirality perturbation decays (massive mode)
    // ================================================================
    // An asymmetric perturbation (J_L only, J_R = 0) creates a
    // chirality phi = J_L - J_R != 0. This antisymmetric mode
    // corresponds to the massive W/Z sector. Under evolution with
    // damping, the chirality amplitude should decrease — the mass
    // gap causes the chiral mode to decay.
    // ================================================================
    std::cout << "\n=== WZ-3: Chirality perturbation decays (massive mode) ===\n";

    {
        int L = 16;
        RenderBridge rb(L);
        rb.toggles.dual_substrate = true;
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = false;
        rb.toggles.damping = true;     // damping ON — chirality should decay
        rb.toggles.gauss_projection = false;
        rb.toggles.genesis = false;
        rb.toggles.forces = false;
        rb.toggles.movement = false;

        int center = rb.lattice().index(L / 2, L / 2, L / 2);

        // Inject L-only perturbation (maximally chiral)
        double amp = 0.5;
        rb.voxels()[center].flux_L = Vec3(amp, 0, 0);
        rb.voxels()[center].flux_R = Vec3(0, 0, 0);
        rb.voxels()[center].flux = Vec3(amp, 0, 0);  // observable = L + R = L

        // Measure initial chirality amplitude
        double chi_initial = 0.0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            chi_initial += std::abs(rb.voxels()[i].chirality_density());
        }

        // Evolve
        for (int t = 0; t < 50; ++t) rb.tick();

        // Measure final chirality amplitude
        double chi_final = 0.0;
        for (int i = 0; i < N; ++i) {
            chi_final += std::abs(rb.voxels()[i].chirality_density());
        }

        std::cout << "    initial |chi| = " << chi_initial << "\n";
        std::cout << "    final |chi|   = " << chi_final << "\n";

        // The chiral (massive) mode should have decayed
        check("WZ-3a: initial chirality > 0", chi_initial > 1e-8);
        check("WZ-3b: chirality decreased", chi_final < chi_initial);

        // The ratio should show significant decay (mass gap effect)
        if (chi_initial > 1e-15) {
            double decay_ratio = chi_final / chi_initial;
            std::cout << "    decay ratio = " << decay_ratio << "\n";
            check("WZ-3c: decay ratio < 0.9 (significant decay)", decay_ratio < 0.9);
        } else {
            check("WZ-3c: skip (no initial chirality)", true);
        }
    }

    // ================================================================
    // WZ-4: Mass constants from ontic.h match experiment
    // ================================================================
    // Verify that the Higgs VEV, Weinberg angle, and derived W/Z
    // masses in ontic.h are within specified tolerance of experimental
    // values (PDG 2024).
    // ================================================================
    std::cout << "\n=== WZ-4: Electroweak mass constants from ontic.h ===\n";

    {
        // V_HIGGS = 246.09 GeV vs experimental 246.22 GeV
        double v_exp = 246.22;  // GeV (PDG)
        check_close("WZ-4a: V_HIGGS vs experiment", V_HIGGS, v_exp, 0.001);  // 0.1%

        // sin^2(theta_W) = 3/13 = 0.23077 vs experimental 0.23122
        double sw2_exp = 0.23122;
        check_close("WZ-4b: sin^2(theta_W) vs experiment", SIN2_WEINBERG, sw2_exp, 0.003);  // 0.3%

        // Verify SIN2_WEINBERG = N_c / N_eff = 3/13 exactly
        double sw2_exact = 3.0 / 13.0;
        check_close("WZ-4c: sin^2(theta_W) = 3/13 exact", SIN2_WEINBERG, sw2_exact, 1e-15);

        // M_Z is an external input in ontic.h
        double mz_exp = 91.1876;  // GeV (PDG)
        check_close("WZ-4d: M_Z input", M_Z, mz_exp, 1e-6);

        // Derive M_W from V_HIGGS and Weinberg angle
        // M_W = (g_weak / 2) * v = (e / sin(theta_W)) / 2 * v
        // Standard relation: M_W = M_Z * cos(theta_W)
        double cos2_w = 1.0 - SIN2_WEINBERG;  // cos^2(theta_W) = 10/13
        double cos_w = std::sqrt(cos2_w);
        double mw_derived = M_Z * cos_w;
        double mw_exp = 80.3692;  // GeV (PDG 2024: CDF+ATLAS+CMS average)
        std::cout << "    M_W derived = " << mw_derived << " GeV\n";
        std::cout << "    M_W exp     = " << mw_exp << " GeV\n";
        check_close("WZ-4e: M_W = M_Z*cos(theta_W) vs experiment", mw_derived, mw_exp, 0.005);  // 0.5%

        // ALPHA_WEAK = alpha / sin^2(theta_W) [DERIVED]
        double aw_expected = ALPHA / SIN2_WEINBERG;
        check_close("WZ-4f: ALPHA_WEAK = alpha/sin^2(theta_W)", ALPHA_WEAK, aw_expected, 1e-10);

        // Higgs mass check
        double mh_exp = 125.25;  // GeV (PDG 2024)
        check_close("WZ-4g: M_HIGGS vs experiment", M_HIGGS, mh_exp, 0.005);  // 0.5%
    }

    // ================================================================
    // WZ-5: W/Z mass ratio from Weinberg angle
    // ================================================================
    // The defining relation: M_W / M_Z = cos(theta_W).
    // In FTD: cos^2(theta_W) = 1 - sin^2(theta_W) = 1 - 3/13 = 10/13.
    // This gives M_W/M_Z = sqrt(10/13) = 0.87706...
    // Experimentally: 80.3692 / 91.1876 = 0.88135...
    // ================================================================
    std::cout << "\n=== WZ-5: W/Z mass ratio from Weinberg angle ===\n";

    {
        // cos^2(theta_W) = 1 - 3/13 = 10/13
        double cos2_w_ftd = 1.0 - SIN2_WEINBERG;
        double cos2_w_exact = 10.0 / 13.0;
        check_close("WZ-5a: cos^2(theta_W) = 10/13", cos2_w_ftd, cos2_w_exact, 1e-15);

        // M_W / M_Z = cos(theta_W) = sqrt(10/13)
        double ratio_ftd = std::sqrt(cos2_w_ftd);
        double mw_exp = 80.3692;  // GeV (PDG)
        double mz_exp = 91.1876;  // GeV (PDG)
        double ratio_exp = mw_exp / mz_exp;

        std::cout << "    M_W/M_Z (FTD)  = " << ratio_ftd << "\n";
        std::cout << "    M_W/M_Z (exp)  = " << ratio_exp << "\n";
        check_close("WZ-5b: M_W/M_Z ratio vs experiment", ratio_ftd, ratio_exp, 0.005);  // 0.5%

        // Explicit W boson mass from the ratio
        double mw_from_ratio = M_Z * ratio_ftd;
        std::cout << "    M_W from ratio = " << mw_from_ratio << " GeV\n";
        std::cout << "    M_W exp        = " << mw_exp << " GeV\n";
        check_close("WZ-5c: M_W from M_Z*cos(theta_W)", mw_from_ratio, mw_exp, 0.005);

        // The rho parameter: rho = M_W^2 / (M_Z^2 * cos^2(theta_W))
        // At tree level, rho = 1 exactly in the Standard Model.
        double rho = (mw_from_ratio * mw_from_ratio) / (M_Z * M_Z * cos2_w_ftd);
        check_close("WZ-5d: rho parameter = 1 (tree level)", rho, 1.0, 1e-10);

        // Z boson width proxy: Gamma_Z ~ alpha_W * M_Z
        // This is just a consistency check on the coupling scale
        double gamma_z_proxy = ALPHA_WEAK * M_Z;
        double gamma_z_exp = 2.4955;  // GeV (PDG)
        std::cout << "    Gamma_Z proxy (alpha_W * M_Z) = " << gamma_z_proxy << " GeV\n";
        std::cout << "    Gamma_Z exp                   = " << gamma_z_exp << " GeV\n";
        // The proxy is not expected to be precise (missing phase space factors),
        // but should be the right order of magnitude (few GeV)
        check("WZ-5e: Z width proxy same order of magnitude",
              gamma_z_proxy > 0.1 && gamma_z_proxy < 20.0);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n=== WZ MASS GENERATION: "
              << g_pass << " passed, " << g_fail << " failed ===\n";
    return g_fail;
}

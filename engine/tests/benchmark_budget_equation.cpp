// BUDGET EQUATION EXPERIMENT
//
// Tests: x/K + G_star/x = 1
//
// The budget equation says the coupling x partitions between two phases:
//   Coulomb (deconfined): fraction = x/K ~ 0.978
//   Confined: fraction = G_star/x ~ 0.022
//   Sum = 1 (completeness)
//
// Physical meaning: the flux field around a charge has a long-range
// (Coulomb, 1/r) component and a short-range (confined, exponential)
// component. The budget equation predicts their relative weights.
//
// Method: Helmholtz decomposition of the flux field J into:
//   J_long (longitudinal, curl-free) -- carries the Coulomb potential
//   J_trans (transverse, div-free) -- carries waves and confinement
//
// Budget prediction:
//   E_div / E_total ~ x/K = 0.978
//   E_curl / E_total ~ G_star/x = 0.022
//
// If this holds, the budget equation is EMERGENT from the lattice dynamics.

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

int main(int argc, char* argv[]) {
    int L = (argc > 1) ? std::atoi(argv[1]) : 32;
    int ticks = (argc > 2) ? std::atoi(argv[2]) : 300;

    std::cout << "experiment,L,col1,col2,col3,col4,col5\n";

    // Budget equation predictions
    double x_plus = 1.0 / ftd::ALPHA;  // 137.036
    double K = 16.0 * ftd::G_STAR * ftd::G_STAR;  // 140.060
    double coulomb_fraction_theory = x_plus / K;     // 0.978
    double confined_fraction_theory = ftd::G_STAR / x_plus;  // 0.022

    std::cerr << "=== BUDGET EQUATION: x/K + G*/x = 1 ===\n";
    std::cerr << "  x+ = 1/alpha = " << x_plus << "\n";
    std::cerr << "  K = 16*G*^2 = " << K << "\n";
    std::cerr << "  G* = " << ftd::G_STAR << "\n";
    std::cerr << "  x+/K = " << coulomb_fraction_theory << " (Coulomb fraction)\n";
    std::cerr << "  G*/x+ = " << confined_fraction_theory << " (Confined fraction)\n";
    std::cerr << "  Sum = " << coulomb_fraction_theory + confined_fraction_theory << " (should = 1)\n\n";

    const int mid = L / 2;

    // ================================================================
    // Experiment A: Helmholtz decomposition of field energy
    //
    // E_long = energy in divergent (charge) component of J
    // E_trans = energy in transverse (wave/curl) component of J
    // ================================================================
    std::cerr << "  Exp A: Helmholtz decomposition (L=" << L << ")\n";
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.damping = false;
        rb.toggles.gravity = false;

        // Single charge at center
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        // Let field fully equilibrate
        rb.run(ticks);

        auto audit = rb.energy_audit();
        double E_field = audit.field_energy;  // total |J|^2
        double E_wave = audit.wave_energy;    // |wave_vel|^2

        // Compute longitudinal energy: E_long = sum (div J)^2
        // and transverse proxy: E_curl = sum |curl J|^2
        double E_div = 0, E_curl = 0;
        int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            double d = rb.divergence_flux(i);
            E_div += d * d;

            ftd::Vec3 c = rb.curl_flux(i);
            E_curl += c.x*c.x + c.y*c.y + c.z*c.z;
        }

        double E_total = E_div + E_curl;
        double div_fraction = (E_total > 1e-30) ? E_div / E_total : 0;
        double curl_fraction = (E_total > 1e-30) ? E_curl / E_total : 0;

        std::cout << "helmholtz," << L << ","
                  << std::setprecision(8) << div_fraction << ","
                  << std::setprecision(8) << curl_fraction << ","
                  << std::setprecision(8) << coulomb_fraction_theory << ","
                  << std::setprecision(8) << confined_fraction_theory << ","
                  << (div_fraction + curl_fraction) << "\n";

        std::cerr << "    E_div_fraction  = " << div_fraction
                  << " (theory x/K = " << coulomb_fraction_theory << ")\n";
        std::cerr << "    E_curl_fraction = " << curl_fraction
                  << " (theory G*/x = " << confined_fraction_theory << ")\n";
        std::cerr << "    Sum = " << div_fraction + curl_fraction << "\n";
        std::cerr << "    E_field=" << E_field << " E_wave=" << E_wave << "\n";
    }

    // ================================================================
    // Experiment B: Radial energy profile E(r)
    //
    // Measure energy in radial shells. Coulomb component falls as 1/r^4,
    // confined component falls exponentially.
    // ================================================================
    std::cerr << "\n  Exp B: Radial energy profile (L=" << L << ")\n";
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.damping = false;
        rb.toggles.gravity = false;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(ticks);

        // Radial bins
        int max_r = L / 2 - 1;
        std::vector<double> E_shell(max_r + 1, 0);
        std::vector<int> count_shell(max_r + 1, 0);

        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            auto c = rb.lattice().coord(i);
            double dx = c.x - mid, dy = c.y - mid, dz = c.z - mid;
            int r = static_cast<int>(std::sqrt(dx*dx + dy*dy + dz*dz) + 0.5);
            if (r <= max_r) {
                E_shell[r] += rb.voxels()[i].density() * rb.voxels()[i].density();
                count_shell[r]++;
            }
        }

        // Output radial profile
        for (int r = 1; r <= std::min(max_r, 14); ++r) {
            double E_avg = (count_shell[r] > 0) ? E_shell[r] / count_shell[r] : 0;
            // Theory: Coulomb energy density ~ 1/r^4
            double E_coulomb_theory = (r > 0) ? 1.0 / (r * r * r * r) : 0;

            std::cout << "radial_energy," << L << ","
                      << r << "," << std::setprecision(8) << E_avg << ","
                      << std::setprecision(8) << E_shell[r] << ","
                      << count_shell[r] << ",0\n";
        }

        // Fit log(E_avg) vs log(r) for power law
        std::vector<double> lr, le;
        for (int r = 2; r <= std::min(max_r, 12); ++r) {
            double E_avg = (count_shell[r] > 0) ? E_shell[r] / count_shell[r] : 0;
            if (E_avg > 1e-30) {
                lr.push_back(std::log(static_cast<double>(r)));
                le.push_back(std::log(E_avg));
            }
        }
        if (lr.size() >= 3) {
            int n = static_cast<int>(lr.size());
            double sx = 0, sy = 0, sxx = 0, sxy = 0;
            for (int i = 0; i < n; ++i) {
                sx += lr[i]; sy += le[i]; sxx += lr[i]*lr[i]; sxy += lr[i]*le[i];
            }
            double denom = n*sxx - sx*sx;
            double exponent = (std::abs(denom) > 1e-30) ? (n*sxy - sx*sy)/denom : 0;

            std::cout << "radial_exponent," << L << ","
                      << std::setprecision(4) << exponent << ",-4,0,0,0\n";
            std::cerr << "    Energy exponent: " << exponent << " (Coulomb theory: -4)\n";
        }
    }

    // ================================================================
    // Experiment C: Budget equation direct test
    //
    // Measure the coupling from field energy at multiple radii.
    // At each r, the "effective coupling" x_eff(r) should satisfy
    // x_eff/K + G*/x_eff ≈ 1 if the budget equation holds.
    // ================================================================
    std::cerr << "\n  Exp C: Budget equation direct test\n";
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.damping = false;
        rb.toggles.gravity = false;

        // Two opposite charges at multiple separations
        for (int r = 4; r <= std::min(L / 3, 12); r += 2) {
            ftd::RenderBridge rb2(L);
            rb2.toggles.genesis = false;
            rb2.toggles.damping = false;
            rb2.toggles.gravity = false;

            rb2.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb2.voxels()[rb2.lattice().index(mid, mid, mid)].locked = true;
            rb2.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
            rb2.voxels()[rb2.lattice().index(mid + r, mid, mid)].locked = true;

            rb2.run(ticks);

            // Measure force via Poisson solver
            double F = rb2.force_diag_at(mid + r, mid, mid).f_coulomb.mag();
            // Extract effective coupling: F = x_eff / (4*pi*r^2) → x_eff = F*4*pi*r^2
            // Wait, force uses alpha not x. F = alpha/(4pi r^2). So x_eff = 1/(F*4pi*r^2)
            // Actually at Scale 0, F = alpha * s * grad(phi) ≈ alpha / r^2
            // So alpha_eff = F * r^2 (roughly), and x_eff = 1/alpha_eff
            double alpha_eff = F * 4.0 * ftd::PI * r * r;
            double x_eff = (alpha_eff > 1e-30) ? 1.0 / alpha_eff : 0;

            double budget_sum = (x_eff > 0) ? x_eff / K + ftd::G_STAR / x_eff : 0;

            std::cout << "budget_test," << L << ","
                      << r << "," << std::setprecision(6) << x_eff << ","
                      << std::setprecision(6) << budget_sum << ","
                      << std::setprecision(6) << alpha_eff << ",0\n";

            std::cerr << "    r=" << r << " alpha_eff=" << alpha_eff
                      << " x_eff=" << x_eff
                      << " budget=" << budget_sum << " (should=1)\n";
        }
    }

    // ================================================================
    // Experiment D: Vieta product/sum verification
    //
    // The master quadratic x^2 - Kx + KG* = 0 has:
    //   x+ + x- = K = 16*G*^2   (Vieta sum)
    //   x+ * x- = K*G* = 16*G*^3  (Vieta product)
    //
    // These are purely algebraic consequences. But on the lattice,
    // we can measure x+ (from Coulomb force) and check if x- = K - x+
    // gives the strong coupling.
    // ================================================================
    std::cerr << "\n  Exp D: Vieta relations from lattice\n";
    {
        // Use the best alpha measurement from the Coulomb benchmark
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        int r = 7;  // optimal radius from previous benchmarks
        rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;
        rb.run(ticks);

        double F = rb.force_diag_at(mid + r, mid, mid).f_coulomb.mag();
        double alpha_measured = F * 4.0 * ftd::PI * r * r;
        double x_plus_measured = (alpha_measured > 1e-30) ? 1.0 / alpha_measured : 0;

        // From Vieta: x- = K - x+
        double x_minus_vieta = K - x_plus_measured;

        // Theory values
        double x_plus_theory = 1.0 / ftd::ALPHA;
        double x_minus_theory = ftd::X_MINUS;

        std::cout << "vieta_sum," << L << ","
                  << std::setprecision(6) << x_plus_measured << ","
                  << std::setprecision(6) << x_minus_vieta << ","
                  << std::setprecision(6) << x_plus_theory << ","
                  << std::setprecision(6) << x_minus_theory << ",0\n";

        std::cerr << "    x+ measured = " << x_plus_measured << " (theory: " << x_plus_theory << ")\n";
        std::cerr << "    x- from Vieta = " << x_minus_vieta << " (theory: " << x_minus_theory << ")\n";
        std::cerr << "    If x- ≈ 3.024, the master quadratic is confirmed on the lattice\n";
    }

    std::cerr << "\n=== BUDGET EQUATION: Is x/K + G*/x = 1 the Schrodinger equation of couplings? ===\n";
    std::cerr << "  Schrodinger: Hpsi = Epsi (eigenvalues of energy)\n";
    std::cerr << "  Budget:      x/K + G*/x = 1 (eigenvalues of coupling)\n";
    std::cerr << "  Both are self-consistency equations with discrete spectra.\n";
    std::cerr << "  Both arise from variational principles.\n";
    std::cerr << "  Schrodinger has infinitely many eigenvalues.\n";
    std::cerr << "  Budget has exactly two: alpha (EM) and N_c (strong).\n";

    return 0;
}

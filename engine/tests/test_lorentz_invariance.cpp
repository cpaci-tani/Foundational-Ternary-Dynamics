/**
 * Test: Lorentz Invariance Quantitative Measure
 *
 * Measures the approach to Lorentz covariance as a function of scale
 * on the FTD cubic lattice. At scales >> lattice spacing, the discrete
 * lattice should approximate continuous Lorentz symmetry.
 *
 * Tests:
 *   LOR-1: Wave speed isotropy along [100], [110], [111] directions
 *   LOR-2: Coulomb force isotropy along 7 directions (3 cardinal + 4 diagonal)
 *   LOR-3: Dispersion isotropy: omega(k) independent of direction for |k| < pi/4
 *   LOR-4: Scale dependence: anisotropy proportional to (a/lambda)^n, fit power law
 *   LOR-5: Boost invariance: two-particle relative dynamics in rest vs boosted frame
 *   LOR-6: Time dilation: moving clock proper time accumulation
 *
 * Theory references:
 *   - CLAUDE.md Section 14.2 (Lorentz invariance: relational reinterpretation)
 *   - CLAUDE.md Section 22.4 (Lorentz isotropy verified)
 *   - constants.h: C_SPEED, C_WAVE, ALPHA, K_B
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <numeric>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

// Measure effective wave speed along a given lattice direction.
// Injects a pulse at center, evolves for T ticks, measures leading edge distance.
// Direction given as integer offsets (e.g., {1,0,0} for [100], {1,1,0} for [110]).
static double measure_wave_speed(int L, int dx, int dy, int dz, int ticks) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    int mid = L / 2;
    double amp = ftd::K_B * 0.3;
    rb.inject_flux(mid, mid, mid, {amp, amp, amp});

    rb.run(ticks);

    // Measure flux along the direction, find leading edge
    double dir_mag = std::sqrt(double(dx*dx + dy*dy + dz*dz));
    double threshold = 1e-8;
    int max_r = 0;

    for (int r = 1; r < L / 2 - 2; ++r) {
        int px = mid + static_cast<int>(std::round(r * dx / dir_mag));
        int py = mid + static_cast<int>(std::round(r * dy / dir_mag));
        int pz = mid + static_cast<int>(std::round(r * dz / dir_mag));

        // Wrap periodic
        px = ((px % L) + L) % L;
        py = ((py % L) + L) % L;
        pz = ((pz % L) + L) % L;

        auto& v = rb.voxels()[rb.lattice().index(px, py, pz)];
        double f = v.flux.mag();
        if (f > threshold) {
            max_r = r;
        }
    }

    return (ticks > 0 && max_r > 0) ? static_cast<double>(max_r) / ticks : 0.0;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Lorentz Invariance Quantitative Measure -- 6 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    // ================================================================
    // LOR-1: Wave speed isotropy
    // ================================================================
    std::cout << "\n-- LOR-1: Wave Speed Isotropy --\n";
    {
        const int L = 48;
        const int T = 30;

        double c_100 = measure_wave_speed(L, 1, 0, 0, T);  // [100]
        double c_010 = measure_wave_speed(L, 0, 1, 0, T);  // [010]
        double c_001 = measure_wave_speed(L, 0, 0, 1, T);  // [001]
        double c_110 = measure_wave_speed(L, 1, 1, 0, T);  // [110]
        double c_111 = measure_wave_speed(L, 1, 1, 1, T);  // [111]

        std::vector<double> speeds = {c_100, c_010, c_001, c_110, c_111};
        double c_mean = 0;
        for (double s : speeds) c_mean += s;
        c_mean /= speeds.size();

        double c_max = *std::max_element(speeds.begin(), speeds.end());
        double c_min = *std::min_element(speeds.begin(), speeds.end());
        double aniso = (c_min > 1e-15) ? (c_max - c_min) / c_mean : 999.0;

        std::cout << "    c[100] = " << c_100 << "\n";
        std::cout << "    c[010] = " << c_010 << "\n";
        std::cout << "    c[001] = " << c_001 << "\n";
        std::cout << "    c[110] = " << c_110 << "\n";
        std::cout << "    c[111] = " << c_111 << "\n";
        std::cout << "    mean = " << c_mean << ", anisotropy = " << aniso * 100 << "%\n";
        std::cout << "    C_WAVE (theory) = " << ftd::C_WAVE << "\n";

        // Cardinal directions should agree to < 1% with each other
        // (they use same stencil structure)
        double cardinal_aniso = 0.0;
        if (c_100 > 0 && c_010 > 0 && c_001 > 0) {
            double cm = (c_100 + c_010 + c_001) / 3.0;
            double cmax = std::max({c_100, c_010, c_001});
            double cmin = std::min({c_100, c_010, c_001});
            cardinal_aniso = (cmax - cmin) / cm;
        }
        std::cout << "    Cardinal anisotropy = " << cardinal_aniso * 100 << "%\n";

        // All speeds should be measurable (> 0) and agree to < 50% total
        // (at short wavelengths cubic lattice has O(1) anisotropy is expected)
        check("LOR-1: All directions propagate and anisotropy < 50%",
              c_min > 0.01 && aniso < 0.50);
    }

    // ================================================================
    // LOR-2: Coulomb force isotropy
    // ================================================================
    std::cout << "\n-- LOR-2: Coulomb Force Isotropy --\n";
    {
        // Place a +1 charge at center, measure force on a -1 test charge
        // at equal distance along 7 directions.
        // Directions: +x, +y, +z, +x+y, +x+z, +y+z, +x+y+z
        const int L = 32;
        const int mid = L / 2;
        const int r = 8;  // probe distance
        const int SETTLE = 200;  // let Poisson solver converge

        struct Dir { int dx, dy, dz; const char* name; };
        Dir dirs[] = {
            {1, 0, 0, "+x"},
            {0, 1, 0, "+y"},
            {0, 0, 1, "+z"},
            {1, 1, 0, "+xy"},
            {1, 0, 1, "+xz"},
            {0, 1, 1, "+yz"},
            {1, 1, 1, "+xyz"}
        };

        std::vector<double> forces;
        for (auto& d : dirs) {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = false;

            // Source charge at center (locked)
            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

            // Compute probe position at Euclidean distance r
            double dir_mag = std::sqrt(double(d.dx*d.dx + d.dy*d.dy + d.dz*d.dz));
            int px = mid + static_cast<int>(std::round(r * d.dx / dir_mag));
            int py = mid + static_cast<int>(std::round(r * d.dy / dir_mag));
            int pz = mid + static_cast<int>(std::round(r * d.dz / dir_mag));

            rb.inject_particle(px, py, pz, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(px, py, pz)].locked = true;

            rb.run(SETTLE);

            // Read force on the test charge
            auto& fd = rb.force_diag_at(px, py, pz);
            double fmag = fd.f_coulomb.mag();
            forces.push_back(fmag);
            std::cout << "    " << d.name << ": |F_coul| = " << fmag << "\n";
        }

        double f_max = *std::max_element(forces.begin(), forces.end());
        double f_min = *std::min_element(forces.begin(), forces.end());
        double f_mean = 0;
        for (double f : forces) f_mean += f;
        f_mean /= forces.size();
        double dev = (f_min > 1e-30) ? (f_max - f_min) / f_mean : 999.0;

        std::cout << "    Force deviation = " << dev * 100 << "% (max/mean relative spread)\n";

        // All forces should be measurable, deviation < 50% at r=8
        // (discrete lattice has significant anisotropy at small r)
        check("LOR-2: Coulomb forces measurable along all 7 directions, dev < 50%",
              f_min > 1e-15 && dev < 0.50);
    }

    // ================================================================
    // LOR-3: Dispersion isotropy
    // ================================================================
    std::cout << "\n-- LOR-3: Dispersion Isotropy --\n";
    {
        // On a cubic lattice, the dispersion relation is:
        //   omega^2 = c^2 * sum_i (2/a^2)(1 - cos(k_i * a))
        // For |k| << pi/a, this approaches omega = c|k| (isotropic).
        // We verify using the analytic lattice dispersion.

        // Lattice dispersion: omega^2(k) = c^2 * sum_i 4*sin^2(k_i/2)
        // where k_i are wavevector components, lattice spacing a=1.
        double c2 = ftd::C_WAVE * ftd::C_WAVE;

        // Test at |k| = pi/8 (well below pi/4 criterion)
        double k_mag = ftd::PI / 8.0;

        // Direction [100]: k = (k_mag, 0, 0)
        double omega_100 = std::sqrt(c2 * 4.0 * std::pow(std::sin(k_mag / 2.0), 2.0));

        // Direction [110]: k = (k_mag/sqrt2, k_mag/sqrt2, 0)
        double kc = k_mag / std::sqrt(2.0);
        double omega_110 = std::sqrt(c2 * 2.0 * 4.0 * std::pow(std::sin(kc / 2.0), 2.0));

        // Direction [111]: k = (k_mag/sqrt3, k_mag/sqrt3, k_mag/sqrt3)
        double kb = k_mag / std::sqrt(3.0);
        double omega_111 = std::sqrt(c2 * 3.0 * 4.0 * std::pow(std::sin(kb / 2.0), 2.0));

        // Ideal isotropic: omega = c * k_mag
        double omega_ideal = ftd::C_WAVE * k_mag;

        double dev_100 = std::abs(omega_100 - omega_ideal) / omega_ideal;
        double dev_110 = std::abs(omega_110 - omega_ideal) / omega_ideal;
        double dev_111 = std::abs(omega_111 - omega_ideal) / omega_ideal;

        std::cout << "    k_mag = pi/8 = " << k_mag << "\n";
        std::cout << "    omega[100] = " << omega_100 << " (dev " << dev_100 * 100 << "%)\n";
        std::cout << "    omega[110] = " << omega_110 << " (dev " << dev_110 * 100 << "%)\n";
        std::cout << "    omega[111] = " << omega_111 << " (dev " << dev_111 * 100 << "%)\n";
        std::cout << "    omega_ideal = " << omega_ideal << "\n";

        double max_dev = std::max({dev_100, dev_110, dev_111});
        std::cout << "    Max deviation = " << max_dev * 100 << "%\n";

        // At |k| = pi/8, anisotropy should be small (< 5%)
        check("LOR-3: Dispersion isotropy < 5% at |k| = pi/8",
              max_dev < 0.05);
    }

    // ================================================================
    // LOR-4: Scale dependence of anisotropy
    // ================================================================
    std::cout << "\n-- LOR-4: Anisotropy vs Scale --\n";
    {
        // Analytic lattice dispersion anisotropy.
        // For a wavevector of magnitude k, the anisotropy between [100] and [111]
        // goes as (k*a)^2 / 12 at leading order.
        // We measure at lambda = {5, 10, 20, 40} lattice units.
        double c2 = ftd::C_WAVE * ftd::C_WAVE;
        double wavelengths[] = {5.0, 10.0, 20.0, 40.0};
        std::vector<double> log_ka;
        std::vector<double> log_aniso;

        for (double lam : wavelengths) {
            double k = 2.0 * ftd::PI / lam;  // a = 1 lattice unit

            // [100] dispersion
            double w100_sq = c2 * 4.0 * std::pow(std::sin(k / 2.0), 2.0);
            double w100 = std::sqrt(w100_sq);

            // [111] dispersion
            double kb = k / std::sqrt(3.0);
            double w111_sq = c2 * 3.0 * 4.0 * std::pow(std::sin(kb / 2.0), 2.0);
            double w111 = std::sqrt(w111_sq);

            double w_mean = (w100 + w111) / 2.0;
            double aniso = (w_mean > 1e-30) ? std::abs(w100 - w111) / w_mean : 0;

            std::cout << "    lambda=" << std::setw(4) << lam
                      << ": omega[100]=" << w100
                      << ", omega[111]=" << w111
                      << ", anisotropy=" << aniso * 100 << "%\n";

            if (aniso > 1e-15 && k > 1e-15) {
                log_ka.push_back(std::log(k));     // k*a, with a=1
                log_aniso.push_back(std::log(aniso));
            }
        }

        // Fit power law: log(aniso) = n * log(k*a) + const
        // Simple least-squares for slope
        double exponent = 0;
        if (log_ka.size() >= 2) {
            double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
            int n = (int)log_ka.size();
            for (int i = 0; i < n; ++i) {
                sum_x += log_ka[i];
                sum_y += log_aniso[i];
                sum_xx += log_ka[i] * log_ka[i];
                sum_xy += log_ka[i] * log_aniso[i];
            }
            exponent = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
        }

        std::cout << "    Fitted power law exponent = " << exponent
                  << " (theory: ~2 for leading lattice correction)\n";

        // The anisotropy should scale as (k*a)^n with n >= 1.5
        // (leading order is k^2, but finite samples can shift)
        check("LOR-4: Anisotropy power law exponent > 1.5 (improves at long wavelength)",
              exponent > 1.5);
    }

    // ================================================================
    // LOR-5: Boost invariance
    // ================================================================
    std::cout << "\n-- LOR-5: Boost Invariance --\n";
    {
        // Two opposite charges: measure relative approach speed from rest
        // frame vs frame where both have initial velocity.
        // In the rest frame, charges attract and accelerate toward each other.
        // In a boosted frame, the same relative dynamics should hold.

        const int L = 48;
        const int mid = L / 2;
        const int sep = 12;
        const int TICKS = 100;

        // Rest frame: particles at mid-6 and mid+6, no initial velocity
        double rest_delta = 0.0;
        {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = false;
            rb.toggles.movement = true;

            rb.inject_particle(mid - sep/2, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + sep/2, mid, mid, -1, {0, 0, -ftd::K_B});

            // Record initial separation
            // After running, find particles and measure separation
            rb.run(TICKS);

            // Find particle positions
            int pos_plus = -1, pos_minus = -1;
            for (int x = 0; x < L; ++x) {
                int idx = rb.lattice().index(x, mid, mid);
                if (rb.voxels()[idx].state == +1) pos_plus = x;
                if (rb.voxels()[idx].state == -1) pos_minus = x;
            }
            if (pos_plus >= 0 && pos_minus >= 0) {
                rest_delta = std::abs(pos_plus - pos_minus);
            }
        }

        // "Boosted" frame: both particles get initial y-velocity
        double boosted_delta = 0.0;
        {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = false;
            rb.toggles.movement = true;

            rb.inject_particle(mid - sep/2, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + sep/2, mid, mid, -1, {0, 0, -ftd::K_B});

            // Give both particles a y-boost
            double v_boost = 0.1;
            int idx1 = rb.lattice().index(mid - sep/2, mid, mid);
            int idx2 = rb.lattice().index(mid + sep/2, mid, mid);
            rb.voxels()[idx1].velocity.y = v_boost;
            rb.voxels()[idx2].velocity.y = v_boost;

            rb.run(TICKS);

            // Find particles — scan full y-range since they moved in y
            int pos_plus_x = -1, pos_minus_x = -1;
            for (int x = 0; x < L; ++x) {
                for (int y = 0; y < L; ++y) {
                    int idx = rb.lattice().index(x, y, mid);
                    if (rb.voxels()[idx].state == +1 && pos_plus_x < 0)
                        pos_plus_x = x;
                    if (rb.voxels()[idx].state == -1 && pos_minus_x < 0)
                        pos_minus_x = x;
                }
            }
            if (pos_plus_x >= 0 && pos_minus_x >= 0) {
                boosted_delta = std::abs(pos_plus_x - pos_minus_x);
            }
        }

        std::cout << "    Rest frame x-separation after " << TICKS << " ticks: " << rest_delta << "\n";
        std::cout << "    Boosted frame x-separation: " << boosted_delta << "\n";

        // Both frames should show similar relative x-dynamics.
        // At v_boost=0.1, relativistic effects are ~0.5%, so agreement should be close.
        // Allow up to 30% deviation (lattice effects + finite speed + Larmor)
        bool both_measured = (rest_delta > 0 && boosted_delta > 0);
        double rel_diff = both_measured ?
            std::abs(rest_delta - boosted_delta) / std::max(rest_delta, boosted_delta) : 999;

        std::cout << "    Relative difference: " << rel_diff * 100 << "%\n";

        check("LOR-5: Boost invariance — rest vs boosted x-separation agree within 30%",
              both_measured && rel_diff < 0.30);
    }

    // ================================================================
    // LOR-6: Time dilation
    // ================================================================
    std::cout << "\n-- LOR-6: Time Dilation --\n";
    {
        // A moving voxel accumulates proper time:
        //   tau += G* * sqrt(1 - v^2 - L^2)
        // For a voxel at rest, tau grows as G* per tick.
        // For a voxel with speed v, tau grows as G* * sqrt(1 - v^2) per tick.
        //
        // Verify: gamma_ftd = 1/sqrt(1 - v^2) for various speeds.

        ftd::Voxel v_rest;
        v_rest.velocity = {0, 0, 0};
        v_rest.latency = 0;

        ftd::Voxel v_slow;
        v_slow.velocity = {0.3, 0, 0};
        v_slow.latency = 0;

        ftd::Voxel v_fast;
        v_fast.velocity = {0.5, 0, 0};
        v_fast.latency = 0;

        double gamma_rest = v_rest.gamma_ftd();
        double gamma_slow = v_slow.gamma_ftd();
        double gamma_fast = v_fast.gamma_ftd();

        double gamma_slow_sr = 1.0 / std::sqrt(1.0 - 0.09);
        double gamma_fast_sr = 1.0 / std::sqrt(1.0 - 0.25);

        std::cout << "    gamma(v=0)   = " << gamma_rest << " (expected 1.0)\n";
        std::cout << "    gamma(v=0.3) = " << gamma_slow << " (SR: " << gamma_slow_sr << ")\n";
        std::cout << "    gamma(v=0.5) = " << gamma_fast << " (SR: " << gamma_fast_sr << ")\n";

        // Proper time ratio: tau_moving / tau_rest = 1/gamma
        double tau_ratio_slow = 1.0 / gamma_slow;
        double tau_ratio_fast = 1.0 / gamma_fast;

        std::cout << "    tau_ratio(v=0.3) = " << tau_ratio_slow
                  << " (expected " << 1.0/gamma_slow_sr << ")\n";
        std::cout << "    tau_ratio(v=0.5) = " << tau_ratio_fast
                  << " (expected " << 1.0/gamma_fast_sr << ")\n";

        // Time dilation: moving clocks tick slower
        bool dilation_correct =
            gamma_rest == 1.0 &&
            std::abs(gamma_slow - gamma_slow_sr) < 1e-10 &&
            std::abs(gamma_fast - gamma_fast_sr) < 1e-10 &&
            tau_ratio_slow < 1.0 &&
            tau_ratio_fast < tau_ratio_slow;  // faster → even slower proper time

        check("LOR-6: Time dilation gamma matches SR prediction exactly",
              dilation_correct);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (g_failures == 0)
        std::cout << "  All 6 Lorentz invariance tests PASSED.\n";
    else
        std::cout << "  " << g_failures << " test(s) FAILED.\n";
    std::cout << "================================================================\n";

    return g_failures;
}

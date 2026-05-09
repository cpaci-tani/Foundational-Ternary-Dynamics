/**
 * Test: Lorentz + Magnetic family (consolidated suite)
 *
 * Merges 5 legacy tests into test_lorentz.cpp (self-ref target) using
 * the Phase 2a ftd::test NDJSON telemetry API:
 *
 *   test_lorentz              -> section "lorentz_factor"      (14 checks)
 *   test_lorentz_force        -> section "lorentz_force"       ( 5 checks)
 *   test_lorentz_invariance   -> section "lorentz_invariance"  ( 6 checks)
 *   test_magnetic             -> section "magnetic"            ( 7 checks)
 *   test_magnetic_lagrangian  -> section "magnetic_lagrangian" ( 5 checks)
 *
 * Every check(...) preserved verbatim. Wave 4b.6 consolidation
 * (2026-04-14). test_lorentz_force, test_lorentz_invariance,
 * test_magnetic all had some failing checks pre-consolidation;
 * structural parity preserved.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/lagrangian.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: lorentz_factor  (from test_lorentz.cpp)
// ============================================================================

static double gamma_ftd_local(double v, double L) {
    double bw = v * v + L * L;
    if (bw >= 1.0) return 1e30;
    return 1.0 / std::sqrt(1.0 - bw);
}

static void section_lorentz_factor() {
    ftd::test::check_close("Rest flat: gamma = 1", gamma_ftd_local(0, 0), 1.0, 1e-15);

    double v_tests[] = {0.1, 0.3, 0.5, 0.8, 0.9, 0.99};
    for (double v : v_tests) {
        double g_ftd = gamma_ftd_local(v, 0);
        double g_sr = 1.0 / std::sqrt(1.0 - v*v);
        char buf[128];
        snprintf(buf, sizeof(buf), "SR v=%.2f: gamma_FTD = gamma_SR", v);
        ftd::test::check_close(buf, g_ftd, g_sr, 1e-12);
    }

    double rs_over_r = 0.1;
    double L = std::sqrt(rs_over_r);
    double g = gamma_ftd_local(0, L);
    double g_schwarz = 1.0 / std::sqrt(1.0 - rs_over_r);
    ftd::test::check_close("Grav dilation r_s/r=0.1", g, g_schwarz, 1e-12);

    double g_dm = gamma_ftd_local(0, 0.75);
    double expected = 1.0 / std::sqrt(1.0 - 0.75*0.75);
    ftd::test::check_close("Dark matter L=0.75", g_dm, expected, 1e-12);
    ftd::test::check("Dark matter gamma > 1.5", g_dm > 1.5);

    double v_combo = 0.6, L_combo = 0.6;
    double budget = v_combo*v_combo + L_combo*L_combo;
    ftd::test::check("v=0.6 L=0.6 budget < 1", budget < 1.0);
    double g_combo = gamma_ftd_local(v_combo, L_combo);
    double expected_combo = 1.0 / std::sqrt(1.0 - budget);
    ftd::test::check_close("Combined v=0.6 L=0.6", g_combo, expected_combo, 1e-12);

    double g_horizon = gamma_ftd_local(0, std::sqrt(0.999));
    ftd::test::check("Near horizon gamma > 30", g_horizon > 30.0);

    double budget_over = 0.8*0.8 + 0.7*0.7;
    ftd::test::check("v=0.8 L=0.7 budget > 1 (forbidden)", budget_over > 1.0);

    ftd::Voxel vox;
    vox.velocity = {0.3, 0.4, 0.0};
    vox.latency = 0.3;
    {
        double f = 1.0 - 0.3 * 0.3;
        double v2 = 0.25;
        double g_vox = vox.gamma_ftd();
        double g_expected = std::sqrt(f) / std::sqrt(f * f - v2);
        ftd::test::check_close("Voxel gamma_ftd v=0.5 L=0.3", g_vox, g_expected, 1e-12);
    }
    {
        double f = 1.0 - 0.3 * 0.3;
        double v2 = 0.25;
        double bi = vox.born_infeld_core();
        double bi_expected = -ftd::K_B * std::sqrt(f * f - v2) / std::sqrt(f);
        ftd::test::check_close("Voxel Born-Infeld core", bi, bi_expected, 1e-12);
    }
    {
        ftd::Voxel v0;
        v0.velocity = {0.5, 0.0, 0.0};
        v0.latency = 0.0;
        double g_vox = v0.gamma_ftd();
        double g_sr = 1.0 / std::sqrt(1.0 - 0.25);
        ftd::test::check_close("Voxel gamma_ftd v=0.5 L=0 matches SR", g_vox, g_sr, 1e-12);
    }
}

// ============================================================================
// Section: lorentz_force  (from test_lorentz_force.cpp)
// ============================================================================

static void section_lorentz_force() {
    // LF1
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.run(200);

        auto fd = rb.force_diag();
        int idx = rb.lattice().index(mid, mid, mid);
        double fmag = fd[idx].f_magnetic.mag();

        std::cout << std::setprecision(8) << std::scientific;
        std::cout << "    |F_lorentz| for stationary particle: " << fmag << "\n";
        ftd::test::check("LF1: Stationary particle has zero Lorentz force", fmag < 1e-15);
    }

    // LF2
    {
        ftd::RenderBridge rb(32);
        int mid = 16;

        for (int dx = -2; dx <= 2; ++dx) {
            double sign = (dx < 0) ? 1.0 : -1.0;
            rb.inject_flux(mid + dx, mid, mid, {0.0, sign * ftd::K_B * 0.5, 0.0});
        }

        rb.toggles.genesis = false;
        rb.run(5);

        rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
        auto& v = rb.voxels()[rb.lattice().index(mid, mid, mid)];
        v.velocity = {0.3, 0.0, 0.0};

        rb.tick();

        auto fd = rb.force_diag();
        bool found = false;
        double fmag = 0.0;
        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            if (rb.voxels()[i].state != 0) {
                fmag = fd[i].f_magnetic.mag();
                found = true;
                break;
            }
        }

        std::cout << "    |F_lorentz| for moving particle in curl: " << fmag << "\n";
        ftd::test::check("LF2: Moving particle has |F_lorentz| > 0 (or no particle found)", !found || fmag > 0);
    }

    // LF3
    {
        ftd::Vec3 v = {0.3, 0.2, 0.1};
        ftd::Vec3 B = {0.05, -0.03, 0.07};
        ftd::Vec3 F = ftd::Vec3::cross(v, B);
        double work = v.dot(F);

        std::cout << "    v = (" << v.x << ", " << v.y << ", " << v.z << ")\n";
        std::cout << "    B = (" << B.x << ", " << B.y << ", " << B.z << ")\n";
        std::cout << "    F = v×B = (" << F.x << ", " << F.y << ", " << F.z << ")\n";
        std::cout << "    v · F = " << work << "\n";
        ftd::test::check("LF3: Lorentz force does no work (|v·F| < 1e-15)", std::abs(work) < 1e-15);
    }

    // LF4
    {
        ftd::Vec3 v1 = {0.1, 0.0, 0.0};
        ftd::Vec3 v2 = {0.2, 0.0, 0.0};
        ftd::Vec3 B = {0.0, 0.0, 0.05};
        int8_t state = 1;

        ftd::Vec3 F1 = ftd::Vec3::cross(v1, B) * (ftd::ALPHA * state);
        ftd::Vec3 F2 = ftd::Vec3::cross(v2, B) * (ftd::ALPHA * state);

        double ratio = (F1.mag() > 1e-30) ? F2.mag() / F1.mag() : 0.0;
        std::cout << "    |F(v=0.1)| = " << F1.mag() << "\n";
        std::cout << "    |F(v=0.2)| = " << F2.mag() << "\n";
        std::cout << "    Ratio = " << std::setprecision(3) << std::fixed << ratio << " (expected 2.0)\n";
        ftd::test::check("LF4: |F| scales linearly with |v| (ratio ≈ 2.0)", std::abs(ratio - 2.0) < 0.01);
    }

    // LF5
    {
        ftd::RenderBridge rb(32);
        int mid = 16;
        rb.toggles.lorentz_force = false;

        rb.inject_particle(mid - 3, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + 3, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.run(100);

        double max_fmag = 0.0;
        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            double m = rb.force_diag()[i].f_magnetic.mag();
            if (m > max_fmag) max_fmag = m;
        }

        std::cout << std::setprecision(8) << std::scientific;
        std::cout << "    max |F_magnetic| with toggle off: " << max_fmag << "\n";
        ftd::test::check("LF5: Toggle off → zero magnetic force everywhere", max_fmag < 1e-15);
    }
}

// ============================================================================
// Section: lorentz_invariance  (from test_lorentz_invariance.cpp)
// ============================================================================

static double measure_wave_speed(int L, int dx, int dy, int dz, int ticks) {
    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    int mid = L / 2;
    double amp = ftd::K_B * 0.3;
    rb.inject_flux(mid, mid, mid, {amp, amp, amp});
    rb.run(ticks);

    double dir_mag = std::sqrt(double(dx*dx + dy*dy + dz*dz));
    double threshold = 1e-8;
    int max_r = 0;

    for (int r = 1; r < L / 2 - 2; ++r) {
        int px = mid + static_cast<int>(std::round(r * dx / dir_mag));
        int py = mid + static_cast<int>(std::round(r * dy / dir_mag));
        int pz = mid + static_cast<int>(std::round(r * dz / dir_mag));

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

static void section_lorentz_invariance() {
    std::cout << std::fixed << std::setprecision(6);

    // LOR-1
    {
        const int L = 48;
        const int T = 30;

        double c_100 = measure_wave_speed(L, 1, 0, 0, T);
        double c_010 = measure_wave_speed(L, 0, 1, 0, T);
        double c_001 = measure_wave_speed(L, 0, 0, 1, T);
        double c_110 = measure_wave_speed(L, 1, 1, 0, T);
        double c_111 = measure_wave_speed(L, 1, 1, 1, T);

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

        double cardinal_aniso = 0.0;
        if (c_100 > 0 && c_010 > 0 && c_001 > 0) {
            double cm = (c_100 + c_010 + c_001) / 3.0;
            double cmax = std::max({c_100, c_010, c_001});
            double cmin = std::min({c_100, c_010, c_001});
            cardinal_aniso = (cmax - cmin) / cm;
        }
        std::cout << "    Cardinal anisotropy = " << cardinal_aniso * 100 << "%\n";

        ftd::test::check("LOR-1: All directions propagate and anisotropy < 50%",
              c_min > 0.01 && aniso < 0.50);
    }

    // LOR-2
    {
        const int L = 32;
        const int mid = L / 2;
        const int r = 8;
        const int SETTLE = 200;

        struct Dir { int dx, dy, dz; const char* name; };
        Dir dirs[] = {
            {1, 0, 0, "+x"}, {0, 1, 0, "+y"}, {0, 0, 1, "+z"},
            {1, 1, 0, "+xy"}, {1, 0, 1, "+xz"}, {0, 1, 1, "+yz"},
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

            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

            double dir_mag = std::sqrt(double(d.dx*d.dx + d.dy*d.dy + d.dz*d.dz));
            int px = mid + static_cast<int>(std::round(r * d.dx / dir_mag));
            int py = mid + static_cast<int>(std::round(r * d.dy / dir_mag));
            int pz = mid + static_cast<int>(std::round(r * d.dz / dir_mag));

            rb.inject_particle(px, py, pz, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(px, py, pz)].locked = true;

            rb.run(SETTLE);

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

        ftd::test::check("LOR-2: Coulomb forces measurable along all 7 directions, dev < 50%",
              f_min > 1e-15 && dev < 0.50);
    }

    // LOR-3
    {
        double c2 = ftd::C_WAVE * ftd::C_WAVE;
        double k_mag = ftd::PI / 8.0;

        double omega_100 = std::sqrt(c2 * 4.0 * std::pow(std::sin(k_mag / 2.0), 2.0));
        double kc = k_mag / std::sqrt(2.0);
        double omega_110 = std::sqrt(c2 * 2.0 * 4.0 * std::pow(std::sin(kc / 2.0), 2.0));
        double kb = k_mag / std::sqrt(3.0);
        double omega_111 = std::sqrt(c2 * 3.0 * 4.0 * std::pow(std::sin(kb / 2.0), 2.0));

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

        ftd::test::check("LOR-3: Dispersion isotropy < 5% at |k| = pi/8",
              max_dev < 0.05);
    }

    // LOR-4
    {
        double c2 = ftd::C_WAVE * ftd::C_WAVE;
        double wavelengths[] = {5.0, 10.0, 20.0, 40.0};
        std::vector<double> log_ka;
        std::vector<double> log_aniso;

        for (double lam : wavelengths) {
            double k = 2.0 * ftd::PI / lam;
            double w100_sq = c2 * 4.0 * std::pow(std::sin(k / 2.0), 2.0);
            double w100 = std::sqrt(w100_sq);
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
                log_ka.push_back(std::log(k));
                log_aniso.push_back(std::log(aniso));
            }
        }

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

        ftd::test::check("LOR-4: Anisotropy power law exponent > 1.5 (improves at long wavelength)",
              exponent > 1.5);
    }

    // LOR-5
    {
        const int L = 48;
        const int mid = L / 2;
        const int sep = 12;
        const int TICKS = 100;

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
            rb.run(TICKS);

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

            double v_boost = 0.1;
            int idx1 = rb.lattice().index(mid - sep/2, mid, mid);
            int idx2 = rb.lattice().index(mid + sep/2, mid, mid);
            rb.voxels()[idx1].velocity.y = v_boost;
            rb.voxels()[idx2].velocity.y = v_boost;

            rb.run(TICKS);

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

        bool both_measured = (rest_delta > 0 && boosted_delta > 0);
        double rel_diff = both_measured ?
            std::abs(rest_delta - boosted_delta) / std::max(rest_delta, boosted_delta) : 999;

        std::cout << "    Relative difference: " << rel_diff * 100 << "%\n";

        ftd::test::check("LOR-5: Boost invariance — rest vs boosted x-separation agree within 30%",
              both_measured && rel_diff < 0.30);
    }

    // LOR-6
    {
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

        double tau_ratio_slow = 1.0 / gamma_slow;
        double tau_ratio_fast = 1.0 / gamma_fast;

        std::cout << "    tau_ratio(v=0.3) = " << tau_ratio_slow
                  << " (expected " << 1.0/gamma_slow_sr << ")\n";
        std::cout << "    tau_ratio(v=0.5) = " << tau_ratio_fast
                  << " (expected " << 1.0/gamma_fast_sr << ")\n";

        bool dilation_correct =
            gamma_rest == 1.0 &&
            std::abs(gamma_slow - gamma_slow_sr) < 1e-10 &&
            std::abs(gamma_fast - gamma_fast_sr) < 1e-10 &&
            tau_ratio_slow < 1.0 &&
            tau_ratio_fast < tau_ratio_slow;

        ftd::test::check("LOR-6: Time dilation gamma matches SR prediction exactly",
              dilation_correct);
    }

    std::cout << std::defaultfloat;
}

// ============================================================================
// Section: magnetic  (from test_magnetic.cpp)
// ============================================================================

static void section_magnetic() {
    // Section 1: Curl operator
    // F11 perf fix: force CPU. Triple-loop inject_flux fills all L³ voxels;
    // on GPU each call triggers a full ~3MB host→device upload (push_to_device),
    // making this O(L⁶) wall time. CPU completes in ms.
    {
        int L = 16;
        ftd::RenderBridge rb(L); rb.force_cpu();

        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    double cx = x - L/2.0;
                    double cy = y - L/2.0;
                    rb.inject_flux(x, y, z, {-cy, cx, 0});
                }
            }
        }

        int ci = rb.lattice().index(L/2, L/2, L/2);
        ftd::Vec3 B = rb.curl_flux(ci);
        std::cout << "    curl(J) at center = (" << B.x << ", " << B.y << ", " << B.z << ")\n";
        std::cout << "    Expected: (0, 0, 2)\n";

        ftd::test::check_close("Bx ≈ 0", B.x, 0.0, 0.1);
        ftd::test::check_close("By ≈ 0", B.y, 0.0, 0.1);
        ftd::test::check_close("Bz ≈ 2", B.z, 2.0, 0.1);
    }

    // Section 2: Perpendicularity
    // F11 perf fix: force CPU (see Section 1 note above). Without this, the
    // 32³=32K inject_flux calls each push the full voxel array to GPU,
    // causing test_lorentz to time out at default ctest 600s.
    {
        int L = 32;
        ftd::RenderBridge rb(L); rb.force_cpu();
        int cx = L / 2;

        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    double bx = x - L/2.0;
                    double by = y - L/2.0;
                    rb.inject_flux(x, y, z, {-by * 0.1, bx * 0.1, 0});
                }
            }
        }

        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cx, cx)].velocity = {0.3, 0, 0};

        double speed_before = rb.voxels()[rb.lattice().index(cx, cx, cx)].speed();

        rb.tick();

        double speed_after = rb.voxels()[rb.lattice().index(cx, cx, cx)].speed();

        std::cout << "    Speed before: " << speed_before << "\n";
        std::cout << "    Speed after:  " << speed_after << "\n";

        double speed_change = std::abs(speed_after - speed_before);
        std::cout << "    |Delta speed| = " << speed_change << "\n";

        ftd::test::check("Speed approximately preserved (magnetic force perpendicular)",
              speed_change < speed_before * 0.5);
    }

    // Section 3: Stationary
    // F11 perf fix: force CPU (see Section 1 note). 32³ inject_flux loop.
    {
        int L = 32;
        ftd::RenderBridge rb(L); rb.force_cpu();
        int cx = L / 2;

        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    double bx = x - L/2.0;
                    double by = y - L/2.0;
                    rb.inject_flux(x, y, z, {-by * 0.1, bx * 0.1, 0});
                }
            }
        }

        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});

        double vy_before = rb.voxels()[rb.lattice().index(cx, cx, cx)].velocity.y;

        rb.tick();

        double vy_after = rb.voxels()[rb.lattice().index(cx, cx, cx)].velocity.y;
        std::cout << "    vy_before = " << vy_before << "\n";
        std::cout << "    vy_after  = " << vy_after << "\n";

        ftd::test::check("No explosion from magnetic force at v=0",
              std::abs(vy_after) < 1.0);
    }

    // Section 4: Velocity dependence
    // F11 perf fix: force CPU (see Section 1 note). 16³ inject_flux loop.
    {
        int L = 16;
        ftd::RenderBridge rb(L); rb.force_cpu();
        int cx = L / 2;

        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    double bx = x - L/2.0;
                    double by = y - L/2.0;
                    rb.inject_flux(x, y, z, {-by, bx, 0});
                }
            }
        }

        int ci = rb.lattice().index(cx, cx, cx);
        ftd::Vec3 B = rb.curl_flux(ci);

        double q = 1.0;
        double v_slow = 0.1;
        double v_fast = 0.3;

        double F_slow = std::abs(ftd::G_C * q * v_slow * B.z);
        double F_fast = std::abs(ftd::G_C * q * v_fast * B.z);

        std::cout << "    B = (" << B.x << ", " << B.y << ", " << B.z << ")\n";
        std::cout << "    |F_mag| at v=0.1: " << F_slow << "\n";
        std::cout << "    |F_mag| at v=0.3: " << F_fast << "\n";

        ftd::test::check("Magnetic force at v=0.1 is nonzero", F_slow > 1e-10);
        ftd::test::check("Magnetic force at v=0.3 is nonzero", F_fast > 1e-10);

        if (F_slow > 1e-15) {
            double ratio = F_fast / F_slow;
            std::cout << "    ratio = " << ratio << " (expect 3.0 for F~v)\n";
            ftd::test::check_close("F_mag scales linearly with velocity", ratio, 3.0, 0.01);
        }
    }
}

// ============================================================================
// Section: magnetic_lagrangian  (from test_magnetic_lagrangian.cpp)
// ============================================================================

static void section_magnetic_lagrangian() {
    // Section 1: Zero for stationary
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = ftd::Vec3(0, 0, ftd::K_B);
        v.velocity = ftd::Vec3(0, 0, 0);

        double L_vel = ftd::velocity_coupling_term(v);
        ftd::test::check_close("L_VELOCITY = 0 for v=0", L_vel, 0.0, 1e-15);
    }

    // Section 2: Nonzero for moving
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = ftd::Vec3(0, 0, ftd::K_B);
        v.velocity = ftd::Vec3(0, 0, 0.5);

        double L_vel = ftd::velocity_coupling_term(v);
        double expected = -ftd::G_C * 1 * (0.5 * ftd::K_B);
        std::cout << "    L_VELOCITY = " << L_vel << "\n";
        std::cout << "    Expected   = " << expected << "\n";
        ftd::test::check_close("L_VELOCITY correct value", L_vel, expected, 1e-10);
        ftd::test::check("L_VELOCITY is nonzero", std::abs(L_vel) > 1e-10);
    }

    // Section 3: Charge-Dependent Sign
    {
        ftd::Voxel v_pos, v_neg;
        v_pos.state = +1;
        v_neg.state = -1;
        v_pos.flux = v_neg.flux = ftd::Vec3(1, 0, 0);
        v_pos.velocity = v_neg.velocity = ftd::Vec3(0.3, 0, 0);

        double L_pos = ftd::velocity_coupling_term(v_pos);
        double L_neg = ftd::velocity_coupling_term(v_neg);

        std::cout << "    L_vel(+1) = " << L_pos << "\n";
        std::cout << "    L_vel(-1) = " << L_neg << "\n";
        ftd::test::check_close("Opposite signs for opposite charges", L_pos + L_neg, 0.0, 1e-15);
    }

    // Section 4: Perpendicular
    {
        ftd::Voxel v;
        v.state = 1;
        v.flux = ftd::Vec3(0, 0, ftd::K_B);
        v.velocity = ftd::Vec3(0.5, 0, 0);

        double L_vel = ftd::velocity_coupling_term(v);
        ftd::test::check_close("L_VELOCITY = 0 when v perp J", L_vel, 0.0, 1e-15);
    }
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("test_lorentz");

    ftd::test::section("lorentz_factor");
    section_lorentz_factor();

    ftd::test::section("lorentz_force");
    section_lorentz_force();

    ftd::test::section("lorentz_invariance");
    section_lorentz_invariance();

    ftd::test::section("magnetic");
    section_magnetic();

    ftd::test::section("magnetic_lagrangian");
    section_magnetic_lagrangian();

    return ftd::test::finalize();
}

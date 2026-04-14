/**
 * Campaign: Wave dynamics (consolidated suite)
 *
 * Merges 4 legacy tests into a single ftd::test-instrumented suite using
 * the Phase 2a NDJSON telemetry API:
 *
 *   test_wave_speed          -> section "wave_speed"
 *   test_interference        -> section "interference"
 *   campaign_wave_isotropy   -> section "wave_isotropy"
 *   campaign_two_slit        -> section "two_slit"
 *
 * Every check(...) from the legacy files is preserved verbatim (same condition,
 * same label) and routed through ftd::test::check for uniform telemetry.
 *
 * Wave 4a.2 consolidation (2026-04-14). All 4 input families were
 * already passing individually; this is a pure structural merge.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <numeric>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: wave_speed  (from test_wave_speed.cpp)
// ============================================================================

static void section_wave_speed() {
    // Section 1: Measure wavefront propagation speed
    std::cout << "\n--- Section 1: Wavefront Speed Measurement ---\n";
    {
        int L = 64;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.inject_flux(cx, cx, cx, {0, 0, 5.0});
        int ticks = 30;
        rb.run(ticks);

        double threshold = 0.001;
        int furthest = 0;
        for (int dx = 1; dx < L/4; ++dx) {
            double rho = rb.voxels()[rb.lattice().index(cx + dx, cx, cx)].density();
            if (rho > threshold) {
                furthest = dx;
            }
        }

        double measured_speed = (double)furthest / ticks;
        std::cout << "    Furthest signal at t=" << ticks << ": x = " << furthest << "\n";
        std::cout << "    Measured speed = " << measured_speed << " voxels/tick\n";
        std::cout << "    Expected C_WAVE = " << ftd::C_WAVE << "\n";

        ftd::test::check("Wavefront propagates (furthest > 0)", furthest > 0);
        ftd::test::check("Wave speed in correct ballpark",
              measured_speed > ftd::C_WAVE * 0.3 && measured_speed < ftd::C_WAVE * 2.0);
    }

    // Section 2: Wave speed is less than C_SPEED
    std::cout << "\n--- Section 2: Wave Speed < Speed of Causality ---\n";
    {
        ftd::test::check("C_WAVE == C_SPEED (unified causal speed)",
              std::abs(ftd::C_WAVE - ftd::C_SPEED) < 1e-15);

        double cfl_limit = std::sqrt(1.0 / 3.0);
        std::cout << "    CFL limit = 1/sqrt(3) = " << cfl_limit << "\n";
        std::cout << "    C_WAVE                = " << ftd::C_WAVE << "\n";
        ftd::test::check_close("C_WAVE = 1/sqrt(3) (derived from CFL)",
                    ftd::C_WAVE, cfl_limit, 1e-12);
        ftd::test::check("C_WAVE at CFL limit (maximal stable speed)",
              std::abs(ftd::C_WAVE - cfl_limit) < 1e-10);
    }

    // Section 3: Dispersion — discrete lattice modifies omega(k)
    std::cout << "\n--- Section 3: Dispersion Relation ---\n";
    {
        double c = ftd::C_WAVE;

        double k_low = 0.1;
        double omega_low = 2.0 * c * std::sin(k_low / 2.0);
        double speed_low = omega_low / k_low;
        std::cout << "    k=0.1: omega = " << omega_low << ", v_phase = " << speed_low << "\n";
        ftd::test::check_close("Low-k phase speed ≈ C_WAVE",
                    speed_low, c, 0.01);

        double k_high = ftd::PI;
        double omega_high = 2.0 * c * std::sin(k_high / 2.0);
        double speed_high = omega_high / k_high;
        std::cout << "    k=pi: omega = " << omega_high << ", v_phase = " << speed_high << "\n";
        ftd::test::check("Short wavelengths are slower (lattice dispersion)",
              speed_high < c);
    }

    // Section 4: Symmetry — wave propagates equally in all directions
    std::cout << "\n--- Section 4: Propagation Isotropy ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.inject_flux(cx, cx, cx, {0, 0, 5.0});
        rb.run(15);

        double rho_x = rb.voxels()[rb.lattice().index(cx+5, cx, cx)].density();
        double rho_y = rb.voxels()[rb.lattice().index(cx, cx+5, cx)].density();
        double rho_z = rb.voxels()[rb.lattice().index(cx, cx, cx+5)].density();

        std::cout << "    rho(+x,5) = " << rho_x << "\n";
        std::cout << "    rho(+y,5) = " << rho_y << "\n";
        std::cout << "    rho(+z,5) = " << rho_z << "\n";

        double avg = (rho_x + rho_y + rho_z) / 3.0;
        if (avg > 1e-10) {
            double max_dev = std::max({std::abs(rho_x - avg),
                                       std::abs(rho_y - avg),
                                       std::abs(rho_z - avg)});
            double rel_dev = max_dev / avg;
            std::cout << "    Relative deviation from isotropy: " << rel_dev << "\n";
            ftd::test::check("Propagation approximately isotropic (< 80% deviation)",
                  rel_dev < 0.8);
        } else {
            std::cout << "    Signal too small at distance 5\n";
            ftd::test::check("Signal reaches distance 5", avg > 1e-10);
        }
    }
}

// ============================================================================
// Section: interference  (from test_interference.cpp)
// ============================================================================

static void section_interference() {
    // Section 1: Two-source constructive interference
    std::cout << "\n--- Section 1: Two-Source Interference ---\n";
    {
        int L = 48;
        int cy = L / 2;
        int cz = L / 2;
        int sep = 10;
        int x1 = L/2 - sep/2;
        int x2 = L/2 + sep/2;
        int ticks = 20;

        double amp = 3.0;
        ftd::RenderBridge rb(L);
        rb.inject_flux(x1, cy, cz, {0, 0, amp});
        rb.inject_flux(x2, cy, cz, {0, 0, amp});
        rb.run(ticks);

        ftd::RenderBridge rb_single(L);
        rb_single.inject_flux(x1, cy, cz, {0, 0, amp});
        rb_single.run(ticks);

        int mx = L / 2;
        double rho_mid = rb.voxels()[rb.lattice().index(mx, cy, cz)].density();
        double rho_single_mid = rb_single.voxels()[rb_single.lattice().index(mx, cy, cz)].density();

        std::cout << "    Two-source density at midpoint: " << rho_mid << "\n";
        std::cout << "    Single-source density at same distance: " << rho_single_mid << "\n";

        ftd::test::check("Constructive interference: two sources > single source",
              rho_mid > rho_single_mid);
    }

    // Section 2: Superposition linearity
    std::cout << "\n--- Section 2: Superposition Linearity ---\n";
    {
        int L = 32;
        int ticks = 10;

        ftd::RenderBridge rb_both(L);
        rb_both.toggles.gauss_projection = false;
        rb_both.toggles.genesis = false;
        rb_both.inject_flux(10, 16, 16, {0, 0, 2.0});
        rb_both.inject_flux(22, 16, 16, {0, 0, 2.0});
        rb_both.run(ticks);

        ftd::RenderBridge rb_a(L);
        rb_a.toggles.gauss_projection = false;
        rb_a.toggles.genesis = false;
        rb_a.inject_flux(10, 16, 16, {0, 0, 2.0});
        rb_a.run(ticks);

        ftd::RenderBridge rb_b(L);
        rb_b.toggles.gauss_projection = false;
        rb_b.toggles.genesis = false;
        rb_b.inject_flux(22, 16, 16, {0, 0, 2.0});
        rb_b.run(ticks);

        double max_err = 0.0;
        int test_points[] = {12, 14, 16, 18, 20};
        for (int tx : test_points) {
            int idx = rb_both.lattice().index(tx, 16, 16);
            ftd::Vec3 j_both = rb_both.voxels()[idx].flux;
            ftd::Vec3 j_a = rb_a.voxels()[idx].flux;
            ftd::Vec3 j_b = rb_b.voxels()[idx].flux;

            ftd::Vec3 j_sum = j_a + j_b;
            double err = (j_both - j_sum).mag();
            double scale = std::max(j_both.mag(), 1e-10);
            double rel_err = err / scale;
            if (rel_err > max_err) max_err = rel_err;
        }

        std::cout << "    Max relative error |J_both - (J_a + J_b)| / |J_both|: "
                  << max_err << "\n";

        ftd::test::check("Superposition holds: relative error < 1%", max_err < 0.01);
    }

    // Section 3: Destructive interference
    std::cout << "\n--- Section 3: Destructive Interference ---\n";
    {
        int L = 48;
        ftd::RenderBridge rb(L);
        int cy = L / 2;
        int cz = L / 2;

        double amp = 3.0;
        rb.inject_flux(L/2 - 5, cy, cz, {0, 0, amp});
        rb.inject_flux(L/2 + 5, cy, cz, {0, 0, -amp});

        ftd::RenderBridge rb_con(L);
        rb_con.inject_flux(L/2 - 5, cy, cz, {0, 0, amp});
        rb_con.inject_flux(L/2 + 5, cy, cz, {0, 0, amp});

        rb.run(15);
        rb_con.run(15);

        int mx = L / 2;
        double rho_destructive = rb.voxels()[rb.lattice().index(mx, cy, cz)].density();
        double rho_constructive = rb_con.voxels()[rb_con.lattice().index(mx, cy, cz)].density();

        std::cout << "    Destructive midpoint density: " << rho_destructive << "\n";
        std::cout << "    Constructive midpoint density: " << rho_constructive << "\n";

        ftd::test::check("Destructive < constructive at midpoint",
              rho_destructive < rho_constructive);
    }
}

// ============================================================================
// Section: wave_isotropy  (from campaign_wave_isotropy.cpp)
// ============================================================================

static double flux_mag_iso(const ftd::RenderBridge& rb, int x, int y, int z) {
    auto& v = rb.voxels()[rb.lattice().index(x, y, z)];
    return std::sqrt(v.flux.x * v.flux.x + v.flux.y * v.flux.y + v.flux.z * v.flux.z);
}

static void section_wave_isotropy() {
    std::cout << std::fixed << std::setprecision(6);

    const int L = 32;
    const int mid = L / 2;
    const int TICKS = 30;

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;

    double amp = ftd::K_B * 0.5;
    rb.inject_flux(mid, mid, mid, {amp, amp, amp});

    double baseline = flux_mag_iso(rb, mid + 10, mid, mid);
    rb.run(TICKS);

    int r = 8;
    double f_100 = flux_mag_iso(rb, mid + r, mid, mid);
    int d110 = static_cast<int>(r / std::sqrt(2.0));
    double f_110 = flux_mag_iso(rb, mid + d110, mid + d110, mid);
    int d111 = static_cast<int>(r / std::sqrt(3.0));
    double f_111 = flux_mag_iso(rb, mid + d111, mid + d111, mid + d111);

    std::cout << "\n--- Wave Amplitudes at r=" << r << " ---\n";
    std::cout << "  (100): |J| = " << f_100 << "\n";
    std::cout << "  (110): |J| = " << f_110 << "\n";
    std::cout << "  (111): |J| = " << f_111 << "\n";
    std::cout << "  Baseline (pre-evolution): " << baseline << "\n";

    double threshold = 1e-10;
    ftd::test::check("WI1: Wave arrives along (100)", f_100 > threshold);
    ftd::test::check("WI2: Wave arrives along (110)", f_110 > threshold);
    ftd::test::check("WI3: Wave arrives along (111)", f_111 > threshold);

    double max_f = std::max({f_100, f_110, f_111});
    double min_f = std::min({f_100, f_110, f_111});
    double isotropy_ratio = (min_f > 1e-30) ? max_f / min_f : 999.0;
    std::cout << "\n  Isotropy ratio = " << isotropy_ratio
              << " (max/min, ideal=1.0)\n";
    ftd::test::check("WI4: Isotropy ratio < 5.0 (cubic lattice, r=8, L=32)",
          isotropy_ratio < 5.0);

    double f_near = flux_mag_iso(rb, mid + 4, mid, mid);
    double f_far = flux_mag_iso(rb, mid + 12, mid, mid);
    std::cout << "  |J|(r=4) = " << f_near << "\n";
    std::cout << "  |J|(r=12) = " << f_far << "\n";
    ftd::test::check("WI5: Flux decreases with distance (wave spreads)",
          f_near > f_far || (f_near < 1e-20 && f_far < 1e-20));

    std::cout << std::defaultfloat;  // reset precision for subsequent sections
}

// ============================================================================
// Section: two_slit  (from campaign_two_slit.cpp)
// ============================================================================

static void inject_pulse(ftd::RenderBridge& rb, int x, int y, int z, double amplitude) {
    rb.inject_flux(x, y, z, {0.0, 0.0, amplitude});
}

static std::vector<double> measure_line(ftd::RenderBridge& rb, int x, int z,
                                        int y_start, int y_end) {
    std::vector<double> intensities;
    for (int y = y_start; y <= y_end; ++y) {
        auto& v = rb.voxel_at(x, y, z);
        double rho = v.flux.x * v.flux.x + v.flux.y * v.flux.y + v.flux.z * v.flux.z;
        intensities.push_back(rho);
    }
    return intensities;
}

static int count_minima(const std::vector<double>& signal, double threshold) {
    int count = 0;
    for (size_t i = 1; i + 1 < signal.size(); ++i) {
        if (signal[i] < signal[i-1] && signal[i] < signal[i+1] &&
            signal[i] < threshold) {
            count++;
        }
    }
    return count;
}

static std::vector<int> find_minima(const std::vector<double>& signal, double threshold) {
    std::vector<int> positions;
    for (size_t i = 1; i + 1 < signal.size(); ++i) {
        if (signal[i] < signal[i-1] && signal[i] < signal[i+1] &&
            signal[i] < threshold) {
            positions.push_back(static_cast<int>(i));
        }
    }
    return positions;
}

static void section_two_slit() {
    int L = 48;
    int mid = L / 2;
    int d = 4;
    int screen_dist = 15;
    double amplitude = ftd::K_B * 0.3;

    std::cout << "\n--- Two-Slit Setup ---\n";
    std::cout << "  Lattice: " << L << "^3\n";
    std::cout << "  Slit separation: " << 2*d << " voxels\n";
    std::cout << "  Screen distance: " << screen_dist << " voxels\n";

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.forces = false;
    rb.toggles.movement = false;
    rb.toggles.gravity = false;

    inject_pulse(rb, mid - screen_dist, mid, mid - d, amplitude);
    inject_pulse(rb, mid - screen_dist, mid, mid + d, amplitude);

    int propagation_ticks = static_cast<int>(screen_dist / ftd::C_WAVE) + 10;
    rb.run(propagation_ticks);

    std::vector<double> intensity = measure_line(rb, mid, mid, 0, L - 1);

    double max_intensity = *std::max_element(intensity.begin(), intensity.end());
    int peak_pos = static_cast<int>(std::max_element(intensity.begin(), intensity.end())
                                     - intensity.begin());

    std::cout << "  Peak intensity: " << std::scientific << max_intensity << "\n";
    std::cout << "  Peak position:  " << peak_pos << " (center=" << mid << ")\n";

    double threshold = max_intensity * 0.3;
    auto minima = find_minima(intensity, threshold);
    int n_minima = static_cast<int>(minima.size());

    std::cout << "  Minima found:   " << n_minima << "\n";
    for (int pos : minima) {
        std::cout << "    y=" << pos << " I=" << std::scientific << intensity[pos] << "\n";
    }

    double avg_fringe_spacing = 0.0;
    if (minima.size() >= 2) {
        for (size_t i = 1; i < minima.size(); ++i) {
            avg_fringe_spacing += (minima[i] - minima[i-1]);
        }
        avg_fringe_spacing /= (minima.size() - 1);
    }
    std::cout << "  Avg fringe spacing: " << std::fixed << std::setprecision(1)
              << avg_fringe_spacing << " voxels\n";

    double predicted_spacing = M_PI * screen_dist / (2.0 * d);
    std::cout << "  Predicted spacing: ~" << predicted_spacing << " voxels (approximate)\n";

    double left_sum = 0, right_sum = 0;
    int half_width = std::min(peak_pos, L - 1 - peak_pos);
    for (int i = 1; i <= std::min(half_width, 10); ++i) {
        if (peak_pos - i >= 0) left_sum += intensity[peak_pos - i];
        if (peak_pos + i < L) right_sum += intensity[peak_pos + i];
    }
    double symmetry = (left_sum + right_sum > 1e-30) ?
        std::min(left_sum, right_sum) / std::max(left_sum, right_sum) : 0.0;
    std::cout << "  Symmetry ratio: " << std::setprecision(3) << symmetry << "\n";

    auto audit_two = rb.energy_audit();

    std::cout << "\n--- Single-Source Control ---\n";

    ftd::RenderBridge rb_single(L);
    rb_single.toggles.genesis = false;
    rb_single.toggles.forces = false;
    rb_single.toggles.movement = false;
    rb_single.toggles.gravity = false;

    inject_pulse(rb_single, mid - screen_dist, mid, mid, amplitude);
    rb_single.run(propagation_ticks);

    std::vector<double> single_intensity = measure_line(rb_single, mid, mid, 0, L - 1);
    double max_single = *std::max_element(single_intensity.begin(), single_intensity.end());
    int n_minima_single = count_minima(single_intensity, max_single * 0.3);
    std::cout << "  Single source minima: " << n_minima_single << "\n";

    std::cout << "\n--- Checks ---\n";

    ftd::test::check("TS1: Central maximum near center (within 5 voxels)",
          std::abs(peak_pos - mid) <= 5);

    ftd::test::check("TS2: At least 2 interference minima detected", n_minima >= 2);

    ftd::test::check("TS3: Fringe pattern symmetry > 0.5", symmetry > 0.5);

    double min_at_minimum = 1e30;
    for (int pos : minima) {
        if (intensity[pos] < min_at_minimum) min_at_minimum = intensity[pos];
    }
    double contrast = (max_intensity > 1e-30 && !minima.empty()) ?
        1.0 - min_at_minimum / max_intensity : 0.0;
    std::cout << "    Fringe contrast: " << std::setprecision(3) << contrast << "\n";
    ftd::test::check("TS4: Fringe contrast > 0.3 (min < 70% of max)",
          contrast > 0.3 || minima.empty());

    double max_single_I = *std::max_element(single_intensity.begin(), single_intensity.end());
    auto single_minima = find_minima(single_intensity, max_single_I * 0.3);
    double single_contrast = 0.0;
    if (!single_minima.empty() && max_single_I > 1e-30) {
        double min_single = 1e30;
        for (int pos : single_minima) {
            if (single_intensity[pos] < min_single) min_single = single_intensity[pos];
        }
        single_contrast = 1.0 - min_single / max_single_I;
    }
    std::cout << "    Single-source contrast: " << std::setprecision(3) << single_contrast << "\n";
    std::cout << "    Two-source contrast:    " << contrast << "\n";
    bool two_source_stronger = (contrast > single_contrast) ||
                                (n_minima > n_minima_single) ||
                                (max_intensity > 2.0 * max_single_I);
    ftd::test::check("TS5: Two-source pattern is more structured than single source",
          two_source_stronger);

    bool spacing_ok = (avg_fringe_spacing > 0 && predicted_spacing > 0) ?
        (avg_fringe_spacing / predicted_spacing > 0.33 &&
         avg_fringe_spacing / predicted_spacing < 3.0) : false;
    ftd::test::check("TS6: Fringe spacing within factor 3 of prediction", spacing_ok);

    auto audit_initial_approx = rb.energy_audit();
    ftd::test::check("TS7: Total flux energy is non-zero (waves propagated)",
          audit_two.field_energy > 1e-20);

    std::cout << std::defaultfloat;
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("campaign_wave_dynamics");

    ftd::test::section("wave_speed");
    section_wave_speed();

    ftd::test::section("interference");
    section_interference();

    ftd::test::section("wave_isotropy");
    section_wave_isotropy();

    ftd::test::section("two_slit");
    section_two_slit();

    return ftd::test::finalize();
}

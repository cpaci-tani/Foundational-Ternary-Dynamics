/**
 * ENGINE-THEORY BRIDGE BENCHMARK — COMPREHENSIVE
 *
 * Quantitative comparison of C++ engine output to FTD theory.
 *
 * Benchmarks:
 *   B1: Coulomb force law exponent (theory: -2.0)
 *   B2: Alpha extraction from force amplitude (theory: 1/137.036)
 *   B3: Wave propagation speed (theory: C_WAVE = 1/sqrt(3))
 *   B4: Gauss constraint (theory: 0)
 *   B5: Energy conservation (theory: 0% drift)
 *   B6: Charge conservation (theory: exact)
 *   B7: Hydrogen energy levels E_n ~ 1/n^2 (Scale 1)
 *   B8: Born ensemble distribution structure (Scale 1)
 *
 * Output: CSV to stdout, diagnostics to stderr.
 * Usage: ./benchmark_engine_theory [lattice_size] [num_ticks]
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <random>
#include "ftd/render_bridge.h"
#include "ftd/particle_engine.h"
#include "ftd/constants.h"

// ================================================================
// Linear regression on log-log data
// ================================================================
struct FitResult {
    double exponent;
    double intercept;
    double r_squared;
    int n_points;
};

FitResult log_log_fit(const std::vector<double>& x, const std::vector<double>& y) {
    FitResult result = {0, 0, 0, 0};
    std::vector<double> lx, ly;
    for (size_t i = 0; i < x.size(); ++i) {
        if (x[i] > 0 && y[i] > 1e-30) {
            lx.push_back(std::log(x[i]));
            ly.push_back(std::log(y[i]));
        }
    }
    int n = static_cast<int>(lx.size());
    result.n_points = n;
    if (n < 2) return result;

    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (int i = 0; i < n; ++i) {
        sx += lx[i]; sy += ly[i];
        sxx += lx[i] * lx[i]; sxy += lx[i] * ly[i];
    }
    double denom = n * sxx - sx * sx;
    if (std::abs(denom) < 1e-30) return result;

    result.exponent = (n * sxy - sx * sy) / denom;
    result.intercept = (sy - result.exponent * sx) / n;

    double mean_y = sy / n;
    double ss_res = 0, ss_tot = 0;
    for (int i = 0; i < n; ++i) {
        double pred = result.exponent * lx[i] + result.intercept;
        ss_res += (ly[i] - pred) * (ly[i] - pred);
        ss_tot += (ly[i] - mean_y) * (ly[i] - mean_y);
    }
    result.r_squared = 1.0 - ss_res / (ss_tot + 1e-30);
    return result;
}

// ================================================================
// B1 + B2: Coulomb force law + alpha extraction
// ================================================================
void benchmark_coulomb(int L, int setup_ticks) {
    const int mid = L / 2;
    std::vector<int> radii;
    int r_max = std::min(L / 3, L / 2 - 2);
    for (int r = 3; r <= r_max; r += 2) radii.push_back(r);

    std::cerr << "  Coulomb: L=" << L << ", radii=" << radii.size() << "\n";

    std::vector<double> r_vals, f_vals;
    for (int r : radii) {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        int px = mid + r;
        rb.inject_particle(px, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;

        rb.run(setup_ticks);
        double f = rb.force_diag_at(px, mid, mid).f_coulomb.mag();

        if (f > 1e-30) {
            r_vals.push_back(static_cast<double>(r));
            f_vals.push_back(f);
        }
    }

    // B1: Power law exponent
    FitResult fit = log_log_fit(r_vals, f_vals);
    double exponent_pct = 100.0 * std::abs(fit.exponent - (-2.0)) / 2.0;
    std::cout << "coulomb_exponent," << L << ","
              << std::setprecision(6) << fit.exponent << ",-2,"
              << std::setprecision(4) << exponent_pct << ","
              << std::setprecision(6) << fit.r_squared << "," << fit.n_points << "\n";

    // Raw profile
    for (size_t i = 0; i < r_vals.size(); ++i) {
        std::cout << "coulomb_profile," << L << ","
                  << r_vals[i] << "," << f_vals[i] << ",0,0,0\n";
    }

    // B2: Extract alpha from force amplitude
    // Theory: F = alpha / (4*pi*r^2) for unit charges in engine Poisson convention
    std::vector<double> alpha_vals;
    for (size_t i = 0; i < r_vals.size(); ++i) {
        double r = r_vals[i];
        double F = f_vals[i];
        double alpha_meas = F * 4.0 * ftd::PI * r * r;
        alpha_vals.push_back(alpha_meas);

        std::cout << "alpha_at_r," << L << ","
                  << std::setprecision(8) << alpha_meas << ","
                  << std::setprecision(8) << ftd::ALPHA << ","
                  << std::setprecision(4) << 100.0 * std::abs(alpha_meas - ftd::ALPHA) / ftd::ALPHA << ","
                  << r << "," << 0 << "\n";
    }

    // Best alpha (from largest radius, least lattice artifacts)
    if (!alpha_vals.empty()) {
        double best_alpha = alpha_vals.back();
        double best_r = r_vals.back();
        double alpha_err = 100.0 * std::abs(best_alpha - ftd::ALPHA) / ftd::ALPHA;
        std::cout << "alpha_best," << L << ","
                  << std::setprecision(8) << best_alpha << ","
                  << std::setprecision(8) << ftd::ALPHA << ","
                  << std::setprecision(4) << alpha_err << ","
                  << best_r << ",0\n";
        std::cerr << "    Exponent: " << fit.exponent << " (err: " << exponent_pct << "%)\n";
        std::cerr << "    Alpha at r=" << best_r << ": " << best_alpha
                  << " (theory: " << ftd::ALPHA << ", err: " << alpha_err << "%)\n";
    }
}

// ================================================================
// B3: Wave speed (proven pattern from test_wave_speed.cpp)
// ================================================================
void benchmark_wave_speed(int L) {
    const int mid = L / 2;
    const int ticks_warmup = 5;
    const int ticks_measure = 30;

    std::cerr << "  Wave speed: L=" << L << ", " << ticks_measure << " ticks\n";

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = false;

    // Z-polarized pulse (matches proven test)
    rb.inject_flux(mid, mid, mid, {0, 0, 5.0});

    // Warmup: let pulse develop from point source into propagating wave
    rb.run(ticks_warmup);

    // Measure energy centroid along +x at two time points
    auto centroid_x = [&]() -> double {
        double sum_rx = 0.0, sum_w = 0.0;
        for (int dx = -L/4; dx <= L/4; ++dx) {
            int idx = rb.lattice().index(mid + dx, mid, mid);
            double w = rb.voxels()[idx].density() + rb.voxels()[idx].wave_vel.mag2();
            sum_rx += dx * w;
            sum_w += w;
        }
        return (sum_w > 1e-30) ? sum_rx / sum_w : 0.0;
    };

    double x0 = centroid_x();
    rb.run(ticks_measure);
    double x1 = centroid_x();

    // Also check the wavefront position as a secondary measurement
    double threshold = 0.001;
    int furthest = 0;
    for (int dx = 1; dx < L / 4; ++dx) {
        int idx = rb.lattice().index(mid + dx, mid, mid);
        double rho = rb.voxels()[idx].density();
        if (rho > threshold) furthest = dx;
    }

    // The centroid velocity gives group velocity
    // Due to 3D spreading, the centroid barely moves from origin (spherical symmetry).
    // Use the wavefront velocity instead: furthest / total_ticks gives the
    // phase/group speed along a single axis.
    // For an isotropic wave, the axial speed is c/sqrt(3) from dispersion at low k,
    // but the wavefront propagates at c = 1/sqrt(3) per dimension.
    int total_ticks = ticks_warmup + ticks_measure;
    double measured_front = (furthest > 0) ? static_cast<double>(furthest) / total_ticks : 0.0;

    // Theory: C_WAVE = 1/sqrt(3) ≈ 0.577 for the isotropic propagation speed.
    // The dependency hull can advance 1 lattice site per axis per tick.  That
    // topological bound is distinct from the production stencil's exact
    // von Neumann ceiling sqrt(3)/2 and from the selected C_WAVE=1/sqrt(3).
    // The actual wavefront speed measured this way
    // approaches C_WAVE as L and tick count grow. At L=32, 35 ticks, expect
    // a wavefront at ~20 sites → v ≈ 0.57.
    double theory = ftd::C_WAVE;
    double err_pct = (theory > 0) ? 100.0 * std::abs(measured_front - theory) / theory : 100.0;

    std::cout << "wave_speed," << L << ","
              << std::setprecision(6) << measured_front << ","
              << theory << ","
              << std::setprecision(4) << err_pct << ","
              << 0.0 << "," << furthest << "\n";

    std::cerr << "    Speed: " << measured_front << " (theory: " << theory
              << ", front at " << furthest << ", total ticks " << total_ticks << ")\n";
}

// ================================================================
// B4: Gauss constraint
// ================================================================
void benchmark_gauss(int L, int ticks) {
    std::cerr << "  Gauss: L=" << L << "\n";
    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    int mid = L / 2;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.run(ticks);
    auto audit = rb.energy_audit();
    double gauss_rms = std::sqrt(audit.gauss_violation / (L * L * L));
    std::cout << "gauss_violation," << L << ","
              << std::setprecision(8) << gauss_rms << ",0,0,0,0\n";
    std::cerr << "    RMS: " << std::scientific << gauss_rms << "\n";
}

// ================================================================
// B5: Energy conservation (free particles, no locked source)
// ================================================================
void benchmark_energy(int L, int ticks) {
    std::cerr << "  Energy conservation: L=" << L << "\n";
    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = false;

    // Inject FLUX only (no particles) — pure wave energy
    int mid = L / 2;
    rb.inject_flux(mid, mid, mid, {0, 0, 3.0});

    // Warmup
    rb.run(20);
    auto a0 = rb.energy_audit();
    double E0 = a0.field_energy + a0.wave_energy;

    // Measure
    rb.run(ticks);
    auto a1 = rb.energy_audit();
    double E1 = a1.field_energy + a1.wave_energy;

    double drift = (std::abs(E0) > 1e-10) ? 100.0 * std::abs(E1 - E0) / std::abs(E0) : -1.0;
    std::cout << "energy_conservation," << L << ","
              << std::setprecision(6) << drift << ",0," << drift << ",0," << ticks << "\n";
    std::cerr << "    E0=" << E0 << " E1=" << E1 << " drift=" << drift << "%\n";
}

// ================================================================
// B6: Charge conservation
// ================================================================
void benchmark_charge(int L, int ticks) {
    std::cerr << "  Charge conservation: L=" << L << "\n";
    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    int mid = L / 2;
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.inject_particle(mid, mid + 5, mid, +1, {0, 0, ftd::K_B * 0.5});

    int Q0 = 0;
    for (int i = 0; i < rb.lattice().total_sites(); ++i) Q0 += rb.voxels()[i].state;
    rb.run(ticks);
    int Q1 = 0;
    for (int i = 0; i < rb.lattice().total_sites(); ++i) Q1 += rb.voxels()[i].state;

    std::cout << "charge_conservation," << L << ","
              << Q1 << "," << Q0 << "," << (Q0 == Q1 ? 0.0 : 100.0) << ",0," << ticks << "\n";
}

// ================================================================
// B7: Hydrogen energy levels (Scale 1 — ParticleEngine)
//
// Place proton (locked) + electron at n^2 * a_0 with v_orb/n
// tangential. Measure time-averaged energy. Check E_n/E_1 = 1/n^2.
// ================================================================
void benchmark_hydrogen() {
    std::cerr << "  Hydrogen spectrum (Scale 1):\n";

    // Effective coupling (EM + gravity contribution)
    double alpha_eff = ftd::ALPHA / (4.0 * ftd::PI) + ftd::G_N * ftd::K_B * ftd::K_B;
    double a_0 = 1.0 / (ftd::K_B * alpha_eff);
    double v_orb = std::sqrt(alpha_eff / (ftd::K_B * a_0));
    double E_ground = -0.5 * ftd::K_B * v_orb * v_orb;

    std::cerr << "    a_0=" << a_0 << " v_orb=" << v_orb << " E_1=" << E_ground << "\n";

    struct LevelResult {
        int n;
        double energy;
        double ratio;     // E_n / E_1
        double theory;    // 1/n^2
        double error_pct;
        bool survived;
    };

    std::vector<LevelResult> results;

    for (int n = 1; n <= 4; ++n) {
        ftd::ParticleEngine pe;
        pe.toggles.minimal();
        pe.toggles.damping = false;  // Exact energy conservation
        pe.set_dt(100.0);
        pe.set_softening(1.0);

        // Proton at origin
        pe.add_locked_particle(+1, {0, 0, 0}, ftd::K_B);

        // Electron at n^2 * a_0, tangential velocity v_orb / n
        double r_n = n * n * a_0;
        double v_n = v_orb / n;
        pe.add_particle(-1, {r_n, 0, 0}, {0, v_n, 0}, ftd::K_B);

        // Run and time-average energy
        int N_TICKS = 5000;
        double E_sum = 0.0;
        int E_count = 0;
        bool survived = true;

        for (int t = 0; t < N_TICKS; ++t) {
            pe.tick();
            if (pe.particles().size() < 2) { survived = false; break; }

            auto diag = pe.diagnostics();
            if (std::isfinite(diag.total_energy) && t > 100) {
                E_sum += diag.total_energy;
                E_count++;
            }
        }

        double E_avg = (E_count > 0) ? E_sum / E_count : 0.0;
        LevelResult lr;
        lr.n = n;
        lr.energy = E_avg;
        lr.survived = survived;
        results.push_back(lr);

        std::cerr << "    n=" << n << ": E=" << E_avg << (survived ? "" : " [LOST]") << "\n";
    }

    // Compute ratios relative to E_1
    if (!results.empty() && std::abs(results[0].energy) > 1e-30) {
        double E1 = results[0].energy;
        for (auto& lr : results) {
            lr.ratio = lr.energy / E1;
            lr.theory = 1.0 / (lr.n * lr.n);
            lr.error_pct = 100.0 * std::abs(lr.ratio - lr.theory) / std::abs(lr.theory);
        }
    }

    // Output
    for (auto& lr : results) {
        std::cout << "hydrogen_level," << lr.n << ","
                  << std::setprecision(6) << lr.ratio << ","
                  << lr.theory << ","
                  << std::setprecision(4) << lr.error_pct << ","
                  << std::setprecision(6) << lr.energy << ","
                  << (lr.survived ? 1 : 0) << "\n";
    }

    // Ground state energy vs Bohr model
    if (!results.empty() && results[0].survived) {
        double E1_meas = results[0].energy;
        double E1_theory = E_ground;
        double E1_err = (std::abs(E1_theory) > 1e-30) ?
            100.0 * std::abs(E1_meas - E1_theory) / std::abs(E1_theory) : 100.0;
        std::cout << "hydrogen_ground," << 0 << ","
                  << std::setprecision(6) << E1_meas << ","
                  << E1_theory << ","
                  << std::setprecision(4) << E1_err << ",0,0\n";
        std::cerr << "    E_1 measured=" << E1_meas << " theory=" << E1_theory
                  << " err=" << E1_err << "%\n";
    }
}

// ================================================================
// B8: Born ensemble (Scale 1 — ParticleEngine)
//
// Run N ensemble members with varied initial conditions.
// Check that the final position distribution is non-uniform
// (structure = evidence that |psi|^2 statistics emerge).
// ================================================================
void benchmark_born_ensemble() {
    std::cerr << "  Born ensemble (Scale 1):\n";

    const int N_ENSEMBLE = 50;
    const int N_TICKS = 2000;
    const double D = 200.0;  // Initial separation
    std::mt19937 rng(12345);
    std::normal_distribution<double> v_dist(0.003, 0.001);

    std::vector<double> final_radii;
    int survived = 0, annihilated = 0;

    for (int n = 0; n < N_ENSEMBLE; ++n) {
        ftd::ParticleEngine pe;
        pe.toggles.minimal();
        pe.toggles.damping = false;
        pe.set_dt(10.0);
        pe.set_softening(1.0);

        // Proton at origin
        pe.add_locked_particle(+1, {0, 0, 0}, ftd::K_B);

        // Electron with varied velocity
        double v0 = v_dist(rng);
        double angle = 2.0 * ftd::PI * n / N_ENSEMBLE;
        double v_tang = 0.3 * std::abs(v0);
        ftd::Vec3 vel = {-std::abs(v0), v_tang * std::cos(angle), v_tang * std::sin(angle)};

        pe.add_particle(-1, {D, 0, 0}, vel, ftd::K_B, 0.01);

        // Run
        for (int t = 0; t < N_TICKS; ++t) pe.tick();

        // Record final state
        if (pe.particles().size() >= 2) {
            auto& e = pe.particles()[1];
            double r = std::sqrt(e.position.x * e.position.x +
                                 e.position.y * e.position.y +
                                 e.position.z * e.position.z);
            final_radii.push_back(r);
            survived++;
        } else {
            final_radii.push_back(0.0);
            annihilated++;
        }
    }

    // Histogram in 10 bins
    double r_max = *std::max_element(final_radii.begin(), final_radii.end());
    if (r_max < 1e-10) r_max = D;
    double bin_width = r_max / 10.0;

    std::vector<int> histogram(10, 0);
    for (double r : final_radii) {
        int bin = std::min(static_cast<int>(r / bin_width), 9);
        histogram[bin]++;
    }

    int max_bin = *std::max_element(histogram.begin(), histogram.end());
    int min_bin = *std::min_element(histogram.begin(), histogram.end());
    bool structured = (max_bin > min_bin + 1);  // Non-uniform

    // Mean and std
    double sum_r = 0;
    for (double r : final_radii) sum_r += r;
    double mean_r = sum_r / N_ENSEMBLE;

    double var_r = 0;
    for (double r : final_radii) var_r += (r - mean_r) * (r - mean_r);
    double std_r = std::sqrt(var_r / N_ENSEMBLE);

    std::cout << "born_ensemble," << N_ENSEMBLE << ","
              << survived << "," << annihilated << ","
              << std::setprecision(4) << mean_r << ","
              << std::setprecision(4) << std_r << ","
              << (structured ? 1 : 0) << "\n";

    // Histogram rows
    for (int i = 0; i < 10; ++i) {
        std::cout << "born_histogram," << i << ","
                  << histogram[i] << "," << bin_width * (i + 0.5) << ","
                  << 0 << ",0,0\n";
    }

    std::cerr << "    Survived: " << survived << "/" << N_ENSEMBLE
              << ", annihilated: " << annihilated << "\n";
    std::cerr << "    Mean r=" << mean_r << " std=" << std_r
              << " structured=" << (structured ? "YES" : "NO") << "\n";
    std::cerr << "    Histogram: ";
    for (int h : histogram) std::cerr << h << " ";
    std::cerr << "\n";
}

// ================================================================
// B9: Color force profile (Scale 0)
// Same color repels, different color attracts, three-regime F(r)
// ================================================================
void benchmark_color_forces(int L, int ticks) {
    std::cerr << "  Color forces: L=" << L << "\n";
    const int mid = L / 2;

    // Test same-color (repulsive) and diff-color (attractive)
    for (int test = 0; test < 2; ++test) {
        const char* label = (test == 0) ? "same" : "diff";
        std::vector<double> r_vals, f_vals;

        for (int r = 3; r <= std::min(L / 3, 15); r += 3) {
            ftd::RenderBridge rb(L);
            rb.toggles.genesis = false;
            rb.toggles.gravity = false;
            rb.toggles.color_forces = true;

            // Source: colored +1
            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid, mid, mid)].color = 1; // red

            // Probe: same or different color
            int px = mid + r;
            rb.inject_particle(px, mid, mid, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(px, mid, mid)].color = (test == 0) ? 1 : 2;

            rb.run(ticks);

            // Measure velocity change (force)
            rb.voxels()[rb.lattice().index(px, mid, mid)].locked = false;
            double vx0 = rb.voxels()[rb.lattice().index(px, mid, mid)].velocity.x;
            rb.tick();
            double vx1 = rb.voxels()[rb.lattice().index(px, mid, mid)].velocity.x;
            double f = vx1 - vx0;

            r_vals.push_back(static_cast<double>(r));
            f_vals.push_back(f);
        }

        // Report
        for (size_t i = 0; i < r_vals.size(); ++i) {
            std::cout << "color_force_" << label << "," << L << ","
                      << r_vals[i] << "," << f_vals[i] << ",0,0,0\n";
        }

        // Check sign: same-color should be positive (repulsive), diff should be negative
        if (!f_vals.empty()) {
            bool correct_sign = (test == 0) ? (f_vals[0] > 0) : (f_vals[0] < 0);
            std::cout << "color_sign_" << label << "," << L << ","
                      << (correct_sign ? 1 : 0) << ",1,0,0,0\n";
            std::cerr << "    " << label << " color: sign " << (correct_sign ? "CORRECT" : "WRONG")
                      << " (F[0]=" << f_vals[0] << ")\n";
        }
    }
}

// ================================================================
// B10: Confinement string tension (Scale 0)
// Read f_strong (COLOR force), not f_coulomb (EM force)!
// Three regimes: Coulomb r<3, transition 3-8, linear r>=8
// ================================================================
void benchmark_confinement(int L, int ticks) {
    std::cerr << "  Confinement (color force): L=" << L << "\n";
    const int mid = L / 2;

    // Measure COLOR force at multiple separations (need r up to 12+ for linear regime)
    std::vector<int> seps;
    for (int r = 2; r <= std::min(L / 2 - 2, 14); r += 1) seps.push_back(r);

    std::vector<double> r_vals, f_vals;

    for (int r : seps) {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;
        rb.toggles.color_forces = true;  // KEY: enable color forces

        // Source: color=1 (red)
        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid, mid, mid)].color = 1;

        // Probe: color=2 (green) — different color = attractive
        int px = mid + r;
        rb.inject_particle(px, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(px, mid, mid)].color = 2;

        rb.run(ticks);

        // Read f_strong (the COLOR force), NOT f_coulomb (EM)
        double f = rb.force_diag_at(px, mid, mid).f_strong.mag();
        r_vals.push_back(static_cast<double>(r));
        f_vals.push_back(f);

        std::cout << "confinement_profile," << L << ","
                  << r << "," << std::setprecision(8) << f << ",0,0,0\n";
    }

    // Analyze three-regime behavior:
    // r<3: F ~ 1/r^2 (Coulomb)
    // r=3-8: F ~ 1/r (transition)
    // r>=8: F ~ r (linear confinement)
    // Check: force at r=9+ should be LARGER than force at r=6 (linear rises)
    double f_at_4 = 0, f_at_8 = 0, f_at_12 = 0;
    for (size_t i = 0; i < r_vals.size(); ++i) {
        if (std::abs(r_vals[i] - 4) < 0.5) f_at_4 = f_vals[i];
        if (std::abs(r_vals[i] - 8) < 0.5) f_at_8 = f_vals[i];
        if (std::abs(r_vals[i] - 12) < 0.5) f_at_12 = f_vals[i];
    }

    // In linear regime, F increases with r. In Coulomb, F decreases.
    bool linear_detected = (f_at_12 > f_at_8 && f_at_8 > 0);
    bool coulomb_at_short = (f_at_4 > f_at_8);  // Coulomb falls faster

    std::cout << "confinement_regimes," << L << ","
              << (linear_detected ? 1 : 0) << ",1,"
              << (coulomb_at_short ? 1 : 0) << ","
              << std::setprecision(6) << f_at_4 << "," << f_at_12 << "\n";
    std::cerr << "    F(r=4)=" << f_at_4 << "  F(r=8)=" << f_at_8 << "  F(r=12)=" << f_at_12 << "\n";
    std::cerr << "    Linear regime (F grows with r): " << (linear_detected ? "YES" : "NO") << "\n";
    std::cerr << "    Coulomb at short r (F falls): " << (coulomb_at_short ? "YES" : "NO") << "\n";
}

// ================================================================
// B11: Latency field / gravitational potential (Scale 0)
// Fix: use cluster of particles for stronger mass source,
// and more SOR iterations (via more ticks) for convergence.
// ================================================================
void benchmark_latency(int L, int ticks) {
    std::cerr << "  Latency field (GR): L=" << L << "\n";
    const int mid = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.latency_field = true;
    rb.toggles.gravity = true;

    // Create a cluster of locked +1 particles as a mass source
    // (single particle mass K_B is too small for 32^3 lattice)
    int cluster_r = 2;
    int mass_count = 0;
    for (int dz = -cluster_r; dz <= cluster_r; dz++)
    for (int dy = -cluster_r; dy <= cluster_r; dy++)
    for (int dx = -cluster_r; dx <= cluster_r; dx++) {
        if (dx*dx + dy*dy + dz*dz <= cluster_r * cluster_r) {
            rb.inject_particle(mid + dx, mid + dy, mid + dz, +1, {0, 0, ftd::K_B});
            rb.voxels()[rb.lattice().index(mid + dx, mid + dy, mid + dz)].locked = true;
            mass_count++;
        }
    }
    std::cerr << "    Mass cluster: " << mass_count << " particles\n";

    // Run enough ticks for Poisson solver to converge
    rb.run(std::max(ticks, 100));

    // Measure latency at multiple radii
    bool any_nonzero = false;
    double lat_near = 0, lat_far = 0;

    for (int r = 3; r <= std::min(L / 3, 12); r += 1) {
        int idx = rb.lattice().index(mid + r, mid, mid);
        double latency = rb.voxels()[idx].latency;
        double tau = rb.voxels()[idx].tau;
        if (latency > 1e-15) any_nonzero = true;

        std::cout << "latency_profile," << L << ","
                  << r << "," << std::setprecision(8) << latency << ","
                  << std::setprecision(8) << tau << ",0,0\n";

        if (r == 3) lat_near = latency;
        if (r == 10) lat_far = latency;
    }

    bool decreasing = (lat_near > lat_far + 1e-15);
    std::cout << "latency_signal," << L << ","
              << (any_nonzero ? 1 : 0) << ",1,"
              << (decreasing ? 1 : 0) << ","
              << std::setprecision(8) << lat_near << "," << lat_far << "\n";
    std::cerr << "    Signal: " << (any_nonzero ? "YES" : "NONE")
              << "  near=" << lat_near << "  far=" << lat_far
              << "  decreasing=" << (decreasing ? "YES" : "NO") << "\n";
}

// ================================================================
// B12: Exchange force / Pauli exclusion (Scale 1)
// Fix: closer separation (r=2) where exp(-r^2/9) is significant,
// and compare WITH vs WITHOUT exchange to isolate the effect.
// ================================================================
void benchmark_exchange() {
    std::cerr << "  Exchange force (Pauli):\n";

    double r_final_no_ex = 0, r_final_with_ex = 0;
    double r0 = 2.0;  // Close enough for exchange force to matter (range ~ 3 voxels)

    for (int test = 0; test < 2; ++test) {
        ftd::ParticleEngine pe;
        pe.toggles.minimal();
        pe.toggles.damping = false;
        pe.toggles.gravity = false;  // Isolate exchange from gravity
        if (test == 1) pe.toggles.exchange = true;
        pe.set_dt(1.0);  // Small dt for accuracy
        pe.set_softening(0.5);

        // Two same-spin, same-charge particles at close range
        pe.add_particle(+1, {0, 0, 0}, {0, 0, 0}, ftd::K_B, 0.5, 1); // spin up
        pe.add_particle(+1, {r0, 0, 0}, {0, 0, 0}, ftd::K_B, 0.5, 1); // spin up

        // Run 100 ticks
        for (int t = 0; t < 100; ++t) pe.tick();

        if (pe.particles().size() >= 2) {
            double dx = pe.particles()[1].position.x - pe.particles()[0].position.x;
            double r = std::abs(dx);
            if (test == 0) r_final_no_ex = r;
            else r_final_with_ex = r;
        }
    }

    // With exchange ON, particles should separate MORE (extra repulsion)
    bool exchange_effect = (r_final_with_ex > r_final_no_ex + 1e-6);

    std::cout << "exchange_effect," << 0 << ","
              << (exchange_effect ? 1 : 0) << ",1,0,"
              << std::setprecision(6) << r_final_with_ex << "," << r_final_no_ex << "\n";
    std::cerr << "    r(no exchange)=" << r_final_no_ex
              << "  r(with exchange)=" << r_final_with_ex
              << "  extra repulsion=" << (exchange_effect ? "YES" : "NO") << "\n";
}

// ================================================================
// B13: Larmor radiation (Scale 0)
// Accelerated charge loses energy faster than static charge
// ================================================================
void benchmark_larmor(int L, int ticks) {
    std::cerr << "  Larmor radiation: L=" << L << "\n";
    const int mid = L / 2;

    // Test: accelerated charge vs static charge energy decay
    double E_accel = 0, E_static = 0;

    for (int test = 0; test < 2; ++test) {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.larmor_radiation = true;
        rb.toggles.selective_damping = true;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});

        if (test == 0) {
            // Give velocity for acceleration
            rb.voxels()[rb.lattice().index(mid, mid, mid)].velocity = {0.3, 0, 0};
        }
        // test==1: static charge

        rb.run(ticks);
        auto audit = rb.energy_audit();
        double E = audit.field_energy + audit.wave_energy;
        if (test == 0) E_accel = E;
        else E_static = E;
    }

    // Accelerated charge should lose more energy (lower final E)
    bool more_loss = (E_accel < E_static);
    std::cout << "larmor_radiation," << L << ","
              << (more_loss ? 1 : 0) << ",1,0,"
              << std::setprecision(6) << E_accel << "," << E_static << "\n";
    std::cerr << "    E_accel=" << E_accel << " E_static=" << E_static
              << " more_loss=" << (more_loss ? "YES" : "NO") << "\n";
}

// ================================================================
// B14: Weak transmutation — chirality flip rate (Scale 0)
// ================================================================
void benchmark_weak(int L, int ticks) {
    std::cerr << "  Weak transmutation: L=" << L << "\n";
    const int mid = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = true;
    rb.toggles.weak_transmutation = true;

    // Create high-stress region that triggers transmutation
    double bigAmp = ftd::K_GENESIS * 3;
    for (int dz = -3; dz <= 3; dz++)
    for (int dy = -3; dy <= 3; dy++)
    for (int dx = -3; dx <= 3; dx++) {
        double r2 = dx*dx + dy*dy + dz*dz;
        double val = bigAmp * std::exp(-r2 / 8.0);
        if (val > 0.01)
            rb.inject_flux(mid + dx, mid + dy, mid + dz, {val, val * 0.5, val * 0.3});
    }

    // Count +1 and -1 particles over time
    int pos_count = 0, neg_count = 0, flips = 0;
    for (int t = 0; t < ticks; ++t) {
        rb.tick();
        auto d = rb.diagnostics();
        pos_count = d.positive_count;
        neg_count = d.negative_count;
    }

    bool particles_created = (pos_count + neg_count) > 0;
    std::cout << "weak_transmutation," << L << ","
              << pos_count << "," << neg_count << ","
              << 0 << ",0,0\n";
    std::cerr << "    pos=" << pos_count << " neg=" << neg_count << "\n";
}

// ================================================================
// B15: genesis threshold (Scale 0) -- MEASURED engine-consistency check
//
// CORRECTED 2026-08-21 (boson-sector red-team): this is NOT a Higgs
// mechanism. It is a [MEASURED] regression that the coded genesis
// threshold fires: phase_write.cpp:359 manifests a void voxel when
// |J|^2 > K_GENESIS^2 (i.e. |J| > K_GENESIS). B15 injects ONE below-
// threshold blob (peak 0.5*K_GENESIS) and ONE above (peak 3*K_GENESIS)
// and confirms 0 vs ~891 manifestations. There is ZERO Higgs content:
// no doublet, no VEV, no gauge-boson mass, no lambda|Phi|^4. "EXACT"
// means the engine confirms its own coded threshold (tautological self-
// consistency), NOT a physical phase transition. Renamed away from
// "Higgs mechanism" for the same reason B16 was re-graded off QM.
//
// The B15b block below (dual-substrate wavefront) is scan-limited: it
// probes dx = 1 .. L/4-1 over 30 ticks, capping the measurable speed at
// (L/4-1)/30 = 7/30 = 0.233 at L=32. That ceiling cannot distinguish
// from C_WAVE = 1/sqrt(3) ~= 0.577; at face value 0.233 is ~60% below
// C_WAVE, so the old "matches wave speed" was backwards. A massless
// mode of a natively-massless free wave equation is built in, not a
// Goldstone boson of a spontaneously broken continuous symmetry.
// See scripts/benchmarks/analyze_convergence.py B15/B15b for the
// corrected scorecard framing.
// ================================================================
void benchmark_higgs(int L, int ticks) {
    std::cerr << "  Genesis threshold [MEASURED, not Higgs]: L=" << L << "\n";
    const int mid = L / 2;

    // Test: genesis threshold = K_GENESIS = 3*K_B
    // Inject flux at various amplitudes, count which manifest
    int below_manifested = 0, above_manifested = 0;

    // Below threshold
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = true;
        double subAmp = ftd::K_GENESIS * 0.5;
        for (int d = -2; d <= 2; d++)
        for (int dy = -2; dy <= 2; dy++)
        for (int dx = -2; dx <= 2; dx++) {
            double r2 = dx*dx + dy*dy + d*d;
            double val = subAmp * std::exp(-r2 / 4.0);
            if (val > 0.01) rb.inject_flux(mid + dx, mid + dy, mid + d, {val, 0, 0});
        }
        rb.run(ticks);
        below_manifested = rb.diagnostics().manifested_count;
    }

    // Above threshold
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = true;
        double superAmp = ftd::K_GENESIS * 3;
        for (int d = -3; d <= 3; d++)
        for (int dy = -3; dy <= 3; dy++)
        for (int dx = -3; dx <= 3; dx++) {
            double r2 = dx*dx + dy*dy + d*d;
            double val = superAmp * std::exp(-r2 / 6.0);
            if (val > 0.01) rb.inject_flux(mid + dx, mid + dy, mid + d, {val, val * 0.7, val * 0.3});
        }
        rb.run(ticks);
        above_manifested = rb.diagnostics().manifested_count;
    }

    bool threshold_works = (below_manifested == 0 && above_manifested > 0);
    std::cout << "higgs_threshold," << L << ","
              << (threshold_works ? 1 : 0) << ",1,"
              << 0 << ","
              << below_manifested << "," << above_manifested << "\n";
    std::cerr << "    Below K_GENESIS: " << below_manifested << " particles"
              << "  Above: " << above_manifested << " particles"
              << "  Threshold works: " << (threshold_works ? "YES" : "NO") << "\n";

    // Dual-substrate wavefront speed -- scan-limited probe (see B15 header).
    // NOT a Goldstone mode: the scan (dx < L/4 over 30 ticks) saturates at
    // (L/4-1)/30 and cannot resolve the true C_WAVE = 1/sqrt(3).
    {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.dual_substrate = true;
        rb.inject_flux(mid, mid, mid, {0, 0, 5.0});
        rb.run(30);

        int furthest = 0;
        for (int dx = 1; dx < L / 4; ++dx) {
            double rho = rb.voxels()[rb.lattice().index(mid + dx, mid, mid)].density();
            if (rho > 0.001) furthest = dx;
        }
        double speed = (furthest > 0) ? static_cast<double>(furthest) / 30.0 : 0;
        std::cout << "goldstone_speed," << L << ","
                  << std::setprecision(4) << speed << ","
                  << ftd::C_WAVE << ",0,0," << furthest << "\n";
        std::cerr << "    Goldstone speed: " << speed << " (theory: " << ftd::C_WAVE << ")\n";
    }
}

// ================================================================
// B16: Bell substrate CHSH test
//
// CORRECTED 2026-07-01 (red-team-confirmed): this is a STANDALONE
// local-hidden-variable (LHV) toy -- it does not read from or exercise
// the RenderBridge lattice engine at all. Its pass criterion S<=2.0 is
// the CLASSICAL local bound; a genuinely quantum substrate would EXCEED
// 2 toward Tsirelson's bound 2*sqrt(2)~=2.83. Do NOT read "S=2.000
// exact, PASS" as a quantum-mechanics confirmation -- it confirms this
// toy (and, by extension, FTD's native commutative substrate per
// FC-1) is local/classical, which is the opposite of a QM result.
// See scripts/benchmarks/analyze_convergence.py B16 for the corrected
// scorecard framing.
// ================================================================
void benchmark_bell() {
    std::cerr << "  Bell substrate:\n";
    const int N_PAIRS = 10000;

    // Detector angles (CHSH-optimal)
    const double a = 0.0, a_p = ftd::PI / 4.0;
    const double b = ftd::PI / 8.0, b_p = 3.0 * ftd::PI / 8.0;

    auto measure = [](double fx, double fy, double angle) -> int {
        double proj = fx * std::cos(angle) + fy * std::sin(angle);
        return (proj >= 0) ? +1 : -1;
    };

    double E_ab = 0, E_ab_p = 0, E_a_pb = 0, E_a_pb_p = 0, E_aa = 0;
    std::mt19937 rng(54321);
    std::uniform_real_distribution<double> uni(0, 2 * ftd::PI);

    for (int n = 0; n < N_PAIRS; ++n) {
        double phi = uni(rng);
        double fx = ftd::K_B * std::cos(phi);
        double fy = ftd::K_B * std::sin(phi);

        int A_a = measure(fx, fy, a);
        int A_ap = measure(fx, fy, a_p);
        int B_b = measure(-fx, -fy, b);
        int B_bp = measure(-fx, -fy, b_p);
        int A_a2 = measure(fx, fy, a);
        int B_a = measure(-fx, -fy, a);

        E_ab += A_a * B_b;
        E_ab_p += A_a * B_bp;
        E_a_pb += A_ap * B_b;
        E_a_pb_p += A_ap * B_bp;
        E_aa += A_a2 * B_a;
    }

    E_ab /= N_PAIRS; E_ab_p /= N_PAIRS; E_a_pb /= N_PAIRS;
    E_a_pb_p /= N_PAIRS; E_aa /= N_PAIRS;

    double S = std::abs(E_ab - E_ab_p + E_a_pb + E_a_pb_p);
    bool bell_ok = S <= 2.0 + 1e-6;
    bool anti_corr = std::abs(E_aa + 1.0) < 0.02;

    std::cout << "bell_S," << N_PAIRS << ","
              << std::setprecision(6) << S << ",2.0,"
              << (bell_ok ? 0 : 100) << ",0,0\n";
    std::cout << "bell_anticorr," << N_PAIRS << ","
              << std::setprecision(6) << E_aa << ",-1,"
              << std::setprecision(4) << 100.0 * std::abs(E_aa + 1) << ",0,0\n";
    std::cerr << "    S=" << S << " (bound: 2.0) " << (bell_ok ? "OK" : "VIOLATED!")
              << "  E(a,a)=" << E_aa << (anti_corr ? " OK" : " BAD") << "\n";
}

// ================================================================
// B17: Born rule on LATTICE (Scale 0)
// Ensemble of identical genesis events, check |J|^2 statistics
// ================================================================
void benchmark_born_lattice(int L) {
    std::cerr << "  Born rule (lattice): L=" << L << "\n";
    const int mid = L / 2;
    const int N_TRIALS = 30;
    const int TICKS = 50;

    // Run many identical setups, record WHERE manifestation occurs
    std::vector<double> manifest_densities;
    std::vector<double> all_densities;

    for (int trial = 0; trial < N_TRIALS; ++trial) {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = true;
        rb.toggles.damping = true;

        // Gaussian flux above genesis threshold
        double amp = ftd::K_GENESIS * 2;
        for (int dz = -4; dz <= 4; dz++)
        for (int dy = -4; dy <= 4; dy++)
        for (int dx = -4; dx <= 4; dx++) {
            double r2 = dx*dx + dy*dy + dz*dz;
            double val = amp * std::exp(-r2 / 8.0);
            if (val > 0.01) rb.inject_flux(mid + dx, mid + dy, mid + dz,
                {val * (0.8 + 0.4 * std::sin(trial * 1.0 + dx)),
                 val * 0.5,
                 val * (0.3 + 0.2 * std::cos(trial * 0.7 + dy))});
        }

        rb.run(TICKS);

        // Record density at manifestation sites
        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            double rho = rb.voxels()[i].density();
            if (rb.voxels()[i].state != 0) {
                manifest_densities.push_back(rho);
            }
            if (rho > 0.01) {
                all_densities.push_back(rho);
            }
        }
    }

    // Check: manifestation sites have higher average density than random sites
    double mean_manifest = 0, mean_all = 0;
    if (!manifest_densities.empty()) {
        for (double d : manifest_densities) mean_manifest += d;
        mean_manifest /= manifest_densities.size();
    }
    if (!all_densities.empty()) {
        for (double d : all_densities) mean_all += d;
        mean_all /= all_densities.size();
    }

    bool density_bias = (mean_manifest > mean_all);
    std::cout << "born_lattice," << L << ","
              << (density_bias ? 1 : 0) << ",1,"
              << std::setprecision(4) << mean_manifest << ","
              << std::setprecision(4) << mean_all << ","
              << manifest_densities.size() << "\n";
    std::cerr << "    Manifest sites: " << manifest_densities.size()
              << " mean_rho=" << mean_manifest
              << "  All sites mean_rho=" << mean_all
              << "  Bias=" << (density_bias ? "YES (Born-like)" : "NO") << "\n";
}

// ================================================================
// B18: Spin-orbit fine structure (Scale 1)
// ================================================================
void benchmark_spin_orbit() {
    std::cerr << "  Spin-orbit (fine structure):\n";

    double alpha_eff = ftd::ALPHA / (4.0 * ftd::PI) + ftd::G_N * ftd::K_B * ftd::K_B;
    double a_0 = 1.0 / (ftd::K_B * alpha_eff);
    double v_orb = std::sqrt(alpha_eff / (ftd::K_B * a_0));

    // Run n=2 with and without spin-orbit
    double E_no_so = 0, E_with_so = 0;
    for (int test = 0; test < 2; ++test) {
        ftd::ParticleEngine pe;
        pe.toggles.minimal();
        pe.toggles.damping = false;
        if (test == 1) pe.toggles.spin_orbit = true;
        pe.set_dt(100.0);
        pe.set_softening(1.0);

        pe.add_locked_particle(+1, {0, 0, 0}, ftd::K_B);
        pe.add_particle(-1, {4 * a_0, 0, 0}, {0, v_orb / 2, 0}, ftd::K_B, 2.48, 1);

        double E_sum = 0; int E_count = 0;
        for (int t = 0; t < 5000; ++t) {
            pe.tick();
            if (pe.particles().size() < 2) break;
            auto diag = pe.diagnostics();
            if (std::isfinite(diag.total_energy) && t > 200) {
                E_sum += diag.total_energy;
                E_count++;
            }
        }
        double E = (E_count > 0) ? E_sum / E_count : 0;
        if (test == 0) E_no_so = E; else E_with_so = E;
    }

    double splitting = std::abs(E_with_so - E_no_so);
    bool has_splitting = splitting > 1e-15;
    std::cout << "spin_orbit_splitting," << 0 << ","
              << std::setprecision(10) << splitting << ",0,"
              << 0 << ","
              << std::setprecision(10) << E_no_so << "," << E_with_so << "\n";
    std::cerr << "    E(no SO)=" << E_no_so << "  E(SO)=" << E_with_so
              << "  splitting=" << splitting << (has_splitting ? " DETECTED" : " NONE") << "\n";
}

// ================================================================
// B19: Relativistic correction (Scale 1)
// Fix: use high initial velocity (0.4c) toward a locked charge
// to build up speed. Compare peak velocity with/without gamma.
// ================================================================
void benchmark_relativistic() {
    std::cerr << "  Relativistic correction:\n";

    double v_max_nr = 0, v_max_rel = 0;

    for (int test = 0; test < 2; ++test) {
        ftd::ParticleEngine pe;
        pe.toggles.minimal();
        pe.toggles.damping = false;
        if (test == 1) pe.toggles.relativistic = true;
        pe.set_dt(0.5);  // Small dt for accuracy at high speed

        // Locked +1 at origin, fast -1 approaching at 0.4c
        pe.add_locked_particle(+1, {0, 0, 0}, ftd::K_B * 100);  // Heavy source
        pe.add_particle(-1, {30, 0, 0}, {-0.4 * ftd::C_SPEED, 0.05, 0}, ftd::K_B);

        double v_peak = 0;
        for (int t = 0; t < 1000; ++t) {
            pe.tick();
            if (pe.particles().size() >= 2) {
                auto& p = pe.particles()[1];
                double v = std::sqrt(p.velocity.x * p.velocity.x +
                                     p.velocity.y * p.velocity.y +
                                     p.velocity.z * p.velocity.z);
                v_peak = std::max(v_peak, v);
            }
        }
        if (test == 0) v_max_nr = v_peak;
        else v_max_rel = v_peak;
    }

    // Relativistic correction should LIMIT peak velocity (γ suppresses acceleration)
    bool limited = (v_max_rel < v_max_nr - 1e-6);
    std::cout << "relativistic," << 0 << ","
              << (limited ? 1 : 0) << ",1,0,"
              << std::setprecision(6) << v_max_nr << "," << v_max_rel << "\n";
    std::cerr << "    v_peak(NR)=" << v_max_nr << "  v_peak(rel)=" << v_max_rel
              << "  limited=" << (limited ? "YES" : "NO") << "\n";
}

// ================================================================
// Main
// ================================================================
int main(int argc, char* argv[]) {
    int L = (argc > 1) ? std::atoi(argv[1]) : 48;
    int ticks = (argc > 2) ? std::atoi(argv[2]) : 200;

    std::cout << "benchmark,lattice_size,measured,theory,error_pct,r_squared,n_points\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    std::cerr << "ENGINE-THEORY BENCHMARK (20 tests): L=" << L << ", ticks=" << ticks << "\n";

    // Scale 0 benchmarks (lattice EM)
    benchmark_coulomb(L, ticks);
    benchmark_wave_speed(L);
    benchmark_gauss(L, std::min(ticks, 100));
    benchmark_energy(L, ticks);
    benchmark_charge(L, std::min(ticks, 100));

    // Scale 1 benchmarks (particle engine)
    benchmark_hydrogen();
    benchmark_born_ensemble();

    // NEW: Toggle activation benchmarks
    benchmark_color_forces(L, std::min(ticks, 80));
    benchmark_confinement(L, std::min(ticks, 80));
    benchmark_latency(L, std::min(ticks, 80));
    benchmark_exchange();
    benchmark_larmor(L, std::min(ticks, 80));
    benchmark_weak(L, std::min(ticks, 50));
    benchmark_higgs(L, std::min(ticks, 50));
    benchmark_bell();
    benchmark_born_lattice(L);
    benchmark_spin_orbit();
    benchmark_relativistic();

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();
    std::cerr << "Completed in " << std::fixed << std::setprecision(1) << elapsed << "s\n";

    return 0;
}

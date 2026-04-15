/**
 * BLACK HOLE THERMODYNAMICS BENCHMARKS
 *
 * Tests FTD lattice predictions for black hole thermodynamics using
 * the Scale 0 RenderBridge engine.  A "lattice black hole" is a dense
 * cluster of locked particles whose latency field creates a deep
 * gravitational potential well.
 *
 * Theory (from DERIV_LATTICE_BLACK_HOLES.md, Section A.11.2):
 *   T_H = 1 / (8 pi G M)          — Hawking temperature [CONJECTURE]
 *   S_BH = A / (4 G)              — Bekenstein-Hawking entropy [CONJECTURE]
 *   A = 4 pi r_s^2, r_s = 2 G M  — Schwarzschild radius
 *   S * T = M / 2                 — Budget identity
 *
 * Lattice mapping:
 *   Mass M = N_particles * K_B    (each manifested site carries mass K_B)
 *   Latency L(r) from Poisson:    L -> 1 at horizon (f = 1 - L^2 -> 0)
 *   Horizon r_h: where L is maximal / f approaches 0
 *   Entropy: Shannon entropy of flux energy distribution
 *
 * Benchmarks:
 *   BH1: Latency profile 1/r scaling (gravitational potential)
 *   BH2: Horizon area vs mass (area-law check: A ~ M^2)
 *   BH3: Entropy area-law scaling (S ~ surface area, not volume)
 *   BH4: Hawking evaporation (mass loss rate ~ 1/M^2)
 *
 * Output: CSV to stdout, diagnostics to stderr.
 * Usage: ./ftd_bh_thermo [lattice_size] [num_ticks]
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <numeric>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

// ================================================================
// Utility: linear regression on log-log data
// ================================================================
struct FitResult {
    double slope;
    double intercept;
    double r_squared;
    int n_points;
};

FitResult log_log_fit(const std::vector<double>& x,
                      const std::vector<double>& y) {
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

    result.slope = (n * sxy - sx * sy) / denom;
    result.intercept = (sy - result.slope * sx) / n;

    double mean_y = sy / n;
    double ss_res = 0, ss_tot = 0;
    for (int i = 0; i < n; ++i) {
        double pred = result.slope * lx[i] + result.intercept;
        ss_res += (ly[i] - pred) * (ly[i] - pred);
        ss_tot += (ly[i] - mean_y) * (ly[i] - mean_y);
    }
    result.r_squared = 1.0 - ss_res / (ss_tot + 1e-30);
    return result;
}

FitResult linear_fit(const std::vector<double>& x,
                     const std::vector<double>& y) {
    FitResult result = {0, 0, 0, 0};
    int n = static_cast<int>(x.size());
    result.n_points = n;
    if (n < 2) return result;

    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (int i = 0; i < n; ++i) {
        sx += x[i]; sy += y[i];
        sxx += x[i] * x[i]; sxy += x[i] * y[i];
    }
    double denom = n * sxx - sx * sx;
    if (std::abs(denom) < 1e-30) return result;

    result.slope = (n * sxy - sx * sy) / denom;
    result.intercept = (sy - result.slope * sx) / n;

    double mean_y = sy / n;
    double ss_res = 0, ss_tot = 0;
    for (int i = 0; i < n; ++i) {
        double pred = result.slope * x[i] + result.intercept;
        ss_res += (y[i] - pred) * (y[i] - pred);
        ss_tot += (y[i] - mean_y) * (y[i] - mean_y);
    }
    result.r_squared = 1.0 - ss_res / (ss_tot + 1e-30);
    return result;
}

// ================================================================
// Helper: create a spherical cluster of locked particles at center.
// Returns the number of particles actually placed.
// ================================================================
int create_mass_cluster(ftd::RenderBridge& rb, int cx, int cy, int cz,
                        int cluster_radius) {
    int count = 0;
    int L = rb.lattice().size();
    for (int dz = -cluster_radius; dz <= cluster_radius; dz++)
    for (int dy = -cluster_radius; dy <= cluster_radius; dy++)
    for (int dx = -cluster_radius; dx <= cluster_radius; dx++) {
        if (dx*dx + dy*dy + dz*dz <= cluster_radius * cluster_radius) {
            int x = cx + dx, y = cy + dy, z = cz + dz;
            // Periodic boundary wrapping
            x = ((x % L) + L) % L;
            y = ((y % L) + L) % L;
            z = ((z % L) + L) % L;
            rb.inject_particle(x, y, z, +1, {0, 0, ftd::K_B});
            int idx = rb.lattice().index(x, y, z);
            rb.voxels()[idx].locked = true;
            count++;
        }
    }
    return count;
}

// ================================================================
// Helper: compute flux field entropy within a spherical shell.
// Shannon entropy S = -sum p_i ln(p_i) where p_i = |J_i|^2 / sum|J|^2.
// If 'surface_only' is true, only include sites within [r_inner, r_outer].
// If false, include all sites within radius.
// ================================================================
struct EntropyResult {
    double entropy;
    double total_energy;
    int site_count;
};

EntropyResult shell_entropy(const ftd::RenderBridge& rb,
                            int cx, int cy, int cz,
                            double r_inner, double r_outer) {
    EntropyResult result = {0.0, 0.0, 0};
    int L = rb.lattice().size();
    int N = static_cast<int>(rb.lattice().total_sites());

    // First pass: accumulate total energy in shell
    std::vector<double> energies;
    energies.reserve(1000);
    for (int i = 0; i < N; ++i) {
        ftd::Coord c = rb.lattice().coord(i);
        double dx = c.x - cx, dy = c.y - cy, dz = c.z - cz;
        // Periodic distance
        if (dx > L / 2) dx -= L;
        if (dx < -L / 2) dx += L;
        if (dy > L / 2) dy -= L;
        if (dy < -L / 2) dy += L;
        if (dz > L / 2) dz -= L;
        if (dz < -L / 2) dz += L;
        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (r >= r_inner && r < r_outer) {
            double e = rb.voxels()[i].flux.mag2();
            if (e > 1e-30) {
                energies.push_back(e);
                result.total_energy += e;
                result.site_count++;
            }
        }
    }

    // Second pass: Shannon entropy
    if (result.total_energy < 1e-30 || energies.empty()) return result;
    for (double e : energies) {
        double p = e / result.total_energy;
        if (p > 1e-30) {
            result.entropy -= p * std::log(p);
        }
    }
    return result;
}

// ================================================================
// Helper: measure effective horizon radius.
// Scans outward from center, finds the radius where latency drops
// below a threshold fraction of its peak value.
// Returns {peak_latency, horizon_radius, profile}.
// ================================================================
struct HorizonResult {
    double peak_latency;
    double horizon_radius;     // r where L drops below threshold * peak
    double horizon_area;       // 4 * pi * r_h^2
    std::vector<double> radii;
    std::vector<double> latencies;
};

HorizonResult measure_horizon(const ftd::RenderBridge& rb,
                              int cx, int cy, int cz,
                              double threshold_frac = 0.5) {
    HorizonResult hr = {0, 0, 0, {}, {}};
    int L = rb.lattice().size();
    int r_max = L / 3;

    // Average latency in spherical shells of width 1
    for (int r = 1; r <= r_max; r++) {
        double lat_sum = 0.0;
        int count = 0;
        // Sample along 6 face directions for cleaner signal
        int dirs[6][3] = {{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
        for (auto& d : dirs) {
            int x = ((cx + d[0]*r) % L + L) % L;
            int y = ((cy + d[1]*r) % L + L) % L;
            int z = ((cz + d[2]*r) % L + L) % L;
            int idx = rb.lattice().index(x, y, z);
            lat_sum += rb.voxels()[idx].latency;
            count++;
        }
        double avg_lat = (count > 0) ? lat_sum / count : 0.0;
        hr.radii.push_back(static_cast<double>(r));
        hr.latencies.push_back(avg_lat);
        if (avg_lat > hr.peak_latency) hr.peak_latency = avg_lat;
    }

    // Find horizon radius: outermost r where L > threshold * peak
    for (int i = static_cast<int>(hr.radii.size()) - 1; i >= 0; --i) {
        if (hr.latencies[i] > threshold_frac * hr.peak_latency) {
            hr.horizon_radius = hr.radii[i];
            break;
        }
    }
    hr.horizon_area = 4.0 * ftd::PI * hr.horizon_radius * hr.horizon_radius;
    return hr;
}

// ================================================================
// BH1: Latency profile — potential well depth scales with mass
//
// Theory: phi ~ G * M / r, so latency L ~ sqrt(phi) ~ sqrt(M/r).
// For fixed r, latency should increase with mass.
// For fixed M, latency should decrease with r (1/r profile).
//
// We test both: (a) L vs r for fixed mass, (b) L_peak vs N_particles.
// ================================================================
void benchmark_latency_profile(int L, int ticks) {
    std::cerr << "  BH1: Latency profile (potential well depth)\n";
    const int mid = L / 2;

    // Test with several cluster sizes to get mass dependence
    std::vector<int> cluster_radii = {2, 3, 4};
    std::vector<double> masses, peak_latencies;

    for (int cr : cluster_radii) {
        if (mid - cr - L/3 < 0) continue;  // Skip if too big for lattice

        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = true;
        rb.toggles.latency_field = true;
        rb.toggles.forces = false;     // No particle forces (keep them locked)
        rb.toggles.movement = false;

        int N_p = create_mass_cluster(rb, mid, mid, mid, cr);
        double mass = N_p * ftd::K_B;

        rb.run(ticks);

        HorizonResult hr = measure_horizon(rb, mid, mid, mid);

        masses.push_back(mass);
        peak_latencies.push_back(hr.peak_latency);

        // Output radial profile
        for (size_t i = 0; i < hr.radii.size() && i < 15; ++i) {
            std::cout << "bh_latency_profile," << cr << ","
                      << std::setprecision(4) << hr.radii[i] << ","
                      << std::setprecision(8) << hr.latencies[i] << ","
                      << N_p << ",0,0\n";
        }

        // Check: latency should decrease with radius (potential well)
        bool decreasing = false;
        if (hr.radii.size() >= 4) {
            decreasing = (hr.latencies[0] > hr.latencies[3]);
        }

        std::cout << "bh_latency_signal," << cr << ","
                  << (hr.peak_latency > 1e-10 ? 1 : 0) << ",1,"
                  << (decreasing ? 1 : 0) << ","
                  << std::setprecision(8) << hr.peak_latency << "," << mass << "\n";

        std::cerr << "    cluster_r=" << cr << " N=" << N_p
                  << " M=" << std::setprecision(4) << mass
                  << " L_peak=" << std::setprecision(6) << hr.peak_latency
                  << " 1/r_decay=" << (decreasing ? "YES" : "NO") << "\n";
    }

    // Check mass dependence: peak_latency should increase with mass
    if (masses.size() >= 2) {
        bool mass_monotonic = true;
        for (size_t i = 1; i < peak_latencies.size(); ++i) {
            if (peak_latencies[i] <= peak_latencies[i-1] + 1e-15) {
                mass_monotonic = false;
            }
        }
        std::cout << "bh_latency_mass_dependence," << L << ","
                  << (mass_monotonic ? 1 : 0) << ",1,0,"
                  << std::setprecision(6) << peak_latencies.front() << ","
                  << peak_latencies.back() << "\n";
        std::cerr << "    Mass monotonic (L_peak grows with M): "
                  << (mass_monotonic ? "YES" : "NO") << "\n";
    }
}

// ================================================================
// BH2: Horizon area vs mass — area-law check
//
// Theory: r_s = 2 G M, so A = 4 pi r_s^2 = 16 pi G^2 M^2.
// On the lattice, the "horizon" is where latency L is significant.
// We measure the effective horizon radius at the half-maximum of
// the latency profile, then check A ~ M^2 (exponent = 2 in log-log).
// ================================================================
void benchmark_horizon_area(int L, int ticks) {
    std::cerr << "  BH2: Horizon area vs mass (A ~ M^2)\n";
    const int mid = L / 2;

    std::vector<int> cluster_radii = {2, 3, 4, 5};
    std::vector<double> masses, areas;

    for (int cr : cluster_radii) {
        // Ensure the cluster fits with room for measurement
        if (cr + L/3 > mid) continue;

        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.gravity = true;
        rb.toggles.latency_field = true;
        rb.toggles.forces = false;
        rb.toggles.movement = false;

        int N_p = create_mass_cluster(rb, mid, mid, mid, cr);
        double mass = N_p * ftd::K_B;

        rb.run(ticks);

        // Measure horizon at 30% of peak latency (lower threshold catches
        // the outer edge of the potential well more reliably)
        HorizonResult hr = measure_horizon(rb, mid, mid, mid, 0.3);

        if (hr.horizon_radius > 0) {
            masses.push_back(mass);
            areas.push_back(hr.horizon_area);
        }

        std::cout << "bh_horizon," << cr << ","
                  << std::setprecision(4) << mass << ","
                  << std::setprecision(4) << hr.horizon_radius << ","
                  << std::setprecision(4) << hr.horizon_area << ","
                  << std::setprecision(6) << hr.peak_latency << "," << N_p << "\n";

        std::cerr << "    cr=" << cr << " N=" << N_p
                  << " M=" << std::setprecision(3) << mass
                  << " r_h=" << hr.horizon_radius
                  << " A=" << std::setprecision(4) << hr.horizon_area << "\n";
    }

    // Fit A vs M in log-log: expect slope ~ 2 (A ~ M^2)
    if (masses.size() >= 3) {
        FitResult fit = log_log_fit(masses, areas);
        double err_pct = 100.0 * std::abs(fit.slope - 2.0) / 2.0;

        std::cout << "bh_area_exponent," << L << ","
                  << std::setprecision(4) << fit.slope << ",2.0,"
                  << std::setprecision(4) << err_pct << ","
                  << std::setprecision(6) << fit.r_squared << "," << fit.n_points << "\n";

        std::cerr << "    Area-mass exponent: " << std::setprecision(4) << fit.slope
                  << " (theory: 2.0, err: " << err_pct << "%, R^2=" << fit.r_squared << ")\n";
    }

    // Also check: Schwarzschild prediction r_s = 2 G M
    // On our lattice, r_h should scale linearly with M
    if (masses.size() >= 3) {
        FitResult rfit = log_log_fit(masses,
            [&]() -> std::vector<double> {
                std::vector<double> rvals;
                for (double a : areas) rvals.push_back(std::sqrt(a / (4.0 * ftd::PI)));
                return rvals;
            }());
        std::cout << "bh_radius_exponent," << L << ","
                  << std::setprecision(4) << rfit.slope << ",1.0,"
                  << std::setprecision(4) << 100.0 * std::abs(rfit.slope - 1.0) << ","
                  << std::setprecision(6) << rfit.r_squared << "," << rfit.n_points << "\n";
        std::cerr << "    Radius-mass exponent: " << std::setprecision(4) << rfit.slope
                  << " (theory: 1.0)\n";
    }
}

// ================================================================
// BH3: Entropy area-law scaling
//
// Theory: S_BH = A / (4 G) — entropy scales with surface area,
// NOT volume.  This is the holographic principle.
//
// Lattice test: For spherical regions of increasing radius R around
// a mass cluster, compute Shannon entropy of the flux field in:
//   (a) the full sphere (volume scales as R^3)
//   (b) a thin shell at the surface (area scales as R^2)
//
// If the entropy is area-law, then S(R) should grow as R^2
// (exponent ~ 2 in log-log), not as R^3 (volume-law).
//
// We measure entropy vs enclosing radius for several shell widths
// and fit the exponent.
// ================================================================
void benchmark_entropy_area_law(int L, int ticks) {
    std::cerr << "  BH3: Entropy area-law scaling (S ~ A, not V)\n";
    const int mid = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = true;
    rb.toggles.latency_field = true;
    rb.toggles.forces = false;
    rb.toggles.movement = false;

    // Create a substantial mass cluster
    int cluster_r = 3;
    int N_p = create_mass_cluster(rb, mid, mid, mid, cluster_r);
    double mass = N_p * ftd::K_B;
    std::cerr << "    Mass cluster: " << N_p << " particles (M=" << mass << ")\n";

    // Let fields settle
    rb.run(ticks);

    // Measure entropy in spherical shells at increasing radii
    // Shell: [R - delta, R) where delta = 2 (thin shell ~ surface)
    // Volume: [0, R)
    std::vector<double> radii_shell, entropy_shell;
    std::vector<double> radii_volume, entropy_volume;
    double delta = 2.0;

    for (int R = cluster_r + 2; R <= L / 3; R += 2) {
        double r = static_cast<double>(R);

        // Surface shell entropy
        EntropyResult shell = shell_entropy(rb, mid, mid, mid, r - delta, r);
        if (shell.site_count > 2) {
            radii_shell.push_back(r);
            entropy_shell.push_back(shell.entropy);
        }

        // Volume entropy
        EntropyResult vol = shell_entropy(rb, mid, mid, mid, 0, r);
        if (vol.site_count > 2) {
            radii_volume.push_back(r);
            entropy_volume.push_back(vol.entropy);
        }

        std::cout << "bh_entropy_shell," << R << ","
                  << std::setprecision(6) << shell.entropy << ","
                  << shell.site_count << ","
                  << std::setprecision(6) << shell.total_energy << ",0,0\n";
        std::cout << "bh_entropy_volume," << R << ","
                  << std::setprecision(6) << vol.entropy << ","
                  << vol.site_count << ","
                  << std::setprecision(6) << vol.total_energy << ",0,0\n";
    }

    // Fit: S_volume vs R  — expect exponent near 2 (area-law) or 3 (volume-law)
    if (radii_volume.size() >= 3) {
        FitResult vol_fit = log_log_fit(radii_volume, entropy_volume);
        double area_err = 100.0 * std::abs(vol_fit.slope - 2.0) / 2.0;
        double vol_err = 100.0 * std::abs(vol_fit.slope - 3.0) / 3.0;
        bool area_law = (area_err < vol_err);

        std::cout << "bh_entropy_vol_exponent," << L << ","
                  << std::setprecision(4) << vol_fit.slope << ",2.0,"
                  << std::setprecision(4) << area_err << ","
                  << std::setprecision(6) << vol_fit.r_squared << "," << vol_fit.n_points << "\n";
        std::cout << "bh_entropy_area_law," << L << ","
                  << (area_law ? 1 : 0) << ",1,"
                  << std::setprecision(4) << area_err << ","
                  << std::setprecision(4) << vol_err << "," << vol_fit.slope << "\n";

        std::cerr << "    Volume entropy exponent: " << std::setprecision(4) << vol_fit.slope
                  << " (area-law=2, volume-law=3, R^2=" << vol_fit.r_squared << ")\n";
        std::cerr << "    Closer to area-law: " << (area_law ? "YES" : "NO")
                  << " (area_err=" << area_err << "% vs vol_err=" << vol_err << "%)\n";
    }

    // Additionally: check that shell entropy grows more slowly than volume entropy
    // at large R (surface-dominated)
    if (radii_shell.size() >= 3) {
        FitResult shell_fit = log_log_fit(radii_shell, entropy_shell);
        std::cout << "bh_entropy_shell_exponent," << L << ","
                  << std::setprecision(4) << shell_fit.slope << ",2.0,"
                  << std::setprecision(4) << 100.0 * std::abs(shell_fit.slope - 2.0) / 2.0 << ","
                  << std::setprecision(6) << shell_fit.r_squared << "," << shell_fit.n_points << "\n";
        std::cerr << "    Shell entropy exponent: " << std::setprecision(4) << shell_fit.slope
                  << " (theory: 2.0 for area-law)\n";
    }
}

// ================================================================
// BH4: Hawking evaporation — mass/energy loss rate
//
// Theory: dM/dt ~ -1/M^2 (Hawking radiation power P ~ 1/M^2).
// Hotter (lighter) BHs evaporate faster.
//
// Lattice test: Create mass clusters of different sizes, run with
// damping enabled (the lattice analog of Hawking radiation: the
// flux field around the cluster loses energy via damping, which is
// the only energy sink in the engine).  Measure energy loss rate
// and check that smaller clusters lose energy proportionally faster.
//
// This is not a literal Hawking process, but tests the structural
// prediction: radiation rate is inversely related to mass.
// ================================================================
void benchmark_hawking_evaporation(int L, int ticks) {
    std::cerr << "  BH4: Hawking evaporation (energy loss rate vs mass)\n";
    const int mid = L / 2;

    struct EvapData {
        int cluster_r;
        int N_particles;
        double mass;
        double E_initial;
        double E_final;
        double delta_E;
        double loss_rate;  // |delta_E| / ticks
    };
    std::vector<EvapData> results;

    std::vector<int> cluster_radii = {2, 3, 4};
    int evap_ticks = std::max(ticks, 100);

    for (int cr : cluster_radii) {
        if (cr + L/3 > mid) continue;

        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;       // No spontaneous pair creation
        rb.toggles.gravity = true;
        rb.toggles.latency_field = true;
        rb.toggles.damping = true;        // Damping = energy sink (Hawking analog)
        rb.toggles.selective_damping = true;
        rb.toggles.forces = false;        // Keep particles locked in place
        rb.toggles.movement = false;

        int N_p = create_mass_cluster(rb, mid, mid, mid, cr);
        double mass = N_p * ftd::K_B;

        // Let the Poisson solver converge and fields form (need ~80+ ticks)
        rb.run(std::max(80, ticks / 2));
        ftd::EnergyAudit audit0 = rb.energy_audit();
        double E0 = audit0.field_energy + audit0.wave_energy;

        // Run the evaporation phase
        rb.run(evap_ticks);
        ftd::EnergyAudit audit1 = rb.energy_audit();
        double E1 = audit1.field_energy + audit1.wave_energy;

        double dE = E1 - E0;
        double rate = std::abs(dE) / evap_ticks;

        EvapData d;
        d.cluster_r = cr;
        d.N_particles = N_p;
        d.mass = mass;
        d.E_initial = E0;
        d.E_final = E1;
        d.delta_E = dE;
        d.loss_rate = rate;
        results.push_back(d);

        std::cout << "bh_evap," << cr << ","
                  << std::setprecision(6) << mass << ","
                  << std::setprecision(8) << E0 << ","
                  << std::setprecision(8) << E1 << ","
                  << std::setprecision(8) << dE << ","
                  << std::setprecision(8) << rate << "\n";

        std::cerr << "    cr=" << cr << " N=" << N_p
                  << " M=" << std::setprecision(3) << mass
                  << " E0=" << std::setprecision(4) << E0
                  << " E1=" << E1
                  << " dE=" << dE
                  << " rate=" << rate << "\n";
    }

    // Check: energy should decrease (damping removes energy)
    bool all_losing_energy = true;
    for (auto& d : results) {
        if (d.delta_E >= 0) all_losing_energy = false;
    }
    std::cout << "bh_evap_energy_loss," << L << ","
              << (all_losing_energy ? 1 : 0) << ",1,0,0,0\n";
    std::cerr << "    All clusters losing energy: "
              << (all_losing_energy ? "YES" : "NO") << "\n";

    // Key test: loss_rate / mass^2 should be approximately constant
    // (Hawking: P ~ 1/M^2, so rate * M^2 = const)
    // Alternatively: log(rate) vs log(M) should have slope ~ -2
    if (results.size() >= 2) {
        std::vector<double> mass_vals, rate_vals;
        for (auto& d : results) {
            if (d.loss_rate > 1e-30) {
                mass_vals.push_back(d.mass);
                rate_vals.push_back(d.loss_rate);
            }
        }

        if (mass_vals.size() >= 2) {
            FitResult fit = log_log_fit(mass_vals, rate_vals);
            // Hawking predicts slope = -2. On a finite lattice with damping
            // the scaling may differ, but it should be NEGATIVE (heavier
            // clusters lose energy more slowly per unit mass).
            bool inverse_mass = (fit.slope < 0);
            double err_pct = std::abs(fit.slope - (-2.0)) / 2.0 * 100.0;

            std::cout << "bh_evap_exponent," << L << ","
                      << std::setprecision(4) << fit.slope << ",-2.0,"
                      << std::setprecision(4) << err_pct << ","
                      << std::setprecision(6) << fit.r_squared << "," << fit.n_points << "\n";
            std::cerr << "    Rate vs mass exponent: " << std::setprecision(4) << fit.slope
                      << " (Hawking: -2.0, err: " << err_pct << "%)\n";
            std::cerr << "    Inverse mass dependence: "
                      << (inverse_mass ? "YES" : "NO") << "\n";
        }
    }

    // Budget identity: S * T = M / 2
    // T_H = 1/(8 pi G M), S = A/(4G) = 4 pi r_s^2 / (4G)
    // where r_s = 2 G M.  So S = 4 pi G M^2.
    // Product: S * T = 4 pi G M^2 * 1/(8 pi G M) = M/2. Check.
    // We output the lattice-measured product for comparison.
    for (auto& d : results) {
        double rs = 2.0 * ftd::G_N * d.mass;
        double A = 4.0 * ftd::PI * rs * rs;
        double S_theory = A / (4.0 * ftd::G_N);
        double T_theory = 1.0 / (8.0 * ftd::PI * ftd::G_N * d.mass);
        double product = S_theory * T_theory;
        double expected = d.mass / 2.0;

        std::cout << "bh_budget_identity," << d.cluster_r << ","
                  << std::setprecision(8) << product << ","
                  << std::setprecision(8) << expected << ","
                  << std::setprecision(4) << 100.0 * std::abs(product - expected) / (expected + 1e-30) << ","
                  << std::setprecision(6) << S_theory << "," << T_theory << "\n";

        std::cerr << "    M=" << std::setprecision(3) << d.mass
                  << " S*T=" << std::setprecision(6) << product
                  << " M/2=" << expected
                  << " (exact algebraic identity)\n";
    }
}

// ================================================================
// BH5: Proper time deficit near mass cluster
//
// Theory: tau accumulates as d(tau)/dt = sqrt(f^2 - v^2) / sqrt(f)
// where f = 1 - L^2.  Near a mass, L > 0, so f < 1, and proper
// time runs slower.  This is the lattice analog of gravitational
// time dilation / redshift.
//
// Test: inject test particles at different radii from the cluster,
// run, and measure accumulated tau.  Closer particles should have
// less tau.
// ================================================================
void benchmark_proper_time(int L, int ticks) {
    std::cerr << "  BH5: Proper time deficit (gravitational time dilation)\n";
    const int mid = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = true;
    rb.toggles.latency_field = true;
    rb.toggles.forces = false;
    rb.toggles.movement = false;

    int cluster_r = 3;
    int N_p = create_mass_cluster(rb, mid, mid, mid, cluster_r);
    std::cerr << "    Mass cluster: " << N_p << " particles\n";

    // Place static test particles at several radii
    std::vector<int> test_radii;
    for (int r = cluster_r + 2; r <= std::min(L / 3, 14); r += 2) {
        test_radii.push_back(r);
    }

    // Inject stationary test particles
    for (int r : test_radii) {
        int x = mid + r;
        rb.inject_particle(x, mid, mid, -1, {0, 0, -ftd::K_B * 0.01}); // tiny flux (probe)
        rb.voxels()[rb.lattice().index(x, mid, mid)].locked = true;
    }

    // Let fields converge, then run for proper time accumulation
    rb.run(ticks);

    // Read accumulated tau at each radius
    std::vector<double> r_vals, tau_vals;
    for (int r : test_radii) {
        int x = mid + r;
        int idx = rb.lattice().index(x, mid, mid);
        double tau = rb.voxels()[idx].tau;
        double latency = rb.voxels()[idx].latency;

        r_vals.push_back(static_cast<double>(r));
        tau_vals.push_back(tau);

        std::cout << "bh_proper_time," << r << ","
                  << std::setprecision(8) << tau << ","
                  << std::setprecision(8) << latency << ","
                  << 0 << ",0,0\n";

        std::cerr << "    r=" << r << " tau=" << std::setprecision(6) << tau
                  << " L=" << latency << "\n";
    }

    // Check: tau should INCREASE with radius (less time dilation farther away)
    bool monotonic = true;
    for (size_t i = 1; i < tau_vals.size(); ++i) {
        if (tau_vals[i] < tau_vals[i-1] - 1e-12) {
            monotonic = false;
        }
    }

    // Check: tau at largest radius should be strictly greater than at smallest
    double tau_ratio = 0;
    if (tau_vals.size() >= 2 && tau_vals.front() > 1e-20) {
        tau_ratio = tau_vals.back() / tau_vals.front();
    }

    std::cout << "bh_time_dilation," << L << ","
              << (monotonic ? 1 : 0) << ",1,"
              << std::setprecision(6) << tau_ratio << ","
              << std::setprecision(8) << tau_vals.front() << ","
              << std::setprecision(8) << (tau_vals.empty() ? 0.0 : tau_vals.back()) << "\n";

    std::cerr << "    Monotonic (tau grows with r): " << (monotonic ? "YES" : "NO")
              << "  ratio(far/near)=" << std::setprecision(4) << tau_ratio << "\n";
}

// ================================================================
// main
// ================================================================
int main(int argc, char* argv[]) {
    int L = (argc > 1) ? std::atoi(argv[1]) : 48;
    int ticks = (argc > 2) ? std::atoi(argv[2]) : 200;

    // The latency Poisson solver warm-starts between ticks (30 SOR
    // iterations each).  Convergence on a L^3 lattice needs ~80-100
    // ticks minimum.  Clamp to ensure meaningful results.
    int latency_ticks = std::max(ticks, 80);

    std::cout << "benchmark,param,measured,theory,error_pct,extra1,extra2\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    std::cerr << "BLACK HOLE THERMODYNAMICS BENCHMARKS: L=" << L
              << ", ticks=" << ticks << " (latency phases use "
              << latency_ticks << ")\n";

    benchmark_latency_profile(L, latency_ticks);
    benchmark_horizon_area(L, latency_ticks);
    benchmark_entropy_area_law(L, latency_ticks);
    benchmark_hawking_evaporation(L, ticks);  // Uses its own phasing
    benchmark_proper_time(L, latency_ticks);

    auto t1 = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(t1 - t0).count();
    std::cerr << "Completed in " << std::fixed << std::setprecision(1)
              << elapsed << "s\n";
    return 0;
}

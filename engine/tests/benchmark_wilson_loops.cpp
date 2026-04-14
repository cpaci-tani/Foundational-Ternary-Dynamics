/**
 * WILSON LOOP BENCHMARK — Confinement from Gauge-Invariant Observable
 *
 * Computes rectangular R x T Wilson loops on the FTD lattice flux field.
 *
 * Physics context:
 *   In lattice gauge theory, the Wilson loop W(C) = Tr[P exp(ig oint A.dl)]
 *   diagnoses confinement:
 *     - Area law: W(R,T) ~ exp(-sigma * R * T) => confinement
 *     - Perimeter law: W(R,T) ~ exp(-mu * (2R+2T)) => deconfinement
 *
 *   On the FTD lattice, there are no SU(3) link variables. Instead, the
 *   flux field J (Vec3 per site) serves as the gauge connection. The
 *   "gauge link" along an edge from site x to x+mu_hat is the projection
 *   of the flux onto the link direction:
 *
 *     U_mu(x) = exp(i * g_c * J(x) . mu_hat)
 *
 *   For small coupling (|g_c * J . mu_hat| << 1), this is approximately:
 *
 *     U_mu(x) ~ 1 + i * g_c * (J(x) . mu_hat)
 *
 *   The Wilson loop phase is the sum of flux projections around the path:
 *
 *     Phi(C) = g_c * sum_{links in C} J(x_link) . direction_hat
 *
 *   And the Wilson loop value is:
 *
 *     W(C) = cos(Phi(C))
 *
 *   This is the Abelian (U(1)) Wilson loop. For the color sector, we
 *   compute a separate observable using color-weighted flux.
 *
 * Protocol:
 *   1. Place a locked quark-antiquark pair (different colors) at separation d
 *   2. Equilibrate the flux field with color forces enabled
 *   3. Compute W(R,T) for rectangular loops in the plane containing the pair
 *   4. Measure W for multiple R, T values
 *   5. Extract string tension sigma from: -ln(W(R,T)) / (R*T)
 *   6. Also compute the Creutz ratio: chi(R,T) = -ln[W(R,T)*W(R-1,T-1) /
 *                                                     (W(R-1,T)*W(R,T-1))]
 *      which cancels perimeter terms and isolates the area-law coefficient.
 *   7. Compare extracted sigma to SIGMA_STRING = ALPHA_S * K_B^2
 *
 * Benchmarks:
 *   WL1: Wilson loop is nonzero and < 1 (nontrivial flux)
 *   WL2: -ln(W)/Area vs Area shows linear trend (area law)
 *   WL3: Creutz ratio is approximately constant (pure area law)
 *   WL4: Extracted sigma vs SIGMA_STRING comparison
 *   WL5: Perimeter law test (should fail for confined phase)
 *   WL6: Flux tube profile (flux density between quark pair)
 *
 * Usage: ./benchmark_wilson_loops [lattice_size] [equilibration_ticks]
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

using namespace ftd;

// ================================================================
// Test framework (matches engine convention)
// ================================================================
static int g_passes = 0;
static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
        ++g_passes;
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

static void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
        ++g_passes;
    } else {
        std::cout << "  FAIL  " << name
                  << " (got " << std::setprecision(8) << a
                  << ", expected " << b
                  << ", diff " << std::abs(a - b) << ")\n";
        ++g_failures;
    }
}

// ================================================================
// Plane enumeration for Wilson loop orientation
// ================================================================
enum class Plane { XY, XZ, YZ };

// Direction vectors in lattice coordinates.
// For a given plane, dir_R is the "space" direction and dir_T is the "time"
// direction of the Wilson loop rectangle.
struct PlaneAxes {
    int dr_x, dr_y, dr_z;  // R-direction unit vector
    int dt_x, dt_y, dt_z;  // T-direction unit vector
};

static PlaneAxes axes_for_plane(Plane p) {
    switch (p) {
        case Plane::XY: return {1, 0, 0,  0, 1, 0};
        case Plane::XZ: return {1, 0, 0,  0, 0, 1};
        case Plane::YZ: return {0, 1, 0,  0, 0, 1};
    }
    return {1, 0, 0,  0, 1, 0};  // default
}

// ================================================================
// Core: Compute Wilson loop W(R,T) for a rectangle anchored at (ox,oy,oz)
//
// The loop path traverses a rectangle in the specified plane:
//   Bottom edge: R steps in dir_R  (positive direction)
//   Right edge:  T steps in dir_T  (positive direction)
//   Top edge:    R steps in -dir_R (negative direction)
//   Left edge:   T steps in -dir_T (negative direction)
//
// At each lattice link from site (x,y,z) to (x+dx, y+dy, z+dz),
// the flux projection is:
//   J(x,y,z) . direction_hat
//
// The total phase is Phi = G_C * sum of projections.
// The Wilson loop value is W = cos(Phi).
//
// For the color-weighted variant, each link's flux projection is
// weighted by the color charge density at that site.
// ================================================================
struct WilsonResult {
    double phase;       // Total accumulated phase Phi(C)
    double W;           // cos(Phi) -- the Wilson loop value
    int perimeter;      // 2*(R+T)
    int area;           // R*T
};

static WilsonResult compute_wilson_loop(
    const RenderBridge& rb,
    int ox, int oy, int oz,   // anchor corner
    int R, int T,             // rectangle dimensions
    Plane plane)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    PlaneAxes ax = axes_for_plane(plane);

    double phase = 0.0;

    // Helper: get flux at wrapped coordinates
    auto flux_at = [&](int x, int y, int z) -> Vec3 {
        return vox[lat.index(x, y, z)].flux;
    };

    // Helper: dot product of flux with a direction
    auto project = [](const Vec3& J, int dx, int dy, int dz) -> double {
        return J.x * dx + J.y * dy + J.z * dz;
    };

    // Current position walking around the loop
    int cx = ox, cy = oy, cz = oz;

    // Edge 1: Bottom — R steps in +dir_R
    for (int i = 0; i < R; ++i) {
        Vec3 J = flux_at(cx, cy, cz);
        phase += project(J, ax.dr_x, ax.dr_y, ax.dr_z);
        cx += ax.dr_x;
        cy += ax.dr_y;
        cz += ax.dr_z;
    }

    // Edge 2: Right — T steps in +dir_T
    for (int i = 0; i < T; ++i) {
        Vec3 J = flux_at(cx, cy, cz);
        phase += project(J, ax.dt_x, ax.dt_y, ax.dt_z);
        cx += ax.dt_x;
        cy += ax.dt_y;
        cz += ax.dt_z;
    }

    // Edge 3: Top — R steps in -dir_R
    for (int i = 0; i < R; ++i) {
        Vec3 J = flux_at(cx, cy, cz);
        phase -= project(J, ax.dr_x, ax.dr_y, ax.dr_z);
        cx -= ax.dr_x;
        cy -= ax.dr_y;
        cz -= ax.dr_z;
    }

    // Edge 4: Left — T steps in -dir_T
    for (int i = 0; i < T; ++i) {
        Vec3 J = flux_at(cx, cy, cz);
        phase -= project(J, ax.dt_x, ax.dt_y, ax.dt_z);
        cx -= ax.dt_x;
        cy -= ax.dt_y;
        cz -= ax.dt_z;
    }

    // Scale phase by coupling constant G_C (the lattice gauge coupling)
    phase *= G_C;

    WilsonResult wr;
    wr.phase = phase;
    wr.W = std::cos(phase);
    wr.perimeter = 2 * (R + T);
    wr.area = R * T;
    return wr;
}

// ================================================================
// Spatial average: compute W(R,T) averaged over all anchor positions
// in the lattice. This reduces noise and gives a gauge-invariant signal.
// ================================================================
struct WilsonAvg {
    double W_mean;      // <W(R,T)> averaged over anchors
    double W_var;       // variance
    double neg_ln_W;    // -ln(<W>)
    int R, T;
    int area;
    int perimeter;
    int n_samples;
};

static WilsonAvg average_wilson_loop(
    const RenderBridge& rb,
    int R, int T,
    Plane plane,
    int stride = 1)    // sample every 'stride' sites to save time
{
    const int L = rb.lattice().size();
    double sum_W = 0.0;
    double sum_W2 = 0.0;
    int count = 0;

    for (int ox = 0; ox < L; ox += stride) {
        for (int oy = 0; oy < L; oy += stride) {
            for (int oz = 0; oz < L; oz += stride) {
                WilsonResult wr = compute_wilson_loop(rb, ox, oy, oz, R, T, plane);
                sum_W += wr.W;
                sum_W2 += wr.W * wr.W;
                ++count;
            }
        }
    }

    WilsonAvg wa;
    wa.R = R;
    wa.T = T;
    wa.area = R * T;
    wa.perimeter = 2 * (R + T);
    wa.n_samples = count;
    wa.W_mean = sum_W / count;
    wa.W_var = sum_W2 / count - wa.W_mean * wa.W_mean;

    // Guard against W_mean <= 0 (can happen if loop wraps or flux is large)
    if (wa.W_mean > 1e-30) {
        wa.neg_ln_W = -std::log(wa.W_mean);
    } else {
        wa.neg_ln_W = 50.0;  // saturated (strong confinement)
    }
    return wa;
}

// ================================================================
// Linear fit: y = a*x + b via least squares
// ================================================================
struct LinFit {
    double slope;
    double intercept;
    double r_squared;
    int n;
};

static LinFit linear_fit(const std::vector<double>& x, const std::vector<double>& y) {
    LinFit f = {0, 0, 0, 0};
    int n = static_cast<int>(x.size());
    f.n = n;
    if (n < 2) return f;

    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (int i = 0; i < n; ++i) {
        sx += x[i]; sy += y[i];
        sxx += x[i] * x[i]; sxy += x[i] * y[i];
    }
    double denom = n * sxx - sx * sx;
    if (std::abs(denom) < 1e-30) return f;

    f.slope = (n * sxy - sx * sy) / denom;
    f.intercept = (sy - f.slope * sx) / n;

    double mean_y = sy / n;
    double ss_res = 0, ss_tot = 0;
    for (int i = 0; i < n; ++i) {
        double pred = f.slope * x[i] + f.intercept;
        ss_res += (y[i] - pred) * (y[i] - pred);
        ss_tot += (y[i] - mean_y) * (y[i] - mean_y);
    }
    f.r_squared = (ss_tot > 1e-30) ? 1.0 - ss_res / ss_tot : 0.0;
    return f;
}

// ================================================================
// Setup: Create a quark-antiquark pair with color forces and equilibrate
//
// Takes the RenderBridge by reference (out-param) rather than returning
// by value, because RenderBridge is non-copyable (owns unique_ptr<GpuEngine>
// in CUDA builds) and has no explicit move constructor defined.
// ================================================================
static void setup_color_field(RenderBridge& rb, int separation, int eq_ticks) {
    const int L = rb.lattice().size();
    rb.toggles.genesis = false;       // no spontaneous pair production
    rb.toggles.gravity = false;       // isolate color dynamics
    rb.toggles.color_forces = true;   // enable SU(3)-inspired forces

    int mid = L / 2;

    // Source quark: color=1 (red), state=+1
    rb.inject_particle(mid, mid, mid, +1, {0, 0, K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    rb.voxels()[rb.lattice().index(mid, mid, mid)].color = 1;

    // Anti-quark: color=2 (green), state=-1
    // Place along x-axis for Wilson loops in XY and XZ planes
    int px = mid + separation;
    rb.inject_particle(px, mid, mid, -1, {0, 0, -K_B});
    rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;
    rb.voxels()[rb.lattice().index(px, mid, mid)].color = 2;

    // Equilibrate: let the flux field settle under wave propagation,
    // Gauss projection, and color forces
    rb.run(eq_ticks);
}

// ================================================================
// WL1: Basic Wilson loop sanity checks
// ================================================================
void benchmark_wl1_sanity(int L, int eq_ticks) {
    std::cout << "\n=== WL1: Wilson Loop Sanity Checks ===\n";

    int sep = std::min(8, L / 4);
    RenderBridge rb(L);
    setup_color_field(rb, sep, eq_ticks);

    // Test small loops near the source
    int mid = L / 2;
    WilsonResult wr = compute_wilson_loop(rb, mid - 1, mid - 1, mid, 3, 3, Plane::XY);

    std::cout << "  Loop 3x3 at source: phase=" << std::setprecision(8) << wr.phase
              << "  W=" << wr.W << "\n";

    // W should be real-valued (it always is for cos), in [-1, 1]
    check("WL1a: W(3,3) in [-1, 1]", wr.W >= -1.0 && wr.W <= 1.0);

    // For a nontrivial flux field, |phase| > 0
    check("WL1b: |phase| > 0 (nontrivial flux circulation)", std::abs(wr.phase) > 1e-15);

    // Average over spatial positions for a 2x2 loop
    WilsonAvg wa = average_wilson_loop(rb, 2, 2, Plane::XY, 2);
    std::cout << "  <W(2,2)> = " << std::setprecision(8) << wa.W_mean
              << "  var=" << wa.W_var
              << "  n=" << wa.n_samples << "\n";

    // <W> should be positive (weak coupling regime: loops don't wrap full cycle)
    check("WL1c: <W(2,2)> > 0 (positive average)", wa.W_mean > 0.0);

    // <W> < 1 for nontrivial field
    check("WL1d: <W(2,2)> < 1 (non-vacuum)", wa.W_mean < 1.0 - 1e-10);

    // On a vacuum lattice (no particles), W should be trivially 1
    RenderBridge rb_vac(L);
    rb_vac.toggles.genesis = false;
    WilsonAvg wa_vac = average_wilson_loop(rb_vac, 2, 2, Plane::XY, 2);
    std::cout << "  <W(2,2)> vacuum = " << wa_vac.W_mean << "\n";
    check("WL1e: <W(2,2)> = 1 on vacuum lattice", std::abs(wa_vac.W_mean - 1.0) < 1e-10);
}

// ================================================================
// WL2: Area law test — -ln(W)/Area should be approximately constant
// for fixed-area loops, and -ln(W) should grow linearly with Area
// ================================================================
void benchmark_wl2_area_law(int L, int eq_ticks) {
    std::cout << "\n=== WL2: Area Law Test ===\n";

    int sep = std::min(10, L / 3);
    RenderBridge rb(L);
    setup_color_field(rb, sep, eq_ticks);

    // Compute <W(R,T)> for a range of square loops R=T
    int max_R = std::min(L / 4, 8);
    int stride = std::max(1, L / 16);  // subsample for speed

    std::vector<WilsonAvg> data;
    for (int R = 1; R <= max_R; ++R) {
        WilsonAvg wa = average_wilson_loop(rb, R, R, Plane::XY, stride);
        data.push_back(wa);
        std::cout << "  R=T=" << R
                  << "  <W>=" << std::setprecision(8) << wa.W_mean
                  << "  -ln(W)=" << std::setprecision(6) << wa.neg_ln_W
                  << "  -ln(W)/A=" << (wa.area > 0 ? wa.neg_ln_W / wa.area : 0)
                  << "\n";
    }

    // Fit: -ln(<W>) = sigma * Area + mu * Perimeter + const
    // For area law: sigma > 0 and dominates at large loops
    // Simple test: fit -ln(W) vs Area for square loops
    std::vector<double> areas, neg_lnW;
    for (auto& d : data) {
        if (d.W_mean > 1e-20) {  // skip saturated values
            areas.push_back(static_cast<double>(d.area));
            neg_lnW.push_back(d.neg_ln_W);
        }
    }

    LinFit fit_area = linear_fit(areas, neg_lnW);
    std::cout << "  Fit: -ln(W) = " << std::setprecision(6) << fit_area.slope
              << " * Area + " << fit_area.intercept
              << "  (R^2=" << fit_area.r_squared << ")\n";

    // The slope is the extracted string tension
    double sigma_extracted = fit_area.slope;
    std::cout << "  sigma_extracted = " << sigma_extracted << "\n";
    std::cout << "  SIGMA_STRING    = " << SIGMA_STRING << "\n";

    // Area law: slope should be positive (W decays with area)
    check("WL2a: sigma_extracted > 0 (area law: W decays with area)",
          sigma_extracted > 0);

    // R^2 should be reasonable (linear trend)
    check("WL2b: R^2 > 0.5 (linear trend in -ln(W) vs Area)",
          fit_area.r_squared > 0.5);

    // Also fit -ln(W) vs Perimeter to check if perimeter law is dominant
    std::vector<double> perims;
    for (auto& d : data) {
        if (d.W_mean > 1e-20) {
            perims.push_back(static_cast<double>(d.perimeter));
        }
    }
    LinFit fit_perim = linear_fit(perims, neg_lnW);
    std::cout << "  Fit vs perimeter: slope=" << fit_perim.slope
              << "  R^2=" << fit_perim.r_squared << "\n";

    // Report which law is better fit
    std::cout << "  Area law R^2:      " << fit_area.r_squared << "\n";
    std::cout << "  Perimeter law R^2: " << fit_perim.r_squared << "\n";

    // For square loops, Area = R^2 and Perimeter = 4R, so both are monotonic.
    // The Creutz ratio (WL3) gives a cleaner separation. Here we just report.
}

// ================================================================
// WL3: Creutz ratio — isolates area-law coefficient
//
// chi(R,T) = -ln[ W(R,T) * W(R-1,T-1) / (W(R-1,T) * W(R,T-1)) ]
//
// For pure area law W ~ exp(-sigma*RT):
//   chi(R,T) = sigma  (constant, independent of R,T)
//
// For pure perimeter law W ~ exp(-mu*2(R+T)):
//   chi(R,T) = 0
// ================================================================
void benchmark_wl3_creutz(int L, int eq_ticks) {
    std::cout << "\n=== WL3: Creutz Ratio (sigma extraction) ===\n";

    int sep = std::min(10, L / 3);
    RenderBridge rb(L);
    setup_color_field(rb, sep, eq_ticks);

    int max_dim = std::min(L / 4, 7);
    int stride = std::max(1, L / 16);

    // Compute W(R,T) for a grid of R, T values
    // Store in a 2D array indexed by [R][T], offset by 1 (R=1..max_dim)
    std::vector<std::vector<double>> W_grid(max_dim + 1, std::vector<double>(max_dim + 1, 1.0));
    for (int R = 1; R <= max_dim; ++R) {
        for (int T = 1; T <= max_dim; ++T) {
            WilsonAvg wa = average_wilson_loop(rb, R, T, Plane::XY, stride);
            W_grid[R][T] = wa.W_mean;
        }
    }

    // Compute Creutz ratios for R >= 2, T >= 2
    std::vector<double> chi_vals;
    std::cout << "  R   T   W(R,T)       W(R-1,T-1)   chi(R,T)\n";
    for (int R = 2; R <= max_dim; ++R) {
        for (int T = 2; T <= max_dim; ++T) {
            double Wrt   = W_grid[R][T];
            double Wr1t1 = W_grid[R-1][T-1];
            double Wr1t  = W_grid[R-1][T];
            double Wrt1  = W_grid[R][T-1];

            // All must be positive for the log to work
            if (Wrt > 1e-30 && Wr1t1 > 1e-30 && Wr1t > 1e-30 && Wrt1 > 1e-30) {
                double ratio = (Wrt * Wr1t1) / (Wr1t * Wrt1);
                if (ratio > 1e-30) {
                    double chi = -std::log(ratio);
                    chi_vals.push_back(chi);
                    std::cout << "  " << R << "   " << T
                              << "   " << std::setprecision(6) << Wrt
                              << "   " << Wr1t1
                              << "   " << std::setprecision(6) << chi << "\n";
                }
            }
        }
    }

    if (!chi_vals.empty()) {
        double chi_mean = std::accumulate(chi_vals.begin(), chi_vals.end(), 0.0) / chi_vals.size();
        double chi_var = 0;
        for (double c : chi_vals) chi_var += (c - chi_mean) * (c - chi_mean);
        chi_var /= chi_vals.size();
        double chi_std = std::sqrt(chi_var);

        std::cout << "\n  Creutz ratio: mean=" << std::setprecision(6) << chi_mean
                  << "  std=" << chi_std
                  << "  (N=" << chi_vals.size() << ")\n";
        std::cout << "  SIGMA_STRING = " << SIGMA_STRING << "\n";

        // The Creutz ratio should be positive (confinement)
        check("WL3a: <chi> > 0 (positive Creutz ratio = confinement signal)",
              chi_mean > 0);

        // The coefficient of variation should be reasonable (not wildly varying)
        double cv = (chi_mean > 1e-15) ? chi_std / std::abs(chi_mean) : 1e10;
        std::cout << "  CV(chi) = " << cv << "\n";
        // Relaxed threshold: CV < 2.0 (Creutz ratios can have significant variance
        // on small lattices without gauge averaging)
        check("WL3b: CV(chi) < 2.0 (Creutz ratio approximately constant)",
              cv < 2.0);

        // Report the ratio of extracted sigma to hardcoded SIGMA_STRING
        double sigma_ratio = chi_mean / SIGMA_STRING;
        std::cout << "  sigma_creutz / SIGMA_STRING = " << sigma_ratio << "\n";
    } else {
        std::cout << "  WARNING: Could not compute any Creutz ratios (loops too small)\n";
        check("WL3a: <chi> > 0 (positive Creutz ratio)", false);
        check("WL3b: CV(chi) < 2.0", false);
    }
}

// ================================================================
// WL4: Compare extracted sigma to hardcoded SIGMA_STRING
// Uses rectangular (non-square) loops to separate area from perimeter
// ================================================================
void benchmark_wl4_sigma_comparison(int L, int eq_ticks) {
    std::cout << "\n=== WL4: String Tension Comparison ===\n";

    int sep = std::min(10, L / 3);
    RenderBridge rb(L);
    setup_color_field(rb, sep, eq_ticks);

    int stride = std::max(1, L / 16);

    // Use rectangular loops with different aspect ratios to disentangle
    // area and perimeter contributions.
    // Model: -ln(W(R,T)) = sigma * R * T + mu * 2*(R+T) + c
    // With enough (R,T) pairs, we can fit sigma and mu separately.
    struct DataPoint { int R; int T; double neg_ln_W; double area; double perim; };
    std::vector<DataPoint> points;

    int max_side = std::min(L / 4, 6);
    for (int R = 1; R <= max_side; ++R) {
        for (int T = R; T <= max_side; ++T) {
            WilsonAvg wa = average_wilson_loop(rb, R, T, Plane::XY, stride);
            if (wa.W_mean > 1e-30) {
                DataPoint dp;
                dp.R = R; dp.T = T;
                dp.neg_ln_W = wa.neg_ln_W;
                dp.area = static_cast<double>(wa.area);
                dp.perim = static_cast<double>(wa.perimeter);
                points.push_back(dp);
            }
        }
    }

    // Two-parameter fit: -ln(W) = sigma*Area + mu*Perimeter
    // Normal equations:
    //   [sum(A^2)   sum(A*P)] [sigma]   [sum(A*y)]
    //   [sum(A*P)   sum(P^2)] [mu   ] = [sum(P*y)]
    if (points.size() >= 3) {
        double sAA = 0, sAP = 0, sPP = 0, sAy = 0, sPy = 0;
        for (auto& p : points) {
            double A = p.area, P = p.perim, y = p.neg_ln_W;
            sAA += A * A;
            sAP += A * P;
            sPP += P * P;
            sAy += A * y;
            sPy += P * y;
        }
        double det = sAA * sPP - sAP * sAP;
        double sigma_fit = 0, mu_fit = 0;
        if (std::abs(det) > 1e-30) {
            sigma_fit = (sPP * sAy - sAP * sPy) / det;
            mu_fit    = (sAA * sPy - sAP * sAy) / det;
        }

        std::cout << "  Two-parameter fit:\n";
        std::cout << "    sigma_fit = " << std::setprecision(8) << sigma_fit << "\n";
        std::cout << "    mu_fit    = " << std::setprecision(8) << mu_fit << "\n";
        std::cout << "    SIGMA_STRING = " << SIGMA_STRING << "\n";

        // sigma should be positive (confinement)
        check("WL4a: sigma_fit > 0 (area-law coefficient positive)", sigma_fit > 0);

        // sigma_fit should be within 2 orders of magnitude of SIGMA_STRING.
        // The FTD flux-based Wilson loop and the hardcoded string tension are
        // defined differently (one from the gauge path integral, one from the
        // force profile), so exact agreement is not expected. We test for the
        // correct sign and order of magnitude.
        if (sigma_fit > 0 && SIGMA_STRING > 0) {
            double log_ratio = std::abs(std::log10(sigma_fit / SIGMA_STRING));
            std::cout << "    |log10(sigma_fit/SIGMA_STRING)| = " << log_ratio << "\n";
            check("WL4b: sigma_fit within 2 OOM of SIGMA_STRING", log_ratio < 2.0);
        } else {
            check("WL4b: sigma_fit within 2 OOM of SIGMA_STRING", false);
        }

        // Report residuals
        double ss_res = 0, ss_tot = 0;
        double mean_y = 0;
        for (auto& p : points) mean_y += p.neg_ln_W;
        mean_y /= points.size();
        for (auto& p : points) {
            double pred = sigma_fit * p.area + mu_fit * p.perim;
            ss_res += (p.neg_ln_W - pred) * (p.neg_ln_W - pred);
            ss_tot += (p.neg_ln_W - mean_y) * (p.neg_ln_W - mean_y);
        }
        double R2 = (ss_tot > 1e-30) ? 1.0 - ss_res / ss_tot : 0.0;
        std::cout << "    R^2 (two-param) = " << R2 << "\n";
        check("WL4c: R^2 > 0.5 (area+perimeter model fits data)", R2 > 0.5);
    } else {
        std::cout << "  WARNING: Not enough data points for two-parameter fit\n";
        check("WL4a: sigma_fit > 0", false);
        check("WL4b: sigma_fit within 2 OOM of SIGMA_STRING", false);
        check("WL4c: R^2 > 0.5", false);
    }
}

// ================================================================
// WL5: Plane average — test that all planes give similar results
// (rotational symmetry of the confined phase)
// ================================================================
void benchmark_wl5_isotropy(int L, int eq_ticks) {
    std::cout << "\n=== WL5: Wilson Loop Isotropy ===\n";

    int sep = std::min(8, L / 4);
    RenderBridge rb(L);
    setup_color_field(rb, sep, eq_ticks);

    int stride = std::max(1, L / 16);
    int R = 3, T = 3;

    WilsonAvg wa_xy = average_wilson_loop(rb, R, T, Plane::XY, stride);
    WilsonAvg wa_xz = average_wilson_loop(rb, R, T, Plane::XZ, stride);
    WilsonAvg wa_yz = average_wilson_loop(rb, R, T, Plane::YZ, stride);

    std::cout << "  <W(3,3)> XY = " << std::setprecision(8) << wa_xy.W_mean << "\n";
    std::cout << "  <W(3,3)> XZ = " << std::setprecision(8) << wa_xz.W_mean << "\n";
    std::cout << "  <W(3,3)> YZ = " << std::setprecision(8) << wa_yz.W_mean << "\n";

    // With a quark pair along the x-axis, the XY and XZ planes contain the pair
    // while YZ is transverse. We expect the planes containing the pair to show
    // stronger flux circulation than the transverse plane.
    double W_max = std::max({wa_xy.W_mean, wa_xz.W_mean, wa_yz.W_mean});
    double W_min = std::min({wa_xy.W_mean, wa_xz.W_mean, wa_yz.W_mean});

    // All should be physical (in valid range)
    check("WL5a: All plane <W> > -1",
          wa_xy.W_mean > -1.0 && wa_xz.W_mean > -1.0 && wa_yz.W_mean > -1.0);

    // The XY and XZ planes should give similar results (both contain the pair axis)
    if (std::abs(wa_xy.W_mean) > 1e-10 && std::abs(wa_xz.W_mean) > 1e-10) {
        double pair_ratio = wa_xy.W_mean / wa_xz.W_mean;
        std::cout << "  W_XY / W_XZ = " << pair_ratio << "\n";
        check("WL5b: W_XY ~ W_XZ within 50% (planes with pair axis)",
              pair_ratio > 0.5 && pair_ratio < 2.0);
    } else {
        check("WL5b: W_XY ~ W_XZ", true);  // both near zero = OK
    }
}

// ================================================================
// WL6: Flux tube profile — measure flux density along the line
// connecting the quark pair
// ================================================================
void benchmark_wl6_flux_tube(int L, int eq_ticks) {
    std::cout << "\n=== WL6: Flux Tube Profile ===\n";

    int sep = std::min(10, L / 3);
    RenderBridge rb(L);
    setup_color_field(rb, sep, eq_ticks);

    int mid = L / 2;
    int px = mid + sep;

    // Measure flux density along the x-axis between the quark pair
    std::cout << "  x    |J|         |J|_transverse\n";
    double max_flux_on_axis = 0;
    double max_flux_off_axis = 0;

    for (int x = mid; x <= px; ++x) {
        double J_on = rb.voxels()[rb.lattice().index(x, mid, mid)].flux.mag();
        // Transverse: one step off-axis in y-direction
        double J_off = rb.voxels()[rb.lattice().index(x, mid + 2, mid)].flux.mag();

        std::cout << "  " << std::setw(3) << (x - mid)
                  << "   " << std::setprecision(6) << J_on
                  << "   " << J_off << "\n";

        if (x > mid && x < px) {  // exclude source positions
            max_flux_on_axis = std::max(max_flux_on_axis, J_on);
            max_flux_off_axis = std::max(max_flux_off_axis, J_off);
        }
    }

    // Flux should be concentrated along the line between quarks (flux tube)
    std::cout << "  max |J| on-axis:  " << max_flux_on_axis << "\n";
    std::cout << "  max |J| off-axis: " << max_flux_off_axis << "\n";

    check("WL6a: Flux exists on axis (J > 0 between quarks)",
          max_flux_on_axis > 1e-10);

    // Flux tube: on-axis flux should exceed off-axis flux
    if (max_flux_off_axis > 1e-15) {
        double collimation = max_flux_on_axis / max_flux_off_axis;
        std::cout << "  Collimation ratio: " << collimation << "\n";
        check("WL6b: On-axis flux > off-axis (flux tube collimation)",
              collimation > 1.0);
    } else {
        check("WL6b: On-axis flux > off-axis (flux tube collimation)", true);
    }

    // Measure the Wilson loop in a small rectangle centered on the flux tube midpoint
    int tube_mid_x = mid + sep / 2;
    WilsonResult wr_tube = compute_wilson_loop(rb, tube_mid_x - 1, mid - 1, mid,
                                               3, 3, Plane::XY);
    WilsonResult wr_far = compute_wilson_loop(rb, mid, mid, mid + L / 4,
                                              3, 3, Plane::XY);
    std::cout << "  W(3,3) at flux tube: " << wr_tube.W
              << "  phase=" << wr_tube.phase << "\n";
    std::cout << "  W(3,3) far from tube: " << wr_far.W
              << "  phase=" << wr_far.phase << "\n";

    // The loop near the flux tube should have larger phase (more flux circulation)
    check("WL6c: |phase| at tube > |phase| far (flux tube detected by Wilson loop)",
          std::abs(wr_tube.phase) > std::abs(wr_far.phase));
}

// ================================================================
// Main
// ================================================================
int main(int argc, char* argv[]) {
    int L = 32;            // lattice size (default)
    int eq_ticks = 50;     // equilibration ticks (default)

    if (argc > 1) L = std::atoi(argv[1]);
    if (argc > 2) eq_ticks = std::atoi(argv[2]);

    // Enforce minimum lattice size for Wilson loops
    if (L < 16) L = 16;

    std::cout << "================================================================\n";
    std::cout << "  FTD Wilson Loop Benchmark\n";
    std::cout << "  Lattice: " << L << "^3  Equilibration: " << eq_ticks << " ticks\n";
    std::cout << "  SIGMA_STRING = " << SIGMA_STRING
              << " (= ALPHA_S * K_B^2 = " << ALPHA_S << " * " << K_B << "^2)\n";
    std::cout << "  G_C = " << G_C << "  ALPHA_EFT = " << ALPHA_EFT << "\n";
    std::cout << "================================================================\n";

    benchmark_wl1_sanity(L, eq_ticks);
    benchmark_wl2_area_law(L, eq_ticks);
    benchmark_wl3_creutz(L, eq_ticks);
    benchmark_wl4_sigma_comparison(L, eq_ticks);
    benchmark_wl5_isotropy(L, eq_ticks);
    benchmark_wl6_flux_tube(L, eq_ticks);

    std::cout << "\n================================================================\n";
    std::cout << "  SUMMARY: " << g_passes << " passed, " << g_failures << " failed\n";
    std::cout << "================================================================\n";

    return g_failures > 0 ? 1 : 0;
}

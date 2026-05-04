/**
 * Phase B.3 (γ''): characterize the A=10 soliton dynamics.
 *
 * Prior finding (test_cluster_a10_centroid_drift.cpp): A=10·K_GENESIS
 * produces a propagating soliton with constant matter (15 voxels) and
 * directed motion (~0.03 voxels/tick in +x direction over 200 ticks).
 *
 * This test characterizes:
 *   1. Velocity: precise centroid-drift rate via linear fit
 *   2. Direction selection: does injection in different directions produce
 *      solitons moving in those directions?
 *   3. Amplitude dependence: does velocity vary with A?
 *   4. Conservation: are total_manifested, total flux energy, total wave
 *      velocity energy conserved over the trajectory?
 *
 * Output: dimensionless soliton properties (v in units of c_lat = 1/sqrt(3),
 * conserved quantities in lattice units). Ratios and conservation laws are
 * the load-bearing observables.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

static const double TWOPI = 2.0 * 3.14159265358979323846;
static const double C_LAT = 1.0 / std::sqrt(3.0);  // CFL stability speed

struct Snapshot {
    int tick;
    int n_total;
    double cx, cy, cz;
    double total_flux_energy;       // sum |J|^2
    double total_wave_energy;       // sum |wave_vel|^2
    double total_J_x;               // net x-momentum proxy
    double total_J_y;
    double total_J_z;
};

static Snapshot snap(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int L = lat.size();
    const int64_t total = lat.total_sites();
    Snapshot s;
    s.tick = rb.current_tick();
    s.n_total = 0;
    s.cx = s.cy = s.cz = 0;
    s.total_flux_energy = s.total_wave_energy = 0;
    s.total_J_x = s.total_J_y = s.total_J_z = 0;

    double sx_x = 0, cx_x = 0, sx_y = 0, cx_y = 0, sx_z = 0, cx_z = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        ++s.n_total;
        auto c = lat.coord(static_cast<int>(i));
        sx_x += std::sin(TWOPI * c.x / L); cx_x += std::cos(TWOPI * c.x / L);
        sx_y += std::sin(TWOPI * c.y / L); cx_y += std::cos(TWOPI * c.y / L);
        sx_z += std::sin(TWOPI * c.z / L); cx_z += std::cos(TWOPI * c.z / L);
        s.total_flux_energy += vox[i].flux.mag2();
        s.total_wave_energy += vox[i].wave_vel.mag2();
        s.total_J_x += vox[i].flux.x;
        s.total_J_y += vox[i].flux.y;
        s.total_J_z += vox[i].flux.z;
    }
    if (s.n_total > 0) {
        s.cx = std::atan2(sx_x, cx_x) * L / TWOPI; if (s.cx < 0) s.cx += L;
        s.cy = std::atan2(sx_y, cx_y) * L / TWOPI; if (s.cy < 0) s.cy += L;
        s.cz = std::atan2(sx_z, cx_z) * L / TWOPI; if (s.cz < 0) s.cz += L;
    }
    return s;
}

struct Result {
    double A_over_KG;
    double Jx, Jy, Jz;  // injection direction
    double v_x, v_y, v_z;
    double speed;
    double speed_over_clat;
    double initial_n;
    double final_n;
    double initial_flux_E;
    double final_flux_E;
    double initial_wave_E;
    double final_wave_E;
    double n_conservation_pct;
    double flux_E_conservation_pct;
};

static Result run_one(double A_over_KG, double dirx, double diry, double dirz) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 300;
    const int SAMPLE = 5;
    const int inj = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.langevin_seed = 1;
    double mag = std::sqrt(dirx*dirx + diry*diry + dirz*dirz);
    rb.inject_flux(inj, inj, inj,
                   {A_over_KG * ftd::K_GENESIS * dirx / mag,
                    A_over_KG * ftd::K_GENESIS * diry / mag,
                    A_over_KG * ftd::K_GENESIS * dirz / mag});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    auto wrap = [L](double d) { if (d > L/2.0) d -= L; if (d < -L/2.0) d += L; return d; };

    Snapshot s0 = snap(rb);
    double cx0 = s0.cx, cy0 = s0.cy, cz0 = s0.cz;

    // Linear fit: collect (t, dx, dy, dz)
    std::vector<double> ts, dxs, dys, dzs;
    for (int t = 0; t <= N_TRACE; t += SAMPLE) {
        if (t > 0) for (int s = 0; s < SAMPLE; ++s) rb.tick();
        Snapshot ss = snap(rb);
        if (ss.n_total == 0) break;
        ts.push_back(t);
        dxs.push_back(wrap(ss.cx - cx0));
        dys.push_back(wrap(ss.cy - cy0));
        dzs.push_back(wrap(ss.cz - cz0));
    }

    Snapshot sf = snap(rb);

    // Linear fit slope = velocity
    auto slope = [&](const std::vector<double>& y) {
        double n = ts.size();
        double sum_t = 0, sum_y = 0, sum_ty = 0, sum_t2 = 0;
        for (size_t i = 0; i < ts.size(); ++i) {
            sum_t += ts[i]; sum_y += y[i];
            sum_ty += ts[i] * y[i]; sum_t2 += ts[i] * ts[i];
        }
        return (n * sum_ty - sum_t * sum_y) / (n * sum_t2 - sum_t * sum_t);
    };

    Result r;
    r.A_over_KG = A_over_KG;
    r.Jx = dirx / mag; r.Jy = diry / mag; r.Jz = dirz / mag;
    r.v_x = slope(dxs);
    r.v_y = slope(dys);
    r.v_z = slope(dzs);
    r.speed = std::sqrt(r.v_x*r.v_x + r.v_y*r.v_y + r.v_z*r.v_z);
    r.speed_over_clat = r.speed / C_LAT;
    r.initial_n = s0.n_total;
    r.final_n = sf.n_total;
    r.initial_flux_E = s0.total_flux_energy;
    r.final_flux_E = sf.total_flux_energy;
    r.initial_wave_E = s0.total_wave_energy;
    r.final_wave_E = sf.total_wave_energy;
    r.n_conservation_pct = (r.final_n / r.initial_n - 1) * 100;
    r.flux_E_conservation_pct = (r.final_flux_E / r.initial_flux_E - 1) * 100;
    return r;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (γ''): A=10 soliton characterization\n";
    std::cout << "================================================================\n\n";

    std::cout << "  c_lat = 1/sqrt(3) ≈ " << std::fixed << std::setprecision(4) << C_LAT
              << " voxels/tick (CFL stability speed; engine 'speed of light')\n\n";

    struct Cfg { double A; double dx, dy, dz; const char* label; };
    std::vector<Cfg> cfgs = {
        {10.0,  1, 0, 0, "A=10 inj +x"},
        {10.0,  0, 1, 0, "A=10 inj +y"},
        {10.0,  0, 0, 1, "A=10 inj +z"},
        {10.0, -1, 0, 0, "A=10 inj -x"},
        {10.0,  1, 1, 0, "A=10 inj +xy"},
        {10.0,  1, 1, 1, "A=10 inj +xyz"},
        {11.0,  1, 0, 0, "A=11 inj +x (other death amp)"},
        { 6.0,  1, 0, 0, "A=6  inj +x (sub-critical control)"},
    };

    std::cout << std::left << std::setw(38) << "config" << std::right
              << std::setw(8) << "n_init"
              << std::setw(8) << "n_final"
              << std::setw(8) << "n_dN%"
              << std::setw(8) << "v_x"
              << std::setw(8) << "v_y"
              << std::setw(8) << "v_z"
              << std::setw(9) << "|v|"
              << std::setw(10) << "v/c_lat"
              << std::setw(11) << "fluxE_dE%"
              << "\n";
    std::cout << "------------------------------------- ------- ------- ------- ------- ------- ------- -------- --------- ----------\n";

    std::vector<Result> results;
    for (const auto& c : cfgs) {
        Result r = run_one(c.A, c.dx, c.dy, c.dz);
        results.push_back(r);
        std::cout << std::left << std::setw(38) << c.label << std::right
                  << std::setw(8) << static_cast<int>(r.initial_n)
                  << std::setw(8) << static_cast<int>(r.final_n)
                  << std::setw(7) << std::fixed << std::setprecision(1) << r.n_conservation_pct << "%"
                  << std::setw(8) << std::setprecision(4) << r.v_x
                  << std::setw(8) << std::setprecision(4) << r.v_y
                  << std::setw(8) << std::setprecision(4) << r.v_z
                  << std::setw(9) << std::setprecision(4) << r.speed
                  << std::setw(10) << std::setprecision(4) << r.speed_over_clat
                  << std::setw(10) << std::setprecision(2) << r.flux_E_conservation_pct << "%"
                  << "\n";
    }

    // Verdict
    std::cout << "\n--- Soliton characterization findings ---\n";

    // (1) Direction selection: does v_i correlate with injected J_i?
    bool direction_aligns = true;
    for (size_t i = 0; i < 6; ++i) {  // first 6 are A=10 with various dirs
        const auto& r = results[i];
        // Velocity direction should align with injection direction
        double dot = r.v_x * r.Jx + r.v_y * r.Jy + r.v_z * r.Jz;
        if (std::abs(r.speed) > 0.001 && dot < 0) direction_aligns = false;
    }
    std::cout << "  (1) Velocity direction tracks injection direction: "
              << (direction_aligns ? "YES" : "no") << "\n";

    // (2) Direction-isotropy: speed should be similar across orientations
    double speed_min = 1e9, speed_max = 0;
    for (size_t i = 0; i < 4; ++i) {  // axial directions
        if (results[i].speed > speed_max) speed_max = results[i].speed;
        if (results[i].speed < speed_min) speed_min = results[i].speed;
    }
    double iso_ratio = (speed_min > 0) ? speed_max / speed_min : -1;
    std::cout << "  (2) Speed isotropy across axial dirs: ratio max/min = "
              << std::fixed << std::setprecision(2) << iso_ratio
              << (iso_ratio < 1.5 ? "  (consistent)" : "  (anisotropic — lattice-direction dependence)")
              << "\n";

    // (3) Conservation: matter and flux energy
    double max_n_drift = 0;
    for (const auto& r : results) {
        if (std::abs(r.n_conservation_pct) > max_n_drift)
            max_n_drift = std::abs(r.n_conservation_pct);
    }
    std::cout << "  (3) Matter conservation max drift: "
              << std::fixed << std::setprecision(1) << max_n_drift << "%\n";

    // (4) Speed vs amplitude
    if (results.size() >= 7) {
        std::cout << "  (4) Speed scaling: A=10 |v|=" << std::setprecision(4) << results[0].speed
                  << " vs A=11 |v|=" << results[6].speed << "\n";
    }

    std::cout << "\n  [SUMMARY]\n";
    std::cout << "  - Soliton velocity ~" << std::setprecision(4) << results[0].speed
              << " voxels/tick = " << results[0].speed_over_clat << " · c_lat\n";
    std::cout << "  - Direction selection: " << (direction_aligns ? "tracks injection" : "no clear pattern") << "\n";
    std::cout << "  - Matter conservation within " << std::setprecision(0) << max_n_drift << "% across all runs\n";

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (single-seed soliton characterization)\n";
    std::cout << "================================================================\n";
    return 0;
}

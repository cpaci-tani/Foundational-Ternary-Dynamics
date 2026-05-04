/**
 * Phase B.3 (β') refinement: is A=10 cluster decay actually CLUSTER DRIFT?
 *
 * The cascade trace (test_cluster_a10_cascade_trace.cpp) revealed that as
 * the original-mask voxels disappear, an equal number of NEW manifested
 * voxels appear OUTSIDE the original mask — total manifested-matter count
 * stays roughly constant (14 → 15 over 133 ticks).
 *
 * Hypothesis: the A=10 cluster doesn't dissolve; it MIGRATES. The mask-
 * persistence metric was biased against moving clusters.
 *
 * Test: track the CENTROID of all manifested voxels (regardless of mask
 * membership) tick-by-tick. If the centroid drifts away from the injection
 * point, the cluster is moving, not dying. If the centroid stays at the
 * injection point while manifested-voxel count drops, the cluster is dying.
 *
 * Compare A=10 (death-amp) vs A=8 (L-invariant stable) for control.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct CentroidSample {
    int tick;
    int total_manifested;
    double cx, cy, cz;        // centroid
    double dist_from_injection;
    double rms_radius;        // RMS distance of manifested voxels from centroid
};

static CentroidSample compute_centroid(const ftd::RenderBridge& rb,
                                        int inj_x, int inj_y, int inj_z) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    const int L = lat.size();

    CentroidSample s;
    s.tick = rb.current_tick();
    s.total_manifested = 0;
    s.cx = s.cy = s.cz = 0;

    // Periodic-aware centroid: use angular method (Hutton & Carter 1986)
    // to handle periodic boundary correctly. For each axis, compute mean
    // angle on the circle.
    double sum_sin_x = 0, sum_cos_x = 0;
    double sum_sin_y = 0, sum_cos_y = 0;
    double sum_sin_z = 0, sum_cos_z = 0;
    const double TWOPI = 2.0 * 3.14159265358979323846;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        ++s.total_manifested;
        auto c = lat.coord(static_cast<int>(i));
        sum_sin_x += std::sin(TWOPI * c.x / L); sum_cos_x += std::cos(TWOPI * c.x / L);
        sum_sin_y += std::sin(TWOPI * c.y / L); sum_cos_y += std::cos(TWOPI * c.y / L);
        sum_sin_z += std::sin(TWOPI * c.z / L); sum_cos_z += std::cos(TWOPI * c.z / L);
    }
    if (s.total_manifested == 0) {
        s.dist_from_injection = 0;
        s.rms_radius = 0;
        return s;
    }
    s.cx = std::atan2(sum_sin_x, sum_cos_x) * L / TWOPI;
    s.cy = std::atan2(sum_sin_y, sum_cos_y) * L / TWOPI;
    s.cz = std::atan2(sum_sin_z, sum_cos_z) * L / TWOPI;
    if (s.cx < 0) s.cx += L;
    if (s.cy < 0) s.cy += L;
    if (s.cz < 0) s.cz += L;

    // Distance from injection (with periodic wrapping)
    auto wrap = [L](double d) {
        if (d > L/2.0) d -= L;
        if (d < -L/2.0) d += L;
        return d;
    };
    double dx = wrap(s.cx - inj_x);
    double dy = wrap(s.cy - inj_y);
    double dz = wrap(s.cz - inj_z);
    s.dist_from_injection = std::sqrt(dx*dx + dy*dy + dz*dz);

    // RMS radius (with periodic wrapping)
    double rms = 0;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state == 0) continue;
        auto c = lat.coord(static_cast<int>(i));
        double dxi = wrap(c.x - s.cx);
        double dyi = wrap(c.y - s.cy);
        double dzi = wrap(c.z - s.cz);
        rms += dxi*dxi + dyi*dyi + dzi*dzi;
    }
    s.rms_radius = std::sqrt(rms / s.total_manifested);
    return s;
}

static void run_one(double A_over_KG, const char* label) {
    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 200;
    const int SAMPLE = 5;

    ftd::RenderBridge rb(L);
    rb.toggles.langevin_seed = 1;
    const int c = L / 2;
    rb.inject_flux(c, c, c, {A_over_KG * ftd::K_GENESIS, 0.0, 0.0});
    for (int t = 0; t < N_WARMUP; ++t) rb.tick();

    std::cout << "\n--- " << label << " (A = " << std::fixed << std::setprecision(1)
              << A_over_KG << " · K_GENESIS) ---\n";
    std::cout << "tick    n_total   centroid_x  centroid_y  centroid_z   d_from_inj   rms_radius\n";
    std::cout << "----    -------   ----------  ----------  ----------   ----------   ----------\n";

    for (int t = 0; t <= N_TRACE; t += SAMPLE) {
        if (t > 0) for (int s = 0; s < SAMPLE; ++s) rb.tick();
        auto s = compute_centroid(rb, c, c, c);
        std::cout << std::setw(4) << t << "    "
                  << std::setw(7) << s.total_manifested << "   "
                  << std::fixed << std::setprecision(3) << std::setw(10) << s.cx << "  "
                  << std::setw(10) << s.cy << "  "
                  << std::setw(10) << s.cz << "   "
                  << std::setw(10) << s.dist_from_injection << "   "
                  << std::setw(10) << s.rms_radius << "\n";
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (β'): centroid drift comparison A=10 vs A=8\n";
    std::cout << "================================================================\n\n";
    std::cout << "Hypothesis: A=10 cluster MIGRATES; A=8 cluster STAYS PUT.\n";
    std::cout << "If centroid drifts at A=10 while total_manifested stays roughly constant,\n";
    std::cout << "the 'death' was actually translation. If total_manifested drops and\n";
    std::cout << "centroid stays put, it was true decay.\n";

    run_one(8.0,  "CONTROL: A=8.0 (L-invariant stable)");
    run_one(10.0, "DEATH-AMP: A=10.0 (universal death amplitude)");

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (single-seed centroid drift comparison)\n";
    std::cout << "================================================================\n";
    return 0;
}

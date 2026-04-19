/**
 * @file benchmark_rutherford_alpha.cpp
 * @brief Thread 4 of the EFT Day-2 program — Rutherford scattering α extraction.
 *
 * Motivation
 * ----------
 * Phases 2 and 4C measure α_eff via the *static* two-charge interaction
 * potential V(r). That method has a known issue: locked charges impose
 * boundary conditions on the flux field that may not match how a
 * genuine scattering process would develop. Result: the extracted α is
 * contaminated by the locked-boundary artefact and comes out 4-16×
 * larger than α_ref depending on method.
 *
 * This benchmark bypasses locked charges entirely. Procedure:
 *   1. Fix one +1 target at the lattice centre (locked).
 *   2. Inject a +1 projectile at impact parameter b along the y-axis,
 *      moving in +x direction with initial momentum p.
 *   3. Run dynamics for long enough that the projectile passes through
 *      the target's Coulomb field and exits on the other side.
 *   4. Measure final velocity direction → deflection angle θ.
 *   5. Fit Rutherford: tan(θ/2) = α / (2 · T · b), with T = kinetic
 *      energy at closest approach, solve for α.
 *
 * For multiple (b, p) pairs, we fit a single α from the ensemble.
 *
 * Caveats
 * -------
 * - The engine's movement phase doesn't implement strict Newtonian motion
 *   on continuous coordinates; particles hop between voxels on an integer
 *   grid. Rutherford's angle-vs-impact-parameter law is approximate on a
 *   discrete lattice.
 * - Both charges are +1 (repulsive), so the projectile deflects away
 *   from the target. Symmetric geometry avoids bound-state confusion.
 * - At small impact parameters the discrete-grid resolution dominates
 *   the angle measurement; we use b ∈ {3, 4, 5, 6, 7} as the usable
 *   range on L = 32.
 *
 * Output: CSV of (b, p, theta, alpha_extracted). Report tex/md with
 * summary statistics + comparison to α_ref.
 */

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"

// Configure lattice for scattering: Coulomb + movement ON, everything else OFF.
static void configure_scattering(ftd::RenderBridge& rb) {
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.damping = false;
    rb.toggles.selective_damping = false;
    rb.toggles.genesis = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.forces = true;       // enable Coulomb force
    rb.toggles.poisson_coulomb = true;
    rb.toggles.gravity = false;
    rb.toggles.movement = true;     // particles can move
    rb.toggles.lorentz_force = false;
    rb.toggles.color_forces = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.strong_force = false;
    rb.toggles.triad_binding = false;
    rb.toggles.pair_production = false;
    rb.toggles.exchange_force = false;
    rb.toggles.latency_field = false;
    rb.toggles.emergent_forces = false;
}

// Locate the projectile (the non-locked +1 particle) by scanning for
// state=+1 that is NOT at the target cell. Returns (x, y, z, velocity) or
// nothing if the projectile was absorbed/lost.
struct ProjectileSnap {
    bool found = false;
    int x = 0, y = 0, z = 0;
    ftd::Vec3 velocity{0, 0, 0};
};

static ProjectileSnap locate_projectile(const ftd::RenderBridge& rb,
                                        int target_x, int target_y, int target_z) {
    const auto& vox = rb.voxels();
    const int L = rb.lattice().size();
    ProjectileSnap snap;
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const int idx = rb.lattice().index(x, y, z);
                if (vox[idx].state != +1) continue;
                if (x == target_x && y == target_y && z == target_z) continue;
                snap.found = true;
                snap.x = x; snap.y = y; snap.z = z;
                snap.velocity = vox[idx].velocity;
                return snap;
            }
    return snap;
}

struct ScatterEvent {
    int impact_param = 0;
    double initial_vx = 0.0;
    double initial_vy = 0.0;
    double final_vx = 0.0;
    double final_vy = 0.0;
    double theta = 0.0;
    int ticks_run = 0;
    bool valid = false;
};

static ScatterEvent run_scattering_trial(int L, int impact_param, double v0,
                                         int n_ticks) {
    ScatterEvent ev;
    ev.impact_param = impact_param;
    ev.initial_vx = v0;
    ev.initial_vy = 0.0;

    ftd::RenderBridge rb(L);
    configure_scattering(rb);

    const int tx = L / 2;
    const int ty = L / 2;
    const int tz = L / 2;

    // Locked +1 target at lattice centre
    rb.inject_particle(tx, ty, tz, +1, {0.0, 0.0, 0.0});
    rb.voxels()[rb.lattice().index(tx, ty, tz)].locked = true;

    // Projectile: +1 at (tx - L/4, ty + b, tz) with velocity (+v0, 0, 0)
    const int px = tx - L / 4;
    const int py = ty + impact_param;
    const int pz = tz;
    rb.inject_particle(px, py, pz, +1, {v0 * 0.1, 0.0, 0.0});
    // Set velocity field
    {
        auto& vox = rb.voxels()[rb.lattice().index(px, py, pz)];
        vox.velocity = ftd::Vec3{v0, 0.0, 0.0};
    }

    // Run dynamics
    for (int t = 0; t < n_ticks; ++t) {
        rb.tick();
    }
    ev.ticks_run = n_ticks;

    // Find projectile position + velocity
    auto snap = locate_projectile(rb, tx, ty, tz);
    if (!snap.found) return ev;
    ev.final_vx = snap.velocity.x;
    ev.final_vy = snap.velocity.y;
    // Deflection: angle between initial and final velocity
    const double num = ev.initial_vx * ev.final_vx + ev.initial_vy * ev.final_vy;
    const double norm1 = std::sqrt(ev.initial_vx*ev.initial_vx + ev.initial_vy*ev.initial_vy);
    const double norm2 = std::sqrt(ev.final_vx*ev.final_vx + ev.final_vy*ev.final_vy);
    if (norm1 < 1e-12 || norm2 < 1e-12) return ev;
    const double cos_theta = num / (norm1 * norm2);
    const double clamped = std::max(-1.0, std::min(1.0, cos_theta));
    ev.theta = std::acos(clamped);
    ev.valid = true;
    return ev;
}

int main(int argc, char** argv) {
    constexpr double PI = 3.14159265358979323846;
    const int L = 32;
    const int n_ticks = 400;
    const double v0 = 0.3;
    std::vector<int> impacts = {3, 4, 5, 6, 7, 8};

    bool quick = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--quick") { quick = true; impacts = {4, 6}; }
    }

    std::cerr << "================================================================\n";
    std::cerr << "  Rutherford Scattering α Extraction (Day-2 Thread 4)\n";
    std::cerr << "  L = " << L << "  v0 = " << v0 << "  n_ticks = " << n_ticks << "\n";
    std::cerr << "================================================================\n";

    std::cout << "impact_param,v0,initial_vx,final_vx,final_vy,theta_rad,alpha_extracted\n";

    std::vector<std::pair<int, double>> alpha_samples;

    for (int b : impacts) {
        std::cerr << "\n-- b = " << b << " --\n";
        auto ev = run_scattering_trial(L, b, v0, n_ticks);
        if (!ev.valid) {
            std::cerr << "  PROJECTILE LOST (absorbed or escaped). Skipping.\n";
            continue;
        }
        std::cerr << "  final velocity: (" << ev.final_vx << ", " << ev.final_vy << ")\n";
        std::cerr << "  theta = " << ev.theta << " rad = " << (ev.theta * 180.0 / PI) << " deg\n";

        // Rutherford: tan(θ/2) = α · Z_target · Z_proj · k_Coulomb / (2 · T_kin · b)
        // With Z_target = Z_proj = 1, k_Coulomb = 1/(4π):
        //    α = 2 · T · b · tan(θ/2) · 4π
        // where T = (1/2) m v²; for unit-mass lattice particles T ≈ v²/2.
        const double T_kin = 0.5 * v0 * v0;
        const double alpha_extracted =
            2.0 * T_kin * b * std::tan(0.5 * ev.theta) * 4.0 * PI;
        std::cerr << "  T_kin = " << T_kin << "  α_extracted = " << alpha_extracted
                  << "  (ratio to α_ref=" << ftd::ALPHA << ": "
                  << (alpha_extracted / ftd::ALPHA) << ")\n";

        std::cout << b << "," << v0 << "," << ev.initial_vx << ","
                  << ev.final_vx << "," << ev.final_vy << ","
                  << std::setprecision(10) << ev.theta << ","
                  << std::setprecision(10) << alpha_extracted << "\n";
        alpha_samples.push_back({b, alpha_extracted});
    }

    if (!alpha_samples.empty()) {
        double sum = 0.0;
        for (auto& p : alpha_samples) sum += p.second;
        const double mean_alpha = sum / alpha_samples.size();
        double var = 0.0;
        for (auto& p : alpha_samples) var += (p.second - mean_alpha) * (p.second - mean_alpha);
        const double std_alpha = std::sqrt(var / alpha_samples.size());
        std::cerr << "\n================================================================\n";
        std::cerr << "  α_mean = " << mean_alpha
                  << "  ± " << std_alpha
                  << "  (ratio to α_ref: " << (mean_alpha / ftd::ALPHA) << ")\n";
        std::cerr << "================================================================\n";
        std::cout << "SUMMARY,n_samples=" << alpha_samples.size() << ",mean="
                  << std::setprecision(10) << mean_alpha << ",std="
                  << std::setprecision(10) << std_alpha << ",ratio="
                  << std::setprecision(6) << (mean_alpha / ftd::ALPHA) << "\n";
    }
    return 0;
}

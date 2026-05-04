/**
 * Phase B.3 (β'): instrument the A=10·K_GENESIS cascade collapse mechanism.
 *
 * A=10 is the universal-death amplitude — clusters die deterministically at
 * tick 135 (L=32) / tick 160 (L=64) across all tested seeds. The decay is
 * NOT stochastic. Some specific cluster-geometry event triggers a cascade
 * that systematically collapses the cluster.
 *
 * This test traces what happens TICK-BY-TICK: how does the manifested-mask
 * voxel set evolve, and what specific event triggers the cascade?
 *
 * Per-tick observables (logged every tick during the cascade window):
 *   - persistence: |M_t ∩ M_0| / |M_0| (mask survival)
 *   - n_plus_in_mask, n_minus_in_mask: state distribution within original mask
 *   - n_void_in_mask: voxels that returned to s=0
 *   - mean_flux_mag, mean_wave_vel: dynamical quantities
 *   - n_plus_outside, n_minus_outside: spread beyond original mask
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_set>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct Snapshot {
    int tick;
    int n_plus_in_mask;
    int n_minus_in_mask;
    int n_void_in_mask;
    int n_plus_outside;
    int n_minus_outside;
    double mean_flux_mag_in_mask;
    double mean_wave_vel_mag_in_mask;
    double max_flux_mag_in_mask;
    double persistence;
};

static std::unordered_set<int> snapshot_mask(const ftd::RenderBridge& rb) {
    std::unordered_set<int> mask;
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    for (int64_t i = 0; i < total; ++i)
        if (vox[i].state != 0) mask.insert(static_cast<int>(i));
    return mask;
}

static Snapshot snapshot_state(const ftd::RenderBridge& rb,
                                const std::unordered_set<int>& mask) {
    Snapshot s;
    s.tick = rb.current_tick();
    s.n_plus_in_mask = s.n_minus_in_mask = s.n_void_in_mask = 0;
    s.n_plus_outside = s.n_minus_outside = 0;
    s.mean_flux_mag_in_mask = 0;
    s.mean_wave_vel_mag_in_mask = 0;
    s.max_flux_mag_in_mask = 0;

    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int still_manifested = 0;

    for (int64_t i = 0; i < total; ++i) {
        bool in_mask = mask.count(static_cast<int>(i)) > 0;
        if (in_mask) {
            if (vox[i].state == 0) ++s.n_void_in_mask;
            else if (vox[i].state > 0) ++s.n_plus_in_mask;
            else ++s.n_minus_in_mask;
            if (vox[i].state != 0) ++still_manifested;
            double fmag = vox[i].flux.mag();
            double wmag = vox[i].wave_vel.mag();
            s.mean_flux_mag_in_mask += fmag;
            s.mean_wave_vel_mag_in_mask += wmag;
            if (fmag > s.max_flux_mag_in_mask) s.max_flux_mag_in_mask = fmag;
        } else {
            if (vox[i].state > 0) ++s.n_plus_outside;
            else if (vox[i].state < 0) ++s.n_minus_outside;
        }
    }
    if (!mask.empty()) {
        s.mean_flux_mag_in_mask /= mask.size();
        s.mean_wave_vel_mag_in_mask /= mask.size();
        s.persistence = static_cast<double>(still_manifested) / mask.size();
    } else {
        s.persistence = 0;
    }
    return s;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  PHASE B.3 (β'): A=10·K_GENESIS cascade trace\n";
    std::cout << "================================================================\n\n";

    const int L = 32;
    const int N_WARMUP = 50;
    const int N_TRACE = 160;     // covers expected death at tick ~135

    ftd::RenderBridge rb(L);
    rb.toggles.langevin_seed = 1;
    const int c = L / 2;
    rb.inject_flux(c, c, c, {10.0 * ftd::K_GENESIS, 0.0, 0.0});

    for (int t = 0; t < N_WARMUP; ++t) rb.tick();
    auto mask = snapshot_mask(rb);

    std::cout << "Configuration: L=" << L << ", warmup=" << N_WARMUP
              << ", trace ticks: " << N_TRACE << "\n";
    std::cout << "Initial mask size: " << mask.size() << " voxels\n";
    std::cout << "Engine defaults; Phase B.3 canonical config\n\n";

    std::cout << "tick   p_t      n+_mask  n-_mask  n0_mask  n+_out  n-_out  meanFlux  meanWvel  maxFlux\n";
    std::cout << "----   ------   -------  -------  -------  ------  ------  --------  --------  -------\n";

    int cascade_start_tick = -1;
    int previous_n_void_in_mask = 0;
    bool dead_reported = false;

    for (int t = 1; t <= N_TRACE; ++t) {
        rb.tick();
        Snapshot s = snapshot_state(rb, mask);

        // Detect cascade onset (first significant void appearance in mask)
        if (cascade_start_tick < 0 && s.n_void_in_mask >= 3 &&
            s.n_void_in_mask > previous_n_void_in_mask) {
            cascade_start_tick = t;
        }
        previous_n_void_in_mask = s.n_void_in_mask;

        // Print every tick once cascade is starting; sparse before
        bool print_this = (t % 5 == 0) ||
                          (cascade_start_tick > 0 && t >= cascade_start_tick - 5) ||
                          (s.persistence < 0.5 && !dead_reported);

        if (print_this) {
            std::cout << std::setw(4) << t << "   "
                      << std::fixed << std::setprecision(4) << std::setw(6) << s.persistence << "   "
                      << std::setw(7) << s.n_plus_in_mask << "  "
                      << std::setw(7) << s.n_minus_in_mask << "  "
                      << std::setw(7) << s.n_void_in_mask << "  "
                      << std::setw(6) << s.n_plus_outside << "  "
                      << std::setw(6) << s.n_minus_outside << "  "
                      << std::fixed << std::setprecision(4) << std::setw(8)
                      << s.mean_flux_mag_in_mask << "  "
                      << std::setw(8) << s.mean_wave_vel_mag_in_mask << "  "
                      << std::setw(7) << s.max_flux_mag_in_mask << "\n";
        }

        if (s.persistence == 0.0 && !dead_reported) {
            dead_reported = true;
            std::cout << "\n  [DEATH] persistence reached 0 at tick " << t << "\n";
        }
    }

    std::cout << "\n--- Cascade analysis ---\n";
    if (cascade_start_tick > 0) {
        std::cout << "  Cascade onset (first significant n_void_in_mask growth): tick "
                  << cascade_start_tick << "\n";
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: REPORTED (single-seed cascade trace)\n";
    std::cout << "================================================================\n";
    return 0;
}

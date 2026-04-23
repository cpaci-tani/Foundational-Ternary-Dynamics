#pragma once
/**
 * @file ftd/eft/manifestation_background.h
 * @brief Forced Poisson manifestation-injection background (Plan B, P2 protocol).
 *
 * Parallel to ftd::eft::prepare_thermal_background in coupling_measurement.h,
 * but instead of thermalizing wave_vel via Langevin, this places N random
 * signed charges at density n = N/L^3 and settles the flux field. Used by
 * the manifestation-scale-flow campaign to measure how density deforms the
 * bare Gaussian fixed point.
 */
#include <cstdint>
#include <cstdio>
#include <memory>
#include <random>
#include <utility>
#include <vector>

#include "ftd/eft/coupling_measurement.h"  // configure_bare_lattice_for_coupling
#include "ftd/render_bridge.h"

namespace ftd {
namespace eft {

/// Produce a manifestation-dressed RenderBridge. Caller owns the result.
///
/// Injects exactly N = floor(n * L^3) locked particles at distinct random
/// sites, with alternating signs so sum_Q = 0 exactly when N is even (if N
/// is odd, the final site is whichever sign closes the ledger, and sum_Q
/// is +-1; the caller should prefer even N).
///
/// Then settles the field for settle_ticks ticks with the bare-lattice
/// toggles (wave_propagation + coupling + gauss_projection), so the flux
/// field is the manifestation background we want to probe on top of.
///
/// Particles are placed on random sites but a rejection step skips any
/// site on the V(r) probe axis y = z = L/2. The test charges at
/// (mid, mid, mid) and (mid+r, mid, mid) sample the Green's function
/// along this line; any background charge anywhere on the axis biases it.
///
/// The BG settles with its own states locked; when the downstream probe
/// (measure_alpha_eff_on_bg, measure_kt_on_bg) creates a measurement
/// bridge, it copies only flux + wave_vel, leaving BG states behind.
/// This gives the clean "fluctuation dressing" of the flux field.
inline std::unique_ptr<RenderBridge> prepare_manifestation_background(
    int L, double density, uint64_t seed,
    int settle_ticks = 200,
    double initial_flux_z = 0.05)
{
    auto rb = std::make_unique<RenderBridge>(L);
    configure_bare_lattice_for_coupling(*rb);
    const int mid = L / 2;
    const int N_target = static_cast<int>(density * static_cast<double>(L) * L * L);

    // Reproducible uniform sampling of distinct sites.
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> uni(0, L - 1);

    std::vector<uint8_t> occupied(static_cast<size_t>(L) * L * L, 0);
    std::vector<int> placed_indices;
    placed_indices.reserve(static_cast<size_t>(N_target));
    int placed = 0;
    int attempts = 0;
    const int max_attempts = N_target * 100 + 1000;  // avoid infinite loops at high n
    int8_t next_sign = +1;
    while (placed < N_target && attempts < max_attempts) {
        ++attempts;
        const int x = uni(rng);
        const int y = uni(rng);
        const int z = uni(rng);
        // Reject any site on the V(r) probe axis (y = z = mid). The test charges
        // at (mid, mid, mid) and (mid+r, mid, mid) sample the Green's function
        // along this line; any background charge anywhere on the axis biases it.
        if (y == mid && z == mid) continue;
        const size_t idx = static_cast<size_t>(x) * L * L + static_cast<size_t>(y) * L + z;
        if (occupied[idx]) continue;
        occupied[idx] = 1;
        rb->inject_particle(x, y, z, next_sign,
                            {0.0, 0.0, static_cast<double>(next_sign) * initial_flux_z});
        placed_indices.push_back(rb->lattice().index(x, y, z));
        next_sign = -next_sign;
        ++placed;
    }

    // Apply locked=true to all placed sites in a single batch. Calling voxels()
    // once ensures the GPU->host sync happens before we write, and sets
    // host_mutated_ so the next tick/run pushes the flags back to the device.
    {
        auto& vox = rb->voxels();
        for (int idx : placed_indices) vox[idx].locked = true;
    }

    if (placed < N_target) {
        std::fprintf(stderr,
            "[prepare_manifestation_background] placed %d/%d at L=%d density=%.3g "
            "(guard/collision budget exhausted)\n",
            placed, N_target, L, density);
    }

    // Settle the flux field under bare-lattice dynamics.
    if (settle_ticks > 0) rb->run(settle_ticks);
    return rb;
}

/// Report how many manifestations were actually placed (useful for logging
/// in high-density cases where the guard corridor reduces the count).
inline int count_manifested_sites(const RenderBridge& rb) {
    int c = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0 && v.locked) ++c;
    return c;
}

}  // namespace eft
}  // namespace ftd

/**
 * @file test_native_evaporation_hazard_observer.cpp
 * @brief Unit and neutrality checks for the FTD-0432 hazard observer.
 */

#include "ftd/eft/native_evaporation_hazard_observer.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace {

int failures = 0;

void check(const char* name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= static_cast<std::uint64_t>(bytes[i]);
        hash *= 1099511628211ull;
    }
}

template <typename T>
void hash_value(std::uint64_t& hash, const T& value) {
    hash_bytes(hash, &value, sizeof(value));
}

std::uint64_t bridge_state_hash(const ftd::RenderBridge& bridge) {
    std::uint64_t hash = 1469598103934665603ull;
    hash_value(hash, bridge.current_tick());
    hash_value(hash, bridge.physical_time());
    for (const auto& voxel : bridge.voxels()) {
        hash_value(hash, voxel.state);
        hash_value(hash, voxel.flux.x);
        hash_value(hash, voxel.flux.y);
        hash_value(hash, voxel.flux.z);
        hash_value(hash, voxel.wave_vel.x);
        hash_value(hash, voxel.wave_vel.y);
        hash_value(hash, voxel.wave_vel.z);
        hash_value(hash, voxel.particle_id);
        hash_value(hash, voxel.spin);
        hash_value(hash, voxel.color);
    }
    return hash;
}

void initialize_square(ftd::RenderBridge& bridge, bool locked = false) {
    const int L = bridge.lattice().size();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int index = bridge.lattice().index(x, y, z);
                bridge.set_state(index, x < L / 2 ? 1 : -1);
                auto& voxel = bridge.voxel_at(x, y, z);
                voxel.flux = {};
                voxel.wave_vel = {};
                voxel.locked = locked;
            }
        }
    }
}

void configure(ftd::RenderBridge& bridge, bool coupled) {
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.evaporation = true;
    bridge.toggles.wave_propagation = coupled;
    bridge.toggles.coupling = coupled;
    bridge.toggles.langevin_seed = 17;
    bridge.seed_rng(17);
}

}  // namespace

int main() {
    ftd::RenderBridge isolated(8);
    configure(isolated, false);
    initialize_square(isolated);
    const auto bare = ftd::eft::observe_native_evaporation_hazard(
        isolated, 0, 1, {1, 0, 0});
    check("bare occupied sites are all eligible",
          bare.occupancy == 512 && bare.eligible_sites == 512);
    check("bare mean probability is exactly 0.1",
          std::abs(bare.mean_site_probability - 0.1) <= 1e-15);
    check("bare source hazard is exactly 0.1",
          std::abs(bare.source_hazard - 0.1) <= 1e-15);
    check("bare predicted energy is zero",
          bare.max_local_energy <= 1e-15);
    check("bare expected loss is one tenth of source",
          std::abs(bare.expected_loss_source - 0.1 * bare.source) <= 1e-15);

    ftd::RenderBridge locked(8);
    configure(locked, true);
    initialize_square(locked, true);
    const auto protected_hazard = ftd::eft::observe_native_evaporation_hazard(
        locked, 0, 1, {1, 0, 0});
    check("locked sites have zero eligible hazard",
          protected_hazard.eligible_sites == 0
          && protected_hazard.expected_removals == 0.0
          && protected_hazard.source_hazard == 0.0);

    ftd::RenderBridge control(8);
    ftd::RenderBridge observed(8);
    configure(control, true);
    configure(observed, true);
    initialize_square(control);
    initialize_square(observed);
    bool observer_finite = true;
    for (int tick = 0; tick < 32; ++tick) {
        const auto hazard = ftd::eft::observe_native_evaporation_hazard(
            observed, tick, 1, {1, 0, 0});
        observer_finite = observer_finite
            && std::isfinite(hazard.source_hazard)
            && hazard.min_site_probability >= 0.0
            && hazard.max_site_probability <= 0.1;
        control.tick();
        observed.tick();
    }
    check("coupled observer remains finite and probability-bounded",
          observer_finite);
    check("observer preserves selected state exactly",
          bridge_state_hash(control) == bridge_state_hash(observed));
    check("observer preserves RNG state exactly",
          control.rng_state_hash() == observed.rng_state_hash());

    std::cout << "native_evaporation_hazard_observer failures="
              << failures << '\n';
    return failures == 0 ? 0 : 1;
}

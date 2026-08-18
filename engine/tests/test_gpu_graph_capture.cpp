// ============================================================================
// test_gpu_graph_capture.cpp — graph replay must be BIT-IDENTICAL to direct
// launch, not merely close.
//
// Component A / Task 8. For each representative toggle profile, two engines
// are built from the same seed and scene; one runs with graph_capture_enabled
// false (direct launch), the other true (capture on the first tick of a key,
// replay thereafter). Every voxel field is compared byte-for-byte.
//   G1..G4  bit-identity across four toggle topologies
//   G5      a toggle flip mid-run adds a second cache entry (recapture)
//   G6      replay actually happened (graph_replays() > 0)
// ============================================================================

#include "ftd/gpu_engine.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

using namespace ftd;

namespace {

void seed_scene(gpu::GpuEngine& e) {
    e.seed_rng_for_test();
    e.inject_particle(3, 3, 3, +1, Vec3{0.0, 0.0, 0.0});
    e.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    e.inject_particle(8, 3, 12, +1, Vec3{0.0, 0.0, 0.0});
    e.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

std::uint64_t fold(const std::vector<Voxel>& voxels) {
    std::uint64_t h = 1469598103934665603ULL;
    const auto mix = [&h](const void* p, std::size_t n) {
        const auto* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < n; ++i) {
            h ^= b[i];
            h *= 1099511628211ULL;
        }
    };
    for (const auto& v : voxels) {
        mix(&v.state, sizeof(v.state));
        mix(&v.flux, sizeof(v.flux));
        mix(&v.wave_vel, sizeof(v.wave_vel));
        mix(&v.velocity, sizeof(v.velocity));
        mix(&v.remainder, sizeof(v.remainder));
        mix(&v.latency, sizeof(v.latency));
        mix(&v.tau, sizeof(v.tau));
        mix(&v.spin, sizeof(v.spin));
        mix(&v.color, sizeof(v.color));
        mix(&v.locked, sizeof(v.locked));
        mix(&v.particle_id, sizeof(v.particle_id));
        mix(&v.pair_id, sizeof(v.pair_id));
    }
    return h;
}

enum class Profile { Defaults, SingleSubstrate, Qcd, Gravity };

void apply_profile(gpu::GpuEngine& e, Profile p) {
    e.toggles.enable_all();
    switch (p) {
        case Profile::Defaults:
            break;
        case Profile::SingleSubstrate:
            e.toggles.dual_substrate = false;
            e.toggles.weak_transmutation = false;
            break;
        case Profile::Qcd:
            e.toggles.color_forces = true;
            e.toggles.strong_force = true;
            e.toggles.triad_binding = true;
            break;
        case Profile::Gravity:
            e.toggles.latency_field = true;
            e.toggles.field_energy_gravity = true;
            e.toggles.pair_production = true;
            break;
    }
}

const char* profile_name(Profile p) {
    switch (p) {
        case Profile::Defaults:        return "defaults";
        case Profile::SingleSubstrate: return "single-substrate";
        case Profile::Qcd:             return "qcd";
        case Profile::Gravity:         return "gravity";
    }
    return "?";
}

void compare_profile(Profile p, int ticks) {
    constexpr int L = 17;
    gpu::GpuEngine direct(L);
    apply_profile(direct, p);
    direct.graph_capture_enabled = false;
    seed_scene(direct);
    for (int t = 0; t < ticks; ++t) direct.tick();

    gpu::GpuEngine graphed(L);
    apply_profile(graphed, p);
    graphed.graph_capture_enabled = true;
    seed_scene(graphed);
    for (int t = 0; t < ticks; ++t) graphed.tick();

    std::vector<Voxel> a, b;
    direct.sync_to_host(a);
    graphed.sync_to_host(b);

    const std::string label = std::string("graph replay is bit-identical: ")
                            + profile_name(p);
    test::check(label.c_str(), fold(a) == fold(b));
    const std::string replayed_label =
        std::string("graph replayed at least once: ") + profile_name(p);
    test::check(replayed_label.c_str(), graphed.graph_replays() > 0);
    const std::string captured_label =
        std::string("capture succeeded (no fallback): ") + profile_name(p);
    test::check(captured_label.c_str(), graphed.graph_capture_failures() == 0);
}

}  // namespace

int main() {
    test::init("test_gpu_graph_capture");

    test::section("G1-G4: bit-identity across four toggle topologies");
    compare_profile(Profile::Defaults, 24);
    compare_profile(Profile::SingleSubstrate, 24);
    compare_profile(Profile::Qcd, 24);
    compare_profile(Profile::Gravity, 24);

    test::section("G5: a toggle flip forces a recapture");
    {
        gpu::GpuEngine engine(17);
        engine.toggles.enable_all();
        engine.graph_capture_enabled = true;
        seed_scene(engine);
        for (int t = 0; t < 6; ++t) engine.tick();
        const std::size_t after_first = engine.graph_cache_size();
        test::check("G5: one cached graph for the first topology",
                    after_first == 1);
        engine.toggles.movement = false;   // topology-affecting toggle
        for (int t = 0; t < 6; ++t) engine.tick();
        test::check("G5: toggle flip added a second cached graph",
                    engine.graph_cache_size() == 2);
        test::check("G5: two captures were performed",
                    engine.graph_captures() == 2);
    }

    test::section("G6: the device tick advances under replay");
    {
        gpu::GpuEngine engine(17);
        engine.toggles.enable_all();
        engine.graph_capture_enabled = true;
        seed_scene(engine);
        for (int t = 0; t < 9; ++t) engine.tick();
        test::check("G6: host tick is 9", engine.current_tick() == 9);
        test::check("G6: device tick is 9", engine.device_tick() == 9);
    }

    test::section("G7: cache eviction at MAX_GRAPH_CACHE");
    {
        // GpuEngine::MAX_GRAPH_CACHE (private, gpu_engine.h) is 16: on the
        // 17th distinct graph_key(), destroy_graph_cache() wipes the whole
        // cache before capturing again. Cycle through more combinations than
        // that to force at least one full-cache eviction and confirm: (a)
        // graph_cache_size() never exceeds the cap, (b) genuine recapture
        // happens after eviction rather than growth silently stalling at the
        // cap (graph_captures() ends up strictly greater than the cap), and
        // (c) nothing crashes or throws — including destroying the exec from
        // the immediately preceding, un-synchronized tick, which may still be
        // executing on the device at the moment of eviction (see
        // destroy_graph_cache()'s doc comment in gpu_engine.cu for why that
        // is safe). No physics/bit-identity assertion is made here — cache
        // eviction guarantees a cache-size bound and correct recapture, not
        // any particular field values.
        constexpr std::size_t kMaxGraphCache = 16;
        constexpr int kCombos = 24;  // > kMaxGraphCache: guarantees eviction
        gpu::GpuEngine engine(17);
        engine.toggles.enable_all();
        engine.graph_capture_enabled = true;
        seed_scene(engine);

        std::size_t max_observed_cache_size = 0;
        for (int i = 0; i < kCombos; ++i) {
            // Five independent topology-affecting toggles (all hashed by
            // graph_key()) bit-flipped per combo give 2^5=32 distinct
            // topologies; kCombos=24 of them is enough to force eviction.
            engine.toggles.movement      = (i & 1)  != 0;
            engine.toggles.forces        = (i & 2)  != 0;
            engine.toggles.latency_field = (i & 4)  != 0;
            engine.toggles.color_forces  = (i & 8)  != 0;
            engine.toggles.damping       = (i & 16) != 0;
            for (int t = 0; t < 3; ++t) engine.tick();
            if (engine.graph_cache_size() > max_observed_cache_size) {
                max_observed_cache_size = engine.graph_cache_size();
            }
        }
        test::check("G7: cache size never exceeds MAX_GRAPH_CACHE",
                    max_observed_cache_size <= kMaxGraphCache);
        test::check("G7: recapture happened after eviction (captures > cap)",
                    engine.graph_captures() > kMaxGraphCache);
        test::check("G7: no capture failures across the eviction sweep",
                    engine.graph_capture_failures() == 0);
    }

    return test::finalize();
}

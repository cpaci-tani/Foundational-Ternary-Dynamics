// ============================================================================
// test_gpu_particle_capacity.cpp — fixed-capacity pairwise-force launches.
//
// Component A / Task 5. The pairwise/triad launches no longer read the
// particle count back to the host to size their grid; they launch
// MAX_PARTICLES threads and bound themselves from a device pointer.
//   P1  Mainline: a handful of colored particles still produce the same
//       strong-force diagnostics as a direct count-sized launch would.
//   P2  Near the bound (8000 of 8192): runs, no overflow flag.
//   P3  Over the bound (9000): the sticky device flag fires and the engine
//       throws at the next synchronization boundary.
// ============================================================================

#include "ftd/gpu_engine.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

#include <stdexcept>
#include <vector>

using namespace ftd;

namespace {

// Fill the first `count` lattice sites with alternating colored charges.
void stage_particles(gpu::GpuEngine& engine, int L, int count) {
    std::vector<Voxel> voxels(static_cast<std::size_t>(L) * L * L);
    for (int i = 0; i < count; ++i) {
        Voxel& v = voxels[static_cast<std::size_t>(i)];
        v.state = (i % 2 == 0) ? int8_t(+1) : int8_t(-1);
        v.color = static_cast<int8_t>(1 + (i % 3));
        v.spin  = (i % 2 == 0) ? int8_t(+1) : int8_t(-1);
        v.particle_id = i;
        v.flux = Vec3{0.05, 0.0, 0.0};
    }
    engine.upload_from_host(voxels);
}

void quiet_toggles(gpu::GpuEngine& engine) {
    engine.toggles.disable_all();
    engine.toggles.dual_substrate = false;
    engine.toggles.genesis = false;
    engine.toggles.evaporation = false;
    engine.toggles.movement = false;
    engine.toggles.gauss_projection = false;
    engine.toggles.weak_transmutation = false;
    engine.toggles.color_forces = true;
    engine.toggles.forces = false;
}

}  // namespace

int main() {
    test::init("test_gpu_particle_capacity");

    test::section("P1: mainline colored particles");
    {
        constexpr int L = 32;
        gpu::GpuEngine engine(L);
        quiet_toggles(engine);
        stage_particles(engine, L, 12);
        engine.tick();
        std::vector<Voxel> out;
        engine.sync_to_host(out);
        int manifested = 0;
        for (const auto& v : out) if (v.state != 0) ++manifested;
        test::check("P1: 12 particles survive one color-force tick",
                    manifested == 12);
        const auto& fd = engine.force_diag();
        bool any_strong = false;
        for (int i = 0; i < 12; ++i) {
            if (fd.strong_x[i] != 0.0 || fd.strong_y[i] != 0.0
                || fd.strong_z[i] != 0.0) { any_strong = true; break; }
        }
        test::check("P1: color force wrote strong-force diagnostics",
                    any_strong);
    }

    test::section("P2: 8000 particles (just under the 8192 bound)");
    {
        constexpr int L = 32;
        gpu::GpuEngine engine(L);
        quiet_toggles(engine);
        stage_particles(engine, L, 8000);
        bool threw = false;
        try {
            engine.tick();
            std::vector<Voxel> out;
            engine.sync_to_host(out);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        test::check("P2: no overflow below the capacity bound", !threw);
    }

    test::section("P3: 9000 particles (over the 8192 bound)");
    {
        constexpr int L = 32;
        gpu::GpuEngine engine(L);
        quiet_toggles(engine);
        stage_particles(engine, L, 9000);
        bool threw = false;
        try {
            engine.tick();
            std::vector<Voxel> out;
            engine.sync_to_host(out);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        test::check("P3: capacity overflow throws at the sync boundary",
                    threw);
        // Distinguishes the NEW device-side detection mechanism from the
        // OLD host-readback-based throw this task removes: at HEAD, before
        // Task 5's changes, GpuBuffers has no d_particle_overflow member at
        // all, so this line fails to COMPILE (the correct TDD-red state for
        // this test — the old code already throws on overflow via a
        // different path, so a runtime assertion alone would be green
        // before any of this task's changes land).
        test::check("P3: sticky device overflow flag exists",
                    engine.bufs().d_particle_overflow != nullptr);

        test::section("P4: overflow flag clears after being observed");
        // Same engine instance that just overflowed in P3 above — the whole
        // point is to prove the sticky flag is sticky-UNTIL-ACKNOWLEDGED,
        // not sticky-forever. P3's sync_to_host() -> ensure_host_synced()
        // already routed through throw_if_particle_overflow(), which reads
        // the device flag (observing it as 1, hence the throw above) and
        // then resets it to 0 on the device. stage_particles() fully
        // overwrites this engine's voxel state via upload_from_host(), so
        // staging a small, safely-under-capacity count now produces a
        // genuinely fresh, non-overflowing state with no leftover particles
        // from P3.
        stage_particles(engine, L, 100);
        bool threw_again = false;
        try {
            engine.tick();
            std::vector<Voxel> out;
            engine.sync_to_host(out);
        } catch (const std::runtime_error&) {
            threw_again = true;
        }
        test::check(
            "P4: no throw once the count is back under capacity "
            "(flag was cleared, not permanently stuck)",
            !threw_again);
    }

    return test::finalize();
}

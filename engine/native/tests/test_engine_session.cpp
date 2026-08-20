#include "ftd/test_telemetry.h"
#include "native/engine_session.h"

#include <string>

int main() {
    ftd::test::init("test_native_session");
    ftd::test::section("in-process session is isolated from the web stack");

    ftd::native::NativeEngineOptions options;
    options.lattice_size = 9;
    options.scenario = "s0-seed-ee-annihilation";
    options.force_cpu = true;

    ftd::native::NativeEngineSession session(options);
    ftd::test::check("session uses CPU when requested",
                     std::string(session.backend_name()) == "cpu");
    ftd::test::check("session keeps requested lattice size",
                     session.lattice_size() == 9);

    ftd::native::NativeFrame frame = session.capture();
    ftd::test::check("scenario or fallback produced visible particles",
                     !frame.particles.empty());
    ftd::test::check("capture reports the loaded scenario",
                     frame.scenario == "s0-seed-ee-annihilation");

    const int before = session.current_tick();
    session.tick();
    session.tick();
    ftd::test::check("tick advances in-process",
                     session.current_tick() == before + 2);

    frame = session.capture();
    ftd::test::check("capture after ticks still returns a frame",
                     frame.lattice_size == 9);

    ftd::test::section("scenario and lattice can be switched in-process");
    session.load_scenario("s0-seed-hydrogen");
    ftd::test::check("load_scenario updates the current name",
                     session.scenario() == "s0-seed-hydrogen");
    frame = session.capture();
    ftd::test::check("switched scenario still produces a frame",
                     frame.lattice_size == 9);
    ftd::test::check("hydrogen seed is visible", !frame.particles.empty());

    session.set_lattice_size(17);
    ftd::test::check("set_lattice_size rebuilds the lattice",
                     session.lattice_size() == 17);
    frame = session.capture();
    ftd::test::check("resized capture reports the new lattice",
                     frame.lattice_size == 17);
    ftd::test::check("resized scenario is preserved",
                     session.scenario() == "s0-seed-hydrogen");

    session.set_flux_boundary(1);
    ftd::test::check("flux boundary can change without a rebuild",
                     session.flux_boundary() == 1);
    frame = session.capture();
    ftd::test::check("boundary change is visible on the next frame",
                     frame.flux_boundary == 1);

    session.reset_current();
    ftd::test::check("reset keeps lattice and scenario",
                     session.lattice_size() == 17 &&
                         session.scenario() == "s0-seed-hydrogen");

    ftd::test::section("interop path is a safe no-op on the CPU backend");
    ftd::test::check("interop starts disabled", !session.interop_enabled());
    const bool enabled =
        session.try_enable_interop(nullptr, 0, nullptr);
    ftd::test::check("try_enable_interop refuses a CPU-backend session",
                     !enabled);
    ftd::test::check("interop_enabled stays false after a refused enable",
                     !session.interop_enabled());
    ftd::test::check("poll_interop_particle_count reports not-ready (-1)",
                     session.poll_interop_particle_count() == -1);
    // Must be a harmless no-op (not a crash) when interop was never enabled.
    session.request_interop_gather(1);
    ftd::test::check("poll after a no-op gather request still reports -1",
                     session.poll_interop_particle_count() == -1);

    return ftd::test::finalize();
}

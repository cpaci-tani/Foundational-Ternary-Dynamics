#include "ftd/test_telemetry.h"
#include "native_desktop/engine_session.h"

#include <string>

int main() {
    ftd::test::init("test_native_desktop_session");
    ftd::test::section("in-process session is isolated from the web stack");

    ftd::native_desktop::NativeEngineOptions options;
    options.lattice_size = 9;
    options.scenario = "s0-seed-ee-annihilation";
    options.force_cpu = true;

    ftd::native_desktop::NativeEngineSession session(options);
    ftd::test::check("session uses CPU when requested",
                     std::string(session.backend_name()) == "cpu");
    ftd::test::check("session keeps requested lattice size",
                     session.lattice_size() == 9);

    ftd::native_desktop::NativeFrame frame = session.capture();
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

    return ftd::test::finalize();
}

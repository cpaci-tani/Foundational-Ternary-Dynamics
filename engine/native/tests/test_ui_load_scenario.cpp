#include "native/command_queue.h"
#include "native/engine_session.h"

#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_ui_load_scenario");

    ftd::native::NativeEngineOptions options;
    options.force_cpu = true;
    options.lattice_size = 9;
    options.scenario = "s0-seed-hydrogen";
    ftd::native::NativeEngineSession session(options);
    ftd::native::CommandQueue queue;

    ftd::test::section("LoadScenario through process_ui_boundary");
    queue.push(ftd::native::LoadScenario{"empty"});
    session.process_ui_boundary(queue);
    ftd::test::check("LoadScenario updates the session name",
                     session.scenario() == "empty");
    ftd::test::check("LoadScenario marks applied_reload", session.applied_reload());
    const auto empty_frame = session.capture();
    ftd::test::check("capture after UI load still works",
                     empty_frame.lattice_size == 9);
    const auto empty_snap = session.snapshot_publisher().acquire();
    ftd::test::check("published snapshot exists after load",
                     empty_snap != nullptr);
    ftd::test::check("published snapshot names the loaded scenario",
                     empty_snap && empty_snap->frame.scenario == "empty");

    ftd::test::section("a second LoadScenario (play-bar Reset uses this path)");
    queue.push(ftd::native::LoadScenario{"s0-seed-hydrogen"});
    session.process_ui_boundary(queue);
    ftd::test::check("second LoadScenario updates the session",
                     session.scenario() == "s0-seed-hydrogen");
    const auto hydrogen = session.capture();
    ftd::test::check("hydrogen is visible after UI reload",
                     !hydrogen.particles.empty());

    ftd::test::section("ApplyReboot through process_ui_boundary");
    session.stage_lattice_size(17);
    queue.push(ftd::native::ApplyReboot{});
    session.process_ui_boundary(queue);
    ftd::test::check("ApplyReboot resizes the session",
                     session.lattice_size() == 17);
    const auto resized = session.capture();
    ftd::test::check("capture after ApplyReboot reports the new lattice",
                     resized.lattice_size == 17);
    ftd::test::check("ApplyReboot keeps the current scenario",
                     session.scenario() == "s0-seed-hydrogen");

    return ftd::test::finalize();
}

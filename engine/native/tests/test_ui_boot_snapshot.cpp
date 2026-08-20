#include "native/cli_options.h"
#include "native/engine_session.h"

#include "ftd/test_telemetry.h"

#include <string>

int main() {
    ftd::test::init("test_ui_boot_snapshot");

    ftd::test::section("interactive defaults");
    ftd::native::NativeEngineOptions defaults;
    ftd::test::check("GPU is the app default", !defaults.force_cpu);
    ftd::test::check("boot scenario is hydrogen",
                     defaults.scenario == "s0-seed-hydrogen");

    ftd::test::section("session publishes a snapshot before any tick");
    ftd::native::NativeEngineOptions options;
    options.force_cpu = true;
    options.lattice_size = 9;
    options.scenario = "s0-seed-hydrogen";
    ftd::native::NativeEngineSession session(options);

    ftd::test::check("session starts paused", session.loop_control().pause);
    const auto snap = session.snapshot_publisher().acquire();
    ftd::test::check("boot published a snapshot", snap != nullptr);
    ftd::test::check("snapshot lattice matches boot",
                     snap && snap->knobs.lattice_size == 9);
    ftd::test::check("snapshot scenario matches boot",
                     snap && snap->frame.scenario == "s0-seed-hydrogen");
    ftd::test::check("snapshot seq is monotone from 1", snap && snap->seq >= 1);
    ftd::test::check("no tick has run yet", session.current_tick() == 0);

    return ftd::test::finalize();
}

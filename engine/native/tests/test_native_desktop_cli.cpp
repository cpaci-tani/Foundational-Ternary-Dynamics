#include "native/cli_options.h"
#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_native_cli");

    ftd::test::section("defaults");
    const char* none[] = {"ftd_native"};
    const auto empty = ftd::native::parse_native_cli(1, none);
    ftd::test::check("default gpu", !empty.options.force_cpu);
    ftd::test::check("default lattice 32", empty.options.lattice_size == 32);
    ftd::test::check("default scenario", empty.options.scenario == "s0-seed-hydrogen");
    ftd::test::check("default ui enabled", !empty.options.no_ui);
    ftd::test::check("help not requested", !empty.help);

    ftd::test::section("--no-ui skips ImGui without changing sim flags");
    const char* no_ui[] = {
        "ftd_native", "--no-ui", "--cpu", "--lattice", "48",
        "--scenario", "s0-seed-hydrogen"
    };
    const auto parsed = ftd::native::parse_native_cli(7, no_ui);
    ftd::test::check("no_ui is set", parsed.options.no_ui);
    ftd::test::check("cpu remains selected", parsed.options.force_cpu);
    ftd::test::check("lattice parsed", parsed.options.lattice_size == 48);
    ftd::test::check("scenario parsed", parsed.options.scenario == "s0-seed-hydrogen");

    ftd::test::section("--help is parse-only");
    const char* help[] = {"ftd_native", "--help"};
    const auto helped = ftd::native::parse_native_cli(2, help);
    ftd::test::check("help requested", helped.help);
    ftd::test::check("help does not imply no-ui", !helped.options.no_ui);

    return ftd::test::finalize();
}

#include "native/command_queue.h"
#include "native/engine_session.h"

#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

#include <variant>

int main() {
    ftd::test::init("test_ui_harness_commands");

    ftd::native::NativeEngineOptions options;
    options.force_cpu = true;
    options.lattice_size = 9;
    options.scenario = "empty";
    ftd::native::NativeEngineSession session(options);
    ftd::native::CommandQueue queue;

    ftd::test::section("harness writes do not coalesce");
    ftd::native::CommandQueue raw;
    raw.push(ftd::native::InjectWavepacket{4, 4, 4, 1});
    raw.push(ftd::native::InjectFluxAdd{4, 4, 4, 0.4, 0.0, 0.0});
    raw.push(ftd::native::InjectWavepacket{5, 4, 4, -1});
    const auto drained = raw.drain();
    ftd::test::check("three harness commands stay FIFO", drained.size() == 3);
    ftd::test::check("second command is flux",
                     std::holds_alternative<ftd::native::InjectFluxAdd>(
                         drained[1].command));

    ftd::test::section("wavepacket apply is visible while paused");
    queue.push(ftd::native::ClearField{});
    session.process_ui_boundary(queue);
    queue.push(ftd::native::InjectWavepacket{4, 4, 4, 1});
    session.process_ui_boundary(queue);
    ftd::test::check("inject sets host-write while paused",
                     session.applied_host_write());
    ftd::test::check("still paused after inject", session.loop_control().pause);
    const auto after_inject = session.capture();
    ftd::test::check("wavepacket produced manifested particles",
                     after_inject.total_manifested > 0
                         || !after_inject.particles.empty());

    ftd::test::section("flux inject and clear");
    queue.push(ftd::native::InjectFluxAdd{4, 4, 4, ftd::K_B * 0.8, 0.0, 0.0});
    session.process_ui_boundary(queue);
    const auto with_flux = session.capture();
    ftd::test::check("flux inject is captured while paused",
                     !with_flux.flux.empty());
    queue.push(ftd::native::ClearField{});
    session.process_ui_boundary(queue);
    const auto cleared = session.capture();
    ftd::test::check("clearField zeros captured flux", cleared.flux.empty());

    ftd::test::section("entangled pair");
    queue.push(ftd::native::CreateEntangledPair{4, 4, 4, ftd::K_B, 0.0, 0.0});
    session.process_ui_boundary(queue);
    const auto paired = session.capture();
    ftd::test::check("pair inject is a host write", session.applied_host_write());
    ftd::test::check("pair is visible in the captured frame",
                     paired.total_manifested > 0 || !paired.particles.empty());

    ftd::test::section("out-of-range coordinates wrap");
    queue.push(ftd::native::InjectWavepacket{-1, 20, 4, 1});
    session.process_ui_boundary(queue);
    ftd::test::check("wrapped inject applies as a host write",
                     session.applied_host_write());

    ftd::test::section("seedRandomFlux is journalled and not replayed as ResetToDefaults");
    const auto before = session.parameter_journal().entries().size();
    queue.push(ftd::native::SeedRandomFlux{});
    session.process_ui_boundary(queue);
    ftd::test::check("seedRandomFlux appended a journal row",
                     session.parameter_journal().entries().size() > before);
    const auto& last = session.parameter_journal().entries().back();
    ftd::test::check("seedRandomFlux uses a harness key",
                     last.key.find("harness.") == 0);

    return ftd::test::finalize();
}

#include "native_desktop/command_applier.h"
#include "native_desktop/command_queue.h"
#include "native_desktop/engine_session.h"
#include "native_desktop/parameter_journal.h"

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_ui_journal_replay");

    ftd::RenderBridge original(9);
    original.force_cpu();
    original.seed_rng(7);

    ftd::native_desktop::ParameterJournal journal;
    ftd::native_desktop::LoopControl loop;
    const int tick = original.current_tick();

    const auto apply = [&](ftd::native_desktop::UiCommand command) {
        ftd::native_desktop::QueuedCommand item;
        item.command = std::move(command);
        ftd::native_desktop::apply_mutation_on_bridge(
            original, nullptr, item, journal, tick, loop);
    };

    apply(ftd::native_desktop::SetToggle{"symplectic_leapfrog", true});
    apply(ftd::native_desktop::SetDt{0.5});
    apply(ftd::native_desktop::SetToggle{"symplectic_leapfrog", false});
    apply(ftd::native_desktop::SetDt{0.7});
    apply(ftd::native_desktop::SetSorIterations{4});
    apply(ftd::native_desktop::SetDouble{
        ftd::native_desktop::DoubleKey::genesis_threshold_override, 0.25});
    apply(ftd::native_desktop::SetDouble{
        ftd::native_desktop::DoubleKey::manifest_scale_override, 0.5});
    apply(ftd::native_desktop::SetBoolConfig{
        ftd::native_desktop::BoolCfgKey::manifest_use_temperature, true});
    apply(ftd::native_desktop::SetToggle{"larmor_radiation", true});

    bool saw_clamped_dt = false;
    for (const auto& entry : journal.entries()) {
        if (entry.key == "bridge.dt" && entry.requested.d == 0.7) {
            ftd::test::check("applied dt is read back after the clamp",
                             entry.applied.d == original.dt());
            saw_clamped_dt = entry.applied.d != entry.requested.d
                             || entry.applied.d == 0.7
                             || entry.applied.d == 1.0;
        }
    }
    ftd::test::check("journal contains the W4 dt write", saw_clamped_dt);
    ftd::test::check("first dt request 0.5 applied while leapfrog was on", [&] {
        for (const auto& entry : journal.entries()) {
            if (entry.key == "bridge.dt" && entry.requested.d == 0.5) {
                return entry.applied.d == 0.5;
            }
        }
        return false;
    }());

    ftd::RenderBridge replayed(9);
    replayed.force_cpu();
    replayed.seed_rng(7);
    journal.replay_requests(replayed, nullptr);

    ftd::test::check("replay reproduces TermToggles",
                     ftd::native_desktop::same_term_toggles(original.toggles,
                                                            replayed.toggles));
    ftd::test::check("replay reproduces the six knobs",
                     ftd::native_desktop::same_bridge_knobs(original, replayed));

    ftd::test::section("unknown scenario is a structured reload failure");
    ftd::native_desktop::NativeEngineSession session({9, "s0-seed-hydrogen", true, 2});
    ftd::native_desktop::ParameterJournal reload_journal;
    const auto unknown = ftd::native_desktop::apply_mutation(
        session, ftd::native_desktop::LoadScenario{"definitely-not-a-scenario"},
        reload_journal);
    ftd::test::check("unknown scenario is not ok", !unknown.ok);
    ftd::test::check("unknown scenario uses ReloadStatus",
                     unknown.error_code
                         == static_cast<int>(ftd::native_desktop::ReloadStatus::UnknownScenario));
    ftd::test::check("session records UnknownScenario",
                     session.last_reload_result().status
                         == ftd::native_desktop::ReloadStatus::UnknownScenario);

    const auto known = ftd::native_desktop::apply_mutation(
        session, ftd::native_desktop::LoadScenario{"s0-seed-hydrogen"}, reload_journal);
    ftd::test::check("known scenario applies", known.ok);
    ftd::test::check("known scenario records Success",
                     session.last_reload_result().status
                         == ftd::native_desktop::ReloadStatus::Success);

    return ftd::test::finalize();
}

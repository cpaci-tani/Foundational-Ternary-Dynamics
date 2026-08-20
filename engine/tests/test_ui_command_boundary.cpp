#include "native_desktop/command_applier.h"
#include "native_desktop/command_queue.h"
#include "native_desktop/parameter_journal.h"

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"
#include "golden_hash.h"

#include <cstdint>

namespace {

void seed_cpu(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(42);
    rb.set_state(4, 4, 4, 1);
}

bool same_ledger(const ftd::EnergyLedger& a, const ftd::EnergyLedger& b) {
    return a.updates == b.updates && a.tick_prev == b.tick_prev;
}

}  // namespace

int main() {
    ftd::test::init("test_ui_command_boundary");

    ftd::test::section("N4: queue drain matches a direct between-tick write");
    ftd::RenderBridge direct(9);
    ftd::RenderBridge queued(9);
    seed_cpu(direct);
    seed_cpu(queued);

    ftd::native_desktop::CommandQueue queue;
    ftd::native_desktop::ParameterJournal journal;
    ftd::native_desktop::UiBoundaryState state;
    state.journal = &journal;

    for (int tick = 1; tick <= 200; ++tick) {
        direct.tick();
        queued.tick();
        if (tick == 50) {
            direct.toggles.larmor_radiation = true;
            queue.push(ftd::native_desktop::SetToggle{"larmor_radiation", true});
            ftd::native_desktop::process_ui_boundary(queued, nullptr, queue, state);
            ftd::test::check("apply-time tick equals the settled boundary",
                             state.apply_tick == queued.current_tick());
            ftd::test::check("journal records the boundary tick",
                             !journal.entries().empty()
                             && journal.entries().back().tick_applied == queued.current_tick());
        }
        ftd::test::check("state-only hash remains equal",
                         ftd::test::compute_state_only_hash(direct) ==
                             ftd::test::compute_state_only_hash(queued));
        ftd::test::check("ledger tick_prev and updates remain equal",
                         same_ledger(direct.energy_ledger(), queued.energy_ledger()));
    }
    ftd::test::check("queued toggle is on after the boundary apply",
                     queued.toggles.larmor_radiation);

    ftd::test::section("Step does not tick inside the drain");
    ftd::RenderBridge stepper(9);
    seed_cpu(stepper);
    stepper.tick();
    const int before = stepper.current_tick();
    ftd::native_desktop::CommandQueue steps;
    ftd::native_desktop::UiBoundaryState step_state;
    steps.push(ftd::native_desktop::Step{1});
    steps.push(ftd::native_desktop::SetToggle{"larmor_radiation", true});
    ftd::native_desktop::process_ui_boundary(stepper, nullptr, steps, step_state);
    ftd::test::check("drain did not call tick", stepper.current_tick() == before);
    ftd::test::check("Step queued a later tick", step_state.loop.pending_steps == 1);
    ftd::test::check("mutation after Step still applied before that tick",
                     stepper.toggles.larmor_radiation);

    ftd::test::section("unresolved SetToggle is a hard error");
    ftd::native_desktop::CommandQueue bad;
    ftd::native_desktop::ParameterJournal bad_journal;
    ftd::native_desktop::UiBoundaryState bad_state;
    bad_state.journal = &bad_journal;
    bad.push(ftd::native_desktop::SetToggle{"not_a_real_toggle", true});
    const auto applied = ftd::native_desktop::apply_mutation_on_bridge(
        stepper, nullptr,
        ftd::native_desktop::QueuedCommand{1, ftd::native_desktop::SetToggle{"not_a_real_toggle", true}},
        bad_journal, stepper.current_tick(), bad_state.loop);
    ftd::test::check("unresolved name fails closed", !applied.ok);
    ftd::test::check("unresolved name is journalled", !bad_journal.entries().empty());

    return ftd::test::finalize();
}

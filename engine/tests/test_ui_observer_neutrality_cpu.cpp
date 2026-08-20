#include "ftd/backend.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/telemetry_snapshot.h"
#include "ftd/term_toggles.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdint>

namespace {

bool same_toggle_profile(const ftd::TermToggles& a,
                         const ftd::TermToggles& b) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (a.*(spec.field) != b.*(spec.field)) return false;
    }
    return a.bcc_stencil == b.bcc_stencil
        && a.langevin_site_filter == b.langevin_site_filter
        && a.langevin_T == b.langevin_T
        && a.langevin_gamma == b.langevin_gamma
        && a.langevin_seed == b.langevin_seed
        && a.coulomb_charge_coupling == b.coulomb_charge_coupling
        && a.coulomb_source_scale == b.coulomb_source_scale
        && a.omega0 == b.omega0
        && a.kinetic_drain == b.kinetic_drain
        && a.flux_boundary == b.flux_boundary;
}

bool same_ledger(const ftd::EnergyLedger& a, const ftd::EnergyLedger& b) {
    return a.updates == b.updates
        && a.tick_prev == b.tick_prev
        && a.E_prev == b.E_prev
        && a.E_curr == b.E_curr
        && a.dE_dt == b.dE_dt
        && a.drift_frac == b.drift_frac
        && a.expected_rate == b.expected_rate
        && a.residual == b.residual
        && a.cumulative_injection == b.cumulative_injection
        && a.cumulative_dissipation == b.cumulative_dissipation
        && a.max_residual_seen == b.max_residual_seen;
}

void exercise_observers(ftd::RenderBridge& rb) {
    (void)rb.diagnostics();
    (void)rb.energy_audit();
    (void)rb.gravity_metric_agg();
    ftd::LagrangianDiag lagrangian{};
    (void)rb.copy_compact_lagrangian(lagrangian);
    (void)rb.inspect_voxel(4, 4, 4);
    (void)rb.inspect_force(4, 4, 4);
    (void)rb.charge_sum();
    (void)rb.continuity_step();
}

void seed_cpu(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(42);
    rb.set_state(4, 4, 4, 1);
}

}  // namespace

int main() {
    ftd::test::init("test_ui_observer_neutrality_cpu");
    ftd::test::section("N1: energy ledger updates exactly once per CPU tick");

    ftd::RenderBridge rb(9);
    seed_cpu(rb);
    ftd::test::check("CPU backend is active",
                     rb.backend().kind() == ftd::Backend::Kind::Cpu);

    double previous_accumulator = 0.0;
    bool saw_accumulator_advance = false;
    for (std::uint64_t expected = 1; expected <= 200; ++expected) {
        rb.tick();
        const auto& ledger = rb.energy_ledger();
        ftd::test::check("ledger update count follows current tick",
                         ledger.updates == expected);
        if (expected == 1) {
            ftd::test::check("first update seeds current tick",
                             ledger.tick_prev == rb.current_tick());
            ftd::test::check("first update seeds equal energies",
                             ledger.E_prev == ledger.E_curr);
        } else {
            ftd::test::check("later updates identify previous tick",
                             ledger.tick_prev == rb.current_tick() - 1);
        }
        const double accumulator =
            ledger.cumulative_injection + ledger.cumulative_dissipation;
        ftd::test::check("ledger accumulators never move backward",
                         accumulator >= previous_accumulator);
        if (accumulator > previous_accumulator) {
            saw_accumulator_advance = true;
        }
        previous_accumulator = accumulator;
    }
    ftd::test::check("ledger accumulators advance during the run",
                     saw_accumulator_advance);

    ftd::test::section("N2: CPU observers do not alter trajectory");
    ftd::RenderBridge bare(9);
    ftd::RenderBridge observed(9);
    seed_cpu(bare);
    seed_cpu(observed);

    for (int tick = 0; tick < 100; ++tick) {
        bare.tick();
        observed.tick();
        exercise_observers(observed);
        ftd::test::check("state-only hash remains equal",
                         ftd::test::compute_state_only_hash(bare) ==
                             ftd::test::compute_state_only_hash(observed));
        ftd::test::check("RNG state remains equal",
                         bare.rng_state_hash() == observed.rng_state_hash());
        ftd::test::check("every ledger field remains equal",
                         same_ledger(bare.energy_ledger(),
                                     observed.energy_ledger()));
        ftd::test::check("tick and clock remain equal",
                         bare.current_tick() == observed.current_tick()
                         && bare.physical_time() == observed.physical_time()
                         && bare.dt() == observed.dt()
                         && bare.sor_iterations() == observed.sor_iterations());
        ftd::test::check("toggle profiles remain field-wise equal",
                         same_toggle_profile(bare.toggles, observed.toggles));
    }

    ftd::test::section("N3: telemetry request slot is single occupancy");
    ftd::RenderBridge slot(9);
    seed_cpu(slot);
    ftd::TelemetrySnapshotRequest first{};
    first.groups = ftd::TELEMETRY_DIAGNOSTICS;
    first.epoch = 11;
    ftd::TelemetrySnapshotRequest rejected = first;
    rejected.epoch = 12;
    ftd::test::check("first request begins",
                     slot.begin_telemetry_snapshot(first));
    ftd::test::check("overlapping request is rejected",
                     !slot.begin_telemetry_snapshot(rejected));
    ftd::TelemetrySnapshot snapshot{};
    ftd::test::check("pending request polls",
                     slot.poll_telemetry_snapshot(snapshot));
    ftd::test::check("rejected request did not replace pending epoch",
                     snapshot.epoch == 11);
    ftd::test::check("poll consumes the slot",
                     !slot.poll_telemetry_snapshot(snapshot));
    ftd::test::check("slot accepts after drain",
                     slot.begin_telemetry_snapshot(rejected));

    return ftd::test::finalize();
}

#include "ftd/backend.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/telemetry_snapshot.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdio>
#include <thread>

namespace {

bool require_gpu(ftd::RenderBridge& rb) {
    if (rb.backend().kind() == ftd::Backend::Kind::Gpu) return true;
    std::printf("[ui-observer-neutrality-gpu] SKIP: no active CUDA backend\n");
    return false;
}

bool drain_snapshot(ftd::RenderBridge& rb, ftd::TelemetrySnapshot& out) {
    for (int attempt = 0; attempt < 10000; ++attempt) {
        if (rb.poll_telemetry_snapshot(out)) return true;
        std::this_thread::yield();
    }
    return false;
}

void seed(ftd::RenderBridge& rb) {
    rb.seed_rng(42);
    rb.set_state(4, 4, 4, 1);
    rb.set_interactive_gpu_mode(true);
}

void observe(ftd::RenderBridge& rb) {
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

}  // namespace

int main() {
    ftd::test::init("test_ui_observer_neutrality_gpu");

    ftd::test::section("N1-gpu: interactive baseline skips EnergyLedger");
    ftd::RenderBridge ledger(9);
    if (!require_gpu(ledger)) return ftd::test::finalize();
    seed(ledger);
    for (int tick = 0; tick < 200; ++tick) ledger.tick();
    ftd::test::check("interactive GPU ledger remains uncomputed in Phase 0A",
                     ledger.energy_ledger().updates == 0);

    ftd::test::section("N2-gpu: compact observers preserve trajectory");
    ftd::RenderBridge bare(9);
    ftd::RenderBridge observed(9);
    if (!require_gpu(bare) || !require_gpu(observed)) {
        return ftd::test::finalize();
    }
    seed(bare);
    seed(observed);
    for (int tick = 0; tick < 100; ++tick) {
        bare.tick();
        observed.tick();
        observe(observed);
        ftd::test::check("GPU observer state hash remains equal",
                         ftd::test::compute_state_only_hash(bare) ==
                             ftd::test::compute_state_only_hash(observed));
        ftd::test::check("GPU observer RNG state remains equal",
                         bare.rng_state_hash() == observed.rng_state_hash());
    }

    ftd::test::section("N6: telemetry want-mask is physics-neutral");
    ftd::RenderBridge no_demand(9);
    ftd::RenderBridge all_demand(9);
    if (!require_gpu(no_demand) || !require_gpu(all_demand)) {
        return ftd::test::finalize();
    }
    seed(no_demand);
    seed(all_demand);
    for (int tick = 0; tick < 100; ++tick) {
        no_demand.tick();
        all_demand.tick();
        ftd::TelemetrySnapshotRequest request{};
        request.groups = ftd::TELEMETRY_ALL;
        request.epoch = static_cast<std::uint64_t>(tick + 1);
        ftd::test::check("GPU telemetry request begins",
                         all_demand.begin_telemetry_snapshot(request));
        ftd::TelemetrySnapshot snapshot{};
        ftd::test::check("GPU telemetry request drains",
                         drain_snapshot(all_demand, snapshot));
        ftd::test::check("GPU demand state hash remains equal",
                         ftd::test::compute_state_only_hash(no_demand) ==
                             ftd::test::compute_state_only_hash(all_demand));
        ftd::test::check("GPU demand RNG state remains equal",
                         no_demand.rng_state_hash() ==
                             all_demand.rng_state_hash());
    }

    return ftd::test::finalize();
}

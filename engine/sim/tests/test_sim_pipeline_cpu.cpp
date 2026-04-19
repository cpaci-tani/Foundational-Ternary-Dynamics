/**
 * @file test_sim_pipeline_cpu.cpp
 * @brief Phase B exit gate — Pipeline<BackendCpu> + 3 reference observables.
 *
 * Verifies:
 *   P1. Pipeline can be constructed, toggles set, flux injected.
 *   P2. Pipeline runs N ticks and advances tick counter.
 *   P3. TotalFieldEnergy matches RenderBridge::energy_audit().field_energy
 *       to floating-point precision after a short run.
 *   P4. MeanAbsFlux matches a direct voxel-loop computation.
 *   P5. StateHistogram counts agree with a direct voxel-loop.
 *   P6. observe_every schedules fire at the expected ticks.
 *   P7. observe_at fires exactly once at the exact tick requested.
 */

#include <cmath>
#include <cstdio>
#include <memory>

#include "ftd/sim/pipeline.h"
#include "ftd/sim/backend_cpu.h"
#include "ftd/sim/observables/total_field_energy.h"
#include "ftd/sim/observables/mean_abs_flux.h"
#include "ftd/sim/observables/state_histogram.h"

using ftd::sim::Pipeline;
using ftd::sim::BackendCpu;
using ftd::sim::TotalFieldEnergy;
using ftd::sim::MeanAbsFlux;
using ftd::sim::StateHistogram;

static int g_failures = 0;
static void check(const char* name, bool ok, const char* detail = nullptr) {
    if (ok) std::printf("  PASS  %s\n", name);
    else {
        std::printf("  FAIL  %s%s%s\n", name,
                    detail ? "  " : "", detail ? detail : "");
        ++g_failures;
    }
}

// P1 — construction + configuration + injection don't crash; state is
// accessible.
static void p1_construct() {
    std::puts("\n--- P1: Pipeline construct + configure + inject ---");
    const int L = 16;
    Pipeline<BackendCpu> p(L);
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.damping = false;
    t.genesis = false;
    p.set_toggles(t);
    p.inject_flux(8, 8, 8, {0.1, 0.2, 0.3});
    p.inject_particle(7, 8, 8, +1, {0.0, 0.0, 0.0});

    const auto& vox = p.state().voxels();
    const int mid = p.state().lattice().index(8, 8, 8);
    const int near = p.state().lattice().index(7, 8, 8);

    check("P1 lattice size == L", p.L() == L);
    check("P1 injected flux visible", std::abs(vox[mid].flux.x - 0.1) < 1e-12);
    check("P1 injected particle state == +1", vox[near].state == +1);
    check("P1 tick == 0 before run()", p.tick() == 0);
}

// P2 — run() advances the tick counter.
static void p2_tick_advance() {
    std::puts("\n--- P2: Pipeline::run advances tick ---");
    Pipeline<BackendCpu> p(16);
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.damping = false;
    t.genesis = false;
    p.set_toggles(t);
    p.inject_flux(8, 8, 8, {0.1, 0.0, 0.0});
    const int t0 = p.tick();
    p.run(25);
    check("P2 tick advanced by 25", p.tick() == t0 + 25);
}

// P3 — TotalFieldEnergy matches RenderBridge::energy_audit().field_energy
static void p3_total_field_energy() {
    std::puts("\n--- P3: TotalFieldEnergy matches engine energy_audit ---");
    Pipeline<BackendCpu> p(16);
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.gauss_projection = true;
    t.damping = false;
    t.genesis = false;
    t.selective_damping = false;
    t.larmor_radiation = false;
    t.forces = false;
    t.lorentz_force = false;
    t.color_forces = false;
    t.movement = false;
    p.set_toggles(t);

    for (int x = 6; x < 10; ++x)
        for (int y = 6; y < 10; ++y)
            for (int z = 6; z < 10; ++z)
                p.inject_flux(x, y, z, {0.1, 0.0, 0.0});

    auto obs = std::make_shared<TotalFieldEnergy<BackendCpu>>();
    p.observe_every(10, obs);
    p.run(50);

    const double measured = obs->result_host();
    const double reference = p.state().energy_audit().field_energy;

    char buf[128];
    std::snprintf(buf, sizeof buf, "(measured=%.6f engine=%.6f)", measured, reference);
    check("P3 TotalFieldEnergy within 1e-6 of engine energy_audit",
          std::abs(measured - reference) / std::max(std::abs(reference), 1e-30) < 1e-6, buf);
    check("P3 History has 5 entries (ticks 10,20,30,40,50)",
          obs->history().size() == 5);
}

// P4 — MeanAbsFlux matches direct computation
static void p4_mean_abs_flux() {
    std::puts("\n--- P4: MeanAbsFlux matches direct loop ---");
    Pipeline<BackendCpu> p(12);
    for (int x = 0; x < 12; ++x)
        for (int y = 0; y < 12; ++y)
            for (int z = 0; z < 12; ++z)
                p.inject_flux(x, y, z, {0.1 * x, 0.2 * y, 0.3 * z});

    auto obs = std::make_shared<MeanAbsFlux<BackendCpu>>();
    p.observe_at(0, obs);  // measure pre-run? observe_at fires on tick==0 after first tick
    // Actually observe_at(0, ...) fires when state.tick() == 0. But run(n) calls
    // tick_once() FIRST, incrementing the counter, then checks. So we need observe_at(1)
    // to measure after 1 tick has run.
    //
    // For this test, let's measure the INITIAL state before any tick.
    ftd::TermToggles t{};
    t.wave_propagation = false;  // no dynamics — preserves initial flux exactly
    t.gauss_projection = false;
    t.damping = false;
    t.genesis = false;
    t.coupling = false;
    p.set_toggles(t);

    // Call measure directly on the state (testing the observable, not the schedule)
    obs->measure(p.state());
    const double measured = obs->result_host();

    // Direct reference
    double sum = 0.0;
    const auto& vox = p.state().voxels();
    for (const auto& v : vox) sum += std::sqrt(v.flux.dot(v.flux));
    const double reference = sum / static_cast<double>(vox.size());

    char buf[128];
    std::snprintf(buf, sizeof buf, "(measured=%.6f direct=%.6f)", measured, reference);
    check("P4 MeanAbsFlux matches direct computation to 1e-12",
          std::abs(measured - reference) < 1e-12, buf);
}

// P5 — StateHistogram counts
static void p5_state_histogram() {
    std::puts("\n--- P5: StateHistogram counts ---");
    Pipeline<BackendCpu> p(10);
    p.inject_particle(1, 1, 1, +1, {0.0, 0.0, 0.0});
    p.inject_particle(2, 2, 2, +1, {0.0, 0.0, 0.0});
    p.inject_particle(3, 3, 3, -1, {0.0, 0.0, 0.0});

    auto obs = std::make_shared<StateHistogram<BackendCpu>>();
    obs->measure(p.state());
    const auto c = obs->result_host();

    char buf[128];
    std::snprintf(buf, sizeof buf, "(n+=%lld n-=%lld n0=%lld)",
                  c.n_plus, c.n_minus, c.n_zero);
    check("P5 n_plus == 2",  c.n_plus == 2, buf);
    check("P5 n_minus == 1", c.n_minus == 1, buf);
    check("P5 n_zero == 997", c.n_zero == 10 * 10 * 10 - 3, buf);
    check("P5 imbalance == +1", c.imbalance() == 1, buf);
    check("P5 manifested == 3", c.manifested() == 3, buf);
}

// P6 — observe_every schedule
static void p6_observe_every() {
    std::puts("\n--- P6: observe_every schedule ---");
    Pipeline<BackendCpu> p(8);
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.gauss_projection = false;  // keep it simple
    t.damping = false;
    t.genesis = false;
    t.coupling = false;
    p.set_toggles(t);
    p.inject_flux(4, 4, 4, {0.1, 0.0, 0.0});

    auto obs = std::make_shared<TotalFieldEnergy<BackendCpu>>();
    p.observe_every(5, obs);  // first at tick 5, then 10, 15, 20
    p.run(20);

    char buf[96];
    std::snprintf(buf, sizeof buf, "(got %zu entries; expected 4)", obs->history().size());
    check("P6 observe_every(5) fires 4 times in run(20)",
          obs->history().size() == 4, buf);
}

// P7 — observe_at
static void p7_observe_at() {
    std::puts("\n--- P7: observe_at fires exactly once ---");
    Pipeline<BackendCpu> p(8);
    ftd::TermToggles t{};
    t.wave_propagation = true;
    t.gauss_projection = false;
    t.damping = false;
    t.genesis = false;
    t.coupling = false;
    p.set_toggles(t);
    p.inject_flux(4, 4, 4, {0.1, 0.0, 0.0});

    auto obs = std::make_shared<TotalFieldEnergy<BackendCpu>>();
    p.observe_at(13, obs);
    p.run(30);

    char buf[128];
    std::snprintf(buf, sizeof buf, "(got %zu entries; first at tick %d)",
                  obs->history().size(),
                  obs->history().empty() ? -1 : obs->history()[0].first);
    check("P7 observe_at fires exactly once",
          obs->history().size() == 1 && obs->history()[0].first == 13, buf);
}

int main() {
    std::puts("================================================================");
    std::puts("  Sim Pipeline — Phase B CPU Backend Tests");
    std::puts("================================================================");

    p1_construct();
    p2_tick_advance();
    p3_total_field_energy();
    p4_mean_abs_flux();
    p5_state_histogram();
    p6_observe_every();
    p7_observe_at();

    std::puts("\n----------------------------------------------------------------");
    if (g_failures == 0) {
        std::puts("  All sim-pipeline CPU tests PASS");
        return 0;
    }
    std::printf("  %d sim-pipeline test(s) FAILED\n", g_failures);
    return 1;
}

#pragma once
/**
 * @file ftd/sim/pipeline.h
 * @brief Pipeline<Backend> — the orchestrator.
 *
 * A Pipeline owns:
 *   - one Backend::DeviceState (CPU backend's RenderBridge, or GPU's GpuEngine)
 *   - a vector of ObservationEntry<Backend> (what to measure, when)
 *
 * Its public methods are:
 *   - set_toggles / inject_* / lock — to configure the initial condition
 *   - observe_every / observe_at   — to attach observables to the tick loop
 *   - run(ticks)                   — to advance the simulation,
 *                                     calling measure() on each observable
 *                                     at its scheduled ticks
 *   - state()                      — raw DeviceState access for custom work
 *
 * The backend parameter selects CPU or GPU at compile time via template
 * specialisation — zero runtime dispatch cost in the tick loop. Code that
 * wants to pick a backend at program start can use a `std::variant` or
 * dispatch through a small factory; most sim-test programs will just
 * `using P = Pipeline<BackendCpu>;` (or GPU) and go.
 *
 * Example usage (see engine/sim/tests/test_sim_pipeline_cpu.cpp):
 *
 *     Pipeline<BackendCpu> p(64);
 *     TermToggles t{}; t.wave_propagation = true; t.gauss_projection = true;
 *     p.set_toggles(t);
 *     p.inject_flux(32, 32, 32, {1.0, 0.0, 0.0});
 *
 *     auto energy = std::make_shared<TotalFieldEnergy<BackendCpu>>();
 *     p.observe_every(50, energy);
 *
 *     p.run(200);
 *     double E_final = energy->result_host();
 */

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>

#include "ftd/sim/device_state.h"
#include "ftd/sim/observable.h"
#include "ftd/term_toggles.h"
#include "ftd/voxel.h"      // Vec3

namespace ftd {
namespace sim {

template <typename Backend>
class Pipeline {
public:
    using DeviceState = typename Backend::DeviceState;
    using Entry = ObservationEntry<Backend>;

    /// Construct with lattice size L. Allocates a DeviceState (and
    /// whatever device memory the backend needs).
    explicit Pipeline(int L) : state_(L) {}

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    // ── Configuration (pre-run) ──────────────────────────────────────
    void set_toggles(const TermToggles& t) { state_.set_toggles(t); }

    void inject_flux(int x, int y, int z, const Vec3& J) {
        state_.inject_flux(x, y, z, J);
    }
    void inject_particle(int x, int y, int z, int8_t s, const Vec3& J) {
        state_.inject_particle(x, y, z, s, J);
    }
    void lock(int x, int y, int z) { state_.lock(x, y, z); }

    // ── Observation schedule ─────────────────────────────────────────
    /// Measure `obs` every `interval` ticks starting at tick `first_tick`.
    /// `interval = 0` and `first_tick = t0` means "measure once, at t0".
    void observe_every(int interval, ObservablePtr<Backend> obs, int first_tick = -1) {
        int t0 = (first_tick < 0) ? interval : first_tick;
        entries_.push_back(Entry{t0, interval, std::move(obs)});
    }
    /// Measure `obs` exactly once, at tick `tick`.
    void observe_at(int tick, ObservablePtr<Backend> obs) {
        entries_.push_back(Entry{tick, 0, std::move(obs)});
    }

    // ── Execution ────────────────────────────────────────────────────
    /// Advance `n` ticks, calling each attached Observable at its
    /// scheduled ticks. Measurement happens AFTER the tick (so at
    /// tick t you see the state after the t-th update).
    void run(int n) {
        for (int step = 0; step < n; ++step) {
            state_.tick_once();
            const int t_now = state_.tick();
            for (const Entry& e : entries_) {
                if (should_measure(e, t_now)) {
                    e.obs->measure(state_);
                }
            }
        }
    }

    // ── Introspection ────────────────────────────────────────────────
    DeviceState& state() { return state_; }
    const DeviceState& state() const { return state_; }
    int tick() const { return state_.tick(); }
    int L() const { return state_.L(); }
    static constexpr const char* backend_name() { return Backend::name(); }

    /// Clear all observation entries (but NOT the state). Useful when
    /// reusing a pipeline for multiple scenarios.
    void clear_schedule() { entries_.clear(); }

private:
    DeviceState state_;
    std::vector<Entry> entries_;

    /// Returns true if observation `e` should fire at tick `t`.
    static bool should_measure(const Entry& e, int t) {
        if (t < e.first_tick) return false;
        if (e.interval == 0) return t == e.first_tick;
        return (t - e.first_tick) % e.interval == 0;
    }
};

}  // namespace sim
}  // namespace ftd

#pragma once
/**
 * @file ftd/sim/observable.h
 * @brief Observable base class — the unit of measurement in a pipeline.
 *
 * An Observable<Backend, T> measures a quantity of type T from the simulation
 * state at the moment Pipeline calls its measure() method. The measure()
 * is expected to leave its result in an internal (small) buffer; then
 * result_host() copies out to the host when the caller wants the value.
 *
 * This two-step pattern (measure → result_host) is what allows the GPU
 * backend to run the reduction on device and defer the PCIe copy until
 * needed. For CPU this is a no-op split; the measure() does the work and
 * result_host() just returns the stored value.
 *
 * Observable lifecycle (per run):
 *   1. Pipeline.observe_every(interval, obs) or .observe_at(tick, obs)
 *   2. During Pipeline::run():
 *        - at the scheduled tick, measure(state) is called
 *        - measure() appends or overwrites the internal result buffer
 *   3. After run() returns, result_host() gives the caller the host value
 *   4. reset() clears internal buffers (so same Observable can be reused)
 *
 * Implementation requirements:
 *   - measure() must be fast (per-tick hot path). Use device reductions.
 *   - result_host() may synchronize (PCIe copy) — called rarely.
 *   - Observables are stateful (they hold their accumulator) but must
 *     be copyable-via-shared_ptr: Pipeline stores them as shared_ptr.
 */

#include <memory>
#include <vector>

namespace ftd {
namespace sim {

/// Non-templated base for storage in Pipeline's observation schedule.
/// Pipeline itself doesn't know the value type T of any individual
/// observable — it just knows "call measure() at the right tick."
template <typename Backend>
class ObservableBase {
public:
    virtual ~ObservableBase() = default;
    virtual void measure(typename Backend::DeviceState& state) = 0;
    virtual void reset() = 0;
};

/// Typed observable. Subclasses specialize measure() and result_host()
/// for the backend-specific semantics.
template <typename Backend, typename T>
class Observable : public ObservableBase<Backend> {
public:
    ~Observable() override = default;
    virtual T result_host() const = 0;
};

/// Convenience alias — observations are stored as shared_ptr in the
/// pipeline schedule. shared_ptr (not unique_ptr) because callers may
/// want to hold a reference to the observable for later result_host()
/// lookup after Pipeline::run() returns.
template <typename Backend>
using ObservablePtr = std::shared_ptr<ObservableBase<Backend>>;

/// Pipeline's internal schedule entry — ticks + which observable to
/// measure at that moment. Pipeline walks this list each tick.
template <typename Backend>
struct ObservationEntry {
    int first_tick;     ///< first tick to measure at (must be >= 0)
    int interval;       ///< 0 = measure only at first_tick; >0 = every `interval` ticks
    ObservablePtr<Backend> obs;
};

}  // namespace sim
}  // namespace ftd

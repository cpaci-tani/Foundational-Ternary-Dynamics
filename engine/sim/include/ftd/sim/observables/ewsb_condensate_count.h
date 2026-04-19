#pragma once
/**
 * @file ftd/sim/observables/ewsb_condensate_count.h
 * @brief EwsbCondensateCount — EWSB-threshold diagnostic.
 *
 * A compound observable that records, at each measurement:
 *   - ⟨|J|⟩ (EWSB order parameter)
 *   - total field energy (Σ|J|²)
 *   - state counts (N+, N-, N0)
 *   - charge imbalance N+ − N−
 *
 * This is exactly what the Day-2 EWSB threshold-map scan measures; the
 * observable bundles them so a single Pipeline::observe_every() call
 * records the full condensate fingerprint at each sampled tick.
 *
 * Result type: a struct EwsbSnapshot containing all four quantities.
 * History is the per-tick trajectory, used by downstream analysis
 * (e.g. scripts/benchmarks/analyze_ewsb_spectroscopy.py).
 */

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

struct EwsbSnapshot {
    int tick = 0;
    double mean_abs_J = 0.0;
    double field_energy = 0.0;
    long long n_plus = 0;
    long long n_minus = 0;
    long long n_zero = 0;
    long long imbalance() const { return n_plus - n_minus; }
    long long manifested() const { return n_plus + n_minus; }
};

template <typename Backend>
class EwsbCondensateCount : public Observable<Backend, EwsbSnapshot> {
public:
    void measure(typename Backend::DeviceState& state) override {
        const auto& vox = state.voxels();
        const int N = static_cast<int>(vox.size());
        EwsbSnapshot s;
        s.tick = state.tick();
        double sum_abs = 0.0, sum_sq = 0.0;
        for (int i = 0; i < N; ++i) {
            const double jmag2 = vox[i].flux.dot(vox[i].flux);
            sum_abs += std::sqrt(jmag2);
            sum_sq += jmag2;
            const int8_t st = vox[i].state;
            if (st > 0) ++s.n_plus;
            else if (st < 0) ++s.n_minus;
            else ++s.n_zero;
        }
        s.mean_abs_J = (N > 0) ? sum_abs / static_cast<double>(N) : 0.0;
        s.field_energy = sum_sq;
        last_value_ = s;
        history_.push_back(s);
    }
    EwsbSnapshot result_host() const override { return last_value_; }
    void reset() override { last_value_ = {}; history_.clear(); }
    const std::vector<EwsbSnapshot>& history() const { return history_; }

private:
    EwsbSnapshot last_value_;
    std::vector<EwsbSnapshot> history_;
};

}  // namespace sim
}  // namespace ftd

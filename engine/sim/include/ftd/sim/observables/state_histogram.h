#pragma once
/**
 * @file ftd/sim/observables/state_histogram.h
 * @brief StateHistogram — counts of state ∈ {−1, 0, +1}.
 *
 * Critical for EWSB-condensate diagnostics (charge imbalance N+ − N−
 * as amp sweeps through the threshold) and for any scenario where the
 * engine's manifestation rules fire.
 *
 * Backend-agnostic implementation via state.voxels().
 */

#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

struct StateCounts {
    long long n_minus = 0;
    long long n_zero = 0;
    long long n_plus = 0;

    long long total() const { return n_minus + n_zero + n_plus; }
    long long imbalance() const { return n_plus - n_minus; }
    long long manifested() const { return n_plus + n_minus; }
};

template <typename Backend>
class StateHistogram : public Observable<Backend, StateCounts> {
public:
    void measure(typename Backend::DeviceState& state) override {
        const auto& vox = state.voxels();
        const int N = static_cast<int>(vox.size());
        StateCounts c;
        for (int i = 0; i < N; ++i) {
            const int8_t s = vox[i].state;
            if (s > 0) ++c.n_plus;
            else if (s < 0) ++c.n_minus;
            else ++c.n_zero;
        }
        last_value_ = c;
        history_.emplace_back(state.tick(), c);
    }
    StateCounts result_host() const override { return last_value_; }
    void reset() override {
        last_value_ = {};
        history_.clear();
    }
    const std::vector<std::pair<int, StateCounts>>& history() const { return history_; }

private:
    StateCounts last_value_;
    std::vector<std::pair<int, StateCounts>> history_;
};

}  // namespace sim
}  // namespace ftd

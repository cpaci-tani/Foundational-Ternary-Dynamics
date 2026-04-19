#pragma once
/**
 * @file ftd/sim/observables/mean_abs_flux.h
 * @brief MeanAbsFlux — average |J| over all voxels.
 *
 * Used as the EWSB order parameter in the Day-2 threshold-map scan
 * (benchmark_ewsb_threshold_map.cpp). The condensate transition is
 * diagnosed by ⟨|J|⟩ growth from ~initial amplitude up to ~2.3 at
 * saturation.
 *
 * Backend-agnostic implementation via state.voxels().
 */

#include <cmath>
#include <utility>
#include <vector>

#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

template <typename Backend>
class MeanAbsFlux : public Observable<Backend, double> {
public:
    void measure(typename Backend::DeviceState& state) override {
        const auto& vox = state.voxels();
        const int N = static_cast<int>(vox.size());
        if (N == 0) { last_value_ = 0.0; return; }
        double sum = 0.0;
        for (int i = 0; i < N; ++i) {
            sum += std::sqrt(vox[i].flux.dot(vox[i].flux));
        }
        last_value_ = sum / static_cast<double>(N);
        history_.emplace_back(state.tick(), last_value_);
    }
    double result_host() const override { return last_value_; }
    void reset() override { last_value_ = 0.0; history_.clear(); }
    const std::vector<std::pair<int, double>>& history() const { return history_; }

private:
    double last_value_ = 0.0;
    std::vector<std::pair<int, double>> history_;
};

}  // namespace sim
}  // namespace ftd

#pragma once
/**
 * @file ftd/sim/observables/total_field_energy.h
 * @brief TotalFieldEnergy — sum of |J|² over all voxels.
 *
 * This is the simplest possible scalar observable — a reduction of the
 * squared-flux magnitude summed over the entire lattice. Convention
 * matches RenderBridge::energy_audit().field_energy exactly (which
 * uses Σ|J|², without a 1/2 prefactor — the engine's internal
 * book-keeping convention).
 *
 * On GPU it will be a cub::DeviceReduce over d_flux_{x,y,z} arrays.
 *
 * Intended use: smoke-test the pipeline plumbing. Every Observable should
 * agree with the direct CPU reduction to 1e-12 on tick 0 (no dynamics run),
 * and to the engine's own energy_audit() to fp precision after run.
 *
 * API:
 *   TotalFieldEnergy<Backend> obs;
 *   pipeline.observe_every(50, std::make_shared<TotalFieldEnergy<Backend>>());
 *   // ... pipeline.run(N) ...
 *   double E = obs.result_host();          // scalar: last measured value
 *   const auto& history = obs.history();   // vector of (tick, energy) pairs
 */

#include <utility>
#include <vector>

#include "ftd/sim/backend_cpu.h"
#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

/// Observable: total field energy Σ (1/2) |J(x)|² summed over all voxels.
/// Value type is double. History records (tick, energy) at every measure().
template <typename Backend>
class TotalFieldEnergy : public Observable<Backend, double> {
public:
    // CPU backend specialisation
    void measure(typename Backend::DeviceState& state) override;
    double result_host() const override { return last_value_; }
    void reset() override { last_value_ = 0.0; history_.clear(); }

    const std::vector<std::pair<int, double>>& history() const { return history_; }

private:
    double last_value_ = 0.0;
    std::vector<std::pair<int, double>> history_;
};

// CPU backend implementation — header-inline (short and hot-ish).
template <>
inline void TotalFieldEnergy<BackendCpu>::measure(BackendCpu::DeviceState& state) {
    const auto& vox = state.voxels();
    const int N = static_cast<int>(vox.size());
    double sum = 0.0;
    for (int i = 0; i < N; ++i) {
        sum += vox[i].flux.dot(vox[i].flux);
    }
    last_value_ = sum;
    history_.emplace_back(state.tick(), sum);
}

// GPU backend implementation is out-of-line in src/observables/total_field_energy_gpu.cu
// (to avoid pulling CUDA into every TU that includes this header).

}  // namespace sim
}  // namespace ftd

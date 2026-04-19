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
 */

#include <utility>
#include <vector>

#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

/// Observable: total field energy Σ |J(x)|² summed over all voxels.
/// Value type is double. History records (tick, energy) at every measure().
///
/// The measure() implementation is backend-agnostic: it calls
/// `state.voxels()` which each backend implements. On CPU this is a
/// zero-cost reference; on GPU it triggers a sync_to_host() PCIe
/// download (slow, but correct — Phase C baseline).
template <typename Backend>
class TotalFieldEnergy : public Observable<Backend, double> {
public:
    void measure(typename Backend::DeviceState& state) override {
        const auto& vox = state.voxels();
        const int N = static_cast<int>(vox.size());
        double sum = 0.0;
        for (int i = 0; i < N; ++i) {
            sum += vox[i].flux.dot(vox[i].flux);
        }
        last_value_ = sum;
        history_.emplace_back(state.tick(), sum);
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

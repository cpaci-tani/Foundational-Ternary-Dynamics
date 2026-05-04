#pragma once
/**
 * @file ftd/sim/observables/field_energy_audit.h
 * @brief FieldEnergyAudit — forwards to the engine's built-in energy audit.
 *
 * This observable doesn't reduce voxels itself; it calls
 * state.energy_audit() which is the engine's canonical per-tick energy
 * book-keeping. Reported fields:
 *   - field_energy  (Σ½|J|², same as TotalFieldEnergy but cached)
 *   - wave_energy   (Σ½|wave_vel|²)
 *   - total_energy
 *
 * Useful for β-function extraction, where we want the engine's OWN
 * view of the energy (which may include cross-terms and corrections
 * that a naive voxel-reduction misses).
 */

#include <utility>
#include <vector>

#include "ftd/render_bridge.h"  // for EnergyAudit
#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

template <typename Backend>
class FieldEnergyAudit : public Observable<Backend, EnergyAudit> {
public:
    void measure(typename Backend::DeviceState& state) override {
        last_value_ = state.energy_audit();
        history_.emplace_back(state.tick(), last_value_);
    }
    EnergyAudit result_host() const override { return last_value_; }
    void reset() override { last_value_ = {}; history_.clear(); }
    const std::vector<std::pair<int, EnergyAudit>>& history() const { return history_; }

private:
    EnergyAudit last_value_{};
    std::vector<std::pair<int, EnergyAudit>> history_;
};

}  // namespace sim
}  // namespace ftd

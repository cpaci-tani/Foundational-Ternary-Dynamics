#pragma once

/**
 * @file field_mediation_probe.h
 * @brief Observer-only causal carrier ledger for Scale-1 research.
 *
 * [IMPOSED RESEARCH HARNESS] This probe replaces an instantaneous source to
 * target observation with an explicit finite carrier record whose arrival is
 * delayed by the radius-one Moore causal ceiling.  It does not apply a force,
 * supply reciprocal recoil, identify a photon, recover QED, or couple to the
 * production ParticleEngine.  Its purpose is to make those missing contracts
 * typed and testable before any production interaction is attempted.
 */

#include "ftd/voxel.h"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace ftd::scale1 {

struct FieldMediationDispatch {
    std::int64_t emission_tick = 0;
    std::int32_t source_id = -1;
    std::int32_t target_id = -1;
    Vec3 source_position;
    Vec3 target_position;
    double signed_amplitude = 0.0;
};

struct FieldMediationArrival {
    std::uint64_t sequence = 0;
    std::int64_t emission_tick = 0;
    std::int64_t arrival_tick = 0;
    std::int32_t source_id = -1;
    std::int32_t target_id = -1;
    int moore_distance = 0;
    double signed_amplitude = 0.0;
    double booked_energy = 0.0;
    bool applied_to_particle_engine = false;
    bool reciprocal_recoil_supplied = false;
    bool photon_identified = false;
    bool born_weight_used = false;
};

class FieldMediationProbe {
public:
    bool dispatch(const FieldMediationDispatch& dispatch,
                  std::string* error = nullptr);
    std::vector<FieldMediationArrival> advance_to(std::int64_t tick);
    void clear();

    std::int64_t current_tick() const { return current_tick_; }
    std::size_t pending_count() const { return pending_.size(); }
    std::uint64_t delivered_count() const { return delivered_count_; }
    double dispatched_energy() const { return dispatched_energy_; }
    double field_energy() const { return field_energy_; }
    double delivered_energy() const { return delivered_energy_; }
    double ledger_residual() const {
        return dispatched_energy_ - field_energy_ - delivered_energy_;
    }

    static constexpr bool production_coupling_supplied() { return false; }
    static constexpr bool reciprocal_recoil_supplied() { return false; }
    static constexpr bool photon_identified() { return false; }
    static constexpr bool qed_amplitude_supplied() { return false; }
    static constexpr bool born_weights_used() { return false; }
    static constexpr bool moving_source_tracking_supplied() { return false; }

private:
    std::deque<FieldMediationArrival> pending_;
    std::int64_t current_tick_ = 0;
    std::uint64_t next_sequence_ = 0;
    std::uint64_t delivered_count_ = 0;
    double dispatched_energy_ = 0.0;
    double field_energy_ = 0.0;
    double delivered_energy_ = 0.0;
};

}  // namespace ftd::scale1

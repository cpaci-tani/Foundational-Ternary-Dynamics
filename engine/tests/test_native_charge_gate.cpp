/**
 * FTD native conserved-charge gate.
 *
 * This test is deliberately hostile to charge emergence. It verifies that the
 * observer is bit/RNG neutral, records actual production events, and that the
 * preregistered exact additive feature basis has a trivial nullspace once
 * genesis and weak transmutation are admitted.
 */

#include "ftd/constants.h"
#include "ftd/eft/conserved_charge_basis.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

std::array<std::int64_t, 4> site_features(
    const ftd::eft::HistorySiteState& site) {
    const std::int64_t s = site.state;
    const std::int64_t h = site.chirality_sign;
    return {std::llabs(s), s, h, s * h};
}

std::array<std::int64_t, 4> event_delta(const ftd::eft::HistoryEvent& event) {
    std::array<std::int64_t, 4> delta{};
    for (int site = 0; site < event.site_count; ++site) {
        const auto before = site_features(event.before[site]);
        const auto after = site_features(event.after[site]);
        for (int f = 0; f < 4; ++f) delta[f] += after[f] - before[f];
    }
    return delta;
}

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= static_cast<std::uint64_t>(bytes[i]);
        hash *= 1099511628211ull;
    }
}

template <typename T>
void hash_value(std::uint64_t& hash, const T& value) {
    hash_bytes(hash, &value, sizeof(value));
}

std::uint64_t bridge_state_hash(const ftd::RenderBridge& rb) {
    std::uint64_t hash = 1469598103934665603ull;
    hash_value(hash, rb.current_tick());
    hash_value(hash, rb.physical_time());
    for (const auto& voxel : rb.voxels()) {
        hash_value(hash, voxel.state);
        hash_value(hash, voxel.flux.x);
        hash_value(hash, voxel.flux.y);
        hash_value(hash, voxel.flux.z);
        hash_value(hash, voxel.flux_L.x);
        hash_value(hash, voxel.flux_L.y);
        hash_value(hash, voxel.flux_L.z);
        hash_value(hash, voxel.flux_R.x);
        hash_value(hash, voxel.flux_R.y);
        hash_value(hash, voxel.flux_R.z);
        hash_value(hash, voxel.wave_vel.x);
        hash_value(hash, voxel.wave_vel.y);
        hash_value(hash, voxel.wave_vel.z);
        hash_value(hash, voxel.wave_vel_L.x);
        hash_value(hash, voxel.wave_vel_L.y);
        hash_value(hash, voxel.wave_vel_L.z);
        hash_value(hash, voxel.wave_vel_R.x);
        hash_value(hash, voxel.wave_vel_R.y);
        hash_value(hash, voxel.wave_vel_R.z);
        hash_value(hash, voxel.velocity.x);
        hash_value(hash, voxel.velocity.y);
        hash_value(hash, voxel.velocity.z);
        hash_value(hash, voxel.remainder.x);
        hash_value(hash, voxel.remainder.y);
        hash_value(hash, voxel.remainder.z);
        hash_value(hash, voxel.latency);
        hash_value(hash, voxel.tau);
        hash_value(hash, voxel.phase);
        hash_value(hash, voxel.locked);
        hash_value(hash, voxel.particle_id);
        hash_value(hash, voxel.pair_id);
        hash_value(hash, voxel.spin);
        hash_value(hash, voxel.color);
        hash_value(hash, voxel.flavor);
        hash_value(hash, voxel.accel_mag);
        hash_value(hash, voxel.flux_strong.x);
        hash_value(hash, voxel.flux_strong.y);
        hash_value(hash, voxel.flux_strong.z);
        hash_value(hash, voxel.wave_vel_strong.x);
        hash_value(hash, voxel.wave_vel_strong.y);
        hash_value(hash, voxel.wave_vel_strong.z);
        hash_value(hash, voxel.flux_weak.x);
        hash_value(hash, voxel.flux_weak.y);
        hash_value(hash, voxel.flux_weak.z);
        hash_value(hash, voxel.wave_vel_weak.x);
        hash_value(hash, voxel.wave_vel_weak.y);
        hash_value(hash, voxel.wave_vel_weak.z);
    }
    return hash;
}

ftd::eft::HistoryEvent run_movement(bool annihilate) {
    ftd::RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.movement = true;
    check("journal enables on CPU", rb.enable_history_journal());
    rb.inject_particle(3, 1, 1, +1, {0, 0, ftd::K_B});
    if (annihilate) rb.inject_particle(4, 1, 1, -1, {0, 0, -ftd::K_B});
    rb.voxel_at(3, 1, 1).velocity = {1, 0, 0};
    std::vector<ftd::eft::HistoryEvent> events;
    for (int tick = 0; tick < 4 && events.empty(); ++tick) {
        rb.tick();
        events = rb.history_events();
    }
    check(annihilate ? "annihilation event recorded" : "movement event recorded",
          events.size() == 1 && events[0].kind ==
              (annihilate ? ftd::eft::HistoryEventKind::Annihilation
                          : ftd::eft::HistoryEventKind::Movement));
    return events.empty() ? ftd::eft::HistoryEvent{} : events[0];
}

ftd::eft::HistoryEvent run_pair_production() {
    ftd::RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.pair_production = true;
    rb.toggles.langevin_seed = 7;
    check("pair journal enables", rb.enable_history_journal());
    rb.inject_flux(3, 3, 3, {1000.0 * ftd::K_GENESIS, 0, 0});
    rb.tick();
    const auto events = rb.history_events();
    check("pair-production event recorded",
          events.size() == 1 &&
          events[0].kind == ftd::eft::HistoryEventKind::PairProduction);
    return events.empty() ? ftd::eft::HistoryEvent{} : events[0];
}

ftd::eft::HistoryEvent run_genesis() {
    ftd::RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.genesis = true;
    rb.toggles.langevin_seed = 31;
    check("genesis journal enables", rb.enable_history_journal());
    rb.inject_flux(3, 3, 3, {1000.0 * ftd::K_GENESIS, 0, 0});
    rb.tick();
    const auto events = rb.history_events();
    const auto it = std::find_if(events.begin(), events.end(), [](const auto& event) {
        return event.kind == ftd::eft::HistoryEventKind::Genesis;
    });
    check("genesis event recorded", it != events.end());
    return it == events.end() ? ftd::eft::HistoryEvent{} : *it;
}

ftd::eft::HistoryEvent run_evaporation() {
    ftd::RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.evaporation = true;
    rb.toggles.langevin_seed = 37;
    check("evaporation journal enables", rb.enable_history_journal());
    rb.inject_particle(3, 3, 3, +1, {0, 0, 0});
    for (int tick = 0; tick < 128; ++tick) {
        rb.tick();
        const auto events = rb.history_events();
        if (!events.empty()) {
            check("evaporation event recorded",
                  events.size() == 1 &&
                  events[0].kind == ftd::eft::HistoryEventKind::Evaporation);
            return events[0];
        }
    }
    check("evaporation event recorded", false);
    return {};
}

ftd::eft::HistoryEvent run_weak() {
    ftd::RenderBridge rb(8);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.dual_substrate = true;
    rb.toggles.weak_transmutation = true;
    rb.toggles.langevin_seed = 11;
    check("weak journal enables", rb.enable_history_journal());
    rb.inject_particle(3, 3, 3, +1, {0.1, 0, 0});
    auto& center = rb.voxel_at(3, 3, 3);
    center.flux_L = {0.1, 0, 0};
    center.flux_R = {};
    center.flux = center.flux_L;
    auto& neighbor = rb.voxel_at(4, 3, 3);
    neighbor.flux_L = {1000.0 * ftd::WEAK_THRESHOLD, 0, 0};
    neighbor.flux_R = {};
    neighbor.flux = neighbor.flux_L;
    rb.tick();
    const auto events = rb.history_events();
    check("weak-transmutation event recorded",
          events.size() == 1 &&
          events[0].kind == ftd::eft::HistoryEventKind::WeakTransmutation);
    return events.empty() ? ftd::eft::HistoryEvent{} : events[0];
}

void check_observer_neutrality() {
    ftd::RenderBridge control(8);
    ftd::RenderBridge observed(8);
    control.force_cpu();
    observed.force_cpu();
    for (auto* rb : {&control, &observed}) {
        rb->toggles.disable_all();
        rb->toggles.pair_production = true;
        rb->toggles.movement = true;
        rb->toggles.langevin_seed = 23;
        rb->seed_rng(23);
        rb->inject_flux(3, 3, 3, {1000.0 * ftd::K_GENESIS, 0, 0});
    }
    check("observer journal enables", observed.enable_history_journal());
    for (int tick = 0; tick < 16; ++tick) {
        control.tick();
        observed.tick();
    }
    check("observer preserves full selected-state hash",
          bridge_state_hash(control) == bridge_state_hash(observed));
    check("observer preserves RNG state",
          control.rng_state_hash() == observed.rng_state_hash());
}

}  // namespace

int main() {
    std::cout << "FTD native conserved-charge gate\n";

    const auto transitions = ftd::eft::frozen_native_charge_transitions();
    const auto basis = ftd::eft::solve_conserved_charge_basis(transitions);
    check("exact transition matrix has rank four", basis.rank == 4);
    check("preregistered additive-charge nullspace is trivial",
          basis.nullity == 0 && basis.integer_basis.empty());

    const auto movement = run_movement(false);
    const auto annihilation = run_movement(true);
    const auto pair = run_pair_production();
    const auto genesis = run_genesis();
    const auto evaporation = run_evaporation();
    const auto weak = run_weak();

    check("movement preserves every preregistered global feature",
          event_delta(movement) == std::array<std::int64_t, 4>{0, 0, 0, 0});
    check("movement journal retains source mechanical velocity",
          movement.before[0].voxel.velocity.x != 0.0);
    check("movement journal retains transported target velocity",
          movement.after[1].voxel.velocity.x != 0.0
          && movement.after[0].voxel.velocity.mag2() == 0.0);
    check("legacy and complete journal state agree",
          movement.before[0].state == movement.before[0].voxel.state
          && movement.after[1].state == movement.after[1].voxel.state);
    check("annihilation preserves signed state but removes occupancy",
          event_delta(annihilation)[0] == -2 &&
          event_delta(annihilation)[1] == 0);
    check("pair production preserves signed state but creates occupancy",
          event_delta(pair)[0] == 2 && event_delta(pair)[1] == 0);
    check("single genesis creates one occupied signed state",
          event_delta(genesis)[0] == 1 &&
          std::llabs(event_delta(genesis)[1]) == 1);
    check("evaporation removes one occupied signed state",
          event_delta(evaporation)[0] == -1 &&
          std::llabs(event_delta(evaporation)[1]) == 1);
    check("weak transmutation changes signed state",
          std::llabs(event_delta(weak)[1]) == 2);

    check_observer_neutrality();

    std::cout << "\nNative charge gate: "
              << (failures == 0 ? "CLOSED NEGATIVE (trivial nullspace)" : "INVALID")
              << "\n";
    return failures;
}

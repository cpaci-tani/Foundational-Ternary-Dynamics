/**
 * @file campaign_mechanical_history_hop_work.cpp
 * @brief FTD-0449 mechanical journal sufficiency and production-hop work.
 */

#include "ftd/causal_kinematics.h"
#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int kL = 9;
constexpr int kX = 4;
constexpr int kY = 4;
constexpr int kZ = 4;
constexpr std::uint64_t kSeed = 4490;
constexpr double kGate = 1e-14;
constexpr double kNonzeroWorkGate = 1e-3;

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

void hash_vec(std::uint64_t& hash, const ftd::Vec3& value) {
  hash_value(hash, value.x);
  hash_value(hash, value.y);
  hash_value(hash, value.z);
}

std::uint64_t voxel_hash(const ftd::Voxel& voxel) {
  std::uint64_t hash = 1469598103934665603ull;
  hash_value(hash, voxel.state);
  hash_vec(hash, voxel.flux);
  hash_vec(hash, voxel.wave_vel);
  hash_vec(hash, voxel.flux_L);
  hash_vec(hash, voxel.flux_R);
  hash_vec(hash, voxel.wave_vel_L);
  hash_vec(hash, voxel.wave_vel_R);
  hash_vec(hash, voxel.velocity);
  hash_vec(hash, voxel.remainder);
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
  hash_vec(hash, voxel.flux_strong);
  hash_vec(hash, voxel.wave_vel_strong);
  hash_vec(hash, voxel.flux_weak);
  hash_vec(hash, voxel.wave_vel_weak);
  return hash;
}

std::uint64_t bridge_hash(const ftd::RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  hash_value(hash, bridge.current_tick());
  hash_value(hash, bridge.physical_time());
  for (const auto& voxel : bridge.voxels()) {
    const auto local = voxel_hash(voxel);
    hash_value(hash, local);
  }
  return hash;
}

std::uint64_t field_hash(const ftd::RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  for (const auto& voxel : bridge.voxels()) {
    hash_vec(hash, voxel.flux);
    hash_vec(hash, voxel.wave_vel);
    hash_vec(hash, voxel.flux_L);
    hash_vec(hash, voxel.flux_R);
    hash_vec(hash, voxel.wave_vel_L);
    hash_vec(hash, voxel.wave_vel_R);
  }
  return hash;
}

void configure(ftd::RenderBridge& bridge, bool journal) {
  bridge.force_cpu();
  bridge.seed_rng(kSeed);
  bridge.toggles.disable_all();
  bridge.toggles.movement = true;
  bridge.toggles.strict_validation = true;
  bridge.set_dt(1.0);
  bridge.inject_particle(kX, kY, kZ, +1, {});
  auto& particle = bridge.voxel_at(kX, kY, kZ);
  particle.velocity = {0.25, 0.0, 0.0};
  particle.remainder = {0.80, 0.0, 0.0};
  particle.particle_id = 449;
  // The source and target flux remain zero, so production's portable
  // self-field transfer is inactive.  A remote x-face value produces
  // divJ(source)=0 and divJ(target)=1 under the central stencil.
  bridge.voxel_at(kX + 2, kY, kZ).flux = {2.0, 0.0, 0.0};
  if (journal) bridge.enable_history_journal(true);
}

ftd::Voxel populated_fixture() {
  ftd::Voxel voxel;
  voxel.state = -1;
  voxel.flux = {1.0, 2.0, 3.0};
  voxel.wave_vel = {4.0, 5.0, 6.0};
  voxel.flux_L = {7.0, 8.0, 9.0};
  voxel.flux_R = {-1.0, -2.0, -3.0};
  voxel.wave_vel_L = {0.1, 0.2, 0.3};
  voxel.wave_vel_R = {-0.1, -0.2, -0.3};
  voxel.velocity = {0.11, -0.12, 0.13};
  voxel.remainder = {0.21, -0.22, 0.23};
  voxel.latency = 0.31;
  voxel.tau = 0.32;
  voxel.phase = 0.33;
  voxel.locked = true;
  voxel.particle_id = 17;
  voxel.pair_id = 19;
  voxel.spin = -1;
  voxel.color = 3;
  voxel.flavor = 2;
  voxel.accel_mag = 0.41;
  voxel.flux_strong = {0.51, 0.52, 0.53};
  voxel.wave_vel_strong = {0.61, 0.62, 0.63};
  voxel.flux_weak = {0.71, 0.72, 0.73};
  voxel.wave_vel_weak = {0.81, 0.82, 0.83};
  return voxel;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0449 mechanical history and hop work v1\n";
  std::cout << "protocol,L," << kL << ",seed," << kSeed
            << ",gate," << kGate
            << ",nonzero_work_gate," << kNonzeroWorkGate << '\n';

  const auto fixture = populated_fixture();
  const auto captured = ftd::eft::capture_history_site(37, fixture);
  const bool complete_snapshot = captured.index == 37
      && voxel_hash(captured.voxel) == voxel_hash(fixture)
      && captured.state == fixture.state
      && captured.flux.x == fixture.flux.x
      && captured.flux_L.y == fixture.flux_L.y
      && captured.flux_R.z == fixture.flux_R.z;

  ftd::RenderBridge control(kL);
  ftd::RenderBridge observed(kL);
  configure(control, false);
  configure(observed, true);
  const int source = observed.lattice().index(kX, kY, kZ);
  const int target = observed.lattice().index(kX + 1, kY, kZ);
  const double divergence_source = observed.divergence_flux(source);
  const double divergence_target = observed.divergence_flux(target);
  const double endpoint_work = ftd::eft::discrete_hop_work(
      +1, divergence_source, divergence_target);
  const double particle_energy_before = ftd::flat_particle_energy(
      observed.voxel_at(kX, kY, kZ).velocity.mag2());
  const auto field_before = field_hash(observed);

  control.tick();
  observed.tick();

  const auto events = observed.history_events();
  const bool one_movement = events.size() == 1
      && events.front().kind == ftd::eft::HistoryEventKind::Movement;
  const auto& target_voxel = observed.voxel_at(kX + 1, kY, kZ);
  const double particle_energy_after = ftd::flat_particle_energy(
      target_voxel.velocity.mag2());
  const double particle_energy_change =
      particle_energy_after - particle_energy_before;
  const double work_mismatch = particle_energy_change - endpoint_work;
  const bool field_fixed = field_hash(observed) == field_before;
  const bool observer_neutral = bridge_hash(control) == bridge_hash(observed)
      && control.rng_state_hash() == observed.rng_state_hash();

  bool event_mechanics_complete = false;
  double event_velocity_transfer_residual = 1.0;
  double event_remainder_transfer_residual = 1.0;
  if (one_movement) {
    const auto& event = events.front();
    event_velocity_transfer_residual =
        (event.after[1].voxel.velocity
         - event.before[0].voxel.velocity).mag();
    event_remainder_transfer_residual =
        (event.after[1].voxel.remainder
         - event.before[0].voxel.remainder).mag();
    event_mechanics_complete = event.before[0].index == source
        && event.after[1].index == target
        && event.before[0].voxel.particle_id == 449
        && event.after[1].voxel.particle_id == 449
        && event.after[0].voxel.state == 0
        && event.after[1].voxel.state == +1
        && event_velocity_transfer_residual <= kGate
        && event_remainder_transfer_residual <= kGate;
  }

  const bool registered_work_nonzero = std::abs(endpoint_work)
      >= kNonzeroWorkGate;
  const bool kinematic_hop = field_fixed && one_movement
      && std::abs(particle_energy_change) <= kGate
      && std::abs(work_mismatch + endpoint_work) <= kGate;

  const char* verdict = "PROTOCOL_INVALID";
  if (complete_snapshot && observer_neutral && event_mechanics_complete
      && registered_work_nonzero && kinematic_hop)
    verdict = "MECHANICAL_HISTORY_SUFFICIENT_HOP_WORK_NOT_APPLIED";
  else if (!complete_snapshot || !event_mechanics_complete)
    verdict = "MECHANICAL_HISTORY_CAPTURE_INCOMPLETE";
  else if (registered_work_nonzero
           && std::abs(work_mismatch) <= kGate)
    verdict = "PRODUCTION_HOP_APPLIES_ACTION_WORK";

  std::cout << "snapshot,complete,"
            << (complete_snapshot ? "true" : "false")
            << ",fixture_hash," << voxel_hash(fixture)
            << ",captured_hash," << voxel_hash(captured.voxel) << '\n';
  std::cout << "observer,state_rng_neutral,"
            << (observer_neutral ? "true" : "false")
            << ",events," << events.size() << '\n';
  std::cout << "movement,one_event," << (one_movement ? "true" : "false")
            << ",mechanics_complete,"
            << (event_mechanics_complete ? "true" : "false")
            << ",velocity_transfer_residual,"
            << event_velocity_transfer_residual
            << ",remainder_transfer_residual,"
            << event_remainder_transfer_residual << '\n';
  std::cout << "hop_work,divergence_source," << divergence_source
            << ",divergence_target," << divergence_target
            << ",endpoint_work," << endpoint_work
            << ",particle_energy_before," << particle_energy_before
            << ",particle_energy_after," << particle_energy_after
            << ",particle_energy_change," << particle_energy_change
            << ",work_mismatch," << work_mismatch
            << ",field_fixed," << (field_fixed ? "true" : "false") << '\n';
  std::cout << "gates,complete_snapshot,"
            << (complete_snapshot ? "true" : "false")
            << ",observer_neutral," << (observer_neutral ? "true" : "false")
            << ",event_mechanics_complete,"
            << (event_mechanics_complete ? "true" : "false")
            << ",registered_work_nonzero,"
            << (registered_work_nonzero ? "true" : "false")
            << ",kinematic_hop," << (kinematic_hop ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

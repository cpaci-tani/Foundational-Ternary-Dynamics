/**
 * @file campaign_sequential_no_reset_transactions.cpp
 * @brief FTD-0459 repeated local transactions on the actual evolved field.
 */

#include "ftd/eft/coupled_wave_tick.h"
#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/minimum_norm_transaction_selector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int kL = 33;
constexpr int kCenter = kL / 2;
constexpr int kTicks = 48;
constexpr double kInitialSpeed = 0.15;
constexpr double kInitialDressingWork = 1e-4;
constexpr double kPacketAmplitude = 0.02;
constexpr double kGate = 1e-10;
const ftd::eft::CubicVector kForward{{1, 0, 0}};

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

void copy_state(const ftd::RenderBridge& source,
                ftd::RenderBridge& target) {
  for (std::size_t index = 0; index < source.voxels().size(); ++index)
    target.voxels()[index] = source.voxels()[index];
}

int periodic_distance(int a, int b) {
  const int distance = std::abs(a - b);
  return std::min(distance, kL - distance);
}

std::vector<std::uint8_t> r1_support(
    const ftd::Lattice& lattice, int source, int target) {
  std::vector<std::uint8_t> support(lattice.total_sites(), 0);
  const auto source_coord = lattice.coord(source);
  const auto target_coord = lattice.coord(target);
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const auto near = [&](const ftd::Coord& center) {
      return periodic_distance(coordinate.x, center.x) <= 1
          && periodic_distance(coordinate.y, center.y) <= 1
          && periodic_distance(coordinate.z, center.z) <= 1;
    };
    support[static_cast<std::size_t>(index)] =
        near(source_coord) || near(target_coord) ? 1 : 0;
  }
  return support;
}

void populate_face_shape(ftd::RenderBridge& bridge, int source,
                         double scale) {
  const auto coordinate = bridge.lattice().coord(source);
  bridge.voxel_at(coordinate.x + 2, coordinate.y, coordinate.z).flux.x
      += 2.0 * scale;
}

void source_free_control(const ftd::RenderBridge& old_state,
                         ftd::RenderBridge& control) {
  copy_state(old_state, control);
  ftd::eft::advance_source_free_wave(control);
}

void apply_impulse(ftd::RenderBridge& state,
                   const std::vector<ftd::Vec3>& impulse, double sign) {
  for (std::size_t index = 0; index < impulse.size(); ++index) {
    state.voxels()[index].flux += impulse[index] * sign;
    state.voxels()[index].wave_vel += impulse[index] * sign;
  }
}

int manifestation_count(const ftd::RenderBridge& state) {
  int count = 0;
  for (const auto& voxel : state.voxels()) count += voxel.state != 0 ? 1 : 0;
  return count;
}

double full_state_residual(const ftd::RenderBridge& lhs,
                           const ftd::RenderBridge& rhs,
                           int& state_mismatches) {
  double residual = 0.0;
  state_mismatches = 0;
  for (std::size_t index = 0; index < lhs.voxels().size(); ++index) {
    const auto& a = lhs.voxels()[index];
    const auto& b = rhs.voxels()[index];
    residual = std::max(residual, (a.flux - b.flux).mag());
    residual = std::max(residual, (a.wave_vel - b.wave_vel).mag());
    residual = std::max(residual, (a.velocity - b.velocity).mag());
    if (a.state != b.state) ++state_mismatches;
  }
  return residual;
}

long double complete_event_energy_change(
    const ftd::RenderBridge& control, const ftd::RenderBridge& event,
    int source, int target, double particle_work) {
  const auto control_wave = ftd::eft::measure_native_wave_energy(control);
  const auto event_wave = ftd::eft::measure_native_wave_energy(event);
  const long double interaction_control = -static_cast<long double>(ftd::G_C)
      * static_cast<long double>(control.divergence_flux(source));
  const long double interaction_event = -static_cast<long double>(ftd::G_C)
      * static_cast<long double>(event.divergence_flux(target));
  return event_wave.tick_invariant - control_wave.tick_invariant
      + interaction_event - interaction_control
      + static_cast<long double>(particle_work);
}

struct Candidate {
  ftd::eft::CubicVector displacement{};
  int target = -1;
  double work = 0.0;
  ftd::eft::SelectedProductionHopUpdate update;
  std::vector<std::uint8_t> support;
  ftd::eft::SupportedPairedRecoilCapacity capacity;
  bool eligible = false;
};

Candidate evaluate_candidate(
    const ftd::RenderBridge& old_state, const ftd::RenderBridge& control,
    int source, const ftd::Vec3& momentum,
    const ftd::eft::CubicVector& displacement) {
  Candidate result;
  result.displacement = displacement;
  const auto coordinate = old_state.lattice().coord(source);
  result.target = old_state.lattice().index(
      coordinate.x + displacement[0], coordinate.y + displacement[1],
      coordinate.z + displacement[2]);
  result.work = ftd::eft::discrete_hop_work(
      +1, control.divergence_flux(source),
      control.divergence_flux(result.target));
  const ftd::Vec3 displacement_vector{
      static_cast<double>(displacement[0]),
      static_cast<double>(displacement[1]),
      static_cast<double>(displacement[2])};
  result.update = ftd::eft::selected_production_hop_update(
      momentum, displacement_vector, result.work);
  if (!result.update.valid) return result;
  result.support = r1_support(old_state.lattice(), source, result.target);
  result.capacity = ftd::eft::minimize_supported_paired_recoil_energy(
      old_state, control, result.target, +1,
      result.update.required_field_recoil, result.support);
  result.eligible = result.capacity.valid
      && result.capacity.minimum_total_energy_change <= 0.0L
      && result.capacity.zero_energy_solution
      && result.capacity.support_exact;
  return result;
}

struct StepRecord {
  int tick = 0;
  bool attempted = false;
  bool event = false;
  int source = -1;
  int target = -1;
  ftd::Vec3 momentum_before{};
  ftd::Vec3 momentum_after{};
  ftd::Vec3 remainder_before{};
  ftd::Vec3 remainder_after{};
  ftd::Vec3 velocity_before{};
  std::vector<ftd::Vec3> impulse;
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0459 sequential no-reset transactions v1\n";
  std::cout << "protocol,L,33,ticks,48,packet_direction,+1,packet_amplitude,"
            << kPacketAmplitude << ",initial_speed," << kInitialSpeed
            << ",initial_dressing_work," << kInitialDressingWork
            << ",support,R1,competitors,26,gate," << kGate << '\n';

  ftd::RenderBridge state(kL);
  ftd::RenderBridge initial(kL);
  configure(state);
  configure(initial);
  const int initial_source = state.lattice().index(kCenter, kCenter, kCenter);

  ftd::RenderBridge packet(kL);
  configure(packet);
  ftd::eft::LocalizedPacketSpec packet_spec;
  packet_spec.x0 = static_cast<double>(kCenter - 6);
  packet_spec.y0 = static_cast<double>(kCenter) - 3.0;
  packet_spec.z0 = static_cast<double>(kCenter);
  packet_spec.direction = +1;
  packet_spec.amplitude = kPacketAmplitude;
  ftd::eft::seed_localized_transverse_packet(packet, packet_spec);
  copy_state(packet, state);

  ftd::RenderBridge unit(kL);
  ftd::RenderBridge unit_control(kL);
  configure(unit);
  configure(unit_control);
  populate_face_shape(unit, initial_source, 1.0);
  source_free_control(unit, unit_control);
  const int initial_target = unit.lattice().index(
      kCenter + 1, kCenter, kCenter);
  const double unit_difference = unit_control.divergence_flux(initial_target)
      - unit_control.divergence_flux(initial_source);
  const double dressing_scale = kInitialDressingWork
      / (ftd::G_C * unit_difference);
  populate_face_shape(state, initial_source, dressing_scale);
  state.voxels()[static_cast<std::size_t>(initial_source)].state = +1;
  state.voxels()[static_cast<std::size_t>(initial_source)].velocity =
      {kInitialSpeed, 0.0, 0.0};
  copy_state(state, initial);

  int particle_site = initial_source;
  ftd::Vec3 momentum = ftd::eft::production_flat_momentum(
      {kInitialSpeed, 0.0, 0.0});
  const ftd::Vec3 initial_momentum = momentum;
  ftd::Vec3 remainder{};
  const ftd::Vec3 initial_remainder = remainder;
  std::vector<StepRecord> history;
  history.reserve(kTicks);

  int attempts = 0;
  int executed = 0;
  int vetoes = 0;
  int recovered_after_veto = 0;
  bool veto_seen = false;
  bool protocol_valid = true;
  int maximum_eligible_neighbors = 0;
  int minimum_eligible_neighbors = 26;
  double worst_kinematic_work = 0.0;
  long double worst_event_energy = 0.0L;
  double worst_event_momentum = 0.0;
  double worst_event_reverse = 0.0;
  long double maximum_non_event_energy_step = 0.0L;

  for (int tick = 0; tick < kTicks; ++tick) {
    StepRecord record;
    record.tick = tick;
    record.source = particle_site;
    record.momentum_before = momentum;
    record.remainder_before = remainder;
    record.velocity_before =
        state.voxels()[static_cast<std::size_t>(particle_site)].velocity;

    ftd::RenderBridge control(kL);
    configure(control);
    copy_state(state, control);
    const auto energy_before = ftd::eft::measure_native_wave_energy(state);
    ftd::eft::advance_coupled_wave_tick(control);
    const auto energy_after = ftd::eft::measure_native_wave_energy(control);
    maximum_non_event_energy_step = std::max(maximum_non_event_energy_step,
        std::abs(energy_after.tick_invariant - energy_before.tick_invariant));

    const ftd::Vec3 velocity =
        ftd::eft::production_flat_velocity_from_momentum(momentum);
    remainder += velocity;
    bool execute = false;
    Candidate forward;
    if (remainder.x >= 1.0) {
      record.attempted = true;
      ++attempts;
      int eligible_neighbors = 0;
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
          for (int dz = -1; dz <= 1; ++dz) {
            if (dx == 0 && dy == 0 && dz == 0) continue;
            const ftd::eft::CubicVector displacement{{dx, dy, dz}};
            const auto candidate = evaluate_candidate(
                state, control, particle_site, momentum, displacement);
            eligible_neighbors += candidate.eligible ? 1 : 0;
            if (displacement == kForward) forward = candidate;
          }
      maximum_eligible_neighbors = std::max(
          maximum_eligible_neighbors, eligible_neighbors);
      minimum_eligible_neighbors = std::min(
          minimum_eligible_neighbors, eligible_neighbors);
      execute = forward.eligible;
      std::cout << "attempt,tick," << tick << ",source," << particle_site
                << ",work," << forward.work
                << ",minimum_energy,"
                << static_cast<double>(
                    forward.capacity.minimum_total_energy_change)
                << ",eligible_neighbors," << eligible_neighbors
                << ",forward_eligible," << (execute ? "true" : "false")
                << '\n';
    }

    if (execute) {
      ftd::RenderBridge event(kL);
      configure(event);
      copy_state(control, event);
      apply_impulse(event, forward.capacity.zero_energy_impulse, +1.0);
      const auto certificate = ftd::eft::certify_minimum_norm_selector(
          state, control, forward.target, +1,
          forward.update.required_field_recoil, forward.support,
          forward.capacity);
      const long double event_energy = std::abs(complete_event_energy_change(
          control, event, particle_site, forward.target, forward.work));
      const auto field_recoil = ftd::eft::central_field_momentum(event)
          - ftd::eft::central_field_momentum(control);
      const double momentum_residual =
          (field_recoil - forward.update.required_field_recoil).mag();
      ftd::RenderBridge reverse_event(kL);
      configure(reverse_event);
      copy_state(event, reverse_event);
      apply_impulse(reverse_event, forward.capacity.zero_energy_impulse, -1.0);
      int reverse_state_mismatch = 0;
      const double event_reverse = full_state_residual(
          reverse_event, control, reverse_state_mismatch);
      const double kinematic_residual = std::abs(
          forward.update.work_residual);
      worst_kinematic_work = std::max(
          worst_kinematic_work, kinematic_residual);
      worst_event_energy = std::max(worst_event_energy, event_energy);
      worst_event_momentum = std::max(
          worst_event_momentum, momentum_residual);
      worst_event_reverse = std::max(worst_event_reverse, event_reverse);
      protocol_valid = protocol_valid && certificate.valid
          && certificate.selected_direction_residual <= kGate
          && certificate.norm2_bound_residual <= kGate
          && kinematic_residual <= kGate
          && event_energy <= kGate
          && momentum_residual <= kGate
          && event_reverse <= kGate
          && reverse_state_mismatch == 0;

      event.voxels()[static_cast<std::size_t>(particle_site)].state = 0;
      event.voxels()[static_cast<std::size_t>(particle_site)].velocity = {};
      event.voxels()[static_cast<std::size_t>(forward.target)].state = +1;
      momentum = forward.update.momentum_after;
      event.voxels()[static_cast<std::size_t>(forward.target)].velocity =
          ftd::eft::production_flat_velocity_from_momentum(momentum);
      remainder.x -= 1.0;
      record.event = true;
      record.target = forward.target;
      record.impulse = forward.capacity.zero_energy_impulse;
      particle_site = forward.target;
      copy_state(event, state);
      ++executed;
      if (veto_seen) ++recovered_after_veto;
    } else {
      if (record.attempted) {
        ++vetoes;
        veto_seen = true;
      }
      copy_state(control, state);
    }
    record.momentum_after = momentum;
    record.remainder_after = remainder;
    history.push_back(std::move(record));
    protocol_valid = protocol_valid && manifestation_count(state) == 1;
  }

  for (auto iterator = history.rbegin(); iterator != history.rend(); ++iterator) {
    const auto& record = *iterator;
    if (record.event) {
      apply_impulse(state, record.impulse, -1.0);
      state.voxels()[static_cast<std::size_t>(record.target)].state = 0;
      state.voxels()[static_cast<std::size_t>(record.target)].velocity = {};
      state.voxels()[static_cast<std::size_t>(record.source)].state = +1;
      state.voxels()[static_cast<std::size_t>(record.source)].velocity =
          record.velocity_before;
      particle_site = record.source;
    }
    momentum = record.momentum_before;
    remainder = record.remainder_before;
    ftd::eft::reverse_coupled_wave_tick(state);
  }

  int state_mismatches = 0;
  const double full_reverse_state = full_state_residual(
      state, initial, state_mismatches);
  const double reverse_momentum = (momentum - initial_momentum).mag();
  const double reverse_remainder = (remainder - initial_remainder).mag();
  protocol_valid = protocol_valid
      && full_reverse_state <= kGate
      && state_mismatches == 0
      && reverse_momentum <= kGate
      && reverse_remainder <= kGate;

  bool stalled_after_events = false;
  if (executed > 0 && executed <= 3 && vetoes > 0
      && recovered_after_veto == 0)
    stalled_after_events = true;
  const char* verdict = "PROTOCOL_INVALID";
  if (protocol_valid && executed >= 4)
    verdict = "SEQUENTIAL_NO_RESET_TRANSACTIONS_SELF_SUSTAINING";
  else if (protocol_valid && stalled_after_events)
    verdict = "SEQUENTIAL_TRANSACTIONS_STALL_AFTER_N";
  else if (protocol_valid && executed == 0)
    verdict = "FIRST_TRANSACTION_UNAVAILABLE";
  else if (protocol_valid)
    verdict = "SEQUENTIAL_TRANSACTIONS_INTERMITTENT";

  std::cout << "summary,ticks," << kTicks << ",attempts," << attempts
            << ",executed," << executed << ",vetoes," << vetoes
            << ",recovered_after_veto," << recovered_after_veto
            << ",minimum_eligible_neighbors," << minimum_eligible_neighbors
            << ",maximum_eligible_neighbors," << maximum_eligible_neighbors
            << ",worst_kinematic_work_residual," << worst_kinematic_work
            << ",worst_event_energy_residual,"
            << static_cast<double>(worst_event_energy)
            << ",worst_event_momentum_residual," << worst_event_momentum
            << ",worst_event_reverse_residual," << worst_event_reverse
            << ",maximum_coupled_wave_energy_step,"
            << static_cast<double>(maximum_non_event_energy_step)
            << ",full_reverse_state_residual," << full_reverse_state
            << ",state_mismatches," << state_mismatches
            << ",reverse_momentum_residual," << reverse_momentum
            << ",reverse_remainder_residual," << reverse_remainder
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}


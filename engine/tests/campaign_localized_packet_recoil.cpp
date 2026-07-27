/**
 * @file campaign_localized_packet_recoil.cpp
 * @brief FTD-0457 finite localized-packet R=1 recoil gate.
 */

#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/native_energy_contract.h"
#include "ftd/eft/supported_paired_recoil_capacity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr double kWork = 1e-4;
constexpr double kSpeed = 0.15;
constexpr double kGate = 1e-10;
constexpr double kWorkGate = 1e-12;
constexpr double kBracketGate = 1e-8;
constexpr int kBisections = 80;
constexpr double kHighAmplitude = 1.0;
constexpr int kPacketOffset = 6;
constexpr double kSigmaX = 3.0;
constexpr double kSigmaT = 3.0;
constexpr int kResidueTicks = 8;
const std::array<int, 3> kLengths{{33, 49, 65}};
const std::array<int, 2> kDirections{{-1, +1}};
const std::array<int, 3> kSampleTicks{{0, 8, 16}};

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

void copy_wave(const ftd::RenderBridge& source, ftd::RenderBridge& target) {
  for (std::size_t index = 0; index < source.voxels().size(); ++index) {
    target.voxels()[index].flux = source.voxels()[index].flux;
    target.voxels()[index].wave_vel = source.voxels()[index].wave_vel;
  }
}

void populate_minimal_face_shape(ftd::RenderBridge& bridge, double scale) {
  const int center = bridge.lattice().size() / 2;
  bridge.voxel_at(center + 2, center, center).flux.x = 2.0 * scale;
}

void make_control(const ftd::RenderBridge& old_state,
                  ftd::RenderBridge& control) {
  copy_wave(old_state, control);
  ftd::eft::advance_source_free_wave(control);
}

int periodic_distance(int a, int b, int length) {
  const int distance = std::abs(a - b);
  return std::min(distance, length - distance);
}

std::vector<std::uint8_t> make_r1_support(
    const ftd::Lattice& lattice, int source, int target) {
  std::vector<std::uint8_t> support(lattice.total_sites(), 0);
  const auto source_coord = lattice.coord(source);
  const auto target_coord = lattice.coord(target);
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const auto near = [&](const ftd::Coord& center) {
      return periodic_distance(coordinate.x, center.x, lattice.size()) <= 1
          && periodic_distance(coordinate.y, center.y, lattice.size()) <= 1
          && periodic_distance(coordinate.z, center.z, lattice.size()) <= 1;
    };
    support[static_cast<std::size_t>(index)] =
        near(source_coord) || near(target_coord) ? 1 : 0;
  }
  return support;
}

void apply_impulse(ftd::RenderBridge& state,
                   const std::vector<ftd::Vec3>& impulse, double sign = 1.0) {
  for (std::size_t index = 0; index < impulse.size(); ++index) {
    state.voxels()[index].flux += impulse[index] * sign;
    state.voxels()[index].wave_vel += impulse[index] * sign;
  }
}

long double complete_event_energy_change(
    const ftd::RenderBridge& control, const ftd::RenderBridge& event,
    int source, int target) {
  const auto control_wave = ftd::eft::measure_native_wave_energy(control);
  const auto event_wave = ftd::eft::measure_native_wave_energy(event);
  const long double interaction_control = -static_cast<long double>(ftd::G_C)
      * static_cast<long double>(control.divergence_flux(source));
  const long double interaction_event = -static_cast<long double>(ftd::G_C)
      * static_cast<long double>(event.divergence_flux(target));
  return event_wave.tick_invariant - control_wave.tick_invariant
      + interaction_event - interaction_control
      + static_cast<long double>(kWork);
}

double maximum_divergence(const ftd::RenderBridge& state) {
  double maximum = 0.0;
  for (int index = 0; index < static_cast<int>(state.voxels().size()); ++index)
    maximum = std::max(maximum, std::abs(state.divergence_flux(index)));
  return maximum;
}

struct Snapshot {
  int tick = 0;
  std::vector<ftd::Vec3> flux;
  std::vector<ftd::Vec3> wave_vel;
};

struct PacketFamily {
  int length = 0;
  int direction = 1;
  double unit_energy = 0.0;
  double energy_drift = 0.0;
  double divergence = 0.0;
  double packet_reverse_residual = 0.0;
  std::vector<Snapshot> snapshots;

  PacketFamily(int length_in, int direction_in)
      : length(length_in), direction(direction_in) {
    ftd::RenderBridge packet(length);
    ftd::RenderBridge initial(length);
    configure(packet);
    configure(initial);
    const int center = length / 2;
    ftd::eft::LocalizedPacketSpec spec;
    spec.x0 = static_cast<double>(center - direction * kPacketOffset);
    spec.y0 = static_cast<double>(center) - kSigmaT;
    spec.z0 = static_cast<double>(center);
    spec.sigma_x = kSigmaX;
    spec.sigma_t = kSigmaT;
    spec.direction = direction;
    ftd::eft::seed_localized_transverse_packet(packet, spec);
    copy_wave(packet, initial);
    divergence = maximum_divergence(packet);
    const auto energy0 = ftd::eft::measure_native_wave_energy(packet);
    unit_energy = static_cast<double>(energy0.tick_invariant);

    int sample_cursor = 0;
    for (int tick = 0; tick <= kSampleTicks.back(); ++tick) {
      if (sample_cursor < static_cast<int>(kSampleTicks.size())
          && tick == kSampleTicks[static_cast<std::size_t>(sample_cursor)]) {
        Snapshot snapshot;
        snapshot.tick = tick;
        snapshot.flux.resize(packet.voxels().size());
        snapshot.wave_vel.resize(packet.voxels().size());
        for (std::size_t index = 0; index < packet.voxels().size(); ++index) {
          snapshot.flux[index] = packet.voxels()[index].flux;
          snapshot.wave_vel[index] = packet.voxels()[index].wave_vel;
        }
        snapshots.push_back(std::move(snapshot));
        ++sample_cursor;
      }
      if (tick < kSampleTicks.back())
        ftd::eft::advance_source_free_wave(packet);
    }
    const auto energy1 = ftd::eft::measure_native_wave_energy(packet);
    energy_drift = std::abs(static_cast<double>(
        energy1.tick_invariant - energy0.tick_invariant));
    for (int tick = 0; tick < kSampleTicks.back(); ++tick)
      ftd::eft::reverse_source_free_wave(packet);
    packet_reverse_residual =
        ftd::eft::wave_state_max_residual(packet, initial);
  }
};

struct ArmState {
  int length = 0;
  int direction = 1;
  int tick = 0;
  int source = 0;
  int target = 0;
  double unit_energy = 0.0;
  ftd::Vec3 requested_recoil{};
  std::vector<ftd::Vec3> base_flux;
  const Snapshot& packet;
  std::vector<std::uint8_t> support;
  ftd::RenderBridge old_state;
  ftd::RenderBridge control;

  ArmState(const PacketFamily& family, const Snapshot& snapshot)
      : length(family.length), direction(family.direction), tick(snapshot.tick),
        unit_energy(family.unit_energy), packet(snapshot), old_state(length),
        control(length) {
    configure(old_state);
    configure(control);
    const int center = length / 2;
    source = old_state.lattice().index(center, center, center);
    target = old_state.lattice().index(center + 1, center, center);
    populate_minimal_face_shape(old_state, 1.0);
    make_control(old_state, control);
    const double unit_difference = control.divergence_flux(target)
        - control.divergence_flux(source);
    const double base_scale = kWork / (ftd::G_C * unit_difference);
    for (auto& voxel : old_state.voxels()) voxel = {};
    populate_minimal_face_shape(old_state, base_scale);
    base_flux.resize(old_state.voxels().size());
    for (std::size_t index = 0; index < old_state.voxels().size(); ++index)
      base_flux[index] = old_state.voxels()[index].flux;
    support = make_r1_support(old_state.lattice(), source, target);
    const ftd::eft::CubicVector displacement{{1, 0, 0}};
    const auto momentum = ftd::eft::production_flat_momentum({kSpeed, 0, 0});
    requested_recoil = ftd::eft::make_half_tick_link_exchange(
        17, momentum, displacement, kWork).field_momentum_exchange;
  }

  ftd::eft::SupportedPairedRecoilCapacity evaluate(double amplitude) {
    for (std::size_t index = 0; index < old_state.voxels().size(); ++index) {
      old_state.voxels()[index].flux =
          base_flux[index] + packet.flux[index] * amplitude;
      old_state.voxels()[index].wave_vel =
          packet.wave_vel[index] * amplitude;
      control.voxels()[index] = {};
    }
    make_control(old_state, control);
    return ftd::eft::minimize_supported_paired_recoil_energy(
        old_state, control, target, +1, requested_recoil, support);
  }
};

struct Result {
  int length = 0;
  int direction = 1;
  int tick = 0;
  bool bracketed = false;
  bool valid = false;
  double threshold = 0.0;
  double threshold_energy = 0.0;
  double energy_residual = 0.0;
  double momentum_residual = 0.0;
  double outgoing_fraction = 0.0;
  double event_reverse_residual = 0.0;
};

Result run_arm(ArmState& arm) {
  Result result;
  result.length = arm.length;
  result.direction = arm.direction;
  result.tick = arm.tick;
  const auto low_capacity = arm.evaluate(0.0);
  const auto high_capacity = arm.evaluate(kHighAmplitude);
  result.valid = low_capacity.valid && high_capacity.valid;
  result.bracketed = result.valid
      && low_capacity.minimum_total_energy_change > kBracketGate
      && high_capacity.minimum_total_energy_change < -kBracketGate;
  if (!result.bracketed) return result;

  double low = 0.0;
  double high = kHighAmplitude;
  for (int iteration = 0; iteration < kBisections; ++iteration) {
    const double middle = 0.5 * (low + high);
    const auto capacity = arm.evaluate(middle);
    result.valid = result.valid && capacity.valid;
    if (capacity.minimum_total_energy_change <= 0.0L) high = middle;
    else low = middle;
  }
  result.threshold = high;
  result.threshold_energy = high * high * arm.unit_energy;
  const auto capacity = arm.evaluate(high);
  result.valid = result.valid && capacity.valid
      && capacity.zero_energy_solution && capacity.support_exact
      && std::abs(capacity.minimum_total_energy_change) <= kGate;

  ftd::RenderBridge event(arm.length);
  ftd::RenderBridge event_initial(arm.length);
  ftd::RenderBridge old_initial(arm.length);
  configure(event);
  configure(event_initial);
  configure(old_initial);
  copy_wave(arm.control, event);
  apply_impulse(event, capacity.zero_energy_impulse);
  copy_wave(event, event_initial);
  copy_wave(arm.old_state, old_initial);
  result.energy_residual = static_cast<double>(complete_event_energy_change(
      arm.control, event, arm.source, arm.target));
  const auto recoil = ftd::eft::central_field_momentum(event)
      - ftd::eft::central_field_momentum(arm.control);
  result.momentum_residual = (recoil - arm.requested_recoil).mag();

  ftd::RenderBridge control_evolved(arm.length);
  configure(control_evolved);
  copy_wave(arm.control, control_evolved);
  for (int tick = 0; tick < kResidueTicks; ++tick) {
    ftd::eft::advance_source_free_wave(event);
    ftd::eft::advance_source_free_wave(control_evolved);
  }
  long double total_norm2 = 0.0L;
  long double outside_norm2 = 0.0L;
  for (std::size_t index = 0; index < event.voxels().size(); ++index) {
    const auto delta_j = event.voxels()[index].flux
        - control_evolved.voxels()[index].flux;
    const auto delta_w = event.voxels()[index].wave_vel
        - control_evolved.voxels()[index].wave_vel;
    const long double site = ftd::eft::dot_long_double(delta_j, delta_j)
        + ftd::eft::dot_long_double(delta_w, delta_w);
    total_norm2 += site;
    if (arm.support[index] == 0) outside_norm2 += site;
  }
  result.outgoing_fraction = total_norm2 > 0.0L
      ? static_cast<double>(outside_norm2 / total_norm2) : 0.0;

  for (int tick = 0; tick < kResidueTicks; ++tick)
    ftd::eft::reverse_source_free_wave(event);
  double reverse_residual = ftd::eft::wave_state_max_residual(
      event, event_initial);
  apply_impulse(event, capacity.zero_energy_impulse, -1.0);
  ftd::eft::reverse_source_free_wave(event);
  reverse_residual = std::max(reverse_residual,
      ftd::eft::wave_state_max_residual(event, old_initial));
  result.event_reverse_residual = reverse_residual;

  const double measured_work = ftd::eft::discrete_hop_work(
      +1, arm.control.divergence_flux(arm.source),
      arm.control.divergence_flux(arm.target));
  result.valid = result.valid
      && std::abs(result.energy_residual) <= kGate
      && result.momentum_residual <= kGate
      && std::abs(measured_work - kWork) <= kWorkGate
      && result.outgoing_fraction >= 0.05
      && result.event_reverse_residual <= kGate;
  return result;
}

double coefficient_of_variation(const std::vector<double>& values) {
  double mean = 0.0;
  for (double value : values) mean += value;
  mean /= static_cast<double>(values.size());
  double variance = 0.0;
  for (double value : values) variance += (value - mean) * (value - mean);
  variance /= static_cast<double>(values.size());
  return mean != 0.0 ? std::sqrt(variance) / std::abs(mean) : 0.0;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0457 localized-packet recoil gate v1\n";
  std::cout << "protocol,lengths,33|49|65,directions,-1|1,ticks,0|8|16,"
            << "sigma_x," << kSigmaX << ",sigma_t," << kSigmaT
            << ",carrier,pi_over_4,offset," << kPacketOffset
            << ",support,R1,work," << kWork << ",speed," << kSpeed
            << ",bracket,0|1,bisections," << kBisections << '\n';

  std::vector<Result> results;
  std::vector<double> unit_energies;
  bool protocol_valid = true;
  double worst_divergence = 0.0;
  double worst_packet_energy_drift = 0.0;
  double worst_packet_reverse = 0.0;
  for (int length : kLengths)
    for (int direction : kDirections) {
      PacketFamily family(length, direction);
      unit_energies.push_back(family.unit_energy);
      worst_divergence = std::max(worst_divergence, family.divergence);
      worst_packet_energy_drift = std::max(
          worst_packet_energy_drift, family.energy_drift);
      worst_packet_reverse = std::max(
          worst_packet_reverse, family.packet_reverse_residual);
      protocol_valid = protocol_valid
          && family.divergence <= kWorkGate
          && family.energy_drift <= kGate
          && family.packet_reverse_residual <= kGate;
      std::cout << "packet,L," << length << ",direction," << direction
                << ",unit_energy," << family.unit_energy
                << ",divergence," << family.divergence
                << ",energy_drift," << family.energy_drift
                << ",reverse_residual," << family.packet_reverse_residual
                << '\n';
      for (const auto& snapshot : family.snapshots) {
        ArmState arm(family, snapshot);
        results.push_back(run_arm(arm));
      }
    }

  std::vector<double> selected_energies;
  std::array<double, 3> volume_energy_sum{};
  std::array<int, 3> volume_energy_count{};
  int family_crossings = 0;
  double worst_energy = 0.0;
  double worst_momentum = 0.0;
  double worst_event_reverse = 0.0;
  double smallest_outgoing = 1.0;
  for (const auto& result : results) {
    if (result.bracketed) {
      worst_energy = std::max(worst_energy, std::abs(result.energy_residual));
      worst_momentum = std::max(worst_momentum, result.momentum_residual);
      worst_event_reverse = std::max(
          worst_event_reverse, result.event_reverse_residual);
      smallest_outgoing = std::min(
          smallest_outgoing, result.outgoing_fraction);
    }
    protocol_valid = protocol_valid && result.valid;
    std::cout << "arm,L," << result.length << ",direction,"
              << result.direction << ",tick," << result.tick
              << ",bracketed," << (result.bracketed ? "true" : "false")
              << ",threshold," << result.threshold
              << ",threshold_energy," << result.threshold_energy
              << ",energy_residual," << result.energy_residual
              << ",momentum_residual," << result.momentum_residual
              << ",outgoing_fraction," << result.outgoing_fraction
              << ",event_reverse_residual," << result.event_reverse_residual
              << ",valid," << (result.valid ? "true" : "false") << '\n';
  }

  for (int length_index = 0;
       length_index < static_cast<int>(kLengths.size()); ++length_index) {
    const int length = kLengths[static_cast<std::size_t>(length_index)];
    for (int direction : kDirections) {
      const Result* best = nullptr;
      for (const auto& result : results) {
        if (result.length != length || result.direction != direction
            || !result.bracketed || !result.valid) continue;
        if (best == nullptr || result.threshold_energy < best->threshold_energy)
          best = &result;
      }
      if (best == nullptr) continue;
      ++family_crossings;
      selected_energies.push_back(best->threshold_energy);
      volume_energy_sum[static_cast<std::size_t>(length_index)]
          += best->threshold_energy;
      ++volume_energy_count[static_cast<std::size_t>(length_index)];
      std::cout << "selected,L," << length << ",direction," << direction
                << ",tick," << best->tick << ",threshold," << best->threshold
                << ",threshold_energy," << best->threshold_energy << '\n';
    }
  }

  const double unit_energy_cv = coefficient_of_variation(unit_energies);
  const double threshold_energy_cv = selected_energies.empty()
      ? INFINITY : coefficient_of_variation(selected_energies);
  std::array<double, 3> volume_means{};
  for (std::size_t index = 0; index < volume_means.size(); ++index)
    if (volume_energy_count[index] > 0)
      volume_means[index] = volume_energy_sum[index]
          / static_cast<double>(volume_energy_count[index]);
  const double high_volume_relative_difference =
      volume_means[1] > 0.0 && volume_means[2] > 0.0
      ? std::abs(volume_means[1] - volume_means[2])
          / (0.5 * (volume_means[1] + volume_means[2]))
      : INFINITY;
  const bool all_cross = family_crossings == 6;
  const bool volume_stable = unit_energy_cv <= 0.01
      && threshold_energy_cv <= 0.10
      && high_volume_relative_difference <= 0.05;

  const char* verdict = "PROTOCOL_INVALID";
  if (protocol_valid && all_cross && volume_stable)
    verdict = "LOCALIZED_PACKET_R1_THRESHOLD_VOLUME_STABLE";
  else if (protocol_valid && all_cross)
    verdict = "LOCALIZED_PACKET_R1_THRESHOLD_VOLUME_UNSTABLE";
  else if (protocol_valid && family_crossings == 0)
    verdict = "NO_LOCALIZED_PACKET_R1_THRESHOLD";
  else if (protocol_valid)
    verdict = "MIXED_LOCALIZED_PACKET_R1_THRESHOLD";

  std::cout << "summary,families,6,family_crossings," << family_crossings
            << ",unit_energy_cv," << unit_energy_cv
            << ",threshold_energy_cv," << threshold_energy_cv
            << ",high_volume_relative_difference,"
            << high_volume_relative_difference
            << ",worst_divergence," << worst_divergence
            << ",worst_packet_energy_drift," << worst_packet_energy_drift
            << ",worst_packet_reverse," << worst_packet_reverse
            << ",worst_energy_residual," << worst_energy
            << ",worst_momentum_residual," << worst_momentum
            << ",smallest_outgoing_fraction," << smallest_outgoing
            << ",worst_event_reverse," << worst_event_reverse
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}


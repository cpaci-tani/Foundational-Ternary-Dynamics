#include "ftd/eft/endogenous_reaction_carrier_bound.h"

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

constexpr double TOL = 1e-12;
constexpr int RUN_TICKS = 128;
constexpr std::array<int, 4> SPECTRAL_VOLUMES{{9, 17, 33, 65}};
constexpr std::array<int, 2> LIVE_VOLUMES{{9, 17}};
constexpr std::array<Coord, 6> DIRECTIONS{{
    {1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
    {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Coord add(Coord lhs, Coord rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Coord scale(Coord value, int factor) {
  return {factor * value.x, factor * value.y, factor * value.z};
}

EndogenousReactionCarrierVolume spectral_bound(int L) {
  EndogenousReactionCarrierVolume result;
  result.lattice_size = L;
  const long double pi = std::acos(-1.0L);
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  long double sum = 0.0L;
  long double maximum_a = 0.0L;
  for (int nx = 0; nx < L; ++nx) {
    const long double kx = 2.0L * pi * nx / L;
    const long double cx = std::cos(kx);
    const long double sx = std::sin(kx);
    for (int ny = 0; ny < L; ++ny) {
      const long double ky = 2.0L * pi * ny / L;
      const long double cy = std::cos(ky);
      const long double sy = std::sin(ky);
      for (int nz = 0; nz < L; ++nz) {
        const long double kz = 2.0L * pi * nz / L;
        const long double cz = std::cos(kz);
        const long double sz = std::sin(kz);
        const long double symbol = 4.0L
            - (2.0L / 3.0L) * (cx + cy + cz)
            - (2.0L / 3.0L) * (cx * cy + cx * cz + cy * cz);
        if (symbol <= 1e-24L) continue;
        const long double a = c2 * symbol;
        maximum_a = std::max(maximum_a, a);
        const long double mode_bound =
            1.0L + 1.0L / std::sqrt(1.0L - a / 4.0L);
        const long double gradient_symbol =
            std::sqrt(sx * sx + sy * sy + sz * sz);
        sum += mode_bound * gradient_symbol / symbol;
      }
    }
  }
  const long double sites = static_cast<long double>(L) * L * L;
  const long double step = static_cast<long double>(G_C) * sum / (c2 * sites);
  const long double pulse = 2.0L * step;
  result.maximum_mode_eigenvalue = static_cast<double>(maximum_a);
  result.single_source_step_bound = static_cast<double>(step);
  result.single_source_pulse_bound = static_cast<double>(pulse);
  result.three_source_pulse_bound = static_cast<double>(3.0L * pulse);
  result.threshold_margin = K_GENESIS - result.three_source_pulse_bound;
  return result;
}

struct LiveArm {
  bool valid = false;
  int genesis_events = 0;
  int evaporation_events = 0;
  double maximum_flux = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  bool support_subset = true;
};

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

std::uint64_t selected_state_hash(const RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  hash_value(hash, bridge.current_tick());
  hash_value(hash, bridge.physical_time());
  for (const auto& voxel : bridge.voxels()) {
    hash_value(hash, voxel.state);
    for (const auto& value : {voxel.flux, voxel.wave_vel, voxel.flux_L,
                             voxel.flux_R, voxel.wave_vel_L,
                             voxel.wave_vel_R, voxel.velocity,
                             voxel.remainder}) {
      hash_value(hash, value.x);
      hash_value(hash, value.y);
      hash_value(hash, value.z);
    }
    hash_value(hash, voxel.locked);
    hash_value(hash, voxel.particle_id);
    hash_value(hash, voxel.spin);
    hash_value(hash, voxel.color);
  }
  return hash;
}

void configure_neutrality_bridge(RenderBridge& bridge, std::uint32_t seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = true;
  bridge.toggles.genesis = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  bridge.toggles.langevin_seed = seed;
  bridge.seed_rng(seed);
  constexpr int L = 9;
  for (int dx : {-1, 0, 1}) {
    bridge.inject_particle(L / 2 + dx, L / 2, L / 2, 1, Vec3{});
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(
        bridge.lattice().index(L / 2 + dx, L / 2, L / 2))];
    voxel.velocity = {};
    voxel.remainder = {};
    voxel.locked = false;
  }
}

bool observer_neutrality() {
  constexpr std::uint32_t seed = 0x0586AA55u;
  RenderBridge control(9);
  RenderBridge observed(9);
  configure_neutrality_bridge(control, seed);
  configure_neutrality_bridge(observed, seed);
  if (!observed.enable_history_journal(true)) return false;
  for (int tick = 0; tick < 32; ++tick) {
    control.tick();
    observed.tick();
  }
  return selected_state_hash(control) == selected_state_hash(observed)
      && control.rng_state_hash() == observed.rng_state_hash();
}

LiveArm run_live_arm(int L, int polarity, Coord direction,
                     int source_count, bool locked,
                     std::uint32_t seed, double pulse_bound) {
  LiveArm result;
  RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = true;
  bridge.toggles.genesis = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  bridge.toggles.langevin_seed = seed;
  bridge.seed_rng(seed);
  if (bridge.backend_kind() != Backend::Kind::Cpu
      || !bridge.toggles.validate()) return result;

  const Coord center{L / 2, L / 2, L / 2};
  std::vector<unsigned char> initial(bridge.voxels().size(), 0);
  std::array<Coord, 3> sites{{center, center, center}};
  if (source_count == 1) {
    sites[0] = add(center, direction);
  } else {
    sites[0] = add(center, scale(direction, -1));
    sites[1] = center;
    sites[2] = add(center, direction);
  }
  for (int j = 0; j < source_count; ++j) {
    const Coord site = sites[static_cast<std::size_t>(j)];
    const int index = bridge.lattice().index(site.x, site.y, site.z);
    bridge.inject_particle(site.x, site.y, site.z,
        static_cast<std::int8_t>(polarity), Vec3{});
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = {};
    voxel.wave_vel = {};
    voxel.velocity = {};
    voxel.remainder = {};
    voxel.locked = locked;
    initial[static_cast<std::size_t>(index)] = 1;
  }
  if (!bridge.enable_history_journal(true)) return result;

  for (int tick = 0; tick < RUN_TICKS; ++tick) {
    bridge.tick();
    for (const auto& event : bridge.history_events()) {
      if (event.kind == HistoryEventKind::Genesis) ++result.genesis_events;
      if (event.kind == HistoryEventKind::Evaporation)
        ++result.evaporation_events;
    }
    for (std::size_t i = 0; i < bridge.voxels().size(); ++i) {
      const auto& voxel = bridge.voxels()[i];
      result.maximum_flux = std::max(result.maximum_flux, voxel.flux.mag());
      result.maximum_velocity = std::max(
          result.maximum_velocity, max_abs(voxel.velocity));
      result.maximum_remainder = std::max(
          result.maximum_remainder, max_abs(voxel.remainder));
      if (voxel.state != 0 && initial[i] == 0) result.support_subset = false;
    }
  }
  result.valid = result.genesis_events == 0
      && result.support_subset
      && result.maximum_flux <= source_count * pulse_bound + TOL
      && result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0;
  return result;
}

int run_external_control(std::uint32_t seed) {
  constexpr int L = 9;
  RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.genesis = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.langevin_seed = seed;
  bridge.seed_rng(seed);
  const Coord center{L / 2, L / 2, L / 2};
  bridge.voxels()[static_cast<std::size_t>(bridge.lattice().index(
      center.x, center.y, center.z))].flux = {100.0, 0.0, 0.0};
  if (!bridge.enable_history_journal(true)) return -1;
  bridge.tick();
  int events = 0;
  for (const auto& event : bridge.history_events())
    if (event.kind == HistoryEventKind::Genesis) ++events;
  return events;
}

}  // namespace

EndogenousReactionCarrierBoundResult
analyze_endogenous_reaction_carrier_bound() {
  EndogenousReactionCarrierBoundResult result;
  result.minimum_threshold_margin = std::numeric_limits<double>::infinity();

  bool spectral_valid = true;
  for (std::size_t i = 0; i < SPECTRAL_VOLUMES.size(); ++i) {
    result.volumes[i] = spectral_bound(SPECTRAL_VOLUMES[i]);
    const auto& volume = result.volumes[i];
    result.maximum_single_source_pulse_bound = std::max(
        result.maximum_single_source_pulse_bound,
        volume.single_source_pulse_bound);
    result.maximum_three_source_pulse_bound = std::max(
        result.maximum_three_source_pulse_bound,
        volume.three_source_pulse_bound);
    result.minimum_threshold_margin = std::min(
        result.minimum_threshold_margin, volume.threshold_margin);
    spectral_valid = spectral_valid
        && volume.maximum_mode_eigenvalue > 0.0
        && volume.maximum_mode_eigenvalue < 4.0
        && volume.single_source_step_bound > 0.0
        && volume.single_source_pulse_bound
            == 2.0 * volume.single_source_step_bound
        && volume.threshold_margin > 0.0;
    ++result.spectral_volume_count;
  }
  result.modal_step_bound_derived = spectral_valid;
  result.rectangular_pulse_bound_derived = spectral_valid;
  result.maximum_initial_sources_closed = static_cast<int>(std::floor(
      (K_GENESIS - TOL) / result.maximum_single_source_pulse_bound));
  result.minimum_sources_not_excluded =
      result.maximum_initial_sources_closed + 1;
  result.no_first_genesis_for_three_sources = spectral_valid
      && result.maximum_initial_sources_closed >= 3
      && result.maximum_three_source_pulse_bound < K_GENESIS;

  bool live_valid = true;
  bool support_subset = true;
  std::uint32_t seed = 0x05860000u;
  for (int L : LIVE_VOLUMES) {
    const auto volume_it = std::find_if(
        result.volumes.begin(), result.volumes.end(),
        [L](const auto& volume) { return volume.lattice_size == L; });
    if (volume_it == result.volumes.end()) {
      live_valid = false;
      continue;
    }
    for (int polarity : {-1, 1}) {
      for (Coord direction : DIRECTIONS) {
        for (int source_count : {1, 3}) {
          for (bool locked : {true, false}) {
            const LiveArm arm = run_live_arm(
                L, polarity, direction, source_count, locked, seed++,
                volume_it->single_source_pulse_bound);
            live_valid = live_valid && arm.valid;
            support_subset = support_subset && arm.support_subset;
            result.endogenous_genesis_events += arm.genesis_events;
            result.endogenous_evaporation_events += arm.evaporation_events;
            result.maximum_observed_flux = std::max(
                result.maximum_observed_flux, arm.maximum_flux);
            result.maximum_bound_excess = std::max(
                result.maximum_bound_excess,
                arm.maximum_flux - source_count
                    * volume_it->single_source_pulse_bound);
            result.maximum_velocity = std::max(
                result.maximum_velocity, arm.maximum_velocity);
            result.maximum_remainder = std::max(
                result.maximum_remainder, arm.maximum_remainder);
            ++result.endogenous_arms;
            result.endogenous_ticks += RUN_TICKS;
            if (locked) ++result.constant_source_arms;
            else ++result.pulse_source_arms;
          }
        }
      }
    }
  }
  result.manifested_support_remained_subset = support_subset;
  result.pulse_removal_exercised = result.endogenous_evaporation_events > 0;
  result.void_kinematics_sanitized = result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0;
  result.observer_neutral = observer_neutrality();

  bool controls_valid = true;
  for (int arm = 0; arm < 4; ++arm) {
    const int events = run_external_control(0x0586F000u + arm);
    controls_valid = controls_valid && events >= 1;
    result.external_control_genesis_events += std::max(events, 0);
    ++result.external_control_arms;
  }
  result.external_genesis_control_live = controls_valid;

  result.four_sources_sufficient = false;
  result.self_sustaining_reaction_carrier_established = false;
  result.production_changed = false;
  result.valid = result.modal_step_bound_derived
      && result.rectangular_pulse_bound_derived
      && result.no_first_genesis_for_three_sources
      && result.minimum_sources_not_excluded == 4
      && result.maximum_initial_sources_closed == 3
      && live_valid
      && result.endogenous_arms == 96
      && result.endogenous_ticks == 96 * RUN_TICKS
      && result.constant_source_arms == 48
      && result.pulse_source_arms == 48
      && result.endogenous_genesis_events == 0
      && result.manifested_support_remained_subset
      && result.pulse_removal_exercised
      && result.maximum_bound_excess <= TOL
      && result.void_kinematics_sanitized
      && result.observer_neutral
      && result.external_genesis_control_live
      && !result.four_sources_sufficient
      && !result.self_sustaining_reaction_carrier_established
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft

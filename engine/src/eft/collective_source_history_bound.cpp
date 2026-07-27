#include "ftd/eft/collective_source_history_bound.h"

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

constexpr double TOL = 1e-12;
constexpr int RUN_TICKS = 128;
constexpr int SYNCHRONOUS_REMOVAL_TICK = 16;
constexpr std::array<int, 4> SPECTRAL_VOLUMES{{9, 17, 33, 65}};
constexpr std::array<int, 2> LIVE_VOLUMES{{9, 17}};

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Coord add(Coord lhs, Coord rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

std::vector<Coord> tetrahedral_sites(int L, int source_count,
                                     int chirality, int translation) {
  const Coord center{L / 2 + translation, L / 2, L / 2};
  const std::array<Coord, 4> positive{{
      {1, 1, 1}, {1, -1, -1}, {-1, 1, -1}, {-1, -1, 1}}};
  const std::array<Coord, 4> negative{{
      {1, 1, -1}, {1, -1, 1}, {-1, 1, 1}, {-1, -1, -1}}};
  const auto& orbit = chirality > 0 ? positive : negative;
  std::vector<Coord> sites;
  sites.reserve(static_cast<std::size_t>(source_count));
  for (Coord offset : orbit) sites.push_back(add(center, offset));
  if (source_count == 5) sites.push_back(center);
  return sites;
}

double parseval_error(int L, const std::vector<Coord>& sites,
                      int polarity) {
  const long double pi = std::acos(-1.0L);
  long double power = 0.0L;
  for (int nx = 0; nx < L; ++nx) {
    const long double kx = 2.0L * pi * nx / L;
    for (int ny = 0; ny < L; ++ny) {
      const long double ky = 2.0L * pi * ny / L;
      for (int nz = 0; nz < L; ++nz) {
        const long double kz = 2.0L * pi * nz / L;
        long double real = 0.0L;
        long double imag = 0.0L;
        for (Coord site : sites) {
          const long double phase = kx * site.x + ky * site.y + kz * site.z;
          real += polarity * std::cos(phase);
          imag -= polarity * std::sin(phase);
        }
        power += real * real + imag * imag;
      }
    }
  }
  const long double sites_in_volume =
      static_cast<long double>(L) * L * L;
  return static_cast<double>(std::abs(
      power / sites_in_volume - static_cast<long double>(sites.size())));
}

CollectiveSourceHistoryVolume spectral_bound(int L,
                                              bool& dominance_valid) {
  CollectiveSourceHistoryVolume result;
  result.lattice_size = L;
  const long double pi = std::acos(-1.0L);
  const long double c2 = static_cast<long double>(C_WAVE) * C_WAVE;
  long double triangle_sum = 0.0L;
  long double common_sum = 0.0L;
  long double maximum_a = 0.0L;
  long double maximum_ratio = 0.0L;
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
        const long double gradient2 = sx * sx + sy * sy + sz * sz;
        const long double p = cx + cy + cz;
        const long double q = cx * cx + cy * cy + cz * cz;
        const long double identity_rhs =
            4.0L * (q - p * p / 3.0L) + (p - 3.0L) * (p - 3.0L) / 3.0L;
        const long double identity_lhs = 3.0L * (symbol - gradient2);
        dominance_valid = dominance_valid
            && identity_rhs >= -1e-17L
            && std::abs(identity_lhs - identity_rhs) <= 2e-14L
            && gradient2 <= symbol + 1e-17L;
        maximum_ratio = std::max(maximum_ratio, gradient2 / symbol);

        const long double a = c2 * symbol;
        maximum_a = std::max(maximum_a, a);
        const long double mode_bound =
            1.0L + 1.0L / std::sqrt(1.0L - a / 4.0L);
        triangle_sum += mode_bound * std::sqrt(gradient2) / symbol;
        common_sum += mode_bound * mode_bound / symbol;
      }
    }
  }

  const long double volume = static_cast<long double>(L) * L * L;
  const long double coupling = static_cast<long double>(G_C);
  const long double one_step = coupling * triangle_sum / (c2 * volume);
  const long double common_step =
      coupling / c2 * std::sqrt(common_sum / volume);
  const long double common_five = 2.0L * common_step * std::sqrt(5.0L);
  const long double common_six = 2.0L * common_step * std::sqrt(6.0L);
  const long double asynchronous_four =
      2.0L * common_step + 4.0L * one_step;
  const long double five_while_remaining =
      common_step * std::sqrt(5.0L) + 4.0L * one_step;
  const long double five_all_removed =
      common_step * std::sqrt(5.0L) + 5.0L * one_step;

  result.maximum_mode_eigenvalue = static_cast<double>(maximum_a);
  result.one_source_step_triangle_bound = static_cast<double>(one_step);
  result.common_step_coefficient = static_cast<double>(common_step);
  result.common_pulse_five_source_bound = static_cast<double>(common_five);
  result.common_pulse_six_source_bound = static_cast<double>(common_six);
  result.common_five_source_margin = K_GENESIS - result.common_pulse_five_source_bound;
  result.asynchronous_four_source_bound =
      static_cast<double>(asynchronous_four);
  result.asynchronous_four_source_margin =
      K_GENESIS - result.asynchronous_four_source_bound;
  result.five_source_while_original_remains_bound =
      static_cast<double>(five_while_remaining);
  result.five_source_while_original_remains_margin =
      K_GENESIS - result.five_source_while_original_remains_bound;
  result.five_source_all_removed_envelope =
      static_cast<double>(five_all_removed);
  result.five_source_all_removed_margin =
      K_GENESIS - result.five_source_all_removed_envelope;
  result.maximum_gradient_stencil_ratio = static_cast<double>(maximum_ratio);
  return result;
}

void configure_bridge(RenderBridge& bridge, std::uint32_t seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = true;
  bridge.toggles.genesis = true;
  bridge.toggles.dual_substrate = false;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  bridge.toggles.langevin_seed = seed;
  bridge.seed_rng(seed);
}

int originals_remaining(const RenderBridge& bridge,
                        const std::vector<int>& original_indices) {
  int remaining = 0;
  for (int index : original_indices) {
    if (bridge.voxels()[static_cast<std::size_t>(index)].state != 0)
      ++remaining;
  }
  return remaining;
}

CollectiveSourceHistoryArm run_arm(
    int L, int source_count, int polarity, int chirality, int translation,
    CollectiveSourceHistoryKind history, std::uint32_t seed,
    const CollectiveSourceHistoryVolume& volume) {
  CollectiveSourceHistoryArm result;
  result.history = history;
  result.lattice_size = L;
  result.source_count = source_count;
  result.polarity = polarity;
  result.chirality = chirality;
  result.translation = translation;
  result.seed = seed;

  RenderBridge bridge(L);
  configure_bridge(bridge, seed);
  if (bridge.backend_kind() != Backend::Kind::Cpu
      || !bridge.toggles.validate()) return result;

  const std::vector<Coord> sites = tetrahedral_sites(
      L, source_count, chirality, translation);
  std::vector<int> original_indices;
  original_indices.reserve(sites.size());
  for (Coord site : sites) {
    bridge.inject_particle(site.x, site.y, site.z,
        static_cast<std::int8_t>(polarity), Vec3{});
    const int index = bridge.lattice().index(site.x, site.y, site.z);
    original_indices.push_back(index);
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = {};
    voxel.wave_vel = {};
    voxel.velocity = {};
    voxel.remainder = {};
    voxel.locked = history != CollectiveSourceHistoryKind::NativeUnlocked;
  }
  if (!bridge.enable_history_journal(true)) return result;

  if (history == CollectiveSourceHistoryKind::LockedStep) {
    result.analytic_bound = volume.common_step_coefficient
        * std::sqrt(static_cast<double>(source_count));
  } else if (history == CollectiveSourceHistoryKind::SynchronousPulse) {
    result.analytic_bound = 2.0 * volume.common_step_coefficient
        * std::sqrt(static_cast<double>(source_count));
  } else if (source_count == 4) {
    result.analytic_bound = volume.asynchronous_four_source_bound;
  } else {
    result.analytic_bound =
        volume.five_source_while_original_remains_bound;
  }

  bool first_genesis_seen = false;
  bool analytic_scope_respected = true;
  for (int tick = 0; tick < RUN_TICKS; ++tick) {
    if (history == CollectiveSourceHistoryKind::SynchronousPulse
        && tick == SYNCHRONOUS_REMOVAL_TICK) {
      for (int index : original_indices) {
        bridge.set_state(index, 0);
        bridge.voxels()[static_cast<std::size_t>(index)].locked = false;
      }
    }

    const int remaining_before = originals_remaining(bridge, original_indices);
    const bool in_analytic_scope = !first_genesis_seen
        && !(history == CollectiveSourceHistoryKind::NativeUnlocked
             && source_count == 5 && remaining_before == 0);
    bridge.tick();
    ++result.ticks;

    int tick_genesis = 0;
    for (const auto& event : bridge.history_events()) {
      if (event.kind == HistoryEventKind::Genesis) {
        ++result.genesis_events;
        ++tick_genesis;
      }
      if (event.kind == HistoryEventKind::Evaporation)
        ++result.evaporation_events;
    }
    if (tick_genesis > 0 && !first_genesis_seen) {
      first_genesis_seen = true;
      result.first_genesis_tick = tick;
      result.originals_remaining_before_first_genesis = remaining_before;
    }

    const int remaining_after = originals_remaining(bridge, original_indices);
    if (remaining_after == 0 && result.all_originals_removed_tick < 0)
      result.all_originals_removed_tick = tick;

    for (const auto& voxel : bridge.voxels()) {
      const double flux = voxel.flux.mag();
      if (in_analytic_scope) {
        result.maximum_flux_in_analytic_scope = std::max(
            result.maximum_flux_in_analytic_scope, flux);
        result.maximum_bound_excess = std::max(
            result.maximum_bound_excess, flux - result.analytic_bound);
        if (flux > result.analytic_bound + TOL)
          analytic_scope_respected = false;
      }
      result.maximum_velocity = std::max(
          result.maximum_velocity, max_abs(voxel.velocity));
      result.maximum_remainder = std::max(
          result.maximum_remainder, max_abs(voxel.remainder));
    }
  }

  result.all_originals_removed = result.all_originals_removed_tick >= 0;
  result.analytic_scope_respected = analytic_scope_respected;
  const bool genesis_allowed = history ==
          CollectiveSourceHistoryKind::NativeUnlocked
      && source_count == 5
      && (result.genesis_events == 0
          || result.originals_remaining_before_first_genesis == 0);
  result.valid = result.ticks == RUN_TICKS
      && result.analytic_scope_respected
      && result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0
      && (result.genesis_events == 0 || genesis_allowed);
  return result;
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
  configure_bridge(bridge, seed);
  const auto sites = tetrahedral_sites(9, 5, 1, 0);
  for (Coord site : sites) {
    bridge.inject_particle(site.x, site.y, site.z, 1, Vec3{});
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(
        bridge.lattice().index(site.x, site.y, site.z))];
    voxel.velocity = {};
    voxel.remainder = {};
    voxel.locked = false;
  }
}

bool observer_neutrality() {
  constexpr std::uint32_t seed = 0x0588AA55u;
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

}  // namespace

const char* collective_source_history_name(CollectiveSourceHistoryKind kind) {
  switch (kind) {
    case CollectiveSourceHistoryKind::LockedStep: return "locked_step";
    case CollectiveSourceHistoryKind::SynchronousPulse:
      return "synchronous_pulse";
    case CollectiveSourceHistoryKind::NativeUnlocked:
      return "native_unlocked";
  }
  return "unknown";
}

CollectiveSourceHistoryBoundResult
analyze_collective_source_history_bound() {
  CollectiveSourceHistoryBoundResult result;
  result.volumes.reserve(SPECTRAL_VOLUMES.size());
  result.arms.reserve(128);

  bool dominance_valid = true;
  bool spectral_valid = true;
  for (int L : SPECTRAL_VOLUMES) {
    auto volume = spectral_bound(L, dominance_valid);
    spectral_valid = spectral_valid
        && volume.maximum_mode_eigenvalue > 0.0
        && volume.maximum_mode_eigenvalue < 4.0
        && volume.maximum_gradient_stencil_ratio < 1.0
        && volume.common_five_source_margin > 0.0
        && volume.asynchronous_four_source_margin > 0.0
        && volume.five_source_while_original_remains_margin > 0.0;
    result.volumes.push_back(volume);
    ++result.spectral_volume_count;
  }
  result.stencil_dominance_derived = dominance_valid;
  result.common_history_n_le_five_closed = spectral_valid;
  result.asynchronous_n_le_four_closed = spectral_valid;
  result.five_source_while_original_remains_closed = spectral_valid;
  result.common_history_minimum_sources_not_excluded = 6;
  result.asynchronous_minimum_sources_not_excluded = 5;

  bool parseval_valid = true;
  for (int L : LIVE_VOLUMES) {
    for (int source_count : {4, 5}) {
      for (int polarity : {-1, 1}) {
        for (int chirality : {-1, 1}) {
          for (int translation : {0, 1}) {
            const double error = parseval_error(
                L, tetrahedral_sites(L, source_count, chirality, translation),
                polarity);
            result.maximum_parseval_error = std::max(
                result.maximum_parseval_error, error);
            parseval_valid = parseval_valid && error <= TOL;
          }
        }
      }
    }
  }
  result.finite_group_parseval_derived = parseval_valid;

  std::uint32_t deterministic_seed = 0x0588C000u;
  for (int L : LIVE_VOLUMES) {
    const auto volume_it = std::find_if(
        result.volumes.begin(), result.volumes.end(),
        [L](const auto& volume) { return volume.lattice_size == L; });
    if (volume_it == result.volumes.end()) continue;
    for (int source_count : {4, 5}) {
      for (int polarity : {-1, 1}) {
        for (int chirality : {-1, 1}) {
          for (int translation : {0, 1}) {
            for (CollectiveSourceHistoryKind history : {
                     CollectiveSourceHistoryKind::LockedStep,
                     CollectiveSourceHistoryKind::SynchronousPulse}) {
              result.arms.push_back(run_arm(
                  L, source_count, polarity, chirality, translation, history,
                  deterministic_seed++, *volume_it));
            }
          }
        }
      }
    }
  }

  std::uint32_t unlocked_seed = 0x05880000u;
  for (int L : LIVE_VOLUMES) {
    const auto volume_it = std::find_if(
        result.volumes.begin(), result.volumes.end(),
        [L](const auto& volume) { return volume.lattice_size == L; });
    if (volume_it == result.volumes.end()) continue;
    for (int source_count : {4, 5}) {
      for (int polarity : {-1, 1}) {
        for (int chirality : {-1, 1}) {
          for (int seed_index = 0; seed_index < 4; ++seed_index) {
            result.arms.push_back(run_arm(
                L, source_count, polarity, chirality, 0,
                CollectiveSourceHistoryKind::NativeUnlocked,
                unlocked_seed++, *volume_it));
          }
        }
      }
    }
  }

  bool arms_valid = true;
  for (const auto& arm : result.arms) {
    arms_valid = arms_valid && arm.valid;
    ++result.total_arms;
    result.total_ticks += arm.ticks;
    result.evaporation_events += arm.evaporation_events;
    result.maximum_bound_excess = std::max(
        result.maximum_bound_excess, arm.maximum_bound_excess);
    result.maximum_velocity = std::max(
        result.maximum_velocity, arm.maximum_velocity);
    result.maximum_remainder = std::max(
        result.maximum_remainder, arm.maximum_remainder);
    if (arm.history == CollectiveSourceHistoryKind::NativeUnlocked) {
      ++result.native_unlocked_arms;
      if (arm.all_originals_removed)
        ++result.unlocked_arms_all_sources_removed;
      if (arm.source_count == 4) {
        result.asynchronous_four_source_genesis_events += arm.genesis_events;
        if (arm.genesis_events > 0)
          result.analytic_contradiction_events += arm.genesis_events;
      } else {
        result.unlocked_five_source_genesis_events += arm.genesis_events;
        if (arm.genesis_events > 0
            && arm.originals_remaining_before_first_genesis == 0) {
          result.five_source_residual_tail_genesis_events +=
              arm.genesis_events;
        } else if (arm.genesis_events > 0) {
          result.analytic_contradiction_events += arm.genesis_events;
        }
      }
    } else {
      ++result.common_history_arms;
      result.common_history_genesis_events += arm.genesis_events;
      if (arm.genesis_events > 0)
        result.analytic_contradiction_events += arm.genesis_events;
    }
  }

  result.five_source_residual_tail_observed =
      result.five_source_residual_tail_genesis_events > 0;
  result.five_source_residual_tail_unresolved =
      !result.five_source_residual_tail_observed;
  result.observer_neutral = observer_neutrality();
  result.production_changed = false;
  result.valid = result.stencil_dominance_derived
      && result.finite_group_parseval_derived
      && result.spectral_volume_count == 4
      && result.common_history_n_le_five_closed
      && result.asynchronous_n_le_four_closed
      && result.five_source_while_original_remains_closed
      && result.common_history_minimum_sources_not_excluded == 6
      && result.asynchronous_minimum_sources_not_excluded == 5
      && result.common_history_arms == 64
      && result.native_unlocked_arms == 64
      && result.total_arms == 128
      && result.total_ticks == 128 * RUN_TICKS
      && result.common_history_genesis_events == 0
      && result.asynchronous_four_source_genesis_events == 0
      && result.analytic_contradiction_events == 0
      && result.unlocked_arms_all_sources_removed > 0
      && result.maximum_bound_excess <= TOL
      && result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0
      && arms_valid
      && result.observer_neutral
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft

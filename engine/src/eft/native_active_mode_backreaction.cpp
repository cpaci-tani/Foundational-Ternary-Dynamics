#include "ftd/eft/native_active_mode_backreaction.h"

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/eft/native_energy_contract.h"
#include "ftd/eft/passive_dressing_depinning_obstruction.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace ftd::eft {
namespace {

constexpr double PI_LOCAL = 3.1415926535897932384626433832795;
constexpr double TOL = 1e-12;

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
  constexpr std::uint64_t prime = 1099511628211ull;
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (std::size_t i = 0; i < size; ++i) {
    hash ^= bytes[i];
    hash *= prime;
  }
}

std::uint64_t field_hash(const RenderBridge& bridge) {
  std::uint64_t hash = 1469598103934665603ull;
  for (const auto& voxel : bridge.voxels()) {
    hash_bytes(hash, &voxel.flux, sizeof(voxel.flux));
    hash_bytes(hash, &voxel.wave_vel, sizeof(voxel.wave_vel));
  }
  return hash;
}

int unique_manifested(const RenderBridge& bridge) {
  int result = -1;
  for (int i = 0; i < static_cast<int>(bridge.voxels().size()); ++i) {
    if (bridge.voxels()[static_cast<std::size_t>(i)].state == 0) continue;
    if (result >= 0) return -2;
    result = i;
  }
  return result;
}

bool configure_native_arm(RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = true;
  bridge.toggles.movement = true;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  bridge.toggles.strict_validation = true;
  return bridge.backend_kind() == Backend::Kind::Cpu
      && bridge.toggles.validate();
}

Vec3 normalized_direction(Coord direction) {
  const Vec3 value{static_cast<double>(direction.x),
                   static_cast<double>(direction.y),
                   static_cast<double>(direction.z)};
  return value * (1.0 / value.mag());
}

Vec3 transverse_polarization(Coord direction) {
  if (direction.y == 0 && direction.z == 0) return {0.0, 1.0, 0.0};
  if (direction.z == 0) return {0.0, 0.0, 1.0};
  return Vec3{1.0, -1.0, 0.0} * (1.0 / std::sqrt(2.0));
}

void seed_unit_mode(RenderBridge& bridge, Coord direction, int phase) {
  const int L = bridge.lattice().size();
  const Vec3 polarization = transverse_polarization(direction);
  const std::array<double, 4> q_amplitude{{1.0, 0.0, -1.0, 0.0}};
  const std::array<double, 4> p_amplitude{{0.0, 1.0, 0.0, -1.0}};
  auto& voxels = bridge.voxels();
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const double theta = 2.0 * PI_LOCAL
            * (direction.x * x + direction.y * y + direction.z * z) / L;
        const double mode = std::cos(theta);
        auto& voxel = voxels[static_cast<std::size_t>(
            bridge.lattice().index(x, y, z))];
        voxel.flux = polarization * (
            q_amplitude[static_cast<std::size_t>(phase)] * mode);
        voxel.wave_vel = polarization * (
            p_amplitude[static_cast<std::size_t>(phase)] * mode);
      }
    }
  }
}

double normalize_mode_energy(RenderBridge& bridge, double target) {
  const auto unit = measure_native_wave_energy(bridge);
  if (!unit.finite || !(unit.tick_invariant > 0.0L))
    return std::numeric_limits<double>::infinity();
  const double scale = std::sqrt(
      target / static_cast<double>(unit.tick_invariant));
  for (auto& voxel : bridge.voxels()) {
    voxel.flux *= scale;
    voxel.wave_vel *= scale;
  }
  const auto normalized = measure_native_wave_energy(bridge);
  if (!normalized.finite) return std::numeric_limits<double>::infinity();
  return std::abs(static_cast<double>(normalized.tick_invariant) - target)
      / std::max(1e-30, target);
}

struct ActiveArm {
  bool valid = false;
  bool field_changed = false;
  double energy_relative_residual = 0.0;
  double energy_to_barrier_ratio = 0.0;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  double anchor_displacement = 0.0;
  int movement_events = 0;
  int reaction_events = 0;
};

ActiveArm run_active_arm(int L, int charge, Coord direction, int phase,
                         double ratio, double maximum_barrier) {
  ActiveArm result;
  RenderBridge bridge(L);
  if (!configure_native_arm(bridge)) return result;
  const int center = L / 2;
  bridge.inject_particle(center, center, center,
      static_cast<std::int8_t>(charge), Vec3{});
  const int original = bridge.lattice().index(center, center, center);
  auto& particle = bridge.voxels()[static_cast<std::size_t>(original)];
  particle.locked = false;
  particle.velocity = {};
  particle.remainder = {};
  seed_unit_mode(bridge, direction, phase);
  result.energy_relative_residual = normalize_mode_energy(
      bridge, ratio * maximum_barrier);
  const auto initial_energy = measure_native_wave_energy(bridge);
  result.energy_to_barrier_ratio = static_cast<double>(
      initial_energy.tick_invariant) / maximum_barrier;
  const std::uint64_t initial_field_hash = field_hash(bridge);
  if (!bridge.enable_history_journal(true)) return result;

  bool exact_state = true;
  for (int tick = 0; tick < 128; ++tick) {
    bridge.tick();
    result.field_changed = result.field_changed
        || field_hash(bridge) != initial_field_hash;
    const int source = unique_manifested(bridge);
    exact_state = exact_state && source == original
        && bridge.voxels()[static_cast<std::size_t>(original)].state == charge;
    if (source >= 0) {
      const auto& voxel = bridge.voxels()[static_cast<std::size_t>(source)];
      result.maximum_velocity = std::max(
          result.maximum_velocity, voxel.velocity.mag());
      result.maximum_remainder = std::max(
          result.maximum_remainder, max_abs(voxel.remainder));
    }
    for (const auto& event : bridge.history_events()) {
      if (event.kind == HistoryEventKind::Movement) ++result.movement_events;
      else ++result.reaction_events;
    }
  }
  const int final_source = unique_manifested(bridge);
  if (final_source >= 0) {
    const auto start = bridge.lattice().coord(original);
    const auto finish = bridge.lattice().coord(final_source);
    const double dx = static_cast<double>(finish.x - start.x);
    const double dy = static_cast<double>(finish.y - start.y);
    const double dz = static_cast<double>(finish.z - start.z);
    result.anchor_displacement = std::sqrt(dx * dx + dy * dy + dz * dz);
  } else {
    result.anchor_displacement = std::numeric_limits<double>::infinity();
  }
  result.valid = exact_state && result.field_changed
      && result.reaction_events == 0
      && result.energy_relative_residual <= TOL
      && result.energy_to_barrier_ratio >= ratio * (1.0 - TOL);
  return result;
}

struct BallisticArm {
  bool valid = false;
  int movement_events = 0;
  int reaction_events = 0;
  double speed_residual = 0.0;
};

BallisticArm run_ballistic_arm(int L, int charge, Coord direction) {
  BallisticArm result;
  RenderBridge bridge(L);
  if (!configure_native_arm(bridge)) return result;
  const int center = L / 2;
  bridge.inject_particle(center, center, center,
      static_cast<std::int8_t>(charge), Vec3{});
  auto& particle = bridge.voxels()[static_cast<std::size_t>(
      bridge.lattice().index(center, center, center))];
  const double expected_speed = 0.5 * C_SPEED;
  particle.velocity = normalized_direction(direction) * expected_speed;
  particle.remainder = {};
  if (!bridge.enable_history_journal(true)) return result;
  bool unique = true;
  for (int tick = 0; tick < 24; ++tick) {
    bridge.tick();
    for (const auto& event : bridge.history_events()) {
      if (event.kind == HistoryEventKind::Movement) ++result.movement_events;
      else ++result.reaction_events;
    }
    const int source = unique_manifested(bridge);
    unique = unique && source >= 0;
    if (source >= 0) {
      result.speed_residual = std::max(result.speed_residual,
          std::abs(bridge.voxels()[static_cast<std::size_t>(source)].speed()
                   - expected_speed));
    }
  }
  result.valid = unique && result.movement_events >= 3
      && result.reaction_events == 0 && result.speed_residual <= TOL;
  return result;
}

Vec3 selected_force_control(int L, int charge) {
  RenderBridge bridge(L);
  bridge.force_cpu();
  if (bridge.backend_kind() != Backend::Kind::Cpu) return {};
  bridge.toggles.disable_all();
  bridge.toggles.forces = true;
  bridge.toggles.emergent_forces = true;
  bridge.toggles.movement = true;
  bridge.toggles.strict_validation = true;
  const int center = L / 2;
  bridge.inject_particle(center, center, center,
      static_cast<std::int8_t>(charge), Vec3{});
  bridge.inject_flux(center + 2, center, center, {1.0, 0.0, 0.0});
  bridge.tick();
  const int source = unique_manifested(bridge);
  return source >= 0
      ? bridge.voxels()[static_cast<std::size_t>(source)].velocity : Vec3{};
}

bool coupling_control(int L, Coord direction, double target) {
  RenderBridge sourced(L), empty(L);
  if (!configure_native_arm(sourced) || !configure_native_arm(empty))
    return false;
  const int center = L / 2;
  sourced.inject_particle(center, center, center, 1, Vec3{});
  seed_unit_mode(sourced, direction, 0);
  seed_unit_mode(empty, direction, 0);
  if (normalize_mode_energy(sourced, target) > TOL
      || normalize_mode_energy(empty, target) > TOL) return false;
  if (field_hash(sourced) != field_hash(empty)) return false;
  for (int tick = 0; tick < 4; ++tick) {
    sourced.tick();
    empty.tick();
  }
  return field_hash(sourced) != field_hash(empty);
}

}  // namespace

NativeActiveModeBackreactionResult analyze_native_active_mode_backreaction() {
  NativeActiveModeBackreactionResult result;
  result.minimum_initial_energy_to_barrier_ratio =
      std::numeric_limits<double>::infinity();
  result.minimum_ballistic_movement_events =
      std::numeric_limits<int>::max();
  result.minimum_selected_force_response =
      std::numeric_limits<double>::infinity();

  const auto parent = analyze_passive_dressing_depinning_obstruction();
  if (!parent.valid || !(parent.maximum_barrier > 0.0)) return result;
  const double maximum_barrier = parent.maximum_barrier;
  const std::array<Coord, 3> directions{{
      {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}};

  bool active_valid = true;
  for (int L : {17, 33}) {
    for (int charge : {-1, 1}) {
      for (Coord direction : directions) {
        for (int phase = 0; phase < 4; ++phase) {
          for (double ratio : {2.0, 8.0, 32.0}) {
            const ActiveArm arm = run_active_arm(
                L, charge, direction, phase, ratio, maximum_barrier);
            active_valid = active_valid && arm.valid;
            if (arm.field_changed) ++result.active_field_changed_arms;
            result.maximum_initial_energy_relative_residual = std::max(
                result.maximum_initial_energy_relative_residual,
                arm.energy_relative_residual);
            result.minimum_initial_energy_to_barrier_ratio = std::min(
                result.minimum_initial_energy_to_barrier_ratio,
                arm.energy_to_barrier_ratio);
            result.maximum_native_velocity_response = std::max(
                result.maximum_native_velocity_response,
                arm.maximum_velocity);
            result.maximum_native_remainder_response = std::max(
                result.maximum_native_remainder_response,
                arm.maximum_remainder);
            result.maximum_native_anchor_displacement = std::max(
                result.maximum_native_anchor_displacement,
                arm.anchor_displacement);
            result.maximum_native_movement_events = std::max(
                result.maximum_native_movement_events,
                arm.movement_events);
            active_valid = active_valid && arm.reaction_events == 0;
            ++result.active_mode_arms;
            result.active_mode_ticks += 128;
          }
        }
      }
    }
  }

  bool ballistic_valid = true;
  for (int L : {17, 33}) {
    for (int charge : {-1, 1}) {
      for (Coord direction : directions) {
        const BallisticArm arm = run_ballistic_arm(L, charge, direction);
        ballistic_valid = ballistic_valid && arm.valid;
        result.minimum_ballistic_movement_events = std::min(
            result.minimum_ballistic_movement_events, arm.movement_events);
        result.maximum_ballistic_speed_residual = std::max(
            result.maximum_ballistic_speed_residual, arm.speed_residual);
        result.maximum_ballistic_reaction_events = std::max(
            result.maximum_ballistic_reaction_events, arm.reaction_events);
        ++result.ballistic_arms;
      }
    }
  }

  bool force_valid = true;
  for (int L : {17, 33}) {
    const Vec3 positive = selected_force_control(L, +1);
    const Vec3 negative = selected_force_control(L, -1);
    result.minimum_selected_force_response = std::min({
        result.minimum_selected_force_response,
        positive.mag(), negative.mag()});
    result.maximum_selected_force_mirror_residual = std::max(
        result.maximum_selected_force_mirror_residual,
        (positive + negative).mag());
    force_valid = force_valid && positive.x > 0.0 && negative.x < 0.0;
    result.selected_force_control_arms += 2;
  }

  for (int L : {17, 33}) {
    for (Coord direction : directions) {
      if (coupling_control(L, direction, 8.0 * maximum_barrier))
        ++result.coupling_control_differences;
      ++result.coupling_control_pairs;
    }
  }

  result.source_graph_one_way = true;
  result.active_native_backreaction_absent = active_valid
      && result.active_mode_arms == 144
      && result.active_mode_ticks == 18432
      && result.active_field_changed_arms == 144
      && result.maximum_initial_energy_relative_residual <= TOL
      && result.minimum_initial_energy_to_barrier_ratio >= 2.0 * (1.0 - TOL)
      && result.maximum_native_velocity_response == 0.0
      && result.maximum_native_remainder_response == 0.0
      && result.maximum_native_anchor_displacement == 0.0
      && result.maximum_native_movement_events == 0;
  result.sensitivity_controls_pass = ballistic_valid && force_valid
      && result.ballistic_arms == 12
      && result.minimum_ballistic_movement_events >= 3
      && result.maximum_ballistic_speed_residual <= TOL
      && result.maximum_ballistic_reaction_events == 0
      && result.selected_force_control_arms == 4
      && result.minimum_selected_force_response > 1e-12
      && result.maximum_selected_force_mirror_residual <= TOL
      && result.coupling_control_pairs == 6
      && result.coupling_control_differences == 6;
  result.native_common_action_implemented = false;
  result.production_changed = false;
  result.valid = result.source_graph_one_way
      && result.active_native_backreaction_absent
      && result.sensitivity_controls_pass
      && !result.native_common_action_implemented
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft

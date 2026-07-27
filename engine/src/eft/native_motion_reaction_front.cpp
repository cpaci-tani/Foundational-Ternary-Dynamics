#include "ftd/eft/native_motion_reaction_front.h"

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
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

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

Vec3 subtract(const Vec3& lhs, const Vec3& rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

Coord add(Coord lhs, Coord rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

Coord scale(Coord value, int factor) {
  return {factor * value.x, factor * value.y, factor * value.z};
}

Vec3 as_vec(Coord value) {
  return {static_cast<double>(value.x), static_cast<double>(value.y),
          static_cast<double>(value.z)};
}

Vec3 normalized(Coord direction) {
  const Vec3 value = as_vec(direction);
  return value * (1.0 / value.mag());
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

struct RestArm {
  bool valid = false;
  double maximum_velocity = 0.0;
  double maximum_remainder = 0.0;
  double displacement = 0.0;
  int movement_events = 0;
  int reaction_events = 0;
};

RestArm run_reaction_free_rest_arm(int charge, Coord direction) {
  RestArm result;
  constexpr int L = 9;
  RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = true;
  bridge.toggles.movement = true;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  if (bridge.backend_kind() != Backend::Kind::Cpu
      || !bridge.toggles.validate()) return result;

  const Coord center{L / 2, L / 2, L / 2};
  const int origin = bridge.lattice().index(center.x, center.y, center.z);
  bridge.inject_particle(center.x, center.y, center.z,
      static_cast<std::int8_t>(charge), Vec3{});
  auto& particle = bridge.voxels()[static_cast<std::size_t>(origin)];
  particle.velocity = {};
  particle.remainder = {};
  particle.locked = false;

  const Coord packet = add(center, scale(direction, 2));
  auto& field_site = bridge.voxels()[static_cast<std::size_t>(
      bridge.lattice().index(packet.x, packet.y, packet.z))];
  field_site.flux = normalized(direction) * 4.0;
  field_site.wave_vel = normalized(direction) * -0.25;
  if (!bridge.enable_history_journal(true)) return result;

  bool exact_anchor = true;
  for (int tick = 0; tick < 32; ++tick) {
    bridge.tick();
    const int active = unique_manifested(bridge);
    exact_anchor = exact_anchor && active == origin;
    if (active >= 0) {
      const auto& voxel = bridge.voxels()[static_cast<std::size_t>(active)];
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

  const int final_site = unique_manifested(bridge);
  if (final_site >= 0) {
    const Coord finish = bridge.lattice().coord(final_site);
    result.displacement = subtract(as_vec(finish), as_vec(center)).mag();
  } else {
    result.displacement = std::numeric_limits<double>::infinity();
  }
  result.valid = exact_anchor && result.maximum_velocity == 0.0
      && result.maximum_remainder == 0.0 && result.displacement == 0.0
      && result.movement_events == 0 && result.reaction_events == 0;
  return result;
}

int run_ballistic_control(int charge, Coord direction) {
  constexpr int L = 9;
  RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.movement = true;
  bridge.toggles.flux_boundary = FluxBoundaryMode::Periodic;
  if (bridge.backend_kind() != Backend::Kind::Cpu
      || !bridge.toggles.validate()) return -1;

  const Coord center{L / 2, L / 2, L / 2};
  bridge.inject_particle(center.x, center.y, center.z,
      static_cast<std::int8_t>(charge), Vec3{});
  auto& particle = bridge.voxels()[static_cast<std::size_t>(
      bridge.lattice().index(center.x, center.y, center.z))];
  particle.velocity = normalized(direction) * (0.5 * C_SPEED);
  particle.remainder = {};
  if (!bridge.enable_history_journal(true)) return -1;

  int hops = 0;
  for (int tick = 0; tick < 24; ++tick) {
    bridge.tick();
    for (const auto& event : bridge.history_events()) {
      if (event.kind == HistoryEventKind::Movement) ++hops;
      else return -1;
    }
  }
  return hops;
}

Vec3 first_moment(const DualCellContinuity& history, bool after) {
  Vec3 result{};
  const auto& rho = after ? history.rho_after : history.rho_before;
  for (int x = 0; x < history.L; ++x)
    for (int y = 0; y < history.L; ++y)
      for (int z = 0; z < history.L; ++z) {
        const double q = static_cast<double>(
            rho[static_cast<std::size_t>(history.index(x, y, z))]);
        result.x += q * x;
        result.y += q * y;
        result.z += q * z;
      }
  return result;
}

Vec3 integrated_current(const DualCellContinuity& history) {
  Vec3 result{};
  for (double value : history.current_x) result.x += value;
  for (double value : history.current_y) result.y += value;
  for (double value : history.current_z) result.z += value;
  return result;
}

Vec3 reaction_first_moment(const DualCellContinuity& history) {
  Vec3 result{};
  for (int x = 0; x < history.L; ++x)
    for (int y = 0; y < history.L; ++y)
      for (int z = 0; z < history.L; ++z) {
        const double source = static_cast<double>(
            history.reaction[static_cast<std::size_t>(
                history.index(x, y, z))]);
        result.x += source * x;
        result.y += source * y;
        result.z += source * z;
      }
  return result;
}

double snapshot_difference(const DualCellContinuity& lhs,
                           const DualCellContinuity& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.rho_before.size(); ++i) {
    result = std::max(result, std::abs(static_cast<double>(
        lhs.rho_before[i] - rhs.rho_before[i])));
    result = std::max(result, std::abs(static_cast<double>(
        lhs.rho_after[i] - rhs.rho_after[i])));
  }
  return result;
}

struct StaleArm {
  bool valid = false;
  int evaporation_ticks = 0;
  double velocity_residual = 0.0;
  double remainder_residual = 0.0;
};

StaleArm run_stale_kinematics_arm(int target_polarity, Coord direction,
                                  unsigned int seed) {
  StaleArm result;
  constexpr int L = 9;
  const Coord center{L / 2, L / 2, L / 2};
  RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.evaporation = true;
  bridge.toggles.langevin_seed = seed;
  if (bridge.backend_kind() != Backend::Kind::Cpu
      || !bridge.toggles.validate()) return result;

  bridge.inject_particle(center.x, center.y, center.z, 1, Vec3{});
  const int center_index = bridge.lattice().index(
      center.x, center.y, center.z);
  auto& initial = bridge.voxels()[static_cast<std::size_t>(center_index)];
  const Vec3 expected_velocity = normalized(direction) * (0.125 * C_SPEED);
  const Vec3 expected_remainder = normalized(direction) * 0.25;
  initial.velocity = expected_velocity;
  initial.remainder = expected_remainder;
  initial.locked = false;
  initial.flux = {};
  initial.wave_vel = {};

  for (int tick = 1; tick <= 256; ++tick) {
    bridge.tick();
    if (bridge.voxels()[static_cast<std::size_t>(center_index)].state == 0) {
      result.evaporation_ticks = tick;
      break;
    }
  }
  if (result.evaporation_ticks == 0) return result;

  auto& void_site = bridge.voxels()[static_cast<std::size_t>(center_index)];
  result.velocity_residual = max_abs(subtract(
      void_site.velocity, expected_velocity));
  result.remainder_residual = max_abs(subtract(
      void_site.remainder, expected_remainder));
  if (result.velocity_residual != 0.0 || result.remainder_residual != 0.0)
    return result;

  bridge.toggles.evaporation = false;
  bridge.toggles.genesis = true;
  void_site.flux = {0.0, 100.0, 0.0};
  const int minus_x = bridge.lattice().index(
      center.x - 1, center.y, center.z);
  const int plus_x = bridge.lattice().index(
      center.x + 1, center.y, center.z);
  bridge.voxels()[static_cast<std::size_t>(minus_x)].flux =
      {-0.25 * target_polarity, 0.0, 0.0};
  bridge.voxels()[static_cast<std::size_t>(plus_x)].flux =
      {0.25 * target_polarity, 0.0, 0.0};
  bridge.tick();

  const auto& remanifested = bridge.voxels()[
      static_cast<std::size_t>(center_index)];
  result.velocity_residual = std::max(result.velocity_residual,
      max_abs(subtract(remanifested.velocity, expected_velocity)));
  result.remainder_residual = std::max(result.remainder_residual,
      max_abs(subtract(remanifested.remainder, expected_remainder)));
  result.valid = remanifested.state == target_polarity
      && unique_manifested(bridge) == center_index
      && result.velocity_residual == 0.0
      && result.remainder_residual == 0.0;
  return result;
}

}  // namespace

NativeMotionReactionFrontResult analyze_native_motion_reaction_front() {
  NativeMotionReactionFrontResult result;
  result.minimum_ballistic_hops = std::numeric_limits<int>::max();

  const std::array<Coord, 6> face_directions{{
      {1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
      {0, -1, 0}, {0, 0, 1}, {0, 0, -1}}};

  bool rest_valid = true;
  for (int charge : {-1, 1}) {
    for (Coord direction : face_directions) {
      const RestArm arm = run_reaction_free_rest_arm(charge, direction);
      rest_valid = rest_valid && arm.valid;
      result.maximum_rest_velocity = std::max(
          result.maximum_rest_velocity, arm.maximum_velocity);
      result.maximum_rest_remainder = std::max(
          result.maximum_rest_remainder, arm.maximum_remainder);
      result.maximum_rest_displacement = std::max(
          result.maximum_rest_displacement, arm.displacement);
      ++result.reaction_free_rest_arms;
      result.reaction_free_rest_ticks += 32;

      const int hops = run_ballistic_control(charge, direction);
      result.minimum_ballistic_hops = std::min(
          result.minimum_ballistic_hops, hops);
      ++result.ballistic_control_arms;
    }
  }
  result.reaction_free_zero_kinematics_invariant = rest_valid
      && result.reaction_free_rest_arms == 12
      && result.reaction_free_rest_ticks == 384
      && result.maximum_rest_velocity == 0.0
      && result.maximum_rest_remainder == 0.0
      && result.maximum_rest_displacement == 0.0
      && result.ballistic_control_arms == 12
      && result.minimum_ballistic_hops >= 3;

  constexpr int L = 9;
  const Coord base{L / 2, L / 2, L / 2};
  const std::array<Coord, 3> translations{{
      {-1, -1, -1}, {0, 0, 0}, {1, 1, 1}}};
  bool decomposition_valid = true;
  for (int charge : {-1, 1}) {
    for (Coord direction : face_directions) {
      for (Coord translation : translations) {
        const Coord start = add(base, translation);
        const Coord finish = add(start, direction);
        std::vector<int> before(static_cast<std::size_t>(L * L * L), 0);
        std::vector<int> after(static_cast<std::size_t>(L * L * L), 0);
        const int source = start.x * L * L + start.y * L + start.z;
        const int target = finish.x * L * L + finish.y * L + finish.z;
        before[static_cast<std::size_t>(source)] = charge;
        after[static_cast<std::size_t>(target)] = charge;

        DualCellContinuity transport;
        const auto extraction = extract_moore_history_from_snapshots(
            L, before, after, transport);
        DualCellContinuity reaction(L);
        reaction.rho_before = before;
        reaction.rho_after = after;
        reaction.reaction[static_cast<std::size_t>(source)] = -charge;
        reaction.reaction[static_cast<std::size_t>(target)] = charge;

        result.maximum_continuity_residual = std::max({
            result.maximum_continuity_residual,
            max_continuity_residual(transport),
            max_continuity_residual(reaction)});
        result.maximum_charge_balance_residual = std::max({
            result.maximum_charge_balance_residual,
            std::abs(static_cast<double>(
                total_after(transport) - total_before(transport)
                - total_reaction(transport))),
            std::abs(static_cast<double>(
                total_after(reaction) - total_before(reaction)
                - total_reaction(reaction)))});

        const Vec3 delta_moment = subtract(
            first_moment(transport, true), first_moment(transport, false));
        const Vec3 transport_rhs = integrated_current(transport);
        const Vec3 reaction_rhs = reaction_first_moment(reaction);
        result.maximum_first_moment_residual = std::max({
            result.maximum_first_moment_residual,
            max_abs(subtract(delta_moment, transport_rhs)),
            max_abs(subtract(delta_moment, reaction_rhs))});
        result.maximum_snapshot_difference = std::max(
            result.maximum_snapshot_difference,
            snapshot_difference(transport, reaction));

        decomposition_valid = decomposition_valid && extraction.valid
            && extraction.transported_events == 1
            && extraction.reaction_sites == 0
            && total_current_l1(transport) == 1.0
            && total_reaction_l1(transport) == 0
            && total_current_l1(reaction) == 0.0
            && total_reaction_l1(reaction) == 2
            && total_reaction(reaction) == 0;
        ++result.transport_fixtures;
        ++result.reaction_front_fixtures;
        result.moment_identity_samples += 2;
      }
    }
  }
  result.same_snapshot_admits_transport_or_reaction_decomposition =
      decomposition_valid && result.transport_fixtures == 36
      && result.reaction_front_fixtures == 36
      && result.moment_identity_samples == 72
      && result.maximum_continuity_residual <= TOL
      && result.maximum_charge_balance_residual <= TOL
      && result.maximum_first_moment_residual <= TOL
      && result.maximum_snapshot_difference == 0.0;

  bool stale_valid = true;
  unsigned int seed = 20260726u;
  for (int polarity : {-1, 1}) {
    for (Coord direction : face_directions) {
      const StaleArm arm = run_stale_kinematics_arm(
          polarity, direction, seed++);
      stale_valid = stale_valid && arm.valid;
      result.maximum_evaporation_ticks = std::max(
          result.maximum_evaporation_ticks, arm.evaporation_ticks);
      result.maximum_stale_velocity_residual = std::max(
          result.maximum_stale_velocity_residual, arm.velocity_residual);
      result.maximum_stale_remainder_residual = std::max(
          result.maximum_stale_remainder_residual, arm.remainder_residual);
      ++result.stale_kinematics_arms;
    }
  }
  result.evaporation_preserves_hidden_kinematics = stale_valid
      && result.stale_kinematics_arms == 12
      && result.maximum_evaporation_ticks > 0
      && result.maximum_evaporation_ticks <= 256
      && result.maximum_stale_velocity_residual == 0.0
      && result.maximum_stale_remainder_residual == 0.0;
  result.genesis_reuses_hidden_kinematics =
      result.evaporation_preserves_hidden_kinematics;

  result.globally_balanced_reaction_source_is_local_current = false;
  result.support_translation_implies_particle_worldline = false;
  result.selected_force_is_common_action = false;
  result.production_changed = false;
  result.valid = result.reaction_free_zero_kinematics_invariant
      && result.same_snapshot_admits_transport_or_reaction_decomposition
      && !result.globally_balanced_reaction_source_is_local_current
      && !result.support_translation_implies_particle_worldline
      && result.evaporation_preserves_hidden_kinematics
      && result.genesis_reuses_hidden_kinematics
      && !result.selected_force_is_common_action
      && !result.production_changed;
  return result;
}

}  // namespace ftd::eft

#include "ftd/eft/contact_quotient_horizon.h"

#include "ftd/constants.h"
#include "ftd/eft/ternary_collision_vertex.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

struct RawCarrier {
  int index = -1;
  int polarity = 0;
  Coord anchor{};
  Vec3 remainder{};
  Vec3 position{};
  Vec3 velocity{};
};

Vec3 coordinate(Coord value) {
  return {static_cast<double>(value.x),
          static_cast<double>(value.y),
          static_cast<double>(value.z)};
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double vector_residual(const Vec3& lhs, const Vec3& rhs) {
  return max_abs(lhs-rhs);
}

int wrap(int value, int L) {
  value %= L;
  return value < 0 ? value + L : value;
}

int flat_index(int L, Coord value) {
  return (wrap(value.x, L) * L + wrap(value.y, L)) * L
      + wrap(value.z, L);
}

std::vector<RawCarrier> carriers(RenderBridge& bridge) {
  std::vector<RawCarrier> result;
  const int count = static_cast<int>(bridge.lattice().total_sites());
  for (int i = 0; i < count; ++i) {
    const int state = bridge.state_at(i);
    if (state == 0) continue;
    const Coord anchor = bridge.lattice().coord(i);
    const auto& voxel = bridge.voxel_at(anchor.x, anchor.y, anchor.z);
    result.push_back({i, state, anchor, voxel.remainder,
        coordinate(anchor)+voxel.remainder, voxel.velocity});
  }
  return result;
}

bool phase_less(const RawCarrier& lhs, const RawCarrier& rhs) {
  const std::array<double, 7> a{{
      lhs.position.x, lhs.position.y, lhs.position.z,
      lhs.velocity.x, lhs.velocity.y, lhs.velocity.z,
      static_cast<double>(lhs.polarity)}};
  const std::array<double, 7> b{{
      rhs.position.x, rhs.position.y, rhs.position.z,
      rhs.velocity.x, rhs.velocity.y, rhs.velocity.z,
      static_cast<double>(rhs.polarity)}};
  return a < b;
}

double phase_multiset_residual(std::vector<RawCarrier> lhs,
                               std::vector<RawCarrier> rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  std::sort(lhs.begin(), lhs.end(), phase_less);
  std::sort(rhs.begin(), rhs.end(), phase_less);
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max({result,
        vector_residual(lhs[i].position, rhs[i].position),
        vector_residual(lhs[i].velocity, rhs[i].velocity),
        std::abs(static_cast<double>(lhs[i].polarity-rhs[i].polarity))});
  }
  return result;
}

double raw_label_residual(const std::vector<RawCarrier>& lhs,
                          const std::vector<RawCarrier>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    if (lhs[i].index != rhs[i].index
        || lhs[i].polarity != rhs[i].polarity) return INFINITY;
    result = std::max({result,
        vector_residual(lhs[i].remainder, rhs[i].remainder),
        vector_residual(lhs[i].velocity, rhs[i].velocity)});
  }
  return result;
}

std::vector<double> density(int L, const std::vector<RawCarrier>& input) {
  const std::size_t volume = static_cast<std::size_t>(L) * L * L;
  std::vector<double> result(volume, 0.0);
  for (const auto& carrier : input) {
    const auto shape = make_subcell_polarity_shape(
        carrier.anchor, carrier.remainder, carrier.polarity);
    if (!shape.valid) return {};
    for (std::size_t i = 0; i < shape.weight_count; ++i) {
      const auto& entry = shape.weights[i];
      result[static_cast<std::size_t>(flat_index(L, entry.site))]
          += entry.weight;
    }
  }
  return result;
}

double field_residual(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

double signature_residual(const PiecewiseCurrentSignature& lhs,
                          const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({
      field_residual(lhs.rho_before, rhs.rho_before),
      field_residual(lhs.rho_after, rhs.rho_after),
      field_residual(lhs.current_x, rhs.current_x),
      field_residual(lhs.current_y, rhs.current_y),
      field_residual(lhs.current_z, rhs.current_z)});
}

PiecewiseCurrentSignature one_tick_signature(
    int L,
    const std::vector<RawCarrier>& before,
    const std::vector<RawCarrier>& after) {
  if (before.size() != after.size()) return {};
  std::vector<PiecewiseWorldline> lines;
  lines.reserve(before.size());
  for (std::size_t i = 0; i < before.size(); ++i) {
    if (before[i].index != after[i].index
        || before[i].polarity != after[i].polarity) return {};
    lines.push_back({before[i].polarity,
        {before[i].position, after[i].position}});
  }
  return make_piecewise_current_signature(L, lines);
}

double global_field_magnitude(RenderBridge& bridge) {
  double result = 0.0;
  const int count = static_cast<int>(bridge.lattice().total_sites());
  for (int i = 0; i < count; ++i) {
    const Coord site = bridge.lattice().coord(i);
    const auto& voxel = bridge.voxel_at(site.x, site.y, site.z);
    result = std::max({result, voxel.flux.mag(), voxel.wave_vel.mag(),
        voxel.flux_L.mag(), voxel.flux_R.mag(),
        voxel.wave_vel_L.mag(), voxel.wave_vel_R.mag(),
        voxel.flux_strong.mag(), voxel.wave_vel_strong.mag(),
        voxel.flux_weak.mag(), voxel.wave_vel_weak.mag()});
  }
  return result;
}

void initialize(RenderBridge& bridge,
                const NativeContactActiveSetGeometry& geometry,
                int polarity,
                const Vec3& first_velocity,
                const Vec3& second_velocity,
                bool symmetric_order,
                unsigned int movement_seed) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.movement = true;
  bridge.toggles.symmetric_movement_order = symmetric_order;
  bridge.toggles.langevin_seed = movement_seed;
  bridge.set_dt(1.0);
  bridge.enable_history_journal(true);
  const int first_index = bridge.lattice().index(
      geometry.first_anchor.x, geometry.first_anchor.y,
      geometry.first_anchor.z);
  const int second_index = bridge.lattice().index(
      geometry.second_anchor.x, geometry.second_anchor.y,
      geometry.second_anchor.z);
  bridge.set_state(first_index, static_cast<int8_t>(polarity));
  bridge.set_state(second_index, static_cast<int8_t>(polarity));
  auto& first = bridge.voxel_at(
      geometry.first_anchor.x, geometry.first_anchor.y,
      geometry.first_anchor.z);
  auto& second = bridge.voxel_at(
      geometry.second_anchor.x, geometry.second_anchor.y,
      geometry.second_anchor.z);
  first.remainder = geometry.first_contact_remainder;
  second.remainder = geometry.second_contact_remainder;
  first.velocity = first_velocity;
  second.velocity = second_velocity;
  first.particle_id = 101;
  second.particle_id = 102;
}

Vec3 relativistic_momentum(const Vec3& velocity) {
  const double speed2 = velocity.mag2();
  const double gamma = 1.0 / std::sqrt(
      1.0-speed2/(C_SPEED*C_SPEED));
  return velocity * (E_REST * gamma / (C_SPEED*C_SPEED));
}

double relativistic_energy(const Vec3& velocity) {
  const double speed2 = velocity.mag2();
  return E_REST / std::sqrt(1.0-speed2/(C_SPEED*C_SPEED));
}

std::array<double, 3> invariants(const std::vector<RawCarrier>& input) {
  double polarity = 0.0;
  Vec3 momentum{};
  double energy = 0.0;
  for (const auto& carrier : input) {
    polarity += carrier.polarity;
    momentum += relativistic_momentum(carrier.velocity);
    energy += relativistic_energy(carrier.velocity);
  }
  return {polarity, momentum.mag(), energy};
}

double invariant_residual(const std::vector<RawCarrier>& lhs,
                          const std::vector<RawCarrier>& rhs) {
  const auto a = invariants(lhs);
  const auto b = invariants(rhs);
  return std::max({std::abs(a[0]-b[0]), std::abs(a[1]-b[1]),
                   std::abs(a[2]-b[2])});
}

double site_state_residual(const RenderBridge& lhs,
                           const RenderBridge& rhs) {
  const int count = static_cast<int>(lhs.lattice().total_sites());
  if (count != static_cast<int>(rhs.lattice().total_sites())) return INFINITY;
  double result = 0.0;
  for (int i = 0; i < count; ++i)
    result = std::max(result, std::abs(static_cast<double>(
        lhs.state_at(i)-rhs.state_at(i))));
  return result;
}

}  // namespace

ContactQuotientHorizonResult analyze_contact_quotient_horizon(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    bool symmetric_movement_order,
    unsigned int movement_seed,
    double tolerance) {
  ContactQuotientHorizonResult result;
  result.symmetric_movement_order = symmetric_movement_order;
  result.geometry = analyze_native_contact_active_set_geometry(
      L, contact_position, chart_direction, polarity, speed, tolerance);
  if (!result.geometry.valid || speed >= C_SPEED
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  result.predicted_horizon_tick =
      result.geometry.predicted_hop_delay_ticks;
  const Vec3 incoming = result.geometry.normal * speed;
  const Vec3 outgoing = incoming * -1.0;
  const Vec3 direction{
      static_cast<double>(chart_direction.x),
      static_cast<double>(chart_direction.y),
      static_cast<double>(chart_direction.z)};
  result.overshoot = result.predicted_horizon_tick*speed
      - 0.5*direction.mag();
  if (result.overshoot < 0.0
      && std::abs(result.overshoot) <= tolerance) result.overshoot = 0.0;
  if (result.overshoot < 0.0) return result;
  result.commensurate_horizon = result.overshoot <= tolerance;
  result.expected_horizon_phase_residual = max_abs(
      result.geometry.normal*result.overshoot);

  RenderBridge crossing(L);
  RenderBridge bounce(L);
  initialize(crossing, result.geometry, polarity,
             incoming, outgoing, symmetric_movement_order, movement_seed);
  initialize(bounce, result.geometry, polarity,
             outgoing, incoming, symmetric_movement_order, movement_seed);

  const auto contact_crossing = carriers(crossing);
  const auto contact_bounce = carriers(bounce);
  if (contact_crossing.size() != 2 || contact_bounce.size() != 2
      || phase_multiset_residual(contact_crossing, contact_bounce)
          > tolerance) return result;

  result.minimum_raw_label_residual = INFINITY;
  bool pre_horizon_ok = true;
  for (int tick = 1; tick <= result.predicted_horizon_tick; ++tick) {
    const auto crossing_before = carriers(crossing);
    const auto bounce_before = carriers(bounce);
    crossing.tick();
    bounce.tick();
    const auto crossing_after = carriers(crossing);
    const auto bounce_after = carriers(bounce);
    if (crossing_after.size() != 2 || bounce_after.size() != 2) return result;

    const double phase = phase_multiset_residual(
        crossing_after, bounce_after);
    const double density_difference = field_residual(
        density(L, crossing_after), density(L, bounce_after));
    if (tick < result.predicted_horizon_tick) {
      const auto crossing_signature = one_tick_signature(
          L, crossing_before, crossing_after);
      const auto bounce_signature = one_tick_signature(
          L, bounce_before, bounce_after);
      const double current = signature_residual(
          crossing_signature, bounce_signature);
      result.worst_pre_horizon_phase_residual = std::max(
          result.worst_pre_horizon_phase_residual, phase);
      result.worst_pre_horizon_density_residual = std::max(
          result.worst_pre_horizon_density_residual, density_difference);
      result.worst_pre_horizon_current_residual = std::max(
          result.worst_pre_horizon_current_residual, current);
      result.minimum_raw_label_residual = std::min(
          result.minimum_raw_label_residual,
          raw_label_residual(crossing_after, bounce_after));
      pre_horizon_ok = pre_horizon_ok
          && phase <= tolerance && density_difference <= tolerance
          && current <= tolerance;
      ++result.pre_horizon_ticks_compared;
    } else {
      result.horizon_phase_residual = phase;
      result.horizon_density_residual = density_difference;
      result.horizon_site_state_residual = site_state_residual(
          crossing, bounce);
      result.horizon_invariant_residual = invariant_residual(
          crossing_after, bounce_after);
      if (phase > tolerance) result.first_physical_divergence_tick = tick;

      const auto& crossing_first = crossing.voxel_at(
          result.geometry.first_anchor.x, result.geometry.first_anchor.y,
          result.geometry.first_anchor.z);
      const auto& crossing_second = crossing.voxel_at(
          result.geometry.second_anchor.x, result.geometry.second_anchor.y,
          result.geometry.second_anchor.z);
      const auto& bounce_first = bounce.voxel_at(
          result.geometry.first_anchor.x, result.geometry.first_anchor.y,
          result.geometry.first_anchor.z);
      const auto& bounce_second = bounce.voxel_at(
          result.geometry.second_anchor.x, result.geometry.second_anchor.y,
          result.geometry.second_anchor.z);
      result.crossing_reset_residual = std::max({
          crossing_first.remainder.mag(), crossing_second.remainder.mag(),
          vector_residual(crossing_first.velocity, outgoing),
          vector_residual(crossing_second.velocity, incoming)});
      const Vec3 expected_first = result.geometry.normal*(-result.overshoot);
      const Vec3 expected_second = result.geometry.normal*result.overshoot;
      result.bounce_overshoot_residual = std::max({
          vector_residual(bounce_first.remainder, expected_first),
          vector_residual(bounce_second.remainder, expected_second),
          vector_residual(bounce_first.velocity, outgoing),
          vector_residual(bounce_second.velocity, incoming)});
    }
    result.maximum_journal_events = std::max({
        result.maximum_journal_events,
        static_cast<int>(crossing.history_events().size()),
        static_cast<int>(bounce.history_events().size())});
    result.field_residual = std::max({result.field_residual,
        global_field_magnitude(crossing), global_field_magnitude(bounce)});
  }

  result.quotient_equivalent_before_horizon = pre_horizon_ok
      && result.worst_pre_horizon_phase_residual <= tolerance
      && result.worst_pre_horizon_density_residual <= tolerance
      && result.worst_pre_horizon_current_residual <= tolerance
      && result.minimum_raw_label_residual > tolerance;
  if (result.commensurate_horizon) {
    result.rejoined_at_commensurate_horizon =
        result.horizon_phase_residual <= tolerance
        && result.horizon_density_residual <= tolerance;
    crossing.tick();
    bounce.tick();
    result.commensurate_extra_tick_residual = phase_multiset_residual(
        carriers(crossing), carriers(bounce));
    result.rejoined_at_commensurate_horizon =
        result.rejoined_at_commensurate_horizon
        && result.commensurate_extra_tick_residual <= tolerance;
  } else {
    result.overshoot_breaks_quotient_at_horizon =
        result.first_physical_divergence_tick
            == result.predicted_horizon_tick
        && std::abs(result.horizon_phase_residual
                    - result.expected_horizon_phase_residual) <= tolerance;
  }
  result.valid = result.quotient_equivalent_before_horizon
      && result.crossing_reset_residual <= tolerance
      && result.bounce_overshoot_residual <= tolerance
      && result.horizon_site_state_residual <= tolerance
      && result.horizon_invariant_residual <= tolerance
      && result.maximum_journal_events == 0
      && result.field_residual <= tolerance
      && (result.commensurate_horizon
          ? result.rejoined_at_commensurate_horizon
          : result.overshoot_breaks_quotient_at_horizon);
  return result;
}

}  // namespace ftd::eft

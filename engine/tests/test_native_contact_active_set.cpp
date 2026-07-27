/** FTD-0525: frozen production vs selected hard-contact active set. */

#include "ftd/eft/native_contact_active_set.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int geometry_arms = 0;
int crossing_arms = 0;
int activation_arms = 0;
double worst_gap_residual = 0.0;
double worst_stable_chart_residual = 0.0;
double minimum_contact_hop_margin = INFINITY;
double minimum_crossed_hop_margin = INFINITY;
double minimum_crossing_depth = INFINITY;
double worst_contact_state_residual = 0.0;
double worst_crossed_state_residual = 0.0;
double worst_crossed_gap_residual = 0.0;
double worst_time_reverse_residual = 0.0;
double worst_pretrigger_residual = 0.0;
double worst_activation_residual = 0.0;
int worst_activation_tick_error = 0;
int minimum_activation_delay = 1000000;
int maximum_activation_delay = 0;
int maximum_journal_events = 0;
double worst_field_residual = 0.0;
double worst_translation_residual = 0.0;
double worst_polarity_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

ftd::Vec3 coordinate(ftd::Coord value) {
  return {static_cast<double>(value.x),
          static_cast<double>(value.y),
          static_cast<double>(value.z)};
}

ftd::Vec3 effective_position(ftd::Coord anchor,
                             const ftd::Vec3& remainder) {
  return coordinate(anchor) + remainder;
}

double pair_gap(const ftd::eft::NativeContactActiveSetGeometry& geometry,
                const ftd::Voxel& first,
                const ftd::Voxel& second) {
  return (effective_position(geometry.second_anchor, second.remainder)
      - effective_position(geometry.first_anchor, first.remainder))
      .dot(geometry.normal);
}

double global_field_magnitude(ftd::RenderBridge& bridge) {
  double result = 0.0;
  for (int i = 0; i < static_cast<int>(bridge.lattice().total_sites()); ++i) {
    const auto site = bridge.lattice().coord(i);
    const auto& voxel = bridge.voxel_at(site.x, site.y, site.z);
    result = std::max({result, voxel.flux.mag(), voxel.wave_vel.mag(),
        voxel.flux_L.mag(), voxel.flux_R.mag(),
        voxel.wave_vel_L.mag(), voxel.wave_vel_R.mag(),
        voxel.flux_strong.mag(), voxel.wave_vel_strong.mag(),
        voxel.flux_weak.mag(), voxel.wave_vel_weak.mag()});
  }
  return result;
}

double geometry_invariant_difference(
    const ftd::eft::NativeContactActiveSetGeometry& lhs,
    const ftd::eft::NativeContactActiveSetGeometry& rhs) {
  return std::max({
      std::abs(lhs.separated_gap-rhs.separated_gap),
      std::abs(lhs.contact_gap-rhs.contact_gap),
      std::abs(lhs.crossed_gap-rhs.crossed_gap),
      std::abs(lhs.contact_hop_margin-rhs.contact_hop_margin),
      std::abs(lhs.crossed_hop_margin-rhs.crossed_hop_margin),
      std::abs(lhs.exact_hop_delay-rhs.exact_hop_delay),
      static_cast<double>(std::abs(lhs.predicted_hop_delay_ticks
                                   - rhs.predicted_hop_delay_ticks))});
}

void initialize_bridge(ftd::RenderBridge& bridge,
                       const ftd::eft::NativeContactActiveSetGeometry& geometry,
                       int polarity,
                       const ftd::Vec3& first_remainder,
                       const ftd::Vec3& second_remainder,
                       const ftd::Vec3& first_velocity,
                       const ftd::Vec3& second_velocity) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.movement = true;
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
  first.remainder = first_remainder;
  second.remainder = second_remainder;
  first.velocity = first_velocity;
  second.velocity = second_velocity;
  first.particle_id = 101;
  second.particle_id = 102;
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool geometry_ok = true;
  bool crossing_ok = true;
  bool reverse_ok = true;
  bool activation_ok = true;
  bool transformed_ok = true;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (double speed : speeds) {
          std::array<ftd::eft::NativeContactActiveSetGeometry, 2>
              polarity_reference{};
          for (int polarity_index = 0; polarity_index < 2;
               ++polarity_index) {
            const int polarity = polarity_index == 0 ? -1 : +1;
            ftd::eft::NativeContactActiveSetGeometry translation_reference;
            for (std::size_t t = 0; t < translations.size(); ++t) {
              const auto translation = translations[t];
              const ftd::Coord source{
                  8 + translation.x, 8 + translation.y,
                  8 + translation.z};
              const ftd::Vec3 contact{
                  static_cast<double>(source.x) + 0.5 * dx,
                  static_cast<double>(source.y) + 0.5 * dy,
                  static_cast<double>(source.z) + 0.5 * dz};
              const auto geometry =
                  ftd::eft::analyze_native_contact_active_set_geometry(
                      L, contact, direction, polarity, speed, gate);
              geometry_ok = geometry_ok && geometry.valid
                  && geometry.charts.valid
                  && geometry.minimum_missing_charge == 0
                  && geometry.same_site_occupancy
                  && geometry.full_phase_distinguishes_gap
                  && geometry.gap_residual <= gate
                  && geometry.stable_chart_residual <= gate
                  && geometry.contact_hop_margin > gate
                  && geometry.crossed_hop_margin > gate
                  && geometry.predicted_hop_delay_ticks > 0;
              if (t == 0) translation_reference = geometry;
              else {
                worst_translation_residual = std::max(
                    worst_translation_residual,
                    geometry_invariant_difference(
                        translation_reference, geometry));
              }
              if (t == 1) {
                polarity_reference[static_cast<std::size_t>(
                    polarity_index)] = geometry;
              }
              worst_gap_residual = std::max(
                  worst_gap_residual, geometry.gap_residual);
              worst_stable_chart_residual = std::max(
                  worst_stable_chart_residual,
                  geometry.stable_chart_residual);
              minimum_contact_hop_margin = std::min(
                  minimum_contact_hop_margin,
                  geometry.contact_hop_margin);
              minimum_crossed_hop_margin = std::min(
                  minimum_crossed_hop_margin,
                  geometry.crossed_hop_margin);
              ++geometry_arms;

              const ftd::Vec3 incoming_first = geometry.normal * speed;
              const ftd::Vec3 incoming_second = incoming_first * -1.0;
              ftd::RenderBridge crossing(L);
              initialize_bridge(
                  crossing, geometry, polarity,
                  geometry.first_separated_remainder,
                  geometry.second_separated_remainder,
                  incoming_first, incoming_second);
              const int first_index = crossing.lattice().index(
                  geometry.first_anchor.x, geometry.first_anchor.y,
                  geometry.first_anchor.z);
              const int second_index = crossing.lattice().index(
                  geometry.second_anchor.x, geometry.second_anchor.y,
                  geometry.second_anchor.z);
              crossing.tick();
              const auto first_contact = crossing.voxel_at(
                  geometry.first_anchor.x, geometry.first_anchor.y,
                  geometry.first_anchor.z);
              const auto second_contact = crossing.voxel_at(
                  geometry.second_anchor.x, geometry.second_anchor.y,
                  geometry.second_anchor.z);
              const double contact_residual = std::max({
                  max_abs(first_contact.remainder
                          - geometry.first_contact_remainder),
                  max_abs(second_contact.remainder
                          - geometry.second_contact_remainder),
                  max_abs(first_contact.velocity-incoming_first),
                  max_abs(second_contact.velocity-incoming_second),
                  std::abs(pair_gap(geometry, first_contact,
                                    second_contact))});
              crossing.tick();
              const auto first_crossed = crossing.voxel_at(
                  geometry.first_anchor.x, geometry.first_anchor.y,
                  geometry.first_anchor.z);
              const auto second_crossed = crossing.voxel_at(
                  geometry.second_anchor.x, geometry.second_anchor.y,
                  geometry.second_anchor.z);
              const double measured_crossed_gap = pair_gap(
                  geometry, first_crossed, second_crossed);
              const double crossed_residual = std::max({
                  max_abs(first_crossed.remainder
                          - geometry.first_crossed_remainder),
                  max_abs(second_crossed.remainder
                          - geometry.second_crossed_remainder),
                  max_abs(first_crossed.velocity-incoming_first),
                  max_abs(second_crossed.velocity-incoming_second)});
              const int crossing_events = static_cast<int>(
                  crossing.history_events().size());
              const double crossing_field = global_field_magnitude(crossing);
              crossing_ok = crossing_ok
                  && crossing.state_at(first_index) == polarity
                  && crossing.state_at(second_index) == polarity
                  && contact_residual <= gate
                  && crossed_residual <= gate
                  && std::abs(measured_crossed_gap
                              - geometry.expected_crossed_gap) <= gate
                  && measured_crossed_gap < -gate
                  && crossing_events == 0
                  && crossing_field <= gate;
              worst_contact_state_residual = std::max(
                  worst_contact_state_residual, contact_residual);
              worst_crossed_state_residual = std::max(
                  worst_crossed_state_residual, crossed_residual);
              worst_crossed_gap_residual = std::max(
                  worst_crossed_gap_residual,
                  std::abs(measured_crossed_gap
                           - geometry.expected_crossed_gap));
              minimum_crossing_depth = std::min(
                  minimum_crossing_depth, -measured_crossed_gap);
              maximum_journal_events = std::max(
                  maximum_journal_events, crossing_events);
              worst_field_residual = std::max(
                  worst_field_residual, crossing_field);

              auto& first_reverse = crossing.voxel_at(
                  geometry.first_anchor.x, geometry.first_anchor.y,
                  geometry.first_anchor.z);
              auto& second_reverse = crossing.voxel_at(
                  geometry.second_anchor.x, geometry.second_anchor.y,
                  geometry.second_anchor.z);
              first_reverse.velocity *= -1.0;
              second_reverse.velocity *= -1.0;
              crossing.tick();
              crossing.tick();
              const auto first_restored = crossing.voxel_at(
                  geometry.first_anchor.x, geometry.first_anchor.y,
                  geometry.first_anchor.z);
              const auto second_restored = crossing.voxel_at(
                  geometry.second_anchor.x, geometry.second_anchor.y,
                  geometry.second_anchor.z);
              const double reverse_residual = std::max({
                  max_abs(first_restored.remainder
                          - geometry.first_separated_remainder),
                  max_abs(second_restored.remainder
                          - geometry.second_separated_remainder),
                  max_abs(first_restored.velocity+incoming_first),
                  max_abs(second_restored.velocity+incoming_second)});
              reverse_ok = reverse_ok
                  && crossing.state_at(first_index) == polarity
                  && crossing.state_at(second_index) == polarity
                  && reverse_residual <= gate
                  && crossing.history_events().empty();
              worst_time_reverse_residual = std::max(
                  worst_time_reverse_residual, reverse_residual);
              ++crossing_arms;

              ftd::RenderBridge activation(L);
              initialize_bridge(
                  activation, geometry, polarity,
                  geometry.first_contact_remainder,
                  geometry.second_contact_remainder,
                  incoming_first, {});
              int measured_activation_tick = 0;
              for (int tick = 1;
                   tick <= geometry.predicted_hop_delay_ticks; ++tick) {
                activation.tick();
                const auto mover = activation.voxel_at(
                    geometry.first_anchor.x, geometry.first_anchor.y,
                    geometry.first_anchor.z);
                const auto target = activation.voxel_at(
                    geometry.second_anchor.x, geometry.second_anchor.y,
                    geometry.second_anchor.z);
                if (tick < geometry.predicted_hop_delay_ticks) {
                  const ftd::Vec3 expected_remainder =
                      geometry.first_contact_remainder
                      + incoming_first * static_cast<double>(tick);
                  const double residual = std::max({
                      max_abs(mover.remainder-expected_remainder),
                      max_abs(mover.velocity-incoming_first),
                      max_abs(target.remainder
                              - geometry.second_contact_remainder),
                      target.velocity.mag()});
                  worst_pretrigger_residual = std::max(
                      worst_pretrigger_residual, residual);
                  activation_ok = activation_ok && residual <= gate;
                } else {
                  measured_activation_tick = tick;
                  const double residual = std::max({
                      mover.remainder.mag(),
                      max_abs(mover.velocity+incoming_first),
                      max_abs(target.remainder
                              - geometry.second_contact_remainder),
                      target.velocity.mag()});
                  worst_activation_residual = std::max(
                      worst_activation_residual, residual);
                  activation_ok = activation_ok && residual <= gate;
                }
              }
              const int activation_events = static_cast<int>(
                  activation.history_events().size());
              const double activation_field = global_field_magnitude(
                  activation);
              const int activation_error = std::abs(
                  measured_activation_tick
                  - geometry.predicted_hop_delay_ticks);
              activation_ok = activation_ok
                  && measured_activation_tick > 0
                  && activation_error == 0
                  && activation_events == 0
                  && activation_field <= gate;
              worst_activation_tick_error = std::max(
                  worst_activation_tick_error, activation_error);
              minimum_activation_delay = std::min(
                  minimum_activation_delay,
                  geometry.predicted_hop_delay_ticks);
              maximum_activation_delay = std::max(
                  maximum_activation_delay,
                  geometry.predicted_hop_delay_ticks);
              maximum_journal_events = std::max(
                  maximum_journal_events, activation_events);
              worst_field_residual = std::max(
                  worst_field_residual, activation_field);
              ++activation_arms;
            }
          }
          worst_polarity_residual = std::max(
              worst_polarity_residual,
              geometry_invariant_difference(
                  polarity_reference[0], polarity_reference[1]));
        }
      }
    }
  }
  transformed_ok = worst_translation_residual <= gate
      && worst_polarity_residual <= gate;

  check("same ternary anchors admit separated, contact, and crossed phases",
        geometry_ok && geometry_arms == 312
        && worst_gap_residual <= gate
        && worst_stable_chart_residual <= gate
        && minimum_contact_hop_margin > gate
        && minimum_crossed_hop_margin > gate);
  check("production crosses phi=0 without collision or field event",
        crossing_ok && crossing_arms == 312
        && worst_contact_state_residual <= gate
        && worst_crossed_state_residual <= gate
        && worst_crossed_gap_residual <= gate
        && minimum_crossing_depth > gate
        && maximum_journal_events == 0
        && worst_field_residual <= gate);
  check("crossed production histories reverse to the separated charts",
        reverse_ok && worst_time_reverse_residual <= gate);
  check("production collision activates only at the later hop threshold",
        activation_ok && activation_arms == 312
        && worst_pretrigger_residual <= gate
        && worst_activation_residual <= gate
        && worst_activation_tick_error == 0
        && minimum_activation_delay > 0
        && maximum_activation_delay >= minimum_activation_delay);
  check("translation and polarity copies preserve every discriminator",
        transformed_ok && worst_translation_residual <= gate
        && worst_polarity_residual <= gate);

  const auto invalid =
      ftd::eft::analyze_native_contact_active_set_geometry(
          2, {}, {}, +1, 0.25, gate);
  check("invalid active-set inputs fail closed", !invalid.valid);

  std::cout.precision(17);
  std::cout << "geometry_arms=" << geometry_arms << '\n'
            << "crossing_arms=" << crossing_arms << '\n'
            << "activation_arms=" << activation_arms << '\n'
            << "worst_gap_residual=" << worst_gap_residual << '\n'
            << "worst_stable_chart_residual="
            << worst_stable_chart_residual << '\n'
            << "minimum_contact_hop_margin="
            << minimum_contact_hop_margin << '\n'
            << "minimum_crossed_hop_margin="
            << minimum_crossed_hop_margin << '\n'
            << "minimum_crossing_depth=" << minimum_crossing_depth << '\n'
            << "worst_contact_state_residual="
            << worst_contact_state_residual << '\n'
            << "worst_crossed_state_residual="
            << worst_crossed_state_residual << '\n'
            << "worst_crossed_gap_residual="
            << worst_crossed_gap_residual << '\n'
            << "worst_time_reverse_residual="
            << worst_time_reverse_residual << '\n'
            << "worst_pretrigger_residual="
            << worst_pretrigger_residual << '\n'
            << "worst_activation_residual="
            << worst_activation_residual << '\n'
            << "worst_activation_tick_error="
            << worst_activation_tick_error << '\n'
            << "minimum_activation_delay="
            << minimum_activation_delay << '\n'
            << "maximum_activation_delay="
            << maximum_activation_delay << '\n'
            << "maximum_journal_events="
            << maximum_journal_events << '\n'
            << "worst_field_residual=" << worst_field_residual << '\n'
            << "worst_translation_residual="
            << worst_translation_residual << '\n'
            << "worst_polarity_residual="
            << worst_polarity_residual << '\n'
            << "native_contact_active_set failures=" << failures << '\n'
            << "verdict="
            << "HARD_CONTACT_REMAINS_SELECTED_PRODUCTION_ACTIVE_SET_IS_LATE\n";
  return failures == 0 ? 0 : 1;
}

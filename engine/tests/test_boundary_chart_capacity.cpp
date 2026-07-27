/** FTD-0507: boundary collision chart-capacity correction. */

#include "ftd/eft/boundary_chart_capacity.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double speed = 0.25;
constexpr double gate = 1e-12;
int failures = 0;
int formula_arms = 0;
int collision_arms = 0;
int production_arms = 0;
double worst_chart_position_residual = 0.0;
double worst_shape_residual = 0.0;
double worst_first_moment_residual = 0.0;
double worst_current_residual = 0.0;
double worst_continuity_residual = 0.0;
double minimum_outgoing_current_l1 = INFINITY;
std::array<double, 3> minimum_current_l1_by_shell{{
    INFINITY, INFINITY, INFINITY}};
std::array<double, 3> maximum_current_l1_by_shell{{0.0, 0.0, 0.0}};
double worst_production_drift_residual = 0.0;
double worst_production_separation_residual = 0.0;
double worst_production_inverse_residual = 0.0;
int maximum_production_journal_events = 0;
int production_state_mismatches = 0;
int production_journal_enable_failures = 0;
double worst_production_field_residual = 0.0;
double worst_seed_output_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

ftd::Vec3 effective_position(ftd::Coord anchor,
                             const ftd::Vec3& remainder) {
  return {static_cast<double>(anchor.x) + remainder.x,
          static_cast<double>(anchor.y) + remainder.y,
          static_cast<double>(anchor.z) + remainder.z};
}

double current_l1(const ftd::eft::PiecewiseCurrentSignature& signature) {
  double result = 0.0;
  for (std::size_t i = 0; i < signature.current_x.size(); ++i) {
    result += std::abs(signature.current_x[i]);
    result += std::abs(signature.current_y[i]);
    result += std::abs(signature.current_z[i]);
  }
  return result;
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

}  // namespace

int main() {
  bool count_formula_ok = true;
  const std::array<ftd::Vec3, 4> count_points{{
      {8.5, 8.5, 8.5},
      {8.0, 8.5, 8.5},
      {8.0, 8.0, 8.5},
      {8.0, 8.0, 8.0}}};
  const std::array<int, 4> expected_counts{{8, 4, 2, 1}};
  for (std::size_t p = 0; p < count_points.size(); ++p) {
    for (int multiplicity = 1; multiplicity <= 10; ++multiplicity) {
      for (int polarity : {-1, +1}) {
        const auto result = ftd::eft::analyze_boundary_chart_capacity(
            count_points[p], multiplicity, polarity, gate);
        const int expected_defect = std::max(
            0, multiplicity - expected_counts[p]);
        const int expected_occupancy =
            (multiplicity + expected_counts[p] - 1) / expected_counts[p];
        count_formula_ok = count_formula_ok && result.valid
            && result.chart_count == expected_counts[p]
            && result.distinct_anchor_count == expected_counts[p]
            && result.minimum_missing_charge == expected_defect
            && result.minimum_per_anchor_occupancy == expected_occupancy
            && result.minimum_chart_aware_alphabet_symbols
                == 2 * expected_occupancy + 1
            && result.canonical_single_anchor_defect == multiplicity - 1
            && result.canonical_single_anchor_alphabet_symbols
                == 2 * multiplicity + 1;
        worst_chart_position_residual = std::max(
            worst_chart_position_residual,
            result.chart_position_residual);
        worst_shape_residual = std::max({
            worst_shape_residual, result.chart_shape_residual,
            result.aggregate_shape_residual,
            std::abs(result.aggregate_charge_residual)});
        worst_first_moment_residual = std::max(
            worst_first_moment_residual,
            max_abs(result.aggregate_first_moment_residual));
        ++formula_arms;
      }
    }
  }
  check("stable chart multiplicity and charge capacity follow 2^(3-k)",
        count_formula_ok && formula_arms == 80
        && worst_chart_position_residual <= gate
        && worst_shape_residual <= gate
        && worst_first_moment_residual <= gate);

  const auto knot = ftd::eft::analyze_boundary_chart_capacity(
      {8.0, 8.0, 8.0}, 2, +1, gate);
  const auto face = ftd::eft::analyze_boundary_chart_capacity(
      {8.5, 8.0, 8.0}, 2, +1, gate);
  check("five-symbol lower bound survives only at a single stable chart",
        knot.valid && knot.chart_count == 1
        && knot.minimum_missing_charge == 1
        && knot.minimum_chart_aware_alphabet_symbols == 5
        && face.valid && face.chart_count == 2
        && face.minimum_missing_charge == 0
        && face.minimum_chart_aware_alphabet_symbols == 3);

  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  bool collision_ok = true;
  bool production_ok = true;
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        const double magnitude = std::sqrt(
            static_cast<double>(dx * dx + dy * dy + dz * dz));
        const ftd::Vec3 unit{dx / magnitude, dy / magnitude, dz / magnitude};
        const int hamming = (dx != 0) + (dy != 0) + (dz != 0);
        const int expected_chart_count = 1 << hamming;
        for (const auto& translation : translations) {
          const ftd::Coord source{
              8 + translation.x, 8 + translation.y, 8 + translation.z};
          const ftd::Vec3 collision_position{
              static_cast<double>(source.x) + 0.5 * dx,
              static_cast<double>(source.y) + 0.5 * dy,
              static_cast<double>(source.z) + 0.5 * dz};
          for (int polarity : {-1, +1}) {
            const auto collision =
                ftd::eft::analyze_boundary_chart_collision(
                    L, collision_position, direction, polarity, speed, gate);
            collision_ok = collision_ok && collision.valid
                && collision.capacity.chart_count == expected_chart_count
                && collision.capacity.minimum_missing_charge == 0
                && collision.first_chart.anchor.x == source.x
                && collision.first_chart.anchor.y == source.y
                && collision.first_chart.anchor.z == source.z
                && collision.endpoint_density_residual <= gate
                && collision.current_quotient_residual <= gate
                && collision.continuity_residual <= gate;
            worst_current_residual = std::max({
                worst_current_residual,
                collision.endpoint_density_residual,
                collision.current_quotient_residual});
            worst_continuity_residual = std::max(
                worst_continuity_residual,
                collision.continuity_residual);
            const double outgoing_current_l1 = current_l1(
                collision.bounce);
            minimum_outgoing_current_l1 = std::min(
                minimum_outgoing_current_l1, outgoing_current_l1);
            minimum_current_l1_by_shell[static_cast<std::size_t>(hamming - 1)] =
                std::min(minimum_current_l1_by_shell[
                             static_cast<std::size_t>(hamming - 1)],
                         outgoing_current_l1);
            maximum_current_l1_by_shell[static_cast<std::size_t>(hamming - 1)] =
                std::max(maximum_current_l1_by_shell[
                             static_cast<std::size_t>(hamming - 1)],
                         outgoing_current_l1);
            ++collision_arms;

            ftd::RenderBridge bridge(L);
            bridge.force_cpu();
            bridge.toggles.disable_all();
            bridge.toggles.movement = true;
            bridge.set_dt(1.0);
            const bool journal_enabled = bridge.enable_history_journal(true);
            production_ok = production_ok && journal_enabled;
            if (!journal_enabled) ++production_journal_enable_failures;
            const auto first_anchor = collision.first_chart.anchor;
            const auto second_anchor = collision.second_chart.anchor;
            const int first_index = bridge.lattice().index(
                first_anchor.x, first_anchor.y, first_anchor.z);
            const int second_index = bridge.lattice().index(
                second_anchor.x, second_anchor.y, second_anchor.z);
            bridge.set_state(first_index, static_cast<int8_t>(polarity));
            bridge.set_state(second_index, static_cast<int8_t>(polarity));
            auto& first = bridge.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            auto& second = bridge.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            first.remainder = collision.first_chart.remainder;
            second.remainder = collision.second_chart.remainder;
            first.velocity = unit * (-speed);
            second.velocity = unit * speed;
            first.particle_id = 101;
            second.particle_id = 102;
            const auto initial_first = first;
            const auto initial_second = second;
            bridge.tick();
            const auto first_after = bridge.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            const auto second_after = bridge.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            const ftd::Vec3 expected_first_remainder =
                initial_first.remainder - unit * speed;
            const ftd::Vec3 expected_second_remainder =
                initial_second.remainder + unit * speed;
            const double drift_residual = std::max({
                max_abs(first_after.remainder - expected_first_remainder),
                max_abs(second_after.remainder - expected_second_remainder),
                max_abs(first_after.velocity - initial_first.velocity),
                max_abs(second_after.velocity - initial_second.velocity)});
            const double separation = (
                effective_position(second_anchor, second_after.remainder)
                - effective_position(first_anchor, first_after.remainder)).mag();
            const double separation_residual = std::abs(
                separation - 2.0 * speed);
            const int journal_events = static_cast<int>(
                bridge.history_events().size());
            const bool first_state_ok = bridge.state_at(first_index) == polarity;
            const bool second_state_ok = bridge.state_at(second_index) == polarity;
            const double field_residual = global_field_magnitude(bridge);
            production_ok = production_ok
                && first_state_ok && second_state_ok
                && drift_residual <= gate
                && separation_residual <= gate
                && journal_events == 0
                && field_residual <= gate;
            if (!first_state_ok || !second_state_ok)
              ++production_state_mismatches;
            worst_production_field_residual = std::max(
                worst_production_field_residual, field_residual);
            worst_production_drift_residual = std::max(
                worst_production_drift_residual, drift_residual);
            worst_production_separation_residual = std::max(
                worst_production_separation_residual,
                separation_residual);
            maximum_production_journal_events = std::max(
                maximum_production_journal_events, journal_events);

            auto& first_reverse = bridge.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            auto& second_reverse = bridge.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            first_reverse.velocity *= -1.0;
            second_reverse.velocity *= -1.0;
            bridge.tick();
            const auto first_restored = bridge.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            const auto second_restored = bridge.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            const double inverse_residual = std::max({
                max_abs(first_restored.remainder - initial_first.remainder),
                max_abs(second_restored.remainder - initial_second.remainder),
                max_abs(first_restored.velocity + initial_first.velocity),
                max_abs(second_restored.velocity + initial_second.velocity)});
            const bool first_restored_state_ok =
                bridge.state_at(first_index) == polarity;
            const bool second_restored_state_ok =
                bridge.state_at(second_index) == polarity;
            const double restored_field_residual =
                global_field_magnitude(bridge);
            production_ok = production_ok
                && first_restored_state_ok && second_restored_state_ok
                && inverse_residual <= gate
                && restored_field_residual <= gate;
            if (!first_restored_state_ok || !second_restored_state_ok)
              ++production_state_mismatches;
            worst_production_field_residual = std::max(
                worst_production_field_residual,
                restored_field_residual);
            worst_production_inverse_residual = std::max(
                worst_production_inverse_residual, inverse_residual);

            // The tick advances its legacy RNG bookkeeping even with all
            // stochastic physics disabled.  Physical independence is tested
            // by replaying the same arm under a different seed.
            ftd::RenderBridge alternate(L);
            alternate.force_cpu();
            alternate.toggles.disable_all();
            alternate.toggles.movement = true;
            alternate.set_dt(1.0);
            alternate.seed_rng(123456u);
            alternate.set_state(first_index, static_cast<int8_t>(polarity));
            alternate.set_state(second_index, static_cast<int8_t>(polarity));
            auto& alternate_first = alternate.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            auto& alternate_second = alternate.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            alternate_first = initial_first;
            alternate_second = initial_second;
            alternate.tick();
            const auto alternate_first_after = alternate.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            const auto alternate_second_after = alternate.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            auto& alternate_first_reverse = alternate.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            auto& alternate_second_reverse = alternate.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            alternate_first_reverse.velocity *= -1.0;
            alternate_second_reverse.velocity *= -1.0;
            alternate.tick();
            const auto alternate_first_restored = alternate.voxel_at(
                first_anchor.x, first_anchor.y, first_anchor.z);
            const auto alternate_second_restored = alternate.voxel_at(
                second_anchor.x, second_anchor.y, second_anchor.z);
            const double seed_output_residual = std::max({
                max_abs(alternate_first_after.remainder
                        - first_after.remainder),
                max_abs(alternate_second_after.remainder
                        - second_after.remainder),
                max_abs(alternate_first_after.velocity
                        - first_after.velocity),
                max_abs(alternate_second_after.velocity
                        - second_after.velocity),
                max_abs(alternate_first_restored.remainder
                        - first_restored.remainder),
                max_abs(alternate_second_restored.remainder
                        - second_restored.remainder),
                max_abs(alternate_first_restored.velocity
                        - first_restored.velocity),
                max_abs(alternate_second_restored.velocity
                        - second_restored.velocity)});
            production_ok = production_ok
                && alternate.state_at(first_index) == polarity
                && alternate.state_at(second_index) == polarity
                && seed_output_residual <= gate
                && global_field_magnitude(alternate) <= gate;
            worst_seed_output_residual = std::max(
                worst_seed_output_residual, seed_output_residual);
            ++production_arms;
          }
        }
      }
    }
  }

  check("all face edge and corner boundary states have exact ternary capacity",
        collision_ok && collision_arms == 156
        && worst_current_residual <= gate
        && worst_continuity_residual <= gate);
  check("the trilinear face current hides axial relative separation only",
        maximum_current_l1_by_shell[0] <= gate
        && minimum_current_l1_by_shell[1] > gate
        && minimum_current_l1_by_shell[2] > gate);
  check("existing production state carries the selected outgoing collision phase",
        production_ok && production_arms == 156
        && worst_production_drift_residual <= gate
        && worst_production_separation_residual <= gate
        && worst_production_inverse_residual <= gate
        && worst_production_field_residual <= gate
        && worst_seed_output_residual <= gate
        && maximum_production_journal_events == 0
        && production_state_mismatches == 0
        && production_journal_enable_failures == 0);

  check("invalid inputs fail closed",
        !ftd::eft::analyze_boundary_chart_capacity(
            {NAN, 0.0, 0.0}, 2, +1).valid
        && !ftd::eft::analyze_boundary_chart_capacity(
            {0.5, 0.0, 0.0}, 0, +1).valid
        && !ftd::eft::analyze_boundary_chart_collision(
            L, {8.5, 8.0, 8.0}, {}, +1).valid);

  std::cout.precision(17);
  std::cout << "formula_arms=" << formula_arms << '\n'
            << "collision_arms=" << collision_arms << '\n'
            << "production_arms=" << production_arms << '\n'
            << "worst_chart_position_residual="
            << worst_chart_position_residual << '\n'
            << "worst_shape_residual=" << worst_shape_residual << '\n'
            << "worst_first_moment_residual="
            << worst_first_moment_residual << '\n'
            << "worst_current_residual=" << worst_current_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "minimum_outgoing_current_l1="
            << minimum_outgoing_current_l1 << '\n'
            << "face_current_l1_min=" << minimum_current_l1_by_shell[0]
            << '\n'
            << "face_current_l1_max=" << maximum_current_l1_by_shell[0]
            << '\n'
            << "edge_current_l1_min=" << minimum_current_l1_by_shell[1]
            << '\n'
            << "edge_current_l1_max=" << maximum_current_l1_by_shell[1]
            << '\n'
            << "corner_current_l1_min=" << minimum_current_l1_by_shell[2]
            << '\n'
            << "corner_current_l1_max=" << maximum_current_l1_by_shell[2]
            << '\n'
            << "worst_production_drift_residual="
            << worst_production_drift_residual << '\n'
            << "worst_production_separation_residual="
            << worst_production_separation_residual << '\n'
            << "worst_production_inverse_residual="
            << worst_production_inverse_residual << '\n'
            << "maximum_production_journal_events="
            << maximum_production_journal_events << '\n'
            << "production_state_mismatches="
            << production_state_mismatches << '\n'
            << "production_journal_enable_failures="
            << production_journal_enable_failures << '\n'
            << "worst_production_field_residual="
            << worst_production_field_residual << '\n'
            << "worst_seed_output_residual="
            << worst_seed_output_residual << '\n'
            << "boundary_chart_capacity failures=" << failures << '\n'
            << "verdict=BOUNDARY_CAPACITY_DEPENDS_ON_CHART_MULTIPLICITY\n";
  return failures == 0 ? 0 : 1;
}

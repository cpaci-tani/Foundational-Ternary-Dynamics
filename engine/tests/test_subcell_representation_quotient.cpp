/** FTD-0498: quotient factorization versus production anchor dependence. */

#include "ftd/eft/face_current_segment.h"
#include "ftd/eft/subcell_representation_quotient.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int current_chart_pairs = 0;
double worst_chart_position_residual = 0.0;
double worst_shape_residual = 0.0;
double worst_current_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_quotient_inverse_residual = 0.0;
double raw_state_l1_difference = 0.0;
double source_formula_residual = 0.0;
double source_response_difference = 0.0;
double collision_outcome_difference = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    residual = std::max(residual, std::abs(lhs[i] - rhs[i]));
  }
  return residual;
}

double max_current_difference(
    const ftd::eft::FaceCurrentSegment& lhs,
    const ftd::eft::FaceCurrentSegment& rhs) {
  return std::max({
      max_difference(lhs.rho_before, rhs.rho_before),
      max_difference(lhs.rho_after, rhs.rho_after),
      max_difference(lhs.current_x, rhs.current_x),
      max_difference(lhs.current_y, rhs.current_y),
      max_difference(lhs.current_z, rhs.current_z)});
}

std::vector<double> deposit_shape(
    const ftd::eft::SubcellChart& chart, int charge) {
  std::vector<double> rho(static_cast<std::size_t>(L * L * L), 0.0);
  const auto shape = ftd::eft::make_subcell_polarity_shape(
      chart.anchor, chart.remainder, charge);
  if (!shape.valid) return {};
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    const auto& entry = shape.weights[i];
    const int index = ((entry.site.x % L + L) % L) * L * L
        + ((entry.site.y % L + L) % L) * L
        + ((entry.site.z % L + L) % L);
    rho[static_cast<std::size_t>(index)] += entry.weight;
  }
  return rho;
}

ftd::eft::SubcellChart find_x_adjacent_chart(
    const std::vector<ftd::eft::SubcellChart>& charts,
    const ftd::eft::SubcellChart& reference) {
  for (const auto& chart : charts) {
    if (chart.anchor.x == reference.anchor.x + 1
        && chart.anchor.y == reference.anchor.y
        && chart.anchor.z == reference.anchor.z) {
      return chart;
    }
  }
  return {};
}

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

void seed_primary_bridge(ftd::RenderBridge& bridge,
                         const ftd::eft::SubcellChart& chart) {
  configure(bridge);
  const int index = bridge.lattice().index(
      chart.anchor.x, chart.anchor.y, chart.anchor.z);
  bridge.set_state(index, +1);
  auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
  voxel.remainder = chart.remainder;
  voxel.locked = true;
}

struct SourceProbe {
  ftd::RenderBridge bridge;
  double formula_residual = 0.0;

  explicit SourceProbe(const ftd::eft::SubcellChart& chart)
      : bridge(L) {
    seed_primary_bridge(bridge, chart);
    bridge.toggles.coupling = true;
    ftd::phase_read_main_loop(bridge);
    ftd::phase_write_main_loop(bridge);
    for (int index = 0;
         index < static_cast<int>(bridge.voxels().size()); ++index) {
      const auto expected = ftd::gradient_state_op(
          bridge.voxels(), bridge.lattice(), index) * -ftd::G_C;
      formula_residual = std::max(
          formula_residual,
          (bridge.voxels()[static_cast<std::size_t>(index)].wave_vel
           - expected).mag());
    }
  }
};

double state_l1(const ftd::RenderBridge& lhs,
                const ftd::RenderBridge& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.voxels().size(); ++i) {
    result += std::abs(static_cast<double>(lhs.voxels()[i].state)
                       - static_cast<double>(rhs.voxels()[i].state));
  }
  return result;
}

double wave_response_max_difference(const ftd::RenderBridge& lhs,
                                    const ftd::RenderBridge& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.voxels().size(); ++i) {
    result = std::max(
        result,
        (lhs.voxels()[i].wave_vel - rhs.voxels()[i].wave_vel).mag());
  }
  return result;
}

struct CollisionProbe {
  bool source_probe_remained = false;
  bool target_probe_arrived = false;
  double probe_velocity_x = 0.0;
};

CollisionProbe run_collision_probe(
    const ftd::eft::SubcellChart& primary,
    const ftd::Coord& shared_lower_anchor) {
  ftd::RenderBridge bridge(L);
  seed_primary_bridge(bridge, primary);
  const ftd::Coord probe_site{
      shared_lower_anchor.x - 1,
      shared_lower_anchor.y,
      shared_lower_anchor.z};
  const int probe = bridge.lattice().index(
      probe_site.x, probe_site.y, probe_site.z);
  const int target = bridge.lattice().index(
      shared_lower_anchor.x,
      shared_lower_anchor.y,
      shared_lower_anchor.z);
  bridge.set_state(probe, +1);
  auto& moving = bridge.voxels()[static_cast<std::size_t>(probe)];
  moving.remainder = {0.9, 0.0, 0.0};
  moving.velocity = {0.2, 0.0, 0.0};
  ftd::phase_movement_main_loop(bridge);

  CollisionProbe result;
  result.source_probe_remained =
      bridge.voxels()[static_cast<std::size_t>(probe)].state != 0;
  result.target_probe_arrived =
      bridge.voxels()[static_cast<std::size_t>(target)].state != 0;
  if (result.source_probe_remained) {
    result.probe_velocity_x =
        bridge.voxels()[static_cast<std::size_t>(probe)].velocity.x;
  } else if (result.target_probe_arrived) {
    result.probe_velocity_x =
        bridge.voxels()[static_cast<std::size_t>(target)].velocity.x;
  }
  return result;
}

}  // namespace

int main() {
  const ftd::Vec3 x0{8.23, 8.37, 8.41};
  const ftd::Vec3 x1{8.42, 8.16, 8.54};
  const ftd::Vec3 displacement = x1 - x0;
  const auto start_charts = ftd::eft::enumerate_subcell_charts(x0);
  const auto end_charts = ftd::eft::enumerate_subcell_charts(x1);
  check("generic three-dimensional point has eight stable charts",
        start_charts.size() == 8 && end_charts.size() == 8);
  check("plane, line, and knot multiplicities are 4, 2, and 1",
        ftd::eft::enumerate_subcell_charts({8.0, 8.37, 8.41}).size() == 4
        && ftd::eft::enumerate_subcell_charts({8.0, 9.0, 8.41}).size() == 2
        && ftd::eft::enumerate_subcell_charts({8.0, 9.0, 10.0}).size() == 1);

  bool positions_ok = true;
  for (const auto& chart : start_charts) {
    const double residual = max_abs(
        ftd::eft::subcell_chart_position(chart) - x0);
    worst_chart_position_residual = std::max(
        worst_chart_position_residual, residual);
    positions_ok = positions_ok && residual <= gate;
  }
  check("all chart representatives project to the same position",
        positions_ok);

  bool shape_ok = true;
  for (int charge : {-1, +1}) {
    const auto reference = deposit_shape(start_charts.front(), charge);
    for (const auto& chart : start_charts) {
      const double residual = max_difference(
          deposit_shape(chart, charge), reference);
      worst_shape_residual = std::max(worst_shape_residual, residual);
      shape_ok = shape_ok && residual <= gate;
    }
  }
  check("trilinear polarity factors through the chart quotient",
        shape_ok);

  bool current_ok = true;
  for (int charge : {-1, +1}) {
    const auto reference = ftd::eft::make_face_current_segment(
        L, start_charts.front().anchor, start_charts.front().remainder,
        end_charts.front().anchor, end_charts.front().remainder, charge);
    current_ok = current_ok && reference.valid;
    for (const auto& start : start_charts) {
      for (const auto& end : end_charts) {
        const auto trial = ftd::eft::make_face_current_segment(
            L, start.anchor, start.remainder,
            end.anchor, end.remainder, charge);
        ++current_chart_pairs;
        const double residual = trial.valid
            ? max_current_difference(trial, reference) : INFINITY;
        worst_current_residual = std::max(
            worst_current_residual, residual);
        worst_continuity_residual = std::max(
            worst_continuity_residual, trial.continuity_residual);
        current_ok = current_ok && trial.valid
            && residual <= gate && trial.continuity_residual <= gate;
      }
    }
  }
  check("all 128 signed start/end chart pairs deposit one exact current",
        current_ok && current_chart_pairs == 128);

  bool quotient_motion_ok = true;
  bool raw_inverse_failure_seen = false;
  const ftd::Vec3 threshold_displacement{0.8, 0.0, 0.0};
  const ftd::Vec3 threshold_endpoint = x0 + threshold_displacement;
  for (const auto& chart : start_charts) {
    const auto moved = ftd::eft::translate_subcell_chart(
        chart, threshold_displacement);
    const auto reversed = ftd::eft::translate_subcell_chart(
        moved, threshold_displacement * -1.0);
    const double forward_residual = max_abs(
        ftd::eft::subcell_chart_position(moved) - threshold_endpoint);
    const double inverse_residual = max_abs(
        ftd::eft::subcell_chart_position(reversed) - x0);
    worst_quotient_inverse_residual = std::max({
        worst_quotient_inverse_residual,
        forward_residual, inverse_residual});
    quotient_motion_ok = quotient_motion_ok && moved.valid
        && reversed.valid && forward_residual <= gate
        && inverse_residual <= gate;
    raw_inverse_failure_seen = raw_inverse_failure_seen
        || reversed.anchor.x != chart.anchor.x
        || reversed.anchor.y != chart.anchor.y
        || reversed.anchor.z != chart.anchor.z
        || max_abs(reversed.remainder - chart.remainder) > gate;
  }
  check("threshold dynamics is exactly invertible on the quotient",
        quotient_motion_ok);
  check("the same quotient motion is not invertible on every raw chart",
        raw_inverse_failure_seen);

  const auto lower = start_charts.front();
  const auto adjacent = find_x_adjacent_chart(start_charts, lower);
  check("locked production representatives are chart-equivalent",
        adjacent.valid
        && ftd::eft::equivalent_subcell_charts(lower, adjacent)
        && max_difference(deposit_shape(lower, +1),
                          deposit_shape(adjacent, +1)) <= gate);

  ftd::RenderBridge raw_lower(L);
  ftd::RenderBridge raw_adjacent(L);
  seed_primary_bridge(raw_lower, lower);
  seed_primary_bridge(raw_adjacent, adjacent);
  raw_state_l1_difference = state_l1(raw_lower, raw_adjacent);
  check("raw ternary manifestation does not factor through the quotient",
        std::abs(raw_state_l1_difference - 2.0) <= gate);

  SourceProbe source_lower(lower);
  SourceProbe source_adjacent(adjacent);
  source_formula_residual = std::max(
      source_lower.formula_residual, source_adjacent.formula_residual);
  source_response_difference = wave_response_max_difference(
      source_lower.bridge, source_adjacent.bridge);
  check("both source probes execute the exact native -G_C grad(s) rule",
        source_formula_residual <= gate);
  check("native state-flux source depends on the chosen raw anchor",
        source_response_difference > 1e-6);

  const auto collision_lower = run_collision_probe(lower, lower.anchor);
  const auto collision_adjacent = run_collision_probe(adjacent, lower.anchor);
  collision_outcome_difference = std::abs(
      collision_lower.probe_velocity_x
      - collision_adjacent.probe_velocity_x);
  check("equivalent primary charts produce different collision outcomes",
        collision_lower.source_probe_remained
        && collision_lower.target_probe_arrived
        && !collision_adjacent.source_probe_remained
        && collision_adjacent.target_probe_arrived
        && collision_lower.probe_velocity_x < 0.0
        && collision_adjacent.probe_velocity_x > 0.0
        && collision_outcome_difference > 0.1);

  check("invalid chart input fails closed",
        !ftd::eft::translate_subcell_chart({}, {0.1, 0.0, 0.0}).valid
        && ftd::eft::enumerate_subcell_charts({NAN, 0.0, 0.0}).empty());

  std::cout.precision(17);
  std::cout << "generic_chart_count=" << start_charts.size() << '\n'
            << "current_chart_pairs=" << current_chart_pairs << '\n'
            << "worst_chart_position_residual="
            << worst_chart_position_residual << '\n'
            << "worst_shape_residual=" << worst_shape_residual << '\n'
            << "worst_current_residual=" << worst_current_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_quotient_inverse_residual="
            << worst_quotient_inverse_residual << '\n'
            << "raw_state_l1_difference="
            << raw_state_l1_difference << '\n'
            << "source_formula_residual="
            << source_formula_residual << '\n'
            << "source_response_difference="
            << source_response_difference << '\n'
            << "collision_outcome_difference="
            << collision_outcome_difference << '\n'
            << "subcell_representation_quotient failures="
            << failures << '\n'
            << "verdict=FACE_PHYSICS_FACTORS_PRODUCTION_DOES_NOT\n";
  return failures == 0 ? 0 : 1;
}

/** FTD-0500: canonical-section repair and exact half-cell obstruction. */

#include "ftd/eft/canonical_subcell_section.h"
#include "ftd/eft/face_current_segment.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int cubic_maps = 0;
int translated_grid_points = 0;
int face_chart_comparisons = 0;
int tie_reversal_failures = 0;
int off_tie_reversal_failures = 0;
bool preregistered_reversal_gate_passed = true;
double worst_position_residual = 0.0;
double worst_translation_residual = 0.0;
double worst_cubic_residual = 0.0;
double worst_reversal_residual = 0.0;
double worst_physical_reversal_residual = 0.0;
double worst_shape_residual = 0.0;
double worst_current_residual = 0.0;
double half_cell_anchor_mismatch = 0.0;
double half_cell_remainder_mismatch = 0.0;
double threshold_shift = 0.0;
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
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  }
  return result;
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

ftd::Vec3 permute_signed(const ftd::Vec3& value,
                         const std::array<int, 3>& permutation,
                         const std::array<int, 3>& sign) {
  const std::array<double, 3> source{value.x, value.y, value.z};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

ftd::Coord permute_signed(const ftd::Coord& value,
                          const std::array<int, 3>& permutation,
                          const std::array<int, 3>& sign) {
  const std::array<int, 3> source{value.x, value.y, value.z};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

double chart_raw_difference(const ftd::eft::SubcellChart& lhs,
                            const ftd::eft::SubcellChart& rhs) {
  return std::max({
      static_cast<double>(std::abs(lhs.anchor.x - rhs.anchor.x)),
      static_cast<double>(std::abs(lhs.anchor.y - rhs.anchor.y)),
      static_cast<double>(std::abs(lhs.anchor.z - rhs.anchor.z)),
      max_abs(lhs.remainder - rhs.remainder)});
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

  explicit SourceProbe(const ftd::eft::SubcellChart& chart) : bridge(L) {
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
  const ftd::Coord probe_site{shared_lower_anchor.x - 1,
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
  bool grid_ok = true;
  bool off_tie_reversal_ok = true;
  bool tie_reversal_failure_seen = false;
  const std::array<ftd::Vec3, 4> displacements{{
      {0.02, -0.03, 0.04}, {-0.73, 0.61, -0.29},
      {1.2, -1.4, 0.8}, {-2.25, 1.75, 0.125}}};
  for (int ix = -6; ix <= 6; ++ix) {
    for (int iy = -4; iy <= 4; ++iy) {
      for (int iz = -3; iz <= 3; ++iz) {
        const ftd::Vec3 point{0.125 * ix, 0.125 * iy, 0.125 * iz};
        const auto chart = ftd::eft::centered_canonical_subcell_chart(point);
        ++translated_grid_points;
        const double position_residual = chart.valid
            ? max_abs(ftd::eft::subcell_chart_position(chart) - point)
            : INFINITY;
        worst_position_residual = std::max(
            worst_position_residual, position_residual);
        grid_ok = grid_ok && chart.valid && position_residual <= gate
            && chart.remainder.x >= -0.5 && chart.remainder.x < 0.5
            && chart.remainder.y >= -0.5 && chart.remainder.y < 0.5
            && chart.remainder.z >= -0.5 && chart.remainder.z < 0.5;
        for (const auto& displacement : displacements) {
          const auto moved = ftd::eft::translate_centered_canonical_chart(
              chart, displacement);
          const auto reversed = ftd::eft::translate_centered_canonical_chart(
              moved, displacement * -1.0);
          const double residual = reversed.valid
              ? chart_raw_difference(reversed, chart) : INFINITY;
          const double physical_residual = reversed.valid
              ? max_abs(ftd::eft::subcell_chart_position(reversed)
                        - ftd::eft::subcell_chart_position(chart))
              : INFINITY;
          worst_reversal_residual = std::max(
              worst_reversal_residual, residual);
          worst_physical_reversal_residual = std::max(
              worst_physical_reversal_residual, physical_residual);
          const bool on_tie = std::abs(chart.remainder.x + 0.5) <= gate
              || std::abs(chart.remainder.y + 0.5) <= gate
              || std::abs(chart.remainder.z + 0.5) <= gate;
          if (!moved.valid || !reversed.valid || residual > gate) {
            preregistered_reversal_gate_passed = false;
            if (on_tie) {
              ++tie_reversal_failures;
              tie_reversal_failure_seen = true;
            } else {
              ++off_tie_reversal_failures;
              off_tie_reversal_ok = false;
            }
          }
        }
      }
    }
  }
  check("centered section is unique and reproduces position on locked grid",
        grid_ok && translated_grid_points == 819);
  check("locked raw reversal gate fails only at reachable half-cell ties",
        !preregistered_reversal_gate_passed
        && tie_reversal_failure_seen && off_tie_reversal_ok
        && off_tie_reversal_failures == 0
        && worst_physical_reversal_residual <= gate);

  bool translation_ok = true;
  const ftd::Vec3 base{1.13, -2.27, 3.39};
  const auto base_chart = ftd::eft::centered_canonical_subcell_chart(base);
  for (int dx = -3; dx <= 3; ++dx) {
    for (int dy = -3; dy <= 3; ++dy) {
      for (int dz = -3; dz <= 3; ++dz) {
        const auto shifted = ftd::eft::centered_canonical_subcell_chart(
            base + ftd::Vec3{static_cast<double>(dx),
                             static_cast<double>(dy),
                             static_cast<double>(dz)});
        ftd::eft::SubcellChart expected = base_chart;
        expected.anchor.x += dx;
        expected.anchor.y += dy;
        expected.anchor.z += dz;
        const double residual = chart_raw_difference(shifted, expected);
        worst_translation_residual = std::max(
            worst_translation_residual, residual);
        translation_ok = translation_ok && shifted.valid
            && residual <= gate;
      }
    }
  }
  check("centered section is exactly integer-translation covariant",
        translation_ok);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  bool cubic_off_ties_ok = true;
  for (const auto& permutation : permutations) {
    for (int mask = 0; mask < 8; ++mask) {
      const std::array<int, 3> sign{{
          (mask & 1) ? -1 : +1,
          (mask & 2) ? -1 : +1,
          (mask & 4) ? -1 : +1}};
      const auto transformed = ftd::eft::centered_canonical_subcell_chart(
          permute_signed(base, permutation, sign));
      ftd::eft::SubcellChart expected;
      expected.anchor = permute_signed(base_chart.anchor, permutation, sign);
      expected.remainder = permute_signed(
          base_chart.remainder, permutation, sign);
      expected.valid = true;
      const double residual = chart_raw_difference(transformed, expected);
      worst_cubic_residual = std::max(worst_cubic_residual, residual);
      cubic_off_ties_ok = cubic_off_ties_ok && transformed.valid
          && residual <= gate;
      ++cubic_maps;
    }
  }
  check("all 48 cubic maps commute with the section away from ties",
        cubic_off_ties_ok && cubic_maps == 48);

  const auto obstruction =
      ftd::eft::analyze_half_cell_section_obstruction();
  half_cell_anchor_mismatch = obstruction.raw_anchor_mismatch;
  half_cell_remainder_mismatch = obstruction.raw_remainder_mismatch;
  check("half-cell translation and inversion demand impossible 2a=1",
        obstruction.valid && !obstruction.integer_solution_exists
        && obstruction.diophantine_residual == 1
        && obstruction.translation_predicted_negative_anchor == 0
        && obstruction.inversion_predicted_negative_anchor == -1);
  check("selected tie chart breaks raw inversion but not physical inversion",
        std::abs(half_cell_anchor_mismatch - 1.0) <= gate
        && std::abs(half_cell_remainder_mismatch - 1.0) <= gate
        && obstruction.physical_inversion_residual <= gate);

  const ftd::Vec3 x0{8.23, 8.37, 8.41};
  const ftd::Vec3 x1{8.42, 8.16, 8.54};
  const auto canonical_start =
      ftd::eft::centered_canonical_subcell_chart(x0);
  const auto canonical_end =
      ftd::eft::centered_canonical_subcell_chart(x1);
  const auto start_charts = ftd::eft::enumerate_subcell_charts(x0);
  const auto end_charts = ftd::eft::enumerate_subcell_charts(x1);
  bool face_ok = canonical_start.valid && canonical_end.valid
      && start_charts.size() == 8 && end_charts.size() == 8;
  for (int charge : {-1, +1}) {
    const auto shape_reference = deposit_shape(canonical_start, charge);
    const auto current_reference = ftd::eft::make_face_current_segment(
        L, canonical_start.anchor, canonical_start.remainder,
        canonical_end.anchor, canonical_end.remainder, charge);
    face_ok = face_ok && current_reference.valid;
    for (const auto& start : start_charts) {
      worst_shape_residual = std::max(
          worst_shape_residual,
          max_difference(shape_reference, deposit_shape(start, charge)));
      for (const auto& end : end_charts) {
        const auto trial = ftd::eft::make_face_current_segment(
            L, start.anchor, start.remainder,
            end.anchor, end.remainder, charge);
        const double residual = trial.valid
            ? max_current_difference(current_reference, trial) : INFINITY;
        worst_current_residual = std::max(
            worst_current_residual, residual);
        face_ok = face_ok && trial.valid && residual <= gate;
        ++face_chart_comparisons;
      }
    }
  }
  check("canonical and all overlapping charts have one shape and current",
        face_ok && worst_shape_residual <= gate
        && worst_current_residual <= gate
        && face_chart_comparisons == 128);

  ftd::eft::SubcellChart production_start;
  production_start.anchor = {8, 8, 8};
  production_start.remainder = {0.49, 0.0, 0.0};
  production_start.valid = true;
  const ftd::Vec3 small_step{0.02, 0.0, 0.0};
  const auto legacy_output = ftd::eft::translate_subcell_chart(
      production_start, small_step);
  const auto canonical_output = ftd::eft::translate_centered_canonical_chart(
      production_start, small_step);
  threshold_shift = 1.0 - 0.5;
  check("canonical section changes the positive hop boundary by half a site",
        legacy_output.valid && canonical_output.valid
        && legacy_output.anchor.x == 8
        && canonical_output.anchor.x == 9
        && std::abs(legacy_output.remainder.x - 0.51) <= gate
        && std::abs(canonical_output.remainder.x + 0.49) <= gate
        && ftd::eft::equivalent_subcell_charts(
            legacy_output, canonical_output)
        && std::abs(threshold_shift - 0.5) <= gate);
  const auto canonical_reverse =
      ftd::eft::translate_centered_canonical_chart(
          canonical_output, small_step * -1.0);
  check("canonical trajectory repairs raw reversal",
        chart_raw_difference(canonical_reverse, production_start) <= gate);

  ftd::RenderBridge raw_legacy(L);
  ftd::RenderBridge raw_canonical(L);
  seed_primary_bridge(raw_legacy, legacy_output);
  seed_primary_bridge(raw_canonical, canonical_output);
  raw_state_l1_difference = state_l1(raw_legacy, raw_canonical);
  check("canonical hop timing changes primitive ternary manifestation",
        std::abs(raw_state_l1_difference - 2.0) <= gate);

  SourceProbe source_legacy(legacy_output);
  SourceProbe source_canonical(canonical_output);
  source_formula_residual = std::max(
      source_legacy.formula_residual, source_canonical.formula_residual);
  source_response_difference = wave_response_max_difference(
      source_legacy.bridge, source_canonical.bridge);
  check("both source arms execute exact native -G_C grad(s)",
        source_formula_residual <= gate);
  check("canonical hop timing changes native source response",
        source_response_difference > 1e-6);

  const auto collision_legacy = run_collision_probe(
      legacy_output, legacy_output.anchor);
  const auto collision_canonical = run_collision_probe(
      canonical_output, legacy_output.anchor);
  collision_outcome_difference = std::abs(
      collision_legacy.probe_velocity_x
      - collision_canonical.probe_velocity_x);
  check("canonical hop timing changes production collision outcome",
        collision_legacy.source_probe_remained
        && collision_legacy.target_probe_arrived
        && !collision_canonical.source_probe_remained
        && collision_canonical.target_probe_arrived
        && collision_legacy.probe_velocity_x < 0.0
        && collision_canonical.probe_velocity_x > 0.0
        && collision_outcome_difference > 0.1);

  check("invalid canonical inputs fail closed",
        !ftd::eft::centered_canonical_subcell_chart(
            {NAN, 0.0, 0.0}).valid
        && !ftd::eft::translate_centered_canonical_chart(
            {}, {0.1, 0.0, 0.0}).valid);

  std::cout.precision(17);
  std::cout << "translated_grid_points=" << translated_grid_points << '\n'
            << "cubic_maps=" << cubic_maps << '\n'
            << "face_chart_comparisons=" << face_chart_comparisons << '\n'
            << "tie_reversal_failures=" << tie_reversal_failures << '\n'
            << "off_tie_reversal_failures="
            << off_tie_reversal_failures << '\n'
            << "preregistered_reversal_gate_passed="
            << (preregistered_reversal_gate_passed ? "true" : "false")
            << '\n'
            << "worst_position_residual=" << worst_position_residual << '\n'
            << "worst_translation_residual="
            << worst_translation_residual << '\n'
            << "worst_cubic_residual=" << worst_cubic_residual << '\n'
            << "worst_reversal_residual=" << worst_reversal_residual << '\n'
            << "worst_physical_reversal_residual="
            << worst_physical_reversal_residual << '\n'
            << "worst_shape_residual=" << worst_shape_residual << '\n'
            << "worst_current_residual=" << worst_current_residual << '\n'
            << "half_cell_anchor_mismatch="
            << half_cell_anchor_mismatch << '\n'
            << "half_cell_remainder_mismatch="
            << half_cell_remainder_mismatch << '\n'
            << "threshold_shift=" << threshold_shift << '\n'
            << "raw_state_l1_difference="
            << raw_state_l1_difference << '\n'
            << "source_formula_residual="
            << source_formula_residual << '\n'
            << "source_response_difference="
            << source_response_difference << '\n'
            << "collision_outcome_difference="
            << collision_outcome_difference << '\n'
            << "canonical_subcell_section failures=" << failures << '\n'
            << "verdict=CANONICAL_CHART_REQUIRES_RULE_REWRITE\n";
  return failures == 0 ? 0 : 1;
}

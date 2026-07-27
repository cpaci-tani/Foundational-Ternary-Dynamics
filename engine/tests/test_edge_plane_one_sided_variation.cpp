/** FTD-0539: in-plane roots and one-sided normal variation. */

#include "ftd/eft/edge_plane_one_sided_variation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double algebra_gate = 1e-12;
constexpr double root_gate = 1e-8;
constexpr double derivative_gate = 1e-7;
constexpr double derivative_step = 0.000244140625;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double component(ftd::Vec3 value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

int normal_axis(ftd::Coord direction) {
  if (direction.x == 0) return 0;
  if (direction.y == 0) return 1;
  return 2;
}

double active_residual(
    const ftd::eft::AtomicFaceEndpointTrialResult& trial,
    int normal) {
  double result = 0.0;
  for (const auto& residual : trial.start_residual)
    for (int axis = 0; axis < 3; ++axis)
      if (axis != normal)
        result = std::max(result, std::abs(component(residual, axis)));
  return result;
}

double active_difference(
    const ftd::eft::AtomicFaceEndpointTrialResult& lhs,
    const ftd::eft::AtomicFaceEndpointTrialResult& rhs,
    int normal) {
  double result = 0.0;
  for (int carrier = 0; carrier < 2; ++carrier)
    for (int axis = 0; axis < 3; ++axis)
      if (axis != normal)
        result = std::max(result, std::abs(
            component(lhs.start_residual[static_cast<std::size_t>(carrier)], axis)
            -component(rhs.start_residual[static_cast<std::size_t>(carrier)], axis)));
  return result;
}

ftd::Vec3 rotate_from_canonical(ftd::Vec3 value,
                                ftd::Coord direction) {
  int active[2]{};
  int missing = 0;
  int count = 0;
  const int d[3] = {direction.x, direction.y, direction.z};
  for (int axis = 0; axis < 3; ++axis) {
    if (d[axis] != 0) active[count++] = axis;
    else missing = axis;
  }
  double out[3]{};
  out[active[0]] = d[active[0]]*value.x;
  out[active[1]] = d[active[1]]*value.y;
  out[missing] = value.z;
  return {out[0], out[1], out[2]};
}

double max_component(ftd::Vec3 value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

ftd::eft::AtomicFaceEndpointTrialResult evaluate(
    const ftd::eft::ImplicitAtomicInitialFixture& fixture,
    const std::array<ftd::Vec3, 2>& endpoint,
    double step) {
  return ftd::eft::evaluate_atomic_face_endpoint_trial(
      fixture.start_position, endpoint, fixture.charge,
      fixture.prescribed_kinetic_start,
      fixture.potential_before, fixture.electric_before,
      fixture.beta, fixture.temporal_scale,
      ftd::E_REST, ftd::C_SPEED, step, algebra_gate, false);
}

}  // namespace

int main() {
  const std::array<double, 2> speeds{{0.125, 0.25}};
  std::array<ftd::eft::EdgePlaneOneSidedVariationResult, 2> canonical{};
  bool canonical_ok = true;
  for (int speed_index = 0; speed_index < 2; ++speed_index) {
    canonical[static_cast<std::size_t>(speed_index)] =
        ftd::eft::solve_edge_plane_one_sided_variation(
            L, {8.5, 8.5, 8.0}, {1, 1, 0}, +1,
            speeds[static_cast<std::size_t>(speed_index)],
            derivative_step, root_gate, derivative_gate, algebra_gate);
    const auto& solve = canonical[static_cast<std::size_t>(speed_index)];
    canonical_ok = canonical_ok && solve.valid;
    std::cout.precision(17);
    std::cout << "canonical_speed="
              << speeds[static_cast<std::size_t>(speed_index)]
              << " valid=" << solve.valid
              << " iterations=" << solve.iterations
              << " active_root=" << solve.final_active_residual
              << " active_derivative_convergence="
              << solve.active_derivative_convergence
              << " normal_derivative_convergence="
              << solve.normal.derivative_convergence
              << " normal_jump=" << solve.maximum_normal_residual_jump
              << " differentiable=" << solve.normal_differentiable
              << " interval_contains_zero="
              << solve.normal_interval_contains_zero
              << " c0_left=" << solve.normal.incoming_residual_left[0]
              << " c0_right=" << solve.normal.incoming_residual_right[0]
              << " c1_left=" << solve.normal.incoming_residual_left[1]
              << " c1_right=" << solve.normal.incoming_residual_right[1]
              << " ordinary_energy="
              << solve.trial.ordinary_total_energy_defect
              << " modified_energy="
              << solve.trial.modified_total_energy_defect << '\n';
  }
  check("both canonical in-plane roots and one-sided diagnostics close",
        canonical_ok);

  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  int arms = 0;
  int differentiable_arms = 0;
  int interval_arms = 0;
  int nonstationary_arms = 0;
  double worst_active_root = 0.0;
  double worst_active_convergence = 0.0;
  double worst_one_sided_convergence = 0.0;
  double minimum_normal_residual = INFINITY;
  double maximum_normal_residual = -INFINITY;
  double minimum_normal_jump = INFINITY;
  double maximum_normal_jump = 0.0;
  double worst_split = 0.0;
  double worst_continuity = 0.0;
  double worst_field = 0.0;
  double worst_gauss = 0.0;
  double worst_causal = 0.0;
  double minimum_ordinary = INFINITY;
  double maximum_ordinary = 0.0;
  double minimum_modified = INFINITY;
  double maximum_modified = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;
  bool orbit_ok = canonical_ok;

  struct Metric {
    ftd::Coord direction{};
    int speed_index = 0;
    int polarity = 0;
    int translation_index = 0;
    double root = 0.0;
    double jump = 0.0;
    double ordinary = 0.0;
    double modified = 0.0;
  };
  std::vector<Metric> metrics;
  metrics.reserve(144);

  const ftd::Coord canonical_direction{1, 1, 0};
  const ftd::Vec3 canonical_contact{8.5, 8.5, 8.0};
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx*dx+dy*dy+dz*dz != 2) continue;
        const ftd::Coord direction{dx, dy, dz};
        const int normal = normal_axis(direction);
        for (int speed_index = 0; speed_index < 2; ++speed_index) {
          const auto& source = canonical[static_cast<std::size_t>(speed_index)];
          for (int polarity : {-1, +1}) {
            for (int translation_index = 0; translation_index < 3;
                 ++translation_index) {
              const auto translation = translations[
                  static_cast<std::size_t>(translation_index)];
              const ftd::Coord base{
                  8+translation.x, 8+translation.y, 8+translation.z};
              const ftd::Vec3 contact{
                  base.x+0.5*dx, base.y+0.5*dy, base.z+0.5*dz};
              const auto fixture =
                  ftd::eft::make_implicit_atomic_initial_fixture(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)],
                      algebra_gate);
              std::array<ftd::Vec3, 2> endpoint{};
              bool matched[2] = {false, false};
              for (int source_index = 0; source_index < 2; ++source_index) {
                const ftd::Vec3 relative = rotate_from_canonical(
                    source.fixture.start_position[
                        static_cast<std::size_t>(source_index)]
                    -canonical_contact, direction);
                int target_match = -1;
                for (int target = 0; target < 2; ++target) {
                  if (max_component(
                      fixture.start_position[static_cast<std::size_t>(target)]
                      -contact-relative) <= 1e-10) target_match = target;
                }
                if (target_match < 0 || matched[target_match]) {
                  orbit_ok = false;
                  continue;
                }
                matched[target_match] = true;
                endpoint[static_cast<std::size_t>(target_match)] =
                    fixture.start_position[static_cast<std::size_t>(target_match)]
                    +rotate_from_canonical(
                        source.displacement[
                            static_cast<std::size_t>(source_index)],
                        direction);
              }
              const auto trial = evaluate(fixture, endpoint, derivative_step);
              const auto fine = evaluate(
                  fixture, endpoint, derivative_step/2.0);
              const auto one_sided =
                  ftd::eft::evaluate_atomic_face_one_sided_normal(
                      fixture.start_position, endpoint, fixture.charge,
                      fixture.prescribed_kinetic_start,
                      fixture.potential_before, fixture.electric_before,
                      fixture.beta, fixture.temporal_scale,
                      ftd::E_REST, ftd::C_SPEED, normal,
                      derivative_step, algebra_gate);
              const double root = active_residual(trial, normal);
              const double convergence = active_difference(trial, fine, normal);
              bool differentiable = true;
              bool interval = true;
              double jump = 0.0;
              for (int carrier = 0; carrier < 2; ++carrier) {
                const std::size_t i = static_cast<std::size_t>(carrier);
                const double left = one_sided.incoming_residual_left[i];
                const double right = one_sided.incoming_residual_right[i];
                minimum_normal_residual = std::min({
                    minimum_normal_residual, left, right});
                maximum_normal_residual = std::max({
                    maximum_normal_residual, left, right});
                jump = std::max(jump, std::abs(right-left));
                differentiable = differentiable
                    && std::abs(right-left) <= derivative_gate;
                interval = interval
                    && std::min(left, right) <= derivative_gate
                    && std::max(left, right) >= -derivative_gate;
              }
              differentiable_arms += differentiable ? 1 : 0;
              interval_arms += interval ? 1 : 0;
              nonstationary_arms += interval ? 0 : 1;
              ++arms;
              orbit_ok = orbit_ok && fixture.valid && trial.valid && fine.valid
                  && one_sided.valid && root <= root_gate
                  && convergence <= derivative_gate
                  && one_sided.derivative_convergence <= derivative_gate;
              worst_active_root = std::max(worst_active_root, root);
              worst_active_convergence = std::max(
                  worst_active_convergence, convergence);
              worst_one_sided_convergence = std::max(
                  worst_one_sided_convergence,
                  one_sided.derivative_convergence);
              minimum_normal_jump = std::min(minimum_normal_jump, jump);
              maximum_normal_jump = std::max(maximum_normal_jump, jump);
              worst_split = std::max(worst_split,
                  trial.current_split_residual);
              worst_continuity = std::max(worst_continuity,
                  trial.continuity_residual);
              worst_field = std::max({worst_field,
                  trial.field_start_equation_residual,
                  trial.field_end_equation_residual,
                  trial.field_update_residual});
              worst_gauss = std::max(
                  worst_gauss, trial.gauss_evolution_residual);
              worst_causal = std::max(worst_causal, trial.causal_excess);
              const double ordinary = std::abs(
                  trial.ordinary_total_energy_defect);
              const double modified = std::abs(
                  trial.modified_total_energy_defect);
              minimum_ordinary = std::min(minimum_ordinary, ordinary);
              maximum_ordinary = std::max(maximum_ordinary, ordinary);
              minimum_modified = std::min(minimum_modified, modified);
              maximum_modified = std::max(maximum_modified, modified);
              metrics.push_back({direction, speed_index, polarity,
                  translation_index, root, jump, ordinary, modified});
            }
          }
        }
      }
    }
  }

  check("both in-plane roots transport to all 144 edge arms", orbit_ok
        && arms == 144);
  const bool algebra_closed = worst_split <= algebra_gate
      && worst_continuity <= algebra_gate
      && worst_field <= algebra_gate
      && worst_gauss <= algebra_gate
      && worst_causal <= algebra_gate;
  check("current field Gauss causality and derivative gates close",
        algebra_closed && worst_active_convergence <= derivative_gate
        && worst_one_sided_convergence <= derivative_gate);

  for (const auto& metric : metrics) {
    for (const auto& other : metrics) {
      if (metric.speed_index != other.speed_index) continue;
      const double difference = std::max({
          std::abs(metric.root-other.root),
          std::abs(metric.jump-other.jump),
          std::abs(metric.ordinary-other.ordinary),
          std::abs(metric.modified-other.modified)});
      if (metric.direction.x == other.direction.x
          && metric.direction.y == other.direction.y
          && metric.direction.z == other.direction.z
          && metric.polarity == other.polarity)
        worst_translation = std::max(worst_translation, difference);
      if (metric.direction.x == other.direction.x
          && metric.direction.y == other.direction.y
          && metric.direction.z == other.direction.z
          && metric.translation_index == other.translation_index)
        worst_polarity = std::max(worst_polarity, difference);
      if (metric.polarity == other.polarity
          && metric.translation_index == other.translation_index)
        worst_cubic = std::max(worst_cubic, difference);
    }
  }
  check("one-sided scalar metrics are translation polarity and cubic covariant",
        worst_translation <= derivative_gate
        && worst_polarity <= derivative_gate
        && worst_cubic <= derivative_gate);
  check("normal variational class is finite and exhaustive",
        differentiable_arms+nonstationary_arms
            <= arms
        && interval_arms+nonstationary_arms == arms
        && std::isfinite(minimum_normal_residual)
        && std::isfinite(maximum_normal_residual));
  check("invalid non-edge input fails closed",
        !ftd::eft::solve_edge_plane_one_sided_variation(
            L, {}, {1, 1, 1}, +1, 0.25).valid);

  const bool energy_closed = maximum_ordinary <= algebra_gate
      && maximum_modified <= algebra_gate;
  const char* verdict = !orbit_ok || !algebra_closed
      ? "EDGE_PLANE_ONE_SIDED_VARIATION_UNRESOLVED"
      : (nonstationary_arms > 0
          ? "EDGE_PLANE_STATIONARITY_CLOSED_NEGATIVE"
          : (differentiable_arms < arms
              ? "EDGE_PLANE_NONSMOOTH_STATIONARY_REQUIRES_SUBGRADIENT_SELECTION"
              : (energy_closed
                  ? "EDGE_PLANE_DIFFERENTIABLE_STATIONARY_ENERGY_CONSTRUCTIVE"
                  : "EDGE_PLANE_DIFFERENTIABLE_STATIONARY_ENERGY_GATE_CLOSED_NEGATIVE")));

  std::cout.precision(17);
  std::cout << "arms=" << arms << '\n'
            << "differentiable_arms=" << differentiable_arms << '\n'
            << "interval_stationary_arms=" << interval_arms << '\n'
            << "nonstationary_arms=" << nonstationary_arms << '\n'
            << "worst_active_root_residual=" << worst_active_root << '\n'
            << "worst_active_derivative_convergence="
            << worst_active_convergence << '\n'
            << "worst_one_sided_derivative_convergence="
            << worst_one_sided_convergence << '\n'
            << "minimum_normal_residual=" << minimum_normal_residual << '\n'
            << "maximum_normal_residual=" << maximum_normal_residual << '\n'
            << "minimum_normal_residual_jump=" << minimum_normal_jump << '\n'
            << "maximum_normal_residual_jump=" << maximum_normal_jump << '\n'
            << "worst_current_split_residual=" << worst_split << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_field_equation_residual=" << worst_field << '\n'
            << "worst_gauss_evolution_residual=" << worst_gauss << '\n'
            << "worst_causal_excess=" << worst_causal << '\n'
            << "minimum_ordinary_total_energy_defect="
            << minimum_ordinary << '\n'
            << "maximum_ordinary_total_energy_defect="
            << maximum_ordinary << '\n'
            << "minimum_modified_total_energy_defect="
            << minimum_modified << '\n'
            << "maximum_modified_total_energy_defect="
            << maximum_modified << '\n'
            << "worst_translation_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_residual=" << worst_polarity << '\n'
            << "worst_signed_cubic_residual=" << worst_cubic << '\n'
            << "edge_plane_one_sided_variation failures=" << failures << '\n'
            << "verdict=" << verdict << '\n';
  return failures == 0 ? 0 : 1;
}


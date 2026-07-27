/** FTD-0538: chart-contained six-coordinate solve of the FTD-0536 action. */

#include "ftd/eft/implicit_atomic_endpoint_solve.h"

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

ftd::Vec3 rotate_from_canonical(ftd::Vec3 value,
                                ftd::Coord direction, int shell) {
  ftd::Vec3 result{};
  if (shell == 3) {
    return {direction.x*value.x,
            direction.y*value.y,
            direction.z*value.z};
  }
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

int canonical_index(int shell, int speed_index) {
  return (shell-2)*2+speed_index;
}

ftd::eft::AtomicFaceEndpointTrialResult evaluate(
    const ftd::eft::ImplicitAtomicInitialFixture& fixture,
    const std::array<ftd::Vec3, 2>& endpoint,
    bool chart_contained) {
  return ftd::eft::evaluate_atomic_face_endpoint_trial(
      fixture.start_position, endpoint, fixture.charge,
      fixture.prescribed_kinetic_start,
      fixture.potential_before, fixture.electric_before,
      fixture.beta, fixture.temporal_scale,
      ftd::E_REST, ftd::C_SPEED,
      derivative_step, algebra_gate, chart_contained);
}

}  // namespace

int main() {
  const std::array<double, 2> speeds{{0.125, 0.25}};
  std::array<ftd::eft::ImplicitAtomicEndpointSolveResult, 4> baseline{};
  std::array<ftd::eft::ImplicitAtomicEndpointSolveResult, 4> canonical{};
  bool canonical_classified = true;
  int canonical_roots = 0;
  int canonical_clearance_failures = 0;
  double worst_canonical_root = 0.0;
  double worst_canonical_derivative = 0.0;
  double maximum_endpoint_difference = 0.0;
  double minimum_pivot = INFINITY;
  double minimum_step_factor = 1.0;
  int maximum_iterations = 0;

  for (int shell : {2, 3}) {
    const ftd::Coord direction = shell == 2
        ? ftd::Coord{1, 1, 0} : ftd::Coord{1, 1, 1};
    const ftd::Vec3 contact{8.0+0.5*direction.x,
                            8.0+0.5*direction.y,
                            8.0+0.5*direction.z};
    for (int speed_index = 0; speed_index < 2; ++speed_index) {
      const std::size_t index = static_cast<std::size_t>(
          canonical_index(shell, speed_index));
      baseline[index] = ftd::eft::solve_implicit_atomic_endpoint(
          L, contact, direction, +1,
          speeds[static_cast<std::size_t>(speed_index)],
          derivative_step, root_gate, algebra_gate, false);
      canonical[index] = ftd::eft::solve_implicit_atomic_endpoint(
          L, contact, direction, +1,
          speeds[static_cast<std::size_t>(speed_index)],
          derivative_step, root_gate, algebra_gate, true);
      const auto& solve = canonical[index];
      const bool root_closed = solve.valid && solve.converged
          && solve.final_residual <= root_gate
          && solve.trial.endpoint_derivative_convergence <= derivative_gate;
      const bool clearance_failed = !solve.valid
          && solve.trial.minimum_endpoint_chart_clearance
              <= std::ldexp(1.0, -30);
      canonical_roots += root_closed ? 1 : 0;
      canonical_clearance_failures += clearance_failed ? 1 : 0;
      canonical_classified = canonical_classified && baseline[index].valid
          && (root_closed || clearance_failed);
      worst_canonical_root = std::max(
          worst_canonical_root, solve.final_residual);
      worst_canonical_derivative = std::max(
          worst_canonical_derivative,
          solve.trial.endpoint_derivative_convergence);
      if (solve.valid) {
        minimum_pivot = std::min(minimum_pivot, solve.minimum_pivot);
        minimum_step_factor = std::min(
            minimum_step_factor, solve.minimum_accepted_step_factor);
      }
      maximum_iterations = std::max(maximum_iterations, solve.iterations);
      if (solve.valid) {
        for (int carrier = 0; carrier < 2; ++carrier) {
          maximum_endpoint_difference = std::max(
              maximum_endpoint_difference,
              (solve.displacement[static_cast<std::size_t>(carrier)]
               -baseline[index].displacement[static_cast<std::size_t>(carrier)])
                  .mag());
        }
      }
      std::cout.precision(17);
      std::cout << "canonical_shell=" << shell
                << " speed=" << speeds[static_cast<std::size_t>(speed_index)]
                << " valid=" << solve.valid
                << " iterations=" << solve.iterations
                << " root=" << solve.final_residual
                << " derivative_convergence="
                << solve.trial.endpoint_derivative_convergence
                << " chart_clearance="
                << solve.trial.minimum_endpoint_chart_clearance
                << " selected_step="
                << solve.trial.minimum_endpoint_derivative_step
                << " endpoint_difference=" << maximum_endpoint_difference
                << " ordinary_energy_defect="
                << solve.trial.ordinary_total_energy_defect
                << " modified_energy_defect="
                << solve.trial.modified_total_energy_defect << '\n';
    }
  }
  check("all canonical arms are rooted or fail the locked clearance gate",
        canonical_classified && canonical_roots == 2
        && canonical_clearance_failures == 2);

  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  int orbit_arms = 0;
  int stationary_arms = 0;
  int clearance_failed_arms = 0;
  double worst_orbit_root = 0.0;
  double worst_derivative = 0.0;
  double minimum_clearance = INFINITY;
  double minimum_selected_step = INFINITY;
  double worst_split = 0.0;
  double worst_continuity = 0.0;
  double worst_field = 0.0;
  double worst_gauss = 0.0;
  double worst_causal = 0.0;
  double worst_action_difference = 0.0;
  double minimum_ordinary = INFINITY;
  double maximum_ordinary = 0.0;
  double minimum_modified = INFINITY;
  double maximum_modified = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;
  bool orbit_classified = canonical_classified;

  struct Metric {
    ftd::Coord direction{};
    int shell = 0;
    int speed_index = 0;
    int polarity = 0;
    int translation_index = 0;
    double root = 0.0;
    double ordinary = 0.0;
    double modified = 0.0;
  };
  std::vector<Metric> metrics;
  metrics.reserve(240);

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = dx*dx+dy*dy+dz*dz;
        if (shell != 2 && shell != 3) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (int speed_index = 0; speed_index < 2; ++speed_index) {
          const auto& source_solve = canonical[static_cast<std::size_t>(
              canonical_index(shell, speed_index))];
          const ftd::Coord canonical_direction = shell == 2
              ? ftd::Coord{1, 1, 0} : ftd::Coord{1, 1, 1};
          const ftd::Vec3 canonical_contact{
              8.0+0.5*canonical_direction.x,
              8.0+0.5*canonical_direction.y,
              8.0+0.5*canonical_direction.z};
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
              std::array<ftd::Vec3, 2> endpoint =
                  fixture.free_end_position;
              if (source_solve.valid) {
                bool matched[2] = {false, false};
                for (int source = 0; source < 2; ++source) {
                  const ftd::Vec3 relative = rotate_from_canonical(
                      source_solve.fixture.start_position[
                          static_cast<std::size_t>(source)]
                      -canonical_contact, direction, shell);
                  int target_match = -1;
                  for (int target = 0; target < 2; ++target) {
                    if (max_component(
                        fixture.start_position[static_cast<std::size_t>(target)]
                        -contact-relative) <= 1e-10) target_match = target;
                  }
                  if (target_match < 0 || matched[target_match]) {
                    orbit_classified = false;
                    continue;
                  }
                  matched[target_match] = true;
                  endpoint[static_cast<std::size_t>(target_match)] =
                      fixture.start_position[
                          static_cast<std::size_t>(target_match)]
                      +rotate_from_canonical(
                          source_solve.displacement[
                              static_cast<std::size_t>(source)],
                          direction, shell);
                }
              }
              const auto trial = evaluate(fixture, endpoint, true);
              const auto ordinary_trial = evaluate(fixture, endpoint, false);
              const bool stationary = trial.valid
                  && trial.residual_infinity_norm <= root_gate
                  && trial.endpoint_derivative_convergence <= derivative_gate;
              const bool clearance_failed = !trial.valid
                  && trial.minimum_endpoint_chart_clearance
                      <= std::ldexp(1.0, -30);
              stationary_arms += stationary ? 1 : 0;
              clearance_failed_arms += clearance_failed ? 1 : 0;
              orbit_classified = orbit_classified
                  && fixture.valid && ordinary_trial.valid
                  && (stationary || clearance_failed);
              ++orbit_arms;
              worst_orbit_root = std::max(
                  worst_orbit_root, trial.residual_infinity_norm);
              worst_derivative = std::max(
                  worst_derivative, trial.endpoint_derivative_convergence);
              minimum_clearance = std::min(
                  minimum_clearance,
                  trial.minimum_endpoint_chart_clearance);
              if (trial.minimum_endpoint_derivative_step > 0.0) {
                minimum_selected_step = std::min(
                    minimum_selected_step,
                    trial.minimum_endpoint_derivative_step);
              }
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
              if (trial.valid) {
                worst_action_difference = std::max({
                    worst_action_difference,
                    std::abs(trial.matter_action
                             -ordinary_trial.matter_action),
                    std::abs(trial.field_action
                             -ordinary_trial.field_action),
                    std::abs(trial.interaction_action
                             -ordinary_trial.interaction_action),
                    std::abs(trial.total_action
                             -ordinary_trial.total_action)});
                const double ordinary = std::abs(
                    trial.ordinary_total_energy_defect);
                const double modified = std::abs(
                    trial.modified_total_energy_defect);
                minimum_ordinary = std::min(minimum_ordinary, ordinary);
                maximum_ordinary = std::max(maximum_ordinary, ordinary);
                minimum_modified = std::min(minimum_modified, modified);
                maximum_modified = std::max(maximum_modified, modified);
                metrics.push_back({direction, shell, speed_index, polarity,
                    translation_index, trial.residual_infinity_norm,
                    ordinary, modified});
              }
            }
          }
        }
      }
    }
  }

  check("all 240 arms are rooted or fail the locked clearance gate",
        orbit_classified && orbit_arms == 240 && stationary_arms == 96
        && clearance_failed_arms == 144);
  const bool algebraic_orbit_closed =
      worst_split <= algebra_gate
      && worst_continuity <= algebra_gate
      && worst_field <= algebra_gate
      && worst_gauss <= algebra_gate
      && worst_causal <= algebra_gate;
  check("all current field Gauss causal and derivative gates close",
        algebraic_orbit_closed && worst_derivative <= derivative_gate);
  check("chart containment changes derivatives but not action values",
        worst_action_difference <= algebra_gate);

  for (const auto& metric : metrics) {
    for (const auto& other : metrics) {
      if (metric.shell != other.shell
          || metric.speed_index != other.speed_index) continue;
      const double difference = std::max({
          std::abs(metric.root-other.root),
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
  check("root and energy metrics remain covariant",
        worst_translation <= derivative_gate
        && worst_polarity <= derivative_gate
        && worst_cubic <= derivative_gate);

  const auto invalid_fixture = ftd::eft::make_implicit_atomic_initial_fixture(
      L, {8.5, 8.5, 8.0}, {1, 1, 0}, +1, 0.25, algebra_gate);
  auto invalid_endpoint = invalid_fixture.free_end_position;
  invalid_endpoint[0].x = std::round(invalid_endpoint[0].x);
  check("invalid and zero-clearance inputs fail closed",
        invalid_fixture.valid
        && !evaluate(invalid_fixture, invalid_endpoint, true).valid
        && !ftd::eft::solve_implicit_atomic_endpoint(
            2, {}, {1, 1, 0}, +1, 0.25,
            derivative_step, root_gate, algebra_gate, true).valid);

  const bool diagnostics_finite = std::isfinite(minimum_clearance)
      && std::isfinite(minimum_selected_step)
      && std::isfinite(maximum_ordinary)
      && std::isfinite(maximum_modified);
  check("registered energy branch is finite and classified",
        diagnostics_finite);
  const bool energy_closed = maximum_ordinary <= algebra_gate
      && maximum_modified <= algebra_gate;
  const char* verdict = !algebraic_orbit_closed || !diagnostics_finite
      || worst_derivative > derivative_gate || clearance_failed_arms > 0
      ? "CHART_CONTAINED_ENDPOINT_SOLVE_UNRESOLVED"
      : (stationary_arms != 240
          ? "CHART_CONTAINED_STATIONARY_ROOT_NOT_CONSTRUCTED"
          : (energy_closed
              ? "CHART_CONTAINED_STATIONARY_ROOT_ENERGY_CONSTRUCTIVE"
              : "CHART_CONTAINED_STATIONARY_ROOT_ENERGY_GATE_CLOSED_NEGATIVE"));

  std::cout.precision(17);
  std::cout << "canonical_roots=" << canonical_roots << '\n'
            << "canonical_clearance_failures="
            << canonical_clearance_failures << '\n'
            << "orbit_arms=" << orbit_arms << '\n'
            << "stationary_arms=" << stationary_arms << '\n'
            << "clearance_failed_arms=" << clearance_failed_arms << '\n'
            << "maximum_newton_iterations=" << maximum_iterations << '\n'
            << "minimum_jacobian_pivot=" << minimum_pivot << '\n'
            << "minimum_accepted_step_factor=" << minimum_step_factor << '\n'
            << "worst_canonical_root_residual="
            << worst_canonical_root << '\n'
            << "worst_orbit_root_residual=" << worst_orbit_root << '\n'
            << "worst_canonical_derivative_convergence="
            << worst_canonical_derivative << '\n'
            << "worst_endpoint_derivative_convergence="
            << worst_derivative << '\n'
            << "minimum_endpoint_chart_clearance="
            << minimum_clearance << '\n'
            << "minimum_selected_derivative_step="
            << minimum_selected_step << '\n'
            << "maximum_endpoint_difference_from_ftd_0537="
            << maximum_endpoint_difference << '\n'
            << "worst_current_split_residual=" << worst_split << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_field_equation_residual=" << worst_field << '\n'
            << "worst_gauss_evolution_residual=" << worst_gauss << '\n'
            << "worst_causal_excess=" << worst_causal << '\n'
            << "worst_action_value_difference="
            << worst_action_difference << '\n'
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
            << "chart_contained_atomic_endpoint_solve failures="
            << failures << '\n'
            << "verdict=" << verdict << '\n';
  return failures == 0 ? 0 : 1;
}

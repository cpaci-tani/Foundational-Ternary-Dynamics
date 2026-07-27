/** FTD-0537: six-coordinate stationary solve of the FTD-0536 action. */

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
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

ftd::Vec3 rotate_from_canonical(ftd::Vec3 value,
                                ftd::Coord direction, int shell) {
  ftd::Vec3 result{};
  if (shell == 3) {
    result = {direction.x*value.x,
              direction.y*value.y,
              direction.z*value.z};
    return result;
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

double nearest_integer_distance(ftd::Vec3 value) {
  return std::min({std::abs(value.x-std::round(value.x)),
                   std::abs(value.y-std::round(value.y)),
                   std::abs(value.z-std::round(value.z))});
}

int canonical_index(int shell, int speed_index) {
  return (shell-2)*2+speed_index;
}

}  // namespace

int main() {
  const std::array<double, 2> speeds{{0.125, 0.25}};
  std::array<ftd::eft::ImplicitAtomicEndpointSolveResult, 4> canonical{};
  bool canonical_ok = true;
  double worst_canonical_root = 0.0;
  double minimum_pivot = INFINITY;
  double minimum_step_factor = 1.0;
  int maximum_iterations = 0;
  double maximum_endpoint_change = 0.0;
  for (int shell : {2, 3}) {
    const ftd::Coord direction = shell == 2
        ? ftd::Coord{1, 1, 0} : ftd::Coord{1, 1, 1};
    const ftd::Vec3 contact{8.0+0.5*direction.x,
                            8.0+0.5*direction.y,
                            8.0+0.5*direction.z};
    for (int speed_index = 0; speed_index < 2; ++speed_index) {
      auto& solve = canonical[static_cast<std::size_t>(
          canonical_index(shell, speed_index))];
      solve = ftd::eft::solve_implicit_atomic_endpoint(
          L, contact, direction, +1,
          speeds[static_cast<std::size_t>(speed_index)],
          0.000244140625, root_gate, algebra_gate);
      canonical_ok = canonical_ok && solve.valid && solve.converged
          && solve.final_residual <= root_gate;
      worst_canonical_root = std::max(
          worst_canonical_root, solve.final_residual);
      minimum_pivot = std::min(minimum_pivot, solve.minimum_pivot);
      minimum_step_factor = std::min(
          minimum_step_factor, solve.minimum_accepted_step_factor);
      maximum_iterations = std::max(maximum_iterations, solve.iterations);
      maximum_endpoint_change = std::max(
          maximum_endpoint_change, solve.maximum_endpoint_change);
      std::cout.precision(17);
      std::cout << "canonical_shell=" << shell
                << " speed=" << speeds[static_cast<std::size_t>(speed_index)]
                << " valid=" << solve.valid
                << " converged=" << solve.converged
                << " iterations=" << solve.iterations
                << " initial_residual=" << solve.initial_residual
                << " final_residual=" << solve.final_residual
                << " derivative_convergence="
                << solve.trial.endpoint_derivative_convergence
                << " ordinary_energy_defect="
                << solve.trial.ordinary_total_energy_defect
                << " modified_energy_defect="
                << solve.trial.modified_total_energy_defect
                << " minimum_endpoint_integer_distance="
                << std::min(nearest_integer_distance(
                                solve.trial.end_position[0]),
                            nearest_integer_distance(
                                solve.trial.end_position[1]))
                << " minimum_pivot=" << solve.minimum_pivot
                << " minimum_step_factor="
                << solve.minimum_accepted_step_factor << '\n';
    }
  }
  check("all four canonical six-coordinate roots converge", canonical_ok);

  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  int orbit_arms = 0;
  int stationary_arms = 0;
  bool orbit_ok = canonical_ok;
  double worst_orbit_root = 0.0;
  double worst_derivative_convergence = 0.0;
  double worst_split = 0.0;
  double worst_continuity = 0.0;
  double worst_field = 0.0;
  double worst_gauss = 0.0;
  double worst_causal = 0.0;
  double minimum_ordinary_energy_defect = INFINITY;
  double maximum_ordinary_energy_defect = 0.0;
  double minimum_modified_energy_defect = INFINITY;
  double maximum_modified_energy_defect = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;
  double minimum_endpoint_integer_distance = INFINITY;
  int chart_straddling_arms = 0;

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
              std::array<ftd::Vec3, 2> endpoint{};
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
                  orbit_ok = false;
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
              const auto trial = ftd::eft::evaluate_atomic_face_endpoint_trial(
                  fixture.start_position, endpoint, fixture.charge,
                  fixture.prescribed_kinetic_start,
                  fixture.potential_before, fixture.electric_before,
                  fixture.beta, fixture.temporal_scale,
                  ftd::E_REST, ftd::C_SPEED,
                  0.000244140625, algebra_gate);
              const bool stationary = trial.valid
                  && trial.residual_infinity_norm <= root_gate;
              stationary_arms += stationary ? 1 : 0;
              orbit_ok = orbit_ok && fixture.valid && stationary;
              ++orbit_arms;
              worst_orbit_root = std::max(
                  worst_orbit_root, trial.residual_infinity_norm);
              worst_derivative_convergence = std::max(
                  worst_derivative_convergence,
                  trial.endpoint_derivative_convergence);
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
              const double endpoint_integer_distance = std::min(
                  nearest_integer_distance(trial.end_position[0]),
                  nearest_integer_distance(trial.end_position[1]));
              minimum_endpoint_integer_distance = std::min(
                  minimum_endpoint_integer_distance,
                  endpoint_integer_distance);
              chart_straddling_arms += endpoint_integer_distance
                      < 0.000244140625
                  ? 1 : 0;
              const double ordinary = std::abs(
                  trial.ordinary_total_energy_defect);
              const double modified = std::abs(
                  trial.modified_total_energy_defect);
              minimum_ordinary_energy_defect = std::min(
                  minimum_ordinary_energy_defect, ordinary);
              maximum_ordinary_energy_defect = std::max(
                  maximum_ordinary_energy_defect, ordinary);
              minimum_modified_energy_defect = std::min(
                  minimum_modified_energy_defect, modified);
              maximum_modified_energy_defect = std::max(
                  maximum_modified_energy_defect, modified);
              metrics.push_back({direction, shell, speed_index, polarity,
                  translation_index, trial.residual_infinity_norm,
                  ordinary, modified});
            }
          }
        }
      }
    }
  }
  check("canonical roots transport to all 240 covariance arms",
        orbit_ok && orbit_arms == 240 && stationary_arms == 240);
  const bool algebraic_orbit_closed =
        worst_split <= algebra_gate
        && worst_continuity <= algebra_gate
        && worst_field <= algebra_gate
        && worst_gauss <= algebra_gate
        && worst_causal <= algebra_gate;
  check("all transported current field Gauss and causal gates close",
        algebraic_orbit_closed);
  const bool derivative_diagnostic_finite =
      std::isfinite(worst_derivative_convergence)
      && std::isfinite(minimum_endpoint_integer_distance);
  check("endpoint derivative diagnostic is finite and classified",
        derivative_diagnostic_finite);

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
  check("root and energy metrics are translation polarity and cubic covariant",
        worst_translation <= derivative_gate
        && worst_polarity <= derivative_gate
        && worst_cubic <= derivative_gate);
  check("invalid endpoint-solve inputs fail closed",
        !ftd::eft::solve_implicit_atomic_endpoint(
            2, {}, {1, 1, 0}, +1, 0.25).valid
        && !ftd::eft::solve_implicit_atomic_endpoint(
            L, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::solve_implicit_atomic_endpoint(
            L, {}, {1, 1, 0}, 0, 0.25).valid
        && !ftd::eft::solve_implicit_atomic_endpoint(
            L, {}, {1, 1, 0}, +1, 0.25, -1.0).valid);

  const bool energy_closed = maximum_ordinary_energy_defect <= algebra_gate
      && maximum_modified_energy_defect <= algebra_gate;
  const bool derivative_closed =
      worst_derivative_convergence <= derivative_gate;
  const char* verdict = !algebraic_orbit_closed
          || !derivative_diagnostic_finite
      ? "IMPLICIT_ATOMIC_ENDPOINT_SOLVE_UNRESOLVED"
      : (!derivative_closed
          ? "IMPLICIT_ATOMIC_ENDPOINT_SOLVE_UNRESOLVED"
          : (stationary_arms != 240
              ? "IMPLICIT_ATOMIC_STATIONARY_ROOT_NOT_CONSTRUCTED"
              : (energy_closed
                  ? "IMPLICIT_ATOMIC_STATIONARY_ROOT_ENERGY_CONSTRUCTIVE"
                  : "IMPLICIT_ATOMIC_STATIONARY_ROOT_ENERGY_GATE_CLOSED_NEGATIVE")));
  std::cout.precision(17);
  std::cout << "canonical_roots=" << canonical.size() << '\n'
            << "orbit_arms=" << orbit_arms << '\n'
            << "stationary_arms=" << stationary_arms << '\n'
            << "maximum_newton_iterations=" << maximum_iterations << '\n'
            << "minimum_jacobian_pivot=" << minimum_pivot << '\n'
            << "minimum_accepted_step_factor=" << minimum_step_factor << '\n'
            << "maximum_endpoint_change_from_free="
            << maximum_endpoint_change << '\n'
            << "worst_canonical_root_residual="
            << worst_canonical_root << '\n'
            << "worst_orbit_root_residual=" << worst_orbit_root << '\n'
            << "worst_endpoint_derivative_convergence="
            << worst_derivative_convergence << '\n'
            << "derivative_gate_closed=" << derivative_closed << '\n'
            << "minimum_endpoint_integer_distance="
            << minimum_endpoint_integer_distance << '\n'
            << "chart_straddling_arms=" << chart_straddling_arms << '\n'
            << "worst_current_split_residual=" << worst_split << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_field_equation_residual=" << worst_field << '\n'
            << "worst_gauss_evolution_residual=" << worst_gauss << '\n'
            << "worst_causal_excess=" << worst_causal << '\n'
            << "minimum_ordinary_total_energy_defect="
            << minimum_ordinary_energy_defect << '\n'
            << "maximum_ordinary_total_energy_defect="
            << maximum_ordinary_energy_defect << '\n'
            << "minimum_modified_total_energy_defect="
            << minimum_modified_energy_defect << '\n'
            << "maximum_modified_total_energy_defect="
            << maximum_modified_energy_defect << '\n'
            << "worst_translation_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_residual=" << worst_polarity << '\n'
            << "worst_signed_cubic_residual=" << worst_cubic << '\n'
            << "implicit_atomic_endpoint_solve failures=" << failures << '\n'
            << "verdict=" << verdict << '\n';
  return failures == 0 ? 0 : 1;
}

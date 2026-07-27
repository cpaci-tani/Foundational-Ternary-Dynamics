/** FTD-0495: reversible scalar dressing ledger and action obstruction. */

#include "ftd/eft/dressing_fiber_ledger.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 8;
constexpr ftd::Coord site{2, 2, 3};
constexpr double jump_amplitude = 0.3;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_energy_residual = 0.0;
double worst_reverse_residual = 0.0;
double worst_symmetry_residual = 0.0;
double path_holonomy = 0.0;
double loop_shift = 0.0;
double multiplier_stationarity_residual = 0.0;
double action_gradient_residual = 0.0;
double constraint_finite_difference_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double checker(int x, int y, int shift_x = 0, int shift_y = 0) {
  return ((x - shift_x + y - shift_y) & 1) == 0
      ? jump_amplitude : -jump_amplitude;
}

ftd::eft::MatchedFaceFlux checker_field(
    int shift_x = 0, int shift_y = 0, double sign = 1.0) {
  ftd::eft::MatchedFaceFlux field(L);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = field.index(x, y, z);
        const double h = sign * checker(x, y, shift_x, shift_y);
        field.x[static_cast<std::size_t>(i)] = 0.5 * h;
        field.y[static_cast<std::size_t>(i)] = -0.5 * h;
      }
    }
  }
  return field;
}

double constraint_action(
    const ftd::eft::CenteredTraceWorkResult& work,
    const ftd::Vec3& displacement,
    double dressing_before,
    double dressing_after,
    double multiplier) {
  const double cusp = ftd::eft::local_cusp_dressing_energy(
      work.jump, displacement, work.charge, work.coupling);
  return multiplier
      * (cusp - (dressing_after - dressing_before));
}

}  // namespace

int main() {
  const ftd::Vec3 displacement{0.23, -0.31, 0.17};
  const auto field = checker_field();
  const auto work = ftd::eft::evaluate_centered_trace_work(
      field, site, displacement, +1, coupling);
  const auto step = ftd::eft::advance_dressing_fiber(1.25, work, 1.0);
  worst_energy_residual = step.extended_energy_residual;
  worst_reverse_residual = step.reverse_dressing_residual;
  action_gradient_residual = step.action_gradient_residual;

  check("one scalar closes centered matter plus exact field energy",
        step.valid && step.extended_energy_residual <= gate);
  check("single-step dressing update reverses exactly",
        step.reverse_dressing_residual <= gate);
  check("ordinary unit-multiplier action restores cusp gradient",
        step.action_gradient_residual <= gate
        && std::abs(step.constraint_derivative_D0 - 1.0) <= gate
        && std::abs(step.constraint_derivative_D1 + 1.0) <= gate);
  constexpr double epsilon = 0.125;
  ftd::Vec3 finite_gradient{};
  for (int axis = 0; axis < 3; ++axis) {
    ftd::Vec3 plus = displacement;
    ftd::Vec3 minus = displacement;
    if (axis == 0) { plus.x += epsilon; minus.x -= epsilon; }
    if (axis == 1) { plus.y += epsilon; minus.y -= epsilon; }
    if (axis == 2) { plus.z += epsilon; minus.z -= epsilon; }
    const double derivative = (
        constraint_action(work, plus, step.dressing_before,
                          step.dressing_after, 1.0)
        - constraint_action(work, minus, step.dressing_before,
                            step.dressing_after, 1.0))
        / (2.0 * epsilon);
    if (axis == 0) finite_gradient.x = derivative;
    if (axis == 1) finite_gradient.y = derivative;
    if (axis == 2) finite_gradient.z = derivative;
  }
  const double derivative_D0 = (
      constraint_action(work, displacement,
                        step.dressing_before + epsilon,
                        step.dressing_after, 1.0)
      - constraint_action(work, displacement,
                          step.dressing_before - epsilon,
                          step.dressing_after, 1.0))
      / (2.0 * epsilon);
  const double derivative_D1 = (
      constraint_action(work, displacement, step.dressing_before,
                        step.dressing_after + epsilon, 1.0)
      - constraint_action(work, displacement, step.dressing_before,
                          step.dressing_after - epsilon, 1.0))
      / (2.0 * epsilon);
  constraint_finite_difference_residual = std::max({
      std::abs(finite_gradient.x
               - step.constraint_position_gradient.x),
      std::abs(finite_gradient.y
               - step.constraint_position_gradient.y),
      std::abs(finite_gradient.z
               - step.constraint_position_gradient.z),
      std::abs(derivative_D0 - step.constraint_derivative_D0),
      std::abs(derivative_D1 - step.constraint_derivative_D1)});
  check("finite difference confirms every constraint derivative",
        constraint_finite_difference_residual <= gate);

  multiplier_stationarity_residual = std::abs(
      ftd::eft::dressing_multiplier_stationarity(0.83, 0.83));
  check("interior dressing variation conserves its multiplier",
        multiplier_stationarity_residual <= gate
        && std::abs(ftd::eft::dressing_multiplier_stationarity(
                        0.83, -0.27)) > 1e-6);

  const auto integrability =
      ftd::eft::evaluate_cusp_dressing_integrability(
          field, site, displacement, +1, coupling);
  const auto paths = ftd::eft::compare_dressing_fiber_paths(
      1.25, integrability, 9.75);
  path_holonomy = paths.path_difference;
  loop_shift = paths.closed_loop_shift;
  check("two paths to one endpoint retain the nonzero fiber holonomy",
        paths.valid && std::abs(path_holonomy) > 1e-6
        && paths.holonomy_residual <= gate
        && std::abs(path_holonomy - 0.438) <= gate);
  check("oriented closed loop changes only the dressing fiber",
        std::abs(loop_shift - path_holonomy) <= gate);
  check("exactly reversed closed loop recovers the fiber",
        paths.reversed_loop_residual <= gate);
  check("arbitrary dressing-energy zero changes no holonomy",
        paths.shifted_zero_holonomy_residual <= gate);

  const auto mirrored_work = ftd::eft::evaluate_centered_trace_work(
      checker_field(0, 0, -1.0), site, displacement * -1.0,
      -1, coupling);
  const auto mirrored_step = ftd::eft::advance_dressing_fiber(
      1.25, mirrored_work, 1.0);
  worst_symmetry_residual = std::max({
      std::abs(mirrored_step.dressing_change - step.dressing_change),
      std::abs(mirrored_step.extended_energy_residual
               - step.extended_energy_residual),
      std::abs(mirrored_step.action_gradient_residual
               - step.action_gradient_residual)});
  check("polarity/field mirror preserves scalar-ledger closure",
        mirrored_step.valid && worst_symmetry_residual <= gate);

  const ftd::Coord shifted_site{site.x + 1, site.y + 1, site.z};
  const auto shifted_work = ftd::eft::evaluate_centered_trace_work(
      checker_field(1, 1), shifted_site, displacement, +1, coupling);
  const auto shifted_step = ftd::eft::advance_dressing_fiber(
      1.25, shifted_work, 1.0);
  const double translation_residual = std::max({
      std::abs(shifted_step.dressing_change - step.dressing_change),
      std::abs(shifted_step.extended_energy_residual
               - step.extended_energy_residual)});
  worst_symmetry_residual = std::max(worst_symmetry_residual,
                                     translation_residual);
  check("translated field/history preserves the ledger",
        shifted_step.valid && translation_residual <= gate);

  check("invalid work record fails closed",
        !ftd::eft::advance_dressing_fiber(
            0.0, ftd::eft::CenteredTraceWorkResult{}, 1.0).valid);

  std::cout.precision(17);
  std::cout << "field_work=" << work.field_work << '\n'
            << "centered_work=" << work.centered_work << '\n'
            << "dressing_change=" << step.dressing_change << '\n'
            << "path_holonomy=" << path_holonomy << '\n'
            << "closed_loop_shift=" << loop_shift << '\n'
            << "worst_energy_residual=" << worst_energy_residual << '\n'
            << "worst_reverse_residual=" << worst_reverse_residual << '\n'
            << "multiplier_stationarity_residual="
            << multiplier_stationarity_residual << '\n'
            << "action_gradient_residual="
            << action_gradient_residual << '\n'
            << "constraint_finite_difference_residual="
            << constraint_finite_difference_residual << '\n'
            << "worst_symmetry_residual="
            << worst_symmetry_residual << '\n'
            << "dressing_fiber_ledger failures=" << failures << '\n'
            << "verdict=SCALAR_LEDGER_CLOSES_BOOKKEEPING_NOT_COMMON_ACTION\n";
  return failures == 0 ? 0 : 1;
}

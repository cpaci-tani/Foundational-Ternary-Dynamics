#include "ftd/eft/fixed_step_energy_scope.h"

#include <cmath>

namespace ftd::eft {
namespace {

double potential(double q) {
  const double q2 = q * q;
  return 0.25 * q2 * q2;
}

double energy(double q, double p) {
  return 0.5 * p * p + potential(q);
}

double quartic_divided_difference(double q0, double q1) {
  return 0.25 * (q1 * q1 * q1 + q1 * q1 * q0
      + q1 * q0 * q0 + q0 * q0 * q0);
}

}  // namespace

FixedStepEnergyScopeResult evaluate_fixed_step_energy_scope(
    double q0, double q1, double step) {
  FixedStepEnergyScopeResult result;
  result.q0 = q0;
  result.q1 = q1;
  result.step = step;
  if (!std::isfinite(q0) || !std::isfinite(q1)
      || !(step > 0.0) || !std::isfinite(step)) return result;

  const double delta = q1 - q0;
  const double velocity = delta / step;
  const double midpoint = 0.5 * (q0 + q1);
  const double midpoint_force = midpoint * midpoint * midpoint;
  result.midpoint_p0 = velocity + 0.5 * step * midpoint_force;
  result.midpoint_p1 = velocity - 0.5 * step * midpoint_force;
  result.midpoint_energy0 = energy(q0, result.midpoint_p0);
  result.midpoint_energy1 = energy(q1, result.midpoint_p1);
  result.midpoint_energy_defect = result.midpoint_energy1
      - result.midpoint_energy0;
  result.analytic_energy_defect = (q0 + q1) * delta * delta * delta / 8.0;
  result.midpoint_identity_residual = std::abs(
      result.midpoint_energy_defect - result.analytic_energy_defect);
  result.discrete_lagrangian_energy = 0.5 * velocity * velocity
      + 0.25 * midpoint * midpoint * midpoint * midpoint;

  result.discrete_gradient = quartic_divided_difference(q0, q1);
  result.gradient_p0 = velocity + 0.5 * step * result.discrete_gradient;
  result.gradient_p1 = velocity - 0.5 * step * result.discrete_gradient;
  result.gradient_energy0 = energy(q0, result.gradient_p0);
  result.gradient_energy1 = energy(q1, result.gradient_p1);
  result.gradient_energy_defect = result.gradient_energy1
      - result.gradient_energy0;
  const double gradient_q0 = 0.25 * (
      q1 * q1 + 2.0 * q1 * q0 + 3.0 * q0 * q0);
  const double gradient_q1 = 0.25 * (
      3.0 * q1 * q1 + 2.0 * q1 * q0 + q0 * q0);
  const double numerator = 1.0 + 0.5 * step * step * gradient_q0;
  const double denominator = 1.0 + 0.5 * step * step * gradient_q1;
  if (denominator == 0.0) return result;
  result.gradient_area_determinant = numerator / denominator;
  result.gradient_area_defect = result.gradient_area_determinant - 1.0;
  result.valid = std::isfinite(result.midpoint_p0)
      && std::isfinite(result.midpoint_p1)
      && std::isfinite(result.midpoint_energy_defect)
      && std::isfinite(result.analytic_energy_defect)
      && std::isfinite(result.discrete_lagrangian_energy)
      && std::isfinite(result.discrete_gradient)
      && std::isfinite(result.gradient_energy_defect)
      && std::isfinite(result.gradient_area_determinant);
  return result;
}

}  // namespace ftd::eft

/** FTD-0547: exact uniform-force accelerated-worldline energy escape. */

#include "ftd/eft/accelerated_worldline_energy.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double gate = 1e-14;
constexpr double rest_energy = 0.511;
constexpr double h = ftd::C_SPEED;

int failures = 0;
int registered_arms = 0;
double worst_exact_work = 0.0;
double worst_defect_identity = 0.0;
double worst_endpoint = 0.0;
double worst_derivative = 0.0;
double worst_causal = 0.0;
double worst_reversal = 0.0;
double worst_cubic = 0.0;
double largest_midpoint_defect = 0.0;
double largest_schedule_deviation = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double scalar_residual(
    const ftd::eft::AcceleratedWorldlineEnergyResult& lhs,
    const ftd::eft::AcceleratedWorldlineEnergyResult& rhs) {
  return std::max({
      std::abs(lhs.midpoint_velocity - rhs.midpoint_velocity),
      std::abs(lhs.secant_velocity - rhs.secant_velocity),
      std::abs(lhs.midpoint_displacement - rhs.midpoint_displacement),
      std::abs(lhs.exact_displacement - rhs.exact_displacement),
      std::abs(lhs.midpoint_work_defect - rhs.midpoint_work_defect),
      std::abs(lhs.midpoint_schedule_deviation
               - rhs.midpoint_schedule_deviation)});
}

}  // namespace

int main() {
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  check("native normalization valid", normalization.valid);
  const std::array<double, 2> betas{{
      1.0, normalization.mapped_field_work_coefficient}};
  const std::array<double, 3> momenta{{0.1, 0.2, 0.3}};
  const std::array<double, 3> fields{{0.04, 0.08, 0.12}};
  const std::array<ftd::Vec3, 4> directions{{
      {1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {0.0, 0.0, 1.0},
      {1.0, 1.0, 1.0}}};

  for (double beta : betas) {
    for (double momentum : momenta) {
      for (double field : fields) {
        for (int charge : {-1, +1}) {
          const double half_impulse = 0.5 * beta * charge * field;
          const auto reference =
              ftd::eft::evaluate_accelerated_worldline_energy(
                  rest_energy, ftd::C_SPEED, h,
                  momentum, half_impulse, directions[0]);
          for (const auto& direction : directions) {
            ++registered_arms;
            const auto result =
                ftd::eft::evaluate_accelerated_worldline_energy(
                    rest_energy, ftd::C_SPEED, h,
                    momentum, half_impulse, direction);
            worst_exact_work = std::max(
                worst_exact_work, std::abs(result.exact_work_defect));
            worst_defect_identity = std::max(
                worst_defect_identity, result.defect_identity_residual);
            worst_endpoint = std::max(
                worst_endpoint, result.endpoint_residual);
            worst_derivative = std::max(
                worst_derivative, result.trajectory_derivative_residual);
            worst_causal = std::max(
                worst_causal, result.causal_speed_excess);
            worst_reversal = std::max({worst_reversal,
                result.reversal_velocity_residual,
                result.reversal_trajectory_residual});
            worst_cubic = std::max(
                worst_cubic, scalar_residual(reference, result));
            largest_midpoint_defect = std::max(
                largest_midpoint_defect,
                std::abs(result.midpoint_work_defect));
            largest_schedule_deviation = std::max(
                largest_schedule_deviation,
                result.midpoint_schedule_deviation);
            check("registered arm " + std::to_string(registered_arms),
                result.valid
                && std::abs(result.exact_work_defect) <= gate
                && result.defect_identity_residual <= gate
                && result.endpoint_residual <= gate
                && result.trajectory_derivative_residual <= gate
                && result.causal_speed_excess <= gate
                && result.reversal_velocity_residual <= gate
                && result.reversal_trajectory_residual <= gate
                && scalar_residual(reference, result) <= gate);
          }
        }
      }
    }
  }
  check("registered arm count", registered_arms == 144);
  check("straight midpoint defect reproduced",
      largest_midpoint_defect > 1e-8);
  check("accelerated temporal schedule nonuniform",
      largest_schedule_deviation > 1e-8);

  const auto asymptotic = ftd::eft::evaluate_accelerated_worldline_energy(
      rest_energy, ftd::C_SPEED, h, 0.2, 1e-3, {1.0, 0.0, 0.0});
  const double leading_relative = std::abs(
      asymptotic.midpoint_work_defect - asymptotic.leading_cubic_term)
      / std::abs(asymptotic.leading_cubic_term);
  check("small-impulse cubic term", asymptotic.valid
      && leading_relative <= 1e-4);

  const auto zero = ftd::eft::evaluate_accelerated_worldline_energy(
      rest_energy, ftd::C_SPEED, h, 0.2, 0.0, {1.0, 0.0, 0.0});
  check("zero-force limit", zero.valid
      && std::abs(zero.secant_velocity-zero.midpoint_velocity) <= gate
      && std::abs(zero.exact_displacement-zero.midpoint_displacement) <= gate
      && std::abs(zero.exact_work_defect) <= gate
      && zero.midpoint_schedule_deviation <= gate);
  check("zero mass fails closed",
      !ftd::eft::evaluate_accelerated_worldline_energy(
          0.0, ftd::C_SPEED, h, 0.2, 0.01,
          {1.0, 0.0, 0.0}).valid);
  check("zero direction fails closed",
      !ftd::eft::evaluate_accelerated_worldline_energy(
          rest_energy, ftd::C_SPEED, h, 0.2, 0.01,
          {0.0, 0.0, 0.0}).valid);
  check("nonfinite input fails closed",
      !ftd::eft::evaluate_accelerated_worldline_energy(
          rest_energy, ftd::C_SPEED, h,
          std::numeric_limits<double>::quiet_NaN(), 0.01,
          {1.0, 0.0, 0.0}).valid);

  std::cout.precision(17);
  std::cout << "registered_arms=" << registered_arms << '\n'
            << "worst_exact_work_residual=" << worst_exact_work << '\n'
            << "worst_defect_identity_residual="
            << worst_defect_identity << '\n'
            << "worst_endpoint_residual=" << worst_endpoint << '\n'
            << "worst_derivative_residual=" << worst_derivative << '\n'
            << "worst_causal_excess=" << worst_causal << '\n'
            << "worst_reversal_residual=" << worst_reversal << '\n'
            << "worst_cubic_residual=" << worst_cubic << '\n'
            << "largest_midpoint_defect=" << largest_midpoint_defect << '\n'
            << "largest_schedule_deviation="
            << largest_schedule_deviation << '\n'
            << "leading_relative_residual=" << leading_relative << '\n'
            << "accelerated_worldline_energy failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

/** FTD-0549: endpoints do not determine the spacetime current split. */

#include "ftd/eft/endpoint_schedule_underdetermination.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double gate = 1e-14;
int failures = 0;
int registered_arms = 0;
double worst_endpoint = 0.0;
double worst_derivative = 0.0;
double worst_total_current = 0.0;
double worst_recombination = 0.0;
double worst_analytic = 0.0;
double worst_reversal = 0.0;
double smallest_monotonicity_margin = 1.0;
double largest_split_difference = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double norm(const ftd::Vec3& value) {
  return std::sqrt(value.mag2());
}

}  // namespace

int main() {
  const std::array<double,3> displacements{{0.05,0.10,0.20}};
  const std::array<double,4> epsilons{{-0.5,-0.25,0.25,0.5}};
  const std::array<ftd::Vec3,4> directions{{
      {1.0,0.0,0.0},
      {0.0,1.0,0.0},
      {0.0,0.0,1.0},
      {1.0,1.0,1.0}}};
  for (double displacement : displacements) {
    for (double epsilon : epsilons) {
      for (int charge : {-1,+1}) {
        const auto reference =
            ftd::eft::evaluate_endpoint_schedule_underdetermination(
                displacement,epsilon,directions[0],charge);
        for (const auto& direction : directions) {
          ++registered_arms;
          const auto result =
              ftd::eft::evaluate_endpoint_schedule_underdetermination(
                  displacement,epsilon,direction,charge);
          worst_endpoint = std::max(
              worst_endpoint,result.endpoint_position_residual);
          worst_derivative = std::max({worst_derivative,
              result.endpoint_derivative_residual,
              result.midpoint_derivative_residual});
          worst_total_current = std::max(
              worst_total_current,norm(result.total_current_difference));
          worst_recombination = std::max(
              worst_recombination,result.split_recombination_residual);
          worst_analytic = std::max(
              worst_analytic,result.analytic_moment_residual);
          worst_reversal = std::max(
              worst_reversal,result.reversal_residual);
          smallest_monotonicity_margin = std::min(
              smallest_monotonicity_margin,result.monotonicity_margin);
          largest_split_difference = std::max(
              largest_split_difference,result.schedule_split_norm);
          check("registered arm " + std::to_string(registered_arms),
              result.valid
              && result.endpoint_position_residual <= gate
              && result.endpoint_derivative_residual <= gate
              && result.midpoint_derivative_residual <= gate
              && norm(result.total_current_difference) <= gate
              && result.split_recombination_residual <= gate
              && result.analytic_moment_residual <= gate
              && result.reversal_residual <= gate
              && std::abs(result.schedule_split_norm
                  -reference.schedule_split_norm) <= gate);
        }
      }
    }
  }
  check("registered arm count", registered_arms == 96);
  check("schedule split is nonzero", largest_split_difference > 1e-8);
  check("zero epsilon fails closed",
      !ftd::eft::evaluate_endpoint_schedule_underdetermination(
          0.1,0.0,{1.0,0.0,0.0},+1).valid);
  check("oversized epsilon fails closed",
      !ftd::eft::evaluate_endpoint_schedule_underdetermination(
          0.1,0.51,{1.0,0.0,0.0},+1).valid);
  check("invalid charge fails closed",
      !ftd::eft::evaluate_endpoint_schedule_underdetermination(
          0.1,0.25,{1.0,0.0,0.0},0).valid);
  check("zero direction fails closed",
      !ftd::eft::evaluate_endpoint_schedule_underdetermination(
          0.1,0.25,{0.0,0.0,0.0},+1).valid);
  check("nonfinite input fails closed",
      !ftd::eft::evaluate_endpoint_schedule_underdetermination(
          std::numeric_limits<double>::quiet_NaN(),0.25,
          {1.0,0.0,0.0},+1).valid);

  std::cout.precision(17);
  std::cout << "registered_arms=" << registered_arms << '\n'
            << "worst_endpoint_residual=" << worst_endpoint << '\n'
            << "worst_derivative_residual=" << worst_derivative << '\n'
            << "worst_total_current_residual=" << worst_total_current << '\n'
            << "worst_recombination_residual=" << worst_recombination << '\n'
            << "worst_analytic_moment_residual=" << worst_analytic << '\n'
            << "worst_reversal_residual=" << worst_reversal << '\n'
            << "smallest_monotonicity_margin="
            << smallest_monotonicity_margin << '\n'
            << "largest_split_difference=" << largest_split_difference << '\n'
            << "endpoint_schedule_underdetermination failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}

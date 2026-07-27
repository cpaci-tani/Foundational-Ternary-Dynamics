/** FTD-0548: exact quadratic-coat current on an accelerated worldline. */

#include "ftd/eft/accelerated_coat_spacetime_current.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 5e-12;
constexpr double rest_energy = 0.511;
constexpr double h = ftd::C_SPEED;

int failures = 0;
int registered_arms = 0;
double worst_total_current = 0.0;
double worst_recombination = 0.0;
double worst_partition = 0.0;
double worst_continuity = 0.0;
double worst_gauge = 0.0;
double worst_reversal = 0.0;
double largest_linear_split_difference = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double max_sum_residual(const std::vector<double>& lhs,
                        const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]+rhs[i]));
  return result;
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

double reversed_face_residual(
    const ftd::eft::MatchedFaceFlux& forward,
    const ftd::eft::MatchedFaceFlux& reverse) {
  return std::max({
      max_sum_residual(forward.x, reverse.x),
      max_sum_residual(forward.y, reverse.y),
      max_sum_residual(forward.z, reverse.z)});
}

void make_gauge(std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  constexpr double pi = 3.1415926535897932384626433832795;
  chi_start.resize(static_cast<std::size_t>(L)*L*L);
  chi_end.resize(chi_start.size());
  ftd::eft::MatchedFaceFlux indexing(L);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(indexing.index(x,y,z));
        chi_start[i] = 0.19*std::sin(2.0*pi*x/L)
            +0.11*std::cos(4.0*pi*y/L)-0.07*std::sin(2.0*pi*z/L);
        chi_end[i] = -0.13*std::cos(2.0*pi*x/L)
            +0.17*std::sin(2.0*pi*y/L)+0.05*std::cos(4.0*pi*z/L);
      }
    }
  }
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
  const ftd::Vec3 start{8.173, 8.281, 8.397};
  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(chi_start, chi_end);

  for (double beta : betas) {
    for (double momentum : momenta) {
      for (double field : fields) {
        for (int charge : {-1, +1}) {
          const double half_impulse = 0.5*beta*charge*field;
          for (const auto& direction : directions) {
            ++registered_arms;
            const auto current =
                ftd::eft::make_accelerated_coat_spacetime_current(
                    L, start, rest_energy, ftd::C_SPEED, h,
                    momentum, half_impulse, direction, charge);
            const double gauge =
                ftd::eft::accelerated_coat_gauge_endpoint_residual(
                    current, chi_start, chi_end);
            const auto reverse =
                ftd::eft::make_accelerated_coat_spacetime_current(
                    L, current.end_position, rest_energy, ftd::C_SPEED, h,
                    -momentum, half_impulse, direction, charge);
            const double reversal = std::max({
                reversed_face_residual(
                    current.spatial_quadrature,
                    reverse.spatial_quadrature),
                reversed_face_residual(
                    current.spatial_start,
                    reverse.spatial_end),
                reversed_face_residual(
                    current.spatial_end,
                    reverse.spatial_start),
                max_difference(current.temporal_charge,
                               reverse.temporal_charge),
                std::abs(reverse.end_position.x-start.x),
                std::abs(reverse.end_position.y-start.y),
                std::abs(reverse.end_position.z-start.z)});
            worst_total_current = std::max(
                worst_total_current, current.total_current_residual);
            worst_recombination = std::max(
                worst_recombination,
                current.split_recombination_residual);
            worst_partition = std::max(
                worst_partition, current.temporal_partition_residual);
            worst_continuity = std::max({worst_continuity,
                current.split_continuity_start_residual,
                current.split_continuity_end_residual});
            worst_gauge = std::max(worst_gauge, gauge);
            worst_reversal = std::max(worst_reversal, reversal);
            largest_linear_split_difference = std::max({
                largest_linear_split_difference,
                current.linear_start_difference,
                current.linear_end_difference,
                current.linear_temporal_difference});
            check("registered arm " + std::to_string(registered_arms),
                current.valid && reverse.valid
                && current.total_current_residual <= gate
                && current.split_recombination_residual <= gate
                && current.temporal_partition_residual <= gate
                && current.split_continuity_start_residual <= gate
                && current.split_continuity_end_residual <= gate
                && gauge <= gate && reversal <= gate);
          }
        }
      }
    }
  }
  check("registered arm count", registered_arms == 144);
  check("accelerated split differs from linear schedule",
      largest_linear_split_difference > 1e-8);

  const auto zero = ftd::eft::make_accelerated_coat_spacetime_current(
      L, start, rest_energy, ftd::C_SPEED, h,
      0.2, 0.0, {1.0,1.0,1.0}, +1);
  check("zero-force linear limit", zero.valid
      && zero.linear_start_difference <= gate
      && zero.linear_end_difference <= gate
      && zero.linear_temporal_difference <= gate);
  check("invalid charge fails closed",
      !ftd::eft::make_accelerated_coat_spacetime_current(
          L, start, rest_energy, ftd::C_SPEED, h,
          0.2, 0.01, {1.0,0.0,0.0}, 0).valid);
  check("invalid volume fails closed",
      !ftd::eft::make_accelerated_coat_spacetime_current(
          3, start, rest_energy, ftd::C_SPEED, h,
          0.2, 0.01, {1.0,0.0,0.0}, +1).valid);
  check("zero direction fails closed",
      !ftd::eft::make_accelerated_coat_spacetime_current(
          L, start, rest_energy, ftd::C_SPEED, h,
          0.2, 0.01, {0.0,0.0,0.0}, +1).valid);
  check("within-tick turning fails closed",
      !ftd::eft::make_accelerated_coat_spacetime_current(
          L, start, rest_energy, ftd::C_SPEED, h,
          0.01, 0.02, {1.0,0.0,0.0}, +1).valid);
  check("nonfinite input fails closed",
      !ftd::eft::make_accelerated_coat_spacetime_current(
          L, start, rest_energy, ftd::C_SPEED, h,
          std::numeric_limits<double>::quiet_NaN(), 0.01,
          {1.0,0.0,0.0}, +1).valid);

  std::cout.precision(17);
  std::cout << "registered_arms=" << registered_arms << '\n'
            << "worst_total_current_residual=" << worst_total_current << '\n'
            << "worst_recombination_residual=" << worst_recombination << '\n'
            << "worst_temporal_partition_residual=" << worst_partition << '\n'
            << "worst_split_continuity_residual=" << worst_continuity << '\n'
            << "worst_gauge_endpoint_residual=" << worst_gauge << '\n'
            << "worst_reversal_residual=" << worst_reversal << '\n'
            << "largest_linear_split_difference="
            << largest_linear_split_difference << '\n'
            << "accelerated_coat_spacetime_current failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}

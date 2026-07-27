/**
 * FTD-0545: the smooth-coat fixed-step action does not automatically satisfy
 * the exact matter-work identity required by the matched field transaction.
 */

#include "ftd/eft/quadratic_coat_matter_work.h"
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
constexpr double h = ftd::C_SPEED;
constexpr double rest_energy = 0.511;
constexpr double beta = 1.0;
constexpr double algebra_gate = 1e-11;
constexpr double action_gate = 1e-12;
constexpr double nonidentity_gate = 1e-8;

int failures = 0;
int registered_arms = 0;
double worst_action_residual = 0.0;
double worst_analytic_residual = 0.0;
double worst_gauge_residual = 0.0;
double largest_nonzero_defect = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double component(const ftd::Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

double max_difference(const ftd::Vec3& a, const ftd::Vec3& b) {
  return std::max({std::abs(a.x - b.x),
                   std::abs(a.y - b.y),
                   std::abs(a.z - b.z)});
}

double energy(const ftd::Vec3& momentum) {
  return std::sqrt(rest_energy * rest_energy
      + ftd::C_SPEED * ftd::C_SPEED * momentum.mag2());
}

ftd::Vec3 normalized(const ftd::Vec3& value) {
  return value * (1.0 / std::sqrt(value.mag2()));
}

ftd::eft::DualGaugePotentialSlab make_uniform_electric(
    const ftd::Vec3& electric) {
  ftd::eft::DualGaugePotentialSlab slab(L, h);
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  for (std::size_t i = 0; i < count; ++i) {
    slab.A_end.x[i] = -h * electric.x;
    slab.A_end.y[i] = -h * electric.y;
    slab.A_end.z[i] = -h * electric.z;
  }
  return slab;
}

void make_gauge(const ftd::eft::DualGaugePotentialSlab& indexing,
                std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  chi_start.assign(count, 0.0);
  chi_end.assign(count, 0.0);
  constexpr double pi = 3.1415926535897932384626433832795;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(indexing.index(x, y, z));
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        chi_start[i] = 0.031 * std::sin(px + py)
            + 0.019 * std::cos(pz);
        chi_end[i] = -0.023 * std::cos(py + pz)
            + 0.029 * std::sin(px);
      }
    }
  }
}

void run_uniform_arm(double momentum_magnitude,
                     double electric_magnitude,
                     int charge,
                     const ftd::Vec3& raw_direction) {
  ++registered_arms;
  const ftd::Vec3 direction = normalized(raw_direction);
  const ftd::Vec3 momentum = direction * momentum_magnitude;
  const ftd::Vec3 electric = direction * electric_magnitude;
  const double free_energy = energy(momentum);
  const ftd::Vec3 displacement = momentum * (
      h * ftd::C_SPEED / free_energy);
  const ftd::Vec3 start{8.173, 8.281, 8.397};
  const ftd::Vec3 end = start + displacement;
  const auto slab = make_uniform_electric(electric);
  const auto result = ftd::eft::evaluate_quadratic_coat_matter_work(
      start, end, charge, rest_energy, ftd::C_SPEED, beta, slab);

  const ftd::Vec3 half_impulse = electric * (0.5 * beta * charge);
  const ftd::Vec3 expected_start = momentum - half_impulse;
  const ftd::Vec3 expected_end = momentum + half_impulse;
  const double expected_work = 2.0 * ftd::C_SPEED * ftd::C_SPEED
      * momentum.dot(half_impulse) / free_energy;
  const double expected_defect = energy(expected_end)
      - energy(expected_start) - expected_work;
  const double analytic_residual = std::max({
      max_difference(result.free_momentum, momentum),
      max_difference(result.kinetic_start, expected_start),
      max_difference(result.kinetic_end, expected_end),
      std::abs(result.field_work - expected_work),
      std::abs(result.matter_work_defect - expected_defect)});
  worst_action_residual = std::max(
      worst_action_residual, result.deposited_action_residual);
  worst_analytic_residual = std::max(
      worst_analytic_residual, analytic_residual);
  largest_nonzero_defect = std::max(
      largest_nonzero_defect, std::abs(result.matter_work_defect));

  const std::string label = "p=" + std::to_string(momentum_magnitude)
      + " E=" + std::to_string(electric_magnitude)
      + " q=" + std::to_string(charge)
      + " dir=" + std::to_string(raw_direction.x)
      + std::to_string(raw_direction.y)
      + std::to_string(raw_direction.z);
  check(label, result.valid && result.derivative_smooth
      && result.deposited_action_residual <= action_gate
      && analytic_residual <= algebra_gate
      && expected_defect * charge < 0.0);
}

void run_zero_field_control() {
  const ftd::Vec3 direction = normalized({1.0, 1.0, 0.0});
  const ftd::Vec3 momentum = direction * 0.2;
  const double free_energy = energy(momentum);
  const ftd::Vec3 start{7.173, 8.281, 9.397};
  const ftd::Vec3 end = start + momentum * (
      h * ftd::C_SPEED / free_energy);
  const auto result = ftd::eft::evaluate_quadratic_coat_matter_work(
      start, end, +1, rest_energy, ftd::C_SPEED, beta,
      make_uniform_electric({0.0, 0.0, 0.0}));
  check("zero-field control", result.valid
      && max_difference(result.kinetic_start, momentum) <= action_gate
      && max_difference(result.kinetic_end, momentum) <= action_gate
      && std::abs(result.field_work) <= action_gate
      && std::abs(result.matter_work_defect) <= action_gate);
}

void run_gauge_control() {
  const ftd::Vec3 direction = normalized({1.0, -1.0, 1.0});
  const ftd::Vec3 momentum = direction * 0.2;
  const ftd::Vec3 electric = direction * 0.08;
  const double free_energy = energy(momentum);
  const ftd::Vec3 start{6.173, 7.281, 8.397};
  const ftd::Vec3 end = start + momentum * (
      h * ftd::C_SPEED / free_energy);
  const auto slab = make_uniform_electric(electric);
  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(slab, chi_start, chi_end);
  const auto transformed = ftd::eft::gauge_transform_slab(
      slab, chi_start, chi_end);
  const auto base = ftd::eft::evaluate_quadratic_coat_matter_work(
      start, end, -1, rest_energy, ftd::C_SPEED, beta, slab);
  const auto gauged = ftd::eft::evaluate_quadratic_coat_matter_work(
      start, end, -1, rest_energy, ftd::C_SPEED, beta, transformed);
  worst_gauge_residual = std::max({
      max_difference(base.kinetic_start, gauged.kinetic_start),
      max_difference(base.kinetic_end, gauged.kinetic_end),
      std::abs(base.field_work - gauged.field_work),
      std::abs(base.matter_work_defect - gauged.matter_work_defect)});
  check("gauge-covariant kinetic endpoints", base.valid && gauged.valid
      && worst_gauge_residual <= algebra_gate);

  const auto zero_slab = make_uniform_electric({0.0, 0.0, 0.0});
  const auto pure_gauge_slab = ftd::eft::gauge_transform_slab(
      zero_slab, chi_start, chi_end);
  const auto pure_gauge = ftd::eft::evaluate_quadratic_coat_matter_work(
      start, end, -1, rest_energy, ftd::C_SPEED, beta, pure_gauge_slab);
  const double pure_gauge_residual = std::max({
      max_difference(pure_gauge.kinetic_start, momentum),
      max_difference(pure_gauge.kinetic_end, momentum),
      std::abs(pure_gauge.field_work),
      std::abs(pure_gauge.matter_work_defect)});
  worst_gauge_residual = std::max(
      worst_gauge_residual, pure_gauge_residual);
  check("pure-gauge control", pure_gauge.valid
      && pure_gauge_residual <= action_gate);
}

void run_invalid_controls() {
  const auto slab = make_uniform_electric({0.04, 0.0, 0.0});
  check("zero charge fails closed",
      !ftd::eft::evaluate_quadratic_coat_matter_work(
          {5.1, 5.2, 5.3}, {5.2, 5.2, 5.3}, 0,
          rest_energy, ftd::C_SPEED, beta, slab).valid);
  check("supercausal segment fails closed",
      !ftd::eft::evaluate_quadratic_coat_matter_work(
          {5.1, 5.2, 5.3}, {5.1 + h, 5.2, 5.3}, +1,
          rest_energy, ftd::C_SPEED, beta, slab).valid);
  check("zero beta fails closed",
      !ftd::eft::evaluate_quadratic_coat_matter_work(
          {5.1, 5.2, 5.3}, {5.2, 5.2, 5.3}, +1,
          rest_energy, ftd::C_SPEED, 0.0, slab).valid);
  check("nonfinite endpoint fails closed",
      !ftd::eft::evaluate_quadratic_coat_matter_work(
          {5.1, 5.2, 5.3},
          {std::numeric_limits<double>::quiet_NaN(), 5.2, 5.3}, +1,
          rest_energy, ftd::C_SPEED, beta, slab).valid);
}

}  // namespace

int main() {
  const std::array<double, 3> momenta{{0.1, 0.2, 0.3}};
  const std::array<double, 3> electric_fields{{0.04, 0.08, 0.12}};
  const std::array<ftd::Vec3, 4> directions{{
      {1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {0.0, 0.0, 1.0},
      {1.0, 1.0, 1.0}}};
  for (double momentum : momenta) {
    for (double electric : electric_fields) {
      for (int charge : {-1, +1}) {
        for (const auto& direction : directions) {
          run_uniform_arm(momentum, electric, charge, direction);
        }
      }
    }
  }
  check("registered arm count", registered_arms == 72);
  check("registered nonidentity witness",
      largest_nonzero_defect > nonidentity_gate);
  run_zero_field_control();
  run_gauge_control();
  run_invalid_controls();

  std::cout.precision(17);
  std::cout << "registered_arms=" << registered_arms << '\n'
            << "worst_action_residual=" << worst_action_residual << '\n'
            << "worst_analytic_residual=" << worst_analytic_residual << '\n'
            << "worst_gauge_residual=" << worst_gauge_residual << '\n'
            << "largest_nonzero_defect=" << largest_nonzero_defect << '\n'
            << "quadratic_coat_matter_work failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

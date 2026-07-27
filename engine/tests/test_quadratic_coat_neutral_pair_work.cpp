/**
 * FTD-0546: neutral self-consistent longitudinal transaction of the smooth
 * quadratic-coat action.
 */

#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/quadratic_coat_neutral_pair_work.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double gate = 1e-12;
constexpr double poisson_gate = 1e-13;
constexpr double gauge_gate = 1e-10;
constexpr double nonidentity_gate = 1e-10;
constexpr double rest_energy = 0.511;

int failures = 0;
int registered_arms = 0;
double worst_poisson = 0.0;
double worst_algebra = 0.0;
double worst_action = 0.0;
double worst_field_work = 0.0;
double worst_gauge = 0.0;
double largest_temporal_midpoint_mismatch = 0.0;
double largest_pair_defect = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

ftd::Vec3 normalized(const ftd::Vec3& value) {
  return value * (1.0 / std::sqrt(value.mag2()));
}

double max_difference(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

void make_gauge(const ftd::eft::DualGaugePotentialSlab& indexing,
                std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  const int L = indexing.L;
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
        chi_start[i] = 0.017 * std::sin(px + py)
            + 0.011 * std::cos(pz);
        chi_end[i] = -0.013 * std::cos(py + pz)
            + 0.019 * std::sin(px);
      }
    }
  }
}

struct ArmGeometry {
  std::array<ftd::Vec3, 2> start{};
  std::array<ftd::Vec3, 2> end{};
  std::array<int, 2> charge{};
};

ArmGeometry make_arm(int L,
                     double separation,
                     double displacement,
                     int radial_sign,
                     int charge_order,
                     const ftd::Vec3& raw_direction) {
  const ftd::Vec3 direction = normalized(raw_direction);
  const double center_base = 0.5 * (L - 1);
  const ftd::Vec3 center{
      center_base + 0.173,
      center_base + 0.281,
      center_base + 0.397};
  ArmGeometry arm;
  arm.start[0] = center - direction * (0.5 * separation);
  arm.start[1] = center + direction * (0.5 * separation);
  arm.end[0] = arm.start[0]
      + direction * (radial_sign * displacement);
  arm.end[1] = arm.start[1]
      - direction * (radial_sign * displacement);
  arm.charge = charge_order == 0
      ? std::array<int, 2>{{+1, -1}}
      : std::array<int, 2>{{-1, +1}};
  return arm;
}

void accumulate(const std::string& label,
                const ftd::eft::QuadraticCoatNeutralPairWorkResult& result) {
  const double algebra = std::max({
      result.neutrality_residual,
      result.temporal_gauss_residual,
      result.split_continuity_residual,
      result.endpoint_gauss_residual,
      result.field_update_residual,
      result.midpoint_split_residual});
  worst_poisson = std::max(worst_poisson, result.poisson_residual);
  worst_algebra = std::max(worst_algebra, algebra);
  worst_action = std::max(worst_action, result.action_residual);
  worst_field_work = std::max(
      worst_field_work, result.field_work_residual);
  largest_temporal_midpoint_mismatch = std::max(
      largest_temporal_midpoint_mismatch,
      result.temporal_endpoint_average_mismatch);
  largest_pair_defect = std::max(
      largest_pair_defect, std::abs(result.pair_matter_work_defect));
  check(label, result.valid && result.poisson_converged
      && result.poisson_residual <= poisson_gate
      && algebra <= gate
      && result.action_residual <= gate
      && result.field_work_residual <= gate
      && std::abs(result.total_energy_defect
          - result.pair_matter_work_defect) <= gate);
}

void run_gauge_control(double beta) {
  const auto arm = make_arm(17, 3.0, 0.05, +1, 0, {1.0, 1.0, 1.0});
  const auto base = ftd::eft::evaluate_quadratic_coat_neutral_pair_work(
      17, arm.start, arm.end, arm.charge, rest_energy, ftd::C_SPEED, beta,
      poisson_gate, 4096);
  check("gauge base valid", base.valid);
  if (!base.valid) return;
  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(base.slab, chi_start, chi_end);
  const auto transformed = ftd::eft::gauge_transform_slab(
      base.slab, chi_start, chi_end);
  double gauged_matter_change = 0.0;
  for (std::size_t carrier = 0; carrier < 2; ++carrier) {
    const auto matter = ftd::eft::evaluate_quadratic_coat_matter_work(
        arm.start[carrier], arm.end[carrier], arm.charge[carrier],
        rest_energy, ftd::C_SPEED, beta, transformed);
    check("gauged matter valid", matter.valid);
    if (!matter.valid) continue;
    worst_gauge = std::max({worst_gauge,
        max_difference(matter.kinetic_start,
                       base.matter[carrier].kinetic_start),
        max_difference(matter.kinetic_end,
                       base.matter[carrier].kinetic_end),
        matter.deposited_action_residual});
    gauged_matter_change += matter.matter_energy_change;
  }
  worst_gauge = std::max(worst_gauge, std::abs(
      gauged_matter_change - base.matter_energy_change));
  const double gauged_pair_defect = gauged_matter_change - base.field_work;
  worst_gauge = std::max(worst_gauge, std::abs(
      gauged_pair_defect - base.pair_matter_work_defect));
  check("neutral pair gauge covariance", worst_gauge <= gauge_gate);
}

void run_zero_motion_control(double beta) {
  auto arm = make_arm(17, 3.0, 0.0, +1, 0, {1.0, 0.0, 0.0});
  const auto result = ftd::eft::evaluate_quadratic_coat_neutral_pair_work(
      17, arm.start, arm.end, arm.charge, rest_energy, ftd::C_SPEED, beta,
      poisson_gate, 4096);
  accumulate("zero-motion algebra", result);
  check("zero-motion energy", result.valid
      && ftd::eft::l1_norm(result.current_total) <= gate
      && std::abs(result.field_energy_change) <= gate
      && std::abs(result.matter_energy_change) <= gate
      && std::abs(result.pair_matter_work_defect) <= gate);
}

void run_invalid_controls(double beta) {
  auto arm = make_arm(17, 3.0, 0.02, +1, 0, {1.0, 0.0, 0.0});
  arm.charge = {{+1, +1}};
  check("non-neutral input fails closed",
      !ftd::eft::evaluate_quadratic_coat_neutral_pair_work(
          17, arm.start, arm.end, arm.charge, rest_energy,
          ftd::C_SPEED, beta, poisson_gate, 4096).valid);
  arm = make_arm(17, 3.0, 0.02, +1, 0, {1.0, 0.0, 0.0});
  arm.end[0] = arm.start[0] + ftd::Vec3{ftd::C_SPEED, 0.0, 0.0};
  check("causal-boundary input fails closed",
      !ftd::eft::evaluate_quadratic_coat_neutral_pair_work(
          17, arm.start, arm.end, arm.charge, rest_energy,
          ftd::C_SPEED, beta, poisson_gate, 4096).valid);
}

}  // namespace

int main() {
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  check("native normalization valid", normalization.valid);
  const double beta = normalization.mapped_field_work_coefficient;
  const std::array<ftd::Vec3, 4> directions{{
      {1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {0.0, 0.0, 1.0},
      {1.0, 1.0, 1.0}}};
  for (int L : {17, 19}) {
    for (double separation : {3.0, 4.0}) {
      for (double displacement : {0.02, 0.05}) {
        for (int radial_sign : {-1, +1}) {
          for (int charge_order : {0, 1}) {
            for (const auto& direction : directions) {
              const auto arm = make_arm(L, separation, displacement,
                  radial_sign, charge_order, direction);
              const auto result =
                  ftd::eft::evaluate_quadratic_coat_neutral_pair_work(
                      L, arm.start, arm.end, arm.charge,
                      rest_energy, ftd::C_SPEED, beta,
                      poisson_gate, 4096);
              ++registered_arms;
              accumulate("registered arm "
                  + std::to_string(registered_arms), result);
            }
          }
        }
      }
    }
  }
  check("registered arm count", registered_arms == 128);
  check("temporal source is not endpoint average",
      largest_temporal_midpoint_mismatch > nonidentity_gate);
  check("self-consistent nonidentity witness",
      largest_pair_defect > nonidentity_gate);
  run_gauge_control(beta);
  run_zero_motion_control(beta);
  run_invalid_controls(beta);

  std::cout.precision(17);
  std::cout << "registered_arms=" << registered_arms << '\n'
            << "beta=" << beta << '\n'
            << "worst_poisson_residual=" << worst_poisson << '\n'
            << "worst_algebra_residual=" << worst_algebra << '\n'
            << "worst_action_residual=" << worst_action << '\n'
            << "worst_field_work_residual=" << worst_field_work << '\n'
            << "worst_gauge_residual=" << worst_gauge << '\n'
            << "largest_temporal_midpoint_mismatch="
            << largest_temporal_midpoint_mismatch << '\n'
            << "largest_pair_defect=" << largest_pair_defect << '\n'
            << "quadratic_coat_neutral_pair_work failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}

/**
 * FTD-0485: two-slab common-action force and threshold differentiability.
 */

#include "ftd/eft/two_slab_variational_force.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double lambda_t = 0.57735026918962576451;
constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_component(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

struct SlabPair {
  ftd::eft::DualGaugePotentialSlab previous{L, lambda_t};
  ftd::eft::DualGaugePotentialSlab next{L, lambda_t};
};

SlabPair general_pair() {
  SlabPair pair;
  constexpr double pi = 3.141592653589793238462643383279502884;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = pair.previous.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        const double a0x = 0.013 * std::sin(py) + 0.004 * std::cos(pz);
        const double a0y = -0.009 * std::sin(pz) + 0.006 * std::cos(px);
        const double a0z = 0.011 * std::sin(px) - 0.005 * std::cos(py);
        const double a1x = a0x + 0.003 * std::cos(px + py);
        const double a1y = a0y - 0.002 * std::sin(py + pz);
        const double a1z = a0z + 0.004 * std::cos(pz + px);
        pair.previous.A_start.x[static_cast<std::size_t>(i)] = a0x;
        pair.previous.A_start.y[static_cast<std::size_t>(i)] = a0y;
        pair.previous.A_start.z[static_cast<std::size_t>(i)] = a0z;
        pair.previous.A_end.x[static_cast<std::size_t>(i)] = a1x;
        pair.previous.A_end.y[static_cast<std::size_t>(i)] = a1y;
        pair.previous.A_end.z[static_cast<std::size_t>(i)] = a1z;
        pair.next.A_start.x[static_cast<std::size_t>(i)] = a1x;
        pair.next.A_start.y[static_cast<std::size_t>(i)] = a1y;
        pair.next.A_start.z[static_cast<std::size_t>(i)] = a1z;
        pair.next.A_end.x[static_cast<std::size_t>(i)] =
            a1x - 0.001 * std::sin(px + pz);
        pair.next.A_end.y[static_cast<std::size_t>(i)] =
            a1y + 0.003 * std::cos(px + py);
        pair.next.A_end.z[static_cast<std::size_t>(i)] =
            a1z - 0.002 * std::sin(py + pz);
        pair.previous.Phi[static_cast<std::size_t>(i)] =
            0.017 * std::sin(px) + 0.007 * std::cos(py);
        pair.next.Phi[static_cast<std::size_t>(i)] =
            -0.012 * std::cos(py) + 0.005 * std::sin(pz);
      }
    }
  }
  return pair;
}

void make_gauge(std::vector<double>& chi0,
                std::vector<double>& chi1,
                std::vector<double>& chi2) {
  chi0.assign(static_cast<std::size_t>(L * L * L), 0.0);
  chi1 = chi0;
  chi2 = chi0;
  constexpr double pi = 3.141592653589793238462643383279502884;
  ftd::eft::DualGaugePotentialSlab indexing(L, lambda_t);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = indexing.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        chi0[static_cast<std::size_t>(i)] =
            0.031 * std::sin(px + py) + 0.011 * std::cos(pz);
        chi1[static_cast<std::size_t>(i)] =
            -0.019 * std::cos(py + pz) + 0.023 * std::sin(px);
        chi2[static_cast<std::size_t>(i)] =
            0.017 * std::sin(pz + px) - 0.029 * std::cos(py);
      }
    }
  }
}

SlabPair uniform_electric_pair(int axis, double electric) {
  SlabPair pair;
  std::vector<double>* previous_end = axis == 0 ? &pair.previous.A_end.x
      : (axis == 1 ? &pair.previous.A_end.y : &pair.previous.A_end.z);
  std::vector<double>* next_start = axis == 0 ? &pair.next.A_start.x
      : (axis == 1 ? &pair.next.A_start.y : &pair.next.A_start.z);
  std::vector<double>* next_end = axis == 0 ? &pair.next.A_end.x
      : (axis == 1 ? &pair.next.A_end.y : &pair.next.A_end.z);
  std::fill(previous_end->begin(), previous_end->end(),
            -lambda_t * electric);
  std::fill(next_start->begin(), next_start->end(),
            -lambda_t * electric);
  std::fill(next_end->begin(), next_end->end(),
            -2.0 * lambda_t * electric);
  return pair;
}

SlabPair uniform_magnetic_pair(double magnetic_z) {
  SlabPair pair;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = pair.previous.index(x, y, z);
        const double ay = magnetic_z * x;
        pair.previous.A_start.y[static_cast<std::size_t>(i)] = ay;
        pair.previous.A_end.y[static_cast<std::size_t>(i)] = ay;
        pair.next.A_start.y[static_cast<std::size_t>(i)] = ay;
        pair.next.A_end.y[static_cast<std::size_t>(i)] = ay;
      }
    }
  }
  return pair;
}

SlabPair threshold_pair(double electric_left, double electric_right) {
  SlabPair pair;
  for (int x = 0; x < L; ++x) {
    const double electric = x == 4 ? electric_left
        : (x == 5 ? electric_right : 0.0);
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = pair.previous.index(x, y, z);
        pair.previous.A_end.x[static_cast<std::size_t>(i)] =
            -lambda_t * electric;
        pair.next.A_start.x[static_cast<std::size_t>(i)] =
            -lambda_t * electric;
        pair.next.A_end.x[static_cast<std::size_t>(i)] =
            -2.0 * lambda_t * electric;
      }
    }
  }
  return pair;
}

}  // namespace

int main() {
  constexpr double coupling = 0.73;
  const ftd::Vec3 previous{5.2, 6.3, 7.4};
  const ftd::Vec3 shared{5.4, 6.45, 7.35};
  const ftd::Vec3 next{5.65, 6.55, 7.2};
  const auto pair = general_pair();
  const auto base = ftd::eft::evaluate_two_slab_variational_force(
      previous, shared, next, +1, pair.previous, pair.next, coupling);
  check("direct action equals FTD-0484 deposited action",
        base.valid
        && base.previous_deposit_action_residual <= gate
        && base.next_deposit_action_residual <= gate);

  std::vector<double> chi0;
  std::vector<double> chi1;
  std::vector<double> chi2;
  make_gauge(chi0, chi1, chi2);
  const auto transformed_previous = ftd::eft::gauge_transform_slab(
      pair.previous, chi0, chi1);
  const auto transformed_next = ftd::eft::gauge_transform_slab(
      pair.next, chi1, chi2);
  const auto transformed = ftd::eft::evaluate_two_slab_variational_force(
      previous, shared, next, +1,
      transformed_previous, transformed_next, coupling);
  const double gauge_force_residual = max_difference(
      base.interaction_impulse, transformed.interaction_impulse);
  check("two-slab interior impulse is gauge invariant",
        transformed.valid && gauge_force_residual <= gate);

  const SlabPair zero;
  const auto pure_previous = ftd::eft::gauge_transform_slab(
      zero.previous, chi0, chi1);
  const auto pure_next = ftd::eft::gauge_transform_slab(
      zero.next, chi1, chi2);
  const auto pure = ftd::eft::evaluate_two_slab_variational_force(
      previous, shared, next, +1,
      pure_previous, pure_next, coupling);
  const double pure_gauge_residual = max_component(pure.interaction_impulse);
  check("nonzero pure gauge gives zero interior impulse",
        pure.valid && pure_gauge_residual <= gate);

  constexpr double electric = 0.037;
  const auto electric_y = uniform_electric_pair(1, electric);
  const ftd::Vec3 rest{5.3, 6.4, 7.5};
  const auto electric_positive = ftd::eft::evaluate_two_slab_variational_force(
      rest, rest, rest, +1,
      electric_y.previous, electric_y.next, 1.0);
  const auto electric_negative = ftd::eft::evaluate_two_slab_variational_force(
      rest, rest, rest, -1,
      electric_y.previous, electric_y.next, 1.0);
  const ftd::Vec3 expected_electric{0.0, lambda_t * electric, 0.0};
  check("stationary particle receives transverse electric impulse",
        electric_positive.valid
        && max_difference(electric_positive.interaction_impulse,
                          expected_electric) <= gate);
  check("polarity reversal reverses variational impulse",
        electric_negative.valid
        && max_difference(electric_negative.interaction_impulse,
                          {0.0, -lambda_t * electric, 0.0}) <= gate);

  const auto electric_x = uniform_electric_pair(0, electric);
  const auto rotated_electric = ftd::eft::evaluate_two_slab_variational_force(
      rest, rest, rest, +1,
      electric_x.previous, electric_x.next, 1.0);
  check("proper-axis rotation rotates electric impulse",
        rotated_electric.valid
        && max_difference(rotated_electric.interaction_impulse,
                          {expected_electric.y, 0.0, 0.0}) <= gate);

  constexpr double magnetic_z = 0.041;
  const auto magnetic = uniform_magnetic_pair(magnetic_z);
  const auto magnetic_result = ftd::eft::evaluate_two_slab_variational_force(
      {5.2, 6.4, 7.5}, {5.4, 6.4, 7.5}, {5.6, 6.4, 7.5}, +1,
      magnetic.previous, magnetic.next, 1.0);
  const ftd::Vec3 expected_magnetic{0.0, -0.2 * magnetic_z, 0.0};
  const double magnetic_origin_residual = max_difference(
      magnetic_result.interaction_impulse, expected_magnetic);
  check("affine connection gives exact magnetic curvature impulse",
        magnetic_result.valid && magnetic_origin_residual <= gate);

  constexpr double epsilon = 1e-8;
  constexpr double electric_left = 0.02;
  constexpr double electric_right = -0.03;
  const auto threshold = threshold_pair(electric_left, electric_right);
  const ftd::Vec3 left{5.0 - epsilon, 6.4, 7.5};
  const ftd::Vec3 right{5.0 + epsilon, 6.4, 7.5};
  const auto left_result = ftd::eft::evaluate_two_slab_variational_force(
      left, left, left, +1, threshold.previous, threshold.next, 1.0);
  const auto right_result = ftd::eft::evaluate_two_slab_variational_force(
      right, right, right, +1, threshold.previous, threshold.next, 1.0);
  const double threshold_gap = max_difference(
      left_result.interaction_impulse, right_result.interaction_impulse);
  const double expected_threshold_gap =
      lambda_t * std::abs(electric_left - electric_right);
  check("allowed compact connection exposes threshold nonuniqueness",
        left_result.valid && right_result.valid
        && threshold_gap > gate
        && std::abs(threshold_gap - expected_threshold_gap) <= gate);

  check("invalid cross-cell slab is rejected",
        !ftd::eft::evaluate_two_slab_variational_force(
            {5.2, 6.4, 7.5}, {6.2, 6.4, 7.5}, {6.4, 6.4, 7.5}, +1,
            zero.previous, zero.next, 1.0).valid);

  std::cout.precision(17);
  std::cout << "previous_deposit_action_residual="
            << base.previous_deposit_action_residual << '\n'
            << "next_deposit_action_residual="
            << base.next_deposit_action_residual << '\n'
            << "gauge_force_residual=" << gauge_force_residual << '\n'
            << "pure_gauge_residual=" << pure_gauge_residual << '\n'
            << "stationary_transverse_impulse="
            << electric_positive.interaction_impulse.y << '\n'
            << "magnetic_origin_residual=" << magnetic_origin_residual << '\n'
            << "threshold_left_impulse=" << left_result.interaction_impulse.x << '\n'
            << "threshold_right_impulse=" << right_result.interaction_impulse.x << '\n'
            << "threshold_force_gap=" << threshold_gap << '\n'
            << "two_slab_variational_force failures=" << failures << '\n'
            << "verdict=INTERIOR_VARIATIONAL_FORCE_DERIVED_THRESHOLD_NONUNIQUE\n";
  return failures == 0 ? 0 : 1;
}

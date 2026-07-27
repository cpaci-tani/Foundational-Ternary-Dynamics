/**
 * FTD-0484: exact spacetime worldline current and gauge-endpoint identity.
 */

#include "ftd/eft/spacetime_worldline_coupling.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double time_scale = 0.57735026918962576451;
constexpr double gate = 1e-12;
int failures = 0;
double worst_split = 0.0;
double worst_partition = 0.0;
double worst_split_continuity = 0.0;
double worst_endpoint = 0.0;
double worst_electric = 0.0;
double worst_magnetic = 0.0;
double worst_curl_gradient = 0.0;
double worst_source_variation = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double value = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    value = std::max(value, std::abs(lhs[i] - rhs[i]));
  }
  return value;
}

double source_variation_residual(
    const ftd::eft::SpacetimeWorldlineCurrent& current,
    const ftd::eft::DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end) {
  constexpr double coupling = 0.73;
  const auto baseline = ftd::eft::evaluate_spacetime_gauge_coupling(
      current, slab, chi_start, chi_end, coupling);
  if (!baseline.valid) return INFINITY;

  std::size_t face_index = 0;
  double face_value = 0.0;
  for (std::size_t i = 0; i < current.spatial_start.x.size(); ++i) {
    const double candidate = std::abs(current.spatial_start.x[i]);
    if (candidate > std::abs(face_value)) {
      face_value = current.spatial_start.x[i];
      face_index = i;
    }
  }
  double face_residual = 0.0;
  if (face_value != 0.0) {
    auto varied = slab;
    varied.A_start.x[face_index] += 1.0;
    const auto shifted = ftd::eft::evaluate_spacetime_gauge_coupling(
        current, varied, chi_start, chi_end, coupling);
    face_residual = std::abs(
        (shifted.interaction_action - baseline.interaction_action)
        - coupling * face_value);
  }

  std::size_t temporal_index = 0;
  double temporal_value = 0.0;
  for (std::size_t i = 0; i < current.temporal_charge.size(); ++i) {
    const double candidate = std::abs(current.temporal_charge[i]);
    if (candidate > std::abs(temporal_value)) {
      temporal_value = current.temporal_charge[i];
      temporal_index = i;
    }
  }
  auto varied = slab;
  varied.Phi[temporal_index] += 1.0;
  const auto shifted = ftd::eft::evaluate_spacetime_gauge_coupling(
      current, varied, chi_start, chi_end, coupling);
  const double temporal_residual = std::abs(
      (shifted.interaction_action - baseline.interaction_action)
      + coupling * time_scale * temporal_value);
  return std::max(face_residual, temporal_residual);
}

ftd::eft::DualGaugePotentialSlab make_slab() {
  ftd::eft::DualGaugePotentialSlab slab(L, time_scale);
  constexpr double pi = 3.141592653589793238462643383279502884;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = slab.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        slab.A_start.x[static_cast<std::size_t>(i)] =
            0.013 * std::sin(py) + 0.007 * std::cos(pz);
        slab.A_start.y[static_cast<std::size_t>(i)] =
            -0.009 * std::sin(pz) + 0.005 * std::cos(px);
        slab.A_start.z[static_cast<std::size_t>(i)] =
            0.011 * std::sin(px) - 0.004 * std::cos(py);
        slab.A_end.x[static_cast<std::size_t>(i)] =
            slab.A_start.x[static_cast<std::size_t>(i)]
            + 0.003 * std::cos(px + py);
        slab.A_end.y[static_cast<std::size_t>(i)] =
            slab.A_start.y[static_cast<std::size_t>(i)]
            - 0.002 * std::sin(py + pz);
        slab.A_end.z[static_cast<std::size_t>(i)] =
            slab.A_start.z[static_cast<std::size_t>(i)]
            + 0.004 * std::cos(pz + px);
        slab.Phi[static_cast<std::size_t>(i)] =
            0.017 * std::sin(px) + 0.012 * std::cos(py)
            - 0.006 * std::sin(pz);
      }
    }
  }
  return slab;
}

void make_gauge(std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  chi_start.assign(static_cast<std::size_t>(L * L * L), 0.0);
  chi_end.assign(static_cast<std::size_t>(L * L * L), 0.0);
  constexpr double pi = 3.141592653589793238462643383279502884;
  ftd::eft::DualGaugePotentialSlab indexing(L, time_scale);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = indexing.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        chi_start[static_cast<std::size_t>(i)] =
            0.031 * std::sin(px + py) + 0.019 * std::cos(pz);
        chi_end[static_cast<std::size_t>(i)] =
            -0.023 * std::cos(py + pz) + 0.029 * std::sin(px);
      }
    }
  }
}

void run_arm(const std::string& label,
             ftd::Coord start_anchor,
             ftd::Vec3 start_remainder,
             ftd::Coord end_anchor,
             ftd::Vec3 end_remainder,
             int charge,
             const ftd::eft::DualGaugePotentialSlab& slab,
             const std::vector<double>& chi_start,
             const std::vector<double>& chi_end) {
  const auto current = ftd::eft::make_spacetime_worldline_current(
      L, start_anchor, start_remainder,
      end_anchor, end_remainder, charge, time_scale);
  const auto result = ftd::eft::evaluate_spacetime_gauge_coupling(
      current, slab, chi_start, chi_end, 0.73);
  const double variation = source_variation_residual(
      current, slab, chi_start, chi_end);
  worst_split = std::max(worst_split, current.spatial_split_residual);
  worst_partition = std::max(
      worst_partition, current.temporal_partition_residual);
  worst_split_continuity = std::max({worst_split_continuity,
      current.split_continuity_start_residual,
      current.split_continuity_end_residual});
  worst_endpoint = std::max(
      worst_endpoint, result.gauge_endpoint_residual);
  worst_electric = std::max(
      worst_electric, result.electric_invariance_residual);
  worst_magnetic = std::max(
      worst_magnetic, result.magnetic_invariance_residual);
  worst_curl_gradient = std::max(
      worst_curl_gradient, result.curl_gradient_residual);
  worst_source_variation = std::max(worst_source_variation, variation);
  check(label, current.valid && result.valid
      && current.spatial_split_residual <= gate
      && current.temporal_partition_residual <= gate
      && current.split_continuity_start_residual <= gate
      && current.split_continuity_end_residual <= gate
      && current.spatial.continuity_residual <= gate
      && current.locality_residual <= gate
      && result.gauge_endpoint_residual <= gate
      && result.electric_invariance_residual <= gate
      && result.magnetic_invariance_residual <= gate
      && result.curl_gradient_residual <= gate
      && variation <= gate);
}

}  // namespace

int main() {
  const auto slab = make_slab();
  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(chi_start, chi_end);

  for (int charge : {-1, +1}) {
    const std::string polarity = charge > 0 ? "+" : "-";
    run_arm(polarity + " static", {5, 6, 7}, {0.2, -0.3, 0.4},
            {5, 6, 7}, {0.2, -0.3, 0.4}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " +x", {5, 6, 7}, {0.1, 0.2, -0.3},
            {5, 6, 7}, {0.8, 0.2, -0.3}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " -x", {5, 6, 7}, {0.8, 0.2, -0.3},
            {5, 6, 7}, {0.1, 0.2, -0.3}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " +y", {5, 6, 7}, {0.2, -0.7, 0.1},
            {5, 6, 7}, {0.2, -0.1, 0.1}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " -y", {5, 6, 7}, {0.2, -0.1, 0.1},
            {5, 6, 7}, {0.2, -0.7, 0.1}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " +z", {5, 6, 7}, {-0.2, 0.3, 0.1},
            {5, 6, 7}, {-0.2, 0.3, 0.9}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " -z", {5, 6, 7}, {-0.2, 0.3, 0.9},
            {5, 6, 7}, {-0.2, 0.3, 0.1}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " two-axis diagonal",
            {5, 6, 7}, {-0.2, 0.15, 0.3},
            {5, 6, 7}, {0.55, -0.45, 0.3}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " three-axis diagonal",
            {5, 6, 7}, {0.2, -0.3, 0.4},
            {5, 6, 7}, {0.75, 0.1, -0.2}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " integer-plane crossing",
            {5, 6, 7}, {0.8, 0.2, -0.3},
            {6, 6, 7}, {0.25, -0.4, 0.2}, charge,
            slab, chi_start, chi_end);
    run_arm(polarity + " periodic-boundary crossing",
            {16, 8, 9}, {0.9, 0.25, -0.5},
            {0, 8, 9}, {0.1, 0.25, -0.5}, charge,
            slab, chi_start, chi_end);
  }

  const auto base = ftd::eft::make_spacetime_worldline_current(
      L, {4, 5, 6}, {0.2, -0.3, 0.4},
      {4, 5, 6}, {0.75, 0.1, -0.2}, +1, time_scale);
  const ftd::Coord shift{3, 2, -1};
  const auto translated = ftd::eft::make_spacetime_worldline_current(
      L, {4 + shift.x, 5 + shift.y, 6 + shift.z},
      {0.2, -0.3, 0.4},
      {4 + shift.x, 5 + shift.y, 6 + shift.z},
      {0.75, 0.1, -0.2}, +1, time_scale);
  double translation_residual = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int source = base.index(x, y, z);
        const int target = translated.index(
            x + shift.x, y + shift.y, z + shift.z);
        translation_residual = std::max({translation_residual,
            std::abs(base.temporal_charge[static_cast<std::size_t>(source)]
                - translated.temporal_charge[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_start.x[static_cast<std::size_t>(source)]
                - translated.spatial_start.x[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_start.y[static_cast<std::size_t>(source)]
                - translated.spatial_start.y[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_start.z[static_cast<std::size_t>(source)]
                - translated.spatial_start.z[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_end.x[static_cast<std::size_t>(source)]
                - translated.spatial_end.x[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_end.y[static_cast<std::size_t>(source)]
                - translated.spatial_end.y[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_end.z[static_cast<std::size_t>(source)]
                - translated.spatial_end.z[static_cast<std::size_t>(target)])});
      }
    }
  }
  check("integer-translation covariance", base.valid && translated.valid
      && translation_residual <= gate);

  const auto rotated = ftd::eft::make_spacetime_worldline_current(
      L, {5, 6, 4}, {-0.3, 0.4, 0.2},
      {5, 6, 4}, {0.1, -0.2, 0.75}, +1, time_scale);
  double rotation_residual = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int source = base.index(x, y, z);
        const int target = rotated.index(y, z, x);
        rotation_residual = std::max({rotation_residual,
            std::abs(base.temporal_charge[static_cast<std::size_t>(source)]
                - rotated.temporal_charge[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_start.x[static_cast<std::size_t>(source)]
                - rotated.spatial_start.z[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_start.y[static_cast<std::size_t>(source)]
                - rotated.spatial_start.x[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_start.z[static_cast<std::size_t>(source)]
                - rotated.spatial_start.y[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_end.x[static_cast<std::size_t>(source)]
                - rotated.spatial_end.z[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_end.y[static_cast<std::size_t>(source)]
                - rotated.spatial_end.x[static_cast<std::size_t>(target)]),
            std::abs(base.spatial_end.z[static_cast<std::size_t>(source)]
                - rotated.spatial_end.y[static_cast<std::size_t>(target)])});
      }
    }
  }
  check("proper-cubic covariance", rotated.valid
      && rotation_residual <= gate);

  check("invalid charge fails closed",
      !ftd::eft::make_spacetime_worldline_current(
          L, {1, 2, 3}, {}, {1, 2, 3}, {}, 0, time_scale).valid);
  check("invalid temporal scale fails closed",
      !ftd::eft::make_spacetime_worldline_current(
          L, {1, 2, 3}, {}, {1, 2, 3}, {}, +1, 0.0).valid);
  auto invalid_slab = slab;
  invalid_slab.Phi[0] = std::numeric_limits<double>::quiet_NaN();
  check("non-finite potential fails closed",
      !ftd::eft::evaluate_spacetime_gauge_coupling(
          base, invalid_slab, chi_start, chi_end).valid);

  std::cout.precision(17);
  std::cout << "worst_spatial_split_residual=" << worst_split << '\n'
            << "worst_temporal_partition_residual=" << worst_partition << '\n'
            << "worst_split_continuity_residual="
            << worst_split_continuity << '\n'
            << "worst_gauge_endpoint_residual=" << worst_endpoint << '\n'
            << "worst_electric_invariance_residual=" << worst_electric << '\n'
            << "worst_magnetic_invariance_residual=" << worst_magnetic << '\n'
            << "worst_curl_gradient_residual=" << worst_curl_gradient << '\n'
            << "worst_source_variation_residual=" << worst_source_variation << '\n'
            << "translation_residual=" << translation_residual << '\n'
            << "rotation_residual=" << rotation_residual << '\n'
            << "spacetime_worldline_coupling failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

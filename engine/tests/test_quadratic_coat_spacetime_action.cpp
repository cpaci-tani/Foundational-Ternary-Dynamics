/**
 * FTD-0542: spacetime current and gauge action of the quadratic coupling coat.
 */

#include "ftd/eft/quadratic_coat_spacetime_action.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double h = 0.57735026918962576451;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
int arms = 0;
double worst_split = 0.0;
double worst_partition = 0.0;
double worst_continuity = 0.0;
double worst_source_variation = 0.0;
double worst_gauge = 0.0;
double worst_electric = 0.0;
double worst_magnetic = 0.0;
double worst_curl_gradient = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double& face_component(ftd::eft::MatchedFaceFlux& field,
                       int axis,
                       std::size_t index) {
  return axis == 0 ? field.x[index]
      : (axis == 1 ? field.y[index] : field.z[index]);
}

const std::vector<double>& face_component(
    const ftd::eft::MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

ftd::eft::DualGaugePotentialSlab make_slab() {
  ftd::eft::DualGaugePotentialSlab slab(L, h);
  constexpr double pi = 3.1415926535897932384626433832795;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(slab.index(x, y, z));
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        slab.A_start.x[i] = 0.021 * std::sin(px + py)
            + 0.013 * std::cos(pz);
        slab.A_start.y[i] = -0.017 * std::cos(py + pz)
            + 0.009 * std::sin(px);
        slab.A_start.z[i] = 0.015 * std::sin(pz + px)
            - 0.011 * std::cos(py);
        slab.A_end.x[i] = slab.A_start.x[i]
            + 0.007 * std::cos(py - pz);
        slab.A_end.y[i] = slab.A_start.y[i]
            - 0.006 * std::sin(pz - px);
        slab.A_end.z[i] = slab.A_start.z[i]
            + 0.005 * std::cos(px - py);
        slab.Phi[i] = 0.019 * std::sin(px + 2.0 * py - pz);
      }
    }
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

double source_variation_residual(
    const ftd::eft::QuadraticCoatSpacetimeCurrent& current,
    const ftd::eft::DualGaugePotentialSlab& slab) {
  const double base = ftd::eft::quadratic_coat_interaction_action(
      current, slab, coupling);
  if (!std::isfinite(base)) return INFINITY;
  constexpr double epsilon = 0.125;
  double residual = 0.0;
  bool found_start = false;
  bool found_end = false;
  bool found_temporal = false;
  for (int axis = 0; axis < 3 && (!found_start || !found_end); ++axis) {
    const auto& start_values = face_component(current.spatial_start, axis);
    const auto& end_values = face_component(current.spatial_end, axis);
    for (std::size_t i = 0; i < start_values.size(); ++i) {
      if (!found_start && start_values[i] != 0.0) {
        auto varied = slab;
        face_component(varied.A_start, axis, i) += epsilon;
        const double derivative =
            (ftd::eft::quadratic_coat_interaction_action(
                 current, varied, coupling) - base) / epsilon;
        residual = std::max(residual,
            std::abs(derivative - coupling * start_values[i]));
        found_start = true;
      }
      if (!found_end && end_values[i] != 0.0) {
        auto varied = slab;
        face_component(varied.A_end, axis, i) += epsilon;
        const double derivative =
            (ftd::eft::quadratic_coat_interaction_action(
                 current, varied, coupling) - base) / epsilon;
        residual = std::max(residual,
            std::abs(derivative - coupling * end_values[i]));
        found_end = true;
      }
    }
  }
  for (std::size_t i = 0; i < current.temporal_charge.size(); ++i) {
    if (current.temporal_charge[i] == 0.0) continue;
    auto varied = slab;
    varied.Phi[i] += epsilon;
    const double derivative =
        (ftd::eft::quadratic_coat_interaction_action(
             current, varied, coupling) - base) / epsilon;
    residual = std::max(residual, std::abs(derivative
        + coupling * h * current.temporal_charge[i]));
    found_temporal = true;
    break;
  }
  if (!found_temporal) return INFINITY;
  // A stationary path has no spatial current, so only the temporal variation
  // is required there.
  if (current.spatial.current_support == 0) return residual;
  return found_start && found_end ? residual : INFINITY;
}

void run_arm(const std::string& label,
             const ftd::Vec3& start,
             const ftd::Vec3& end,
             int charge,
             const ftd::eft::DualGaugePotentialSlab& slab,
             const std::vector<double>& chi_start,
             const std::vector<double>& chi_end) {
  ++arms;
  const auto current = ftd::eft::make_quadratic_coat_spacetime_current(
      L, start, end, charge, h);
  const auto action = ftd::eft::evaluate_quadratic_coat_gauge_action(
      current, slab, chi_start, chi_end, coupling);
  const double source = source_variation_residual(current, slab);
  worst_split = std::max(worst_split, current.spatial_split_residual);
  worst_partition = std::max(
      worst_partition, current.temporal_partition_residual);
  worst_continuity = std::max({worst_continuity,
      current.split_continuity_start_residual,
      current.split_continuity_end_residual});
  worst_source_variation = std::max(worst_source_variation, source);
  worst_gauge = std::max(worst_gauge, action.gauge_endpoint_residual);
  worst_electric = std::max(
      worst_electric, action.electric_invariance_residual);
  worst_magnetic = std::max(
      worst_magnetic, action.magnetic_invariance_residual);
  worst_curl_gradient = std::max(
      worst_curl_gradient, action.curl_gradient_residual);
  check(label, current.valid && action.valid
      && current.spatial.valid
      && current.spatial_split_residual <= gate
      && current.temporal_partition_residual <= gate
      && current.split_continuity_start_residual <= gate
      && current.split_continuity_end_residual <= gate
      && current.locality_residual <= gate
      && current.causal_excess <= gate
      && source <= gate
      && action.gauge_endpoint_residual <= gate
      && action.electric_invariance_residual <= gate
      && action.magnetic_invariance_residual <= gate
      && action.curl_gradient_residual <= gate);
}

double translated_residual(
    const ftd::eft::QuadraticCoatSpacetimeCurrent& base,
    const ftd::eft::QuadraticCoatSpacetimeCurrent& translated,
    const ftd::Coord& shift) {
  double residual = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto source = static_cast<std::size_t>(base.index(x, y, z));
        const auto target = static_cast<std::size_t>(translated.index(
            x + shift.x, y + shift.y, z + shift.z));
        residual = std::max(residual, std::abs(
            base.temporal_charge[source] - translated.temporal_charge[target]));
        for (int axis = 0; axis < 3; ++axis) {
          residual = std::max({residual,
              std::abs(face_component(base.spatial_start, axis)[source]
                  - face_component(translated.spatial_start, axis)[target]),
              std::abs(face_component(base.spatial_end, axis)[source]
                  - face_component(translated.spatial_end, axis)[target])});
        }
      }
    }
  }
  return residual;
}

}  // namespace

int main() {
  const auto slab = make_slab();
  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(slab, chi_start, chi_end);

  const std::vector<std::pair<ftd::Vec3, ftd::Vec3>> paths{
      {{5.2, 6.7, 7.4}, {5.2, 6.7, 7.4}},
      {{5.1, 6.2, 7.7}, {5.8, 6.2, 7.7}},
      {{5.8, 6.2, 7.7}, {5.1, 6.2, 7.7}},
      {{5.2, 6.1, 7.3}, {5.2, 6.8, 7.3}},
      {{5.2, 6.8, 7.3}, {5.2, 6.1, 7.3}},
      {{5.2, 6.3, 7.1}, {5.2, 6.3, 7.8}},
      {{5.2, 6.3, 7.8}, {5.2, 6.3, 7.1}},
      {{5.2, 6.15, 7.3}, {5.75, 5.55, 7.3}},
      {{5.2, 6.7, 7.4}, {5.75, 7.1, 6.8}},
      {{5.8, 6.2, 7.7}, {6.25, 5.6, 8.2}},
      {{5.25, 6.2, 7.7}, {5.75, 6.2, 7.7}},
      {{16.9, 8.25, 9.5}, {0.1, 8.25, 9.5}}};
  for (int charge : {-1, +1}) {
    for (std::size_t i = 0; i < paths.size(); ++i) {
      run_arm(std::string(charge > 0 ? "+" : "-")
              + " arm " + std::to_string(i),
              paths[i].first, paths[i].second, charge,
              slab, chi_start, chi_end);
    }
  }

  const auto base = ftd::eft::make_quadratic_coat_spacetime_current(
      L, {4.2, 5.7, 6.4}, {4.75, 6.1, 5.8}, +1, h);
  const ftd::Coord shift{3, 2, -1};
  const auto translated = ftd::eft::make_quadratic_coat_spacetime_current(
      L, {7.2, 7.7, 5.4}, {7.75, 8.1, 4.8}, +1, h);
  const double translation = translated_residual(base, translated, shift);
  check("integer-translation covariance",
      base.valid && translated.valid && translation <= gate);

  const auto rotated = ftd::eft::make_quadratic_coat_spacetime_current(
      L, {5.7, 6.4, 4.2}, {6.1, 5.8, 4.75}, +1, h);
  double rotation = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto source = static_cast<std::size_t>(base.index(x, y, z));
        const auto target = static_cast<std::size_t>(rotated.index(y, z, x));
        rotation = std::max(rotation, std::abs(
            base.temporal_charge[source] - rotated.temporal_charge[target]));
        rotation = std::max({rotation,
            std::abs(base.spatial_start.x[source]
                - rotated.spatial_start.z[target]),
            std::abs(base.spatial_start.y[source]
                - rotated.spatial_start.x[target]),
            std::abs(base.spatial_start.z[source]
                - rotated.spatial_start.y[target]),
            std::abs(base.spatial_end.x[source]
                - rotated.spatial_end.z[target]),
            std::abs(base.spatial_end.y[source]
                - rotated.spatial_end.x[target]),
            std::abs(base.spatial_end.z[source]
                - rotated.spatial_end.y[target])});
      }
    }
  }
  check("proper-cubic covariance", rotated.valid && rotation <= gate);

  const auto reverse = ftd::eft::make_quadratic_coat_spacetime_current(
      L, {4.75, 6.1, 5.8}, {4.2, 5.7, 6.4}, +1, h);
  double reversal = 0.0;
  for (std::size_t i = 0; i < base.temporal_charge.size(); ++i) {
    reversal = std::max(reversal, std::abs(
        base.temporal_charge[i] - reverse.temporal_charge[i]));
    for (int axis = 0; axis < 3; ++axis) {
      reversal = std::max({reversal,
          std::abs(face_component(base.spatial_start, axis)[i]
              + face_component(reverse.spatial_end, axis)[i]),
          std::abs(face_component(base.spatial_end, axis)[i]
              + face_component(reverse.spatial_start, axis)[i])});
    }
  }
  check("path-reversal split", reverse.valid && reversal <= gate);

  check("invalid charge fails closed",
      !ftd::eft::make_quadratic_coat_spacetime_current(
          L, {1.2, 2.3, 3.4}, {1.3, 2.4, 3.5}, 0, h).valid);
  check("invalid duration fails closed",
      !ftd::eft::make_quadratic_coat_spacetime_current(
          L, {1.2, 2.3, 3.4}, {1.3, 2.4, 3.5}, +1, 0.0).valid);
  check("over-causal segment fails closed",
      !ftd::eft::make_quadratic_coat_spacetime_current(
          L, {1.2, 2.3, 3.4}, {2.3, 2.4, 3.5}, +1, h).valid);
  auto invalid_slab = slab;
  invalid_slab.Phi[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite field fails closed",
      !ftd::eft::evaluate_quadratic_coat_gauge_action(
          base, invalid_slab, chi_start, chi_end, coupling).valid);

  std::cout.precision(17);
  std::cout << "arms=" << arms << '\n'
            << "worst_spatial_split_residual=" << worst_split << '\n'
            << "worst_temporal_partition_residual=" << worst_partition << '\n'
            << "worst_split_continuity_residual=" << worst_continuity << '\n'
            << "worst_source_variation_residual=" << worst_source_variation << '\n'
            << "worst_gauge_endpoint_residual=" << worst_gauge << '\n'
            << "worst_electric_invariance_residual=" << worst_electric << '\n'
            << "worst_magnetic_invariance_residual=" << worst_magnetic << '\n'
            << "worst_curl_gradient_residual=" << worst_curl_gradient << '\n'
            << "translation_residual=" << translation << '\n'
            << "rotation_residual=" << rotation << '\n'
            << "reversal_residual=" << reversal << '\n'
            << "quadratic_coat_spacetime_action failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

#include "ftd/eft/matched_regional_energy_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {
namespace {

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

bool finite(const MatchedFaceFlux& field) {
  return finite(field.x) && finite(field.y) && finite(field.z);
}

bool finite(const MatchedEdgeField& field) {
  return finite(field.x) && finite(field.y) && finite(field.z);
}

bool valid_size(const MatchedFaceFlux& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

bool valid_size(const MatchedEdgeField& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

double periodic_distance(double coordinate, double origin, int L) {
  double delta = coordinate - origin;
  const double half = 0.5 * static_cast<double>(L);
  while (delta > half) delta -= L;
  while (delta < -half) delta += L;
  return std::abs(delta);
}

bool inside(const Vec3& position,
            const Vec3& center,
            double radius,
            int L) {
  return std::max({periodic_distance(position.x, center.x, L),
                   periodic_distance(position.y, center.y, L),
                   periodic_distance(position.z, center.z, L)})
      <= radius;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

long double dot(const MatchedFaceFlux& lhs,
                const MatchedFaceFlux& rhs) {
  return dot(lhs.x, rhs.x) + dot(lhs.y, rhs.y) + dot(lhs.z, rhs.z);
}

long double dot(const MatchedEdgeField& lhs,
                const MatchedEdgeField& rhs) {
  return dot(lhs.x, rhs.x) + dot(lhs.y, rhs.y) + dot(lhs.z, rhs.z);
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  return result;
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

double max_difference(const MatchedEdgeField& lhs,
                      const MatchedEdgeField& rhs) {
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

MatchedFaceFlux masked(const MatchedFaceFlux& field,
                       const Vec3& center,
                       double radius,
                       bool select_inside) {
  MatchedFaceFlux result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto i = static_cast<std::size_t>(field.index(x, y, z));
        const bool take_x = inside({x + 0.5, static_cast<double>(y),
                                    static_cast<double>(z)},
                                   center, radius, field.L) == select_inside;
        const bool take_y = inside({static_cast<double>(x), y + 0.5,
                                    static_cast<double>(z)},
                                   center, radius, field.L) == select_inside;
        const bool take_z = inside({static_cast<double>(x),
                                    static_cast<double>(y), z + 0.5},
                                   center, radius, field.L) == select_inside;
        if (take_x) result.x[i] = field.x[i];
        if (take_y) result.y[i] = field.y[i];
        if (take_z) result.z[i] = field.z[i];
      }
    }
  }
  return result;
}

MatchedEdgeField masked(const MatchedEdgeField& field,
                        const Vec3& center,
                        double radius,
                        bool select_inside) {
  MatchedEdgeField result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto i = static_cast<std::size_t>(field.index(x, y, z));
        const bool take_x = inside({static_cast<double>(x), y + 0.5, z + 0.5},
                                   center, radius, field.L) == select_inside;
        const bool take_y = inside({x + 0.5, static_cast<double>(y), z + 0.5},
                                   center, radius, field.L) == select_inside;
        const bool take_z = inside({x + 0.5, y + 0.5, static_cast<double>(z)},
                                   center, radius, field.L) == select_inside;
        if (take_x) result.x[i] = field.x[i];
        if (take_y) result.y[i] = field.y[i];
        if (take_z) result.z[i] = field.z[i];
      }
    }
  }
  return result;
}

double regional_energy(const MatchedFaceFlux& electric,
                       const MatchedEdgeField& magnetic,
                       double lambda,
                       const Vec3& center,
                       double radius,
                       bool select_inside) {
  const auto electric_selected = masked(
      electric, center, radius, select_inside);
  const auto magnetic_selected = masked(
      magnetic, center, radius, select_inside);
  const auto curl_full = matched_curl_adjoint(electric);
  const auto curl_selected = matched_curl_adjoint(electric_selected);
  return static_cast<double>(
      0.5L * dot(electric_selected, electric_selected)
      + 0.5L * dot(magnetic_selected, magnetic_selected)
      - 0.25L * lambda
          * (dot(magnetic_selected, curl_full)
             + dot(magnetic, curl_selected)));
}

double partition_residual(const MatchedFaceFlux& electric,
                          const MatchedEdgeField& magnetic,
                          double lambda,
                          const Vec3& center,
                          double radius) {
  const double inside_energy = regional_energy(
      electric, magnetic, lambda, center, radius, true);
  const double outside_energy = regional_energy(
      electric, magnetic, lambda, center, radius, false);
  const double total = matched_modified_energy(electric, magnetic, lambda);
  return std::abs(inside_energy + outside_energy - total);
}

}  // namespace

MatchedRegionalEnergySnapshot measure_matched_regional_energy(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    double lambda,
    const Vec3& center,
    double chebyshev_radius,
    double tolerance) {
  MatchedRegionalEnergySnapshot result;
  result.L = electric.L;
  result.center = center;
  result.chebyshev_radius = chebyshev_radius;
  result.lambda = lambda;
  if (result.L < 5 || !(chebyshev_radius >= 0.0)
      || !std::isfinite(chebyshev_radius) || !(lambda > 0.0)
      || !std::isfinite(lambda) || !(tolerance > 0.0)
      || !std::isfinite(center.x) || !std::isfinite(center.y)
      || !std::isfinite(center.z)
      || !valid_size(electric,result.L)
      || !valid_size(magnetic,result.L)
      || !finite(electric) || !finite(magnetic)) return result;
  result.total_energy = matched_modified_energy(electric,magnetic,lambda);
  result.inside_energy = regional_energy(
      electric,magnetic,lambda,center,chebyshev_radius,true);
  result.outside_energy = regional_energy(
      electric,magnetic,lambda,center,chebyshev_radius,false);
  result.partition_residual = std::abs(
      result.inside_energy+result.outside_energy-result.total_energy);
  result.valid = result.partition_residual <= tolerance
      && std::isfinite(result.total_energy)
      && std::isfinite(result.inside_energy)
      && std::isfinite(result.outside_energy);
  return result;
}

MatchedRegionalEnergyTransportResult
evaluate_matched_regional_energy_transport(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const MatchedFaceFlux& electric_pre_current,
    const MatchedEdgeField& magnetic_after,
    const MatchedFaceFlux& electric_after,
    double lambda,
    const Vec3& center,
    double chebyshev_radius,
    double tolerance) {
  MatchedRegionalEnergyTransportResult result;
  result.L = electric_before.L;
  result.center = center;
  result.chebyshev_radius = chebyshev_radius;
  result.lambda = lambda;
  if (result.L < 5 || !(chebyshev_radius >= 0.0)
      || !std::isfinite(chebyshev_radius) || !std::isfinite(lambda)
      || !std::isfinite(center.x) || !std::isfinite(center.y)
      || !std::isfinite(center.z)
      || !(lambda > 0.0) || !(tolerance > 0.0)
      || !valid_size(electric_before, result.L)
      || !valid_size(magnetic_before, result.L)
      || !valid_size(electric_pre_current, result.L)
      || !valid_size(magnetic_after, result.L)
      || !valid_size(electric_after, result.L)
      || !finite(electric_before) || !finite(magnetic_before)
      || !finite(electric_pre_current) || !finite(magnetic_after)
      || !finite(electric_after)) {
    return result;
  }

  auto expected_magnetic = magnetic_before;
  const auto curl_adjoint = matched_curl_adjoint(electric_before);
  for (std::size_t i = 0; i < expected_magnetic.x.size(); ++i) {
    expected_magnetic.x[i] -= lambda * curl_adjoint.x[i];
    expected_magnetic.y[i] -= lambda * curl_adjoint.y[i];
    expected_magnetic.z[i] -= lambda * curl_adjoint.z[i];
  }
  result.magnetic_update_residual = max_difference(
      expected_magnetic, magnetic_after);

  auto expected_pre_current = electric_before;
  const auto curl = matched_curl(magnetic_after);
  for (std::size_t i = 0; i < expected_pre_current.x.size(); ++i) {
    expected_pre_current.x[i] += lambda * curl.x[i];
    expected_pre_current.y[i] += lambda * curl.y[i];
    expected_pre_current.z[i] += lambda * curl.z[i];
  }
  result.electric_pre_update_residual = max_difference(
      expected_pre_current, electric_pre_current);

  result.energy_before = regional_energy(
      electric_before, magnetic_before, lambda,
      center, chebyshev_radius, true);
  result.energy_pre_current = regional_energy(
      electric_pre_current, magnetic_after, lambda,
      center, chebyshev_radius, true);
  result.energy_after = regional_energy(
      electric_after, magnetic_after, lambda,
      center, chebyshev_radius, true);
  result.boundary_transport_into =
      result.energy_pre_current - result.energy_before;
  result.source_exchange_into_field =
      result.energy_after - result.energy_pre_current;
  result.energy_change = result.energy_after - result.energy_before;
  result.regional_ledger_residual = std::abs(
      result.energy_change - result.boundary_transport_into
      - result.source_exchange_into_field);

  result.global_source_free_residual = std::abs(
      matched_modified_energy(electric_pre_current, magnetic_after, lambda)
      - matched_modified_energy(electric_before, magnetic_before, lambda));
  result.partition_residual = std::max({
      partition_residual(electric_before, magnetic_before, lambda,
                         center, chebyshev_radius),
      partition_residual(electric_pre_current, magnetic_after, lambda,
                         center, chebyshev_radius),
      partition_residual(electric_after, magnetic_after, lambda,
                         center, chebyshev_radius)});

  result.valid = result.magnetic_update_residual <= tolerance
      && result.electric_pre_update_residual <= tolerance
      && result.global_source_free_residual <= tolerance
      && result.partition_residual <= tolerance
      && result.regional_ledger_residual <= tolerance
      && std::isfinite(result.energy_before)
      && std::isfinite(result.energy_pre_current)
      && std::isfinite(result.energy_after);
  return result;
}

}  // namespace ftd::eft

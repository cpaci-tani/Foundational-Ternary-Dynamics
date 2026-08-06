#include "ftd/eft/batched_regional_energy_profile.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

namespace ftd::eft {
namespace {

bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

template <typename Field>
bool valid_field(const Field& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count
      && finite(field.x) && finite(field.y) && finite(field.z);
}

int periodic_abs_doubled(int coordinate2, int origin2, int L) {
  const int period = 2 * L;
  int delta = (coordinate2 - origin2) % period;
  if (delta < 0) delta += period;
  return std::min(delta, period - delta);
}

int radius2(int x2, int y2, int z2,
            int ox2, int oy2, int oz2, int L) {
  return std::max({periodic_abs_doubled(x2, ox2, L),
                   periodic_abs_doubled(y2, oy2, L),
                   periodic_abs_doubled(z2, oz2, L)});
}

double max_difference(const std::vector<double>& left,
                      const std::vector<double>& right) {
  double result = 0.0;
  for (std::size_t index = 0; index < left.size(); ++index)
    result = std::max(result, std::abs(left[index] - right[index]));
  return result;
}

template <typename Field>
double max_difference(const Field& left, const Field& right) {
  return std::max({max_difference(left.x, right.x),
                   max_difference(left.y, right.y),
                   max_difference(left.z, right.z)});
}

struct EnergyProfile {
  std::vector<double> inside;
  double total_local = 0.0;
  double total_exact = 0.0;
  double closure_residual = 0.0;
};

EnergyProfile energy_profile(const MatchedFaceFlux& electric,
                             const MatchedEdgeField& magnetic,
                             double lambda,
                             const Vec3& center,
                             const std::vector<int>& radii) {
  EnergyProfile result;
  result.inside.assign(radii.size(), 0.0);
  std::vector<long double> bins(
      static_cast<std::size_t>(electric.L + 1), 0.0L);
  long double local_total = 0.0L;
  const auto curl_b = matched_curl(magnetic);
  const auto curl_adjoint_e = matched_curl_adjoint(electric);
  const int L = electric.L;
  const int ox2 = static_cast<int>(2.0 * center.x);
  const int oy2 = static_cast<int>(2.0 * center.y);
  const int oz2 = static_cast<int>(2.0 * center.z);
  auto add = [&](long double contribution, int x2, int y2, int z2) {
    local_total += contribution;
    const int component_radius2 = radius2(
        x2, y2, z2, ox2, oy2, oz2, L);
    bins[static_cast<std::size_t>(component_radius2)] += contribution;
  };
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(electric.index(x, y, z));
        add(0.5L * electric.x[index] * electric.x[index]
                - 0.25L * lambda * electric.x[index] * curl_b.x[index],
            2 * x + 1, 2 * y, 2 * z);
        add(0.5L * electric.y[index] * electric.y[index]
                - 0.25L * lambda * electric.y[index] * curl_b.y[index],
            2 * x, 2 * y + 1, 2 * z);
        add(0.5L * electric.z[index] * electric.z[index]
                - 0.25L * lambda * electric.z[index] * curl_b.z[index],
            2 * x, 2 * y, 2 * z + 1);
        add(0.5L * magnetic.x[index] * magnetic.x[index]
                - 0.25L * lambda * magnetic.x[index]
                    * curl_adjoint_e.x[index],
            2 * x, 2 * y + 1, 2 * z + 1);
        add(0.5L * magnetic.y[index] * magnetic.y[index]
                - 0.25L * lambda * magnetic.y[index]
                    * curl_adjoint_e.y[index],
            2 * x + 1, 2 * y, 2 * z + 1);
        add(0.5L * magnetic.z[index] * magnetic.z[index]
                - 0.25L * lambda * magnetic.z[index]
                    * curl_adjoint_e.z[index],
            2 * x + 1, 2 * y + 1, 2 * z);
      }
    }
  }
  std::vector<long double> cumulative(bins.size(), 0.0L);
  long double running = 0.0L;
  for (std::size_t index = 0; index < bins.size(); ++index) {
    running += bins[index];
    cumulative[index] = running;
  }
  for (std::size_t radius = 0; radius < radii.size(); ++radius) {
    const int bin = std::min(2 * radii[radius], L);
    result.inside[radius] = static_cast<double>(
        cumulative[static_cast<std::size_t>(bin)]);
  }
  result.total_local = static_cast<double>(local_total);
  result.total_exact = matched_modified_energy(electric, magnetic, lambda);
  result.closure_residual = std::abs(
      result.total_local - result.total_exact);
  return result;
}

}  // namespace

BatchedRegionalEnergyProfile evaluate_batched_regional_energy_profile(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const MatchedFaceFlux& electric_pre_current,
    const MatchedEdgeField& magnetic_after,
    const MatchedFaceFlux& electric_after,
    double lambda,
    const Vec3& integer_center,
    const std::vector<int>& chebyshev_radii,
    double tolerance) {
  BatchedRegionalEnergyProfile result;
  result.L = electric_before.L;
  result.center = integer_center;
  result.lambda = lambda;
  const int L = result.L;
  const bool ordered = std::adjacent_find(
      chebyshev_radii.begin(), chebyshev_radii.end(),
      [](int left, int right) { return left >= right; })
      == chebyshev_radii.end();
  if (L < 5 || L % 2 == 0 || chebyshev_radii.empty() || !ordered
      || chebyshev_radii.front() < 0
      || !std::isfinite(lambda) || !(lambda > 0.0)
      || !std::isfinite(tolerance) || !(tolerance > 0.0)
      || !std::isfinite(integer_center.x)
      || !std::isfinite(integer_center.y)
      || !std::isfinite(integer_center.z)
      || integer_center.x != std::round(integer_center.x)
      || integer_center.y != std::round(integer_center.y)
      || integer_center.z != std::round(integer_center.z)
      || !valid_field(electric_before, L)
      || !valid_field(magnetic_before, L)
      || !valid_field(electric_pre_current, L)
      || !valid_field(magnetic_after, L)
      || !valid_field(electric_after, L))
    return result;

  auto expected_magnetic = magnetic_before;
  const auto curl_adjoint_before = matched_curl_adjoint(electric_before);
  for (std::size_t index = 0; index < expected_magnetic.x.size(); ++index) {
    expected_magnetic.x[index] -= lambda * curl_adjoint_before.x[index];
    expected_magnetic.y[index] -= lambda * curl_adjoint_before.y[index];
    expected_magnetic.z[index] -= lambda * curl_adjoint_before.z[index];
  }
  const double magnetic_residual = max_difference(
      expected_magnetic, magnetic_after);
  auto expected_pre_current = electric_before;
  const auto curl_after = matched_curl(magnetic_after);
  for (std::size_t index = 0; index < expected_pre_current.x.size(); ++index) {
    expected_pre_current.x[index] += lambda * curl_after.x[index];
    expected_pre_current.y[index] += lambda * curl_after.y[index];
    expected_pre_current.z[index] += lambda * curl_after.z[index];
  }
  const double electric_residual = max_difference(
      expected_pre_current, electric_pre_current);

  const auto before = energy_profile(
      electric_before, magnetic_before, lambda,
      integer_center, chebyshev_radii);
  const auto pre = energy_profile(
      electric_pre_current, magnetic_after, lambda,
      integer_center, chebyshev_radii);
  const auto after = energy_profile(
      electric_after, magnetic_after, lambda,
      integer_center, chebyshev_radii);
  const double global_residual = std::abs(
      matched_modified_energy(electric_pre_current, magnetic_after, lambda)
      - matched_modified_energy(electric_before, magnetic_before, lambda));
  const double partition_residual = std::max({
      before.closure_residual, pre.closure_residual, after.closure_residual});
  result.energy_before = before.total_exact;
  result.energy_pre_current = pre.total_exact;
  result.energy_after = after.total_exact;

  result.regions.resize(chebyshev_radii.size());
  result.valid = true;
  for (std::size_t radius = 0; radius < chebyshev_radii.size(); ++radius) {
    auto& record = result.regions[radius];
    record.L = L;
    record.center = integer_center;
    record.chebyshev_radius = chebyshev_radii[radius];
    record.lambda = lambda;
    record.energy_before = before.inside[radius];
    record.energy_pre_current = pre.inside[radius];
    record.energy_after = after.inside[radius];
    record.boundary_transport_into =
        record.energy_pre_current - record.energy_before;
    record.source_exchange_into_field =
        record.energy_after - record.energy_pre_current;
    record.energy_change = record.energy_after - record.energy_before;
    record.magnetic_update_residual = magnetic_residual;
    record.electric_pre_update_residual = electric_residual;
    record.global_source_free_residual = global_residual;
    record.partition_residual = partition_residual;
    record.regional_ledger_residual = std::abs(
        record.energy_change - record.boundary_transport_into
        - record.source_exchange_into_field);
    record.valid = magnetic_residual <= tolerance
        && electric_residual <= tolerance && global_residual <= tolerance
        && partition_residual <= tolerance
        && record.regional_ledger_residual <= tolerance
        && std::isfinite(record.energy_before)
        && std::isfinite(record.energy_pre_current)
        && std::isfinite(record.energy_after);
    result.valid = result.valid && record.valid;
  }
  result.maximum_scalar_equivalence_residual = partition_residual;
  return result;
}

}  // namespace ftd::eft

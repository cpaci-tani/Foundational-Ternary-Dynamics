#include "ftd/eft/component_aware_radial_field_profile.h"

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

bool valid_size(const MatchedFaceFlux& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count
      && finite(field.x) && finite(field.y) && finite(field.z);
}

bool valid_size(const MatchedEdgeField& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count
      && finite(field.x) && finite(field.y) && finite(field.z);
}

bool is_integer(double value) {
  return std::isfinite(value) && value == std::round(value);
}

int periodic_abs_doubled(int coordinate2, int origin2, int L) {
  const int period = 2 * L;
  int delta = (coordinate2 - origin2) % period;
  if (delta < 0) delta += period;
  return std::min(delta, period - delta);
}

int doubled_radius(int x2, int y2, int z2,
                   int ox2, int oy2, int oz2, int L) {
  return std::max({periodic_abs_doubled(x2, ox2, L),
                   periodic_abs_doubled(y2, oy2, L),
                   periodic_abs_doubled(z2, oz2, L)});
}

int quantile_bin(const std::vector<double>& cumulative,
                 double total,
                 double quantile) {
  if (!(total > 0.0)) return 0;
  const double threshold = quantile * total;
  for (std::size_t index = 0; index < cumulative.size(); ++index)
    if (cumulative[index] >= threshold) return static_cast<int>(index);
  return cumulative.empty() ? 0 : static_cast<int>(cumulative.size() - 1);
}

}  // namespace

ComponentAwareRadialFieldProfile observe_component_aware_radial_field_profile(
    const MatchedFaceFlux& reference_electric,
    const MatchedEdgeField& reference_magnetic,
    const MatchedFaceFlux& candidate_electric,
    const MatchedEdgeField& candidate_magnetic,
    const Vec3& integer_origin,
    double field_energy_scale,
    double wave_speed,
    double tolerance) {
  ComponentAwareRadialFieldProfile result;
  result.L = reference_electric.L;
  result.origin = integer_origin;
  result.field_energy_scale = field_energy_scale;
  result.wave_speed = wave_speed;
  const int L = result.L;
  if (L <= 0 || L % 2 == 0 || !(field_energy_scale > 0.0)
      || !(wave_speed > 0.0) || !(tolerance > 0.0)
      || !std::isfinite(field_energy_scale) || !std::isfinite(wave_speed)
      || !is_integer(integer_origin.x) || !is_integer(integer_origin.y)
      || !is_integer(integer_origin.z)
      || !valid_size(reference_electric, L)
      || !valid_size(reference_magnetic, L)
      || !valid_size(candidate_electric, L)
      || !valid_size(candidate_magnetic, L))
    return result;

  std::vector<long double> bins(static_cast<std::size_t>(L + 1), 0.0L);
  const int ox2 = static_cast<int>(2.0 * integer_origin.x);
  const int oy2 = static_cast<int>(2.0 * integer_origin.y);
  const int oz2 = static_cast<int>(2.0 * integer_origin.z);
  const long double electric_scale = 0.5L * field_energy_scale;
  const long double magnetic_scale = electric_scale * wave_speed * wave_speed;
  long double direct = 0.0L;
  long double first_moment = 0.0L;
  long double second_moment = 0.0L;

  auto add = [&](double delta, long double scale,
                 int x2, int y2, int z2) {
    const int radius2 = doubled_radius(
        x2, y2, z2, ox2, oy2, oz2, L);
    const long double weight = scale * static_cast<long double>(delta) * delta;
    bins[static_cast<std::size_t>(radius2)] += weight;
    direct += weight;
    const long double radius = 0.5L * radius2;
    first_moment += weight * radius;
    second_moment += weight * radius * radius;
  };

  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto index = static_cast<std::size_t>(
            reference_electric.index(x, y, z));
        add(candidate_electric.x[index] - reference_electric.x[index],
            electric_scale, 2 * x + 1, 2 * y, 2 * z);
        add(candidate_electric.y[index] - reference_electric.y[index],
            electric_scale, 2 * x, 2 * y + 1, 2 * z);
        add(candidate_electric.z[index] - reference_electric.z[index],
            electric_scale, 2 * x, 2 * y, 2 * z + 1);
        add(candidate_magnetic.x[index] - reference_magnetic.x[index],
            magnetic_scale, 2 * x, 2 * y + 1, 2 * z + 1);
        add(candidate_magnetic.y[index] - reference_magnetic.y[index],
            magnetic_scale, 2 * x + 1, 2 * y, 2 * z + 1);
        add(candidate_magnetic.z[index] - reference_magnetic.z[index],
            magnetic_scale, 2 * x + 1, 2 * y + 1, 2 * z);
      }
    }
  }

  result.shell_norm_by_doubled_radius.resize(bins.size());
  result.cumulative_norm_by_doubled_radius.resize(bins.size());
  long double binned = 0.0L;
  double previous = 0.0;
  for (std::size_t index = 0; index < bins.size(); ++index) {
    result.shell_norm_by_doubled_radius[index] =
        static_cast<double>(bins[index]);
    binned += bins[index];
    const double cumulative = static_cast<double>(binned);
    result.cumulative_norm_by_doubled_radius[index] = cumulative;
    result.monotonicity_residual = std::max(
        result.monotonicity_residual, previous - cumulative);
    previous = cumulative;
  }
  result.direct_total_norm = static_cast<double>(direct);
  result.total_norm = static_cast<double>(binned);
  result.partition_residual = std::abs(
      result.direct_total_norm - result.total_norm);
  result.cumulative_residual = result.cumulative_norm_by_doubled_radius.empty()
      ? std::abs(result.total_norm)
      : std::abs(result.cumulative_norm_by_doubled_radius.back()
                 - result.total_norm);
  result.zero_profile = result.total_norm == 0.0;
  if (result.total_norm > 0.0) {
    result.mean_radius = static_cast<double>(first_moment / direct);
    result.rms_radius = std::sqrt(
        static_cast<double>(second_moment / direct));
    result.doubled_radius_50 = quantile_bin(
        result.cumulative_norm_by_doubled_radius, result.total_norm, 0.50);
    result.doubled_radius_90 = quantile_bin(
        result.cumulative_norm_by_doubled_radius, result.total_norm, 0.90);
    result.doubled_radius_99 = quantile_bin(
        result.cumulative_norm_by_doubled_radius, result.total_norm, 0.99);
  }
  result.finite = std::isfinite(result.total_norm)
      && std::isfinite(result.direct_total_norm)
      && std::isfinite(result.mean_radius)
      && std::isfinite(result.rms_radius)
      && finite(result.shell_norm_by_doubled_radius)
      && finite(result.cumulative_norm_by_doubled_radius);
  const bool nonnegative = std::all_of(
      result.shell_norm_by_doubled_radius.begin(),
      result.shell_norm_by_doubled_radius.end(),
      [](double value) { return value >= 0.0; });
  result.valid = result.finite && nonnegative
      && result.partition_residual <= tolerance
      && result.cumulative_residual <= tolerance
      && result.monotonicity_residual <= tolerance;
  return result;
}

}  // namespace ftd::eft

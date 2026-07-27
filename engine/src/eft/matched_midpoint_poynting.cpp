#include "ftd/eft/matched_midpoint_poynting.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

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

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value,
                double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

void add_scaled(MatchedEdgeField& target,
                const MatchedEdgeField& value,
                double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
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

double field_energy(const MatchedFaceFlux& electric,
                    const MatchedEdgeField& magnetic) {
  return static_cast<double>(0.5L * (
      dot(electric, electric) + dot(magnetic, magnetic)));
}

}  // namespace

MatchedMidpointPoyntingResult evaluate_matched_midpoint_poynting(
    const MatchedFaceFlux& electric_midpoint,
    const MatchedEdgeField& magnetic_midpoint,
    const QuadraticCoatFaceCurrent& source,
    double temporal_scale) {
  MatchedMidpointPoyntingResult result;
  result.L = electric_midpoint.L;
  result.temporal_scale = temporal_scale;
  result.current = MatchedFaceFlux(result.L);
  result.electric_midpoint = electric_midpoint;
  result.magnetic_midpoint = magnetic_midpoint;
  result.electric_before = MatchedFaceFlux(result.L);
  result.electric_after = MatchedFaceFlux(result.L);
  result.magnetic_before = MatchedEdgeField(result.L);
  result.magnetic_after = MatchedEdgeField(result.L);
  if (!source.valid || result.L < 5 || source.L != result.L
      || !(temporal_scale > 0.0) || !std::isfinite(temporal_scale)
      || !valid_size(electric_midpoint, result.L)
      || !valid_size(magnetic_midpoint, result.L)
      || !finite(electric_midpoint) || !finite(magnetic_midpoint)) {
    return result;
  }
  result.current.x = source.current_x;
  result.current.y = source.current_y;
  result.current.z = source.current_z;
  const auto curl_b = matched_curl(magnetic_midpoint);
  const auto curl_t_e = matched_curl_adjoint(electric_midpoint);
  MatchedFaceFlux delta_e = curl_b;
  for (std::size_t i = 0; i < delta_e.x.size(); ++i) {
    delta_e.x[i] = temporal_scale * delta_e.x[i] - result.current.x[i];
    delta_e.y[i] = temporal_scale * delta_e.y[i] - result.current.y[i];
    delta_e.z[i] = temporal_scale * delta_e.z[i] - result.current.z[i];
  }
  MatchedEdgeField delta_b = curl_t_e;
  for (std::size_t i = 0; i < delta_b.x.size(); ++i) {
    delta_b.x[i] *= -temporal_scale;
    delta_b.y[i] *= -temporal_scale;
    delta_b.z[i] *= -temporal_scale;
  }
  result.electric_before = electric_midpoint;
  result.electric_after = electric_midpoint;
  add_scaled(result.electric_before, delta_e, -0.5);
  add_scaled(result.electric_after, delta_e, 0.5);
  result.magnetic_before = magnetic_midpoint;
  result.magnetic_after = magnetic_midpoint;
  add_scaled(result.magnetic_before, delta_b, -0.5);
  add_scaled(result.magnetic_after, delta_b, 0.5);

  MatchedFaceFlux reconstructed_e = result.electric_before;
  add_scaled(reconstructed_e, result.electric_after, 1.0);
  for (std::size_t i = 0; i < reconstructed_e.x.size(); ++i) {
    reconstructed_e.x[i] *= 0.5;
    reconstructed_e.y[i] *= 0.5;
    reconstructed_e.z[i] *= 0.5;
  }
  MatchedEdgeField reconstructed_b = result.magnetic_before;
  add_scaled(reconstructed_b, result.magnetic_after, 1.0);
  for (std::size_t i = 0; i < reconstructed_b.x.size(); ++i) {
    reconstructed_b.x[i] *= 0.5;
    reconstructed_b.y[i] *= 0.5;
    reconstructed_b.z[i] *= 0.5;
  }
  result.electric_midpoint_residual = max_difference(
      reconstructed_e, electric_midpoint);
  result.magnetic_midpoint_residual = max_difference(
      reconstructed_b, magnetic_midpoint);

  MatchedFaceFlux ampere = result.electric_before;
  add_scaled(ampere, curl_b, temporal_scale);
  add_scaled(ampere, result.current, -1.0);
  result.ampere_residual = max_difference(ampere, result.electric_after);
  MatchedEdgeField faraday = result.magnetic_before;
  add_scaled(faraday, curl_t_e, -temporal_scale);
  result.faraday_residual = max_difference(faraday, result.magnetic_after);
  result.adjoint_residual = std::abs(static_cast<double>(
      dot(electric_midpoint, curl_b)
      - dot(curl_t_e, magnetic_midpoint)));

  result.field_energy_before = field_energy(
      result.electric_before, result.magnetic_before);
  result.field_energy_after = field_energy(
      result.electric_after, result.magnetic_after);
  result.current_work = static_cast<double>(
      dot(electric_midpoint, result.current));
  result.poynting_residual = std::abs(
      result.field_energy_after - result.field_energy_before
      + result.current_work);

  const std::size_t count = static_cast<std::size_t>(result.L)
      * result.L * result.L;
  result.rho_before.assign(count, 0.0);
  result.rho_after.assign(count, 0.0);
  for (int x = 0; x < result.L; ++x) {
    for (int y = 0; y < result.L; ++y) {
      for (int z = 0; z < result.L; ++z) {
        const auto i = static_cast<std::size_t>(
            result.electric_before.index(x, y, z));
        result.rho_before[i] = divergence_at(
            result.electric_before, x, y, z);
        result.rho_after[i] = divergence_at(
            result.electric_after, x, y, z);
        result.gauss_transport_residual = std::max(
            result.gauss_transport_residual,
            std::abs(result.rho_after[i] - result.rho_before[i]
                + quadratic_coat_current_divergence_at(source, x, y, z)));
      }
    }
  }
  result.valid = finite(result.current)
      && finite(result.electric_before) && finite(result.electric_after)
      && finite(result.magnetic_before) && finite(result.magnetic_after)
      && result.electric_midpoint_residual <= 1e-12
      && result.magnetic_midpoint_residual <= 1e-12
      && result.ampere_residual <= 1e-12
      && result.faraday_residual <= 1e-12
      && result.adjoint_residual <= 1e-12
      && result.poynting_residual <= 1e-12
      && result.gauss_transport_residual <= 1e-12;
  return result;
}

}  // namespace ftd::eft

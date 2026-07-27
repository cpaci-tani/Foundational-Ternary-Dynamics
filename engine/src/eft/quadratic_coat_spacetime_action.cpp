#include "ftd/eft/quadratic_coat_spacetime_action.h"

#include "ftd/eft/local_polarity_regularity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

std::size_t flat_index(int L, int x, int y, int z) {
  const auto side = static_cast<std::size_t>(L);
  const auto wx = static_cast<std::size_t>(wrap(x, L));
  const auto wy = static_cast<std::size_t>(wrap(y, L));
  const auto wz = static_cast<std::size_t>(wrap(z, L));
  return (wx * side + wy) * side + wz;
}

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

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

bool valid_face_size(const MatchedFaceFlux& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

double b1(double u) {
  return evaluate_local_polarity_kernel(LocalPolarityKernel::Hat, u);
}

double b2(double u) {
  return evaluate_local_polarity_kernel(
      LocalPolarityKernel::QuadraticBSpline, u);
}

std::vector<double> half_integer_breaks(const Vec3& start,
                                        const Vec3& end) {
  std::vector<double> breaks{0.0, 1.0};
  for (int axis = 0; axis < 3; ++axis) {
    const double p0 = component(start, axis);
    const double delta = component(end, axis) - p0;
    if (delta == 0.0) continue;
    const double lower = std::min(p0, p0 + delta);
    const double upper = std::max(p0, p0 + delta);
    const int first = static_cast<int>(std::floor(lower)) - 2;
    const int last = static_cast<int>(std::ceil(upper)) + 2;
    for (int k = first; k <= last; ++k) {
      const double plane = static_cast<double>(k) + 0.5;
      const double t = (plane - p0) / delta;
      if (t > 0.0 && t < 1.0) breaks.push_back(t);
    }
  }
  std::sort(breaks.begin(), breaks.end());
  breaks.erase(std::unique(breaks.begin(), breaks.end(),
      [](double a, double b) {
        return std::abs(a - b)
            <= 32.0 * std::numeric_limits<double>::epsilon();
      }), breaks.end());
  return breaks;
}

double distance_to_interval(double value, double lower, double upper) {
  if (value < lower) return lower - value;
  if (value > upper) return value - upper;
  return 0.0;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result += static_cast<long double>(lhs[i]) * rhs[i];
  }
  return result;
}

long double dot(const MatchedFaceFlux& lhs,
                const MatchedFaceFlux& rhs) {
  return dot(lhs.x, rhs.x) + dot(lhs.y, rhs.y) + dot(lhs.z, rhs.z);
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    residual = std::max(residual, std::abs(lhs[i] - rhs[i]));
  }
  return residual;
}

double max_difference(const MatchedFaceFlux& lhs,
                      const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L) return INFINITY;
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

double max_difference(const MatchedEdgeField& lhs,
                      const MatchedEdgeField& rhs) {
  if (lhs.L != rhs.L) return INFINITY;
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

void deposit_quadrature_piece(QuadraticCoatSpacetimeCurrent& result,
                              const Vec3& start,
                              const Vec3& delta,
                              const int lower[3],
                              const int upper[3],
                              double ta,
                              double tb) {
  constexpr std::array<long double, 4> nodes{{
      -0.86113631159405257522394648889281L,
      -0.33998104358485626480266575910324L,
       0.33998104358485626480266575910324L,
       0.86113631159405257522394648889281L}};
  constexpr std::array<long double, 4> weights{{
      0.34785484513745385737306394922199L,
      0.65214515486254614262693605077801L,
      0.65214515486254614262693605077801L,
      0.34785484513745385737306394922199L}};
  const long double midpoint = 0.5L * (ta + tb);
  const long double half_width = 0.5L * (tb - ta);
  for (std::size_t sample = 0; sample < nodes.size(); ++sample) {
    const long double quadrature_weight = half_width * weights[sample];
    const double t = static_cast<double>(
        midpoint + half_width * nodes[sample]);
    const Vec3 point = start + delta * t;

    for (int x = lower[0]; x <= upper[0]; ++x) {
      const double wx = b2(point.x - x);
      if (wx == 0.0) continue;
      for (int y = lower[1]; y <= upper[1]; ++y) {
        const double wy = b2(point.y - y);
        if (wy == 0.0) continue;
        for (int z = lower[2]; z <= upper[2]; ++z) {
          const double wz = b2(point.z - z);
          if (wz == 0.0) continue;
          const auto index = flat_index(result.L, x, y, z);
          const long double deposited = quadrature_weight
              * result.charge * wx * wy * wz;
          result.temporal_charge[index] += static_cast<double>(deposited);
          result.locality_residual = std::max({
              result.locality_residual,
              std::max(0.0, distance_to_interval(x,
                  std::min(start.x, start.x + delta.x),
                  std::max(start.x, start.x + delta.x)) - 1.5),
              std::max(0.0, distance_to_interval(y,
                  std::min(start.y, start.y + delta.y),
                  std::max(start.y, start.y + delta.y)) - 1.5),
              std::max(0.0, distance_to_interval(z,
                  std::min(start.z, start.z + delta.z),
                  std::max(start.z, start.z + delta.z)) - 1.5)});
        }
      }
    }

    for (int axis = 0; axis < 3; ++axis) {
      const double axis_delta = component(delta, axis);
      if (axis_delta == 0.0) continue;
      std::vector<double>* start_field = axis == 0
          ? &result.spatial_start.x
          : (axis == 1 ? &result.spatial_start.y
                       : &result.spatial_start.z);
      std::vector<double>* end_field = axis == 0
          ? &result.spatial_end.x
          : (axis == 1 ? &result.spatial_end.y
                       : &result.spatial_end.z);
      for (int x = lower[0]; x <= upper[0]; ++x) {
        for (int y = lower[1]; y <= upper[1]; ++y) {
          for (int z = lower[2]; z <= upper[2]; ++z) {
            const int coordinate[3] = {x, y, z};
            long double basis = 1.0L;
            for (int d = 0; d < 3; ++d) {
              const double center = coordinate[d] + (d == axis ? 0.5 : 0.0);
              basis *= d == axis
                  ? b1(component(point, d) - center)
                  : b2(component(point, d) - center);
            }
            if (basis == 0.0L) continue;
            const auto index = flat_index(result.L, x, y, z);
            const long double common = quadrature_weight * result.charge
                * axis_delta * basis;
            (*start_field)[index] += static_cast<double>((1.0 - t) * common);
            (*end_field)[index] += static_cast<double>(t * common);
          }
        }
      }
    }
  }
}

}  // namespace

int QuadraticCoatSpacetimeCurrent::index(int x, int y, int z) const {
  if (L <= 0) return -1;
  return static_cast<int>(flat_index(L, x, y, z));
}

QuadraticCoatSpacetimeCurrent make_quadratic_coat_spacetime_current(
    int L,
    const Vec3& start_effective_position,
    const Vec3& end_effective_position,
    int charge,
    double temporal_scale) {
  QuadraticCoatSpacetimeCurrent result;
  result.L = L;
  result.charge = charge;
  result.temporal_scale = temporal_scale;
  result.spatial_start = MatchedFaceFlux(L);
  result.spatial_end = MatchedFaceFlux(L);
  if (L < 5 || !finite(start_effective_position)
      || !finite(end_effective_position)
      || (charge != -1 && charge != 1)
      || !(temporal_scale > 0.0) || !std::isfinite(temporal_scale)) {
    return result;
  }
  result.spatial = make_quadratic_coat_face_current(
      L, start_effective_position, end_effective_position, charge);
  if (!result.spatial.valid) return result;
  result.causal_excess = result.spatial.causal_excess;
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  result.temporal_charge.assign(count, 0.0);

  const Vec3 start = result.spatial.start_effective_position;
  const Vec3 end = result.spatial.end_effective_position;
  const Vec3 delta = end - start;
  const int lower[3] = {
      static_cast<int>(std::floor(std::min(start.x, end.x))) - 3,
      static_cast<int>(std::floor(std::min(start.y, end.y))) - 3,
      static_cast<int>(std::floor(std::min(start.z, end.z))) - 3};
  const int upper[3] = {
      static_cast<int>(std::ceil(std::max(start.x, end.x))) + 3,
      static_cast<int>(std::ceil(std::max(start.y, end.y))) + 3,
      static_cast<int>(std::ceil(std::max(start.z, end.z))) + 3};
  const auto breaks = half_integer_breaks(start, end);
  for (std::size_t piece = 1; piece < breaks.size(); ++piece) {
    deposit_quadrature_piece(result, start, delta, lower, upper,
                             breaks[piece - 1], breaks[piece]);
  }

  long double temporal_sum = 0.0L;
  for (double value : result.temporal_charge) {
    temporal_sum += static_cast<long double>(value);
    if (value != 0.0) ++result.temporal_support;
  }
  result.temporal_partition_residual = std::abs(static_cast<double>(
      temporal_sum - static_cast<long double>(charge)));
  for (std::size_t i = 0; i < count; ++i) {
    result.spatial_split_residual = std::max({
        result.spatial_split_residual,
        std::abs(result.spatial_start.x[i] + result.spatial_end.x[i]
                 - result.spatial.current_x[i]),
        std::abs(result.spatial_start.y[i] + result.spatial_end.y[i]
                 - result.spatial.current_y[i]),
        std::abs(result.spatial_start.z[i] + result.spatial_end.z[i]
                 - result.spatial.current_z[i])});
  }
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = flat_index(L, x, y, z);
        result.split_continuity_start_residual = std::max(
            result.split_continuity_start_residual,
            std::abs(divergence_at(result.spatial_start, x, y, z)
                     + result.temporal_charge[i]
                     - result.spatial.rho_before[i]));
        result.split_continuity_end_residual = std::max(
            result.split_continuity_end_residual,
            std::abs(divergence_at(result.spatial_end, x, y, z)
                     - result.temporal_charge[i]
                     + result.spatial.rho_after[i]));
      }
    }
  }
  result.locality_residual = std::max(
      result.locality_residual, result.spatial.locality_residual);
  result.valid = finite(result.spatial_start)
      && finite(result.spatial_end) && finite(result.temporal_charge)
      && result.spatial_split_residual <= 1e-12
      && result.temporal_partition_residual <= 1e-12
      && result.split_continuity_start_residual <= 1e-12
      && result.split_continuity_end_residual <= 1e-12
      && result.locality_residual <= 1e-12
      && result.causal_excess <= 1e-12;
  return result;
}

double quadratic_coat_interaction_action(
    const QuadraticCoatSpacetimeCurrent& current,
    const DualGaugePotentialSlab& slab,
    double coupling) {
  if (!current.valid || current.L != slab.L || slab.L <= 0
      || current.temporal_scale != slab.temporal_scale
      || !std::isfinite(coupling)
      || !valid_face_size(slab.A_start, slab.L)
      || !valid_face_size(slab.A_end, slab.L)
      || slab.Phi.size() != current.temporal_charge.size()
      || !finite(slab.A_start) || !finite(slab.A_end)
      || !finite(slab.Phi)) return NAN;
  const long double spatial = dot(slab.A_start, current.spatial_start)
      + dot(slab.A_end, current.spatial_end);
  const long double temporal = dot(slab.Phi, current.temporal_charge);
  return static_cast<double>(static_cast<long double>(coupling)
      * (spatial - current.temporal_scale * temporal));
}

QuadraticCoatGaugeActionResult evaluate_quadratic_coat_gauge_action(
    const QuadraticCoatSpacetimeCurrent& current,
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end,
    double coupling) {
  QuadraticCoatGaugeActionResult result;
  result.coupling = coupling;
  result.electric = MatchedFaceFlux(slab.L);
  result.magnetic_start = MatchedEdgeField(slab.L);
  result.magnetic_end = MatchedEdgeField(slab.L);
  const std::size_t count = slab.L > 0
      ? static_cast<std::size_t>(slab.L) * slab.L * slab.L : 0;
  if (!current.valid || current.L != slab.L || slab.L <= 0
      || current.temporal_scale != slab.temporal_scale
      || !std::isfinite(coupling)
      || chi_start.size() != count || chi_end.size() != count
      || !finite(chi_start) || !finite(chi_end)) return result;

  result.interaction_action = quadratic_coat_interaction_action(
      current, slab, coupling);
  if (!std::isfinite(result.interaction_action)) return result;
  result.electric = slab_electric_field(slab);
  result.magnetic_start = matched_curl_adjoint(slab.A_start);
  result.magnetic_end = matched_curl_adjoint(slab.A_end);
  const auto transformed = gauge_transform_slab(
      slab, chi_start, chi_end);
  result.transformed_action = quadratic_coat_interaction_action(
      current, transformed, coupling);
  result.action_shift = result.transformed_action
      - result.interaction_action;
  result.endpoint_shift = coupling * static_cast<double>(
      dot(current.spatial.rho_after, chi_end)
      - dot(current.spatial.rho_before, chi_start));
  result.gauge_endpoint_residual = std::abs(
      result.action_shift - result.endpoint_shift);

  const auto transformed_electric = slab_electric_field(transformed);
  const auto transformed_magnetic_start = matched_curl_adjoint(
      transformed.A_start);
  const auto transformed_magnetic_end = matched_curl_adjoint(
      transformed.A_end);
  result.electric_invariance_residual = max_difference(
      result.electric, transformed_electric);
  result.magnetic_invariance_residual = std::max(
      max_difference(result.magnetic_start, transformed_magnetic_start),
      max_difference(result.magnetic_end, transformed_magnetic_end));
  const auto gradient_start = matched_forward_gradient(
      slab.L, chi_start);
  const auto gradient_end = matched_forward_gradient(slab.L, chi_end);
  const MatchedEdgeField zero(slab.L);
  result.curl_gradient_residual = std::max(
      max_difference(matched_curl_adjoint(gradient_start), zero),
      max_difference(matched_curl_adjoint(gradient_end), zero));
  result.valid = finite(result.electric)
      && finite(result.magnetic_start) && finite(result.magnetic_end)
      && std::isfinite(result.transformed_action)
      && std::isfinite(result.gauge_endpoint_residual)
      && std::isfinite(result.electric_invariance_residual)
      && std::isfinite(result.magnetic_invariance_residual)
      && std::isfinite(result.curl_gradient_residual);
  return result;
}

}  // namespace ftd::eft

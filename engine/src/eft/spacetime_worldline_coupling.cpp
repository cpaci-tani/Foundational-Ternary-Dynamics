#include "ftd/eft/spacetime_worldline_coupling.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

std::size_t flat_index(int L, int x, int y, int z) {
  const std::size_t wx = static_cast<std::size_t>(wrap(x, L));
  const std::size_t wy = static_cast<std::size_t>(wrap(y, L));
  const std::size_t wz = static_cast<std::size_t>(wrap(z, L));
  return (wx * static_cast<std::size_t>(L) + wy)
      * static_cast<std::size_t>(L) + wz;
}

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

Coord with_component(Coord value, int axis, int coordinate) {
  if (axis == 0) value.x = coordinate;
  if (axis == 1) value.y = coordinate;
  if (axis == 2) value.z = coordinate;
  return value;
}

double hat_weight(double position, int site) {
  return std::max(0.0, 1.0 - std::abs(position - site));
}

std::vector<double> segment_breaks(const Vec3& start, const Vec3& end) {
  std::vector<double> breaks{0.0, 1.0};
  for (int axis = 0; axis < 3; ++axis) {
    const double p0 = component(start, axis);
    const double p1 = component(end, axis);
    const double delta = p1 - p0;
    if (delta == 0.0) continue;
    const double lower = std::min(p0, p1);
    const double upper = std::max(p0, p1);
    const int first_plane = static_cast<int>(std::floor(lower)) + 1;
    const int last_plane = static_cast<int>(std::ceil(upper)) - 1;
    for (int plane = first_plane; plane <= last_plane; ++plane) {
      const double tau = (static_cast<double>(plane) - p0) / delta;
      if (tau > 0.0 && tau < 1.0) breaks.push_back(tau);
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

Vec3 point_on_segment(const Vec3& start, const Vec3& delta, double tau) {
  return start + delta * tau;
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
  const std::size_t count = static_cast<std::size_t>(L * L * L);
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return NAN;
  long double value = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    value += static_cast<long double>(lhs[i]) * rhs[i];
  }
  return value;
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

void deposit_spatial_piece(
    SpacetimeWorldlineCurrent& result,
    int axis,
    const Vec3& start,
    const Vec3& delta,
    double ta,
    double tb) {
  const double axis_delta = component(delta, axis);
  if (axis_delta == 0.0 || tb <= ta) return;

  const double h = tb - ta;
  const double tm = 0.5 * (ta + tb);
  const Vec3 pa = point_on_segment(start, delta, ta);
  const Vec3 pb = point_on_segment(start, delta, tb);
  const Vec3 pm = point_on_segment(start, delta, tm);
  const int face_coordinate = static_cast<int>(
      std::floor(component(pm, axis)));
  const int transverse_a = (axis + 1) % 3;
  const int transverse_b = (axis + 2) % 3;
  const int lower_a = static_cast<int>(
      std::floor(component(pm, transverse_a)));
  const int lower_b = static_cast<int>(
      std::floor(component(pm, transverse_b)));

  std::vector<double>* start_field = axis == 0 ? &result.spatial_start.x
      : (axis == 1 ? &result.spatial_start.y : &result.spatial_start.z);
  std::vector<double>* end_field = axis == 0 ? &result.spatial_end.x
      : (axis == 1 ? &result.spatial_end.y : &result.spatial_end.z);

  for (int da = 0; da <= 1; ++da) {
    const int site_a = lower_a + da;
    const long double a0 = hat_weight(
        component(pa, transverse_a), site_a);
    const long double a1 = hat_weight(
        component(pb, transverse_a), site_a);
    const long double ad = a1 - a0;
    for (int db = 0; db <= 1; ++db) {
      const int site_b = lower_b + db;
      const long double b0 = hat_weight(
          component(pa, transverse_b), site_b);
      const long double b1 = hat_weight(
          component(pb, transverse_b), site_b);
      const long double bd = b1 - b0;

      const long double c0 = a0 * b0;
      const long double c1 = a0 * bd + ad * b0;
      const long double c2 = ad * bd;
      const long double integral_0 = c0 + c1 / 2.0L + c2 / 3.0L;
      const long double integral_u =
          c0 / 2.0L + c1 / 3.0L + c2 / 4.0L;
      const long double common = static_cast<long double>(result.charge)
          * axis_delta * h;
      const long double deposited_start = common
          * ((1.0L - ta) * integral_0 - h * integral_u);
      const long double deposited_end = common
          * (ta * integral_0 + h * integral_u);

      Coord face{};
      face = with_component(face, axis, face_coordinate);
      face = with_component(face, transverse_a, site_a);
      face = with_component(face, transverse_b, site_b);
      const std::size_t index = flat_index(
          result.L, face.x, face.y, face.z);
      (*start_field)[index] += static_cast<double>(deposited_start);
      (*end_field)[index] += static_cast<double>(deposited_end);
    }
  }
}

void deposit_temporal_piece(
    SpacetimeWorldlineCurrent& result,
    const Vec3& start,
    const Vec3& delta,
    double ta,
    double tb) {
  if (tb <= ta) return;
  const double h = tb - ta;
  const double tm = 0.5 * (ta + tb);
  const Vec3 pa = point_on_segment(start, delta, ta);
  const Vec3 pb = point_on_segment(start, delta, tb);
  const Vec3 pm = point_on_segment(start, delta, tm);
  int lower[3]{};
  for (int axis = 0; axis < 3; ++axis) {
    lower[axis] = static_cast<int>(std::floor(component(pm, axis)));
  }

  for (int dx = 0; dx <= 1; ++dx) {
    for (int dy = 0; dy <= 1; ++dy) {
      for (int dz = 0; dz <= 1; ++dz) {
        const int sites[3] = {lower[0] + dx,
                              lower[1] + dy,
                              lower[2] + dz};
        long double a[3]{};
        long double d[3]{};
        for (int axis = 0; axis < 3; ++axis) {
          a[axis] = hat_weight(component(pa, axis), sites[axis]);
          const long double at_end = hat_weight(
              component(pb, axis), sites[axis]);
          d[axis] = at_end - a[axis];
        }
        const long double c0 = a[0] * a[1] * a[2];
        const long double c1 = d[0] * a[1] * a[2]
            + a[0] * d[1] * a[2] + a[0] * a[1] * d[2];
        const long double c2 = d[0] * d[1] * a[2]
            + d[0] * a[1] * d[2] + a[0] * d[1] * d[2];
        const long double c3 = d[0] * d[1] * d[2];
        const long double integral = h * (c0 + c1 / 2.0L
            + c2 / 3.0L + c3 / 4.0L);
        const std::size_t index = flat_index(
            result.L, sites[0], sites[1], sites[2]);
        result.temporal_charge[index] += static_cast<double>(
            static_cast<long double>(result.charge) * integral);
      }
    }
  }
}

double action(const SpacetimeWorldlineCurrent& current,
              const DualGaugePotentialSlab& slab,
              double coupling) {
  const long double spatial = dot(slab.A_start, current.spatial_start)
      + dot(slab.A_end, current.spatial_end);
  const long double temporal = dot(slab.Phi, current.temporal_charge);
  return static_cast<double>(static_cast<long double>(coupling)
      * (spatial - current.temporal_scale * temporal));
}

}  // namespace

int SpacetimeWorldlineCurrent::index(int x, int y, int z) const {
  if (L <= 0) return -1;
  return static_cast<int>(flat_index(L, x, y, z));
}

DualGaugePotentialSlab::DualGaugePotentialSlab(
    int size, double time_scale)
    : L(size),
      temporal_scale(time_scale),
      A_start(size),
      A_end(size),
      Phi(static_cast<std::size_t>(size * size * size), 0.0) {}

int DualGaugePotentialSlab::index(int x, int y, int z) const {
  if (L <= 0) return -1;
  return static_cast<int>(flat_index(L, x, y, z));
}

SpacetimeWorldlineCurrent make_spacetime_worldline_current(
    int L,
    Coord start_anchor,
    const Vec3& start_remainder,
    Coord end_anchor,
    const Vec3& end_remainder,
    int charge,
    double temporal_scale) {
  SpacetimeWorldlineCurrent result;
  result.L = L;
  result.charge = charge;
  result.temporal_scale = temporal_scale;
  result.spatial = make_face_current_segment(
      L, start_anchor, start_remainder,
      end_anchor, end_remainder, charge);
  if (!result.spatial.valid || !std::isfinite(temporal_scale)
      || temporal_scale <= 0.0) {
    return result;
  }

  result.spatial_start = MatchedFaceFlux(L);
  result.spatial_end = MatchedFaceFlux(L);
  const std::size_t count = static_cast<std::size_t>(L * L * L);
  result.temporal_charge.assign(count, 0.0);

  const Vec3 start = result.spatial.start_effective_position;
  const Vec3 end = result.spatial.end_effective_position;
  const Vec3 delta = end - start;
  const auto breaks = segment_breaks(start, end);
  for (std::size_t piece = 0; piece + 1 < breaks.size(); ++piece) {
    const double ta = breaks[piece];
    const double tb = breaks[piece + 1];
    for (int axis = 0; axis < 3; ++axis) {
      deposit_spatial_piece(result, axis, start, delta, ta, tb);
    }
    deposit_temporal_piece(result, start, delta, ta, tb);
  }

  result.spatial_split_residual = 0.0;
  const std::vector<double>* existing[3] = {
      &result.spatial.current_x,
      &result.spatial.current_y,
      &result.spatial.current_z};
  const std::vector<double>* first[3] = {
      &result.spatial_start.x,
      &result.spatial_start.y,
      &result.spatial_start.z};
  const std::vector<double>* second[3] = {
      &result.spatial_end.x,
      &result.spatial_end.y,
      &result.spatial_end.z};
  for (int axis = 0; axis < 3; ++axis) {
    for (std::size_t i = 0; i < count; ++i) {
      result.spatial_split_residual = std::max(
          result.spatial_split_residual,
          std::abs((*first[axis])[i] + (*second[axis])[i]
                   - (*existing[axis])[i]));
    }
  }
  const long double temporal_sum = std::accumulate(
      result.temporal_charge.begin(), result.temporal_charge.end(), 0.0L);
  result.temporal_partition_residual = std::abs(static_cast<double>(
      temporal_sum - static_cast<long double>(charge)));
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const std::size_t i = flat_index(L, x, y, z);
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
  result.temporal_support = static_cast<int>(std::count_if(
      result.temporal_charge.begin(), result.temporal_charge.end(),
      [](double value) { return value != 0.0; }));
  result.locality_residual = result.spatial.locality_residual;
  result.valid = finite(result.spatial_start)
      && finite(result.spatial_end)
      && finite(result.temporal_charge)
      && std::isfinite(result.spatial_split_residual)
      && std::isfinite(result.temporal_partition_residual)
      && std::isfinite(result.split_continuity_start_residual)
      && std::isfinite(result.split_continuity_end_residual);
  return result;
}

MatchedFaceFlux matched_forward_gradient(
    int L, const std::vector<double>& site_scalar) {
  MatchedFaceFlux result(L);
  const std::size_t count = static_cast<std::size_t>(L * L * L);
  if (L <= 0 || site_scalar.size() != count || !finite(site_scalar)) {
    return result;
  }
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const std::size_t i = flat_index(L, x, y, z);
        result.x[i] = site_scalar[flat_index(L, x + 1, y, z)]
            - site_scalar[i];
        result.y[i] = site_scalar[flat_index(L, x, y + 1, z)]
            - site_scalar[i];
        result.z[i] = site_scalar[flat_index(L, x, y, z + 1)]
            - site_scalar[i];
      }
    }
  }
  return result;
}

MatchedFaceFlux slab_electric_field(const DualGaugePotentialSlab& slab) {
  MatchedFaceFlux result(slab.L);
  if (slab.L <= 0 || !std::isfinite(slab.temporal_scale)
      || slab.temporal_scale <= 0.0
      || !valid_face_size(slab.A_start, slab.L)
      || !valid_face_size(slab.A_end, slab.L)
      || slab.Phi.size() != result.x.size()
      || !finite(slab.A_start) || !finite(slab.A_end)
      || !finite(slab.Phi)) {
    return result;
  }
  const auto gradient = matched_forward_gradient(slab.L, slab.Phi);
  for (std::size_t i = 0; i < result.x.size(); ++i) {
    result.x[i] = -(slab.A_end.x[i] - slab.A_start.x[i])
        / slab.temporal_scale - gradient.x[i];
    result.y[i] = -(slab.A_end.y[i] - slab.A_start.y[i])
        / slab.temporal_scale - gradient.y[i];
    result.z[i] = -(slab.A_end.z[i] - slab.A_start.z[i])
        / slab.temporal_scale - gradient.z[i];
  }
  return result;
}

DualGaugePotentialSlab gauge_transform_slab(
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end) {
  DualGaugePotentialSlab result = slab;
  const std::size_t count = static_cast<std::size_t>(slab.L * slab.L * slab.L);
  if (slab.L <= 0 || !std::isfinite(slab.temporal_scale)
      || slab.temporal_scale <= 0.0
      || chi_start.size() != count || chi_end.size() != count
      || !finite(chi_start) || !finite(chi_end)) {
    result.L = 0;
    return result;
  }
  const auto gradient_start = matched_forward_gradient(slab.L, chi_start);
  const auto gradient_end = matched_forward_gradient(slab.L, chi_end);
  for (std::size_t i = 0; i < count; ++i) {
    result.A_start.x[i] += gradient_start.x[i];
    result.A_start.y[i] += gradient_start.y[i];
    result.A_start.z[i] += gradient_start.z[i];
    result.A_end.x[i] += gradient_end.x[i];
    result.A_end.y[i] += gradient_end.y[i];
    result.A_end.z[i] += gradient_end.z[i];
    result.Phi[i] -= (chi_end[i] - chi_start[i])
        / slab.temporal_scale;
  }
  return result;
}

SpacetimeGaugeCouplingResult evaluate_spacetime_gauge_coupling(
    const SpacetimeWorldlineCurrent& current,
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end,
    double coupling) {
  SpacetimeGaugeCouplingResult result;
  result.coupling = coupling;
  result.electric = MatchedFaceFlux(slab.L);
  result.magnetic_start = MatchedEdgeField(slab.L);
  result.magnetic_end = MatchedEdgeField(slab.L);
  const std::size_t count = static_cast<std::size_t>(slab.L * slab.L * slab.L);
  if (!current.valid || current.L != slab.L || slab.L <= 0
      || current.temporal_scale != slab.temporal_scale
      || !std::isfinite(coupling)
      || chi_start.size() != count || chi_end.size() != count
      || !finite(chi_start) || !finite(chi_end)
      || !valid_face_size(slab.A_start, slab.L)
      || !valid_face_size(slab.A_end, slab.L)
      || slab.Phi.size() != count || !finite(slab.A_start)
      || !finite(slab.A_end) || !finite(slab.Phi)) {
    return result;
  }

  result.interaction_action = action(current, slab, coupling);
  result.electric = slab_electric_field(slab);
  result.magnetic_start = matched_curl_adjoint(slab.A_start);
  result.magnetic_end = matched_curl_adjoint(slab.A_end);

  const auto transformed = gauge_transform_slab(
      slab, chi_start, chi_end);
  if (transformed.L != slab.L) return result;
  result.transformed_action = action(current, transformed, coupling);
  result.action_shift = result.transformed_action - result.interaction_action;
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

  const auto gradient_start = matched_forward_gradient(slab.L, chi_start);
  const auto gradient_end = matched_forward_gradient(slab.L, chi_end);
  const auto curl_gradient_start = matched_curl_adjoint(gradient_start);
  const auto curl_gradient_end = matched_curl_adjoint(gradient_end);
  MatchedEdgeField zero(slab.L);
  result.curl_gradient_residual = std::max(
      max_difference(curl_gradient_start, zero),
      max_difference(curl_gradient_end, zero));
  result.valid = finite(result.electric)
      && finite(result.magnetic_start) && finite(result.magnetic_end)
      && std::isfinite(result.interaction_action)
      && std::isfinite(result.transformed_action)
      && std::isfinite(result.gauge_endpoint_residual)
      && std::isfinite(result.electric_invariance_residual)
      && std::isfinite(result.magnetic_invariance_residual)
      && std::isfinite(result.curl_gradient_residual);
  return result;
}

}  // namespace ftd::eft

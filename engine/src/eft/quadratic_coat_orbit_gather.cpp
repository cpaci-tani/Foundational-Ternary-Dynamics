#include "ftd/eft/quadratic_coat_orbit_gather.h"

#include "ftd/eft/local_polarity_regularity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

void set_component(Vec3& value, int axis, double component_value) {
  if (axis == 0) value.x = component_value;
  else if (axis == 1) value.y = component_value;
  else value.z = component_value;
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

std::size_t volume(int L) {
  return L > 0 ? static_cast<std::size_t>(L)*L*L : 0;
}

bool finite(const MatchedFaceFlux& field) {
  const std::size_t expected = volume(field.L);
  if (field.L <= 0 || field.x.size() != expected
      || field.y.size() != expected || field.z.size() != expected)
    return false;
  const auto all_finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  return all_finite(field.x) && all_finite(field.y) && all_finite(field.z);
}

bool finite(const MatchedEdgeField& field) {
  const std::size_t expected = volume(field.L);
  if (field.L <= 0 || field.x.size() != expected
      || field.y.size() != expected || field.z.size() != expected)
    return false;
  const auto all_finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  return all_finite(field.x) && all_finite(field.y) && all_finite(field.z);
}

double b1(double u) {
  return evaluate_local_polarity_kernel(LocalPolarityKernel::Hat, u);
}

double b2(double u) {
  return evaluate_local_polarity_kernel(
      LocalPolarityKernel::QuadraticBSpline, u);
}

double b2_derivative(double u) {
  return b1(u+0.5)-b1(u-0.5);
}

const std::vector<double>& face_component(
    const MatchedFaceFlux& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

const std::vector<double>& edge_component(
    const MatchedEdgeField& field, int axis) {
  return axis == 0 ? field.x : (axis == 1 ? field.y : field.z);
}

double face_component_at(const MatchedFaceFlux& field,
                         int axis, const Vec3& position) {
  const auto& coefficients = face_component(field, axis);
  const int lower_x = static_cast<int>(std::floor(position.x))-2;
  const int lower_y = static_cast<int>(std::floor(position.y))-2;
  const int lower_z = static_cast<int>(std::floor(position.z))-2;
  long double result = 0.0L;
  for (int x = lower_x; x <= lower_x+4; ++x) {
    const double wx = axis == 0 ? b1(position.x-x-0.5)
                                : b2(position.x-x);
    if (wx == 0.0) continue;
    for (int y = lower_y; y <= lower_y+4; ++y) {
      const double wy = axis == 1 ? b1(position.y-y-0.5)
                                  : b2(position.y-y);
      if (wy == 0.0) continue;
      for (int z = lower_z; z <= lower_z+4; ++z) {
        const double wz = axis == 2 ? b1(position.z-z-0.5)
                                    : b2(position.z-z);
        if (wz == 0.0) continue;
        result += static_cast<long double>(wx)*wy*wz
            *coefficients[static_cast<std::size_t>(field.index(x, y, z))];
      }
    }
  }
  return static_cast<double>(result);
}

double edge_component_at(const MatchedEdgeField& field,
                         int axis, const Vec3& position) {
  const auto& coefficients = edge_component(field, axis);
  const int lower_x = static_cast<int>(std::floor(position.x))-2;
  const int lower_y = static_cast<int>(std::floor(position.y))-2;
  const int lower_z = static_cast<int>(std::floor(position.z))-2;
  long double result = 0.0L;
  for (int x = lower_x; x <= lower_x+4; ++x) {
    const double wx = axis == 0 ? b2(position.x-x)
                                : b1(position.x-x-0.5);
    if (wx == 0.0) continue;
    for (int y = lower_y; y <= lower_y+4; ++y) {
      const double wy = axis == 1 ? b2(position.y-y)
                                  : b1(position.y-y-0.5);
      if (wy == 0.0) continue;
      for (int z = lower_z; z <= lower_z+4; ++z) {
        const double wz = axis == 2 ? b2(position.z-z)
                                    : b1(position.z-z-0.5);
        if (wz == 0.0) continue;
        result += static_cast<long double>(wx)*wy*wz
            *coefficients[static_cast<std::size_t>(field.index(x, y, z))];
      }
    }
  }
  return static_cast<double>(result);
}

std::vector<double> half_integer_breaks(const Vec3& start,
                                        const Vec3& end) {
  std::vector<double> breaks{0.0, 1.0};
  for (int axis = 0; axis < 3; ++axis) {
    const double p0 = component(start, axis);
    const double delta = component(end, axis)-p0;
    if (delta == 0.0) continue;
    const double lower = std::min(p0, p0+delta);
    const double upper = std::max(p0, p0+delta);
    const int first = static_cast<int>(std::floor(lower))-2;
    const int last = static_cast<int>(std::ceil(upper))+2;
    for (int knot = first; knot <= last; ++knot) {
      const double tau = (static_cast<double>(knot)+0.5-p0)/delta;
      if (tau > 0.0 && tau < 1.0) breaks.push_back(tau);
    }
  }
  std::sort(breaks.begin(), breaks.end());
  breaks.erase(std::unique(breaks.begin(), breaks.end(),
      [](double a, double b) {
        return std::abs(a-b) <= 32.0
            *std::numeric_limits<double>::epsilon();
      }), breaks.end());
  return breaks;
}

template <typename Function>
Vec3 integrate_orbit(const Vec3& start,
                     const Vec3& end,
                     const std::vector<double>& breaks,
                     Function&& function) {
  constexpr std::array<long double, 8> nodes{{
      -0.960289856497536231683560868569L,
      -0.796666477413626739591553936476L,
      -0.525532409916328985817739049189L,
      -0.183434642495649804939476142360L,
       0.183434642495649804939476142360L,
       0.525532409916328985817739049189L,
       0.796666477413626739591553936476L,
       0.960289856497536231683560868569L}};
  constexpr std::array<long double, 8> weights{{
      0.101228536290376259152531354310L,
      0.222381034453374470544355994426L,
      0.313706645877887287337962201987L,
      0.362683783378361982965150449277L,
      0.362683783378361982965150449277L,
      0.313706645877887287337962201987L,
      0.222381034453374470544355994426L,
      0.101228536290376259152531354310L}};
  const Vec3 displacement = end-start;
  long double sum_x = 0.0L;
  long double sum_y = 0.0L;
  long double sum_z = 0.0L;
  for (std::size_t piece = 1; piece < breaks.size(); ++piece) {
    const long double lower = breaks[piece-1];
    const long double upper = breaks[piece];
    const long double midpoint = 0.5L*(lower+upper);
    const long double half_width = 0.5L*(upper-lower);
    for (std::size_t sample = 0; sample < nodes.size(); ++sample) {
      const double tau = static_cast<double>(midpoint+half_width*nodes[sample]);
      const Vec3 value = function(start+displacement*tau);
      const long double weight = half_width*weights[sample];
      sum_x += weight*value.x;
      sum_y += weight*value.y;
      sum_z += weight*value.z;
    }
  }
  return {static_cast<double>(sum_x), static_cast<double>(sum_y),
          static_cast<double>(sum_z)};
}

double current_pairing(const QuadraticCoatFaceCurrent& segment,
                       const MatchedFaceFlux& field) {
  const std::size_t expected = volume(segment.L);
  if (field.L != segment.L || field.x.size() != expected
      || field.y.size() != expected || field.z.size() != expected)
    return std::numeric_limits<double>::infinity();
  long double result = 0.0L;
  if (!segment.dense_materialized) {
    for (const auto& entry : segment.sparse_current) {
      if (entry.axis < 0 || entry.axis > 2 || !std::isfinite(entry.value))
        return std::numeric_limits<double>::infinity();
      const auto& coefficients = face_component(field, entry.axis);
      result += static_cast<long double>(entry.value)
          *coefficients[static_cast<std::size_t>(segment.index(
              entry.face.x, entry.face.y, entry.face.z))];
    }
    return static_cast<double>(result);
  }
  if (segment.current_x.size() != expected
      || segment.current_y.size() != expected
      || segment.current_z.size() != expected)
    return std::numeric_limits<double>::infinity();
  for (std::size_t i = 0; i < expected; ++i) {
    result += static_cast<long double>(segment.current_x[i])*field.x[i]
        +static_cast<long double>(segment.current_y[i])*field.y[i]
        +static_cast<long double>(segment.current_z[i])*field.z[i];
  }
  return static_cast<double>(result);
}

bool field_shape_valid(const MatchedFaceFlux& field, int L) {
  const std::size_t expected = volume(L);
  return L > 0 && field.L == L && field.x.size() == expected
      && field.y.size() == expected && field.z.size() == expected;
}

bool field_shape_valid(const MatchedEdgeField& field, int L) {
  const std::size_t expected = volume(L);
  return L > 0 && field.L == L && field.x.size() == expected
      && field.y.size() == expected && field.z.size() == expected;
}

double sparse_coefficient(const std::vector<QuadraticCoatFaceCurrent>& segments,
                          int axis, int wrapped_index) {
  long double result = 0.0L;
  for (const auto& segment : segments) {
    for (const auto& entry : segment.sparse_current) {
      if (entry.axis == axis
          && segment.index(entry.face.x, entry.face.y, entry.face.z)
              == wrapped_index)
        result += static_cast<long double>(entry.value);
    }
  }
  return static_cast<double>(result);
}

double sparse_component_at(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    int L, int axis, const Vec3& position) {
  const int lower_x = static_cast<int>(std::floor(position.x))-2;
  const int lower_y = static_cast<int>(std::floor(position.y))-2;
  const int lower_z = static_cast<int>(std::floor(position.z))-2;
  long double result = 0.0L;
  for (int x = lower_x; x <= lower_x+4; ++x) {
    const double wx = axis == 0 ? b1(position.x-x-0.5)
                                : b2(position.x-x);
    if (wx == 0.0) continue;
    for (int y = lower_y; y <= lower_y+4; ++y) {
      const double wy = axis == 1 ? b1(position.y-y-0.5)
                                  : b2(position.y-y);
      if (wy == 0.0) continue;
      for (int z = lower_z; z <= lower_z+4; ++z) {
        const double wz = axis == 2 ? b1(position.z-z-0.5)
                                    : b2(position.z-z);
        if (wz == 0.0) continue;
        const int wrapped_index = static_cast<int>(
            (static_cast<std::size_t>((x%L+L)%L)*L
             +static_cast<std::size_t>((y%L+L)%L))*L
             +static_cast<std::size_t>((z%L+L)%L));
        result += static_cast<long double>(wx)*wy*wz
            *sparse_coefficient(segments, axis, wrapped_index);
      }
    }
  }
  return static_cast<double>(result);
}

Vec3 sparse_field_at(const std::vector<QuadraticCoatFaceCurrent>& segments,
                     int L, const Vec3& position) {
  return {sparse_component_at(segments, L, 0, position),
          sparse_component_at(segments, L, 1, position),
          sparse_component_at(segments, L, 2, position)};
}

double sparse_midpoint_coefficient(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    const MatchedFaceFlux& fixed_electric,
    const MatchedFaceFlux& electric_pre_current,
    double current_scale, int axis, int wrapped_index) {
  const auto& fixed = face_component(fixed_electric, axis);
  const auto& prepared = face_component(electric_pre_current, axis);
  return 0.5*(fixed[static_cast<std::size_t>(wrapped_index)]
      +prepared[static_cast<std::size_t>(wrapped_index)]
      +current_scale*sparse_coefficient(segments, axis, wrapped_index));
}

QuadraticCoatOrbitGatherResult evaluate_prevalidated(
    const QuadraticCoatFaceCurrent& segment,
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    const Vec3& discrete_gradient_velocity,
    double temporal_scale,
    double beta,
    double polarity_scale) {
  QuadraticCoatOrbitGatherResult result;
  result.L = segment.L;
  result.charge = segment.charge;
  result.start_effective_position = segment.start_effective_position;
  result.end_effective_position = segment.end_effective_position;
  result.displacement = result.end_effective_position
      -result.start_effective_position;
  result.discrete_gradient_velocity = discrete_gradient_velocity;
  result.temporal_scale = temporal_scale;
  result.beta = beta;
  if (!segment.valid || (segment.charge != -1 && segment.charge != 1)
      || !field_shape_valid(electric, segment.L)
      || !field_shape_valid(magnetic, segment.L)
      || !finite(discrete_gradient_velocity)
      || !(temporal_scale > 0.0) || !std::isfinite(temporal_scale)
      || !std::isfinite(beta) || !(polarity_scale > 0.0)
      || !std::isfinite(polarity_scale))
    return result;

  const std::vector<double> breaks = half_integer_breaks(
      result.start_effective_position, result.end_effective_position);
  result.quadrature_pieces = static_cast<int>(breaks.size())-1;
  const Vec3 electric_average = integrate_orbit(
      result.start_effective_position, result.end_effective_position, breaks,
      [&electric](const Vec3& position) {
        return Vec3{face_component_at(electric, 0, position),
                    face_component_at(electric, 1, position),
                    face_component_at(electric, 2, position)};
      });
  result.magnetic_average = integrate_orbit(
      result.start_effective_position, result.end_effective_position, breaks,
      [&magnetic](const Vec3& position) {
        return Vec3{edge_component_at(magnetic, 0, position),
                    edge_component_at(magnetic, 1, position),
                    edge_component_at(magnetic, 2, position)};
      });
  const double effective_charge = polarity_scale*segment.charge;
  result.electric_force = electric_average*effective_charge;
  result.current_work = polarity_scale*current_pairing(segment, electric);
  result.electric_work = result.displacement.dot(result.electric_force);
  result.electric_adjoint_residual = std::abs(
      result.current_work-result.electric_work);
  result.magnetic_impulse = Vec3::cross(
      discrete_gradient_velocity, result.magnetic_average)
      *(temporal_scale*beta*effective_charge);
  result.magnetic_work_residual = std::abs(
      discrete_gradient_velocity.dot(result.magnetic_impulse));
  const Vec3 kinematic = result.displacement
      -discrete_gradient_velocity*temporal_scale;
  result.kinematic_residual = std::max({std::abs(kinematic.x),
      std::abs(kinematic.y), std::abs(kinematic.z)});
  result.causal_excess = std::max(0.0,
      discrete_gradient_velocity.mag()-C_SPEED);
  result.valid = finite(result.electric_force)
      && finite(result.magnetic_average) && finite(result.magnetic_impulse)
      && std::isfinite(result.current_work)
      && result.electric_adjoint_residual <= 5e-13
      && result.magnetic_work_residual <= 5e-13
      && result.kinematic_residual <= 5e-13
      && result.causal_excess <= 5e-13;
  return result;
}

}  // namespace

Vec3 interpolate_quadratic_face_field(
    const MatchedFaceFlux& field, const Vec3& position) {
  if (!finite(field) || !finite(position)) return {NAN, NAN, NAN};
  return {face_component_at(field, 0, position),
          face_component_at(field, 1, position),
          face_component_at(field, 2, position)};
}

Vec3 interpolate_quadratic_edge_field(
    const MatchedEdgeField& field, const Vec3& position) {
  if (!finite(field) || !finite(position)) return {NAN, NAN, NAN};
  return {edge_component_at(field, 0, position),
          edge_component_at(field, 1, position),
          edge_component_at(field, 2, position)};
}

Vec3 curl_interpolated_quadratic_face_potential(
    const MatchedFaceFlux& potential, const Vec3& position) {
  if (!finite(potential) || !finite(position)) return {NAN, NAN, NAN};
  Vec3 curl{};
  const int lower_x = static_cast<int>(std::floor(position.x))-2;
  const int lower_y = static_cast<int>(std::floor(position.y))-2;
  const int lower_z = static_cast<int>(std::floor(position.z))-2;
  long double cx = 0.0L;
  long double cy = 0.0L;
  long double cz = 0.0L;
  for (int x = lower_x; x <= lower_x+4; ++x) {
    const double bx1 = b1(position.x-x-0.5);
    const double bx2 = b2(position.x-x);
    const double dx2 = b2_derivative(position.x-x);
    for (int y = lower_y; y <= lower_y+4; ++y) {
      const double by1 = b1(position.y-y-0.5);
      const double by2 = b2(position.y-y);
      const double dy2 = b2_derivative(position.y-y);
      for (int z = lower_z; z <= lower_z+4; ++z) {
        const double bz1 = b1(position.z-z-0.5);
        const double bz2 = b2(position.z-z);
        const double dz2 = b2_derivative(position.z-z);
        const std::size_t i = static_cast<std::size_t>(
            potential.index(x, y, z));
        cx += static_cast<long double>(potential.z[i])*bx2*dy2*bz1
            -static_cast<long double>(potential.y[i])*bx2*by1*dz2;
        cy += static_cast<long double>(potential.x[i])*bx1*by2*dz2
            -static_cast<long double>(potential.z[i])*dx2*by2*bz1;
        cz += static_cast<long double>(potential.y[i])*dx2*by1*bz2
            -static_cast<long double>(potential.x[i])*bx1*dy2*bz2;
      }
    }
  }
  curl.x = static_cast<double>(cx);
  curl.y = static_cast<double>(cy);
  curl.z = static_cast<double>(cz);
  return curl;
}

double quadratic_spline_curl_commutation_residual(
    const MatchedFaceFlux& potential,
    const std::vector<Vec3>& sample_positions) {
  if (!finite(potential) || sample_positions.empty()) return INFINITY;
  const MatchedEdgeField discrete_curl = matched_curl_adjoint(potential);
  double residual = 0.0;
  for (const auto& position : sample_positions) {
    if (!finite(position)) return INFINITY;
    const Vec3 interpolated = interpolate_quadratic_edge_field(
        discrete_curl, position);
    const Vec3 analytic = curl_interpolated_quadratic_face_potential(
        potential, position);
    residual = std::max({residual, std::abs(interpolated.x-analytic.x),
        std::abs(interpolated.y-analytic.y),
        std::abs(interpolated.z-analytic.z)});
  }
  return residual;
}

QuadraticCoatOrbitGatherResult evaluate_quadratic_coat_orbit_gather(
    const QuadraticCoatFaceCurrent& segment,
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    const Vec3& discrete_gradient_velocity,
    double temporal_scale,
    double beta,
    double polarity_scale) {
  if (!segment.valid || (segment.charge != -1 && segment.charge != 1)
      || electric.L != segment.L || magnetic.L != segment.L
      || !finite(electric) || !finite(magnetic)
      || !finite(discrete_gradient_velocity)
      || !(temporal_scale > 0.0) || !std::isfinite(beta)
      || !(polarity_scale > 0.0) || !std::isfinite(polarity_scale))
    return {};
  return evaluate_prevalidated(segment, electric, magnetic,
      discrete_gradient_velocity, temporal_scale, beta, polarity_scale);
}

QuadraticCoatOrbitGatherResult
evaluate_quadratic_coat_orbit_gather_prevalidated_fields(
    const QuadraticCoatFaceCurrent& segment,
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    const Vec3& discrete_gradient_velocity,
    double temporal_scale,
    double beta,
    double polarity_scale) {
  return evaluate_prevalidated(segment, electric, magnetic,
      discrete_gradient_velocity, temporal_scale, beta, polarity_scale);
}

std::vector<QuadraticCoatOrbitGatherResult>
evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_prevalidated_fields(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    const MatchedFaceFlux& fixed_electric,
    const MatchedFaceFlux& electric_pre_current,
    double current_scale,
    const MatchedEdgeField& magnetic,
    const std::vector<Vec3>& discrete_gradient_velocities,
    double temporal_scale,
    double beta,
    double polarity_scale) {
  std::vector<QuadraticCoatOrbitGatherResult> results;
  if (segments.empty() || segments.size() != discrete_gradient_velocities.size()
      || !std::isfinite(current_scale) || !(temporal_scale > 0.0)
      || !std::isfinite(temporal_scale) || !std::isfinite(beta)
      || !(polarity_scale > 0.0) || !std::isfinite(polarity_scale))
    return results;
  const int L = segments.front().L;
  if (!field_shape_valid(fixed_electric, L)
      || !field_shape_valid(electric_pre_current, L)
      || !field_shape_valid(magnetic, L)) return results;
  for (const auto& segment : segments)
    if (!segment.valid || segment.L != L || segment.dense_materialized)
      return results;

  results.reserve(segments.size());
  for (std::size_t item = 0; item < segments.size(); ++item) {
    const auto& segment = segments[item];
    QuadraticCoatOrbitGatherResult result;
    result.L = L;
    result.charge = segment.charge;
    result.start_effective_position = segment.start_effective_position;
    result.end_effective_position = segment.end_effective_position;
    result.displacement = result.end_effective_position
        -result.start_effective_position;
    result.discrete_gradient_velocity = discrete_gradient_velocities[item];
    result.temporal_scale = temporal_scale;
    result.beta = beta;
    if (!finite(result.discrete_gradient_velocity)) return {};
    const auto breaks = half_integer_breaks(
        result.start_effective_position, result.end_effective_position);
    result.quadrature_pieces = static_cast<int>(breaks.size())-1;
    const Vec3 electric_average = integrate_orbit(
        result.start_effective_position, result.end_effective_position, breaks,
        [&](const Vec3& position) {
          const Vec3 fixed{face_component_at(fixed_electric, 0, position),
                           face_component_at(fixed_electric, 1, position),
                           face_component_at(fixed_electric, 2, position)};
          const Vec3 prepared{
              face_component_at(electric_pre_current, 0, position),
              face_component_at(electric_pre_current, 1, position),
              face_component_at(electric_pre_current, 2, position)};
          return (fixed+prepared
              +sparse_field_at(segments, L, position)*current_scale)*0.5;
        });
    result.magnetic_average = integrate_orbit(
        result.start_effective_position, result.end_effective_position, breaks,
        [&magnetic](const Vec3& position) {
          return Vec3{edge_component_at(magnetic, 0, position),
                      edge_component_at(magnetic, 1, position),
                      edge_component_at(magnetic, 2, position)};
        });
    const double effective_charge = polarity_scale*segment.charge;
    result.electric_force = electric_average*effective_charge;
    long double current_work = 0.0L;
    for (const auto& entry : segment.sparse_current) {
      const int wrapped_index = segment.index(
          entry.face.x, entry.face.y, entry.face.z);
      current_work += static_cast<long double>(entry.value)
          *sparse_midpoint_coefficient(segments, fixed_electric,
              electric_pre_current, current_scale, entry.axis, wrapped_index);
    }
    result.current_work = polarity_scale*static_cast<double>(current_work);
    result.electric_work = result.displacement.dot(result.electric_force);
    result.electric_adjoint_residual = std::abs(
        result.current_work-result.electric_work);
    result.magnetic_impulse = Vec3::cross(
        result.discrete_gradient_velocity, result.magnetic_average)
        *(temporal_scale*beta*effective_charge);
    result.magnetic_work_residual = std::abs(
        result.discrete_gradient_velocity.dot(result.magnetic_impulse));
    const Vec3 kinematic = result.displacement
        -result.discrete_gradient_velocity*temporal_scale;
    result.kinematic_residual = std::max({std::abs(kinematic.x),
        std::abs(kinematic.y), std::abs(kinematic.z)});
    result.causal_excess = std::max(0.0,
        result.discrete_gradient_velocity.mag()-C_SPEED);
    result.valid = finite(result.electric_force)
        && finite(result.magnetic_average) && finite(result.magnetic_impulse)
        && std::isfinite(result.current_work)
        && result.electric_adjoint_residual <= 5e-13
        && result.magnetic_work_residual <= 5e-13
        && result.kinematic_residual <= 5e-13
        && result.causal_excess <= 5e-13;
    results.push_back(result);
  }
  return results;
}

}  // namespace ftd::eft

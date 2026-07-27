#include "ftd/eft/quadratic_coat_face_current.h"

#include "ftd/eft/local_polarity_regularity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder+L : remainder;
}

std::size_t flat_index(int L, int x, int y, int z) {
  const std::size_t wx = static_cast<std::size_t>(wrap(x, L));
  const std::size_t wy = static_cast<std::size_t>(wrap(y, L));
  const std::size_t wz = static_cast<std::size_t>(wrap(z, L));
  return (wx*static_cast<std::size_t>(L)+wy)*static_cast<std::size_t>(L)+wz;
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

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double nearest_periodic_image(double start, double end, int L) {
  return end+std::round((start-end)/static_cast<double>(L))*L;
}

double distance_to_interval(double value, double lower, double upper) {
  if (value < lower) return lower-value;
  if (value > upper) return value-upper;
  return 0.0;
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
    const double delta = component(end, axis)-p0;
    if (delta == 0.0) continue;
    const double lower = std::min(p0, p0+delta);
    const double upper = std::max(p0, p0+delta);
    const int first = static_cast<int>(std::floor(lower))-2;
    const int last = static_cast<int>(std::ceil(upper))+2;
    for (int k = first; k <= last; ++k) {
      const double plane = static_cast<double>(k)+0.5;
      const double t = (plane-p0)/delta;
      if (t > 0.0 && t < 1.0) breaks.push_back(t);
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

double axis_basis_integral(int axis,
                           Coord face,
                           const Vec3& start,
                           const Vec3& delta,
                           const std::vector<double>& breaks) {
  constexpr long double node = 0.774596669241483377035853079956L;
  constexpr std::array<long double, 3> nodes{{-node, 0.0L, node}};
  constexpr std::array<long double, 3> weights{{
      5.0L/9.0L, 8.0L/9.0L, 5.0L/9.0L}};
  long double integral = 0.0L;
  for (std::size_t piece = 1; piece < breaks.size(); ++piece) {
    const long double ta = breaks[piece-1];
    const long double tb = breaks[piece];
    const long double midpoint = 0.5L*(ta+tb);
    const long double half_width = 0.5L*(tb-ta);
    long double piece_sum = 0.0L;
    for (std::size_t sample = 0; sample < nodes.size(); ++sample) {
      const double t = static_cast<double>(
          midpoint+half_width*nodes[sample]);
      const Vec3 point = start+delta*t;
      long double basis = 1.0L;
      for (int d = 0; d < 3; ++d) {
        const int coordinate = d == 0 ? face.x : (d == 1 ? face.y : face.z);
        const double center = static_cast<double>(coordinate)
            +(d == axis ? 0.5 : 0.0);
        const double factor = d == axis
            ? b1(component(point, d)-center)
            : b2(component(point, d)-center);
        basis *= static_cast<long double>(factor);
      }
      piece_sum += weights[sample]*basis;
    }
    integral += half_width*piece_sum;
  }
  return static_cast<double>(integral);
}

void deposit_coat(const QuadraticPolarityCoat& coat,
                  int L,
                  std::vector<double>& field) {
  for (std::size_t i = 0; i < coat.weight_count; ++i) {
    const auto& entry = coat.weights[i];
    field[flat_index(L, entry.site.x, entry.site.y, entry.site.z)]
        += entry.weight;
  }
}

double sum(const std::vector<double>& field) {
  long double result = 0.0L;
  for (double value : field) result += static_cast<long double>(value);
  return static_cast<double>(result);
}

int support_count(const std::vector<double>& field) {
  return static_cast<int>(std::count_if(field.begin(), field.end(),
      [](double value) { return value != 0.0; }));
}

}  // namespace

QuadraticPolarityCoat make_quadratic_polarity_coat(
    const Vec3& effective_position, int polarity) {
  QuadraticPolarityCoat result;
  result.effective_position = effective_position;
  result.polarity = polarity;
  if (!finite(effective_position) || (polarity != -1 && polarity != 1))
    return result;

  const int lower_x = static_cast<int>(std::floor(effective_position.x))-2;
  const int lower_y = static_cast<int>(std::floor(effective_position.y))-2;
  const int lower_z = static_cast<int>(std::floor(effective_position.z))-2;
  long double partition = 0.0L;
  long double moment_x = 0.0L;
  long double moment_y = 0.0L;
  long double moment_z = 0.0L;
  double minimum_unsigned = std::numeric_limits<double>::infinity();
  for (int x = lower_x; x <= lower_x+4; ++x) {
    const double wx = b2(effective_position.x-x);
    if (wx == 0.0) continue;
    for (int y = lower_y; y <= lower_y+4; ++y) {
      const double wy = b2(effective_position.y-y);
      if (wy == 0.0) continue;
      for (int z = lower_z; z <= lower_z+4; ++z) {
        const double wz = b2(effective_position.z-z);
        if (wz == 0.0) continue;
        if (result.weight_count >= result.weights.size()) return result;
        const double unsigned_weight = wx*wy*wz;
        const double weight = polarity*unsigned_weight;
        result.weights[result.weight_count++] = {{x, y, z}, weight};
        minimum_unsigned = std::min(minimum_unsigned, unsigned_weight);
        result.locality_residual = std::max({
            result.locality_residual,
            std::max(0.0, std::abs(effective_position.x-x)-1.5),
            std::max(0.0, std::abs(effective_position.y-y)-1.5),
            std::max(0.0, std::abs(effective_position.z-z)-1.5)});
        partition += static_cast<long double>(weight);
        moment_x += static_cast<long double>(weight*x);
        moment_y += static_cast<long double>(weight*y);
        moment_z += static_cast<long double>(weight*z);
      }
    }
  }
  result.partition_residual = static_cast<double>(
      partition-static_cast<long double>(polarity));
  result.first_moment_residual = {
      static_cast<double>(moment_x-polarity*effective_position.x),
      static_cast<double>(moment_y-polarity*effective_position.y),
      static_cast<double>(moment_z-polarity*effective_position.z)};
  result.minimum_unsigned_weight = std::isfinite(minimum_unsigned)
      ? minimum_unsigned : 0.0;
  result.valid = result.weight_count > 0
      && result.weight_count <= result.weights.size()
      && result.minimum_unsigned_weight >= 0.0
      && std::abs(result.partition_residual) <= 1e-12
      && std::abs(result.first_moment_residual.x) <= 1e-12
      && std::abs(result.first_moment_residual.y) <= 1e-12
      && std::abs(result.first_moment_residual.z) <= 1e-12
      && result.locality_residual <= 1e-12;
  return result;
}

int QuadraticCoatFaceCurrent::index(int x, int y, int z) const {
  if (L <= 0) return -1;
  return static_cast<int>(flat_index(L, x, y, z));
}

double quadratic_coat_current_divergence_at(
    const QuadraticCoatFaceCurrent& segment, int x, int y, int z) {
  if (segment.L <= 0 || segment.current_x.empty()) return NAN;
  const auto at = [&segment](const std::vector<double>& field,
                             int sx, int sy, int sz) {
    return field[flat_index(segment.L, sx, sy, sz)];
  };
  return at(segment.current_x, x, y, z)
      -at(segment.current_x, x-1, y, z)
      +at(segment.current_y, x, y, z)
      -at(segment.current_y, x, y-1, z)
      +at(segment.current_z, x, y, z)
      -at(segment.current_z, x, y, z-1);
}

double quadratic_coat_continuity_at(
    const QuadraticCoatFaceCurrent& segment, int x, int y, int z) {
  if (segment.L <= 0 || segment.rho_before.empty()) return NAN;
  const auto index = flat_index(segment.L, x, y, z);
  return segment.rho_after[index]-segment.rho_before[index]
      +quadratic_coat_current_divergence_at(segment, x, y, z);
}

QuadraticCoatFaceCurrent make_quadratic_coat_face_current(
    int L,
    const Vec3& start_effective_position,
    const Vec3& raw_end_effective_position,
    int charge) {
  QuadraticCoatFaceCurrent result;
  result.L = L;
  result.charge = charge;
  result.start_effective_position = start_effective_position;
  if (L < 5 || !finite(start_effective_position)
      || !finite(raw_end_effective_position)
      || (charge != -1 && charge != 1)) return result;
  result.end_effective_position = {
      nearest_periodic_image(start_effective_position.x,
                             raw_end_effective_position.x, L),
      nearest_periodic_image(start_effective_position.y,
                             raw_end_effective_position.y, L),
      nearest_periodic_image(start_effective_position.z,
                             raw_end_effective_position.z, L)};
  const Vec3 delta = result.end_effective_position
      -result.start_effective_position;
  result.causal_excess = std::max({0.0, std::abs(delta.x)-1.0,
      std::abs(delta.y)-1.0, std::abs(delta.z)-1.0});
  if (result.causal_excess > 1e-12) return result;

  result.start_coat = make_quadratic_polarity_coat(
      result.start_effective_position, charge);
  result.end_coat = make_quadratic_polarity_coat(
      result.end_effective_position, charge);
  if (!result.start_coat.valid || !result.end_coat.valid) return result;

  const std::size_t side = static_cast<std::size_t>(L);
  if (side > std::numeric_limits<std::size_t>::max()/side
      || side*side > std::numeric_limits<std::size_t>::max()/side)
    return result;
  const std::size_t volume = side*side*side;
  result.rho_before.assign(volume, 0.0);
  result.rho_after.assign(volume, 0.0);
  result.current_x.assign(volume, 0.0);
  result.current_y.assign(volume, 0.0);
  result.current_z.assign(volume, 0.0);
  deposit_coat(result.start_coat, L, result.rho_before);
  deposit_coat(result.end_coat, L, result.rho_after);

  const auto breaks = half_integer_breaks(
      result.start_effective_position, result.end_effective_position);
  const int lower[3] = {
      static_cast<int>(std::floor(std::min(
          result.start_effective_position.x,
          result.end_effective_position.x)))-3,
      static_cast<int>(std::floor(std::min(
          result.start_effective_position.y,
          result.end_effective_position.y)))-3,
      static_cast<int>(std::floor(std::min(
          result.start_effective_position.z,
          result.end_effective_position.z)))-3};
  const int upper[3] = {
      static_cast<int>(std::ceil(std::max(
          result.start_effective_position.x,
          result.end_effective_position.x)))+3,
      static_cast<int>(std::ceil(std::max(
          result.start_effective_position.y,
          result.end_effective_position.y)))+3,
      static_cast<int>(std::ceil(std::max(
          result.start_effective_position.z,
          result.end_effective_position.z)))+3};

  for (int axis = 0; axis < 3; ++axis) {
    const double axis_delta = component(delta, axis);
    if (axis_delta == 0.0) continue;
    std::vector<double>* field = axis == 0 ? &result.current_x
        : (axis == 1 ? &result.current_y : &result.current_z);
    for (int i = lower[0]; i <= upper[0]; ++i) {
      for (int j = lower[1]; j <= upper[1]; ++j) {
        for (int k = lower[2]; k <= upper[2]; ++k) {
          const Coord face{i, j, k};
          const double basis_integral = axis_basis_integral(
              axis, face, result.start_effective_position, delta, breaks);
          const double deposited = charge*axis_delta*basis_integral;
          if (deposited == 0.0) continue;
          (*field)[flat_index(L, i, j, k)] += deposited;
          for (int d = 0; d < 3; ++d) {
            const int coordinate = d == 0 ? i : (d == 1 ? j : k);
            const double center = static_cast<double>(coordinate)
                +(d == axis ? 0.5 : 0.0);
            const double lower_position = std::min(
                component(result.start_effective_position, d),
                component(result.end_effective_position, d));
            const double upper_position = std::max(
                component(result.start_effective_position, d),
                component(result.end_effective_position, d));
            const double support_radius = d == axis ? 1.0 : 1.5;
            result.locality_residual = std::max(
                result.locality_residual,
                std::max(0.0, distance_to_interval(
                    center, lower_position, upper_position)-support_radius));
          }
        }
      }
    }
  }

  result.rho_support = support_count(result.rho_before)
      +support_count(result.rho_after);
  result.current_support = support_count(result.current_x)
      +support_count(result.current_y)+support_count(result.current_z);
  result.partition_residual = std::max(
      std::abs(sum(result.rho_before)-charge),
      std::abs(sum(result.rho_after)-charge));
  result.first_moment_residual = std::max({
      std::abs(result.start_coat.first_moment_residual.x),
      std::abs(result.start_coat.first_moment_residual.y),
      std::abs(result.start_coat.first_moment_residual.z),
      std::abs(result.end_coat.first_moment_residual.x),
      std::abs(result.end_coat.first_moment_residual.y),
      std::abs(result.end_coat.first_moment_residual.z)});
  result.current_moment_residual = std::max({
      std::abs(sum(result.current_x)-charge*delta.x),
      std::abs(sum(result.current_y)-charge*delta.y),
      std::abs(sum(result.current_z)-charge*delta.z)});
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z)
        result.continuity_residual = std::max(
            result.continuity_residual,
            std::abs(quadratic_coat_continuity_at(result, x, y, z)));
  result.locality_residual = std::max({result.locality_residual,
      result.start_coat.locality_residual, result.end_coat.locality_residual});
  result.valid = result.partition_residual <= 1e-12
      && result.first_moment_residual <= 1e-12
      && result.continuity_residual <= 1e-12
      && result.current_moment_residual <= 1e-12
      && result.locality_residual <= 1e-12
      && result.causal_excess <= 1e-12;
  return result;
}

double quadratic_coat_connection_coupling(
    const QuadraticCoatFaceCurrent& segment,
    const std::vector<double>& potential_x,
    const std::vector<double>& potential_y,
    const std::vector<double>& potential_z) {
  if (!segment.valid || potential_x.size() != segment.current_x.size()
      || potential_y.size() != segment.current_y.size()
      || potential_z.size() != segment.current_z.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < segment.current_x.size(); ++i) {
    result += static_cast<long double>(potential_x[i])*segment.current_x[i]
        +static_cast<long double>(potential_y[i])*segment.current_y[i]
        +static_cast<long double>(potential_z[i])*segment.current_z[i];
  }
  return static_cast<double>(result);
}

}  // namespace ftd::eft

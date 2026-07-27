#include "ftd/eft/face_current_segment.h"

#include <algorithm>
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

double nearest_periodic_image(double start, double end, int L) {
  return end + std::round((start - end) / static_cast<double>(L)) * L;
}

double hat_weight(double position, int site) {
  return std::max(0.0, 1.0 - std::abs(position - site));
}

long double integrated_linear_product(
    double a0, double a1, double b0, double b1) {
  const long double a = a0;
  const long double da = static_cast<long double>(a1) - a0;
  const long double b = b0;
  const long double db = static_cast<long double>(b1) - b0;
  return a * b + 0.5L * (a * db + da * b) + da * db / 3.0L;
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
      const double t = (static_cast<double>(plane) - p0) / delta;
      if (t > 0.0 && t < 1.0) breaks.push_back(t);
    }
  }
  std::sort(breaks.begin(), breaks.end());
  breaks.erase(std::unique(breaks.begin(), breaks.end(),
      [](double a, double b) {
        return std::abs(a - b) <= 32.0
            * std::numeric_limits<double>::epsilon();
      }), breaks.end());
  return breaks;
}

Vec3 point_on_segment(const Vec3& start, const Vec3& delta, double t) {
  return start + delta * t;
}

void deposit_shape(const SubcellPolarityShape& shape,
                   int L,
                   std::vector<double>& rho,
                   std::vector<unsigned char>& support) {
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    const auto& entry = shape.weights[i];
    const std::size_t index = flat_index(
        L, entry.site.x, entry.site.y, entry.site.z);
    rho[index] += entry.weight;
    support[index] = 1;
  }
}

void deposit_axis_piece(
    FaceCurrentSegment& result,
    int axis,
    const Vec3& start,
    const Vec3& delta,
    double ta,
    double tb,
    std::vector<unsigned char>& support) {
  const double axis_delta = component(delta, axis);
  if (axis_delta == 0.0 || tb <= ta) return;

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

  std::vector<double>* field = axis == 0 ? &result.current_x
      : (axis == 1 ? &result.current_y : &result.current_z);
  for (int da = 0; da <= 1; ++da) {
    const int site_a = lower_a + da;
    const double wa0 = hat_weight(component(pa, transverse_a), site_a);
    const double wa1 = hat_weight(component(pb, transverse_a), site_a);
    for (int db = 0; db <= 1; ++db) {
      const int site_b = lower_b + db;
      const double wb0 = hat_weight(component(pa, transverse_b), site_b);
      const double wb1 = hat_weight(component(pb, transverse_b), site_b);
      const long double transverse_integral = integrated_linear_product(
          wa0, wa1, wb0, wb1);
      const long double deposited = static_cast<long double>(result.charge)
          * axis_delta * (tb - ta) * transverse_integral;

      Coord face{};
      face = with_component(face, axis, face_coordinate);
      face = with_component(face, transverse_a, site_a);
      face = with_component(face, transverse_b, site_b);
      const std::size_t index = flat_index(
          result.L, face.x, face.y, face.z);
      (*field)[index] += static_cast<double>(deposited);
      support[index] = 1;
    }
  }
}

double sum_field(const std::vector<double>& field) {
  long double sum = 0.0L;
  for (double value : field) sum += value;
  return static_cast<double>(sum);
}

double outside_support_residual(
    const std::vector<double>& field,
    const std::vector<unsigned char>& support) {
  double residual = 0.0;
  for (std::size_t i = 0; i < field.size(); ++i) {
    if (support[i] == 0) residual = std::max(residual, std::abs(field[i]));
  }
  return residual;
}

int count_support(const std::vector<double>& field) {
  return static_cast<int>(std::count_if(
      field.begin(), field.end(), [](double value) { return value != 0.0; }));
}

}  // namespace

int FaceCurrentSegment::index(int x, int y, int z) const {
  if (L <= 0) return -1;
  return static_cast<int>(flat_index(L, x, y, z));
}

double face_current_divergence_at(
    const FaceCurrentSegment& segment, int x, int y, int z) {
  if (segment.L <= 0 || segment.current_x.empty()) return NAN;
  const auto at = [&segment](const std::vector<double>& field,
                             int sx, int sy, int sz) {
    return field[flat_index(segment.L, sx, sy, sz)];
  };
  return at(segment.current_x, x, y, z)
      - at(segment.current_x, x - 1, y, z)
      + at(segment.current_y, x, y, z)
      - at(segment.current_y, x, y - 1, z)
      + at(segment.current_z, x, y, z)
      - at(segment.current_z, x, y, z - 1);
}

double face_current_continuity_at(
    const FaceCurrentSegment& segment, int x, int y, int z) {
  if (segment.L <= 0 || segment.rho_before.empty()) return NAN;
  const std::size_t index = flat_index(segment.L, x, y, z);
  return segment.rho_after[index] - segment.rho_before[index]
      + face_current_divergence_at(segment, x, y, z);
}

double max_face_current_continuity_residual(
    const FaceCurrentSegment& segment) {
  if (segment.L <= 0 || segment.rho_before.empty()) return INFINITY;
  double residual = 0.0;
  for (int x = 0; x < segment.L; ++x) {
    for (int y = 0; y < segment.L; ++y) {
      for (int z = 0; z < segment.L; ++z) {
        residual = std::max(residual,
            std::abs(face_current_continuity_at(segment, x, y, z)));
      }
    }
  }
  return residual;
}

FaceCurrentSegment make_face_current_segment(
    int L,
    Coord start_anchor,
    const Vec3& start_remainder,
    Coord end_anchor,
    const Vec3& end_remainder,
    int charge) {
  FaceCurrentSegment result;
  result.L = L;
  result.charge = charge;
  result.start_anchor = start_anchor;
  result.end_anchor = end_anchor;
  result.start_remainder = start_remainder;
  result.end_remainder = end_remainder;

  if (L < 2 || (charge != -1 && charge != 1)) return result;
  result.start_shape = make_subcell_polarity_shape(
      start_anchor, start_remainder, charge);
  const SubcellPolarityShape raw_end_shape = make_subcell_polarity_shape(
      end_anchor, end_remainder, charge);
  if (!result.start_shape.valid || !raw_end_shape.valid) return result;

  const std::size_t side = static_cast<std::size_t>(L);
  if (side > std::numeric_limits<std::size_t>::max() / side
      || side * side > std::numeric_limits<std::size_t>::max() / side) {
    return result;
  }
  const std::size_t volume = side * side * side;
  result.rho_before.assign(volume, 0.0);
  result.rho_after.assign(volume, 0.0);
  result.current_x.assign(volume, 0.0);
  result.current_y.assign(volume, 0.0);
  result.current_z.assign(volume, 0.0);

  result.start_effective_position = result.start_shape.effective_position;
  result.end_effective_position = {
      nearest_periodic_image(result.start_effective_position.x,
                             raw_end_shape.effective_position.x, L),
      nearest_periodic_image(result.start_effective_position.y,
                             raw_end_shape.effective_position.y, L),
      nearest_periodic_image(result.start_effective_position.z,
                             raw_end_shape.effective_position.z, L)};

  const Coord unwrapped_end_anchor{
      static_cast<int>(std::llround(
          result.end_effective_position.x - end_remainder.x)),
      static_cast<int>(std::llround(
          result.end_effective_position.y - end_remainder.y)),
      static_cast<int>(std::llround(
          result.end_effective_position.z - end_remainder.z))};
  result.end_shape = make_subcell_polarity_shape(
      unwrapped_end_anchor, end_remainder, charge);
  if (!result.start_shape.valid || !result.end_shape.valid) return result;

  std::vector<unsigned char> rho_support(volume, 0);
  std::vector<unsigned char> x_support(volume, 0);
  std::vector<unsigned char> y_support(volume, 0);
  std::vector<unsigned char> z_support(volume, 0);
  deposit_shape(result.start_shape, L, result.rho_before, rho_support);
  deposit_shape(result.end_shape, L, result.rho_after, rho_support);

  const Vec3 delta = result.end_effective_position
      - result.start_effective_position;
  const std::vector<double> breaks = segment_breaks(
      result.start_effective_position, result.end_effective_position);
  for (std::size_t i = 1; i < breaks.size(); ++i) {
    const double ta = breaks[i - 1];
    const double tb = breaks[i];
    deposit_axis_piece(result, 0, result.start_effective_position,
                       delta, ta, tb, x_support);
    deposit_axis_piece(result, 1, result.start_effective_position,
                       delta, ta, tb, y_support);
    deposit_axis_piece(result, 2, result.start_effective_position,
                       delta, ta, tb, z_support);
  }

  result.rho_support = count_support(result.rho_before)
      + count_support(result.rho_after);
  result.current_support = count_support(result.current_x)
      + count_support(result.current_y) + count_support(result.current_z);
  result.partition_residual = std::max(
      std::abs(sum_field(result.rho_before) - charge),
      std::abs(sum_field(result.rho_after) - charge));
  result.first_moment_residual = std::max(
      max_first_moment_residual(result.start_shape),
      max_first_moment_residual(result.end_shape));
  result.continuity_residual = max_face_current_continuity_residual(result);
  result.locality_residual = std::max({
      outside_support_residual(result.rho_before, rho_support),
      outside_support_residual(result.rho_after, rho_support),
      outside_support_residual(result.current_x, x_support),
      outside_support_residual(result.current_y, y_support),
      outside_support_residual(result.current_z, z_support)});
  result.valid = std::isfinite(result.partition_residual)
      && std::isfinite(result.first_moment_residual)
      && std::isfinite(result.continuity_residual)
      && std::isfinite(result.locality_residual)
      && result.partition_residual <= 1e-12
      && result.first_moment_residual <= 1e-12
      && result.continuity_residual <= 1e-12
      && result.locality_residual <= 1e-12;
  return result;
}

}  // namespace ftd::eft

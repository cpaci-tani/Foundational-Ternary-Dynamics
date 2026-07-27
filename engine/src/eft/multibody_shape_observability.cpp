#include "ftd/eft/multibody_shape_observability.h"

#include "ftd/eft/canonical_subcell_section.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

int wrap(int value, int L) {
  value %= L;
  return value < 0 ? value + L : value;
}

int flat_index(int L, int x, int y, int z) {
  return (wrap(x, L) * L + wrap(y, L)) * L + wrap(z, L);
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double field_l1(const std::vector<double>& field) {
  double result = 0.0;
  for (double value : field) result += std::abs(value);
  return result;
}

}  // namespace

OneDimensionalCICMoment one_dimensional_cic_moment(
    const std::vector<double>& fractions,
    const std::vector<int>& charges) {
  OneDimensionalCICMoment result;
  if (fractions.size() != charges.size() || fractions.empty()) return result;
  for (std::size_t i = 0; i < fractions.size(); ++i) {
    if (!std::isfinite(fractions[i]) || fractions[i] < 0.0
        || fractions[i] > 1.0
        || (charges[i] != -1 && charges[i] != +1)) {
      return result;
    }
    result.signed_charge += charges[i];
    result.signed_first_moment += charges[i] * fractions[i];
  }
  result.upper_weight = result.signed_first_moment;
  result.lower_weight = static_cast<double>(result.signed_charge)
      - result.signed_first_moment;
  result.valid = true;
  return result;
}

int AggregateShapeCurrent::index(int x, int y, int z) const {
  return flat_index(L, x, y, z);
}

AggregateShapeCurrent make_aggregate_shape_current(
    int L,
    const std::vector<ShapeWorldline>& worldlines) {
  AggregateShapeCurrent result;
  result.L = L;
  result.particle_count = static_cast<int>(worldlines.size());
  if (L < 3 || worldlines.empty()) return result;
  const std::size_t side = static_cast<std::size_t>(L);
  if (side > static_cast<std::size_t>(-1) / side / side) return result;
  const std::size_t volume = side * side * side;
  result.rho_before.assign(volume, 0.0);
  result.rho_after.assign(volume, 0.0);
  result.current_x.assign(volume, 0.0);
  result.current_y.assign(volume, 0.0);
  result.current_z.assign(volume, 0.0);

  bool segments_valid = true;
  Vec3 unsigned_sum_before{};
  Vec3 unsigned_sum_after{};
  for (const auto& worldline : worldlines) {
    if ((worldline.charge != -1 && worldline.charge != +1)
        || !finite(worldline.start_position)
        || !finite(worldline.end_position)) {
      return result;
    }
    const auto start = centered_canonical_subcell_chart(
        worldline.start_position);
    const auto end = centered_canonical_subcell_chart(
        worldline.end_position);
    if (!start.valid || !end.valid) return result;
    const auto segment = make_face_current_segment(
        L, start.anchor, start.remainder,
        end.anchor, end.remainder, worldline.charge);
    if (!segment.valid || segment.rho_before.size() != volume) {
      segments_valid = false;
      continue;
    }
    result.total_charge += worldline.charge;
    result.signed_first_moment_before +=
        worldline.start_position * static_cast<double>(worldline.charge);
    result.signed_first_moment_after +=
        worldline.end_position * static_cast<double>(worldline.charge);
    unsigned_sum_before += worldline.start_position;
    unsigned_sum_after += worldline.end_position;
    for (std::size_t i = 0; i < volume; ++i) {
      result.rho_before[i] += segment.rho_before[i];
      result.rho_after[i] += segment.rho_after[i];
      result.current_x[i] += segment.current_x[i];
      result.current_y[i] += segment.current_y[i];
      result.current_z[i] += segment.current_z[i];
    }
    result.constituent_current_l1 += field_l1(segment.current_x)
        + field_l1(segment.current_y) + field_l1(segment.current_z);
  }
  if (!segments_valid) return result;

  const double count = static_cast<double>(result.particle_count);
  result.unsigned_center_before = unsigned_sum_before * (1.0 / count);
  result.unsigned_center_after = unsigned_sum_after * (1.0 / count);
  result.aggregate_current_l1 = field_l1(result.current_x)
      + field_l1(result.current_y) + field_l1(result.current_z);

  double continuity = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto at = [&result](const std::vector<double>& field,
                                  int sx, int sy, int sz) {
          return field[static_cast<std::size_t>(
              result.index(sx, sy, sz))];
        };
        const int index = result.index(x, y, z);
        const double divergence =
            at(result.current_x, x, y, z)
            - at(result.current_x, x - 1, y, z)
            + at(result.current_y, x, y, z)
            - at(result.current_y, x, y - 1, z)
            + at(result.current_z, x, y, z)
            - at(result.current_z, x, y, z - 1);
        continuity = std::max(
            continuity,
            std::abs(result.rho_after[static_cast<std::size_t>(index)]
                     - result.rho_before[static_cast<std::size_t>(index)]
                     + divergence));
      }
    }
  }
  result.aggregate_continuity_residual = continuity;
  result.valid = std::isfinite(continuity) && continuity <= 1e-12;
  return result;
}

double two_body_squared_separation(
    const std::vector<ShapeWorldline>& worldlines,
    bool use_end_positions) {
  if (worldlines.size() != 2) return NAN;
  const Vec3 lhs = use_end_positions
      ? worldlines[0].end_position : worldlines[0].start_position;
  const Vec3 rhs = use_end_positions
      ? worldlines[1].end_position : worldlines[1].start_position;
  const Vec3 displacement = rhs - lhs;
  return displacement.mag2();
}

}  // namespace ftd::eft

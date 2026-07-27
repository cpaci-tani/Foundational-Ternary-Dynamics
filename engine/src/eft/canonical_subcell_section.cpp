#include "ftd/eft/canonical_subcell_section.h"

#include <algorithm>
#include <climits>
#include <cmath>

namespace ftd::eft {
namespace {

struct CanonicalAxis {
  int anchor = 0;
  double remainder = 0.0;
  bool valid = false;
};

CanonicalAxis canonical_axis(double position) {
  CanonicalAxis result;
  if (!std::isfinite(position)) return result;
  const double anchor_value = std::floor(position + 0.5);
  if (anchor_value < static_cast<double>(INT_MIN)
      || anchor_value > static_cast<double>(INT_MAX)) {
    return result;
  }
  result.anchor = static_cast<int>(anchor_value);
  result.remainder = position - anchor_value;
  // Absorb tiny arithmetic excursions at the included lower boundary only.
  if (result.remainder < -0.5
      && result.remainder >= -0.5 - 8.0e-15) {
    result.remainder = -0.5;
  }
  result.valid = result.remainder >= -0.5
      && result.remainder < 0.5;
  return result;
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

}  // namespace

SubcellChart centered_canonical_subcell_chart(
    const Vec3& effective_position) {
  SubcellChart result;
  const auto x = canonical_axis(effective_position.x);
  const auto y = canonical_axis(effective_position.y);
  const auto z = canonical_axis(effective_position.z);
  if (!x.valid || !y.valid || !z.valid) return result;
  result.anchor = {x.anchor, y.anchor, z.anchor};
  result.remainder = {x.remainder, y.remainder, z.remainder};
  result.valid = true;
  return result;
}

SubcellChart translate_centered_canonical_chart(
    const SubcellChart& start,
    const Vec3& displacement) {
  if (!start.valid || !finite(displacement)) return {};
  return centered_canonical_subcell_chart(
      subcell_chart_position(start) + displacement);
}

HalfCellSectionObstruction analyze_half_cell_section_obstruction() {
  HalfCellSectionObstruction result;
  const auto positive = centered_canonical_subcell_chart({0.5, 0.0, 0.0});
  const auto negative = centered_canonical_subcell_chart({-0.5, 0.0, 0.0});
  if (!positive.valid || !negative.valid) return result;

  result.positive_anchor = positive.anchor.x;
  result.negative_anchor = negative.anchor.x;
  result.translation_predicted_negative_anchor = positive.anchor.x - 1;
  result.inversion_predicted_negative_anchor = -positive.anchor.x;
  result.diophantine_residual = std::abs(2 * positive.anchor.x - 1);
  result.integer_solution_exists = false;
  result.raw_anchor_mismatch = std::abs(static_cast<double>(
      negative.anchor.x + positive.anchor.x));
  result.raw_remainder_mismatch = std::abs(
      negative.remainder.x + positive.remainder.x);
  const Vec3 inverted_positive{
      -subcell_chart_position(positive).x,
      -subcell_chart_position(positive).y,
      -subcell_chart_position(positive).z};
  const Vec3 selected_negative = subcell_chart_position(negative);
  result.physical_inversion_residual = std::max({
      std::abs(inverted_positive.x - selected_negative.x),
      std::abs(inverted_positive.y - selected_negative.y),
      std::abs(inverted_positive.z - selected_negative.z)});
  result.valid = result.translation_predicted_negative_anchor
          != result.inversion_predicted_negative_anchor
      && result.diophantine_residual > 0;
  return result;
}

}  // namespace ftd::eft

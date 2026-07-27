/**
 * Focused algebra tests for the isolated sub-cell polarity shape.
 */

#include "ftd/eft/subcell_polarity_shape.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

bool close(double a, double b, double tolerance = 1e-14) {
  return std::abs(a - b) <= tolerance;
}

double weight_at(const ftd::eft::SubcellPolarityShape& shape,
                 int x, int y, int z) {
  double value = 0.0;
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    const auto& entry = shape.weights[i];
    if (entry.site.x == x && entry.site.y == y && entry.site.z == z) {
      value += entry.weight;
    }
  }
  return value;
}

}  // namespace

int main() {
  constexpr double gate = 1e-12;
  const ftd::Coord anchor{4, -3, 7};
  const ftd::Vec3 remainder{0.25, -0.5, 0.75};
  const auto positive = ftd::eft::make_subcell_polarity_shape(
      anchor, remainder, +1);
  const auto negative = ftd::eft::make_subcell_polarity_shape(
      anchor, remainder, -1);

  check("positive signed-octant shape valid",
        positive.valid && positive.weight_count == 8);
  check("negative signed-octant shape valid",
        negative.valid && negative.weight_count == 8);
  check("effective position is anchor plus signed remainder",
        close(positive.effective_position.x, 4.25)
        && close(positive.effective_position.y, -3.5)
        && close(positive.effective_position.z, 7.75));
  check("both polarities partition exactly",
        std::abs(positive.partition_residual) <= gate
        && std::abs(negative.partition_residual) <= gate);
  check("both polarities reproduce the signed first moment",
        ftd::eft::max_first_moment_residual(positive) <= gate
        && ftd::eft::max_first_moment_residual(negative) <= gate);

  bool compact_octant = true;
  bool polarity_pair = true;
  for (std::size_t i = 0; i < positive.weight_count; ++i) {
    const auto& entry = positive.weights[i];
    compact_octant = compact_octant
        && (entry.site.x == anchor.x || entry.site.x == anchor.x + 1)
        && (entry.site.y == anchor.y || entry.site.y == anchor.y - 1)
        && (entry.site.z == anchor.z || entry.site.z == anchor.z + 1)
        && entry.weight > 0.0;
    polarity_pair = polarity_pair
        && close(weight_at(negative, entry.site.x, entry.site.y, entry.site.z),
                 -entry.weight);
  }
  check("support is confined to the selected signed octant", compact_octant);
  check("negative polarity is the exact signed partner", polarity_pair);

  const auto plus_threshold = ftd::eft::make_subcell_polarity_shape(
      {2, 3, 4}, {1.0, 0.0, 0.0}, +1);
  const auto minus_threshold = ftd::eft::make_subcell_polarity_shape(
      {2, 3, 4}, {-1.0, 0.0, 0.0}, -1);
  check("+1 threshold transfers all weight to positive neighbour",
        plus_threshold.valid && plus_threshold.weight_count == 1
        && close(weight_at(plus_threshold, 3, 3, 4), 1.0));
  check("-1 threshold transfers all signed weight to negative neighbour",
        minus_threshold.valid && minus_threshold.weight_count == 1
        && close(weight_at(minus_threshold, 1, 3, 4), -1.0));

  const ftd::Coord shift{3, 5, -2};
  const auto translated = ftd::eft::make_subcell_polarity_shape(
      {anchor.x + shift.x, anchor.y + shift.y, anchor.z + shift.z},
      remainder, +1);
  bool translation_covariant = translated.valid;
  for (std::size_t i = 0; i < positive.weight_count; ++i) {
    const auto& entry = positive.weights[i];
    translation_covariant = translation_covariant
        && close(weight_at(translated,
                           entry.site.x + shift.x,
                           entry.site.y + shift.y,
                           entry.site.z + shift.z),
                 entry.weight);
  }
  check("integer translation preserves all coefficients",
        translation_covariant);

  const auto permuted = ftd::eft::make_subcell_polarity_shape(
      {anchor.y, anchor.z, anchor.x},
      {remainder.y, remainder.z, remainder.x}, +1);
  bool permutation_covariant = permuted.valid;
  for (std::size_t i = 0; i < positive.weight_count; ++i) {
    const auto& entry = positive.weights[i];
    permutation_covariant = permutation_covariant
        && close(weight_at(permuted,
                           entry.site.y, entry.site.z, entry.site.x),
                 entry.weight);
  }
  check("cyclic cubic-axis permutation preserves the shape",
        permutation_covariant);

  const auto inverted = ftd::eft::make_subcell_polarity_shape(
      {-anchor.x, -anchor.y, -anchor.z},
      {-remainder.x, -remainder.y, -remainder.z}, +1);
  bool inversion_covariant = inverted.valid;
  for (std::size_t i = 0; i < positive.weight_count; ++i) {
    const auto& entry = positive.weights[i];
    inversion_covariant = inversion_covariant
        && close(weight_at(inverted,
                           -entry.site.x, -entry.site.y, -entry.site.z),
                 entry.weight);
  }
  check("cubic inversion maps support and coefficients exactly",
        inversion_covariant);

  check("zero polarity is rejected",
        !ftd::eft::make_subcell_polarity_shape(
            anchor, remainder, 0).valid);
  check("remainder beyond a movement threshold is rejected",
        !ftd::eft::make_subcell_polarity_shape(
            anchor, {1.0 + 1e-9, 0.0, 0.0}, +1).valid);

  const double worst_partition = std::max(
      std::abs(positive.partition_residual),
      std::abs(negative.partition_residual));
  const double worst_first_moment = std::max(
      ftd::eft::max_first_moment_residual(positive),
      ftd::eft::max_first_moment_residual(negative));
  std::cout.precision(17);
  std::cout << "worst_partition_residual=" << worst_partition << '\n'
            << "worst_first_moment_residual=" << worst_first_moment << '\n';
  std::cout << "subcell_polarity_shape failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

/**
 * Focused continuity, locality, and cubic-covariance tests for exact
 * straight-segment face-current deposition.
 */

#include "ftd/eft/face_current_segment.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

bool close(double a, double b, double tolerance = 1e-13) {
  return std::abs(a - b) <= tolerance;
}

double sum(const std::vector<double>& field) {
  return std::accumulate(field.begin(), field.end(), 0.0);
}

double max_difference(const std::vector<double>& a,
                      const std::vector<double>& b) {
  if (a.size() != b.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    residual = std::max(residual, std::abs(a[i] - b[i]));
  }
  return residual;
}

double max_opposite_residual(const std::vector<double>& a,
                             const std::vector<double>& b) {
  if (a.size() != b.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < a.size(); ++i) {
    residual = std::max(residual, std::abs(a[i] + b[i]));
  }
  return residual;
}

ftd::Coord translate(ftd::Coord value, ftd::Coord shift) {
  return {value.x + shift.x, value.y + shift.y, value.z + shift.z};
}

ftd::Coord permute(ftd::Coord value) {
  return {value.y, value.z, value.x};
}

ftd::Vec3 permute(const ftd::Vec3& value) {
  return {value.y, value.z, value.x};
}

ftd::Coord invert(ftd::Coord value) {
  return {-value.x, -value.y, -value.z};
}

ftd::Vec3 invert(const ftd::Vec3& value) {
  return {-value.x, -value.y, -value.z};
}

}  // namespace

int main() {
  constexpr int L = 16;
  constexpr double gate = 1e-12;

  const auto axial = ftd::eft::make_face_current_segment(
      L, {2, 3, 4}, {0.25, 0.4, 0.6},
      {2, 3, 4}, {0.75, 0.4, 0.6}, +1);
  check("analytic axial segment valid", axial.valid);
  check("analytic axial segment closes continuity",
        axial.continuity_residual <= gate);
  check("analytic transverse weight 00",
        close(axial.current_x[axial.index(2, 3, 4)], 0.12));
  check("analytic transverse weight 01",
        close(axial.current_x[axial.index(2, 3, 5)], 0.18));
  check("analytic transverse weight 10",
        close(axial.current_x[axial.index(2, 4, 4)], 0.08));
  check("analytic transverse weight 11",
        close(axial.current_x[axial.index(2, 4, 5)], 0.12));
  check("axial current partition is displacement",
        close(sum(axial.current_x), 0.5)
        && close(sum(axial.current_y), 0.0)
        && close(sum(axial.current_z), 0.0));

  const ftd::Coord start_anchor{4, 5, 6};
  const ftd::Vec3 start_remainder{0.2, -0.3, 0.4};
  const ftd::Coord end_anchor{4, 5, 6};
  const ftd::Vec3 end_remainder{0.75, 0.1, -0.2};
  const auto positive = ftd::eft::make_face_current_segment(
      L, start_anchor, start_remainder,
      end_anchor, end_remainder, +1);
  const auto negative = ftd::eft::make_face_current_segment(
      L, start_anchor, start_remainder,
      end_anchor, end_remainder, -1);
  check("piecewise trilinear diagonal segment valid",
        positive.valid && negative.valid);
  check("both polarities close continuity below gate",
        positive.continuity_residual <= gate
        && negative.continuity_residual <= gate);
  check("both endpoint shapes preserve partition and first moment",
        positive.partition_residual <= gate
        && positive.first_moment_residual <= gate
        && negative.partition_residual <= gate
        && negative.first_moment_residual <= gate);
  check("deposition is compact on the crossed local cells",
        positive.locality_residual == 0.0
        && positive.rho_support <= 16
        && positive.current_support <= 36);
  check("negative segment is exact sign reversal",
        max_opposite_residual(positive.rho_before, negative.rho_before) <= gate
        && max_opposite_residual(positive.rho_after, negative.rho_after) <= gate
        && max_opposite_residual(positive.current_x, negative.current_x) <= gate
        && max_opposite_residual(positive.current_y, negative.current_y) <= gate
        && max_opposite_residual(positive.current_z, negative.current_z) <= gate);
  check("integrated face current equals signed endpoint displacement",
        close(sum(positive.current_x),
              positive.end_effective_position.x
                  - positive.start_effective_position.x)
        && close(sum(positive.current_y),
                 positive.end_effective_position.y
                     - positive.start_effective_position.y)
        && close(sum(positive.current_z),
                 positive.end_effective_position.z
                     - positive.start_effective_position.z));

  const auto stationary = ftd::eft::make_face_current_segment(
      L, start_anchor, start_remainder,
      start_anchor, start_remainder, +1);
  check("stationary shape carries exactly zero current",
        stationary.valid && stationary.current_support == 0
        && max_difference(stationary.rho_before,
                          stationary.rho_after) == 0.0);

  const auto boundary = ftd::eft::make_face_current_segment(
      L, {15, 8, 9}, {0.9, 0.25, -0.5},
      {0, 8, 9}, {0.1, 0.25, -0.5}, +1);
  check("periodic threshold crossing selects nearest straight image",
        boundary.valid
        && close(boundary.start_effective_position.x, 15.9)
        && close(boundary.end_effective_position.x, 16.1)
        && close(sum(boundary.current_x), 0.2));
  check("periodic threshold crossing closes continuity",
        boundary.continuity_residual <= gate);

  const ftd::Coord shift{3, 2, -1};
  const auto translated = ftd::eft::make_face_current_segment(
      L, translate(start_anchor, shift), start_remainder,
      translate(end_anchor, shift), end_remainder, +1);
  double translation_residual = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int source = positive.index(x, y, z);
        const int target = translated.index(
            x + shift.x, y + shift.y, z + shift.z);
        translation_residual = std::max({
            translation_residual,
            std::abs(positive.rho_before[source]
                     - translated.rho_before[target]),
            std::abs(positive.rho_after[source]
                     - translated.rho_after[target]),
            std::abs(positive.current_x[source]
                     - translated.current_x[target]),
            std::abs(positive.current_y[source]
                     - translated.current_y[target]),
            std::abs(positive.current_z[source]
                     - translated.current_z[target])});
      }
    }
  }
  check("integer translation covariantly shifts charge and current",
        translated.valid && translation_residual <= gate);

  const auto rotated = ftd::eft::make_face_current_segment(
      L, permute(start_anchor), permute(start_remainder),
      permute(end_anchor), permute(end_remainder), +1);
  double rotation_residual = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int source = positive.index(x, y, z);
        const int target = rotated.index(y, z, x);
        rotation_residual = std::max({
            rotation_residual,
            std::abs(positive.rho_before[source]
                     - rotated.rho_before[target]),
            std::abs(positive.rho_after[source]
                     - rotated.rho_after[target]),
            std::abs(positive.current_x[source]
                     - rotated.current_z[target]),
            std::abs(positive.current_y[source]
                     - rotated.current_x[target]),
            std::abs(positive.current_z[source]
                     - rotated.current_y[target])});
      }
    }
  }
  check("cyclic cubic rotation maps all oriented-face components",
        rotated.valid && rotation_residual <= gate);

  const auto inverted = ftd::eft::make_face_current_segment(
      L, invert(start_anchor), invert(start_remainder),
      invert(end_anchor), invert(end_remainder), +1);
  double inversion_residual = 0.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int source = positive.index(x, y, z);
        inversion_residual = std::max({
            inversion_residual,
            std::abs(positive.rho_before[source]
                     - inverted.rho_before[inverted.index(-x, -y, -z)]),
            std::abs(positive.rho_after[source]
                     - inverted.rho_after[inverted.index(-x, -y, -z)]),
            std::abs(positive.current_x[source]
                     + inverted.current_x[inverted.index(-x - 1, -y, -z)]),
            std::abs(positive.current_y[source]
                     + inverted.current_y[inverted.index(-x, -y - 1, -z)]),
            std::abs(positive.current_z[source]
                     + inverted.current_z[inverted.index(-x, -y, -z - 1)])});
      }
    }
  }
  check("cubic inversion reverses oriented current on reflected faces",
        inverted.valid && inversion_residual <= gate);

  check("invalid polarity fails closed",
        !ftd::eft::make_face_current_segment(
            L, start_anchor, start_remainder,
            end_anchor, end_remainder, 0).valid);

  const double worst_continuity = std::max({
      axial.continuity_residual, positive.continuity_residual,
      negative.continuity_residual, stationary.continuity_residual,
      boundary.continuity_residual, translated.continuity_residual,
      rotated.continuity_residual, inverted.continuity_residual});
  const double worst_partition = std::max({
      axial.partition_residual, positive.partition_residual,
      negative.partition_residual, stationary.partition_residual,
      boundary.partition_residual, translated.partition_residual,
      rotated.partition_residual, inverted.partition_residual});
  const double worst_first_moment = std::max({
      axial.first_moment_residual, positive.first_moment_residual,
      negative.first_moment_residual, stationary.first_moment_residual,
      boundary.first_moment_residual, translated.first_moment_residual,
      rotated.first_moment_residual, inverted.first_moment_residual});
  std::cout.precision(17);
  std::cout << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_partition_residual=" << worst_partition << '\n'
            << "worst_first_moment_residual=" << worst_first_moment << '\n'
            << "translation_residual=" << translation_residual << '\n'
            << "rotation_residual=" << rotation_residual << '\n'
            << "inversion_residual=" << inversion_residual << '\n'
            << "worst_locality_residual="
            << std::max({positive.locality_residual,
                         boundary.locality_residual,
                         translated.locality_residual,
                         rotated.locality_residual,
                         inverted.locality_residual}) << '\n';
  std::cout << "face_current_segment failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

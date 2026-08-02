// FTD-0683: fixed-origin component-aware radial field profile.

#include "ftd/eft/component_aware_radial_field_profile.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr int L = 9;
constexpr double gate = 2e-12;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

template <typename Field>
Field translated(const Field& field, int dx, int dy, int dz) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto old_index = static_cast<std::size_t>(field.index(x, y, z));
        const auto new_index = static_cast<std::size_t>(
            result.index(x + dx, y + dy, z + dz));
        result.x[new_index] = field.x[old_index];
        result.y[new_index] = field.y[old_index];
        result.z[new_index] = field.z[old_index];
      }
    }
  }
  return result;
}

template <typename Field>
Field cyclic_rotated(const Field& field) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto old_index = static_cast<std::size_t>(field.index(x, y, z));
        const auto new_index = static_cast<std::size_t>(
            result.index(y, z, x));
        result.x[new_index] = field.y[old_index];
        result.y[new_index] = field.z[old_index];
        result.z[new_index] = field.x[old_index];
      }
    }
  }
  return result;
}

double max_vector_difference(const std::vector<double>& left,
                             const std::vector<double>& right) {
  if (left.size() != right.size()) return std::numeric_limits<double>::infinity();
  double result = 0.0;
  for (std::size_t index = 0; index < left.size(); ++index)
    result = std::max(result, std::abs(left[index] - right[index]));
  return result;
}

double max_profile_difference(
    const ftd::eft::ComponentAwareRadialFieldProfile& left,
    const ftd::eft::ComponentAwareRadialFieldProfile& right) {
  return std::max({
      std::abs(left.total_norm - right.total_norm),
      std::abs(left.mean_radius - right.mean_radius),
      std::abs(left.rms_radius - right.rms_radius),
      max_vector_difference(left.shell_norm_by_doubled_radius,
                            right.shell_norm_by_doubled_radius),
      max_vector_difference(left.cumulative_norm_by_doubled_radius,
                            right.cumulative_norm_by_doubled_radius)});
}

}  // namespace

int main() {
  ftd::eft::MatchedFaceFlux reference_e(L);
  ftd::eft::MatchedEdgeField reference_b(L);
  auto candidate_e = reference_e;
  auto candidate_b = reference_b;
  const ftd::Vec3 origin{2.0, 3.0, 1.0};
  candidate_e.x[static_cast<std::size_t>(candidate_e.index(2, 3, 1))] = 2.0;
  candidate_e.z[static_cast<std::size_t>(candidate_e.index(5, 3, 1))] = 3.0;
  candidate_b.y[static_cast<std::size_t>(candidate_b.index(2, 7, 1))] = 4.0;
  constexpr double beta = 2.0;
  constexpr double speed = 0.5;
  const auto profile = ftd::eft::observe_component_aware_radial_field_profile(
      reference_e, reference_b, candidate_e, candidate_b,
      origin, beta, speed);

  // E_x contributes 4 at rho2=1, E_z contributes 9 at rho2=6, and
  // B_y contributes 4 at rho2=8 because beta*c^2/2=1/4.
  check("profile valid", profile.valid);
  check("not zero", !profile.zero_profile);
  check("bin count", profile.shell_norm_by_doubled_radius.size() == L + 1);
  check("isolated face half-step bin",
        std::abs(profile.shell_norm_by_doubled_radius[1] - 4.0) <= gate);
  check("far face bin",
        std::abs(profile.shell_norm_by_doubled_radius[6] - 9.0) <= gate);
  check("edge half-step geometry bin",
        std::abs(profile.shell_norm_by_doubled_radius[8] - 4.0) <= gate);
  check("total norm", std::abs(profile.total_norm - 17.0) <= gate);
  check("partition", profile.partition_residual <= gate);
  check("cumulative", profile.cumulative_residual <= gate);
  check("monotone", profile.monotonicity_residual <= gate);
  check("quantile 50", profile.doubled_radius_50 == 6);
  check("quantile 90", profile.doubled_radius_90 == 8);
  check("quantile 99", profile.doubled_radius_99 == 8);

  constexpr int dx = 3;
  constexpr int dy = -2;
  constexpr int dz = 1;
  const auto shifted = ftd::eft::observe_component_aware_radial_field_profile(
      translated(reference_e, dx, dy, dz),
      translated(reference_b, dx, dy, dz),
      translated(candidate_e, dx, dy, dz),
      translated(candidate_b, dx, dy, dz),
      {origin.x + dx, origin.y + dy, origin.z + dz}, beta, speed);
  check("integer translation valid", shifted.valid);
  check("integer translation covariance",
        max_profile_difference(profile, shifted) <= gate);

  const auto rotated = ftd::eft::observe_component_aware_radial_field_profile(
      cyclic_rotated(reference_e), cyclic_rotated(reference_b),
      cyclic_rotated(candidate_e), cyclic_rotated(candidate_b),
      {origin.y, origin.z, origin.x}, beta, speed);
  check("cyclic rotation valid", rotated.valid);
  check("cyclic cubic covariance",
        max_profile_difference(profile, rotated) <= gate);

  const auto zero = ftd::eft::observe_component_aware_radial_field_profile(
      reference_e, reference_b, reference_e, reference_b,
      origin, beta, speed);
  check("zero profile valid", zero.valid && zero.zero_profile);
  check("zero quantiles", zero.doubled_radius_50 == 0
      && zero.doubled_radius_90 == 0 && zero.doubled_radius_99 == 0);

  ftd::eft::MatchedFaceFlux even_e(8);
  ftd::eft::MatchedEdgeField even_b(8);
  check("even volume fails closed",
        !ftd::eft::observe_component_aware_radial_field_profile(
            even_e, even_b, even_e, even_b, {1.0, 2.0, 3.0}, beta, speed).valid);
  check("noninteger origin fails closed",
        !ftd::eft::observe_component_aware_radial_field_profile(
            reference_e, reference_b, candidate_e, candidate_b,
            {2.5, 3.0, 1.0}, beta, speed).valid);
  auto nonfinite = candidate_e;
  nonfinite.y[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite field fails closed",
        !ftd::eft::observe_component_aware_radial_field_profile(
            reference_e, reference_b, nonfinite, candidate_b,
            origin, beta, speed).valid);
  check("nonpositive scale fails closed",
        !ftd::eft::observe_component_aware_radial_field_profile(
            reference_e, reference_b, candidate_e, candidate_b,
            origin, 0.0, speed).valid);

  std::cout.precision(17);
  std::cout << "total=" << profile.total_norm
            << " mean_radius=" << profile.mean_radius
            << " rms_radius=" << profile.rms_radius
            << " partition=" << profile.partition_residual
            << " failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}

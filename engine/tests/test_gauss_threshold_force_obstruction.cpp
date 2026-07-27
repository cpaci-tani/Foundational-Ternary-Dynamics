/** FTD-0487: Gauss-source lower bound on threshold force jumps. */

#include "ftd/constants.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/gauss_threshold_force_obstruction.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

std::vector<double> dipole_source(
    const ftd::eft::MatchedFaceFlux& field,
    int source_index,
    int sink_index,
    double amount) {
  std::vector<double> source(field.x.size(), 0.0);
  source[static_cast<std::size_t>(source_index)] = amount;
  source[static_cast<std::size_t>(sink_index)] = -amount;
  return source;
}

ftd::eft::GaussThresholdForceObstruction analyze_dipole(
    int sx, int sy, int sz,
    int tx, int ty, int tz,
    double amount,
    double coupling) {
  ftd::eft::MatchedFaceFlux field(L);
  const int source_index = field.index(sx, sy, sz);
  const int sink_index = field.index(tx, ty, tz);
  if (!ftd::eft::seed_dipole_path(
          field, source_index, sink_index, amount)) {
    return {};
  }
  return ftd::eft::analyze_gauss_threshold_force_obstruction(
      field, dipole_source(field, source_index, sink_index, amount),
      coupling, ftd::C_SPEED);
}

}  // namespace

int main() {
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double kappa = normalization.native_action_work_coefficient;
  const double coupling = kappa / ftd::C_SPEED;

  const auto positive = analyze_dipole(
      2, 5, 7, 12, 10, 9, +1.0, coupling);
  const auto negative = analyze_dipole(
      2, 5, 7, 12, 10, 9, -1.0, coupling);
  const auto translated = analyze_dipole(
      5, 7, 6, 15, 12, 8, +1.0, coupling);
  const auto rotated = analyze_dipole(
      5, 7, 2, 10, 9, 12, +1.0, coupling);

  check("positive dipole closes jump-divergence and Gauss identities",
        positive.valid
        && positive.divergence_identity_residual <= gate
        && positive.gauss_residual <= gate);
  check("negative dipole closes jump-divergence and Gauss identities",
        negative.valid
        && negative.divergence_identity_residual <= gate
        && negative.gauss_residual <= gate);
  check("translated dipole preserves obstruction",
        translated.valid && translated.gauss_residual <= gate
        && translated.pointwise_bound_violation <= gate);
  check("cyclically rotated dipole preserves obstruction",
        rotated.valid && rotated.gauss_residual <= gate
        && rotated.pointwise_bound_violation <= gate);
  check("pointwise source jump lower bound is exact",
        positive.pointwise_bound_violation <= gate
        && negative.pointwise_bound_violation <= gate
        && positive.minimum_nonzero_source_ratio >= 1.0 - gate
        && negative.minimum_nonzero_source_ratio >= 1.0 - gate);
  check("both source polarities force a nonzero normal jump",
        positive.maximum_component_jump >= 1.0 / 3.0
        && negative.maximum_component_jump >= 1.0 / 3.0);

  ftd::eft::MatchedFaceFlux continuous(L);
  constexpr double pi = 3.141592653589793238462643383279502884;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = continuous.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        continuous.x[static_cast<std::size_t>(i)] =
            0.01 * std::sin(py) + 0.02 * std::cos(pz);
        continuous.y[static_cast<std::size_t>(i)] =
            0.03 * std::sin(pz) - 0.01 * std::cos(px);
        continuous.z[static_cast<std::size_t>(i)] =
            0.02 * std::sin(px) + 0.01 * std::cos(py);
      }
    }
  }
  const std::vector<double> vacuum(continuous.x.size(), 0.0);
  const auto continuous_result =
      ftd::eft::analyze_gauss_threshold_force_obstruction(
          continuous, vacuum, coupling, ftd::C_SPEED);
  check("global axial threshold continuity forces zero divergence",
        continuous_result.valid
        && continuous_result.maximum_component_jump == 0.0
        && continuous_result.gauss_residual == 0.0);

  ftd::eft::MatchedEdgeField edge(L);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = edge.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        edge.z[static_cast<std::size_t>(i)] =
            1e-3 * std::sin(px) * std::sin(py);
      }
    }
  }
  const auto transverse = ftd::eft::matched_curl(edge);
  const auto transverse_result =
      ftd::eft::analyze_gauss_threshold_force_obstruction(
          transverse, vacuum, coupling, ftd::C_SPEED);
  check("divergence-free field can still have threshold jumps",
        transverse_result.valid
        && transverse_result.gauss_residual <= gate
        && transverse_result.maximum_component_jump > 1e-6);

  const double expected_impulse_bound = kappa / 3.0;
  check("minimal-action impulse lower bound has forced coefficient",
        std::abs(positive.normalized_impulse_lower_bound
                 - expected_impulse_bound) <= 1e-15
        && std::abs(negative.normalized_impulse_lower_bound
                    - expected_impulse_bound) <= 1e-15);

  check("invalid source size fails closed",
        !ftd::eft::analyze_gauss_threshold_force_obstruction(
            continuous, {}, coupling, ftd::C_SPEED).valid);

  std::cout.precision(17);
  std::cout << "positive_gauss_residual=" << positive.gauss_residual << '\n'
            << "pointwise_bound_violation="
            << positive.pointwise_bound_violation << '\n'
            << "minimum_nonzero_source_ratio="
            << positive.minimum_nonzero_source_ratio << '\n'
            << "maximum_source_component_jump="
            << positive.maximum_component_jump << '\n'
            << "normalized_impulse_lower_bound="
            << positive.normalized_impulse_lower_bound << '\n'
            << "transverse_source_residual="
            << transverse_result.gauss_residual << '\n'
            << "transverse_component_jump="
            << transverse_result.maximum_component_jump << '\n'
            << "gauss_threshold_force_obstruction failures="
            << failures << '\n'
            << "verdict=NONZERO_GAUSS_SOURCE_FORCES_THRESHOLD_MULTIVALUEDNESS\n";
  return failures == 0 ? 0 : 1;
}

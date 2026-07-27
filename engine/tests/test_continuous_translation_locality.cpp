/** FTD-0554: exact continuous translation versus strict locality. */

#include "ftd/eft/continuous_translation_locality.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/quadratic_coat_composite_peierls.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  if (pass) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const std::vector<double> fractions{
      0.0, 0.125, 0.25, 0.375, 0.5};
  const std::vector<std::pair<double, double>> compositions{
      {0.125, 0.25}, {0.25, 0.25}};
  const std::vector<int> separations{1, 3};
  const std::vector<std::pair<double, double>> moves{
      {0.0, 0.125}, {0.125, 0.25}, {0.25, 0.5}};
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  check("native normalization", normalization.valid);
  const double beta = normalization.mapped_field_work_coefficient;

  int registered_volumes = 0;
  int registered_shift_samples = 0;
  int registered_continuity_samples = 0;
  int minimum_noninteger_support = 1000000;
  int minimum_density_change_support = 1000000;
  int minimum_current_support = 1000000;
  double most_negative_weight = 0.0;
  double worst_cardinal = 0.0;
  double worst_group = 0.0;
  double worst_energy = 0.0;
  double worst_identity = 0.0;
  double smallest_local_coefficient = INFINITY;
  double smallest_local_barrier = INFINITY;

  for (int L : {17, 33}) {
    const auto result = ftd::eft::analyze_continuous_translation_locality(
        L, fractions, compositions, separations, moves, beta);
    ++registered_volumes;
    registered_shift_samples += static_cast<int>(result.shift_samples.size());
    registered_continuity_samples +=
        static_cast<int>(result.continuity_samples.size());
    check("analytic locality theorem registered", result.valid
        && result.finite_laurent_unitary_is_monomial
        && result.continuous_finite_range_shift_group_impossible);
    check("bandlimited identities", result.maximum_identity_residual <= gate
        && result.cardinal_residual <= gate
        && result.maximum_group_residual <= gate
        && result.maximum_energy_residual <= gate);
    check("generic fractional shift is globally supported and signed",
        result.minimum_noninteger_support == L
        && result.most_negative_weight < 0.0);
    check("pinning-free continuity source is nonlocal",
        result.minimum_density_change_support > L/2
        && result.minimum_current_support > L/2);

    minimum_noninteger_support = std::min(
        minimum_noninteger_support, result.minimum_noninteger_support);
    minimum_density_change_support = std::min(
        minimum_density_change_support,
        result.minimum_density_change_support);
    minimum_current_support = std::min(
        minimum_current_support, result.minimum_current_support);
    most_negative_weight = std::min(
        most_negative_weight, result.most_negative_weight);
    worst_cardinal = std::max(worst_cardinal, result.cardinal_residual);
    worst_group = std::max(worst_group, result.maximum_group_residual);
    worst_energy = std::max(worst_energy, result.maximum_energy_residual);
    worst_identity = std::max(
        worst_identity, result.maximum_identity_residual);

    const std::vector<ftd::eft::QuadraticCompositeConstituent> dipole{
        {{0,0,0},+1}, {{1,0,0},-1}};
    const auto local = ftd::eft::evaluate_quadratic_composite_peierls(
        L, dipole, {L/2,L/2,L/2}, 0, beta, {0.0,0.5}, {});
    check("compact quadratic control retains Peierls barrier",
        local.valid && local.maximum_identity_residual <= gate
        && local.peierls_coefficient > 0.0 && local.barrier > gate);
    smallest_local_coefficient = std::min(
        smallest_local_coefficient, local.peierls_coefficient);
    smallest_local_barrier = std::min(
        smallest_local_barrier, local.barrier);
  }

  check("locked campaign cardinalities", registered_volumes == 2
      && registered_shift_samples == 10
      && registered_continuity_samples == 12);
  const bool trilemma = failures == 0;
  std::cout << "registered_volumes=" << registered_volumes << '\n'
            << "registered_shift_samples=" << registered_shift_samples << '\n'
            << "registered_continuity_samples="
            << registered_continuity_samples << '\n'
            << "minimum_noninteger_support="
            << minimum_noninteger_support << '\n'
            << "minimum_density_change_support="
            << minimum_density_change_support << '\n'
            << "minimum_current_support=" << minimum_current_support << '\n'
            << "most_negative_weight=" << most_negative_weight << '\n'
            << "worst_cardinal_residual=" << worst_cardinal << '\n'
            << "worst_group_residual=" << worst_group << '\n'
            << "worst_energy_residual=" << worst_energy << '\n'
            << "worst_identity_residual=" << worst_identity << '\n'
            << "smallest_local_peierls_coefficient="
            << smallest_local_coefficient << '\n'
            << "smallest_local_barrier=" << smallest_local_barrier << '\n'
            << "verdict="
            << (trilemma
                ? "EXACT_TRANSLATION_REQUIRES_NONLOCAL_COUPLING"
                : "CONTINUOUS_TRANSLATION_TRILEMMA_OBSERVER_INVALID") << '\n'
            << "continuous_translation_locality failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}

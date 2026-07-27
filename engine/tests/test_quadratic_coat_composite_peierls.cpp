/**
 * FTD-0553: exact Peierls obstruction for rigid integer-offset neutral
 * composites built from the compact quadratic polarity coat.
 */

#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/quadratic_coat_composite_peierls.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr double gate = 1e-12;
int failures = 0;

using Constituent = ftd::eft::QuadraticCompositeConstituent;
using Result = ftd::eft::QuadraticCompositePeierlsResult;

void check(const std::string& label, bool pass) {
  if (pass) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

ftd::Coord cycle(const ftd::Coord& value) {
  return {value.z, value.x, value.y};
}

std::vector<Constituent> cycle(
    const std::vector<Constituent>& constituents) {
  std::vector<Constituent> result = constituents;
  for (auto& constituent : result)
    constituent.offset = cycle(constituent.offset);
  return result;
}

std::vector<Constituent> mirror(
    const std::vector<Constituent>& constituents) {
  std::vector<Constituent> result = constituents;
  for (auto& constituent : result) constituent.polarity *= -1;
  return result;
}

double result_difference(const Result& lhs, const Result& rhs) {
  double residual = std::max({
      std::abs(lhs.spectral_energy_zero-rhs.spectral_energy_zero),
      std::abs(lhs.peierls_coefficient-rhs.peierls_coefficient),
      std::abs(lhs.barrier-rhs.barrier)});
  if (lhs.samples.size() != rhs.samples.size()
      || lhs.work_samples.size() != rhs.work_samples.size()) return INFINITY;
  for (std::size_t i = 0; i < lhs.samples.size(); ++i)
    residual = std::max({residual,
        std::abs(lhs.samples[i].spectral_energy
                 -rhs.samples[i].spectral_energy),
        std::abs(lhs.samples[i].poisson_energy
                 -rhs.samples[i].poisson_energy)});
  for (std::size_t i = 0; i < lhs.work_samples.size(); ++i)
    residual = std::max({residual,
        std::abs(lhs.work_samples[i].field_energy_change
                 -rhs.work_samples[i].field_energy_change),
        std::abs(lhs.work_samples[i].current_work
                 -rhs.work_samples[i].current_work),
        std::abs(lhs.work_samples[i].net_force_component
                 -rhs.work_samples[i].net_force_component)});
  return residual;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  check("native face normalization", normalization.valid);
  const double beta = normalization.mapped_field_work_coefficient;
  const std::vector<double> fractions{
      0.0, 0.125, 0.25, 0.375, 0.5};
  const std::vector<std::pair<double, double>> intervals{
      {0.0, 0.0625}, {0.125, 0.1875},
      {0.25, 0.3125}, {0.375, 0.4375}};
  const std::vector<std::vector<Constituent>> structures{
      {{{0,0,0},+1}, {{1,0,0},-1}},
      {{{0,0,0},+1}, {{3,0,0},-1}},
      {{{0,0,0},+1}, {{1,1,1},-1}},
      {{{-1,0,0},+1}, {{1,0,0},+1},
       {{0,-1,0},-1}, {{0,1,0},-1}}};

  int registered_arms = 0;
  std::size_t minimum_positive_terms =
      std::numeric_limits<std::size_t>::max();
  int maximum_poisson_iterations = 0;
  double worst_identity = 0.0;
  double worst_polarity = 0.0;
  double worst_translation = 0.0;
  double worst_rotation = 0.0;
  double smallest_coefficient = INFINITY;
  double smallest_barrier = INFINITY;
  double largest_barrier = 0.0;

  for (int L : {17, 33}) {
    const ftd::Coord origin{L/2, L/2, L/2};
    const ftd::Coord translated_origin{L/2+2, L/2-3, L/2+1};
    for (const auto& structure : structures) {
      for (int axis = 0; axis < 3; ++axis) {
        const Result base = ftd::eft::evaluate_quadratic_composite_peierls(
            L, structure, origin, axis, beta, fractions, intervals);
        const Result polarity = ftd::eft::evaluate_quadratic_composite_peierls(
            L, mirror(structure), origin, axis, beta, fractions, intervals);
        const Result translation =
            ftd::eft::evaluate_quadratic_composite_peierls(
                L, structure, translated_origin, axis, beta,
                fractions, intervals);
        const Result rotation = ftd::eft::evaluate_quadratic_composite_peierls(
            L, cycle(structure), cycle(origin), (axis+1)%3, beta,
            fractions, intervals);
        const std::array<const Result*, 4> arms{{
            &base, &polarity, &translation, &rotation}};
        for (const Result* arm : arms) {
          ++registered_arms;
          check("registered arm valid", arm->valid && arm->neutral
              && arm->distinct_primitive_sites && !arm->axis_invariant
              && arm->samples.size() == fractions.size()
              && arm->work_samples.size() == intervals.size());
          check("strict positive Peierls coefficient",
              arm->peierls_coefficient > 0.0
              && arm->barrier > gate
              && arm->positive_spectral_terms > 0);
          check("all theorem/work identities", arm->maximum_identity_residual
              <= gate);
          minimum_positive_terms = std::min(
              minimum_positive_terms, arm->positive_spectral_terms);
          maximum_poisson_iterations = std::max(
              maximum_poisson_iterations, arm->maximum_poisson_iterations);
          worst_identity = std::max(
              worst_identity, arm->maximum_identity_residual);
          smallest_coefficient = std::min(
              smallest_coefficient, arm->peierls_coefficient);
          smallest_barrier = std::min(smallest_barrier, arm->barrier);
          largest_barrier = std::max(largest_barrier, arm->barrier);
        }
        worst_polarity = std::max(
            worst_polarity, result_difference(base, polarity));
        worst_translation = std::max(
            worst_translation, result_difference(base, translation));
        worst_rotation = std::max(
            worst_rotation, result_difference(base, rotation));
      }
    }
  }

  check("locked registered arm count", registered_arms == 96);
  check("polarity mirror covariance", worst_polarity <= gate);
  check("integer translation covariance", worst_translation <= gate);
  check("cyclic cubic covariance", worst_rotation <= gate);
  const bool obstruction = failures == 0 && smallest_barrier > gate;
  std::cout << "registered_arms=" << registered_arms << '\n'
            << "beta=" << beta << '\n'
            << "minimum_positive_spectral_terms="
            << minimum_positive_terms << '\n'
            << "maximum_poisson_iterations="
            << maximum_poisson_iterations << '\n'
            << "worst_identity_residual=" << worst_identity << '\n'
            << "worst_polarity_residual=" << worst_polarity << '\n'
            << "worst_translation_residual=" << worst_translation << '\n'
            << "worst_rotation_residual=" << worst_rotation << '\n'
            << "smallest_peierls_coefficient=" << smallest_coefficient << '\n'
            << "smallest_barrier=" << smallest_barrier << '\n'
            << "largest_barrier=" << largest_barrier << '\n'
            << "verdict="
            << (obstruction
                ? "RIGID_NEUTRAL_COMPOSITE_PEIERLS_OBSTRUCTION"
                : "NEUTRAL_COMPOSITE_OBSERVER_INVALID") << '\n'
            << "quadratic_coat_composite_peierls failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}

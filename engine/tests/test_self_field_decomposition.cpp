/** FTD-0488: locality and provenance limits of self-field subtraction. */

#include "ftd/eft/self_field_decomposition.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-10;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

std::vector<int> dipole_source(
    const ftd::eft::MatchedFaceFlux& field,
    int source_index, int sink_index, int amount) {
  std::vector<int> source(field.x.size(), 0);
  source[static_cast<std::size_t>(source_index)] = amount;
  source[static_cast<std::size_t>(sink_index)] = -amount;
  return source;
}

void add(ftd::eft::MatchedFaceFlux& target,
         const ftd::eft::MatchedFaceFlux& value,
         double scale = 1.0) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

double max_difference(const ftd::eft::MatchedFaceFlux& lhs,
                      const ftd::eft::MatchedFaceFlux& rhs) {
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    residual = std::max({residual,
        std::abs(lhs.x[i] - rhs.x[i]),
        std::abs(lhs.y[i] - rhs.y[i]),
        std::abs(lhs.z[i] - rhs.z[i])});
  }
  return residual;
}

}  // namespace

int main() {
  ftd::eft::MatchedFaceFlux indexing(L);
  const int source_index = indexing.index(2, 5, 7);
  const int sink_index = indexing.index(12, 10, 9);
  const auto source = dipole_source(indexing, source_index, sink_index, +1);

  ftd::eft::MatchedGaussDynamics longitudinal_solver(L);
  const auto minimum = longitudinal_solver.initialize_minimum_energy(source);
  ftd::eft::MatchedFaceFlux total = longitudinal_solver.electric();
  const auto edge = ftd::eft::make_transverse_challenge(L, 1e-3);
  const auto transverse = ftd::eft::matched_curl(edge);
  add(total, transverse);

  const auto decomposition = ftd::eft::decompose_matched_self_field(
      total, source);
  check("neutral minimum-norm longitudinal field converges",
        minimum.valid && minimum.converged
        && minimum.gauss_residual <= gate
        && minimum.curl_adjoint_residual <= gate);
  check("global Hodge decomposition is valid",
        decomposition.valid
        && decomposition.input_gauss_residual <= gate
        && decomposition.transverse_divergence_residual <= gate
        && decomposition.longitudinal_curl_residual <= gate);
  check("longitudinal and transverse sectors are orthogonal",
        decomposition.orthogonality_residual <= gate
        && decomposition.energy_split_residual <= gate);
  check("known transverse curl is recovered",
        max_difference(decomposition.transverse, transverse) <= gate);

  ftd::eft::MatchedFaceFlux string_field(L);
  check("comparison string seeds",
        ftd::eft::seed_dipole_path(
            string_field, source_index, sink_index, +1.0));
  check("minimum-norm field beats routed string energy",
        quadratic_energy(decomposition.longitudinal)
        < quadratic_energy(string_field));

  std::vector<int> lone_source(indexing.x.size(), 0);
  lone_source[static_cast<std::size_t>(source_index)] = +1;
  ftd::eft::MatchedGaussDynamics lone_solver(L);
  const auto lone = lone_solver.initialize_minimum_energy(lone_source);
  check("periodic lone-charge self-field is unsolvable",
        !lone.valid && !lone.neutral);
  check("periodic divergence telescopes to zero",
        std::abs(static_cast<double>(
            ftd::eft::periodic_divergence_sum(total))) <= 1e-12);

  ftd::eft::MatchedFaceFlux attributed_one(L);
  ftd::eft::MatchedFaceFlux attributed_two(L);
  const int source_two = attributed_two.index(4, 3, 11);
  const int sink_two = attributed_two.index(14, 8, 2);
  check("two attributed neutral strings seed",
        ftd::eft::seed_dipole_path(
            attributed_one, source_index, sink_index, +1.0)
        && ftd::eft::seed_dipole_path(
            attributed_two, source_two, sink_two, +1.0));
  ftd::eft::MatchedFaceFlux attributed_total = attributed_one;
  add(attributed_total, attributed_two);
  ftd::eft::MatchedFaceFlux alternative_one = attributed_one;
  ftd::eft::MatchedFaceFlux alternative_two = attributed_two;
  add(alternative_one, transverse);
  add(alternative_two, transverse, -1.0);
  ftd::eft::MatchedFaceFlux alternative_total = alternative_one;
  add(alternative_total, alternative_two);
  const auto source_one = dipole_source(
      attributed_one, source_index, sink_index, +1);
  const auto source_two_values = dipole_source(
      attributed_two, source_two, sink_two, +1);
  const double attribution_change = std::max(
      max_difference(attributed_one, alternative_one),
      max_difference(attributed_two, alternative_two));
  const double attribution_gauss_residual = std::max(
      ftd::eft::max_gauss_residual(alternative_one, source_one),
      ftd::eft::max_gauss_residual(alternative_two, source_two_values));
  check("source attribution is nonunique up to divergence-free transfer",
        max_difference(attributed_total, alternative_total) <= gate
        && attribution_gauss_residual <= gate
        && attribution_change > 1e-6);

  const int translated_source = indexing.index(5, 7, 6);
  const int translated_sink = indexing.index(15, 12, 8);
  const auto mirrored_source = dipole_source(
      indexing, translated_source, translated_sink, -1);
  ftd::eft::MatchedGaussDynamics mirrored_solver(L);
  const auto mirrored = mirrored_solver.initialize_minimum_energy(
      mirrored_source);
  check("translated polarity mirror preserves global solution",
        mirrored.valid && mirrored.gauss_residual <= gate
        && mirrored.curl_adjoint_residual <= gate);

  std::cout.precision(17);
  std::cout << "minimum_gauss_residual=" << minimum.gauss_residual << '\n'
            << "minimum_curl_residual="
            << minimum.curl_adjoint_residual << '\n'
            << "transverse_divergence_residual="
            << decomposition.transverse_divergence_residual << '\n'
            << "orthogonality_residual="
            << decomposition.orthogonality_residual << '\n'
            << "energy_split_residual="
            << decomposition.energy_split_residual << '\n'
            << "longitudinal_support="
            << decomposition.longitudinal_support << '\n'
            << "attribution_change=" << attribution_change << '\n'
            << "attribution_gauss_residual="
            << attribution_gauss_residual << '\n'
            << "self_field_decomposition failures=" << failures << '\n'
            << "verdict=LOCAL_PER_PARTICLE_SELF_FIELD_SUBTRACTION_UNAVAILABLE\n";
  return failures == 0 ? 0 : 1;
}

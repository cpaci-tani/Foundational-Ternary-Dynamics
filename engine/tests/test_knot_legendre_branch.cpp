/** FTD-0491: branch multiplicity of the Legendre equation at a knot. */

#include "ftd/eft/knot_legendre_branch.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr ftd::Coord knot{8, 8, 8};
constexpr double c_speed = 0.57735026918962576451;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_gauss_residual = 0.0;
double worst_initial_residual = 0.0;
double worst_momentum_residual = 0.0;
double worst_gauge_residual = 0.0;
double worst_orbit_residual = 0.0;
double epsilon_endpoint_residual = 0.0;
double polarity_mirror_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

void accumulate(const ftd::eft::KnotLegendreBranchResult& result) {
  worst_gauss_residual = std::max({worst_gauss_residual,
      result.gauss_residual, result.bias_divergence_residual});
  worst_initial_residual = std::max(
      worst_initial_residual, result.worst_initial_kinetic_residual);
  worst_momentum_residual = std::max(
      worst_momentum_residual, result.worst_analytic_momentum_residual);
  worst_gauge_residual = std::max(
      worst_gauge_residual, result.worst_gauge_kinetic_residual);
  worst_orbit_residual = std::max(
      worst_orbit_residual, result.displacement_orbit_residual);
}

}  // namespace

int main() {
  const double epsilons[3] = {1e-4, 1e-6, 1e-8};
  ftd::eft::KnotLegendreBranchResult reference_positive;
  for (int polarity : {-1, +1}) {
    ftd::eft::KnotLegendreBranchResult previous;
    for (double epsilon : epsilons) {
      const auto result = ftd::eft::analyze_knot_legendre_branches(
          L, knot, polarity, epsilon, rest_energy,
          c_speed, c_speed, coupling);
      accumulate(result);
      check((polarity > 0 ? "+ " : "- ")
                + std::string("eight branches at epsilon=")
                + std::to_string(epsilon),
            result.valid && result.sign_consistent_count == 8
            && result.solved_branch_count == 8
            && result.gauss_residual <= gate
            && result.worst_initial_kinetic_residual <= gate
            && result.worst_analytic_momentum_residual <= gate
            && result.worst_gauge_kinetic_residual <= gate
            && result.displacement_orbit_residual <= gate);
      if (previous.valid) {
        for (std::size_t i = 0; i < result.branches.size(); ++i) {
          epsilon_endpoint_residual = std::max(
              epsilon_endpoint_residual,
              std::abs(result.branches[i].displacement.x
                       - previous.branches[i].displacement.x));
          epsilon_endpoint_residual = std::max(
              epsilon_endpoint_residual,
              std::abs(result.branches[i].displacement.y
                       - previous.branches[i].displacement.y));
          epsilon_endpoint_residual = std::max(
              epsilon_endpoint_residual,
              std::abs(result.branches[i].displacement.z
                       - previous.branches[i].displacement.z));
        }
      }
      previous = result;
      if (polarity > 0 && epsilon == epsilons[2]) {
        reference_positive = result;
      }
      if (polarity < 0 && epsilon == epsilons[2]) {
        for (std::size_t i = 0; i < result.branches.size(); ++i) {
          polarity_mirror_residual = std::max({
              polarity_mirror_residual,
              std::abs(result.branches[i].displacement.x
                       - reference_positive.branches[i].displacement.x),
              std::abs(result.branches[i].displacement.y
                       - reference_positive.branches[i].displacement.y),
              std::abs(result.branches[i].displacement.z
                       - reference_positive.branches[i].displacement.z)});
        }
      }
    }
  }
  // The loop encounters negative polarity first; compare explicitly.
  const auto mirror_negative = ftd::eft::analyze_knot_legendre_branches(
      L, knot, -1, 1e-8, rest_energy,
      c_speed, c_speed, coupling);
  polarity_mirror_residual = 0.0;
  for (std::size_t i = 0; i < reference_positive.branches.size(); ++i) {
    polarity_mirror_residual = std::max({polarity_mirror_residual,
        std::abs(reference_positive.branches[i].displacement.x
                 - mirror_negative.branches[i].displacement.x),
        std::abs(reference_positive.branches[i].displacement.y
                 - mirror_negative.branches[i].displacement.y),
        std::abs(reference_positive.branches[i].displacement.z
                 - mirror_negative.branches[i].displacement.z)});
  }
  check("epsilon limit preserves analytic endpoints",
        epsilon_endpoint_residual <= gate);
  check("polarity-mirrored self-field preserves branch orbit",
        polarity_mirror_residual <= gate);

  const ftd::Vec3 bias{0.4, 0.5, 0.6};
  for (int polarity : {-1, +1}) {
    const auto biased = ftd::eft::analyze_knot_legendre_branches(
        L, knot, polarity, 1e-8, rest_energy,
        c_speed, c_speed, coupling, bias);
    accumulate(biased);
    check(polarity > 0
              ? "generic bias selects one + incident cell"
              : "generic bias selects one - incident cell",
          biased.valid && biased.sign_consistent_count == 1
          && biased.solved_branch_count == 1
          && biased.gauss_residual <= gate
          && biased.bias_divergence_residual <= gate
          && biased.worst_initial_kinetic_residual <= gate
          && biased.worst_gauge_kinetic_residual <= gate);
  }

  check("invalid knot input fails closed",
        !ftd::eft::analyze_knot_legendre_branches(
            L, {0, 0, 0}, +1, 1e-8, rest_energy,
            c_speed, c_speed, coupling).valid);

  std::cout.precision(17);
  std::cout << "worst_gauss_residual=" << worst_gauss_residual << '\n'
            << "worst_initial_kinetic_residual="
            << worst_initial_residual << '\n'
            << "worst_analytic_momentum_residual="
            << worst_momentum_residual << '\n'
            << "worst_gauge_kinetic_residual="
            << worst_gauge_residual << '\n'
            << "worst_cubic_orbit_residual="
            << worst_orbit_residual << '\n'
            << "epsilon_endpoint_residual="
            << epsilon_endpoint_residual << '\n'
            << "polarity_mirror_residual="
            << polarity_mirror_residual << '\n'
            << "symmetric_solved_branch_count=8\n"
            << "biased_solved_branch_count=1\n"
            << "knot_legendre_branch failures=" << failures << '\n'
            << "verdict=SYMMETRIC_KNOT_HAS_EIGHT_LEGENDRE_BRANCHES\n";
  return failures == 0 ? 0 : 1;
}

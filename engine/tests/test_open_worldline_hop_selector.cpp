/** FTD-0489: an open charged-worldline action is not an endpoint cost. */

#include "ftd/eft/open_worldline_hop_selector.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double time_scale = 0.57735026918962576451;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_identity_residual = 0.0;
double worst_field_residual = 0.0;
double worst_translation_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

std::vector<double> endpoint_gauge(
    const ftd::eft::DualGaugePotentialSlab& slab,
    ftd::Coord positive_site,
    ftd::Coord negative_site,
    double amplitude) {
  std::vector<double> chi(
      static_cast<std::size_t>(L * L * L), 0.0);
  chi[static_cast<std::size_t>(slab.index(
      positive_site.x, positive_site.y, positive_site.z))] = amplitude;
  chi[static_cast<std::size_t>(slab.index(
      negative_site.x, negative_site.y, negative_site.z))] = -amplitude;
  return chi;
}

ftd::eft::OpenWorldlineHopComparison run_comparison(
    ftd::Coord start,
    ftd::Coord end_x,
    ftd::Coord end_y,
    int charge,
    double cost_x,
    double cost_y,
    double gauge_amplitude) {
  ftd::eft::DualGaugePotentialSlab slab(L, time_scale);
  std::vector<double> chi_start(
      static_cast<std::size_t>(L * L * L), 0.0);
  const auto chi_end = endpoint_gauge(
      slab, end_x, end_y, gauge_amplitude);
  return ftd::eft::compare_open_worldline_hops(
      L, start, {}, end_x, {}, cost_x, end_y, {}, cost_y,
      charge, slab, chi_start, chi_end, coupling);
}

void accumulate(const ftd::eft::OpenWorldlineHopComparison& result) {
  worst_identity_residual = std::max(
      worst_identity_residual, result.difference_shift_residual);
  worst_field_residual = std::max({worst_field_residual,
      result.first.electric_invariance_residual,
      result.first.magnetic_invariance_residual,
      result.second.electric_invariance_residual,
      result.second.magnetic_invariance_residual});
}

}  // namespace

int main() {
  const ftd::Coord start{8, 8, 8};
  const ftd::Coord end_x{9, 8, 8};
  const ftd::Coord end_y{8, 9, 8};

  const auto gauge_plus = run_comparison(
      start, end_x, end_y, +1, 0.25, 0.25, +2.0);
  const auto gauge_minus = run_comparison(
      start, end_x, end_y, +1, 0.25, 0.25, -2.0);
  accumulate(gauge_plus);
  accumulate(gauge_minus);
  check("equal-cost open actions obey endpoint identity",
        gauge_plus.valid && gauge_minus.valid
        && gauge_plus.difference_shift_residual <= gate
        && gauge_minus.difference_shift_residual <= gate);
  check("gauge transformation leaves E and B unchanged",
        worst_field_residual <= gate);
  check("opposite endpoint gauges reverse candidate ordering",
        gauge_plus.transformed_action_difference
            * gauge_minus.transformed_action_difference < 0.0);

  const auto unequal_plus = run_comparison(
      start, end_x, end_y, +1, 0.1, 0.9, +4.0);
  const auto unequal_minus = run_comparison(
      start, end_x, end_y, +1, 0.1, 0.9, -4.0);
  accumulate(unequal_plus);
  accumulate(unequal_minus);
  check("finite gauge-invariant matter costs do not fix ordering",
        unequal_plus.valid && unequal_minus.valid
        && unequal_plus.transformed_action_difference
            * unequal_minus.transformed_action_difference < 0.0);

  const auto negative_plus = run_comparison(
      start, end_x, end_y, -1, 0.25, 0.25, +2.0);
  accumulate(negative_plus);
  check("polarity reverses endpoint action shift",
        negative_plus.valid
        && std::abs(negative_plus.transformed_action_difference
                    + gauge_plus.transformed_action_difference) <= gate);

  const ftd::Coord shift{3, -2, 4};
  const ftd::Coord shifted_start{
      start.x + shift.x, start.y + shift.y, start.z + shift.z};
  const ftd::Coord shifted_x{
      end_x.x + shift.x, end_x.y + shift.y, end_x.z + shift.z};
  const ftd::Coord shifted_y{
      end_y.x + shift.x, end_y.y + shift.y, end_y.z + shift.z};
  const auto translated = run_comparison(
      shifted_start, shifted_x, shifted_y, +1, 0.25, 0.25, +2.0);
  accumulate(translated);
  worst_translation_residual = std::abs(
      translated.transformed_action_difference
      - gauge_plus.transformed_action_difference);
  check("integer translation preserves comparison",
        translated.valid && worst_translation_residual <= gate);

  const auto orbits = ftd::eft::summarize_cubic_moore_hops();
  check("Moore displacements form 6/12/8 cubic shells",
        orbits.face_count == 6 && orbits.edge_count == 12
        && orbits.corner_count == 8);
  check("no nonzero Moore displacement is reflection-fixed",
        orbits.nonzero_reflection_fixed_count == 0
        && orbits.orbit_sum.x == 0.0
        && orbits.orbit_sum.y == 0.0
        && orbits.orbit_sum.z == 0.0);

  const ftd::Vec3 remainder{0.72, -0.83, 0.35};
  const ftd::Vec3 velocity{0.42, -0.31, 0.9};
  const auto displacement = ftd::eft::threshold_moore_displacement(
      remainder, velocity, 1.0);
  // Proper cubic rotation (x,y,z)->(y,z,x).
  const auto rotated = ftd::eft::threshold_moore_displacement(
      {remainder.y, remainder.z, remainder.x},
      {velocity.y, velocity.z, velocity.x}, 1.0);
  check("prior kinematics supplies a cubic-covariant endpoint selector",
        rotated.x == displacement.y
        && rotated.y == displacement.z
        && rotated.z == displacement.x);

  check("invalid charge fails closed",
        !run_comparison(start, end_x, end_y, 0,
                        0.0, 0.0, 1.0).valid);

  std::cout.precision(17);
  std::cout << "equal_gauge_plus_difference="
            << gauge_plus.transformed_action_difference << '\n'
            << "equal_gauge_minus_difference="
            << gauge_minus.transformed_action_difference << '\n'
            << "unequal_gauge_plus_difference="
            << unequal_plus.transformed_action_difference << '\n'
            << "unequal_gauge_minus_difference="
            << unequal_minus.transformed_action_difference << '\n'
            << "worst_endpoint_identity_residual="
            << worst_identity_residual << '\n'
            << "worst_field_invariance_residual="
            << worst_field_residual << '\n'
            << "translation_residual="
            << worst_translation_residual << '\n'
            << "open_worldline_hop_selector failures=" << failures << '\n'
            << "verdict=OPEN_WORLDLINE_ACTION_NOT_A_HOP_SELECTOR\n";
  return failures == 0 ? 0 : 1;
}

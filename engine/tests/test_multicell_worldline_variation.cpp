/** FTD-0533: complete deposited-action variation through internal knots. */

#include "ftd/eft/diagonal_endpoint_action_domain.h"
#include "ftd/eft/multicell_worldline_variation.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double lambda_t = ftd::C_SPEED;
constexpr double largest_step = 0.0009765625;
constexpr double gate = 1e-8;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_component(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return max_component(lhs-rhs);
}

struct SlabPair {
  ftd::eft::DualGaugePotentialSlab previous{L, lambda_t};
  ftd::eft::DualGaugePotentialSlab next{L, lambda_t};
};

SlabPair general_pair() {
  SlabPair pair;
  constexpr double pi = 3.141592653589793238462643383279502884;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = pair.previous.index(x, y, z);
        const double px = 2.0*pi*x/L;
        const double py = 2.0*pi*y/L;
        const double pz = 2.0*pi*z/L;
        const double a0x = 0.13*std::sin(py)+0.04*std::cos(pz);
        const double a0y = -0.09*std::sin(pz)+0.06*std::cos(px);
        const double a0z = 0.11*std::sin(px)-0.05*std::cos(py);
        const double a1x = a0x+0.03*std::cos(px+py);
        const double a1y = a0y-0.02*std::sin(py+pz);
        const double a1z = a0z+0.04*std::cos(pz+px);
        pair.previous.A_start.x[static_cast<std::size_t>(i)] = a0x;
        pair.previous.A_start.y[static_cast<std::size_t>(i)] = a0y;
        pair.previous.A_start.z[static_cast<std::size_t>(i)] = a0z;
        pair.previous.A_end.x[static_cast<std::size_t>(i)] = a1x;
        pair.previous.A_end.y[static_cast<std::size_t>(i)] = a1y;
        pair.previous.A_end.z[static_cast<std::size_t>(i)] = a1z;
        pair.next.A_start.x[static_cast<std::size_t>(i)] = a1x;
        pair.next.A_start.y[static_cast<std::size_t>(i)] = a1y;
        pair.next.A_start.z[static_cast<std::size_t>(i)] = a1z;
        pair.next.A_end.x[static_cast<std::size_t>(i)] =
            a1x-0.01*std::sin(px+pz);
        pair.next.A_end.y[static_cast<std::size_t>(i)] =
            a1y+0.03*std::cos(px+py);
        pair.next.A_end.z[static_cast<std::size_t>(i)] =
            a1z-0.02*std::sin(py+pz);
        pair.previous.Phi[static_cast<std::size_t>(i)] =
            0.17*std::sin(px)+0.07*std::cos(py);
        pair.next.Phi[static_cast<std::size_t>(i)] =
            -0.12*std::cos(py)+0.05*std::sin(pz);
      }
    }
  }
  return pair;
}

void make_gauge(std::vector<double>& chi0,
                std::vector<double>& chi1,
                std::vector<double>& chi2) {
  chi0.assign(static_cast<std::size_t>(L*L*L), 0.0);
  chi1 = chi0;
  chi2 = chi0;
  constexpr double pi = 3.141592653589793238462643383279502884;
  ftd::eft::DualGaugePotentialSlab indexing(L, lambda_t);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = indexing.index(x, y, z);
        const double px = 2.0*pi*x/L;
        const double py = 2.0*pi*y/L;
        const double pz = 2.0*pi*z/L;
        chi0[static_cast<std::size_t>(i)] =
            0.031*std::sin(px+py)+0.011*std::cos(pz);
        chi1[static_cast<std::size_t>(i)] =
            -0.019*std::cos(py+pz)+0.023*std::sin(px);
        chi2[static_cast<std::size_t>(i)] =
            0.017*std::sin(pz+px)-0.029*std::cos(py);
      }
    }
  }
}

SlabPair threshold_pair(double electric_left, double electric_right) {
  SlabPair pair;
  for (int x = 0; x < L; ++x) {
    const double electric = x == 4 ? electric_left
        : (x == 5 ? electric_right : 0.0);
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = pair.previous.index(x, y, z);
        pair.previous.A_end.x[static_cast<std::size_t>(i)] =
            -lambda_t*electric;
        pair.next.A_start.x[static_cast<std::size_t>(i)] =
            -lambda_t*electric;
        pair.next.A_end.x[static_cast<std::size_t>(i)] =
            -2.0*lambda_t*electric;
      }
    }
  }
  return pair;
}

bool one_sided_refinement_ok(
    const ftd::eft::MulticellWorldlineVariationResult& result) {
  for (int fine = 2; fine <= 3; ++fine) {
    const double fine_gap = result.maximum_one_sided_gap[
        static_cast<std::size_t>(fine)];
    if (fine_gap <= 1e-10) continue;
    const double coarse_gap = result.maximum_one_sided_gap[
        static_cast<std::size_t>(fine-1)];
    if (!(coarse_gap/fine_gap >= 3.0)) return false;
  }
  return true;
}

ftd::Vec3 carrier_position(const ftd::eft::ContactCarrierRecord& carrier) {
  return {carrier.anchor.x+carrier.remainder.x,
          carrier.anchor.y+carrier.remainder.y,
          carrier.anchor.z+carrier.remainder.z};
}

}  // namespace

int main() {
  constexpr double coupling = 0.73;
  const auto pair = general_pair();

  const ftd::Vec3 interior_previous{5.2, 6.3, 7.4};
  const ftd::Vec3 interior_shared{5.4, 6.45, 7.35};
  const ftd::Vec3 interior_next{5.65, 6.55, 7.2};
  const auto compact = ftd::eft::evaluate_two_slab_variational_force(
      interior_previous, interior_shared, interior_next, +1,
      pair.previous, pair.next, coupling);
  const auto interior = ftd::eft::evaluate_multicell_worldline_variation(
      interior_previous, interior_shared, interior_next, +1,
      pair.previous, pair.next, coupling, largest_step);
  const double interior_gradient_residual = max_difference(
      compact.interaction_impulse, interior.interaction_impulse);
  check("global variation reproduces analytic compact interior gradient",
        compact.valid && interior.valid
        && interior_gradient_residual <= gate);

  struct KnotCase {
    ftd::Vec3 previous{};
    ftd::Vec3 shared{};
    ftd::Vec3 next{};
    int multiplicity = 0;
  };
  const std::array<KnotCase, 3> knots{{
      {{4.75, 6.31, 7.42}, {4.875, 6.375, 7.375},
       {5.125, 6.4375, 7.3125}, 1},
      {{4.75, 5.75, 7.42}, {4.875, 5.875, 7.375},
       {5.125, 6.125, 7.3125}, 2},
      {{4.75, 5.75, 6.75}, {4.875, 5.875, 6.875},
       {5.125, 6.125, 7.125}, 3}}};
  std::array<ftd::eft::MulticellWorldlineVariationResult, 3> knot_results{};
  bool knot_convergence = true;
  bool knot_one_sided = true;
  bool knot_directional = true;
  double worst_knot_convergence = 0.0;
  double worst_knot_directional = 0.0;
  double minimum_final_gap_ratio = INFINITY;
  for (std::size_t i = 0; i < knots.size(); ++i) {
    const auto& value = knots[i];
    auto& result = knot_results[i];
    result = ftd::eft::evaluate_multicell_worldline_variation(
        value.previous, value.shared, value.next, +1,
        pair.previous, pair.next, coupling, largest_step);
    knot_convergence = knot_convergence && result.valid
        && result.next_internal_breaks == 1
        && result.maximum_simultaneous_crossing_multiplicity
            == value.multiplicity
        && result.final_centered_convergence_residual <= gate;
    knot_one_sided = knot_one_sided && one_sided_refinement_ok(result);
    knot_directional = knot_directional
        && result.maximum_directional_linearity_residual <= gate;
    worst_knot_convergence = std::max(worst_knot_convergence,
        result.final_centered_convergence_residual);
    worst_knot_directional = std::max(worst_knot_directional,
        result.maximum_directional_linearity_residual);
    const double fine_gap = result.maximum_one_sided_gap[3];
    const double coarse_gap = result.maximum_one_sided_gap[2];
    if (fine_gap > 1e-10) minimum_final_gap_ratio = std::min(
        minimum_final_gap_ratio, coarse_gap/fine_gap);
  }
  check("face edge and corner internal-knot gradients converge",
        knot_convergence);
  check("one-sided gaps refine toward one internal-knot derivative",
        knot_one_sided);
  check("all signed Moore directional derivatives are gradient-linear",
        knot_directional);

  std::vector<double> chi0;
  std::vector<double> chi1;
  std::vector<double> chi2;
  make_gauge(chi0, chi1, chi2);
  const auto transformed_previous = ftd::eft::gauge_transform_slab(
      pair.previous, chi0, chi1);
  const auto transformed_next = ftd::eft::gauge_transform_slab(
      pair.next, chi1, chi2);
  const auto transformed_corner =
      ftd::eft::evaluate_multicell_worldline_variation(
          knots[2].previous, knots[2].shared, knots[2].next, +1,
          transformed_previous, transformed_next, coupling, largest_step);
  const double gauge_gradient_residual = max_difference(
      knot_results[2].interaction_impulse,
      transformed_corner.interaction_impulse);
  SlabPair zero;
  const auto pure_previous = ftd::eft::gauge_transform_slab(
      zero.previous, chi0, chi1);
  const auto pure_next = ftd::eft::gauge_transform_slab(
      zero.next, chi1, chi2);
  const auto pure_corner = ftd::eft::evaluate_multicell_worldline_variation(
      knots[2].previous, knots[2].shared, knots[2].next, +1,
      pure_previous, pure_next, coupling, largest_step);
  const double pure_gauge_gradient = max_component(
      pure_corner.interaction_impulse);
  check("joined gauge transformations preserve the internal-knot gradient",
        transformed_corner.valid && gauge_gradient_residual <= gate);
  check("pure gauge cancels from the shared internal-knot variation",
        pure_corner.valid && pure_gauge_gradient <= gate);

  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  int zero_geometry_arms = 0;
  double worst_zero_gradient = 0.0;
  double worst_zero_convergence = 0.0;
  bool zero_geometry_ok = true;
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = dx*dx+dy*dy+dz*dz;
        if (shell != 2 && shell != 3) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (double speed : speeds) {
          for (int polarity : {-1, +1}) {
            for (const auto& translation : translations) {
              const ftd::Coord source{
                  8+translation.x, 8+translation.y, 8+translation.z};
              const ftd::Vec3 contact{
                  source.x+0.5*dx, source.y+0.5*dy,
                  source.z+0.5*dz};
              const auto coupled =
                  ftd::eft::solve_symmetric_diagonal_coupled_endpoint(
                      L, contact, direction, polarity, speed, 1e-12);
              if (!coupled.valid) {
                zero_geometry_ok = false;
                continue;
              }
              const auto& carrier = coupled.rebase.bounce_preimage.carrier[0];
              const ftd::Vec3 shared = carrier_position(carrier);
              const ftd::Vec3 unit = carrier.velocity*(1.0/speed);
              const ftd::Vec3 previous = shared
                  -unit*coupled.reference_displacement_magnitude;
              const ftd::Vec3 next = shared
                  +unit*coupled.displacement_magnitude;
              const auto global =
                  ftd::eft::evaluate_multicell_worldline_variation(
                      previous, shared, next, polarity,
                      zero.previous, zero.next, 1.0, largest_step);
              zero_geometry_ok = zero_geometry_ok && global.valid
                  && global.next_internal_breaks == 1
                  && global.maximum_simultaneous_crossing_multiplicity
                      == shell;
              worst_zero_gradient = std::max(
                  worst_zero_gradient, max_component(global.interaction_impulse));
              worst_zero_convergence = std::max(
                  worst_zero_convergence,
                  global.final_centered_convergence_residual);
              ++zero_geometry_arms;
            }
          }
        }
      }
    }
  }
  check("all FTD-0532 diagonal geometries enter the global action domain",
        zero_geometry_ok && zero_geometry_arms == 240
        && worst_zero_gradient <= 1e-10
        && worst_zero_convergence <= 1e-10);

  constexpr double epsilon = 1e-8;
  const auto threshold = threshold_pair(0.02, -0.03);
  const ftd::Vec3 left{5.0-epsilon, 6.4, 7.5};
  const ftd::Vec3 right{5.0+epsilon, 6.4, 7.5};
  const auto left_result = ftd::eft::evaluate_two_slab_variational_force(
      left, left, left, +1,
      threshold.previous, threshold.next, 1.0);
  const auto right_result = ftd::eft::evaluate_two_slab_variational_force(
      right, right, right, +1,
      threshold.previous, threshold.next, 1.0);
  const double endpoint_threshold_gap = max_difference(
      left_result.interaction_impulse, right_result.interaction_impulse);
  check("internal-knot extension does not erase endpoint-threshold no-go",
        left_result.valid && right_result.valid
        && endpoint_threshold_gap > 1e-4);

  auto unjoined = pair.next;
  unjoined.A_start.x[0] += 0.1;
  check("invalid charge join causality and step inputs fail closed",
        !ftd::eft::evaluate_multicell_worldline_variation(
            interior_previous, interior_shared, interior_next, 0,
            pair.previous, pair.next, coupling, largest_step).valid
        && !ftd::eft::evaluate_multicell_worldline_variation(
            interior_previous, interior_shared, interior_next, +1,
            pair.previous, unjoined, coupling, largest_step).valid
        && !ftd::eft::evaluate_multicell_worldline_variation(
            {4.0, 4.0, 4.0}, {5.0, 5.0, 5.0}, {5.1, 5.1, 5.1}, +1,
            zero.previous, zero.next, 1.0, largest_step).valid
        && !ftd::eft::evaluate_multicell_worldline_variation(
            interior_previous, interior_shared, interior_next, +1,
            pair.previous, pair.next, coupling,
            std::numeric_limits<double>::quiet_NaN()).valid);

  std::cout.precision(17);
  std::cout << "interior_gradient_residual="
            << interior_gradient_residual << '\n'
            << "worst_internal_knot_centered_convergence="
            << worst_knot_convergence << '\n'
            << "worst_internal_knot_directional_linearity="
            << worst_knot_directional << '\n'
            << "minimum_final_one_sided_gap_ratio="
            << minimum_final_gap_ratio << '\n'
            << "corner_one_sided_gap_h0="
            << knot_results[2].maximum_one_sided_gap[0] << '\n'
            << "corner_one_sided_gap_h1="
            << knot_results[2].maximum_one_sided_gap[1] << '\n'
            << "corner_one_sided_gap_h2="
            << knot_results[2].maximum_one_sided_gap[2] << '\n'
            << "corner_one_sided_gap_h3="
            << knot_results[2].maximum_one_sided_gap[3] << '\n'
            << "gauge_gradient_residual=" << gauge_gradient_residual << '\n'
            << "pure_gauge_gradient=" << pure_gauge_gradient << '\n'
            << "zero_geometry_arms=" << zero_geometry_arms << '\n'
            << "worst_zero_connection_gradient=" << worst_zero_gradient << '\n'
            << "worst_zero_connection_convergence="
            << worst_zero_convergence << '\n'
            << "endpoint_threshold_gap=" << endpoint_threshold_gap << '\n'
            << "multicell_worldline_variation failures=" << failures << '\n'
            << "verdict=GLOBAL_DEPOSITED_ACTION_HAS_UNIQUE_INTERNAL_KNOT_VARIATION\n";
  return failures == 0 ? 0 : 1;
}


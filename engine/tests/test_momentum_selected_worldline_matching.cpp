/** FTD-0503: phase-space selection of the free multibody current 1-chain. */

#include "ftd/eft/momentum_selected_worldline_matching.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double rest_energy = 0.511;
constexpr double c_speed = 0.57735026918962576451;
constexpr double gate = 1e-12;
int failures = 0;
int transformed_matching_arms = 0;
double worst_best_residual = 0.0;
double minimum_residual_gap = INFINITY;
double worst_selected_current_residual = 0.0;
double worst_selected_continuity_residual = 0.0;
double worst_causal_residual = 0.0;
double collision_target_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  }
  return result;
}

double signature_difference(
    const ftd::eft::AggregateShapeCurrent& lhs,
    const ftd::eft::AggregateShapeCurrent& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({
      max_difference(lhs.rho_before, rhs.rho_before),
      max_difference(lhs.rho_after, rhs.rho_after),
      max_difference(lhs.current_x, rhs.current_x),
      max_difference(lhs.current_y, rhs.current_y),
      max_difference(lhs.current_z, rhs.current_z)});
}

std::array<ftd::Vec3, 4> square_points() {
  return {{{8.25, 8.25, 8.25}, {8.75, 8.25, 8.25},
           {8.75, 8.75, 8.25}, {8.25, 8.75, 8.25}}};
}

std::vector<ftd::eft::ShapeWorldline> make_history(
    const std::array<ftd::Vec3, 4>& points,
    const std::array<int, 4>& endpoint_index) {
  std::vector<ftd::eft::ShapeWorldline> result;
  result.reserve(4);
  for (int i = 0; i < 4; ++i) {
    result.push_back({points[static_cast<std::size_t>(i)],
                      points[static_cast<std::size_t>(endpoint_index[
                          static_cast<std::size_t>(i)])], +1});
  }
  return result;
}

std::vector<ftd::eft::PhaseSpaceCarrier> carriers_from_history(
    const std::vector<ftd::eft::ShapeWorldline>& history) {
  const ftd::eft::DualGaugePotentialSlab zero(L, c_speed);
  std::vector<ftd::eft::PhaseSpaceCarrier> result;
  result.reserve(history.size());
  for (const auto& line : history) {
    const auto legendre = ftd::eft::evaluate_discrete_legendre_worldline(
        line.start_position, line.end_position, line.charge,
        rest_energy, c_speed, zero, 0.0);
    if (!legendre.valid) return {};
    result.push_back({line.start_position,
                      legendre.kinetic_start, line.charge});
  }
  return result;
}

double maximum_length(
    const std::vector<ftd::eft::ShapeWorldline>& worldlines) {
  double result = 0.0;
  for (const auto& line : worldlines) {
    result = std::max(
        result, (line.end_position - line.start_position).mag());
  }
  return result;
}

struct ArmResult {
  bool valid = false;
  ftd::eft::MomentumSelectedMatching matching;
  double selected_current_residual = INFINITY;
  double selected_continuity_residual = INFINITY;
  double causal_residual = INFINITY;
};

ArmResult run_arm(
    const std::vector<ftd::eft::ShapeWorldline>& intended,
    const std::vector<ftd::Vec3>& unordered_endpoints,
    const std::vector<int>& expected_assignment) {
  ArmResult result;
  const auto carriers = carriers_from_history(intended);
  if (carriers.empty()) return result;
  result.matching = ftd::eft::match_free_worldline_endpoints(
      L, carriers, unordered_endpoints,
      rest_energy, c_speed, 1.0, gate);
  const auto selected = ftd::eft::worldlines_from_matching(
      carriers, unordered_endpoints, result.matching);
  if (selected.empty()) return result;
  const auto intended_current = ftd::eft::make_aggregate_shape_current(
      L, intended);
  const auto selected_current = ftd::eft::make_aggregate_shape_current(
      L, selected);
  result.selected_current_residual = signature_difference(
      intended_current, selected_current);
  result.selected_continuity_residual =
      selected_current.aggregate_continuity_residual;
  result.causal_residual = std::max(
      0.0, maximum_length(selected) - c_speed);
  worst_best_residual = std::max(
      worst_best_residual, result.matching.best_residual);
  minimum_residual_gap = std::min(
      minimum_residual_gap, result.matching.residual_gap);
  worst_selected_current_residual = std::max(
      worst_selected_current_residual,
      result.selected_current_residual);
  worst_selected_continuity_residual = std::max(
      worst_selected_continuity_residual,
      result.selected_continuity_residual);
  worst_causal_residual = std::max(
      worst_causal_residual, result.causal_residual);
  result.valid = result.matching.valid
      && result.matching.permutations_evaluated == 24
      && result.matching.exact_match_count == 1
      && result.matching.assignment == expected_assignment
      && result.matching.best_residual <= gate
      && result.matching.residual_gap > gate
      && result.selected_current_residual <= gate
      && result.selected_continuity_residual <= gate
      && result.causal_residual <= gate;
  return result;
}

ftd::Vec3 permute_signed(const ftd::Vec3& value,
                         const std::array<int, 3>& permutation,
                         const std::array<int, 3>& sign) {
  const std::array<double, 3> source{value.x, value.y, value.z};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

std::vector<ftd::eft::ShapeWorldline> transform_worldlines(
    const std::vector<ftd::eft::ShapeWorldline>& input,
    const std::array<int, 3>& permutation,
    const std::array<int, 3>& sign,
    const ftd::Vec3& translation) {
  const ftd::Vec3 origin{8.0, 8.0, 8.0};
  std::vector<ftd::eft::ShapeWorldline> result;
  result.reserve(input.size());
  for (const auto& line : input) {
    const auto transform = [&](const ftd::Vec3& point) {
      return origin + permute_signed(
          point - origin, permutation, sign) + translation;
    };
    result.push_back({transform(line.start_position),
                      transform(line.end_position), line.charge});
  }
  return result;
}

std::vector<ftd::Vec3> transform_points(
    const std::array<ftd::Vec3, 4>& input,
    const std::array<int, 3>& permutation,
    const std::array<int, 3>& sign,
    const ftd::Vec3& translation) {
  const ftd::Vec3 origin{8.0, 8.0, 8.0};
  std::vector<ftd::Vec3> result;
  result.reserve(input.size());
  for (const auto& point : input) {
    result.push_back(origin + permute_signed(
        point - origin, permutation, sign) + translation);
  }
  return result;
}

}  // namespace

int main() {
  const auto points = square_points();
  const std::vector<ftd::Vec3> endpoints(
      points.begin(), points.end());
  const auto stationary = make_history(points, {{0, 1, 2, 3}});
  const auto clockwise = make_history(points, {{1, 2, 3, 0}});
  const auto counterclockwise = make_history(points, {{3, 0, 1, 2}});

  const auto static_result = run_arm(
      stationary, endpoints, {0, 1, 2, 3});
  const auto clockwise_result = run_arm(
      clockwise, endpoints, {1, 2, 3, 0});
  const auto counterclockwise_result = run_arm(
      counterclockwise, endpoints, {3, 0, 1, 2});
  check("zero momentum uniquely selects the stationary matching",
        static_result.valid);
  check("clockwise momenta uniquely select the clockwise current chain",
        clockwise_result.valid);
  check("counterclockwise momenta uniquely select the reverse current chain",
        counterclockwise_result.valid);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  const std::array<ftd::Vec3, 3> translations{{
      {-2.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {2.0, -1.0, 1.0}}};
  bool transformed_ok = true;
  for (const auto& permutation : permutations) {
    for (int mask = 0; mask < 8; ++mask) {
      const std::array<int, 3> sign{{
          (mask & 1) ? -1 : +1,
          (mask & 2) ? -1 : +1,
          (mask & 4) ? -1 : +1}};
      for (const auto& translation : translations) {
        const auto transformed_endpoints = transform_points(
            points, permutation, sign, translation);
        transformed_ok = transformed_ok
            && run_arm(transform_worldlines(
                stationary, permutation, sign, translation),
                transformed_endpoints, {0, 1, 2, 3}).valid;
        ++transformed_matching_arms;
        transformed_ok = transformed_ok
            && run_arm(transform_worldlines(
                clockwise, permutation, sign, translation),
                transformed_endpoints, {1, 2, 3, 0}).valid;
        ++transformed_matching_arms;
        transformed_ok = transformed_ok
            && run_arm(transform_worldlines(
                counterclockwise, permutation, sign, translation),
                transformed_endpoints, {3, 0, 1, 2}).valid;
        ++transformed_matching_arms;
      }
    }
  }
  check("phase-space matching is unique in all transformed free arms",
        transformed_ok && transformed_matching_arms == 432
        && worst_best_residual <= gate
        && minimum_residual_gap > gate
        && worst_selected_current_residual <= gate
        && worst_selected_continuity_residual <= gate
        && worst_causal_residual <= gate);

  const ftd::eft::DualGaugePotentialSlab zero(L, c_speed);
  const std::array<ftd::Vec3, 2> collision_start{{
      {8.25, 8.25, 8.25}, {8.75, 8.25, 8.25}}};
  const ftd::Vec3 collision_endpoint{8.50, 8.25, 8.25};
  std::vector<ftd::eft::PhaseSpaceCarrier> collision_carriers;
  for (const auto& start : collision_start) {
    const auto legendre = ftd::eft::evaluate_discrete_legendre_worldline(
        start, collision_endpoint, +1,
        rest_energy, c_speed, zero, 0.0);
    collision_carriers.push_back({start, legendre.kinetic_start, +1});
    const auto recovered = start
        + ftd::eft::free_displacement_from_momentum(
            legendre.kinetic_start, rest_energy, c_speed, c_speed);
    collision_target_residual = std::max(
        collision_target_residual,
        max_abs(recovered - collision_endpoint));
  }
  const std::vector<ftd::Vec3> duplicate_endpoints{
      collision_endpoint, collision_endpoint};
  const auto collision = ftd::eft::match_free_worldline_endpoints(
      L, collision_carriers, duplicate_endpoints,
      rest_energy, c_speed, 1.0, gate);
  check("coincident free targets fail explicitly into collision semantics",
        collision_target_residual <= gate
        && !collision.valid && collision.collision_rule_required
        && collision.permutations_evaluated == 0
        && ftd::eft::worldlines_from_matching(
            collision_carriers, duplicate_endpoints, collision).empty());

  check("invalid matching inputs fail closed",
        !ftd::eft::match_free_worldline_endpoints(
            2, carriers_from_history(stationary), endpoints,
            rest_energy, c_speed).valid
        && !ftd::eft::match_free_worldline_endpoints(
            L, {}, {}, rest_energy, c_speed).valid);

  std::cout.precision(17);
  std::cout << "base_permutations_evaluated="
            << clockwise_result.matching.permutations_evaluated << '\n'
            << "base_valid_permutations="
            << clockwise_result.matching.valid_permutations << '\n'
            << "transformed_matching_arms="
            << transformed_matching_arms << '\n'
            << "worst_best_residual="
            << worst_best_residual << '\n'
            << "minimum_residual_gap="
            << minimum_residual_gap << '\n'
            << "worst_selected_current_residual="
            << worst_selected_current_residual << '\n'
            << "worst_selected_continuity_residual="
            << worst_selected_continuity_residual << '\n'
            << "worst_causal_residual="
            << worst_causal_residual << '\n'
            << "collision_target_residual="
            << collision_target_residual << '\n'
            << "momentum_selected_worldline_matching failures="
            << failures << '\n'
            << "verdict=PHASE_SPACE_SELECTS_FREE_WORLDLINE_CHAIN\n";
  return failures == 0 ? 0 : 1;
}

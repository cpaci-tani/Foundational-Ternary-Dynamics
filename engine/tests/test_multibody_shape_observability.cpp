/** FTD-0501: multibody kernel of additive trilinear shape/current. */

#include "ftd/eft/canonical_subcell_section.h"
#include "ftd/eft/multibody_shape_observability.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int covariance_history_pairs = 0;
double same_sign_history_difference = 0.0;
double same_sign_separation_squared_difference = 0.0;
double neutral_history_difference = 0.0;
double neutral_center_difference = 0.0;
double neutral_aggregate_current_l1 = 0.0;
double neutral_constituent_current_l1 = 0.0;
double vacuum_pair_signature_norm = 0.0;
double worst_covariant_kernel_difference = 0.0;
double worst_aggregate_continuity_residual = 0.0;
double same_sign_raw_remainder_difference = 0.0;
double neutral_raw_remainder_difference = 0.0;

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
  if (!lhs.valid || !rhs.valid || lhs.L != rhs.L) return INFINITY;
  return std::max({
      max_difference(lhs.rho_before, rhs.rho_before),
      max_difference(lhs.rho_after, rhs.rho_after),
      max_difference(lhs.current_x, rhs.current_x),
      max_difference(lhs.current_y, rhs.current_y),
      max_difference(lhs.current_z, rhs.current_z)});
}

double signature_norm(const ftd::eft::AggregateShapeCurrent& value) {
  std::vector<double> zero(value.rho_before.size(), 0.0);
  return std::max({
      max_difference(value.rho_before, zero),
      max_difference(value.rho_after, zero),
      max_difference(value.current_x, zero),
      max_difference(value.current_y, zero),
      max_difference(value.current_z, zero)});
}

std::vector<ftd::eft::ShapeWorldline> make_worldlines(
    const std::vector<double>& x,
    const std::vector<int>& charge,
    double displacement = 0.05) {
  std::vector<ftd::eft::ShapeWorldline> result;
  if (x.size() != charge.size()) return result;
  result.reserve(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) {
    result.push_back({{x[i], 8.0, 8.0},
                      {x[i] + displacement, 8.0, 8.0},
                      charge[i]});
  }
  return result;
}

struct RawAnchorRecord {
  std::vector<std::int8_t> state;
  std::vector<ftd::Vec3> remainder;
  bool valid = false;
};

RawAnchorRecord raw_anchor_record(
    const std::vector<ftd::eft::ShapeWorldline>& worldlines) {
  RawAnchorRecord result;
  result.state.assign(static_cast<std::size_t>(L * L * L), 0);
  result.remainder.assign(static_cast<std::size_t>(L * L * L), {});
  for (const auto& line : worldlines) {
    const auto chart = ftd::eft::centered_canonical_subcell_chart(
        line.start_position);
    if (!chart.valid) return result;
    const int x = (chart.anchor.x % L + L) % L;
    const int y = (chart.anchor.y % L + L) % L;
    const int z = (chart.anchor.z % L + L) % L;
    const int index = (x * L + y) * L + z;
    if (result.state[static_cast<std::size_t>(index)] != 0) return result;
    result.state[static_cast<std::size_t>(index)] =
        static_cast<std::int8_t>(line.charge);
    result.remainder[static_cast<std::size_t>(index)] = chart.remainder;
  }
  result.valid = true;
  return result;
}

double state_difference(const RawAnchorRecord& lhs,
                        const RawAnchorRecord& rhs) {
  if (!lhs.valid || !rhs.valid || lhs.state.size() != rhs.state.size()) {
    return INFINITY;
  }
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.state.size(); ++i) {
    result = std::max(
        result,
        std::abs(static_cast<double>(lhs.state[i])
                 - static_cast<double>(rhs.state[i])));
  }
  return result;
}

double remainder_difference(const RawAnchorRecord& lhs,
                            const RawAnchorRecord& rhs) {
  if (!lhs.valid || !rhs.valid
      || lhs.remainder.size() != rhs.remainder.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.remainder.size(); ++i) {
    result = std::max(
        result, max_abs(lhs.remainder[i] - rhs.remainder[i]));
  }
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

}  // namespace

int main() {
  const std::vector<int> plus_charges{+1, +1};
  const auto moment_plus_a = ftd::eft::one_dimensional_cic_moment(
      {0.25, 0.75}, plus_charges);
  const auto moment_plus_b = ftd::eft::one_dimensional_cic_moment(
      {0.375, 0.625}, plus_charges);
  check("1D CIC same-sign density factors exactly through Q and M",
        moment_plus_a.valid && moment_plus_b.valid
        && moment_plus_a.signed_charge == 2
        && moment_plus_a.signed_charge == moment_plus_b.signed_charge
        && std::abs(moment_plus_a.signed_first_moment - 1.0) <= gate
        && std::abs(moment_plus_a.signed_first_moment
                    - moment_plus_b.signed_first_moment) <= gate
        && std::abs(moment_plus_a.lower_weight - 1.0) <= gate
        && std::abs(moment_plus_a.upper_weight - 1.0) <= gate);

  const auto plus_a_lines = make_worldlines(
      {8.25, 8.75}, plus_charges);
  const auto plus_b_lines = make_worldlines(
      {8.375, 8.625}, plus_charges);
  const auto plus_a = ftd::eft::make_aggregate_shape_current(
      L, plus_a_lines);
  const auto plus_b = ftd::eft::make_aggregate_shape_current(
      L, plus_b_lines);
  same_sign_history_difference = signature_difference(plus_a, plus_b);
  same_sign_separation_squared_difference =
      ftd::eft::two_body_squared_separation(plus_a_lines)
      - ftd::eft::two_body_squared_separation(plus_b_lines);
  worst_aggregate_continuity_residual = std::max(
      plus_a.aggregate_continuity_residual,
      plus_b.aggregate_continuity_residual);
  check("distinct same-sign pairs have identical rho/current history",
        plus_a.valid && plus_b.valid
        && same_sign_history_difference <= gate);
  check("the invisible same-sign internal separation differs by 3/16",
        std::abs(same_sign_separation_squared_difference - 3.0 / 16.0)
            <= gate);

  const auto raw_plus_a = raw_anchor_record(plus_a_lines);
  const auto raw_plus_b = raw_anchor_record(plus_b_lines);
  same_sign_raw_remainder_difference = remainder_difference(
      raw_plus_a, raw_plus_b);
  check("same-sign raw anchors agree while remainders distinguish the pair",
        state_difference(raw_plus_a, raw_plus_b) <= gate
        && std::abs(same_sign_raw_remainder_difference - 0.125) <= gate);

  const std::vector<int> neutral_charges{+1, -1};
  const auto moment_neutral_a = ftd::eft::one_dimensional_cic_moment(
      {0.35, 0.65}, neutral_charges);
  const auto moment_neutral_b = ftd::eft::one_dimensional_cic_moment(
      {0.45, 0.75}, neutral_charges);
  check("1D CIC neutral density retains dipole moment but erases center",
        moment_neutral_a.valid && moment_neutral_b.valid
        && moment_neutral_a.signed_charge == 0
        && std::abs(moment_neutral_a.signed_first_moment + 0.3) <= gate
        && std::abs(moment_neutral_a.signed_first_moment
                    - moment_neutral_b.signed_first_moment) <= gate
        && std::abs(moment_neutral_a.lower_weight - 0.3) <= gate
        && std::abs(moment_neutral_a.upper_weight + 0.3) <= gate);

  const auto neutral_a_lines = make_worldlines(
      {8.35, 8.65}, neutral_charges);
  const auto neutral_b_lines = make_worldlines(
      {8.45, 8.75}, neutral_charges);
  const auto neutral_a = ftd::eft::make_aggregate_shape_current(
      L, neutral_a_lines);
  const auto neutral_b = ftd::eft::make_aggregate_shape_current(
      L, neutral_b_lines);
  neutral_history_difference = signature_difference(neutral_a, neutral_b);
  neutral_center_difference = max_abs(
      neutral_a.unsigned_center_before
      - neutral_b.unsigned_center_before);
  neutral_aggregate_current_l1 = std::max(
      neutral_a.aggregate_current_l1,
      neutral_b.aggregate_current_l1);
  neutral_constituent_current_l1 = std::min(
      neutral_a.constituent_current_l1,
      neutral_b.constituent_current_l1);
  worst_aggregate_continuity_residual = std::max({
      worst_aggregate_continuity_residual,
      neutral_a.aggregate_continuity_residual,
      neutral_b.aggregate_continuity_residual});
  check("translated neutral pairs have identical complete rho/current history",
        neutral_a.valid && neutral_b.valid
        && neutral_history_difference <= gate);
  check("the invisible neutral center differs by one tenth of a site",
        std::abs(neutral_center_difference - 0.1) <= gate);
  check("neutral aggregate current vanishes while constituents move",
        neutral_aggregate_current_l1 <= gate
        && neutral_constituent_current_l1 > 0.09);

  const auto raw_neutral_a = raw_anchor_record(neutral_a_lines);
  const auto raw_neutral_b = raw_anchor_record(neutral_b_lines);
  neutral_raw_remainder_difference = remainder_difference(
      raw_neutral_a, raw_neutral_b);
  check("neutral raw anchors agree while remainders retain its center",
        state_difference(raw_neutral_a, raw_neutral_b) <= gate
        && std::abs(neutral_raw_remainder_difference - 0.1) <= gate);

  const std::vector<ftd::eft::ShapeWorldline> coincident_pair{{
      {8.4, 8.0, 8.0}, {8.4, 8.0, 8.0}, +1},
      {{8.4, 8.0, 8.0}, {8.4, 8.0, 8.0}, -1}};
  const auto vacuum_kernel = ftd::eft::make_aggregate_shape_current(
      L, coincident_pair);
  vacuum_pair_signature_norm = signature_norm(vacuum_kernel);
  worst_aggregate_continuity_residual = std::max(
      worst_aggregate_continuity_residual,
      vacuum_kernel.aggregate_continuity_residual);
  check("signed shape cannot distinguish a coincident neutral pair from vacuum",
        vacuum_kernel.valid && vacuum_kernel.particle_count == 2
        && vacuum_kernel.total_charge == 0
        && vacuum_pair_signature_norm <= gate);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  const std::array<ftd::Vec3, 3> translations{{
      {-2.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {2.0, -1.0, 1.0}}};
  bool covariance_ok = true;
  for (const auto& permutation : permutations) {
    for (int mask = 0; mask < 8; ++mask) {
      const std::array<int, 3> sign{{
          (mask & 1) ? -1 : +1,
          (mask & 2) ? -1 : +1,
          (mask & 4) ? -1 : +1}};
      for (const auto& translation : translations) {
        const auto transformed_plus_a = ftd::eft::make_aggregate_shape_current(
            L, transform_worldlines(
                plus_a_lines, permutation, sign, translation));
        const auto transformed_plus_b = ftd::eft::make_aggregate_shape_current(
            L, transform_worldlines(
                plus_b_lines, permutation, sign, translation));
        const auto transformed_neutral_a =
            ftd::eft::make_aggregate_shape_current(
                L, transform_worldlines(
                    neutral_a_lines, permutation, sign, translation));
        const auto transformed_neutral_b =
            ftd::eft::make_aggregate_shape_current(
                L, transform_worldlines(
                    neutral_b_lines, permutation, sign, translation));
        const double plus_difference = signature_difference(
            transformed_plus_a, transformed_plus_b);
        const double neutral_difference = signature_difference(
            transformed_neutral_a, transformed_neutral_b);
        worst_covariant_kernel_difference = std::max({
            worst_covariant_kernel_difference,
            plus_difference, neutral_difference});
        worst_aggregate_continuity_residual = std::max({
            worst_aggregate_continuity_residual,
            transformed_plus_a.aggregate_continuity_residual,
            transformed_plus_b.aggregate_continuity_residual,
            transformed_neutral_a.aggregate_continuity_residual,
            transformed_neutral_b.aggregate_continuity_residual});
        covariance_ok = covariance_ok
            && plus_difference <= gate && neutral_difference <= gate;
        covariance_history_pairs += 2;
      }
    }
  }
  check("both kernels survive all cubic maps and integer translations",
        covariance_ok && covariance_history_pairs == 288
        && worst_covariant_kernel_difference <= gate);
  check("every aggregate history obeys exact discrete continuity",
        worst_aggregate_continuity_residual <= gate);

  check("invalid moment and worldline inputs fail closed",
        !ftd::eft::one_dimensional_cic_moment({0.2}, {}).valid
        && !ftd::eft::one_dimensional_cic_moment(
            {1.2}, {+1}).valid
        && !ftd::eft::make_aggregate_shape_current(
            2, plus_a_lines).valid
        && std::isnan(ftd::eft::two_body_squared_separation(
            {plus_a_lines.front()})));

  std::cout.precision(17);
  std::cout << "covariance_history_pairs="
            << covariance_history_pairs << '\n'
            << "same_sign_history_difference="
            << same_sign_history_difference << '\n'
            << "same_sign_separation_squared_difference="
            << same_sign_separation_squared_difference << '\n'
            << "same_sign_raw_remainder_difference="
            << same_sign_raw_remainder_difference << '\n'
            << "neutral_history_difference="
            << neutral_history_difference << '\n'
            << "neutral_center_difference="
            << neutral_center_difference << '\n'
            << "neutral_aggregate_current_l1="
            << neutral_aggregate_current_l1 << '\n'
            << "neutral_constituent_current_l1="
            << neutral_constituent_current_l1 << '\n'
            << "neutral_raw_remainder_difference="
            << neutral_raw_remainder_difference << '\n'
            << "vacuum_pair_signature_norm="
            << vacuum_pair_signature_norm << '\n'
            << "worst_covariant_kernel_difference="
            << worst_covariant_kernel_difference << '\n'
            << "worst_aggregate_continuity_residual="
            << worst_aggregate_continuity_residual << '\n'
            << "multibody_shape_observability failures="
            << failures << '\n'
            << "verdict=SHAPE_CURRENT_REQUIRES_WORLDLINE_DECOMPOSITION\n";
  return failures == 0 ? 0 : 1;
}

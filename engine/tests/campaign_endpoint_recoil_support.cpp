/**
 * @file campaign_endpoint_recoil_support.cpp
 * @brief FTD-0448 cubic covariance versus endpoint recoil support.
 */

#include "ftd/eft/endpoint_recoil_support.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int kRegisteredTotalScale = 12;

const std::array<std::array<int, 3>, 6> kPermutations{{
    {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
    {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};

std::vector<ftd::eft::SignedPermutation> cubic_group() {
  std::vector<ftd::eft::SignedPermutation> group;
  for (const auto& permutation : kPermutations)
    for (int sx : {-1, 1})
      for (int sy : {-1, 1})
        for (int sz : {-1, 1})
          group.push_back({permutation, {{sx, sy, sz}}});
  return group;
}

bool equal(const ftd::eft::CubicVector& a,
           const ftd::eft::CubicVector& b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

bool equal(const ftd::eft::EndpointRecoil& a,
           const ftd::eft::EndpointRecoil& b) {
  return equal(a.source, b.source) && equal(a.target, b.target);
}

int configuration_distance_squared(const ftd::eft::EndpointRecoil& a,
                                   const ftd::eft::EndpointRecoil& b) {
  ftd::eft::CubicVector source_difference{};
  ftd::eft::CubicVector target_difference{};
  for (int axis = 0; axis < 3; ++axis) {
    source_difference[static_cast<std::size_t>(axis)] =
        a.source[static_cast<std::size_t>(axis)]
        - b.source[static_cast<std::size_t>(axis)];
    target_difference[static_cast<std::size_t>(axis)] =
        a.target[static_cast<std::size_t>(axis)]
        - b.target[static_cast<std::size_t>(axis)];
  }
  return ftd::eft::norm2(source_difference)
      + ftd::eft::norm2(target_difference);
}

}  // namespace

int main() {
  const auto group = cubic_group();
  std::cout << "FTD-0448 endpoint recoil support v1\n";
  std::cout << "protocol,group_size,48,moore_directions,26,total_scale,"
            << kRegisteredTotalScale
            << ",candidate_rules,source_only_target_only_midpoint\n";

  int direction_count = 0;
  int total_failures = 0;
  int endpoint_energy_failures = 0;
  int distinct_failures = 0;
  int covariance_failures = 0;
  int midpoint_exchange_failures = 0;
  int asymmetric_exchange_failures = 0;
  int midpoint_energy_order_failures = 0;
  int minimum_configuration_distance_squared = 1000000;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++direction_count;
        const ftd::eft::CubicVector displacement{{dx, dy, dz}};
        const auto total = ftd::eft::integer_work_response(
            displacement, kRegisteredTotalScale);
        const auto source_only =
            ftd::eft::split_endpoint_recoil(total, 1, 1);
        const auto target_only =
            ftd::eft::split_endpoint_recoil(total, 0, 1);
        const auto midpoint =
            ftd::eft::split_endpoint_recoil(total, 1, 2);

        for (const auto& candidate : {source_only, target_only, midpoint})
          if (!equal(ftd::eft::total_endpoint_recoil(candidate), total))
            ++total_failures;
        if (ftd::eft::endpoint_quadratic_norm(source_only)
            != ftd::eft::endpoint_quadratic_norm(target_only))
          ++endpoint_energy_failures;
        const int distance_squared = configuration_distance_squared(
            source_only, target_only);
        minimum_configuration_distance_squared = std::min(
            minimum_configuration_distance_squared, distance_squared);
        if (distance_squared == 0) ++distinct_failures;
        if (!equal(ftd::eft::exchange_endpoints(midpoint), midpoint))
          ++midpoint_exchange_failures;
        if (!equal(ftd::eft::exchange_endpoints(source_only), target_only))
          ++asymmetric_exchange_failures;
        if (!(ftd::eft::endpoint_quadratic_norm(midpoint)
              < ftd::eft::endpoint_quadratic_norm(source_only)))
          ++midpoint_energy_order_failures;

        for (const auto& transform : group) {
          const auto transformed_displacement =
              ftd::eft::apply_signed_permutation(transform, displacement);
          const auto transformed_total = ftd::eft::integer_work_response(
              transformed_displacement, kRegisteredTotalScale);
          const std::array<ftd::eft::EndpointRecoil, 3> transformed_rules{{
              ftd::eft::split_endpoint_recoil(transformed_total, 1, 1),
              ftd::eft::split_endpoint_recoil(transformed_total, 0, 1),
              ftd::eft::split_endpoint_recoil(transformed_total, 1, 2)}};
          const std::array<ftd::eft::EndpointRecoil, 3> original_rules{{
              source_only, target_only, midpoint}};
          for (std::size_t rule = 0; rule < original_rules.size(); ++rule) {
            if (!equal(ftd::eft::apply_signed_permutation(
                           transform, original_rules[rule]),
                       transformed_rules[rule]))
              ++covariance_failures;
          }
        }
      }
    }
  }

  const bool totals_pass = total_failures == 0;
  const bool two_equal_norm_rules = endpoint_energy_failures == 0
      && distinct_failures == 0
      && minimum_configuration_distance_squared > 0;
  const bool all_rules_covariant = covariance_failures == 0;
  const bool midpoint_requires_exchange = midpoint_exchange_failures == 0
      && asymmetric_exchange_failures == 0
      && midpoint_energy_order_failures == 0;

  const char* verdict = "PROTOCOL_INVALID";
  if (direction_count == 26 && group.size() == 48 && totals_pass
      && two_equal_norm_rules && all_rules_covariant
      && midpoint_requires_exchange)
    verdict = "CUBIC_SYMMETRY_LEAVES_ENDPOINT_RECOIL_AMBIGUITY";

  std::cout << "support_family,directions," << direction_count
            << ",total_failures," << total_failures
            << ",equal_norm_failures," << endpoint_energy_failures
            << ",distinct_failures," << distinct_failures
            << ",minimum_configuration_distance_squared,"
            << minimum_configuration_distance_squared << '\n';
  std::cout << "covariance,tests," << direction_count * group.size() * 3
            << ",failures," << covariance_failures
            << ",all_rules_covariant,"
            << (all_rules_covariant ? "true" : "false") << '\n';
  std::cout << "endpoint_exchange,midpoint_failures,"
            << midpoint_exchange_failures
            << ",asymmetric_pair_failures," << asymmetric_exchange_failures
            << ",midpoint_energy_order_failures,"
            << midpoint_energy_order_failures
            << ",extra_principle_required,"
            << (midpoint_requires_exchange ? "true" : "false") << '\n';
  std::cout << "gates,totals_pass," << (totals_pass ? "true" : "false")
            << ",two_equal_norm_rules,"
            << (two_equal_norm_rules ? "true" : "false")
            << ",all_rules_covariant,"
            << (all_rules_covariant ? "true" : "false")
            << ",midpoint_requires_exchange,"
            << (midpoint_requires_exchange ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

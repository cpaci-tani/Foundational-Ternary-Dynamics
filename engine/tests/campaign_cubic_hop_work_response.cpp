/**
 * @file campaign_cubic_hop_work_response.cpp
 * @brief FTD-0447 exact cubic-stabilizer closure of isolated-hop work.
 */

#include "ftd/eft/cubic_hop_response.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int kRegisteredWork = 6;

const std::array<std::array<int, 3>, 6> kPermutations{{
    {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
    {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};

std::vector<ftd::eft::SignedPermutation> cubic_group() {
  std::vector<ftd::eft::SignedPermutation> group;
  for (const auto& permutation : kPermutations) {
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          group.push_back({permutation, {{sx, sy, sz}}});
        }
      }
    }
  }
  return group;
}

bool equal(const ftd::eft::CubicVector& a,
           const ftd::eft::CubicVector& b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

ftd::eft::CubicVector cross(const ftd::eft::CubicVector& a,
                            const ftd::eft::CubicVector& b) {
  return {{a[1] * b[2] - a[2] * b[1],
           a[2] * b[0] - a[0] * b[2],
           a[0] * b[1] - a[1] * b[0]}};
}

int determinant(const ftd::eft::CubicVector& a,
                const ftd::eft::CubicVector& b,
                const ftd::eft::CubicVector& c) {
  return ftd::eft::dot(a, cross(b, c));
}

int exact_three_column_rank(
    const std::vector<ftd::eft::CubicVector>& rows) {
  bool nonzero = false;
  for (const auto& row : rows) nonzero = nonzero || ftd::eft::norm2(row) != 0;
  if (!nonzero) return 0;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    for (std::size_t j = i + 1; j < rows.size(); ++j) {
      if (ftd::eft::norm2(cross(rows[i], rows[j])) == 0) continue;
      for (std::size_t k = j + 1; k < rows.size(); ++k)
        if (determinant(rows[i], rows[j], rows[k]) != 0) return 3;
      return 2;
    }
  }
  return 1;
}

std::vector<ftd::eft::CubicVector> stabilizer_constraints(
    const ftd::eft::CubicVector& displacement,
    const std::vector<ftd::eft::SignedPermutation>& group,
    int& stabilizer_size) {
  std::vector<ftd::eft::CubicVector> rows;
  stabilizer_size = 0;
  for (const auto& transform : group) {
    if (!equal(ftd::eft::apply_signed_permutation(transform, displacement),
               displacement))
      continue;
    ++stabilizer_size;
    for (int output_axis = 0; output_axis < 3; ++output_axis) {
      ftd::eft::CubicVector row{};
      const int input_axis = transform.permutation[
          static_cast<std::size_t>(output_axis)];
      row[static_cast<std::size_t>(input_axis)] +=
          transform.signs[static_cast<std::size_t>(output_axis)];
      row[static_cast<std::size_t>(output_axis)] -= 1;
      rows.push_back(row);
    }
  }
  return rows;
}

}  // namespace

int main() {
  const auto group = cubic_group();
  std::cout << "FTD-0447 cubic hop work response v1\n";
  std::cout << "protocol,group_size,48,moore_directions,26,registered_work,"
            << kRegisteredWork << ",expected_fixed_dimension,1\n";

  int direction_count = 0;
  int face_count = 0;
  int edge_count = 0;
  int corner_count = 0;
  int rank_failures = 0;
  int displacement_fixed_failures = 0;
  int stabilizer_size_failures = 0;
  int work_failures = 0;
  int covariance_failures = 0;
  int minimum_stabilizer_size = 1000;
  int maximum_stabilizer_size = 0;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::eft::CubicVector displacement{{dx, dy, dz}};
        const int length_squared = ftd::eft::norm2(displacement);
        if (length_squared == 1) ++face_count;
        if (length_squared == 2) ++edge_count;
        if (length_squared == 3) ++corner_count;
        ++direction_count;

        int stabilizer_size = 0;
        const auto rows = stabilizer_constraints(
            displacement, group, stabilizer_size);
        minimum_stabilizer_size = std::min(minimum_stabilizer_size,
                                           stabilizer_size);
        maximum_stabilizer_size = std::max(maximum_stabilizer_size,
                                           stabilizer_size);
        const int expected_stabilizer_size = length_squared == 1 ? 8
            : (length_squared == 2 ? 4 : 6);
        if (stabilizer_size != expected_stabilizer_size)
          ++stabilizer_size_failures;
        if (exact_three_column_rank(rows) != 2) ++rank_failures;
        for (const auto& row : rows)
          if (ftd::eft::dot(row, displacement) != 0)
            ++displacement_fixed_failures;

        const auto response = ftd::eft::integer_work_response(
            displacement, kRegisteredWork);
        if (ftd::eft::dot(response, displacement) != kRegisteredWork)
          ++work_failures;
        for (const auto& transform : group) {
          const auto transformed_displacement =
              ftd::eft::apply_signed_permutation(transform, displacement);
          const auto transformed_response =
              ftd::eft::apply_signed_permutation(transform, response);
          const auto response_after_transform =
              ftd::eft::integer_work_response(
                  transformed_displacement, kRegisteredWork);
          if (!equal(transformed_response, response_after_transform))
            ++covariance_failures;
        }
      }
    }
  }

  const bool combinatorics_pass = group.size() == 48
      && direction_count == 26 && face_count == 6
      && edge_count == 12 && corner_count == 8;
  const bool stabilizer_pass = stabilizer_size_failures == 0
      && rank_failures == 0 && displacement_fixed_failures == 0;
  const bool unique_response_pass = work_failures == 0
      && covariance_failures == 0;

  const char* verdict = "PROTOCOL_INVALID";
  if (combinatorics_pass && stabilizer_pass && unique_response_pass)
    verdict = "CUBIC_STABILIZER_FIXES_LONGITUDINAL_WORK_RESPONSE";

  std::cout << "combinatorics,directions," << direction_count
            << ",face," << face_count << ",edge," << edge_count
            << ",corner," << corner_count << ",pass,"
            << (combinatorics_pass ? "true" : "false") << '\n';
  std::cout << "stabilizers,minimum_size," << minimum_stabilizer_size
            << ",maximum_size," << maximum_stabilizer_size
            << ",size_failures," << stabilizer_size_failures
            << ",rank_failures," << rank_failures
            << ",displacement_fixed_failures,"
            << displacement_fixed_failures
            << ",fixed_dimension,1,pass,"
            << (stabilizer_pass ? "true" : "false") << '\n';
  std::cout << "work_response,work_failures," << work_failures
            << ",covariance_tests," << direction_count * group.size()
            << ",covariance_failures," << covariance_failures
            << ",pass," << (unique_response_pass ? "true" : "false")
            << '\n';
  std::cout << "gates,combinatorics_pass,"
            << (combinatorics_pass ? "true" : "false")
            << ",stabilizer_pass," << (stabilizer_pass ? "true" : "false")
            << ",unique_response_pass,"
            << (unique_response_pass ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

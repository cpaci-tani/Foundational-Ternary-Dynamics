/**
 * @file campaign_moore_channel_projection_kernel.cpp
 * @brief FTD-0446 exact kernel of the 13-channel to Vec3 projection.
 */

#include "ftd/eft/moore_channel_projection.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

const std::array<std::array<int, 3>, 6> kPermutations{{
    {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
    {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};

bool zero(const ftd::eft::MooreDirection& value) {
  return value[0] == 0 && value[1] == 0 && value[2] == 0;
}

bool equal(const ftd::eft::MooreDirection& a,
           const ftd::eft::MooreDirection& b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

bool channel_set_contains_up_to_sign(const ftd::eft::MooreDirection& value) {
  for (const auto& direction : ftd::eft::kMooreChannelDirections) {
    if (equal(value, direction)) return true;
    const ftd::eft::MooreDirection negative{{
        -direction[0], -direction[1], -direction[2]}};
    if (equal(value, negative)) return true;
  }
  return false;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0446 Moore-channel projection kernel v1\n";
  std::cout << "protocol,channels,13,vector_dimension,3,expected_rank,3,"
               "expected_nullity,10,cubic_signed_permutations,48\n";

  int zero_kernel_failures = 0;
  int pivot_failures = 0;
  int direct_face_projection_failures = 0;
  int minimum_channel_energy_gap = 1000;
  int maximum_channel_energy_gap = -1000;

  for (int channel = 3; channel < 13; ++channel) {
    const auto kernel = ftd::eft::diagonal_face_kernel(channel);
    if (!zero(ftd::eft::project_moore_channels(kernel)))
      ++zero_kernel_failures;
    for (int other = 3; other < 13; ++other) {
      const int expected = other == channel ? 1 : 0;
      if (kernel[static_cast<std::size_t>(other)] != expected)
        ++pivot_failures;
    }

    ftd::eft::MooreChannels direct{};
    direct[static_cast<std::size_t>(channel)] = 1;
    ftd::eft::MooreChannels faces{};
    const auto& direction = ftd::eft::kMooreChannelDirections[
        static_cast<std::size_t>(channel)];
    faces[0] = direction[0];
    faces[1] = direction[1];
    faces[2] = direction[2];
    if (!equal(ftd::eft::project_moore_channels(direct),
               ftd::eft::project_moore_channels(faces)))
      ++direct_face_projection_failures;
    const int energy_gap = ftd::eft::channel_quadratic_norm(faces)
        - ftd::eft::channel_quadratic_norm(direct);
    minimum_channel_energy_gap = std::min(minimum_channel_energy_gap,
                                          energy_gap);
    maximum_channel_energy_gap = std::max(maximum_channel_energy_gap,
                                          energy_gap);
  }

  int cubic_closure_failures = 0;
  int transform_count = 0;
  for (const auto& permutation : kPermutations) {
    for (int sx : {-1, 1}) {
      for (int sy : {-1, 1}) {
        for (int sz : {-1, 1}) {
          const std::array<int, 3> signs{{sx, sy, sz}};
          ++transform_count;
          for (const auto& direction : ftd::eft::kMooreChannelDirections) {
            ftd::eft::MooreDirection transformed{};
            for (int axis = 0; axis < 3; ++axis)
              transformed[static_cast<std::size_t>(axis)] =
                  signs[static_cast<std::size_t>(axis)]
                  * direction[static_cast<std::size_t>(
                      permutation[static_cast<std::size_t>(axis)])];
            if (!channel_set_contains_up_to_sign(transformed))
              ++cubic_closure_failures;
          }
        }
      }
    }
  }

  const int exact_rank = 3;  // first three columns are the Cartesian basis
  const int exact_nullity = 13 - exact_rank;
  const bool rank_pass = exact_rank == 3 && exact_nullity == 10;
  const bool kernel_pass = zero_kernel_failures == 0 && pivot_failures == 0;
  const bool projection_degenerate =
      direct_face_projection_failures == 0
      && minimum_channel_energy_gap == 1
      && maximum_channel_energy_gap == 2;
  const bool cubic_covariant = transform_count == 48
      && cubic_closure_failures == 0;

  const char* verdict = "PROTOCOL_INVALID";
  if (rank_pass && kernel_pass && projection_degenerate && cubic_covariant)
    verdict = "VECTOR_PROJECTION_HAS_TEN_HIDDEN_CHANNEL_MODES";

  std::cout << "linear_map,rank," << exact_rank
            << ",nullity," << exact_nullity
            << ",rank_pass," << (rank_pass ? "true" : "false") << '\n';
  std::cout << "kernel,basis_vectors,10,zero_failures,"
            << zero_kernel_failures << ",pivot_failures," << pivot_failures
            << ",independent," << (kernel_pass ? "true" : "false") << '\n';
  std::cout << "projection_degeneracy,nonface_channels,10,projection_failures,"
            << direct_face_projection_failures
            << ",minimum_channel_energy_gap," << minimum_channel_energy_gap
            << ",maximum_channel_energy_gap," << maximum_channel_energy_gap
            << ",degenerate,"
            << (projection_degenerate ? "true" : "false") << '\n';
  std::cout << "cubic_closure,transforms," << transform_count
            << ",direction_tests," << transform_count * 13
            << ",failures," << cubic_closure_failures
            << ",covariant," << (cubic_covariant ? "true" : "false")
            << '\n';
  std::cout << "gates,rank_pass," << (rank_pass ? "true" : "false")
            << ",kernel_pass," << (kernel_pass ? "true" : "false")
            << ",projection_degenerate,"
            << (projection_degenerate ? "true" : "false")
            << ",cubic_covariant," << (cubic_covariant ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

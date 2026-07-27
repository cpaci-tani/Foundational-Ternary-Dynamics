/**
 * @file campaign_half_tick_link_exchange.cpp
 * @brief FTD-0451 reversible half-tick Moore-link exchange ledger.
 */

#include "ftd/eft/half_tick_link_exchange.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int kTick = 17;
constexpr double kWork = 1e-4;
constexpr double kClosureGate = 1e-12;

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

ftd::Vec3 transform(const ftd::eft::SignedPermutation& transform,
                    const ftd::Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int axis = 0; axis < 3; ++axis)
    output[static_cast<std::size_t>(axis)] =
        transform.signs[static_cast<std::size_t>(axis)]
        * input[static_cast<std::size_t>(
            transform.permutation[static_cast<std::size_t>(axis)])];
  return {output[0], output[1], output[2]};
}

bool equal(const ftd::eft::CubicVector& a,
           const ftd::eft::CubicVector& b) {
  return a[0] == b[0] && a[1] == b[1] && a[2] == b[2];
}

ftd::Vec3 transverse_unit(const ftd::Vec3& direction) {
  const ftd::Vec3 reference = std::abs(direction.x) < 0.8
      ? ftd::Vec3{1.0, 0.0, 0.0}
      : ftd::Vec3{0.0, 1.0, 0.0};
  auto transverse = ftd::Vec3::cross(direction, reference);
  transverse *= 1.0 / transverse.mag();
  return transverse;
}

}  // namespace

int main() {
  const auto group = cubic_group();
  std::cout << std::setprecision(17);
  std::cout << "FTD-0451 half-tick link exchange v1\n";
  std::cout << "protocol,tick," << kTick << ",twice_time,"
            << 2 * kTick + 1 << ",directions,26,group_size,48,work,"
            << kWork << ",closure_gate," << kClosureGate << '\n';

  int direction_count = 0;
  int channel_failures = 0;
  int reverse_channel_failures = 0;
  int covariance_failures = 0;
  bool exchanges_valid = true;
  double worst_momentum_residual = 0.0;
  double worst_energy_residual = 0.0;
  double worst_particle_round_trip_residual = 0.0;
  double worst_link_momentum_cancel_residual = 0.0;
  double worst_link_energy_cancel_residual = 0.0;
  double worst_covariance_residual = 0.0;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++direction_count;
        const ftd::eft::CubicVector displacement{{dx, dy, dz}};
        const ftd::Vec3 displacement_vec{
            static_cast<double>(dx), static_cast<double>(dy),
            static_cast<double>(dz)};
        const auto direction = displacement_vec
            * (1.0 / displacement_vec.mag());
        const auto velocity = direction * 0.15
            + transverse_unit(direction) * 0.03;
        const auto momentum = ftd::eft::production_flat_momentum(velocity);
        const auto forward = ftd::eft::make_half_tick_link_exchange(
            kTick, momentum, displacement, kWork);
        const ftd::eft::CubicVector reverse_displacement{{-dx, -dy, -dz}};
        const auto reverse = ftd::eft::make_half_tick_link_exchange(
            kTick, forward.particle_momentum_after,
            reverse_displacement, -kWork);
        exchanges_valid = exchanges_valid && forward.valid && reverse.valid;
        if (!equal(ftd::eft::reconstruct_channel_displacement(forward.channel),
                   displacement))
          ++channel_failures;
        if (!equal(ftd::eft::reconstruct_channel_displacement(reverse.channel),
                   reverse_displacement)
            || reverse.channel.index != forward.channel.index
            || reverse.channel.orientation != -forward.channel.orientation)
          ++reverse_channel_failures;
        worst_momentum_residual = std::max(
            worst_momentum_residual,
            std::max(forward.momentum_residual, reverse.momentum_residual));
        worst_energy_residual = std::max(
            worst_energy_residual,
            std::max(std::abs(forward.energy_residual),
                     std::abs(reverse.energy_residual)));
        worst_particle_round_trip_residual = std::max(
            worst_particle_round_trip_residual,
            (reverse.particle_momentum_after - momentum).mag());
        worst_link_momentum_cancel_residual = std::max(
            worst_link_momentum_cancel_residual,
            (forward.field_momentum_exchange
             + reverse.field_momentum_exchange).mag());
        worst_link_energy_cancel_residual = std::max(
            worst_link_energy_cancel_residual,
            std::abs(forward.field_energy_exchange
                     + reverse.field_energy_exchange));

        for (const auto& cubic : group) {
          const auto transformed_displacement =
              ftd::eft::apply_signed_permutation(cubic, displacement);
          const auto transformed_momentum = transform(cubic, momentum);
          const auto transformed = ftd::eft::make_half_tick_link_exchange(
              kTick, transformed_momentum, transformed_displacement, kWork);
          exchanges_valid = exchanges_valid && transformed.valid;
          const double momentum_covariance =
              (transformed.particle_momentum_after
               - transform(cubic, forward.particle_momentum_after)).mag();
          const double recoil_covariance =
              (transformed.field_momentum_exchange
               - transform(cubic, forward.field_momentum_exchange)).mag();
          worst_covariance_residual = std::max(
              worst_covariance_residual,
              std::max(momentum_covariance, recoil_covariance));
          if (!equal(ftd::eft::reconstruct_channel_displacement(
                         transformed.channel),
                     transformed_displacement))
            ++covariance_failures;
        }
      }
    }
  }

  const bool channel_pass = channel_failures == 0
      && reverse_channel_failures == 0 && covariance_failures == 0;
  const bool local_closure_pass = exchanges_valid
      && worst_momentum_residual <= kClosureGate
      && worst_energy_residual <= kClosureGate;
  const bool reverse_pass =
      worst_particle_round_trip_residual <= kClosureGate
      && worst_link_momentum_cancel_residual <= kClosureGate
      && worst_link_energy_cancel_residual <= kClosureGate;
  const bool covariance_pass =
      worst_covariance_residual <= kClosureGate;

  const char* verdict = "PROTOCOL_INVALID";
  if (direction_count == 26 && group.size() == 48 && channel_pass
      && local_closure_pass && reverse_pass && covariance_pass)
    verdict = "REVERSIBLE_HALF_TICK_LINK_LEDGER_CONSTRUCTED_NOT_DYNAMICS";

  std::cout << "channels,failures," << channel_failures
            << ",reverse_failures," << reverse_channel_failures
            << ",covariance_reconstruction_failures," << covariance_failures
            << ",pass," << (channel_pass ? "true" : "false") << '\n';
  std::cout << "local_closure,worst_momentum_residual,"
            << worst_momentum_residual
            << ",worst_energy_residual," << worst_energy_residual
            << ",pass," << (local_closure_pass ? "true" : "false")
            << '\n';
  std::cout << "reverse,worst_particle_round_trip_residual,"
            << worst_particle_round_trip_residual
            << ",worst_link_momentum_cancel_residual,"
            << worst_link_momentum_cancel_residual
            << ",worst_link_energy_cancel_residual,"
            << worst_link_energy_cancel_residual
            << ",pass," << (reverse_pass ? "true" : "false") << '\n';
  std::cout << "covariance,tests," << direction_count * group.size()
            << ",worst_residual," << worst_covariance_residual
            << ",pass," << (covariance_pass ? "true" : "false") << '\n';
  std::cout << "gates,exchanges_valid,"
            << (exchanges_valid ? "true" : "false")
            << ",channel_pass," << (channel_pass ? "true" : "false")
            << ",local_closure_pass,"
            << (local_closure_pass ? "true" : "false")
            << ",reverse_pass," << (reverse_pass ? "true" : "false")
            << ",covariance_pass," << (covariance_pass ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

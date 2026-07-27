/** FTD-0505: exact tick-boundary collision resolution trilemma. */

#include "ftd/eft/boundary_collision_resolution.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double a = 0.25;
constexpr double dt = 1.0;
constexpr double v = a / dt;
constexpr double rest_energy = 0.511;
constexpr double c_speed = 0.57735026918962576451;
constexpr double gate = 1e-12;
int failures = 0;
int boundary_arms = 0;
int separated_attempts = 0;
int exclusion_arms = 0;
int timing_shift_arms = 0;
double worst_collision_time_residual = 0.0;
double minimum_temporal_causal_defect = INFINITY;
double worst_exclusion_residual = 0.0;
double worst_exclusion_reversal_residual = 0.0;
double minimum_exclusion_signature_difference = INFINITY;
double minimum_timing_energy_shift = INFINITY;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

ftd::Vec3 permute_signed(const ftd::Vec3& value,
                         const std::array<int, 3>& permutation,
                         const std::array<int, 3>& sign) {
  const std::array<double, 3> source{value.x, value.y, value.z};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

}  // namespace

int main() {
  const ftd::Vec3 base_center{8.5, 8.5, 8.5};
  const ftd::Vec3 base_direction{1.0, 2.0, 3.0};

  const std::array<double, 4> distances{{
      1.0 / 64.0, 1.0 / 32.0, 1.0 / 16.0, 1.0 / 8.0}};
  bool separation_impossible = true;
  for (double distance : distances) {
    for (double output_speed : {v, c_speed}) {
      const auto attempt = ftd::eft::analyze_same_tick_separated_output(
          a, v, dt, distance, output_speed, c_speed, gate);
      separation_impossible = separation_impossible && attempt.valid
          && !attempt.same_tick_causal
          && std::abs(attempt.temporal_causal_defect
              - distance / output_speed) <= gate;
      minimum_temporal_causal_defect = std::min(
          minimum_temporal_causal_defect,
          attempt.temporal_causal_defect);
      ++separated_attempts;
    }
  }
  check("positive separation after a boundary collision exceeds the tick",
        separation_impossible && separated_attempts == 8
        && minimum_temporal_causal_defect > 0.0);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  const std::array<ftd::Vec3, 3> translations{{
      {-2.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {2.0, -1.0, 1.0}}};
  const std::array<double, 3> radii{{a / 4.0, a / 2.0, 3.0 * a / 4.0}};
  bool boundary_ok = true;
  bool exclusion_ok = true;
  for (const auto& permutation : permutations) {
    for (int mask = 0; mask < 8; ++mask) {
      const std::array<int, 3> sign_map{{
          (mask & 1) ? -1 : +1,
          (mask & 2) ? -1 : +1,
          (mask & 4) ? -1 : +1}};
      const ftd::Vec3 direction = permute_signed(
          base_direction, permutation, sign_map);
      for (const auto& translation : translations) {
        for (int polarity : {-1, +1}) {
          const auto boundary =
              ftd::eft::analyze_boundary_collision_resolution(
                  base_center + translation, direction,
                  a, v, dt, polarity, gate);
          boundary_ok = boundary_ok && boundary.valid
              && boundary.endpoint_capacity.valid
              && boundary.endpoint_capacity.minimum_charge_defect == 1
              && boundary.minimum_charge_alphabet_symbols == 5
              && boundary.minimum_auxiliary_occupancy_bits == 1;
          worst_collision_time_residual = std::max(
              worst_collision_time_residual,
              boundary.collision_time_residual);
          ++boundary_arms;

          std::array<ftd::eft::PrecontactExclusionResult, 3> family{};
          for (std::size_t i = 0; i < radii.size(); ++i) {
            family[i] = ftd::eft::analyze_precontact_exclusion(
                L, base_center + translation, direction,
                a, v, dt, radii[i], polarity,
                rest_energy, c_speed, gate);
            const auto& result = family[i];
            exclusion_ok = exclusion_ok && result.valid
                && result.endpoint_separation > 0.0
                && std::abs(result.endpoint_separation
                    - 4.0 * radii[i]) <= gate;
            worst_exclusion_residual = std::max({
                worst_exclusion_residual, result.energy_residual,
                result.momentum_residual, result.charge_residual,
                result.causal_residual, result.continuity_residual});
            worst_exclusion_reversal_residual = std::max(
                worst_exclusion_reversal_residual,
                result.reversal_residual);
            ++exclusion_arms;
          }
          for (std::size_t i = 0; i < family.size(); ++i) {
            for (std::size_t j = i + 1; j < family.size(); ++j) {
              minimum_exclusion_signature_difference = std::min(
                  minimum_exclusion_signature_difference,
                  ftd::eft::collision_signature_difference(
                      family[i].current, family[j].current));
            }
          }
        }
      }
    }
  }
  check("boundary overload and five-symbol charge bound are cubic covariant",
        boundary_ok && boundary_arms == 288
        && worst_collision_time_residual <= gate);
  check("each selected finite exclusion radius is conservative and reversible",
        exclusion_ok && exclusion_arms == 864
        && worst_exclusion_residual <= gate
        && worst_exclusion_reversal_residual <= gate);
  check("distinct exclusion radii produce distinct exact current histories",
        minimum_exclusion_signature_difference > gate);

  bool timing_ok = true;
  for (double delta : {0.1, 0.2, 0.3}) {
    const auto shift = ftd::eft::analyze_collision_timing_shift(
        a, dt, delta, rest_energy, c_speed);
    timing_ok = timing_ok && shift.valid
        && shift.early_speed > shift.baseline_speed
        && shift.late_speed < shift.baseline_speed
        && shift.early_energy_shift > 0.0
        && shift.late_energy_shift < 0.0
        && shift.early_causal_residual <= gate;
    minimum_timing_energy_shift = std::min(
        minimum_timing_energy_shift,
        shift.minimum_absolute_energy_shift);
    ++timing_shift_arms;
  }
  check("moving the collision off the tick changes production energy",
        timing_ok && timing_shift_arms == 3
        && minimum_timing_energy_shift > gate);

  check("zero-radius and zero-phase limits retain the boundary obstruction",
        !ftd::eft::analyze_precontact_exclusion(
            L, base_center, base_direction, a, v, dt, 0.0,
            +1, rest_energy, c_speed, gate).valid
        && !ftd::eft::analyze_collision_timing_shift(
            a, dt, 0.0, rest_energy, c_speed).valid);
  check("invalid boundary inputs fail closed",
        !ftd::eft::analyze_boundary_collision_resolution(
            base_center, {}, a, v, dt, +1).valid
        && !ftd::eft::analyze_same_tick_separated_output(
            a, v, dt, 0.1, 2.0 * c_speed, c_speed).valid);

  std::cout.precision(17);
  std::cout << "boundary_arms=" << boundary_arms << '\n'
            << "separated_attempts=" << separated_attempts << '\n'
            << "exclusion_arms=" << exclusion_arms << '\n'
            << "timing_shift_arms=" << timing_shift_arms << '\n'
            << "worst_collision_time_residual="
            << worst_collision_time_residual << '\n'
            << "minimum_temporal_causal_defect="
            << minimum_temporal_causal_defect << '\n'
            << "worst_exclusion_residual="
            << worst_exclusion_residual << '\n'
            << "worst_exclusion_reversal_residual="
            << worst_exclusion_reversal_residual << '\n'
            << "minimum_exclusion_signature_difference="
            << minimum_exclusion_signature_difference << '\n'
            << "minimum_timing_energy_shift="
            << minimum_timing_energy_shift << '\n'
            << "boundary_collision_resolution failures=" << failures << '\n'
            << "verdict=BOUNDARY_COLLISION_REQUIRES_CAPACITY_RANGE_OR_PHASE\n";
  return failures == 0 ? 0 : 1;
}

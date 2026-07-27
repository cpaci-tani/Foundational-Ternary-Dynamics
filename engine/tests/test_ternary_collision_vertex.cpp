/** FTD-0504: ternary collision capacity and identical-crossing quotient. */

#include "ftd/eft/ternary_collision_vertex.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double rest_energy = 0.511;
constexpr double c_speed = 0.57735026918962576451;
constexpr double gate = 1e-12;
int failures = 0;
int capacity_arms = 0;
int transformed_crossing_arms = 0;
double worst_phase_residual = 0.0;
double worst_current_residual = 0.0;
double worst_conservation_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_reversal_residual = 0.0;

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

void accumulate(const ftd::eft::IdenticalCrossingResult& result) {
  worst_phase_residual = std::max(
      worst_phase_residual, result.phase_space_multiset_residual);
  worst_current_residual = std::max(
      worst_current_residual, result.current_signature_residual);
  worst_conservation_residual = std::max({worst_conservation_residual,
      result.energy_residual, result.momentum_residual,
      result.charge_residual, result.causal_residual});
  worst_continuity_residual = std::max(
      worst_continuity_residual, result.continuity_residual);
  worst_reversal_residual = std::max(
      worst_reversal_residual, result.time_reversal_residual);
}

}  // namespace

int main() {
  bool capacity_ok = true;
  for (int sign : {-1, +1}) {
    for (int multiplicity = 2; multiplicity <= 8; ++multiplicity) {
      const auto result = ftd::eft::analyze_ternary_same_sign_capacity(
          multiplicity, sign);
      capacity_ok = capacity_ok && result.valid
          && result.required_charge == sign * multiplicity
          && result.best_ternary_state == sign
          && result.minimum_charge_defect == multiplicity - 1;
      ++capacity_arms;
    }
  }
  check("same-sign multiplicity has exact ternary capacity defect m-1",
        capacity_ok && capacity_arms == 14);

  const ftd::eft::CarrierIntrinsicAttributes identical{
      +1, +1, 2, 4, {7, 13}};
  auto base = ftd::eft::analyze_identical_crossing(
      L, {8.5, 8.5, 8.5}, {1.0, 2.0, 3.0},
      0.25, 0.40, 1.0, +1, rest_energy, c_speed,
      identical, identical, gate);
  accumulate(base);
  check("interior pass-through and bounce are one unlabeled phase-space event",
        base.valid && !base.boundary_overload
        && base.attributes_identical && base.label_quotient_equivalent
        && std::abs(base.collision_time - 0.625) <= gate
        && std::abs(base.remaining_time - 0.375) <= gate
        && base.phase_space_multiset_residual <= gate);
  check("interior pass-through and bounce deposit the same exact face current",
        base.current_signature_residual <= gate
        && base.continuity_residual <= gate
        && base.pass_through.valid && base.elastic_bounce.valid);
  check("identical crossing conserves energy, momentum, charge, and causality",
        base.energy_residual <= gate && base.momentum_residual <= gate
        && base.charge_residual <= gate && base.causal_residual <= gate);
  check("identical crossing is exactly reversible as an unlabeled current chain",
        base.time_reversal_residual <= gate);

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
      const ftd::Vec3 direction = permute_signed(
          {1.0, 2.0, 3.0}, permutation, sign);
      for (const auto& translation : translations) {
        const auto result = ftd::eft::analyze_identical_crossing(
            L, ftd::Vec3{8.5, 8.5, 8.5} + translation, direction,
            0.25, 0.40, 1.0, +1, rest_energy, c_speed,
            identical, identical, gate);
        accumulate(result);
        transformed_ok = transformed_ok && result.valid
            && result.label_quotient_equivalent
            && result.phase_space_multiset_residual <= gate
            && result.current_signature_residual <= gate
            && result.continuity_residual <= gate
            && result.time_reversal_residual <= gate;
        ++transformed_crossing_arms;
      }
    }
  }
  check("identical-crossing quotient survives cubic maps and translations",
        transformed_ok && transformed_crossing_arms == 144);

  const auto boundary = ftd::eft::analyze_identical_crossing(
      L, {8.5, 8.5, 8.5}, {1.0, 2.0, 3.0},
      0.25, 0.25, 1.0, +1, rest_energy, c_speed,
      identical, identical, gate);
  check("tick-boundary coincidence returns ternary endpoint overload",
        boundary.valid && boundary.boundary_overload
        && !boundary.label_quotient_equivalent
        && boundary.endpoint_capacity.valid
        && boundary.endpoint_capacity.minimum_charge_defect == 1
        && boundary.charge_residual == 1.0);

  bool distinguishability_ok = true;
  std::array<ftd::eft::CarrierIntrinsicAttributes, 5> different{{
      {-1, +1, 2, 4, {7, 13}},
      {+1, -1, 2, 4, {7, 13}},
      {+1, +1, 3, 4, {7, 13}},
      {+1, +1, 2, 5, {7, 13}},
      {+1, +1, 2, 4, {7, 14}}}};
  for (const auto& attributes : different) {
    const auto result = ftd::eft::analyze_identical_crossing(
        L, {8.5, 8.5, 8.5}, {1.0, 2.0, 3.0},
        0.25, 0.40, 1.0, +1, rest_energy, c_speed,
        identical, attributes, gate);
    distinguishability_ok = distinguishability_ok && result.valid
        && !result.attributes_identical
        && !result.label_quotient_equivalent;
  }
  check("any transported physical attribute defeats the label quotient",
        distinguishability_ok);

  const auto counterfamily =
      ftd::eft::analyze_elastic_scattering_counterfamily(
          0.40, rest_energy, c_speed);
  check("3D conservation admits at least five distinct elastic outputs",
        counterfamily.valid && counterfamily.output_count == 5
        && counterfamily.maximum_total_momentum_residual <= gate
        && counterfamily.maximum_total_energy_residual <= gate
        && counterfamily.minimum_direction_separation > 0.0);

  check("invalid collision inputs fail closed",
        !ftd::eft::analyze_ternary_same_sign_capacity(1, +1).valid
        && !ftd::eft::analyze_identical_crossing(
            2, {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0},
            0.25, 0.40, 1.0, +1, rest_energy, c_speed,
            identical, identical).valid
        && !ftd::eft::analyze_elastic_scattering_counterfamily(
            c_speed, rest_energy, c_speed).valid);

  std::cout.precision(17);
  std::cout << "capacity_arms=" << capacity_arms << '\n'
            << "transformed_crossing_arms=" << transformed_crossing_arms << '\n'
            << "worst_phase_residual=" << worst_phase_residual << '\n'
            << "worst_current_residual=" << worst_current_residual << '\n'
            << "worst_conservation_residual="
            << worst_conservation_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_reversal_residual=" << worst_reversal_residual << '\n'
            << "boundary_charge_defect=" << boundary.charge_residual << '\n'
            << "elastic_output_count=" << counterfamily.output_count << '\n'
            << "minimum_direction_separation="
            << counterfamily.minimum_direction_separation << '\n'
            << "ternary_collision_vertex failures=" << failures << '\n'
            << "verdict=IDENTICAL_INTERIOR_CROSSING_IS_PERMUTATION_GAUGE\n";
  return failures == 0 ? 0 : 1;
}

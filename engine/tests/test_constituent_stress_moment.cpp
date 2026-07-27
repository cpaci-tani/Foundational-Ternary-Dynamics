/** FTD-0513: minimal constituent kinetic-stress lift. */

#include "ftd/eft/constituent_stress_moment.h"

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
int two_stream_arms = 0;
int covariance_arms = 0;
int multistream_arms = 0;
double worst_psd_residual = 0.0;
double worst_rank_one_residual = 0.0;
double worst_axis_projector_residual = 0.0;
double worst_energy_recovery_residual = 0.0;
double worst_momentum_recovery_residual = 0.0;
double worst_kinetic_recovery_residual = 0.0;
double worst_translation_residual = 0.0;
double worst_polarity_residual = 0.0;
double worst_cubic_covariance_residual = 0.0;
double minimum_stress_trace = INFINITY;
double minimum_multistream_separation = INFINITY;
double worst_multistream_conserved_residual = 0.0;
double worst_multistream_stress_residual = 0.0;
double minimum_fourth_moment_difference = INFINITY;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double tensor_difference(const ftd::eft::SymmetricTensor3& lhs,
                         const ftd::eft::SymmetricTensor3& rhs) {
  return std::max({std::abs(lhs.xx - rhs.xx),
                   std::abs(lhs.yy - rhs.yy),
                   std::abs(lhs.zz - rhs.zz),
                   std::abs(lhs.xy - rhs.xy),
                   std::abs(lhs.xz - rhs.xz),
                   std::abs(lhs.yz - rhs.yz)});
}

double lift_difference(
    const ftd::eft::TwoStreamStressLiftResult& lhs,
    const ftd::eft::TwoStreamStressLiftResult& rhs) {
  return std::max({
      tensor_difference(lhs.moment.stress, rhs.moment.stress),
      tensor_difference(lhs.recovered_axis_projector,
                        rhs.recovered_axis_projector),
      std::abs(lhs.moment.total_energy - rhs.moment.total_energy),
      std::abs(lhs.moment.kinetic_energy - rhs.moment.kinetic_energy),
      std::abs(lhs.recovered_momentum_magnitude
               - rhs.recovered_momentum_magnitude)});
}

ftd::Vec3 signed_permute(const ftd::Vec3& value,
                         const std::array<int, 3>& permutation,
                         const std::array<int, 3>& sign) {
  const std::array<double, 3> source{{value.x, value.y, value.z}};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool two_stream_ok = true;
  bool translation_ok = true;
  bool polarity_ok = true;
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (double speed : speeds) {
          std::array<ftd::eft::TwoStreamStressLiftResult, 2>
              polarity_reference{};
          for (int polarity_index = 0; polarity_index < 2;
               ++polarity_index) {
            const int polarity = polarity_index == 0 ? -1 : +1;
            ftd::eft::TwoStreamStressLiftResult reference;
            for (std::size_t t = 0; t < translations.size(); ++t) {
              const auto translation = translations[t];
              const ftd::Coord source{
                  8 + translation.x, 8 + translation.y,
                  8 + translation.z};
              const ftd::Vec3 position{
                  static_cast<double>(source.x) + 0.5 * dx,
                  static_cast<double>(source.y) + 0.5 * dy,
                  static_cast<double>(source.z) + 0.5 * dz};
              const auto result = ftd::eft::analyze_two_stream_stress_lift(
                  L, position, direction, polarity, speed,
                  rest_energy, c_speed, gate);
              two_stream_ok = two_stream_ok && result.valid
                  && result.vector_current_cancelled
                  && result.stress_retains_relative_mode
                  && result.moment.valid
                  && result.moment.psd_residual <= gate
                  && result.moment.total_momentum.mag() <= gate
                  && result.moment.stress_trace > gate
                  && result.rank_one_residual <= gate
                  && result.axis_projector_residual <= gate
                  && result.energy_recovery_residual <= gate
                  && result.momentum_recovery_residual <= gate
                  && result.kinetic_recovery_residual <= gate;
              if (t == 0) reference = result;
              else {
                worst_translation_residual = std::max(
                    worst_translation_residual,
                    lift_difference(reference, result));
              }
              if (t == 1) {
                polarity_reference[static_cast<std::size_t>(
                    polarity_index)] = result;
              }
              worst_psd_residual = std::max(
                  worst_psd_residual, result.moment.psd_residual);
              worst_rank_one_residual = std::max(
                  worst_rank_one_residual, result.rank_one_residual);
              worst_axis_projector_residual = std::max(
                  worst_axis_projector_residual,
                  result.axis_projector_residual);
              worst_energy_recovery_residual = std::max(
                  worst_energy_recovery_residual,
                  result.energy_recovery_residual);
              worst_momentum_recovery_residual = std::max(
                  worst_momentum_recovery_residual,
                  result.momentum_recovery_residual);
              worst_kinetic_recovery_residual = std::max(
                  worst_kinetic_recovery_residual,
                  result.kinetic_recovery_residual);
              minimum_stress_trace = std::min(
                  minimum_stress_trace, result.moment.stress_trace);
              ++two_stream_arms;
            }
          }
          const double polarity_residual = lift_difference(
              polarity_reference[0], polarity_reference[1]);
          worst_polarity_residual = std::max(
              worst_polarity_residual, polarity_residual);
          polarity_ok = polarity_ok && polarity_residual <= gate;
        }
      }
    }
  }
  translation_ok = worst_translation_residual <= gate;
  check("rank-2 stress reconstructs every registered two-stream mode",
        two_stream_ok && two_stream_arms == 312
        && worst_psd_residual <= gate
        && worst_rank_one_residual <= gate
        && worst_axis_projector_residual <= gate
        && worst_energy_recovery_residual <= gate
        && worst_momentum_recovery_residual <= gate
        && worst_kinetic_recovery_residual <= gate
        && minimum_stress_trace > gate);
  check("stress lift is translation and polarity independent",
        translation_ok && polarity_ok
        && worst_polarity_residual <= gate);

  const std::array<ftd::Vec3, 3> representatives{{
      {1.0, 0.0, 0.0},
      {0.70710678118654752440, 0.70710678118654752440, 0.0},
      {0.57735026918962576451, 0.57735026918962576451,
       0.57735026918962576451}}};
  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  bool cubic_ok = true;
  constexpr double covariance_momentum = 0.37;
  for (const auto& direction : representatives) {
    const std::vector<ftd::Vec3> base_momenta{{
        direction * covariance_momentum,
        direction * -covariance_momentum}};
    const auto base = ftd::eft::make_constituent_stress_moment(
        base_momenta, rest_energy, c_speed, gate);
    for (const auto& permutation : permutations) {
      for (int mask = 0; mask < 8; ++mask) {
        const std::array<int, 3> sign{{
            (mask & 1) ? -1 : +1,
            (mask & 2) ? -1 : +1,
            (mask & 4) ? -1 : +1}};
        const std::vector<ftd::Vec3> transformed_momenta{{
            signed_permute(base_momenta[0], permutation, sign),
            signed_permute(base_momenta[1], permutation, sign)}};
        const auto measured = ftd::eft::make_constituent_stress_moment(
            transformed_momenta, rest_energy, c_speed, gate);
        const auto expected = ftd::eft::transform_symmetric_tensor(
            base.stress, permutation.data(), sign.data());
        const double residual = tensor_difference(
            measured.stress, expected);
        worst_cubic_covariance_residual = std::max(
            worst_cubic_covariance_residual, residual);
        cubic_ok = cubic_ok && base.valid && measured.valid
            && residual <= gate
            && std::abs(base.stress_trace
                        - measured.stress_trace) <= gate;
        ++covariance_arms;
      }
    }
  }
  check("stress transforms exactly as R Sigma R^T under O_h",
        cubic_ok && covariance_arms == 144
        && worst_cubic_covariance_residual <= gate);

  bool multistream_ok = true;
  for (double momentum : {0.1, 0.25, 0.5}) {
    const auto result =
        ftd::eft::analyze_multistream_stress_counterexample(
            momentum, rest_energy, c_speed, gate);
    multistream_ok = multistream_ok && result.valid
        && result.momentum_multiset_separation > gate
        && result.total_momentum_residual <= gate
        && result.total_energy_residual <= gate
        && result.stress_residual <= gate
        && result.fourth_moment_difference > gate;
    minimum_multistream_separation = std::min(
        minimum_multistream_separation,
        result.momentum_multiset_separation);
    worst_multistream_conserved_residual = std::max({
        worst_multistream_conserved_residual,
        result.total_momentum_residual,
        result.total_energy_residual});
    worst_multistream_stress_residual = std::max(
        worst_multistream_stress_residual,
        result.stress_residual);
    minimum_fourth_moment_difference = std::min(
        minimum_fourth_moment_difference,
        result.fourth_moment_difference);
    ++multistream_arms;
  }
  check("rank-2 stress is not a complete general multistream state",
        multistream_ok && multistream_arms == 3
        && minimum_multistream_separation > gate
        && worst_multistream_conserved_residual <= gate
        && worst_multistream_stress_residual <= gate
        && minimum_fourth_moment_difference > gate);

  const auto invalid = ftd::eft::make_constituent_stress_moment(
      {}, rest_energy, c_speed, gate);
  check("invalid stress inputs fail closed", !invalid.valid);

  std::cout.precision(17);
  std::cout << "two_stream_arms=" << two_stream_arms << '\n'
            << "covariance_arms=" << covariance_arms << '\n'
            << "multistream_arms=" << multistream_arms << '\n'
            << "worst_psd_residual=" << worst_psd_residual << '\n'
            << "worst_rank_one_residual="
            << worst_rank_one_residual << '\n'
            << "worst_axis_projector_residual="
            << worst_axis_projector_residual << '\n'
            << "worst_energy_recovery_residual="
            << worst_energy_recovery_residual << '\n'
            << "worst_momentum_recovery_residual="
            << worst_momentum_recovery_residual << '\n'
            << "worst_kinetic_recovery_residual="
            << worst_kinetic_recovery_residual << '\n'
            << "worst_translation_residual="
            << worst_translation_residual << '\n'
            << "worst_polarity_residual="
            << worst_polarity_residual << '\n'
            << "worst_cubic_covariance_residual="
            << worst_cubic_covariance_residual << '\n'
            << "minimum_stress_trace=" << minimum_stress_trace << '\n'
            << "minimum_multistream_separation="
            << minimum_multistream_separation << '\n'
            << "worst_multistream_conserved_residual="
            << worst_multistream_conserved_residual << '\n'
            << "worst_multistream_stress_residual="
            << worst_multistream_stress_residual << '\n'
            << "minimum_fourth_moment_difference="
            << minimum_fourth_moment_difference << '\n'
            << "constituent_stress_moment failures=" << failures << '\n'
            << "verdict="
            << "RANK2_STRESS_IS_MINIMAL_FOR_TWO_STREAM_KERNEL_NOT_COMPLETE\n";
  return failures == 0 ? 0 : 1;
}

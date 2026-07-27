/** FTD-0516: selected relativistic hard-contact corner action. */

#include "ftd/eft/hard_contact_corner_action.h"

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
int collision_arms = 0;
int covariance_arms = 0;
double worst_reference_residual = 0.0;
double worst_corner_residual = 0.0;
double worst_kkt_residual = 0.0;
double worst_legendre_residual = 0.0;
double worst_face_balance_residual = 0.0;
double worst_reversal_residual = 0.0;
double worst_translation_residual = 0.0;
double worst_polarity_residual = 0.0;
double worst_cubic_covariance_residual = 0.0;
double minimum_multiplier = INFINITY;
double minimum_outgoing_gap_rate = INFINITY;
double maximum_incoming_gap_rate = -INFINITY;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double vector_difference(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return max_abs(lhs - rhs);
}

double invariant_difference(
    const ftd::eft::HardContactCornerActionResult& lhs,
    const ftd::eft::HardContactCornerActionResult& rhs) {
  return std::max({
      vector_difference(lhs.momentum_first_before,
                        rhs.momentum_first_before),
      vector_difference(lhs.momentum_second_before,
                        rhs.momentum_second_before),
      vector_difference(lhs.momentum_first_after,
                        rhs.momentum_first_after),
      vector_difference(lhs.momentum_second_after,
                        rhs.momentum_second_after),
      std::abs(lhs.impulse_multiplier-rhs.impulse_multiplier),
      std::abs(lhs.incoming_gap_rate-rhs.incoming_gap_rate),
      std::abs(lhs.outgoing_gap_rate-rhs.outgoing_gap_rate)});
}

ftd::Vec3 signed_permute(const ftd::Vec3& value,
                         const std::array<int, 3>& permutation,
                         const std::array<int, 3>& sign) {
  const std::array<double, 3> source{{value.x, value.y, value.z}};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

ftd::Coord signed_permute(ftd::Coord value,
                          const std::array<int, 3>& permutation,
                          const std::array<int, 3>& sign) {
  const std::array<int, 3> source{{value.x, value.y, value.z}};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

double covariance_difference(
    const ftd::eft::HardContactCornerActionResult& base,
    const ftd::eft::HardContactCornerActionResult& measured,
    const std::array<int, 3>& permutation,
    const std::array<int, 3>& sign) {
  return std::max({
      vector_difference(signed_permute(base.normal, permutation, sign),
                        measured.normal),
      vector_difference(signed_permute(base.momentum_first_before,
                                       permutation, sign),
                        measured.momentum_first_before),
      vector_difference(signed_permute(base.momentum_second_before,
                                       permutation, sign),
                        measured.momentum_second_before),
      vector_difference(signed_permute(base.momentum_first_after,
                                       permutation, sign),
                        measured.momentum_first_after),
      vector_difference(signed_permute(base.momentum_second_after,
                                       permutation, sign),
                        measured.momentum_second_after),
      vector_difference(signed_permute(base.velocity_first_before,
                                       permutation, sign),
                        measured.velocity_first_before),
      vector_difference(signed_permute(base.velocity_first_after,
                                       permutation, sign),
                        measured.velocity_first_after),
      std::abs(base.impulse_multiplier-measured.impulse_multiplier),
      std::abs(base.incoming_gap_rate-measured.incoming_gap_rate),
      std::abs(base.outgoing_gap_rate-measured.outgoing_gap_rate)});
}

ftd::Vec3 collision_position(ftd::Coord direction,
                             ftd::Coord translation = {}) {
  return {8.0 + translation.x + 0.5 * direction.x,
          8.0 + translation.y + 0.5 * direction.y,
          8.0 + translation.z + 0.5 * direction.z};
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool reference_ok = true;
  bool corner_ok = true;
  bool kkt_ok = true;
  bool face_ok = true;
  bool reversal_ok = true;
  bool covariance_ok = true;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (double speed : speeds) {
          std::array<ftd::eft::HardContactCornerActionResult, 2>
              polarity_reference{};
          for (int polarity_index = 0; polarity_index < 2;
               ++polarity_index) {
            const int polarity = polarity_index == 0 ? -1 : +1;
            ftd::eft::HardContactCornerActionResult translation_reference;
            for (std::size_t t = 0; t < translations.size(); ++t) {
              const auto result =
                  ftd::eft::analyze_hard_contact_corner_action(
                      L, collision_position(direction, translations[t]),
                      direction, polarity, speed, rest_energy,
                      c_speed, 0.25, gate);
              reference_ok = reference_ok && result.valid
                  && result.reference_collision.valid
                  && result.reference_collision_residual <= gate
                  && result.multiplier_match_residual <= gate;
              corner_ok = corner_ok
                  && result.normal_impulse_residual <= gate
                  && result.tangential_corner_residual <= gate
                  && result.common_corner_gradient_residual <= gate
                  && result.collision_time_gradient_residual <= gate
                  && result.total_momentum_residual <= gate
                  && result.total_energy_residual <= gate
                  && result.action_density_residual <= gate
                  && result.legendre_residual <= gate;
              kkt_ok = kkt_ok
                  && result.impulse_multiplier > gate
                  && result.inactive_control_multiplier == 0.0
                  && result.incoming_gap_rate < -gate
                  && result.outgoing_gap_rate > gate
                  && result.branch_polynomial_residual <= gate
                  && result.nontrivial_branch_residual <= gate
                  && result.kkt_dual_residual <= gate
                  && result.complementarity_residual <= gate
                  && result.incoming_gate_residual <= gate
                  && result.outgoing_gate_residual <= gate;
              face_ok = face_ok && result.face_balance.valid
                  && result.face_balance_residual <= gate;
              reversal_ok = reversal_ok
                  && result.reversal_residual <= gate
                  && result.reversal_multiplier_residual <= gate;
              if (t == 0) translation_reference = result;
              else {
                worst_translation_residual = std::max(
                    worst_translation_residual,
                    invariant_difference(translation_reference, result));
              }
              if (t == 1) {
                polarity_reference[static_cast<std::size_t>(
                    polarity_index)] = result;
              }
              worst_reference_residual = std::max({
                  worst_reference_residual,
                  result.reference_collision_residual,
                  result.multiplier_match_residual});
              worst_corner_residual = std::max({
                  worst_corner_residual,
                  result.normal_impulse_residual,
                  result.tangential_corner_residual,
                  result.common_corner_gradient_residual,
                  result.collision_time_gradient_residual,
                  result.total_momentum_residual,
                  result.total_energy_residual,
                  result.action_density_residual});
              worst_kkt_residual = std::max({
                  worst_kkt_residual,
                  result.branch_polynomial_residual,
                  result.nontrivial_branch_residual,
                  result.kkt_dual_residual,
                  result.complementarity_residual,
                  result.incoming_gate_residual,
                  result.outgoing_gate_residual,
                  std::abs(result.inactive_control_multiplier)});
              worst_legendre_residual = std::max(
                  worst_legendre_residual, result.legendre_residual);
              worst_face_balance_residual = std::max(
                  worst_face_balance_residual,
                  result.face_balance_residual);
              worst_reversal_residual = std::max({
                  worst_reversal_residual,
                  result.reversal_residual,
                  result.reversal_multiplier_residual});
              minimum_multiplier = std::min(
                  minimum_multiplier, result.impulse_multiplier);
              minimum_outgoing_gap_rate = std::min(
                  minimum_outgoing_gap_rate, result.outgoing_gap_rate);
              maximum_incoming_gap_rate = std::max(
                  maximum_incoming_gap_rate, result.incoming_gap_rate);
              ++collision_arms;
            }
          }
          worst_polarity_residual = std::max(
              worst_polarity_residual,
              invariant_difference(polarity_reference[0],
                                   polarity_reference[1]));
        }
      }
    }
  }

  const std::array<ftd::Coord, 3> representatives{{
      {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}};
  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  for (const auto& base_direction : representatives) {
    const auto base = ftd::eft::analyze_hard_contact_corner_action(
        L, collision_position(base_direction), base_direction, +1,
        0.25, rest_energy, c_speed, 0.25, gate);
    for (const auto& permutation : permutations) {
      for (int mask = 0; mask < 8; ++mask) {
        const std::array<int, 3> sign{{
            (mask & 1) ? -1 : +1,
            (mask & 2) ? -1 : +1,
            (mask & 4) ? -1 : +1}};
        const auto direction = signed_permute(
            base_direction, permutation, sign);
        const auto measured =
            ftd::eft::analyze_hard_contact_corner_action(
                L, collision_position(direction), direction, +1,
                0.25, rest_energy, c_speed, 0.25, gate);
        const double residual = covariance_difference(
            base, measured, permutation, sign);
        worst_cubic_covariance_residual = std::max(
            worst_cubic_covariance_residual, residual);
        covariance_ok = covariance_ok && base.valid && measured.valid
            && residual <= gate;
        ++covariance_arms;
      }
    }
  }

  check("corner action independently reproduces FTD-0512 reflection",
        reference_ok && collision_arms == 312
        && worst_reference_residual <= gate);
  check("corner variations give normal impulse, momentum, and energy laws",
        corner_ok && worst_corner_residual <= gate
        && worst_legendre_residual <= gate);
  check("KKT and outgoing admissibility select the nontrivial branch",
        kkt_ok && worst_kkt_residual <= gate
        && minimum_multiplier > gate
        && maximum_incoming_gap_rate < -gate
        && minimum_outgoing_gap_rate > gate);
  check("selected corner composes with exact FTD-0514 face balance",
        face_ok && worst_face_balance_residual <= gate);
  check("translation, polarity, O_h covariance, and reversal close",
        covariance_ok && covariance_arms == 144
        && worst_translation_residual <= gate
        && worst_polarity_residual <= gate
        && worst_cubic_covariance_residual <= gate
        && reversal_ok && worst_reversal_residual <= gate);

  const auto invalid = ftd::eft::analyze_hard_contact_corner_action(
      2, {}, {}, +1, 0.25, rest_energy, c_speed, 0.25, gate);
  const double penetrating = ftd::eft::selected_hard_contact_multiplier(
      -0.25, 1.0, gate);
  check("invalid and penetrating inputs fail closed",
        !invalid.valid && std::isnan(penetrating));

  std::cout.precision(17);
  std::cout << "collision_arms=" << collision_arms << '\n'
            << "covariance_arms=" << covariance_arms << '\n'
            << "worst_reference_residual="
            << worst_reference_residual << '\n'
            << "worst_corner_residual=" << worst_corner_residual << '\n'
            << "worst_kkt_residual=" << worst_kkt_residual << '\n'
            << "worst_legendre_residual="
            << worst_legendre_residual << '\n'
            << "worst_face_balance_residual="
            << worst_face_balance_residual << '\n'
            << "worst_reversal_residual="
            << worst_reversal_residual << '\n'
            << "worst_translation_residual="
            << worst_translation_residual << '\n'
            << "worst_polarity_residual="
            << worst_polarity_residual << '\n'
            << "worst_cubic_covariance_residual="
            << worst_cubic_covariance_residual << '\n'
            << "minimum_multiplier=" << minimum_multiplier << '\n'
            << "maximum_incoming_gap_rate="
            << maximum_incoming_gap_rate << '\n'
            << "minimum_outgoing_gap_rate="
            << minimum_outgoing_gap_rate << '\n'
            << "hard_contact_corner_action failures=" << failures << '\n'
            << "verdict="
            << "SELECTED_HARD_CONTACT_ACTION_DERIVES_RESTRICTED_IMPULSE_NO_FIELD_ORIGIN\n";
  return failures == 0 ? 0 : 1;
}

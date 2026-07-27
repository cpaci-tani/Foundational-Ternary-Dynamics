/** FTD-0512: constituent-relative selected collision and face-kernel audit. */

#include "ftd/eft/constituent_relative_collision.h"

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
int arms = 0;
int face_arms = 0;
int edge_arms = 0;
int corner_arms = 0;
double worst_chart_residual = 0.0;
double worst_conservation_residual = 0.0;
double worst_collision_solution_residual = 0.0;
double worst_reversal_residual = 0.0;
double worst_causal_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_translation_residual = 0.0;
double worst_polarity_mirror_residual = 0.0;
double minimum_face_constituent_current_l1 = INFINITY;
double maximum_face_aggregate_current_l1 = 0.0;
double maximum_face_signature_residual = 0.0;
double minimum_face_kinetic_energy_gap = INFINITY;
std::array<double, 3> minimum_aggregate_l1{{
    INFINITY, INFINITY, INFINITY}};
std::array<double, 3> maximum_aggregate_l1{{0.0, 0.0, 0.0}};

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double vector_difference(const std::vector<double>& lhs,
                         const std::vector<double>& rhs,
                         double sign = 1.0) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i] - sign * rhs[i]));
  return result;
}

double signature_polarity_mirror(
    const ftd::eft::PiecewiseCurrentSignature& negative,
    const ftd::eft::PiecewiseCurrentSignature& positive) {
  if (!negative.valid || !positive.valid) return INFINITY;
  return std::max({
      vector_difference(negative.rho_before, positive.rho_before, -1.0),
      vector_difference(negative.rho_after, positive.rho_after, -1.0),
      vector_difference(negative.current_x, positive.current_x, -1.0),
      vector_difference(negative.current_y, positive.current_y, -1.0),
      vector_difference(negative.current_z, positive.current_z, -1.0)});
}

double translation_residual(
    const ftd::eft::ConstituentRelativeCollisionResult& lhs,
    const ftd::eft::ConstituentRelativeCollisionResult& rhs) {
  return std::max({
      max_abs(lhs.chart_normal - rhs.chart_normal),
      max_abs(lhs.momentum_first_before - rhs.momentum_first_before),
      max_abs(lhs.momentum_second_before - rhs.momentum_second_before),
      max_abs(lhs.momentum_first_after - rhs.momentum_first_after),
      max_abs(lhs.momentum_second_after - rhs.momentum_second_after),
      max_abs(lhs.impulse_first - rhs.impulse_first),
      max_abs(lhs.impulse_second - rhs.impulse_second),
      std::abs(lhs.aggregate_current_l1 - rhs.aggregate_current_l1),
      std::abs(lhs.constituent_current_l1 - rhs.constituent_current_l1),
      std::abs(lhs.matter_kinetic_energy_gap
               - rhs.matter_kinetic_energy_gap)});
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool map_ok = true;
  bool covariance_ok = true;
  bool face_kernel_ok = true;
  bool nonface_report_ok = true;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        const int shell = (dx != 0) + (dy != 0) + (dz != 0);
        const double magnitude = std::sqrt(
            static_cast<double>(dx * dx + dy * dy + dz * dz));
        const ftd::Vec3 expected_normal{
            dx / magnitude, dy / magnitude, dz / magnitude};
        for (double speed : speeds) {
          ftd::eft::ConstituentRelativeCollisionResult negative;
          ftd::eft::ConstituentRelativeCollisionResult positive;
          for (int polarity : {-1, +1}) {
            ftd::eft::ConstituentRelativeCollisionResult reference;
            for (std::size_t t = 0; t < translations.size(); ++t) {
              const auto translation = translations[t];
              const ftd::Coord source{
                  8 + translation.x, 8 + translation.y,
                  8 + translation.z};
              const ftd::Vec3 position{
                  static_cast<double>(source.x) + 0.5 * dx,
                  static_cast<double>(source.y) + 0.5 * dy,
                  static_cast<double>(source.z) + 0.5 * dz};
              const auto result =
                  ftd::eft::analyze_constituent_relative_collision(
                      L, position, direction, polarity, speed,
                      rest_energy, c_speed, 0.25, gate);
              map_ok = map_ok && result.valid
                  && result.selected_central_contact
                  && result.chart_position_residual <= gate
                  && result.incoming_normal_momentum > gate
                  && result.outgoing_normal_momentum < -gate
                  && result.normal_com_momentum_residual <= gate
                  && result.impulse_sum_residual <= gate
                  && result.total_momentum_residual <= gate
                  && result.matter_energy_residual <= gate
                  && result.central_impulse_residual <= gate
                  && result.impulse_solution_residual <= gate
                  && result.tangential_relative_residual <= gate
                  && result.outgoing_condition_residual <= gate
                  && result.involution_residual <= gate
                  && result.time_reversal_residual <= gate
                  && result.causal_residual <= gate
                  && result.continuity_residual <= gate;
              covariance_ok = covariance_ok
                  && max_abs(result.chart_normal - expected_normal) <= gate
                  && max_abs(result.momentum_first_after
                             + result.momentum_first_before) <= gate
                  && max_abs(result.momentum_second_after
                             + result.momentum_second_before) <= gate;
              if (t == 0) reference = result;
              else worst_translation_residual = std::max(
                  worst_translation_residual,
                  translation_residual(reference, result));
              if (polarity < 0 && t == 1) negative = result;
              if (polarity > 0 && t == 1) positive = result;

              worst_chart_residual = std::max(
                  worst_chart_residual, result.chart_position_residual);
              worst_conservation_residual = std::max({
                  worst_conservation_residual,
                  result.normal_com_momentum_residual,
                  result.impulse_sum_residual,
                  result.total_momentum_residual,
                  result.matter_energy_residual});
              worst_collision_solution_residual = std::max({
                  worst_collision_solution_residual,
                  result.central_impulse_residual,
                  result.impulse_solution_residual,
                  result.tangential_relative_residual,
                  result.outgoing_condition_residual});
              worst_reversal_residual = std::max({
                  worst_reversal_residual,
                  result.involution_residual,
                  result.time_reversal_residual});
              worst_causal_residual = std::max(
                  worst_causal_residual, result.causal_residual);
              worst_continuity_residual = std::max(
                  worst_continuity_residual,
                  result.continuity_residual);
              minimum_aggregate_l1[static_cast<std::size_t>(shell - 1)] =
                  std::min(minimum_aggregate_l1[
                               static_cast<std::size_t>(shell - 1)],
                           result.aggregate_current_l1);
              maximum_aggregate_l1[static_cast<std::size_t>(shell - 1)] =
                  std::max(maximum_aggregate_l1[
                               static_cast<std::size_t>(shell - 1)],
                           result.aggregate_current_l1);
              if (shell == 1) {
                ++face_arms;
                face_kernel_ok = face_kernel_ok
                    && result.aggregate_face_kernel;
                minimum_face_constituent_current_l1 = std::min(
                    minimum_face_constituent_current_l1,
                    result.constituent_current_l1);
                maximum_face_aggregate_current_l1 = std::max(
                    maximum_face_aggregate_current_l1,
                    result.aggregate_current_l1);
                maximum_face_signature_residual = std::max(
                    maximum_face_signature_residual,
                    result.aggregate_static_separating_residual);
                minimum_face_kinetic_energy_gap = std::min(
                    minimum_face_kinetic_energy_gap,
                    result.matter_kinetic_energy_gap);
              } else if (shell == 2) {
                ++edge_arms;
                nonface_report_ok = nonface_report_ok
                    && !result.aggregate_face_kernel;
              } else {
                ++corner_arms;
                nonface_report_ok = nonface_report_ok
                    && !result.aggregate_face_kernel;
              }
              ++arms;
            }
          }

          const ftd::Coord source{8, 8, 8};
          const ftd::Vec3 position{
              static_cast<double>(source.x) + 0.5 * dx,
              static_cast<double>(source.y) + 0.5 * dy,
              static_cast<double>(source.z) + 0.5 * dz};
          negative = ftd::eft::analyze_constituent_relative_collision(
              L, position, direction, -1, speed,
              rest_energy, c_speed, 0.25, gate);
          positive = ftd::eft::analyze_constituent_relative_collision(
              L, position, direction, +1, speed,
              rest_energy, c_speed, 0.25, gate);
          worst_polarity_mirror_residual = std::max(
              worst_polarity_mirror_residual,
              signature_polarity_mirror(
                  negative.aggregate_separating,
                  positive.aggregate_separating));
        }
      }
    }
  }

  check("selected central reflection closes every registered arm",
        map_ok && arms == 312
        && worst_chart_residual <= gate
        && worst_conservation_residual <= gate
        && worst_collision_solution_residual <= gate
        && worst_reversal_residual <= gate
        && worst_causal_residual <= gate
        && worst_continuity_residual <= gate);
  check("integer translations, signed cubic directions, and polarity mirror",
        covariance_ok && worst_translation_residual <= gate
        && worst_polarity_mirror_residual <= gate);
  check("all face-normal relative modes lie in the aggregate projection kernel",
        face_kernel_ok && face_arms == 72
        && maximum_face_signature_residual <= gate
        && maximum_face_aggregate_current_l1 <= gate
        && minimum_face_constituent_current_l1 > gate
        && minimum_face_kinetic_energy_gap > 1e-6);
  check("edge and corner tensor-product responses remain distinguished",
        nonface_report_ok && edge_arms == 144 && corner_arms == 96
        && minimum_aggregate_l1[1] > gate
        && minimum_aggregate_l1[2] > gate);

  const auto invalid = ftd::eft::analyze_constituent_relative_collision(
      2, {}, {}, 0, 0.0, rest_energy, c_speed);
  check("invalid collision inputs fail closed", !invalid.valid);

  std::cout.precision(17);
  std::cout << "registered_arms=" << arms << '\n'
            << "face_arms=" << face_arms << '\n'
            << "edge_arms=" << edge_arms << '\n'
            << "corner_arms=" << corner_arms << '\n'
            << "worst_chart_residual=" << worst_chart_residual << '\n'
            << "worst_conservation_residual="
            << worst_conservation_residual << '\n'
            << "worst_collision_solution_residual="
            << worst_collision_solution_residual << '\n'
            << "worst_reversal_residual="
            << worst_reversal_residual << '\n'
            << "worst_causal_residual=" << worst_causal_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_translation_residual="
            << worst_translation_residual << '\n'
            << "worst_polarity_mirror_residual="
            << worst_polarity_mirror_residual << '\n'
            << "minimum_face_constituent_current_l1="
            << minimum_face_constituent_current_l1 << '\n'
            << "maximum_face_aggregate_current_l1="
            << maximum_face_aggregate_current_l1 << '\n'
            << "maximum_face_signature_residual="
            << maximum_face_signature_residual << '\n'
            << "minimum_face_kinetic_energy_gap="
            << minimum_face_kinetic_energy_gap << '\n'
            << "edge_current_l1_min=" << minimum_aggregate_l1[1] << '\n'
            << "edge_current_l1_max=" << maximum_aggregate_l1[1] << '\n'
            << "corner_current_l1_min=" << minimum_aggregate_l1[2] << '\n'
            << "corner_current_l1_max=" << maximum_aggregate_l1[2] << '\n'
            << "constituent_relative_collision failures="
            << failures << '\n'
            << "verdict="
            << "SELECTED_REFLECTION_EXISTS_FACE_ACTION_CANNOT_DERIVE_IT\n";
  return failures == 0 ? 0 : 1;
}

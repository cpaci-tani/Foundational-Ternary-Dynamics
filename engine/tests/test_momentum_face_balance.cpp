/** FTD-0514: exact local momentum balance from oriented face current. */

#include "ftd/eft/momentum_face_balance.h"

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
int free_arms = 0;
int collision_arms = 0;
double worst_free_local_residual = 0.0;
double worst_free_global_residual = 0.0;
double worst_free_first_moment_residual = 0.0;
double worst_free_stress_bridge_residual = 0.0;
double worst_free_causal_residual = 0.0;
double worst_collision_segment_residual = 0.0;
double worst_constituent_impulse_residual = 0.0;
double worst_aggregate_impulse_l1 = 0.0;
double minimum_individual_impulse_l1 = INFINITY;
double worst_collision_local_residual = 0.0;
double worst_collision_global_residual = 0.0;
double worst_collision_energy_residual = 0.0;
double worst_collision_tensor_moment_residual = 0.0;
double worst_reversal_endpoint_residual = 0.0;
double worst_reversal_flux_residual = 0.0;
double worst_reversal_source_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double momentum_magnitude(double speed) {
  const double beta_squared = speed * speed / (c_speed * c_speed);
  return rest_energy * speed
      / (c_speed * c_speed * std::sqrt(1.0 - beta_squared));
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool free_ok = true;
  bool collision_ok = true;
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        const double norm = std::sqrt(
            static_cast<double>(dx * dx + dy * dy + dz * dz));
        const ftd::Vec3 unit{dx / norm, dy / norm, dz / norm};
        for (const auto& translation : translations) {
          const ftd::Vec3 free_start{
              8.25 + translation.x,
              8.25 + translation.y,
              8.25 + translation.z};
          const ftd::Coord source{
              8 + translation.x, 8 + translation.y,
              8 + translation.z};
          const ftd::Vec3 collision_position{
              static_cast<double>(source.x) + 0.5 * dx,
              static_cast<double>(source.y) + 0.5 * dy,
              static_cast<double>(source.z) + 0.5 * dz};
          for (int polarity : {-1, +1}) {
            (void)polarity;
            for (double speed : speeds) {
              const ftd::Vec3 momentum = unit * momentum_magnitude(speed);
              const auto free =
                  ftd::eft::analyze_free_momentum_transport_balance(
                      L, free_start, momentum, rest_energy,
                      c_speed, 1.0, gate);
              free_ok = free_ok && free.valid
                  && free.worldline.valid
                  && free.worldline.local_balance_residual <= gate
                  && free.worldline.global_momentum_residual <= gate
                  && free.worldline.face_first_moment_residual <= gate
                  && free.stress_bridge_residual <= gate
                  && free.causal_residual <= gate;
              worst_free_local_residual = std::max(
                  worst_free_local_residual,
                  free.worldline.local_balance_residual);
              worst_free_global_residual = std::max(
                  worst_free_global_residual,
                  free.worldline.global_momentum_residual);
              worst_free_first_moment_residual = std::max(
                  worst_free_first_moment_residual,
                  free.worldline.face_first_moment_residual);
              worst_free_stress_bridge_residual = std::max(
                  worst_free_stress_bridge_residual,
                  free.stress_bridge_residual);
              worst_free_causal_residual = std::max(
                  worst_free_causal_residual,
                  free.causal_residual);
              ++free_arms;

              const auto collision =
                  ftd::eft::analyze_collision_momentum_face_balance(
                      L, collision_position, direction, polarity,
                      speed, rest_energy, c_speed, 0.25, gate);
              collision_ok = collision_ok && collision.valid
                  && collision.individual_segment_residual <= gate
                  && collision.constituent_impulse_residual <= gate
                  && collision.aggregate_impulse_source_l1 <= gate
                  && collision.individual_impulse_source_l1 > gate
                  && collision.aggregate_local_balance_residual <= gate
                  && collision.aggregate_global_momentum_residual <= gate
                  && collision.energy_residual <= gate
                  && collision.tensor_moment_residual <= gate
                  && collision.reversal_endpoint_residual <= gate
                  && collision.reversal_tensor_flux_residual <= gate
                  && collision.reversal_impulse_source_residual <= gate;
              worst_collision_segment_residual = std::max(
                  worst_collision_segment_residual,
                  collision.individual_segment_residual);
              worst_constituent_impulse_residual = std::max(
                  worst_constituent_impulse_residual,
                  collision.constituent_impulse_residual);
              worst_aggregate_impulse_l1 = std::max(
                  worst_aggregate_impulse_l1,
                  collision.aggregate_impulse_source_l1);
              minimum_individual_impulse_l1 = std::min(
                  minimum_individual_impulse_l1,
                  collision.individual_impulse_source_l1);
              worst_collision_local_residual = std::max(
                  worst_collision_local_residual,
                  collision.aggregate_local_balance_residual);
              worst_collision_global_residual = std::max(
                  worst_collision_global_residual,
                  collision.aggregate_global_momentum_residual);
              worst_collision_energy_residual = std::max(
                  worst_collision_energy_residual,
                  collision.energy_residual);
              worst_collision_tensor_moment_residual = std::max(
                  worst_collision_tensor_moment_residual,
                  collision.tensor_moment_residual);
              worst_reversal_endpoint_residual = std::max(
                  worst_reversal_endpoint_residual,
                  collision.reversal_endpoint_residual);
              worst_reversal_flux_residual = std::max(
                  worst_reversal_flux_residual,
                  collision.reversal_tensor_flux_residual);
              worst_reversal_source_residual = std::max(
                  worst_reversal_source_residual,
                  collision.reversal_impulse_source_residual);
              ++collision_arms;
            }
          }
        }
      }
    }
  }

  check("exact scalar continuity lifts to free local momentum balance",
        free_ok && free_arms == 312
        && worst_free_local_residual <= gate
        && worst_free_global_residual <= gate
        && worst_free_first_moment_residual <= gate);
  check("integrated tensor face flux equals dt times kinetic stress",
        worst_free_stress_bridge_residual <= gate
        && worst_free_causal_residual <= gate);
  check("selected contact closes constituent and aggregate momentum balance",
        collision_ok && collision_arms == 312
        && worst_collision_segment_residual <= gate
        && worst_constituent_impulse_residual <= gate
        && worst_aggregate_impulse_l1 <= gate
        && minimum_individual_impulse_l1 > gate
        && worst_collision_local_residual <= gate
        && worst_collision_global_residual <= gate
        && worst_collision_energy_residual <= gate
        && worst_collision_tensor_moment_residual <= gate);
  check("momentum density is odd while tensor flux and impulse are T-even",
        worst_reversal_endpoint_residual <= gate
        && worst_reversal_flux_residual <= gate
        && worst_reversal_source_residual <= gate);

  const auto invalid = ftd::eft::make_momentum_worldline_balance(
      2, {}, {}, {}, gate);
  check("invalid momentum-balance inputs fail closed", !invalid.valid);

  std::cout.precision(17);
  std::cout << "free_arms=" << free_arms << '\n'
            << "collision_arms=" << collision_arms << '\n'
            << "worst_free_local_residual="
            << worst_free_local_residual << '\n'
            << "worst_free_global_residual="
            << worst_free_global_residual << '\n'
            << "worst_free_first_moment_residual="
            << worst_free_first_moment_residual << '\n'
            << "worst_free_stress_bridge_residual="
            << worst_free_stress_bridge_residual << '\n'
            << "worst_free_causal_residual="
            << worst_free_causal_residual << '\n'
            << "worst_collision_segment_residual="
            << worst_collision_segment_residual << '\n'
            << "worst_constituent_impulse_residual="
            << worst_constituent_impulse_residual << '\n'
            << "worst_aggregate_impulse_l1="
            << worst_aggregate_impulse_l1 << '\n'
            << "minimum_individual_impulse_l1="
            << minimum_individual_impulse_l1 << '\n'
            << "worst_collision_local_residual="
            << worst_collision_local_residual << '\n'
            << "worst_collision_global_residual="
            << worst_collision_global_residual << '\n'
            << "worst_collision_energy_residual="
            << worst_collision_energy_residual << '\n'
            << "worst_collision_tensor_moment_residual="
            << worst_collision_tensor_moment_residual << '\n'
            << "worst_reversal_endpoint_residual="
            << worst_reversal_endpoint_residual << '\n'
            << "worst_reversal_flux_residual="
            << worst_reversal_flux_residual << '\n'
            << "worst_reversal_source_residual="
            << worst_reversal_source_residual << '\n'
            << "momentum_face_balance failures=" << failures << '\n'
            << "verdict="
            << "EXACT_MOMENTUM_FACE_BALANCE_CLOSES_SELECTED_CONTACT_COMPATIBILITY_ONLY\n";
  return failures == 0 ? 0 : 1;
}

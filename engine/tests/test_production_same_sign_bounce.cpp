/** FTD-0506: production same-sign bounce reciprocity audit. */

#include "ftd/eft/production_same_sign_bounce.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int production_arms = 0;
double worst_documented_output_residual = 0.0;
double minimum_specular_remainder_residual = INFINITY;
double minimum_production_causal_residual = INFINITY;
double worst_specular_causal_residual = 0.0;
double worst_energy_residual = 0.0;
double minimum_pair_momentum_defect = INFINITY;
double worst_field_state_change_residual = 0.0;
double minimum_current_difference = INFINITY;
double worst_current_continuity_residual = 0.0;
double minimum_missing_journal_current_residual = INFINITY;
double minimum_inverse_phase_space_residual = INFINITY;
int maximum_journal_events = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double global_field_magnitude(ftd::RenderBridge& bridge) {
  double result = 0.0;
  for (int i = 0; i < static_cast<int>(bridge.lattice().total_sites()); ++i) {
    const auto site = bridge.lattice().coord(i);
    const auto& voxel = bridge.voxel_at(site.x, site.y, site.z);
    result = std::max({result, voxel.flux.mag(), voxel.wave_vel.mag(),
        voxel.flux_L.mag(), voxel.flux_R.mag(),
        voxel.wave_vel_L.mag(), voxel.wave_vel_R.mag(),
        voxel.flux_strong.mag(), voxel.wave_vel_strong.mag(),
        voxel.flux_weak.mag(), voxel.wave_vel_weak.mag()});
  }
  return result;
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  bool measured_output_ok = true;
  bool specular_negative = true;
  bool momentum_negative = true;
  bool current_negative = true;
  bool inverse_negative = true;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord hop{dx, dy, dz};
        for (const auto& translation : translations) {
          const ftd::Coord source{
              8 + translation.x, 8 + translation.y, 8 + translation.z};
          const ftd::Coord target{
              source.x + dx, source.y + dy, source.z + dz};
          for (int polarity : {-1, +1}) {
            ftd::RenderBridge bridge(L);
            bridge.force_cpu();
            bridge.toggles.disable_all();
            bridge.toggles.movement = true;
            bridge.set_dt(1.0);
            measured_output_ok = measured_output_ok
                && bridge.enable_history_journal(true);
            const int source_index = bridge.lattice().index(
                source.x, source.y, source.z);
            const int target_index = bridge.lattice().index(
                target.x, target.y, target.z);
            bridge.set_state(source_index, static_cast<int8_t>(polarity));
            bridge.set_state(target_index, static_cast<int8_t>(polarity));
            auto& mover = bridge.voxel_at(source.x, source.y, source.z);
            auto& obstacle = bridge.voxel_at(target.x, target.y, target.z);
            mover.remainder = {0.80 * dx, 0.80 * dy, 0.80 * dz};
            mover.velocity = {0.25 * dx, 0.25 * dy, 0.25 * dz};
            mover.particle_id = 101;
            mover.pair_id = 201;
            mover.spin = +1;
            mover.color = 1;
            mover.flavor = 1;
            obstacle.particle_id = 102;
            obstacle.pair_id = 202;
            obstacle.spin = -1;
            obstacle.color = 2;
            obstacle.flavor = 2;

            const ftd::Voxel source_before = mover;
            const ftd::Voxel target_before = obstacle;
            bridge.tick();
            const ftd::Voxel source_after = bridge.voxel_at(
                source.x, source.y, source.z);
            const ftd::Voxel target_after = bridge.voxel_at(
                target.x, target.y, target.z);
            const int journal_after_first = static_cast<int>(
                bridge.history_events().size());
            const double field_after_first = global_field_magnitude(bridge);
            bridge.tick();
            const ftd::Voxel source_after_second = bridge.voxel_at(
                source.x, source.y, source.z);
            const ftd::Voxel target_after_second = bridge.voxel_at(
                target.x, target.y, target.z);

            const auto result =
                ftd::eft::analyze_production_same_sign_bounce(
                    L, source, hop,
                    source_before, target_before,
                    source_after, target_after,
                    source_after_second, target_after_second,
                    journal_after_first, 1.0, gate);
            measured_output_ok = measured_output_ok && result.valid
                && result.manifestation_residual <= gate
                && result.source_velocity_reflection_residual <= gate
                && result.source_remainder_reset_residual <= gate
                && result.target_unchanged_residual <= gate
                && result.journal_event_count == 0;
            worst_documented_output_residual = std::max({
                worst_documented_output_residual,
                result.manifestation_residual,
                result.source_velocity_reflection_residual,
                result.source_remainder_reset_residual,
                result.target_unchanged_residual});
            maximum_journal_events = std::max(
                maximum_journal_events, result.journal_event_count);

            specular_negative = specular_negative
                && result.specular_remainder_residual > gate
                && result.production_effective_causal_residual > gate
                && result.specular_arc_causal_residual <= gate;
            minimum_specular_remainder_residual = std::min(
                minimum_specular_remainder_residual,
                result.specular_remainder_residual);
            minimum_production_causal_residual = std::min(
                minimum_production_causal_residual,
                result.production_effective_causal_residual);
            worst_specular_causal_residual = std::max(
                worst_specular_causal_residual,
                result.specular_arc_causal_residual);

            momentum_negative = momentum_negative
                && result.pair_energy_residual <= gate
                && result.pair_momentum_defect > gate
                && result.field_state_change_residual <= gate
                && field_after_first <= gate;
            worst_energy_residual = std::max(
                worst_energy_residual, result.pair_energy_residual);
            minimum_pair_momentum_defect = std::min(
                minimum_pair_momentum_defect,
                result.pair_momentum_defect);
            worst_field_state_change_residual = std::max({
                worst_field_state_change_residual,
                result.field_state_change_residual,
                field_after_first});

            current_negative = current_negative
                && result.exact_current_difference > gate
                && result.exact_current_continuity_residual <= gate
                && result.missing_journal_current_residual > gate;
            minimum_current_difference = std::min(
                minimum_current_difference,
                result.exact_current_difference);
            worst_current_continuity_residual = std::max(
                worst_current_continuity_residual,
                result.exact_current_continuity_residual);
            minimum_missing_journal_current_residual = std::min(
                minimum_missing_journal_current_residual,
                result.missing_journal_current_residual);

            inverse_negative = inverse_negative
                && result.inverse_phase_space_residual > gate;
            minimum_inverse_phase_space_residual = std::min(
                minimum_inverse_phase_space_residual,
                result.inverse_phase_space_residual);
            ++production_arms;
          }
        }
      }
    }
  }

  check("production axis-flip/reset behavior is measured in every arm",
        measured_output_ok && production_arms == 156
        && worst_documented_output_residual <= gate
        && maximum_journal_events == 0);
  check("production remainder reset is not the causal specular endpoint",
        specular_negative
        && minimum_specular_remainder_residual > gate
        && minimum_production_causal_residual > gate
        && worst_specular_causal_residual <= gate);
  check("production bounce preserves energy but not pair+field momentum",
        momentum_negative && worst_energy_residual <= gate
        && minimum_pair_momentum_defect > gate
        && worst_field_state_change_residual <= gate);
  check("site-level no-op omits the exact subcell collision current",
        current_negative && minimum_current_difference > gate
        && worst_current_continuity_residual <= gate
        && minimum_missing_journal_current_residual > gate);
  check("an unchanged second tick does not invert the bounce",
        inverse_negative && minimum_inverse_phase_space_residual > gate);

  std::cout.precision(17);
  std::cout << "production_arms=" << production_arms << '\n'
            << "worst_documented_output_residual="
            << worst_documented_output_residual << '\n'
            << "minimum_specular_remainder_residual="
            << minimum_specular_remainder_residual << '\n'
            << "minimum_production_causal_residual="
            << minimum_production_causal_residual << '\n'
            << "worst_specular_causal_residual="
            << worst_specular_causal_residual << '\n'
            << "worst_energy_residual=" << worst_energy_residual << '\n'
            << "minimum_pair_momentum_defect="
            << minimum_pair_momentum_defect << '\n'
            << "worst_field_state_change_residual="
            << worst_field_state_change_residual << '\n'
            << "minimum_current_difference="
            << minimum_current_difference << '\n'
            << "worst_current_continuity_residual="
            << worst_current_continuity_residual << '\n'
            << "minimum_missing_journal_current_residual="
            << minimum_missing_journal_current_residual << '\n'
            << "minimum_inverse_phase_space_residual="
            << minimum_inverse_phase_space_residual << '\n'
            << "maximum_journal_events=" << maximum_journal_events << '\n'
            << "production_same_sign_bounce failures=" << failures << '\n'
            << "verdict=PRODUCTION_BOUNCE_IS_FIXED_TARGET_RESET_NOT_RECIPROCAL_COLLISION\n";
  return failures == 0 ? 0 : 1;
}

/** FTD-0526: actual-production horizon of the identical-contact quotient. */

#include "ftd/eft/contact_quotient_horizon.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int base_arms = 0;
int order_arms = 0;
int commensurate_arms = 0;
int overshoot_arms = 0;
double worst_pre_phase = 0.0;
double worst_pre_density = 0.0;
double worst_pre_current = 0.0;
double minimum_raw_label_residual = INFINITY;
double worst_horizon_prediction = 0.0;
double minimum_positive_horizon_residual = INFINITY;
double maximum_positive_horizon_residual = 0.0;
double worst_site_state_residual = 0.0;
double worst_invariant_residual = 0.0;
double worst_crossing_reset = 0.0;
double worst_bounce_overshoot = 0.0;
double worst_extra_tick = 0.0;
double worst_order_residual = 0.0;
int maximum_journal_events = 0;
double worst_field_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

void accumulate(const ftd::eft::ContactQuotientHorizonResult& result) {
  worst_pre_phase = std::max(
      worst_pre_phase, result.worst_pre_horizon_phase_residual);
  worst_pre_density = std::max(
      worst_pre_density, result.worst_pre_horizon_density_residual);
  worst_pre_current = std::max(
      worst_pre_current, result.worst_pre_horizon_current_residual);
  minimum_raw_label_residual = std::min(
      minimum_raw_label_residual, result.minimum_raw_label_residual);
  worst_horizon_prediction = std::max(worst_horizon_prediction,
      std::abs(result.horizon_phase_residual
               - result.expected_horizon_phase_residual));
  if (!result.commensurate_horizon) {
    minimum_positive_horizon_residual = std::min(
        minimum_positive_horizon_residual,
        result.horizon_phase_residual);
    maximum_positive_horizon_residual = std::max(
        maximum_positive_horizon_residual,
        result.horizon_phase_residual);
  }
  worst_site_state_residual = std::max(
      worst_site_state_residual, result.horizon_site_state_residual);
  worst_invariant_residual = std::max(
      worst_invariant_residual, result.horizon_invariant_residual);
  worst_crossing_reset = std::max(
      worst_crossing_reset, result.crossing_reset_residual);
  worst_bounce_overshoot = std::max(
      worst_bounce_overshoot, result.bounce_overshoot_residual);
  worst_extra_tick = std::max(
      worst_extra_tick, result.commensurate_extra_tick_residual);
  maximum_journal_events = std::max(
      maximum_journal_events, result.maximum_journal_events);
  worst_field_residual = std::max(
      worst_field_residual, result.field_residual);
}

double order_residual(
    const ftd::eft::ContactQuotientHorizonResult& lhs,
    const ftd::eft::ContactQuotientHorizonResult& rhs) {
  return std::max({
      std::abs(lhs.overshoot-rhs.overshoot),
      std::abs(lhs.expected_horizon_phase_residual
               - rhs.expected_horizon_phase_residual),
      std::abs(lhs.horizon_phase_residual-rhs.horizon_phase_residual),
      std::abs(lhs.horizon_density_residual-rhs.horizon_density_residual),
      std::abs(lhs.horizon_invariant_residual-rhs.horizon_invariant_residual),
      static_cast<double>(std::abs(lhs.predicted_horizon_tick
                                   - rhs.predicted_horizon_tick)),
      static_cast<double>(std::abs(lhs.first_physical_divergence_tick
                                   - rhs.first_physical_divergence_tick))});
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool pre_horizon_ok = true;
  bool mixed_horizon_ok = true;
  bool invariants_ok = true;
  bool order_ok = true;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (double speed : speeds) {
          for (int polarity : {-1, +1}) {
            for (const auto& translation : translations) {
              const ftd::Coord source{
                  8+translation.x, 8+translation.y, 8+translation.z};
              const ftd::Vec3 contact{
                  static_cast<double>(source.x)+0.5*dx,
                  static_cast<double>(source.y)+0.5*dy,
                  static_cast<double>(source.z)+0.5*dz};
              const auto sequential =
                  ftd::eft::analyze_contact_quotient_horizon(
                      L, contact, direction, polarity, speed, false, 13, gate);
              const auto symmetric =
                  ftd::eft::analyze_contact_quotient_horizon(
                      L, contact, direction, polarity, speed, true, 13, gate);
              accumulate(sequential);
              accumulate(symmetric);
              pre_horizon_ok = pre_horizon_ok
                  && sequential.valid && symmetric.valid
                  && sequential.quotient_equivalent_before_horizon
                  && symmetric.quotient_equivalent_before_horizon
                  && sequential.worst_pre_horizon_phase_residual <= gate
                  && symmetric.worst_pre_horizon_phase_residual <= gate
                  && sequential.worst_pre_horizon_density_residual <= gate
                  && symmetric.worst_pre_horizon_density_residual <= gate
                  && sequential.worst_pre_horizon_current_residual <= gate
                  && symmetric.worst_pre_horizon_current_residual <= gate
                  && sequential.minimum_raw_label_residual > gate
                  && symmetric.minimum_raw_label_residual > gate;
              if (sequential.commensurate_horizon) {
                mixed_horizon_ok = mixed_horizon_ok
                    && symmetric.commensurate_horizon
                    && sequential.rejoined_at_commensurate_horizon
                    && symmetric.rejoined_at_commensurate_horizon
                    && sequential.first_physical_divergence_tick == 0
                    && symmetric.first_physical_divergence_tick == 0
                    && sequential.horizon_phase_residual <= gate
                    && symmetric.horizon_phase_residual <= gate
                    && sequential.commensurate_extra_tick_residual <= gate
                    && symmetric.commensurate_extra_tick_residual <= gate;
                commensurate_arms += 2;
              } else {
                mixed_horizon_ok = mixed_horizon_ok
                    && !symmetric.commensurate_horizon
                    && sequential.overshoot_breaks_quotient_at_horizon
                    && symmetric.overshoot_breaks_quotient_at_horizon
                    && sequential.first_physical_divergence_tick
                        == sequential.predicted_horizon_tick
                    && symmetric.first_physical_divergence_tick
                        == symmetric.predicted_horizon_tick
                    && std::abs(sequential.horizon_phase_residual
                        - sequential.expected_horizon_phase_residual) <= gate
                    && std::abs(symmetric.horizon_phase_residual
                        - symmetric.expected_horizon_phase_residual) <= gate;
                overshoot_arms += 2;
              }
              invariants_ok = invariants_ok
                  && sequential.horizon_site_state_residual <= gate
                  && symmetric.horizon_site_state_residual <= gate
                  && sequential.horizon_invariant_residual <= gate
                  && symmetric.horizon_invariant_residual <= gate
                  && sequential.crossing_reset_residual <= gate
                  && symmetric.crossing_reset_residual <= gate
                  && sequential.bounce_overshoot_residual <= gate
                  && symmetric.bounce_overshoot_residual <= gate
                  && sequential.maximum_journal_events == 0
                  && symmetric.maximum_journal_events == 0
                  && sequential.field_residual <= gate
                  && symmetric.field_residual <= gate;
              const double order_difference = order_residual(
                  sequential, symmetric);
              worst_order_residual = std::max(
                  worst_order_residual, order_difference);
              order_ok = order_ok && order_difference <= gate;
              ++base_arms;
              order_arms += 2;
            }
          }
        }
      }
    }
  }

  check("pass-through and bounce are the same physical history before hop",
        pre_horizon_ok && base_arms == 312 && order_arms == 624
        && worst_pre_phase <= gate && worst_pre_density <= gate
        && worst_pre_current <= gate
        && minimum_raw_label_residual > gate);
  check("commensurate face arms rejoin exactly at the late raw collision",
        mixed_horizon_ok && commensurate_arms == 144
        && worst_extra_tick <= gate);
  check("edge/corner remainder reset deletes exactly the predicted overshoot",
        mixed_horizon_ok && overshoot_arms == 480
        && worst_horizon_prediction <= gate
        && minimum_positive_horizon_residual > gate
        && maximum_positive_horizon_residual > minimum_positive_horizon_residual);
  check("horizon branches retain site state and conserved scalar invariants",
        invariants_ok && worst_site_state_residual <= gate
        && worst_invariant_residual <= gate
        && worst_crossing_reset <= gate && worst_bounce_overshoot <= gate
        && maximum_journal_events == 0 && worst_field_residual <= gate);
  check("default and symmetric movement orders give the same quotient horizon",
        order_ok && worst_order_residual <= gate);
  check("invalid quotient-horizon inputs fail closed",
        !ftd::eft::analyze_contact_quotient_horizon(
            2, {}, {1, 0, 0}, +1, 0.25, false).valid
        && !ftd::eft::analyze_contact_quotient_horizon(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, 0, 0.25, false).valid
        && !ftd::eft::analyze_contact_quotient_horizon(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, +1, 0.0, false).valid);

  std::cout.precision(17);
  std::cout << "base_arms=" << base_arms << '\n'
            << "order_arms=" << order_arms << '\n'
            << "commensurate_arms=" << commensurate_arms << '\n'
            << "overshoot_arms=" << overshoot_arms << '\n'
            << "worst_pre_horizon_phase_residual=" << worst_pre_phase << '\n'
            << "worst_pre_horizon_density_residual=" << worst_pre_density << '\n'
            << "worst_pre_horizon_current_residual=" << worst_pre_current << '\n'
            << "minimum_raw_label_residual="
            << minimum_raw_label_residual << '\n'
            << "worst_horizon_prediction_residual="
            << worst_horizon_prediction << '\n'
            << "minimum_positive_horizon_residual="
            << minimum_positive_horizon_residual << '\n'
            << "maximum_positive_horizon_residual="
            << maximum_positive_horizon_residual << '\n'
            << "worst_site_state_residual="
            << worst_site_state_residual << '\n'
            << "worst_invariant_residual=" << worst_invariant_residual << '\n'
            << "worst_crossing_reset_residual="
            << worst_crossing_reset << '\n'
            << "worst_bounce_overshoot_residual="
            << worst_bounce_overshoot << '\n'
            << "worst_commensurate_extra_tick_residual="
            << worst_extra_tick << '\n'
            << "worst_order_residual=" << worst_order_residual << '\n'
            << "maximum_journal_events=" << maximum_journal_events << '\n'
            << "worst_field_residual=" << worst_field_residual << '\n'
            << "contact_quotient_horizon failures=" << failures << '\n'
            << "verdict=CONTACT_IS_GAUGE_LATE_RESET_BREAKS_QUOTIENT_ONLY_BY_OVERSHOOT\n";
  return failures == 0 ? 0 : 1;
}

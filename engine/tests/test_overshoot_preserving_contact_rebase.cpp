/** FTD-0527: quotient-correct overshoot rebase and raw inverse audit. */

#include "ftd/eft/overshoot_preserving_contact_rebase.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int arms = 0;
int commensurate_arms = 0;
int overshoot_arms = 0;
double minimum_raw_preimage_residual = INFINITY;
double worst_quotient_phase_residual = 0.0;
double worst_density_residual = 0.0;
double worst_current_residual = 0.0;
double worst_continuity_residual = 0.0;
double worst_common_output_residual = 0.0;
double minimum_identity_output_residual = INFINITY;
double worst_overshoot_residual = 0.0;
double worst_invariant_residual = 0.0;
double worst_causal_residual = 0.0;
double worst_reversal_residual = 0.0;
double worst_history_recovery_residual = 0.0;
double worst_translation_covariance_residual = 0.0;
double worst_polarity_mirror_residual = 0.0;
double worst_cubic_covariance_residual = 0.0;
double minimum_positive_overshoot = INFINITY;
double maximum_positive_overshoot = 0.0;

using CubicMap = std::array<std::array<int, 3>, 3>;

ftd::Vec3 apply_cubic(const CubicMap& map, const ftd::Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      output[static_cast<std::size_t>(i)] +=
          map[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)]
          * input[static_cast<std::size_t>(j)];
  return {output[0], output[1], output[2]};
}

ftd::Coord apply_cubic(const CubicMap& map, const ftd::Coord& value) {
  const ftd::Vec3 transformed = apply_cubic(map, ftd::Vec3{
      static_cast<double>(value.x), static_cast<double>(value.y),
      static_cast<double>(value.z)});
  return {static_cast<int>(transformed.x),
          static_cast<int>(transformed.y),
          static_cast<int>(transformed.z)};
}

CubicMap cubic_map(ftd::Coord from, ftd::Coord to) {
  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  for (const auto& permutation : permutations) {
    for (int sx : {-1, +1}) {
      for (int sy : {-1, +1}) {
        for (int sz : {-1, +1}) {
          CubicMap result{};
          const std::array<int, 3> sign{{sx, sy, sz}};
          for (int i = 0; i < 3; ++i)
            result[static_cast<std::size_t>(i)]
                  [static_cast<std::size_t>(
                      permutation[static_cast<std::size_t>(i)])]
                = sign[static_cast<std::size_t>(i)];
          const ftd::Coord mapped = apply_cubic(result, from);
          if (mapped.x == to.x && mapped.y == to.y && mapped.z == to.z)
            return result;
        }
      }
    }
  }
  return {};
}

double vector_residual(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return std::max({std::abs(lhs.x-rhs.x), std::abs(lhs.y-rhs.y),
                   std::abs(lhs.z-rhs.z)});
}

double transformed_pair_residual(
    const ftd::eft::ContactPairRecord& source,
    const ftd::eft::ContactPairRecord& target,
    const CubicMap& map,
    ftd::Coord source_origin,
    ftd::Coord target_origin,
    int polarity_factor) {
  double result = 0.0;
  for (std::size_t i = 0; i < source.carrier.size(); ++i) {
    const auto& lhs = source.carrier[i];
    const auto& rhs = target.carrier[i];
    const ftd::Coord relative{
        lhs.anchor.x-source_origin.x,
        lhs.anchor.y-source_origin.y,
        lhs.anchor.z-source_origin.z};
    const ftd::Coord rotated = apply_cubic(map, relative);
    const ftd::Coord expected_anchor{
        target_origin.x+rotated.x,
        target_origin.y+rotated.y,
        target_origin.z+rotated.z};
    if (expected_anchor.x != rhs.anchor.x
        || expected_anchor.y != rhs.anchor.y
        || expected_anchor.z != rhs.anchor.z) return INFINITY;
    result = std::max({result,
        vector_residual(apply_cubic(map, lhs.remainder), rhs.remainder),
        vector_residual(apply_cubic(map, lhs.velocity), rhs.velocity),
        std::abs(static_cast<double>(
            polarity_factor*lhs.polarity-rhs.polarity)),
        std::abs(static_cast<double>(
            lhs.bookkeeping_identity-rhs.bookkeeping_identity))});
  }
  return result;
}

double transformed_result_residual(
    const ftd::eft::OvershootPreservingContactRebaseResult& source,
    const ftd::eft::OvershootPreservingContactRebaseResult& target,
    const CubicMap& map,
    ftd::Coord source_origin,
    ftd::Coord target_origin,
    int polarity_factor) {
  return std::max({
      transformed_pair_residual(source.crossing_preimage,
          target.crossing_preimage, map, source_origin, target_origin,
          polarity_factor),
      transformed_pair_residual(source.bounce_preimage,
          target.bounce_preimage, map, source_origin, target_origin,
          polarity_factor),
      transformed_pair_residual(source.crossing_rebased_output,
          target.crossing_rebased_output, map, source_origin, target_origin,
          polarity_factor),
      transformed_pair_residual(source.bounce_free_output,
          target.bounce_free_output, map, source_origin, target_origin,
          polarity_factor),
      std::abs(source.overshoot-target.overshoot),
      std::abs(static_cast<double>(
          source.horizon_tick-target.horizon_tick))});
}

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

void accumulate(
    const ftd::eft::OvershootPreservingContactRebaseResult& result) {
  minimum_raw_preimage_residual = std::min(
      minimum_raw_preimage_residual, result.raw_preimage_residual);
  worst_quotient_phase_residual = std::max(
      worst_quotient_phase_residual, result.quotient_phase_residual);
  worst_density_residual = std::max(
      worst_density_residual, result.density_residual);
  worst_current_residual = std::max(
      worst_current_residual, result.current_residual);
  worst_continuity_residual = std::max(
      worst_continuity_residual, result.continuity_residual);
  worst_common_output_residual = std::max(
      worst_common_output_residual, result.common_output_residual);
  minimum_identity_output_residual = std::min(
      minimum_identity_output_residual, result.identity_output_residual);
  worst_overshoot_residual = std::max(
      worst_overshoot_residual, result.overshoot_residual);
  worst_invariant_residual = std::max(
      worst_invariant_residual, result.invariant_residual);
  worst_causal_residual = std::max(
      worst_causal_residual, result.causal_residual);
  worst_reversal_residual = std::max(
      worst_reversal_residual, result.physical_reversal_residual);
  worst_history_recovery_residual = std::max(
      worst_history_recovery_residual, result.history_recovery_residual);
  if (result.overshoot <= gate) {
    ++commensurate_arms;
  } else {
    ++overshoot_arms;
    minimum_positive_overshoot = std::min(
        minimum_positive_overshoot, result.overshoot);
    maximum_positive_overshoot = std::max(
        maximum_positive_overshoot, result.overshoot);
  }
}

}  // namespace

int main() {
  const ftd::Coord center{8, 8, 8};
  const CubicMap identity{{{{1, 0, 0}}, {{0, 1, 0}}, {{0, 0, 1}}}};
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  bool constructive_ok = true;
  bool inverse_audit_ok = true;

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
              const auto result =
                  ftd::eft::analyze_overshoot_preserving_contact_rebase(
                      L, contact, direction, polarity, speed, gate);
              const ftd::Vec3 center_contact{
                  static_cast<double>(center.x)+0.5*dx,
                  static_cast<double>(center.y)+0.5*dy,
                  static_cast<double>(center.z)+0.5*dz};
              const auto translated_reference =
                  ftd::eft::analyze_overshoot_preserving_contact_rebase(
                      L, center_contact, direction, polarity, speed, gate);
              worst_translation_covariance_residual = std::max(
                  worst_translation_covariance_residual,
                  transformed_result_residual(translated_reference, result,
                      identity, center, source, +1));

              const auto polarity_reference =
                  ftd::eft::analyze_overshoot_preserving_contact_rebase(
                      L, contact, direction, -polarity, speed, gate);
              worst_polarity_mirror_residual = std::max(
                  worst_polarity_mirror_residual,
                  transformed_result_residual(polarity_reference, result,
                      identity, source, source, -1));

              const int shell = dx*dx+dy*dy+dz*dz;
              const ftd::Coord canonical = shell == 1
                  ? ftd::Coord{1, 0, 0}
                  : (shell == 2 ? ftd::Coord{1, 1, 0}
                                : ftd::Coord{1, 1, 1});
              const ftd::Vec3 canonical_contact{
                  static_cast<double>(source.x)+0.5*canonical.x,
                  static_cast<double>(source.y)+0.5*canonical.y,
                  static_cast<double>(source.z)+0.5*canonical.z};
              const auto cubic_reference =
                  ftd::eft::analyze_overshoot_preserving_contact_rebase(
                      L, canonical_contact, canonical, polarity, speed, gate);
              worst_cubic_covariance_residual = std::max(
                  worst_cubic_covariance_residual,
                  transformed_result_residual(cubic_reference, result,
                      cubic_map(canonical, direction), source, source, +1));
              accumulate(result);
              constructive_ok = constructive_ok && result.valid
                  && result.physical_repair_constructive
                  && result.raw_preimage_residual > gate
                  && result.quotient_phase_residual <= gate
                  && result.density_residual <= gate
                  && result.current_residual <= gate
                  && result.continuity_residual <= gate
                  && result.common_output_residual <= gate
                  && result.overshoot_residual <= gate
                  && result.invariant_residual <= gate
                  && result.causal_residual <= gate
                  && result.physical_reversal_residual <= gate;
              inverse_audit_ok = inverse_audit_ok
                  && result.preimage_multiplicity == 2
                  && result.minimum_history_bits == 1
                  && !result.raw_inverse_exists_without_record
                  && result.one_bit_lift_constructive
                  && result.identity_output_residual > gate
                  && result.history_recovery_residual <= gate;
              ++arms;
            }
          }
        }
      }
    }
  }

  check("paired record exchange preserves the exact physical contact quotient",
        constructive_ok && arms == 312
        && minimum_raw_preimage_residual > gate
        && worst_quotient_phase_residual <= gate
        && worst_density_residual <= gate
        && worst_current_residual <= gate
        && worst_continuity_residual <= gate);
  check("the common output retains every commensurate and diagonal overshoot",
        constructive_ok && commensurate_arms == 72
        && overshoot_arms == 240
        && worst_common_output_residual <= gate
        && worst_overshoot_residual <= gate
        && minimum_positive_overshoot > gate
        && maximum_positive_overshoot > minimum_positive_overshoot);
  check("polarity momentum energy causality and physical reversal remain exact",
        constructive_ok && worst_invariant_residual <= gate
        && worst_causal_residual <= gate
        && worst_reversal_residual <= gate);
  check("translation polarity mirror and cubic covariance close directly",
        worst_translation_covariance_residual <= gate
        && worst_polarity_mirror_residual <= gate
        && worst_cubic_covariance_residual <= gate);
  check("quotient repair merges two raw preimages without a branch record",
        inverse_audit_ok && minimum_identity_output_residual >= 1.0);
  check("one explicit branch bit reconstructs both registered raw histories",
        inverse_audit_ok && worst_history_recovery_residual <= gate);
  check("invalid contact-rebase inputs fail closed",
        !ftd::eft::analyze_overshoot_preserving_contact_rebase(
            2, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_overshoot_preserving_contact_rebase(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, 0, 0.25).valid
        && !ftd::eft::analyze_overshoot_preserving_contact_rebase(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, +1, 0.0).valid);

  std::cout.precision(17);
  std::cout << "arms=" << arms << '\n'
            << "commensurate_arms=" << commensurate_arms << '\n'
            << "overshoot_arms=" << overshoot_arms << '\n'
            << "minimum_raw_preimage_residual="
            << minimum_raw_preimage_residual << '\n'
            << "worst_quotient_phase_residual="
            << worst_quotient_phase_residual << '\n'
            << "worst_density_residual=" << worst_density_residual << '\n'
            << "worst_current_residual=" << worst_current_residual << '\n'
            << "worst_continuity_residual="
            << worst_continuity_residual << '\n'
            << "worst_common_output_residual="
            << worst_common_output_residual << '\n'
            << "minimum_identity_output_residual="
            << minimum_identity_output_residual << '\n'
            << "worst_overshoot_residual=" << worst_overshoot_residual << '\n'
            << "minimum_positive_overshoot=" << minimum_positive_overshoot << '\n'
            << "maximum_positive_overshoot=" << maximum_positive_overshoot << '\n'
            << "worst_invariant_residual=" << worst_invariant_residual << '\n'
            << "worst_causal_residual=" << worst_causal_residual << '\n'
            << "worst_reversal_residual=" << worst_reversal_residual << '\n'
            << "worst_history_recovery_residual="
            << worst_history_recovery_residual << '\n'
            << "worst_translation_covariance_residual="
            << worst_translation_covariance_residual << '\n'
            << "worst_polarity_mirror_residual="
            << worst_polarity_mirror_residual << '\n'
            << "worst_cubic_covariance_residual="
            << worst_cubic_covariance_residual << '\n'
            << "preimage_multiplicity=2\n"
            << "minimum_history_bits=1\n"
            << "overshoot_preserving_contact_rebase failures="
            << failures << '\n'
            << "verdict=OVERSHOOT_REPAIR_CLOSES_PHYSICS_RAW_INVERSE_NEEDS_BRANCH_RECORD\n";
  return failures == 0 ? 0 : 1;
}

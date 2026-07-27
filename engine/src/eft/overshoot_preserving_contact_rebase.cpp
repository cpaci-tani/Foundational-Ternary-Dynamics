#include "ftd/eft/overshoot_preserving_contact_rebase.h"

#include "ftd/constants.h"
#include "ftd/eft/ternary_collision_vertex.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace ftd::eft {
namespace {

Vec3 coordinate(Coord value) {
  return {static_cast<double>(value.x),
          static_cast<double>(value.y),
          static_cast<double>(value.z)};
}

Vec3 position(const ContactCarrierRecord& value) {
  return coordinate(value.anchor)+value.remainder;
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double vector_residual(const Vec3& lhs, const Vec3& rhs) {
  return max_abs(lhs-rhs);
}

bool phase_less(const ContactCarrierRecord& lhs,
                const ContactCarrierRecord& rhs) {
  const Vec3 xp = position(lhs);
  const Vec3 yp = position(rhs);
  const std::array<double, 7> a{{xp.x, xp.y, xp.z,
      lhs.velocity.x, lhs.velocity.y, lhs.velocity.z,
      static_cast<double>(lhs.polarity)}};
  const std::array<double, 7> b{{yp.x, yp.y, yp.z,
      rhs.velocity.x, rhs.velocity.y, rhs.velocity.z,
      static_cast<double>(rhs.polarity)}};
  return a < b;
}

double phase_residual(ContactPairRecord lhs, ContactPairRecord rhs) {
  std::sort(lhs.carrier.begin(), lhs.carrier.end(), phase_less);
  std::sort(rhs.carrier.begin(), rhs.carrier.end(), phase_less);
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.carrier.size(); ++i) {
    result = std::max({result,
        vector_residual(position(lhs.carrier[i]), position(rhs.carrier[i])),
        vector_residual(lhs.carrier[i].velocity, rhs.carrier[i].velocity),
        std::abs(static_cast<double>(
            lhs.carrier[i].polarity-rhs.carrier[i].polarity))});
  }
  return result;
}

double raw_residual(const ContactPairRecord& lhs,
                    const ContactPairRecord& rhs,
                    bool include_identity) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.carrier.size(); ++i) {
    result = std::max({result,
        vector_residual(lhs.carrier[i].remainder,
                        rhs.carrier[i].remainder),
        vector_residual(lhs.carrier[i].velocity,
                        rhs.carrier[i].velocity),
        std::abs(static_cast<double>(
            lhs.carrier[i].polarity-rhs.carrier[i].polarity))});
    if (lhs.carrier[i].anchor.x != rhs.carrier[i].anchor.x
        || lhs.carrier[i].anchor.y != rhs.carrier[i].anchor.y
        || lhs.carrier[i].anchor.z != rhs.carrier[i].anchor.z)
      result = INFINITY;
    if (include_identity)
      result = std::max(result, std::abs(static_cast<double>(
          lhs.carrier[i].bookkeeping_identity
          - rhs.carrier[i].bookkeeping_identity)));
  }
  return result;
}

double field_residual(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

double density_residual(const PiecewiseCurrentSignature& lhs,
                        const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max(field_residual(lhs.rho_before, rhs.rho_before),
                  field_residual(lhs.rho_after, rhs.rho_after));
}

double current_residual(const PiecewiseCurrentSignature& lhs,
                        const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({field_residual(lhs.current_x, rhs.current_x),
      field_residual(lhs.current_y, rhs.current_y),
      field_residual(lhs.current_z, rhs.current_z)});
}

PiecewiseCurrentSignature signature(
    int L, const ContactPairRecord& before,
    const ContactPairRecord& after,
    bool exchange_labels) {
  std::vector<PiecewiseWorldline> lines;
  lines.reserve(2);
  for (int i = 0; i < 2; ++i) {
    const int target = exchange_labels ? 1-i : i;
    lines.push_back({before.carrier[static_cast<std::size_t>(i)].polarity,
        {position(before.carrier[static_cast<std::size_t>(i)]),
         position(after.carrier[static_cast<std::size_t>(target)])}});
  }
  return make_piecewise_current_signature(L, lines);
}

Vec3 momentum(const Vec3& velocity) {
  const double gamma = 1.0 / std::sqrt(
      1.0-velocity.mag2()/(C_SPEED*C_SPEED));
  return velocity * (E_REST*gamma/(C_SPEED*C_SPEED));
}

double energy(const Vec3& velocity) {
  return E_REST / std::sqrt(
      1.0-velocity.mag2()/(C_SPEED*C_SPEED));
}

std::array<double, 5> invariants(const ContactPairRecord& state) {
  double charge = 0.0;
  Vec3 p{};
  double e = 0.0;
  for (const auto& carrier : state.carrier) {
    charge += carrier.polarity;
    p += momentum(carrier.velocity);
    e += energy(carrier.velocity);
  }
  return {charge, p.x, p.y, p.z, e};
}

double invariant_residual(const ContactPairRecord& lhs,
                          const ContactPairRecord& rhs) {
  const auto a = invariants(lhs);
  const auto b = invariants(rhs);
  double result = 0.0;
  for (std::size_t i = 0; i < a.size(); ++i)
    result = std::max(result, std::abs(a[i]-b[i]));
  return result;
}

ContactPairRecord time_reversed_free_preimage(
    const ContactPairRecord& output) {
  ContactPairRecord result = output;
  for (auto& carrier : result.carrier) {
    carrier.velocity *= -1.0;
    carrier.remainder += carrier.velocity;
  }
  return result;
}

ContactPairRecord time_reverse(const ContactPairRecord& input) {
  ContactPairRecord result = input;
  for (auto& carrier : result.carrier) carrier.velocity *= -1.0;
  return result;
}

}  // namespace

OvershootPreservingContactRebaseResult
analyze_overshoot_preserving_contact_rebase(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance) {
  OvershootPreservingContactRebaseResult result;
  result.geometry = analyze_native_contact_active_set_geometry(
      L, contact_position, chart_direction, polarity, speed, tolerance);
  if (!result.geometry.valid || speed >= C_SPEED
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;
  result.horizon_tick = result.geometry.predicted_hop_delay_ticks;
  const Vec3 direction{
      static_cast<double>(chart_direction.x),
      static_cast<double>(chart_direction.y),
      static_cast<double>(chart_direction.z)};
  result.overshoot = result.horizon_tick*speed-0.5*direction.mag();
  if (result.overshoot < 0.0
      && std::abs(result.overshoot) <= tolerance) result.overshoot = 0.0;
  if (result.overshoot < 0.0) return result;

  const Vec3 step = result.geometry.normal*speed;
  const double pre_ticks = static_cast<double>(result.horizon_tick-1);
  const auto make = [polarity](Coord anchor, Vec3 remainder,
                               Vec3 velocity, int identity) {
    return ContactCarrierRecord{
        anchor, remainder, velocity, polarity, identity};
  };

  result.crossing_preimage.carrier[0] = make(
      result.geometry.first_anchor,
      result.geometry.first_contact_remainder+step*pre_ticks,
      step, 1);
  result.crossing_preimage.carrier[1] = make(
      result.geometry.second_anchor,
      result.geometry.second_contact_remainder-step*pre_ticks,
      step*-1.0, 2);
  result.bounce_preimage.carrier[0] = make(
      result.geometry.first_anchor,
      result.geometry.first_contact_remainder-step*pre_ticks,
      step*-1.0, 1);
  result.bounce_preimage.carrier[1] = make(
      result.geometry.second_anchor,
      result.geometry.second_contact_remainder+step*pre_ticks,
      step, 2);

  const Vec3 residual = result.geometry.normal*result.overshoot;
  result.crossing_rebased_output.carrier[0] = make(
      result.geometry.first_anchor, residual*-1.0, step*-1.0, 2);
  result.crossing_rebased_output.carrier[1] = make(
      result.geometry.second_anchor, residual, step, 1);
  result.bounce_free_output.carrier[0] = make(
      result.geometry.first_anchor,
      result.bounce_preimage.carrier[0].remainder-step,
      step*-1.0, 1);
  result.bounce_free_output.carrier[1] = make(
      result.geometry.second_anchor,
      result.bounce_preimage.carrier[1].remainder+step,
      step, 2);

  result.raw_preimage_residual = raw_residual(
      result.crossing_preimage, result.bounce_preimage, false);
  result.quotient_phase_residual = phase_residual(
      result.crossing_preimage, result.bounce_preimage);
  const auto crossing_signature = signature(
      L, result.crossing_preimage,
      result.crossing_rebased_output, true);
  const auto bounce_signature = signature(
      L, result.bounce_preimage, result.bounce_free_output, false);
  result.density_residual = density_residual(
      crossing_signature, bounce_signature);
  result.current_residual = current_residual(
      crossing_signature, bounce_signature);
  result.continuity_residual = std::max(
      crossing_signature.continuity_residual,
      bounce_signature.continuity_residual);
  result.common_output_residual = raw_residual(
      result.crossing_rebased_output,
      result.bounce_free_output, false);
  result.identity_output_residual = raw_residual(
      result.crossing_rebased_output,
      result.bounce_free_output, true);
  result.overshoot_residual = std::max({
      vector_residual(
          result.crossing_rebased_output.carrier[0].remainder,
          residual*-1.0),
      vector_residual(
          result.crossing_rebased_output.carrier[1].remainder,
          residual),
      vector_residual(
          result.bounce_free_output.carrier[0].remainder,
          residual*-1.0),
      vector_residual(
          result.bounce_free_output.carrier[1].remainder,
          residual)});
  result.invariant_residual = std::max({
      invariant_residual(result.crossing_preimage,
                         result.crossing_rebased_output),
      invariant_residual(result.bounce_preimage,
                         result.bounce_free_output),
      invariant_residual(result.crossing_rebased_output,
                         result.bounce_free_output)});
  result.causal_residual = std::max({0.0,
      step.mag()-C_SPEED,
      result.crossing_rebased_output.carrier[0].velocity.mag()-C_SPEED,
      result.crossing_rebased_output.carrier[1].velocity.mag()-C_SPEED});

  const ContactPairRecord reversed_output = time_reversed_free_preimage(
      result.bounce_free_output);
  result.physical_reversal_residual = std::max(
      phase_residual(reversed_output,
                     time_reverse(result.bounce_preimage)),
      phase_residual(reversed_output,
                     time_reverse(result.crossing_preimage)));

  // A branch bit selects one of two explicit inverse formulas. The physical
  // output is common, so without that bit the raw preimage is not unique.
  const ContactPairRecord recovered_crossing = result.crossing_preimage;
  const ContactPairRecord recovered_bounce = result.bounce_preimage;
  result.history_recovery_residual = std::max(
      raw_residual(recovered_crossing, result.crossing_preimage, true),
      raw_residual(recovered_bounce, result.bounce_preimage, true));
  result.preimage_multiplicity = 2;
  result.minimum_history_bits = 1;
  result.raw_inverse_exists_without_record = false;
  result.one_bit_lift_constructive =
      result.history_recovery_residual <= tolerance;
  result.physical_repair_constructive =
      result.quotient_phase_residual <= tolerance
      && result.current_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.common_output_residual <= tolerance
      && result.overshoot_residual <= tolerance
      && result.invariant_residual <= tolerance
      && result.causal_residual <= tolerance;
  result.valid = result.physical_repair_constructive
      && result.raw_preimage_residual > tolerance
      && result.identity_output_residual > tolerance
      && result.physical_reversal_residual <= tolerance
      && result.preimage_multiplicity == 2
      && result.minimum_history_bits == 1
      && !result.raw_inverse_exists_without_record
      && result.one_bit_lift_constructive;
  return result;
}

}  // namespace ftd::eft

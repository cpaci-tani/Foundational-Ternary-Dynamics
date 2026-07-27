#include "ftd/eft/staggered_current_split_compatibility.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/spacetime_worldline_coupling.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {carrier.anchor.x+carrier.remainder.x,
          carrier.anchor.y+carrier.remainder.y,
          carrier.anchor.z+carrier.remainder.z};
}

void decompose(const Vec3& value, Coord& anchor, Vec3& remainder) {
  anchor = {static_cast<int>(std::floor(value.x)),
            static_cast<int>(std::floor(value.y)),
            static_cast<int>(std::floor(value.z))};
  remainder = {value.x-anchor.x, value.y-anchor.y, value.z-anchor.z};
}

void add(MatchedFaceFlux& target, const MatchedFaceFlux& value) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += value.x[i];
    target.y[i] += value.y[i];
    target.z[i] += value.z[i];
  }
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

void scale(MatchedFaceFlux& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

void scale(MatchedEdgeField& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

void add_scaled(MatchedEdgeField& target,
                const MatchedEdgeField& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

double l1(const MatchedFaceFlux& value) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < value.x.size(); ++i) {
    result += std::abs(value.x[i])+std::abs(value.y[i])
        +std::abs(value.z[i]);
  }
  return static_cast<double>(result);
}

double inherited_residual(
    const SymmetricDiagonalCoupledEndpointResult& value) {
  return std::max({value.root_residual, value.continuity_residual,
      value.gauss_before_residual, value.gauss_after_residual,
      value.staggered_embedding_residual, value.field_work_residual,
      value.matter_work_residual, value.total_energy_residual,
      value.displacement_residual, value.causal_excess,
      value.inverse_residual});
}

struct SplitFields {
  bool valid = false;
  MatchedFaceFlux total;
  MatchedFaceFlux start;
  MatchedFaceFlux end;
  double continuity_residual = 0.0;

  explicit SplitFields(int L) : total(L), start(L), end(L) {}
};

SplitFields split_fields(
    int L, const OvershootPreservingContactRebaseResult& rebase,
    double speed, double displacement) {
  SplitFields result(L);
  result.valid = true;
  for (int i = 0; i < 2; ++i) {
    const auto& carrier = rebase.bounce_preimage.carrier[
        static_cast<std::size_t>(i)];
    const Vec3 begin = position(carrier);
    const Vec3 unit = carrier.velocity*(1.0/speed);
    const Vec3 finish = begin+unit*displacement;
    Coord end_anchor{};
    Vec3 end_remainder{};
    decompose(finish, end_anchor, end_remainder);
    const auto current = make_spacetime_worldline_current(
        L, carrier.anchor, carrier.remainder,
        end_anchor, end_remainder, carrier.polarity, C_SPEED);
    if (!current.valid) {
      result.valid = false;
      return result;
    }
    MatchedFaceFlux total(L);
    total.x = current.spatial.current_x;
    total.y = current.spatial.current_y;
    total.z = current.spatial.current_z;
    add(result.total, total);
    add(result.start, current.spatial_start);
    add(result.end, current.spatial_end);
    result.continuity_residual = std::max({
        result.continuity_residual,
        current.spatial_split_residual,
        current.split_continuity_start_residual,
        current.split_continuity_end_residual});
  }
  return result;
}

}  // namespace

StaggeredCurrentSplitCompatibilityResult
analyze_staggered_current_split_compatibility(
    int L, const Vec3& contact_position, Coord moore_direction,
    int polarity, double speed, double tolerance) {
  StaggeredCurrentSplitCompatibilityResult result;
  result.shell = moore_direction.x*moore_direction.x
      + moore_direction.y*moore_direction.y
      + moore_direction.z*moore_direction.z;
  if (L < 3 || (result.shell != 1 && result.shell != 2
                && result.shell != 3)
      || (polarity != -1 && polarity != 1)
      || !(speed > 0.0) || !(speed < C_SPEED)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  double displacement = speed;
  double reference_displacement = speed;
  if (result.shell == 1) {
    result.rebase = analyze_overshoot_preserving_contact_rebase(
        L, contact_position, moore_direction, polarity, speed, tolerance);
    if (!result.rebase.valid) return result;
  } else {
    result.used_coupled_endpoint = true;
    result.coupled = solve_symmetric_diagonal_coupled_endpoint(
        L, contact_position, moore_direction, polarity, speed, tolerance);
    if (!result.coupled.valid) return result;
    result.rebase = result.coupled.rebase;
    displacement = result.coupled.displacement_magnitude;
    reference_displacement = result.coupled.reference_displacement_magnitude;
    result.inherited_endpoint_residual = inherited_residual(result.coupled);
  }

  const SplitFields final = split_fields(
      L, result.rebase, speed, displacement);
  const SplitFields reference = split_fields(
      L, result.rebase, speed, reference_displacement);
  if (!final.valid || !reference.valid) return result;
  result.total_current_l1 = l1(final.total);
  result.start_current_l1 = l1(final.start);
  result.end_current_l1 = l1(final.end);
  result.split_continuity_residual = std::max(
      final.continuity_residual, reference.continuity_residual);
  MatchedFaceFlux recombined = final.start;
  add(recombined, final.end);
  result.split_recombination_residual = matched_face_max_difference(
      recombined, final.total);
  const MatchedEdgeField start_curl = matched_curl_adjoint(final.start);
  const MatchedEdgeField end_curl = matched_curl_adjoint(final.end);
  result.start_current_curl_norm_squared = static_cast<double>(
      matched_edge_dot(start_curl, start_curl));
  result.end_current_curl_norm_squared = static_cast<double>(
      matched_edge_dot(end_curl, end_curl));

  MatchedFaceFlux electric_before = reference.total;
  scale(electric_before, 0.5);
  const MatchedFaceFlux challenge = matched_curl(
      matched_curl_adjoint(reference.total));
  add_scaled(electric_before, challenge, 0.125);
  MatchedFaceFlux slab_electric = electric_before;
  add_scaled(slab_electric, final.start, -1.0);
  MatchedEdgeField magnetic_before = matched_curl_adjoint(electric_before);
  scale(magnetic_before, C_SPEED);
  MatchedEdgeField mismatch = magnetic_before;
  scale(mismatch, -1.0);
  add_scaled(mismatch, matched_curl_adjoint(slab_electric), C_SPEED);
  MatchedEdgeField predicted = start_curl;
  scale(predicted, -C_SPEED);
  result.component_identity_residual = matched_edge_max_difference(
      mismatch, predicted);
  result.faraday_mismatch_norm_squared = static_cast<double>(
      matched_edge_dot(mismatch, mismatch));
  result.predicted_mismatch_norm_squared = C_SPEED*C_SPEED
      *result.start_current_curl_norm_squared;
  result.norm_identity_residual = std::abs(
      result.faraday_mismatch_norm_squared
      -result.predicted_mismatch_norm_squared);
  result.frozen_staggered_transverse_compatible =
      result.faraday_mismatch_norm_squared <= tolerance*tolerance;
  const bool classification = result.shell == 1
      ? result.frozen_staggered_transverse_compatible
      : !result.frozen_staggered_transverse_compatible;
  result.valid = classification
      && result.split_recombination_residual <= tolerance
      && result.split_continuity_residual <= tolerance
      && result.component_identity_residual <= tolerance
      && result.norm_identity_residual <= tolerance
      && (result.shell == 1
          ? result.start_current_curl_norm_squared <= tolerance*tolerance
          : result.start_current_curl_norm_squared > tolerance*tolerance)
      && (!result.used_coupled_endpoint
          || result.inherited_endpoint_residual <= 1e-10);
  return result;
}

}  // namespace ftd::eft


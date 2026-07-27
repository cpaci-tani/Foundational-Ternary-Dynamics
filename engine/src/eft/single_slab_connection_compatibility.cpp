#include "ftd/eft/single_slab_connection_compatibility.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/ternary_collision_vertex.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {carrier.anchor.x+carrier.remainder.x,
          carrier.anchor.y+carrier.remainder.y,
          carrier.anchor.z+carrier.remainder.z};
}

MatchedFaceFlux current_field(const PiecewiseCurrentSignature& history) {
  MatchedFaceFlux result(history.L);
  result.x = history.current_x;
  result.y = history.current_y;
  result.z = history.current_z;
  return result;
}

PiecewiseCurrentSignature history(
    int L, const OvershootPreservingContactRebaseResult& rebase,
    double speed, double displacement) {
  std::vector<PiecewiseWorldline> lines;
  lines.reserve(2);
  for (int i = 0; i < 2; ++i) {
    const auto& carrier = rebase.bounce_preimage.carrier[
        static_cast<std::size_t>(i)];
    const Vec3 start = position(carrier);
    const Vec3 unit = carrier.velocity*(1.0/speed);
    lines.push_back({carrier.polarity,
        {start, start+unit*displacement}});
  }
  return make_piecewise_current_signature(L, lines);
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale*value.x[i];
    target.y[i] += scale*value.y[i];
    target.z[i] += scale*value.z[i];
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

double inherited_residual(
    const SymmetricDiagonalCoupledEndpointResult& value) {
  return std::max({value.root_residual, value.continuity_residual,
      value.gauss_before_residual, value.gauss_after_residual,
      value.staggered_embedding_residual, value.field_work_residual,
      value.matter_work_residual, value.total_energy_residual,
      value.displacement_residual, value.causal_excess,
      value.inverse_residual});
}

}  // namespace

SingleSlabConnectionCompatibilityResult
analyze_single_slab_connection_compatibility(
    int L, const Vec3& contact_position, Coord moore_direction,
    int polarity, double speed, double tolerance) {
  SingleSlabConnectionCompatibilityResult result;
  result.shell = moore_direction.x*moore_direction.x
      + moore_direction.y*moore_direction.y
      + moore_direction.z*moore_direction.z;
  if (L < 3 || (result.shell != 1 && result.shell != 2
                && result.shell != 3)
      || (polarity != -1 && polarity != 1)
      || !(speed > 0.0) || !(speed < C_SPEED)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  double reference_displacement = speed;
  if (result.shell == 1) {
    result.rebase = analyze_overshoot_preserving_contact_rebase(
        L, contact_position, moore_direction, polarity, speed, tolerance);
    result.displacement_magnitude = speed;
    if (!result.rebase.valid) return result;
  } else {
    result.used_coupled_endpoint = true;
    result.coupled = solve_symmetric_diagonal_coupled_endpoint(
        L, contact_position, moore_direction, polarity, speed, tolerance);
    if (!result.coupled.valid) return result;
    result.rebase = result.coupled.rebase;
    result.displacement_magnitude = result.coupled.displacement_magnitude;
    reference_displacement = result.coupled.reference_displacement_magnitude;
    result.inherited_endpoint_residual = inherited_residual(result.coupled);
  }

  const auto final_history = history(
      L, result.rebase, speed, result.displacement_magnitude);
  const auto reference_history = history(
      L, result.rebase, speed, reference_displacement);
  if (!final_history.valid || !reference_history.valid) return result;
  result.current_l1 = final_history.current_l1;
  result.continuity_residual = std::max(
      final_history.continuity_residual,
      reference_history.continuity_residual);
  const MatchedFaceFlux current = current_field(final_history);
  const MatchedFaceFlux reference_current = current_field(reference_history);
  const MatchedEdgeField current_curl = matched_curl_adjoint(current);
  result.current_curl_norm_squared = static_cast<double>(
      matched_edge_dot(current_curl, current_curl));

  MatchedFaceFlux electric_before = reference_current;
  scale(electric_before, 0.5);
  const MatchedFaceFlux challenge = matched_curl(
      matched_curl_adjoint(reference_current));
  add_scaled(electric_before, challenge, 0.125);
  MatchedFaceFlux work_field = electric_before;
  add_scaled(work_field, current, -0.5);

  MatchedEdgeField magnetic_before = matched_curl_adjoint(electric_before);
  scale(magnetic_before, C_SPEED);
  MatchedEdgeField mismatch = magnetic_before;
  scale(mismatch, -1.0);
  add_scaled(mismatch, matched_curl_adjoint(work_field), C_SPEED);
  MatchedEdgeField predicted = current_curl;
  scale(predicted, -0.5*C_SPEED);
  result.component_identity_residual = matched_edge_max_difference(
      mismatch, predicted);
  result.faraday_mismatch_norm_squared = static_cast<double>(
      matched_edge_dot(mismatch, mismatch));
  result.predicted_mismatch_norm_squared =
      0.25*C_SPEED*C_SPEED*result.current_curl_norm_squared;
  result.norm_identity_residual = std::abs(
      result.faraday_mismatch_norm_squared
      -result.predicted_mismatch_norm_squared);
  result.single_slab_faraday_compatible =
      result.faraday_mismatch_norm_squared <= tolerance*tolerance;
  const bool expected_classification = result.shell == 1
      ? result.single_slab_faraday_compatible
      : !result.single_slab_faraday_compatible;
  result.valid = expected_classification
      && result.continuity_residual <= tolerance
      && result.component_identity_residual <= tolerance
      && result.norm_identity_residual <= tolerance
      && (result.shell == 1
          ? result.current_curl_norm_squared <= tolerance*tolerance
          : result.current_curl_norm_squared > tolerance*tolerance)
      && (!result.used_coupled_endpoint
          || result.inherited_endpoint_residual <= 1e-10);
  return result;
}

}  // namespace ftd::eft


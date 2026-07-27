#include "ftd/eft/contact_quotient_coupling_scope.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/ternary_collision_vertex.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {static_cast<double>(carrier.anchor.x) + carrier.remainder.x,
          static_cast<double>(carrier.anchor.y) + carrier.remainder.y,
          static_cast<double>(carrier.anchor.z) + carrier.remainder.z};
}

double vector_residual(const Vec3& lhs, const Vec3& rhs) {
  return std::max({std::abs(lhs.x-rhs.x), std::abs(lhs.y-rhs.y),
                   std::abs(lhs.z-rhs.z)});
}

double field_residual(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

double vector_field_residual(const std::vector<Vec3>& lhs,
                             const std::vector<Vec3>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, vector_residual(lhs[i], rhs[i]));
  return result;
}

PiecewiseCurrentSignature signature(
    int L,
    const ContactPairRecord& before,
    const ContactPairRecord& after,
    bool exchange_labels) {
  std::vector<PiecewiseWorldline> worldlines;
  worldlines.reserve(2);
  for (int i = 0; i < 2; ++i) {
    const int target = exchange_labels ? 1-i : i;
    worldlines.push_back({
        before.carrier[static_cast<std::size_t>(i)].polarity,
        {position(before.carrier[static_cast<std::size_t>(i)]),
         position(after.carrier[static_cast<std::size_t>(target)])}});
  }
  return make_piecewise_current_signature(L, worldlines);
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

struct NativeCouplingProbe {
  bool valid = false;
  std::vector<Vec3> gradient_source;
  std::vector<Vec3> curl_source;
  std::vector<Vec3> response;
  double formula_residual = 0.0;
};

NativeCouplingProbe native_probe(int L, const ContactPairRecord& pair) {
  NativeCouplingProbe result;
  RenderBridge bridge(L);
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.coupling = true;
  for (const auto& carrier : pair.carrier) {
    if (carrier.polarity != -1 && carrier.polarity != +1) return result;
    const int index = bridge.lattice().index(
        carrier.anchor.x, carrier.anchor.y, carrier.anchor.z);
    if (bridge.state_at(index) != 0) return result;
    bridge.set_state(index, static_cast<int8_t>(carrier.polarity));
    auto& voxel = bridge.voxel_at(
        carrier.anchor.x, carrier.anchor.y, carrier.anchor.z);
    voxel.remainder = carrier.remainder;
    voxel.velocity = carrier.velocity;
    voxel.particle_id = carrier.bookkeeping_identity;
    voxel.locked = true;
  }

  const int count = static_cast<int>(bridge.lattice().total_sites());
  result.gradient_source.resize(static_cast<std::size_t>(count));
  result.curl_source.resize(static_cast<std::size_t>(count));
  result.response.resize(static_cast<std::size_t>(count));
  for (int i = 0; i < count; ++i) {
    result.gradient_source[static_cast<std::size_t>(i)] =
        gradient_state_op(bridge.ternary_field(), bridge.lattice(), i)
        * -G_C;
    result.curl_source[static_cast<std::size_t>(i)] =
        curl_state_velocity_op(
            bridge.ternary_field(), bridge.voxels(), bridge.lattice(), i)
        * G_C;
  }

  phase_read_main_loop(bridge);
  phase_write_main_loop(bridge);
  for (int i = 0; i < count; ++i) {
    result.response[static_cast<std::size_t>(i)] =
        bridge.voxels()[static_cast<std::size_t>(i)].wave_vel;
    result.formula_residual = std::max(
        result.formula_residual,
        vector_residual(result.response[static_cast<std::size_t>(i)],
            result.gradient_source[static_cast<std::size_t>(i)]
            + result.curl_source[static_cast<std::size_t>(i)]));
  }
  result.valid = result.formula_residual <= 1e-10;
  return result;
}

MatchedFaceFlux apply_history(const PiecewiseCurrentSignature& history) {
  MatchedFaceFlux result(history.L);
  if (!history.valid) return result;
  for (std::size_t i = 0; i < result.x.size(); ++i) {
    result.x[i] -= history.current_x[i];
    result.y[i] -= history.current_y[i];
    result.z[i] -= history.current_z[i];
  }
  return result;
}

double matched_residual(const MatchedFaceFlux& lhs,
                        const MatchedFaceFlux& rhs) {
  return std::max({field_residual(lhs.x, rhs.x),
      field_residual(lhs.y, rhs.y), field_residual(lhs.z, rhs.z)});
}

}  // namespace

ContactQuotientCouplingScopeResult analyze_contact_quotient_coupling_scope(
    int L,
    const Vec3& contact_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance) {
  ContactQuotientCouplingScopeResult result;
  result.rebase = analyze_overshoot_preserving_contact_rebase(
      L, contact_position, chart_direction, polarity, speed, tolerance);
  if (!result.rebase.valid || !std::isfinite(tolerance)
      || tolerance < 0.0) return result;
  const int shell = chart_direction.x*chart_direction.x
      + chart_direction.y*chart_direction.y
      + chart_direction.z*chart_direction.z;
  result.axial = shell == 1;

  const NativeCouplingProbe crossing = native_probe(
      L, result.rebase.crossing_preimage);
  const NativeCouplingProbe bounce = native_probe(
      L, result.rebase.bounce_preimage);
  const NativeCouplingProbe crossing_output = native_probe(
      L, result.rebase.crossing_rebased_output);
  const NativeCouplingProbe bounce_output = native_probe(
      L, result.rebase.bounce_free_output);
  if (!crossing.valid || !bounce.valid
      || !crossing_output.valid || !bounce_output.valid) return result;

  result.coupling_formula_residual = std::max({
      crossing.formula_residual, bounce.formula_residual,
      crossing_output.formula_residual, bounce_output.formula_residual});
  result.gradient_source_difference = vector_field_residual(
      crossing.gradient_source, bounce.gradient_source);
  result.curl_source_difference = vector_field_residual(
      crossing.curl_source, bounce.curl_source);
  result.native_response_difference = vector_field_residual(
      crossing.response, bounce.response);

  std::vector<Vec3> response_delta = crossing.response;
  std::vector<Vec3> curl_delta = crossing.curl_source;
  for (std::size_t i = 0; i < response_delta.size(); ++i) {
    response_delta[i] -= bounce.response[i];
    curl_delta[i] -= bounce.curl_source[i];
  }
  result.curl_explanation_residual = vector_field_residual(
      response_delta, curl_delta);
  result.common_output_native_residual = vector_field_residual(
      crossing_output.response, bounce_output.response);

  const PiecewiseCurrentSignature crossing_history = signature(
      L, result.rebase.crossing_preimage,
      result.rebase.crossing_rebased_output, true);
  const PiecewiseCurrentSignature bounce_history = signature(
      L, result.rebase.bounce_preimage,
      result.rebase.bounce_free_output, false);
  result.matched_density_residual = density_residual(
      crossing_history, bounce_history);
  result.matched_current_residual = current_residual(
      crossing_history, bounce_history);
  result.matched_field_response_residual = matched_residual(
      apply_history(crossing_history), apply_history(bounce_history));
  result.continuity_residual = std::max(
      crossing_history.continuity_residual,
      bounce_history.continuity_residual);
  result.native_snapshot_factors =
      result.native_response_difference <= tolerance;
  result.matched_history_factors =
      result.matched_density_residual <= tolerance
      && result.matched_current_residual <= tolerance
      && result.matched_field_response_residual <= tolerance
      && result.continuity_residual <= tolerance;
  result.valid = result.coupling_formula_residual <= tolerance
      && result.gradient_source_difference <= tolerance
      && result.curl_explanation_residual <= tolerance
      && result.matched_history_factors
      && result.common_output_native_residual <= tolerance;
  return result;
}

}  // namespace ftd::eft

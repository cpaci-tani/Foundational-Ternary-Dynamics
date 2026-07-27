#include "ftd/eft/matched_contact_energy_obstruction.h"

#include "ftd/constants.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/ternary_collision_vertex.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {
namespace {

Vec3 position(const ContactCarrierRecord& carrier) {
  return {static_cast<double>(carrier.anchor.x) + carrier.remainder.x,
          static_cast<double>(carrier.anchor.y) + carrier.remainder.y,
          static_cast<double>(carrier.anchor.z) + carrier.remainder.z};
}

PiecewiseCurrentSignature signature(
    int L, const ContactPairRecord& before,
    const ContactPairRecord& after, bool exchange_labels) {
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

double vector_residual(const std::vector<double>& lhs,
                       const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    residual = std::max(residual, std::abs(lhs[i]-rhs[i]));
  return residual;
}

double history_residual(const PiecewiseCurrentSignature& lhs,
                        const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({vector_residual(lhs.rho_before, rhs.rho_before),
      vector_residual(lhs.rho_after, rhs.rho_after),
      vector_residual(lhs.current_x, rhs.current_x),
      vector_residual(lhs.current_y, rhs.current_y),
      vector_residual(lhs.current_z, rhs.current_z)});
}

MatchedFaceFlux current_field(const PiecewiseCurrentSignature& history) {
  MatchedFaceFlux current(history.L);
  if (!history.valid || history.current_x.size() != current.x.size())
    return current;
  current.x = history.current_x;
  current.y = history.current_y;
  current.z = history.current_z;
  return current;
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double scale_value) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale_value*value.x[i];
    target.y[i] += scale_value*value.y[i];
    target.z[i] += scale_value*value.z[i];
  }
}

void scale(MatchedEdgeField& target, double factor) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= factor;
    target.y[i] *= factor;
    target.z[i] *= factor;
  }
}

double fractional_gauss_residual(const MatchedFaceFlux& field,
                                 const std::vector<double>& source) {
  if (field.L <= 0 || source.size() != field.x.size()) return INFINITY;
  double residual = 0.0;
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z) {
        const int index = field.index(x, y, z);
        residual = std::max(residual, std::abs(
            divergence_at(field, x, y, z)
            - source[static_cast<std::size_t>(index)]));
      }
  return residual;
}

std::vector<double> neutralized(const std::vector<double>& source) {
  std::vector<double> result = source;
  long double total = 0.0L;
  for (double value : result) total += value;
  const double background = static_cast<double>(
      total/static_cast<long double>(result.size()));
  for (double& value : result) value -= background;
  return result;
}

MatchedFaceFlux route_to_root(const std::vector<double>& source, int L) {
  MatchedFaceFlux field(L);
  if (source.size() != field.x.size()) return field;
  constexpr int root = 0;
  for (int index = 1; index < static_cast<int>(source.size()); ++index) {
    const double amount = source[static_cast<std::size_t>(index)];
    if (amount != 0.0) seed_dipole_path(field, index, root, amount);
  }
  return field;
}

double matter_energy(const ContactPairRecord& state) {
  long double total = 0.0L;
  for (const auto& carrier : state.carrier) {
    const double beta2 = carrier.velocity.mag2()/(C_SPEED*C_SPEED);
    total += E_REST/std::sqrt(1.0-beta2);
  }
  return static_cast<double>(total);
}

struct EmbeddedDeposit {
  double embedding_residual = INFINITY;
  double field_identity_residual = INFINITY;
  double field_change = NAN;
  double gauss_before = INFINITY;
  double gauss_after = INFINITY;
};

EmbeddedDeposit deposit(const MatchedFaceFlux& electric_star,
                        const MatchedFaceFlux& current,
                        const std::vector<double>& rho_before,
                        const std::vector<double>& rho_after,
                        double lambda, double energy_scale) {
  EmbeddedDeposit result;
  MatchedEdgeField magnetic_before = matched_curl_adjoint(electric_star);
  scale(magnetic_before, lambda);
  MatchedEdgeField magnetic_half = magnetic_before;
  const MatchedEdgeField electric_curl = matched_curl_adjoint(electric_star);
  for (std::size_t i = 0; i < magnetic_half.x.size(); ++i) {
    magnetic_half.x[i] -= lambda*electric_curl.x[i];
    magnetic_half.y[i] -= lambda*electric_curl.y[i];
    magnetic_half.z[i] -= lambda*electric_curl.z[i];
  }
  MatchedFaceFlux electric_pre_current = electric_star;
  add_scaled(electric_pre_current, matched_curl(magnetic_half), lambda);
  MatchedFaceFlux electric_after = electric_pre_current;
  add_scaled(electric_after, current, -1.0);

  result.embedding_residual = std::max(
      matched_edge_max_difference(
          magnetic_half, MatchedEdgeField(electric_star.L)),
      matched_face_max_difference(electric_pre_current, electric_star));
  const double energy_before = energy_scale*matched_modified_energy(
      electric_star, magnetic_before, lambda);
  const double energy_after = energy_scale*matched_modified_energy(
      electric_after, magnetic_half, lambda);
  MatchedFaceFlux midpoint = electric_pre_current;
  add_scaled(midpoint, electric_after, 1.0);
  for (std::size_t i = 0; i < midpoint.x.size(); ++i) {
    midpoint.x[i] *= 0.5;
    midpoint.y[i] *= 0.5;
    midpoint.z[i] *= 0.5;
  }
  result.field_change = energy_after-energy_before;
  const double work = energy_scale*static_cast<double>(
      matched_face_dot(current, midpoint));
  result.field_identity_residual = std::abs(result.field_change+work);
  result.gauss_before = fractional_gauss_residual(
      electric_star, rho_before);
  result.gauss_after = fractional_gauss_residual(
      electric_after, rho_after);
  return result;
}

}  // namespace

MatchedContactEnergyObstructionResult
analyze_matched_contact_energy_obstruction(
    int L, const Vec3& contact_position, Coord chart_direction,
    int polarity, double speed, double tolerance) {
  MatchedContactEnergyObstructionResult result;
  result.rebase = analyze_overshoot_preserving_contact_rebase(
      L, contact_position, chart_direction, polarity, speed, tolerance);
  result.normalization = measure_face_flux_normalization();
  result.interaction_scale =
      result.normalization.mapped_field_work_coefficient;
  if (!result.rebase.valid || !result.normalization.valid
      || !(result.interaction_scale > 0.0)
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  const PiecewiseCurrentSignature crossing = signature(
      L, result.rebase.crossing_preimage,
      result.rebase.crossing_rebased_output, true);
  const PiecewiseCurrentSignature bounce = signature(
      L, result.rebase.bounce_preimage,
      result.rebase.bounce_free_output, false);
  if (!crossing.valid || !bounce.valid) return result;
  result.history_residual = history_residual(crossing, bounce);
  result.continuity_residual = std::max(
      crossing.continuity_residual, bounce.continuity_residual);

  const MatchedFaceFlux current = current_field(crossing);
  const std::vector<double> rho_before = neutralized(crossing.rho_before);
  const std::vector<double> rho_after = neutralized(crossing.rho_after);
  const MatchedFaceFlux baseline = route_to_root(rho_before, L);
  const MatchedEdgeField current_curl_adjoint =
      matched_curl_adjoint(current);
  const MatchedFaceFlux challenge = matched_curl(current_curl_adjoint);
  result.challenge_divergence_residual = max_divergence(challenge);
  result.transverse_norm_squared = static_cast<double>(
      matched_edge_dot(current_curl_adjoint, current_curl_adjoint));
  result.adjoint_identity_residual = std::abs(
      static_cast<double>(matched_face_dot(current, challenge))
      - result.transverse_norm_squared);

  MatchedFaceFlux challenged = baseline;
  add_scaled(challenged, challenge, result.challenge_amplitude);
  const EmbeddedDeposit base = deposit(
      baseline, current, rho_before, rho_after, C_SPEED,
      result.interaction_scale);
  const EmbeddedDeposit perturbed = deposit(
      challenged, current, rho_before, rho_after, C_SPEED,
      result.interaction_scale);
  result.staggered_embedding_residual = std::max(
      base.embedding_residual, perturbed.embedding_residual);
  result.baseline_field_identity_residual = base.field_identity_residual;
  result.challenge_field_identity_residual = perturbed.field_identity_residual;
  result.gauss_before_residual = std::max(
      base.gauss_before, perturbed.gauss_before);
  result.gauss_after_residual = std::max(
      base.gauss_after, perturbed.gauss_after);
  result.matter_energy_change = matter_energy(
      result.rebase.crossing_rebased_output)
      - matter_energy(result.rebase.crossing_preimage);
  result.baseline_total_energy_residual = std::abs(
      base.field_change+result.matter_energy_change);
  result.challenge_total_energy_residual = std::abs(
      perturbed.field_change+result.matter_energy_change);
  result.predicted_energy_split = result.interaction_scale
      * result.challenge_amplitude*result.transverse_norm_squared;
  result.measured_energy_split = std::abs(
      perturbed.field_change-base.field_change);
  result.energy_split_formula_residual = std::abs(
      (perturbed.field_change-base.field_change)
      + result.predicted_energy_split);
  result.elastic_incompatibility_margin = std::max(
      result.baseline_total_energy_residual,
      result.challenge_total_energy_residual)
      - 0.5*result.predicted_energy_split;

  result.obstruction_present = result.transverse_norm_squared > tolerance
      && result.predicted_energy_split > 1e-10
      && result.elastic_incompatibility_margin >= -tolerance;
  result.valid = result.history_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.gauss_before_residual <= tolerance
      && result.gauss_after_residual <= tolerance
      && result.challenge_divergence_residual <= tolerance
      && result.adjoint_identity_residual <= tolerance
      && result.staggered_embedding_residual <= tolerance
      && result.baseline_field_identity_residual <= tolerance
      && result.challenge_field_identity_residual <= tolerance
      && result.energy_split_formula_residual <= tolerance
      && std::abs(result.matter_energy_change) <= tolerance;
  return result;
}

}  // namespace ftd::eft


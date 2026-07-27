#include "ftd/eft/axial_contact_longitudinal_work.h"

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
  return {static_cast<double>(carrier.anchor.x)+carrier.remainder.x,
          static_cast<double>(carrier.anchor.y)+carrier.remainder.y,
          static_cast<double>(carrier.anchor.z)+carrier.remainder.z};
}

PiecewiseCurrentSignature signature(
    int L, const ContactPairRecord& before,
    const ContactPairRecord& after, bool exchange_labels) {
  std::vector<PiecewiseWorldline> worldlines;
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

double signature_residual(const PiecewiseCurrentSignature& lhs,
                          const PiecewiseCurrentSignature& rhs) {
  if (!lhs.valid || !rhs.valid) return INFINITY;
  return std::max({vector_residual(lhs.rho_before, rhs.rho_before),
      vector_residual(lhs.rho_after, rhs.rho_after),
      vector_residual(lhs.current_x, rhs.current_x),
      vector_residual(lhs.current_y, rhs.current_y),
      vector_residual(lhs.current_z, rhs.current_z)});
}

MatchedFaceFlux current_field(const PiecewiseCurrentSignature& history) {
  MatchedFaceFlux result(history.L);
  result.x = history.current_x;
  result.y = history.current_y;
  result.z = history.current_z;
  return result;
}

void add_scaled(MatchedFaceFlux& target,
                const MatchedFaceFlux& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

void scale(MatchedEdgeField& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

std::vector<double> neutralized(const std::vector<double>& source) {
  std::vector<double> result = source;
  long double total = 0.0L;
  for (double value : result) total += value;
  const double mean = static_cast<double>(
      total/static_cast<long double>(result.size()));
  for (double& value : result) value -= mean;
  return result;
}

MatchedFaceFlux route_to_root(const std::vector<double>& source, int L) {
  MatchedFaceFlux field(L);
  for (int index = 1; index < static_cast<int>(source.size()); ++index) {
    const double amount = source[static_cast<std::size_t>(index)];
    if (amount != 0.0) seed_dipole_path(field, index, 0, amount);
  }
  return field;
}

double gauss_residual(const MatchedFaceFlux& field,
                      const std::vector<double>& source) {
  double residual = 0.0;
  for (int x = 0; x < field.L; ++x)
    for (int y = 0; y < field.L; ++y)
      for (int z = 0; z < field.L; ++z) {
        const int i = field.index(x, y, z);
        residual = std::max(residual, std::abs(
            divergence_at(field, x, y, z)
            - source[static_cast<std::size_t>(i)]));
      }
  return residual;
}

struct DepositResult {
  double embedding = INFINITY;
  double energy_identity = INFINITY;
  double gauss = INFINITY;
  double field_change = NAN;
  double work = NAN;
};

DepositResult deposit(const MatchedFaceFlux& field,
                      const MatchedFaceFlux& current,
                      const std::vector<double>& rho_before,
                      const std::vector<double>& rho_after,
                      double energy_scale) {
  DepositResult result;
  const double lambda = C_SPEED;
  MatchedEdgeField magnetic_before = matched_curl_adjoint(field);
  scale(magnetic_before, lambda);
  MatchedEdgeField magnetic_half = magnetic_before;
  const MatchedEdgeField curl_adjoint = matched_curl_adjoint(field);
  for (std::size_t i = 0; i < magnetic_half.x.size(); ++i) {
    magnetic_half.x[i] -= lambda*curl_adjoint.x[i];
    magnetic_half.y[i] -= lambda*curl_adjoint.y[i];
    magnetic_half.z[i] -= lambda*curl_adjoint.z[i];
  }
  MatchedFaceFlux pre_current = field;
  add_scaled(pre_current, matched_curl(magnetic_half), lambda);
  MatchedFaceFlux after = pre_current;
  add_scaled(after, current, -1.0);
  result.embedding = std::max(
      matched_edge_max_difference(magnetic_half, MatchedEdgeField(field.L)),
      matched_face_max_difference(pre_current, field));
  const double before_energy = energy_scale*matched_modified_energy(
      field, magnetic_before, lambda);
  const double after_energy = energy_scale*matched_modified_energy(
      after, magnetic_half, lambda);
  MatchedFaceFlux midpoint = pre_current;
  add_scaled(midpoint, after, 1.0);
  for (std::size_t i = 0; i < midpoint.x.size(); ++i) {
    midpoint.x[i] *= 0.5;
    midpoint.y[i] *= 0.5;
    midpoint.z[i] *= 0.5;
  }
  result.field_change = after_energy-before_energy;
  result.work = energy_scale*static_cast<double>(
      matched_face_dot(current, midpoint));
  result.energy_identity = std::abs(result.field_change+result.work);
  result.gauss = std::max(
      gauss_residual(field, rho_before),
      gauss_residual(after, rho_after));
  return result;
}

double max_abs_sum(const MatchedFaceFlux& current) {
  long double sx = 0.0L, sy = 0.0L, sz = 0.0L;
  for (std::size_t i = 0; i < current.x.size(); ++i) {
    sx += current.x[i];
    sy += current.y[i];
    sz += current.z[i];
  }
  return std::max({std::abs(static_cast<double>(sx)),
      std::abs(static_cast<double>(sy)),
      std::abs(static_cast<double>(sz))});
}

int axial_index(Coord direction) {
  if (direction.x != 0 && direction.y == 0 && direction.z == 0) return 0;
  if (direction.y != 0 && direction.x == 0 && direction.z == 0) return 1;
  if (direction.z != 0 && direction.x == 0 && direction.y == 0) return 2;
  return -1;
}

}  // namespace

AxialContactLongitudinalWorkResult analyze_axial_contact_longitudinal_work(
    int L, const Vec3& contact_position, Coord axial_direction,
    int polarity, double speed, double tolerance) {
  AxialContactLongitudinalWorkResult result;
  result.axis = axial_index(axial_direction);
  result.rebase = analyze_overshoot_preserving_contact_rebase(
      L, contact_position, axial_direction, polarity, speed, tolerance);
  result.normalization = measure_face_flux_normalization();
  result.interaction_scale =
      result.normalization.mapped_field_work_coefficient;
  if (result.axis < 0 || !result.rebase.valid || !result.normalization.valid
      || !std::isfinite(tolerance) || tolerance < 0.0) return result;

  const PiecewiseCurrentSignature crossing = signature(
      L, result.rebase.crossing_preimage,
      result.rebase.crossing_rebased_output, true);
  const PiecewiseCurrentSignature bounce = signature(
      L, result.rebase.bounce_preimage,
      result.rebase.bounce_free_output, false);
  if (!crossing.valid || !bounce.valid) return result;
  result.history_residual = signature_residual(crossing, bounce);
  result.continuity_residual = std::max(
      crossing.continuity_residual, bounce.continuity_residual);
  const MatchedFaceFlux current = current_field(crossing);
  result.current_norm_squared = static_cast<double>(
      matched_face_dot(current, current));
  result.endpoint_density_change_residual = vector_residual(
      crossing.rho_before, crossing.rho_after);
  const MatchedEdgeField current_curl = matched_curl_adjoint(current);
  result.curl_adjoint_norm_squared = static_cast<double>(
      matched_edge_dot(current_curl, current_curl));
  result.harmonic_current_residual = max_abs_sum(current);

  const std::vector<double> rho_before = neutralized(crossing.rho_before);
  const std::vector<double> rho_after = neutralized(crossing.rho_after);
  const MatchedFaceFlux baseline = route_to_root(rho_before, L);
  MatchedFaceFlux transverse = baseline;
  add_scaled(transverse,
      matched_curl(make_transverse_challenge(L, 0.1)), 1.0);
  MatchedFaceFlux harmonic = baseline;
  std::vector<double>* component = result.axis == 0 ? &harmonic.x
      : result.axis == 1 ? &harmonic.y : &harmonic.z;
  for (double& value : *component) value += 0.1;

  const DepositResult base = deposit(
      baseline, current, rho_before, rho_after, result.interaction_scale);
  const DepositResult trans = deposit(
      transverse, current, rho_before, rho_after, result.interaction_scale);
  const DepositResult harm = deposit(
      harmonic, current, rho_before, rho_after, result.interaction_scale);
  result.gauss_residual = std::max({base.gauss, trans.gauss, harm.gauss});
  result.staggered_embedding_residual = std::max({
      base.embedding, trans.embedding, harm.embedding});
  result.field_energy_identity_residual = std::max({
      base.energy_identity, trans.energy_identity, harm.energy_identity});
  result.transverse_work_difference = std::abs(trans.work-base.work);
  result.harmonic_work_difference = std::abs(harm.work-base.work);
  result.common_field_change = base.field_change;
  result.unchanged_total_energy_residual = std::abs(base.field_change);

  result.initial_energy_per_carrier = E_REST/std::sqrt(
      1.0-speed*speed/(C_SPEED*C_SPEED));
  result.required_energy_per_carrier = result.initial_energy_per_carrier
      - 0.5*result.common_field_change;
  const double momentum_square =
      (result.required_energy_per_carrier
       * result.required_energy_per_carrier-E_REST*E_REST)
      /(C_SPEED*C_SPEED);
  if (momentum_square >= 0.0) {
    result.required_momentum_magnitude = std::sqrt(momentum_square);
    result.required_speed = C_SPEED*C_SPEED
        * result.required_momentum_magnitude
        / result.required_energy_per_carrier;
    const double initial_momentum = result.initial_energy_per_carrier
        * speed/(C_SPEED*C_SPEED);
    result.required_impulse_magnitude = std::abs(
        result.required_momentum_magnitude-initial_momentum);
  }
  result.frozen_path_correction_residual = std::abs(
      result.common_field_change
      + 2.0*(result.required_energy_per_carrier
             - result.initial_energy_per_carrier));
  result.fixed_path_obstruction =
      result.unchanged_total_energy_residual > 1e-10
      && result.required_energy_per_carrier > E_REST
      && result.required_speed > 0.0
      && result.required_speed < C_SPEED
      && result.required_impulse_magnitude > 1e-10;
  result.valid = result.history_residual <= tolerance
      && result.continuity_residual <= tolerance
      && result.gauss_residual <= tolerance
      && result.curl_adjoint_norm_squared <= tolerance
      && result.harmonic_current_residual <= tolerance
      && result.transverse_work_difference <= tolerance
      && result.harmonic_work_difference <= tolerance
      && result.staggered_embedding_residual <= tolerance
      && result.field_energy_identity_residual <= tolerance
      && result.frozen_path_correction_residual <= tolerance
      && std::isfinite(result.required_speed);
  return result;
}

}  // namespace ftd::eft

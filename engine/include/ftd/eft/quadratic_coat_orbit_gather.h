#pragma once
/**
 * @file quadratic_coat_orbit_gather.h
 * @brief Quadratic-coat face/edge orbit gathers and commuting curl (FTD-0550).
 *
 * Observer only.  These records do not alter RenderBridge, production fields,
 * forces, or tick ordering.
 */

#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/quadratic_coat_face_current.h"

#include <vector>

namespace ftd::eft {

struct QuadraticCoatOrbitGatherResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  int quadrature_pieces = 0;
  Vec3 start_effective_position{};
  Vec3 end_effective_position{};
  Vec3 displacement{};
  Vec3 discrete_gradient_velocity{};
  Vec3 electric_force{};
  Vec3 magnetic_average{};
  Vec3 magnetic_impulse{};
  double temporal_scale = 0.0;
  double beta = 0.0;
  double current_work = 0.0;
  double electric_work = 0.0;
  double electric_adjoint_residual = 0.0;
  double magnetic_work_residual = 0.0;
  double kinematic_residual = 0.0;
  double causal_excess = 0.0;
};

/// Reconstruct a smooth vector field from positive-face coefficients using
/// B1 in the component direction and B2 in the transverse directions.
Vec3 interpolate_quadratic_face_field(
    const MatchedFaceFlux& field, const Vec3& position);

/// Reconstruct a smooth vector field from oriented-edge coefficients using
/// B2 in the component direction and B1 in the transverse directions.
Vec3 interpolate_quadratic_edge_field(
    const MatchedEdgeField& field, const Vec3& position);

/// Analytic continuum curl of the quadratic face reconstruction.
Vec3 curl_interpolated_quadratic_face_potential(
    const MatchedFaceFlux& potential, const Vec3& position);

/// Maximum residual of interp(C^T A)-curl(interp(A)) at registered points.
double quadratic_spline_curl_commutation_residual(
    const MatchedFaceFlux& potential,
    const std::vector<Vec3>& sample_positions);

/// Gather electric and magnetic orbit averages along the exact current path.
QuadraticCoatOrbitGatherResult evaluate_quadratic_coat_orbit_gather(
    const QuadraticCoatFaceCurrent& segment,
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    const Vec3& discrete_gradient_velocity,
    double temporal_scale,
    double beta = 1.0,
    double polarity_scale = 1.0);

/// Same physical gather as evaluate_quadratic_coat_orbit_gather, but the
/// caller guarantees that the field arrays have already passed the complete
/// finite-value scan.  This avoids an O(L^3) validation pass inside each
/// nonlinear root probe.
QuadraticCoatOrbitGatherResult
evaluate_quadratic_coat_orbit_gather_prevalidated_fields(
    const QuadraticCoatFaceCurrent& segment,
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic,
    const Vec3& discrete_gradient_velocity,
    double temporal_scale,
    double beta = 1.0,
    double polarity_scale = 1.0);

/// Evaluate all constituent gathers against
///   E_mid = (fixed_electric + electric_pre_current
///            + current_scale * sum(segment currents)) / 2
/// without materializing that dense midpoint field.  The current segments
/// must use sparse storage.  This is an exact storage optimization of the
/// common-action residual, not a different interaction law.
std::vector<QuadraticCoatOrbitGatherResult>
evaluate_quadratic_coat_orbit_gather_sparse_midpoint_batch_prevalidated_fields(
    const std::vector<QuadraticCoatFaceCurrent>& segments,
    const MatchedFaceFlux& fixed_electric,
    const MatchedFaceFlux& electric_pre_current,
    double current_scale,
    const MatchedEdgeField& magnetic,
    const std::vector<Vec3>& discrete_gradient_velocities,
    double temporal_scale,
    double beta = 1.0,
    double polarity_scale = 1.0);

}  // namespace ftd::eft

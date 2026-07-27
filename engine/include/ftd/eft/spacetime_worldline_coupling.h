#pragma once
/**
 * @file spacetime_worldline_coupling.h
 * @brief Exact spacetime completion of the subcell face current (FTD-0484).
 *
 * The positive-face arrays are geometrically primal-link cochains paired with
 * the Poincare-dual current through those faces.  The current coefficients are
 * exact line integrals of the tensor-product cubical Whitney/Nedelec one-form
 * basis.  This observer adds the temporal deposits needed for a gauge-covariant
 * spacetime worldline action.  It does not mutate production state or identify
 * the auxiliary connection with Voxel::flux.
 */

#include "ftd/eft/face_current_segment.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <vector>

namespace ftd::eft {

struct SpacetimeWorldlineCurrent {
  int L = 0;
  int charge = 0;
  double temporal_scale = 0.0;
  FaceCurrentSegment spatial{};

  // q integral (1-tau) W.dx and q integral tau W.dx.
  MatchedFaceFlux spatial_start{};
  MatchedFaceFlux spatial_end{};

  // q integral Lambda_i(x(tau)) d tau on temporal site-links.
  std::vector<double> temporal_charge;

  int temporal_support = 0;
  double spatial_split_residual = 0.0;
  double temporal_partition_residual = 0.0;
  double split_continuity_start_residual = 0.0;
  double split_continuity_end_residual = 0.0;
  double locality_residual = 0.0;
  bool valid = false;

  int index(int x, int y, int z) const;
};

/** Auxiliary connection on one time slab.
 *
 * A_start and A_end occupy the primal-link / positive-face slot. Phi occupies
 * sites and represents the scalar potential over a slab of length
 * temporal_scale.  The selected field representatives are
 *
 *   E = -(A_end-A_start)/temporal_scale - G Phi,
 *   B = C^T A.
 */
struct DualGaugePotentialSlab {
  int L = 0;
  double temporal_scale = 0.0;
  MatchedFaceFlux A_start{};
  MatchedFaceFlux A_end{};
  std::vector<double> Phi;

  explicit DualGaugePotentialSlab(int size = 0,
                                  double time_scale = 1.0);
  int index(int x, int y, int z) const;
};

struct SpacetimeGaugeCouplingResult {
  bool valid = false;
  double coupling = 0.0;
  double interaction_action = 0.0;
  double transformed_action = 0.0;
  double action_shift = 0.0;
  double endpoint_shift = 0.0;
  double gauge_endpoint_residual = 0.0;
  double curl_gradient_residual = 0.0;
  double electric_invariance_residual = 0.0;
  double magnetic_invariance_residual = 0.0;
  MatchedFaceFlux electric{};
  MatchedEdgeField magnetic_start{};
  MatchedEdgeField magnetic_end{};
};

/// Analytic straight-worldline deposits.  The spatial segment is exactly the
/// FTD-0478 FaceCurrentSegment; no coordinate route is introduced.
SpacetimeWorldlineCurrent make_spacetime_worldline_current(
    int L,
    Coord start_anchor,
    const Vec3& start_remainder,
    Coord end_anchor,
    const Vec3& end_remainder,
    int charge,
    double temporal_scale);

/// Forward coboundary G=-D^T in the existing positive-face array layout.
MatchedFaceFlux matched_forward_gradient(
    int L, const std::vector<double>& site_scalar);

/// Build the selected gauge-invariant field representatives of a slab.
MatchedFaceFlux slab_electric_field(const DualGaugePotentialSlab& slab);

/// Apply A^n -> A^n+G chi^n and
/// Phi -> Phi-(chi^1-chi^0)/temporal_scale.
DualGaugePotentialSlab gauge_transform_slab(
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end);

/// Evaluate S_int and its exact finite gauge transformation.  The endpoint
/// term is g(<rho_end,chi_end>-<rho_start,chi_start>).
SpacetimeGaugeCouplingResult evaluate_spacetime_gauge_coupling(
    const SpacetimeWorldlineCurrent& current,
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end,
    double coupling = 1.0);

}  // namespace ftd::eft

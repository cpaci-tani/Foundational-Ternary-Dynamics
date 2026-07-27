#pragma once
/**
 * @file axial_face_hop_reciprocity.h
 * @brief Observer-only axial face-current transaction through one native
 *        remainder threshold (FTD-0497).
 *
 * The transaction closes exact face work, Gauss transport, and relativistic
 * discrete-gradient matter work.  It also measures, rather than hides, the
 * raw (site,remainder) inverse defect of the production +/-1 reanchoring map.
 */

#include "ftd/eft/face_current_segment.h"
#include "ftd/eft/matched_gauss_transport.h"

namespace ftd::eft {

struct AxialFaceHopInput {
  MatchedFaceFlux electric_before;
  Coord site{};
  Vec3 remainder{};
  Vec3 momentum_before{};
  double dressing_before = 0.0;
  int axis = 0;
  int charge = 0;
  double coupling = 1.0;
  double dt = 1.0;
  double rest_energy = 0.511;
  double causal_speed = 0.57735026918962576451;
  double initial_momentum_guess = 0.0;
  bool use_initial_guess = false;
  int max_iterations = 512;
  double solver_tolerance = 1e-15;
};

struct AxialFaceHopStep {
  bool transaction_valid = false;
  bool converged = false;
  bool uniqueness_certified = false;
  bool hopped = false;
  bool strict_discrete_inverse = false;
  bool preimage_collision = false;
  int iterations = 0;
  int axis = 0;
  int hop_direction = 0;
  int charge = 0;
  Coord site_before{};
  Coord site_after{};
  Coord inverse_site{};
  Vec3 remainder_before{};
  Vec3 remainder_after{};
  Vec3 inverse_remainder{};
  Vec3 momentum_before{};
  Vec3 momentum_after{};
  Vec3 displacement{};

  FaceCurrentSegment current;
  FaceCurrentSegment inverse_current;
  MatchedFaceFlux electric_before;
  MatchedFaceFlux electric_midpoint;
  MatchedFaceFlux electric_after;
  double dressing_before = 0.0;
  double dressing_after = 0.0;

  double coupling = 0.0;
  double dt = 0.0;
  double rest_energy = 0.0;
  double causal_speed = 0.0;
  double path_averaged_field = 0.0;
  double contraction_bound = 0.0;
  double uniform_field_residual = 0.0;
  double fixed_point_residual = 0.0;
  double impulse_residual = 0.0;
  double displacement_residual = 0.0;
  double field_work = 0.0;
  double matter_work_residual = 0.0;
  double total_energy_residual = 0.0;
  double continuity_residual = 0.0;
  double relative_gauss_residual = 0.0;
  double locality_residual = 0.0;
  double speed = 0.0;
  double causal_excess = 0.0;
  double physical_inverse_residual = 0.0;
  double shape_inverse_residual = 0.0;
  double field_inverse_residual = 0.0;
  double momentum_inverse_residual = 0.0;
  double raw_remainder_inverse_residual = 0.0;
  int raw_anchor_inverse_mismatch = 0;
  double preimage_shape_residual = 0.0;
  double preimage_output_residual = 0.0;
};

/// Conservative uniform-pre-field contraction bound for an axial segment
/// crossing at most one integer plane.
double axial_face_hop_contraction_bound(
    double coupling,
    double dt,
    double rest_energy,
    double causal_speed);

/// Solve one observer transaction.  transaction_valid covers the exact
/// algebraic gates; strict_discrete_inverse is intentionally reported as a
/// separate frozen-ontology gate.
AxialFaceHopStep solve_axial_face_hop_step(
    const AxialFaceHopInput& input);

}  // namespace ftd::eft

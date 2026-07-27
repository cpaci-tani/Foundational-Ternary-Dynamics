#pragma once
/**
 * @file coupled_matched_face_transaction.h
 * @brief Observer-only coupled matter/matched-field transaction (FTD-0479).
 *
 * This sidecar closes one finite straight-segment charge history against the
 * matched face-E / edge-B update and the production particle dispersion
 *
 *   E(p)^2 = E_REST^2 + C_SPEED^2 |p|^2.
 *
 * The nonlinear endpoint is solved locally.  Its chord velocity
 *
 *   v_bar = C_SPEED^2 (p_0+p_1)/(E_0+E_1)
 *
 * is the exact discrete gradient of E(p).  FaceCurrentSegment supplies the
 * same fractional endpoint density and integrated face current to Gauss and
 * to the midpoint-work pairing.  No RenderBridge state or production toggle
 * is read or changed.
 *
 * Honest scope: the current-compatible electric path gather is fixed by the
 * face-current pairing only along components with nonzero displacement.  A
 * zero-displacement component has zero current work and therefore leaves its
 * transverse electric force underdetermined; the implementation reports the
 * selected midpoint representative.  The collocated edge-B path gather is
 * likewise selected.  Neither uniqueness statement has been derived from a
 * discrete gauge action.
 */

#include "ftd/eft/face_current_segment.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <vector>

namespace ftd::eft {

struct MatchedMatterPoint {
  Coord anchor{};
  Vec3 remainder{};
  Vec3 momentum{};
};

struct CoupledMatchedFaceState {
  MatchedFaceFlux electric;
  MatchedEdgeField magnetic_half;
  MatchedMatterPoint matter{};

  explicit CoupledMatchedFaceState(int size = 0)
      : electric(size), magnetic_half(size) {}
};

struct CoupledMatchedFaceOptions {
  double wave_speed = C_SPEED;
  double dt = 1.0;
  double gate_tolerance = 1e-12;
  double solve_tolerance = 2e-14;
  double finite_difference_scale = 2e-7;
  int max_iterations = 48;
  bool infer_inverse = true;
};

struct LocalImplicitSolveDiagnostics {
  bool attempted = false;
  bool converged = false;
  int iterations = 0;
  int rejected_steps = 0;
  double residual = 0.0;
  double step_residual = 0.0;
  double minimum_abs_jacobian_determinant = 0.0;
};

struct CoupledMatchedFaceInverseDiagnostics {
  bool explicit_available = false;
  bool inferred_attempted = false;
  bool inferred_converged = false;
  int inferred_iterations = 0;
  double explicit_residual = 0.0;
  double inferred_solve_residual = 0.0;
  double inferred_state_residual = 0.0;
};

struct CoupledMatchedFaceTransaction {
  bool valid = false;
  bool gates_pass = false;
  bool electric_transverse_rule_underderived = true;
  bool magnetic_rule_underderived = true;
  int charge = 0;
  FaceFluxNormalization normalization{};
  double interaction_scale = 0.0;

  CoupledMatchedFaceState before;
  CoupledMatchedFaceState after;
  FaceCurrentSegment segment;
  LocalImplicitSolveDiagnostics solve;
  CoupledMatchedFaceInverseDiagnostics inverse;

  Vec3 displacement{};
  Vec3 discrete_gradient_velocity{};
  Vec3 electric_path_average{};
  Vec3 magnetic_path_average{};
  Vec3 electric_impulse{};
  Vec3 magnetic_impulse{};
  Vec3 total_impulse{};

  double particle_energy_before = 0.0;
  double particle_energy_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double current_midpoint_work = 0.0;
  double magnetic_work = 0.0;

  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double force_residual = 0.0;
  double discrete_gradient_residual = 0.0;
  double work_residual = 0.0;
  double field_work_residual = 0.0;
  double total_energy_residual = 0.0;
  /// Maximum of endpoint kinematics and the exact current/force work
  /// pairing.  This is a discrete work-momentum covariance diagnostic, not a
  /// claim of continuum Lorentz covariance.
  double covariance_residual = 0.0;
  double causal_speed_excess = 0.0;
};

// Plan-facing name retained as a stable analysis interface.  The longer
// implementation name documents the matched staggered field representation.
using CommonActionStepResult = CoupledMatchedFaceTransaction;

/**
 * Advance one observer-only coupled transaction.
 *
 * `stationary_density` is a fractional site density left unchanged during
 * the event (normally the distant countercharge required by a periodic
 * computational window).  It must have L^3 entries.  The moving density is
 * generated losslessly from the supplied matter endpoint.
 */
CoupledMatchedFaceTransaction solve_coupled_matched_face_transaction(
    const CoupledMatchedFaceState& before,
    int charge,
    const std::vector<double>& stationary_density,
    const CoupledMatchedFaceOptions& options = {});

/// Fractional Gauss residual using the same backward face divergence as the
/// matched complex.  This overload deliberately does not round site density.
double max_fractional_gauss_residual(
    const MatchedFaceFlux& electric,
    const std::vector<double>& density);

}  // namespace ftd::eft

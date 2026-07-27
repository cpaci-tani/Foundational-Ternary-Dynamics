#pragma once
/**
 * @file site_ontic_atomic_reciprocal_hop.h
 * @brief Locked observer-only Gate R0 atomic hop candidate (FTD-0599).
 *
 * This is an analysis object.  It does not alter RenderBridge, production
 * phases, toggles, scenarios, or the persistent Voxel layout.
 */

#include "ftd/eft/common_moore_worldline_action.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <string>
#include <vector>

namespace ftd::eft {

struct SiteOnticAtomicState {
  int L = 0;
  Coord site{};
  Vec3 remainder{};   // canonical half-open chart: [0,1)^3
  Vec3 velocity{};    // persistent production variable
  int polarity = 0;
  std::vector<Vec3> flux;      // J
  std::vector<Vec3> wave;      // W
};

struct SiteOnticRootDiagnostics {
  int admitted_roots = 0;
  int starts_attempted = 0;
  int converged_starts = 0;
  int maximum_iterations = 0;
  double residual = 0.0;
  double jacobian_condition = 0.0;
  bool interval_certified = false;
  bool unique = false;
};

struct SiteOnticAtomicStepResult {
  bool evaluated = false;
  bool algebraically_valid = false;
  bool one_event_gates_pass = false;
  std::string failure_gate;

  SiteOnticAtomicState before;
  SiteOnticAtomicState after;
  SiteOnticRootDiagnostics forward_root;

  MooreSpacetimeCurrent deposited;
  std::vector<Vec3> source;
  Vec3 momentum_before{};
  Vec3 momentum_after{};
  Vec3 matter_impulse{};
  Vec3 field_momentum_before{};
  Vec3 field_momentum_after{};
  Coord site_shift{};

  double continuity_residual = 0.0;
  double source_replay_residual = 0.0;
  double field_update_residual = 0.0;
  double recoil_residual = 0.0;
  double kinematic_residual = 0.0;
  double causal_speed_excess = 0.0;
  double locality_residual = 0.0;
  double energy_relative_residual = 0.0;
  double work_relative_residual = 0.0;
  double total_energy_before = 0.0;
  double total_energy_after = 0.0;
  double particle_energy_before = 0.0;
  double particle_energy_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double interaction_energy_before = 0.0;
  double interaction_energy_after = 0.0;
  double matter_work = 0.0;
};

struct SiteOnticAtomicCampaignResult {
  bool valid = false;
  bool one_event_passed = false;
  bool repeated_campaign_run = false;
  std::string verdict;
  std::string decisive_arm;
  int arms_attempted = 0;
  int arms_passed = 0;
  SiteOnticAtomicStepResult decisive_result;
};

/// Construct the locked zero-mean native static dressing KJ=-G_C grad(rho).
SiteOnticAtomicState make_site_ontic_dressed_state(
    int L, int polarity, Coord site, const Vec3& remainder,
    const Vec3& velocity);

/// Solve one forward R0 transaction.  The independent interval certificate
/// is supplied by scripts/proofs/proof_site_ontic_atomic_reciprocal_hop.py;
/// this numerical object therefore reports but does not manufacture it.
SiteOnticAtomicStepResult solve_site_ontic_atomic_reciprocal_hop(
    const SiteOnticAtomicState& before);

/// Execute the preregistered deterministic one-event ordering until the first
/// conjunctive counterexample.  Repeated arms are not run after such a failure.
SiteOnticAtomicCampaignResult
analyze_site_ontic_atomic_reciprocal_hop();

}  // namespace ftd::eft

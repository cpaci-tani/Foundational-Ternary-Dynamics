#pragma once
/**
 * @file knot_legendre_branch.h
 * @brief Incident-cell branch census at a manifested lattice knot (FTD-0491).
 */

#include "ftd/eft/discrete_legendre_worldline.h"

#include <array>

namespace ftd::eft {

struct KnotLegendreBranch {
  bool sign_consistent = false;
  bool valid = false;
  Coord incident_sign{};
  Vec3 local_electric{};
  Vec3 analytic_momentum{};
  Vec3 displacement{};
  Vec3 endpoint{};
  double initial_kinetic_residual = 0.0;
  double analytic_momentum_residual = 0.0;
  double gauge_kinetic_residual = 0.0;
};

struct KnotLegendreBranchResult {
  bool valid = false;
  int L = 0;
  int polarity = 0;
  Coord knot{};
  Vec3 external_bias{};
  double epsilon = 0.0;
  double gauss_residual = 0.0;
  double bias_divergence_residual = 0.0;
  int sign_consistent_count = 0;
  int solved_branch_count = 0;
  double worst_initial_kinetic_residual = 0.0;
  double worst_analytic_momentum_residual = 0.0;
  double worst_gauge_kinetic_residual = 0.0;
  double displacement_orbit_residual = 0.0;
  std::array<KnotLegendreBranch, 8> branches{};
};

KnotLegendreBranchResult analyze_knot_legendre_branches(
    int L,
    Coord knot,
    int polarity,
    double epsilon,
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double coupling,
    const Vec3& external_bias = {});

}  // namespace ftd::eft

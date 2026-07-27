#pragma once
/**
 * @file boundary_collision_resolution.h
 * @brief Observer-only boundary-collision capacity/range/phase trilemma
 *        (FTD-0505).
 */

#include "ftd/eft/ternary_collision_vertex.h"

namespace ftd::eft {

struct BoundaryCollisionResolution {
  bool valid = false;
  Vec3 center{};
  Vec3 unit_direction{};
  double half_separation = 0.0;
  double speed = 0.0;
  double dt = 0.0;
  double collision_time = 0.0;
  double collision_time_residual = 0.0;
  int minimum_charge_alphabet_symbols = 0;
  int minimum_auxiliary_occupancy_bits = 0;
  TernaryCapacityResult endpoint_capacity{};
};

/// Analyze the exactly tick-boundary symmetric collision fixture.
BoundaryCollisionResolution analyze_boundary_collision_resolution(
    const Vec3& center,
    const Vec3& direction,
    double half_separation,
    double speed,
    double dt,
    int charge,
    double tolerance = 1e-12);

struct SameTickSeparatedOutputAttempt {
  bool valid = false;
  double output_distance = 0.0;
  double output_speed = 0.0;
  double collision_time = 0.0;
  double required_total_time = 0.0;
  double temporal_causal_defect = 0.0;
  bool same_tick_causal = false;
};

/// Time needed to move a positive distance after a boundary collision.
SameTickSeparatedOutputAttempt analyze_same_tick_separated_output(
    double half_separation,
    double incoming_speed,
    double dt,
    double output_distance,
    double output_speed,
    double c_speed,
    double tolerance = 1e-12);

struct PrecontactExclusionResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  double exclusion_radius = 0.0;
  double contact_time = 0.0;
  double remaining_time = 0.0;
  Vec3 left_endpoint{};
  Vec3 right_endpoint{};
  double endpoint_separation = 0.0;
  double energy_residual = 0.0;
  double momentum_residual = 0.0;
  double charge_residual = 0.0;
  double causal_residual = 0.0;
  double continuity_residual = 0.0;
  double reversal_residual = 0.0;
  PiecewiseCurrentSignature current{};
};

/// Selected hard-core reflection at center +/- radius*direction.
PrecontactExclusionResult analyze_precontact_exclusion(
    int L,
    const Vec3& center,
    const Vec3& direction,
    double half_separation,
    double speed,
    double dt,
    double exclusion_radius,
    int charge,
    double rest_energy,
    double c_speed,
    double tolerance = 1e-12);

/// Maximum componentwise difference across density and oriented current.
double collision_signature_difference(
    const PiecewiseCurrentSignature& lhs,
    const PiecewiseCurrentSignature& rhs);

struct CollisionTimingShiftResult {
  bool valid = false;
  double delta = 0.0;
  double baseline_speed = 0.0;
  double early_speed = 0.0;
  double late_speed = 0.0;
  double baseline_pair_energy = 0.0;
  double early_pair_energy = 0.0;
  double late_pair_energy = 0.0;
  double early_energy_shift = 0.0;
  double late_energy_shift = 0.0;
  double minimum_absolute_energy_shift = 0.0;
  double early_causal_residual = 0.0;
};

/// Symmetric speed/energy change needed to move the collision to dt +/- delta
/// while retaining the registered initial separation.
CollisionTimingShiftResult analyze_collision_timing_shift(
    double half_separation,
    double dt,
    double delta,
    double rest_energy,
    double c_speed);

}  // namespace ftd::eft

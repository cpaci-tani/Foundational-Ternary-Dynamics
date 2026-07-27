#pragma once
/**
 * @file momentum_face_balance.h
 * @brief Exact componentwise momentum-continuity lift of oriented face
 *        current (FTD-0514).
 */

#include "ftd/eft/constituent_stress_moment.h"

#include <array>
#include <vector>

namespace ftd::eft {

using SiteVectorField = std::array<std::vector<double>, 3>;
using TensorFaceField = std::array<SiteVectorField, 3>;
using TensorRows3 = std::array<Vec3, 3>;

struct MomentumWorldlineBalance {
  bool valid = false;
  int L = 0;
  Vec3 start_position{};
  Vec3 end_position{};
  Vec3 displacement{};
  Vec3 momentum{};
  PiecewiseCurrentSignature scalar_transport{};
  SiteVectorField momentum_before{};
  SiteVectorField momentum_after{};
  TensorFaceField tensor_face_flux{};
  TensorRows3 integrated_flux_moment{};
  TensorRows3 expected_outer_moment{};
  double local_balance_residual = 0.0;
  double global_momentum_residual = 0.0;
  double face_first_moment_residual = 0.0;
};

/// Lift one exact unit-carrier face-current segment by constant momentum.
MomentumWorldlineBalance make_momentum_worldline_balance(
    int L,
    const Vec3& start_position,
    const Vec3& end_position,
    const Vec3& momentum,
    double tolerance = 1e-12);

struct FreeMomentumTransportBalance {
  bool valid = false;
  double dt = 0.0;
  double rest_energy = 0.0;
  double c_speed = 0.0;
  Vec3 velocity{};
  ConstituentStressMoment stress{};
  MomentumWorldlineBalance worldline{};
  double stress_bridge_residual = 0.0;
  double causal_residual = 0.0;
};

/// Use the production dispersion to generate a free endpoint and verify that
/// integrated tensor face flux equals dt times kinetic stress.
FreeMomentumTransportBalance analyze_free_momentum_transport_balance(
    int L,
    const Vec3& start_position,
    const Vec3& momentum,
    double rest_energy,
    double c_speed,
    double dt = 1.0,
    double tolerance = 1e-12);

struct CollisionMomentumFaceBalance {
  bool valid = false;
  int L = 0;
  int polarity = 0;
  Coord chart_direction{};
  Vec3 collision_position{};
  ConstituentRelativeCollisionResult collision{};
  MomentumWorldlineBalance first_incoming{};
  MomentumWorldlineBalance second_incoming{};
  MomentumWorldlineBalance first_outgoing{};
  MomentumWorldlineBalance second_outgoing{};
  SiteVectorField aggregate_momentum_before{};
  SiteVectorField aggregate_momentum_after{};
  TensorFaceField aggregate_tensor_flux{};
  SiteVectorField aggregate_impulse_source{};
  TensorRows3 integrated_flux_moment{};
  TensorRows3 expected_piecewise_outer_moment{};
  double individual_segment_residual = 0.0;
  double constituent_impulse_residual = 0.0;
  double aggregate_impulse_source_l1 = 0.0;
  double individual_impulse_source_l1 = 0.0;
  double aggregate_local_balance_residual = 0.0;
  double aggregate_global_momentum_residual = 0.0;
  double energy_residual = 0.0;
  double tensor_moment_residual = 0.0;
  double reversal_endpoint_residual = 0.0;
  double reversal_tensor_flux_residual = 0.0;
  double reversal_impulse_source_residual = 0.0;
};

/// Compose two incoming segments, a selected FTD-0512 internal impulse pair,
/// and two outgoing segments into one exact local momentum balance.
CollisionMomentumFaceBalance analyze_collision_momentum_face_balance(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double segment_distance = 0.25,
    double tolerance = 1e-12);

}  // namespace ftd::eft

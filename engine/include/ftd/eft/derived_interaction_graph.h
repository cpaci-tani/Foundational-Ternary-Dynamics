#pragma once
/**
 * @file derived_interaction_graph.h
 * @brief Observer-only reversible derived-topology pair transaction (FTD-0721).
 *
 * Connectivity is computed from instantaneous constituent positions.  It is
 * not stored as an ontic edge or event-history bit.  The v1 solver is limited
 * to the equal-mass, zero-COM, collinear, non-crossing pair sector.
 */

#include "ftd/voxel.h"

namespace ftd::eft {

struct RelationalPairState {
  Vec3 first_position{};
  Vec3 second_position{};
  Vec3 first_momentum{};
  Vec3 second_momentum{};
  int first_polarity = +1;
  int second_polarity = -1;
};

struct DerivedInteractionGraphOptions {
  double dt = 0.25;
  double rest_energy = E_REST;
  double speed = C_SPEED;
  double well_depth = 0.01;
  double cutoff_distance_squared = 1.5;
  double solve_tolerance = 1e-14;
  double gate_tolerance = 1e-12;
  int max_iterations = 80;
};

struct DerivedInteractionGraphStep {
  bool valid = false;
  bool gates_pass = false;
  bool converged = false;
  bool edge_before = false;
  bool edge_after = false;
  bool graph_changed = false;
  int iterations = 0;
  RelationalPairState earlier{};
  RelationalPairState later{};
  Vec3 first_velocity{};
  Vec3 second_velocity{};
  Vec3 first_impulse{};
  Vec3 second_impulse{};
  double separation_before = 0.0;
  double separation_after = 0.0;
  double scalar_momentum_before = 0.0;
  double scalar_momentum_after = 0.0;
  double kinetic_energy_before = 0.0;
  double kinetic_energy_after = 0.0;
  double potential_energy_before = 0.0;
  double potential_energy_after = 0.0;
  double total_energy_before = 0.0;
  double total_energy_after = 0.0;
  double root_residual = 0.0;
  double energy_residual = 0.0;
  double momentum_residual = 0.0;
  double impulse_balance_residual = 0.0;
  double kinematic_residual = 0.0;
  double causal_speed_excess = 0.0;
};

double derived_interaction_potential(
    double distance_squared,
    const DerivedInteractionGraphOptions& options = {});

double derived_interaction_potential_derivative(
    double distance_squared,
    const DerivedInteractionGraphOptions& options = {});

bool derived_interaction_edge(
    const RelationalPairState& state,
    const DerivedInteractionGraphOptions& options = {});

RelationalPairState make_relational_pair_state(
    const Vec3& center, const Vec3& direction, double separation,
    double inward_momentum, int first_polarity = +1,
    int second_polarity = -1);

DerivedInteractionGraphStep solve_derived_interaction_graph_step(
    const RelationalPairState& earlier,
    const DerivedInteractionGraphOptions& options = {});

double relational_pair_state_max_difference(
    const RelationalPairState& lhs, const RelationalPairState& rhs);

}  // namespace ftd::eft

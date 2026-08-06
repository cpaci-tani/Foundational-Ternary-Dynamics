#include "ftd/eft/derived_interaction_graph.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x),std::abs(value.y),std::abs(value.z)});
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

double kinetic_energy(double momentum,
                      const DerivedInteractionGraphOptions& options) {
  const double c2 = options.speed*options.speed;
  return std::sqrt(options.rest_energy*options.rest_energy
      +c2*momentum*momentum);
}

double discrete_velocity(double p0, double p1,
                         const DerivedInteractionGraphOptions& options) {
  const double denominator = kinetic_energy(p0,options)
      +kinetic_energy(p1,options);
  return options.speed*options.speed*(p0+p1)/denominator;
}

double divided_potential_gradient(
    double d0, double d1,
    const DerivedInteractionGraphOptions& options) {
  if (std::abs(d1-d0) > 1e-13*std::max({1.0,std::abs(d0),std::abs(d1)}))
    return (derived_interaction_potential(d1,options)
        -derived_interaction_potential(d0,options))/(d1-d0);
  return derived_interaction_potential_derivative(0.5*(d0+d1),options);
}

bool valid_options(const DerivedInteractionGraphOptions& options) {
  return std::isfinite(options.dt) && options.dt != 0.0
      && options.rest_energy > 0.0 && std::isfinite(options.rest_energy)
      && options.speed > 0.0 && std::isfinite(options.speed)
      && options.well_depth > 0.0 && std::isfinite(options.well_depth)
      && options.cutoff_distance_squared == 1.5
      && options.solve_tolerance > 0.0
      && options.gate_tolerance > 0.0
      && options.max_iterations > 0;
}

}  // namespace

double derived_interaction_potential(
    double distance_squared,
    const DerivedInteractionGraphOptions& options) {
  if (!(distance_squared >= 0.0) || !std::isfinite(distance_squared)
      || !valid_options(options))
    return std::numeric_limits<double>::quiet_NaN();
  if (distance_squared >= options.cutoff_distance_squared) return 0.0;
  const double a = distance_squared-1.5;
  return -16.0*options.well_depth*a*a*(distance_squared-0.75);
}

double derived_interaction_potential_derivative(
    double distance_squared,
    const DerivedInteractionGraphOptions& options) {
  if (!(distance_squared >= 0.0) || !std::isfinite(distance_squared)
      || !valid_options(options))
    return std::numeric_limits<double>::quiet_NaN();
  if (distance_squared >= options.cutoff_distance_squared) return 0.0;
  const double a = distance_squared-1.5;
  return -16.0*options.well_depth
      *(2.0*a*(distance_squared-0.75)+a*a);
}

bool derived_interaction_edge(
    const RelationalPairState& state,
    const DerivedInteractionGraphOptions& options) {
  const Vec3 delta = state.second_position-state.first_position;
  return finite(delta) && delta.mag2() < options.cutoff_distance_squared;
}

RelationalPairState make_relational_pair_state(
    const Vec3& center, const Vec3& direction, double separation,
    double inward_momentum, int first_polarity, int second_polarity) {
  RelationalPairState result;
  const double norm = direction.mag();
  if (!finite(center) || !finite(direction) || !(norm > 0.0)
      || !(separation > 0.0) || !std::isfinite(separation)
      || !std::isfinite(inward_momentum)) return result;
  const Vec3 unit = direction*(1.0/norm);
  result.first_position = center-unit*(0.5*separation);
  result.second_position = center+unit*(0.5*separation);
  result.first_momentum = unit*inward_momentum;
  result.second_momentum = unit*(-inward_momentum);
  result.first_polarity = first_polarity;
  result.second_polarity = second_polarity;
  return result;
}

DerivedInteractionGraphStep solve_derived_interaction_graph_step(
    const RelationalPairState& earlier,
    const DerivedInteractionGraphOptions& options) {
  DerivedInteractionGraphStep result;
  result.earlier = earlier;
  if (!valid_options(options)
      || (earlier.first_polarity != -1 && earlier.first_polarity != +1)
      || earlier.second_polarity != -earlier.first_polarity
      || !finite(earlier.first_position) || !finite(earlier.second_position)
      || !finite(earlier.first_momentum) || !finite(earlier.second_momentum))
    return result;

  const Vec3 delta = earlier.second_position-earlier.first_position;
  const double r0 = delta.mag();
  if (!(r0 > 0.0) || !std::isfinite(r0)) return result;
  const Vec3 unit = delta*(1.0/r0);
  const Vec3 center = (earlier.first_position+earlier.second_position)*0.5;
  const double p0 = earlier.first_momentum.dot(unit);
  const Vec3 transverse = earlier.first_momentum-unit*p0;
  const double input_tolerance = 10.0*options.gate_tolerance;
  if (max_abs(earlier.first_momentum+earlier.second_momentum)
          > input_tolerance
      || max_abs(transverse) > input_tolerance) return result;

  const double d0 = r0*r0;
  const auto residual = [&](double p1) {
    const double velocity = discrete_velocity(p0,p1,options);
    const double r1 = r0-2.0*options.dt*velocity;
    const double d1 = r1*r1;
    const double gradient = divided_potential_gradient(d0,d1,options);
    return p1-p0-options.dt*gradient*(r0+r1);
  };

  double p1 = p0;
  double current = residual(p1);
  for (int iteration = 0; iteration <= options.max_iterations; ++iteration) {
    result.iterations = iteration;
    result.root_residual = std::abs(current);
    if (result.root_residual <= options.solve_tolerance) {
      result.converged = true;
      break;
    }
    if (iteration == options.max_iterations) break;
    const double h = 1e-7*std::max(1.0,std::abs(p1));
    const double derivative = (residual(p1+h)-residual(p1-h))/(2.0*h);
    if (!(std::abs(derivative) > 1e-12) || !std::isfinite(derivative)) break;
    const double step = -current/derivative;
    bool accepted = false;
    double scale = 1.0;
    for (int line = 0; line < 24; ++line) {
      const double trial = p1+scale*step;
      const double trial_residual = residual(trial);
      if (std::isfinite(trial_residual)
          && std::abs(trial_residual) < std::abs(current)) {
        p1 = trial;
        current = trial_residual;
        accepted = true;
        break;
      }
      scale *= 0.5;
    }
    if (!accepted) break;
  }
  if (!result.converged) return result;

  const double velocity = discrete_velocity(p0,p1,options);
  const double r1 = r0-2.0*options.dt*velocity;
  if (!(r1 > 0.0) || !std::isfinite(r1)) return result;
  result.later = make_relational_pair_state(
      center,unit,r1,p1,earlier.first_polarity,earlier.second_polarity);
  result.first_velocity = unit*velocity;
  result.second_velocity = result.first_velocity*(-1.0);
  result.first_impulse = result.later.first_momentum-earlier.first_momentum;
  result.second_impulse = result.later.second_momentum-earlier.second_momentum;
  result.separation_before = r0;
  result.separation_after = r1;
  result.scalar_momentum_before = p0;
  result.scalar_momentum_after = p1;
  result.edge_before = derived_interaction_edge(earlier,options);
  result.edge_after = derived_interaction_edge(result.later,options);
  result.graph_changed = result.edge_before != result.edge_after;
  result.kinetic_energy_before = 2.0*kinetic_energy(p0,options);
  result.kinetic_energy_after = 2.0*kinetic_energy(p1,options);
  result.potential_energy_before = derived_interaction_potential(d0,options);
  result.potential_energy_after = derived_interaction_potential(r1*r1,options);
  result.total_energy_before = result.kinetic_energy_before
      +result.potential_energy_before;
  result.total_energy_after = result.kinetic_energy_after
      +result.potential_energy_after;
  result.energy_residual = std::abs(
      result.total_energy_after-result.total_energy_before);
  result.momentum_residual = max_abs(
      result.later.first_momentum+result.later.second_momentum
      -earlier.first_momentum-earlier.second_momentum);
  result.impulse_balance_residual = max_abs(
      result.first_impulse+result.second_impulse);
  result.kinematic_residual = std::max(
      max_abs(result.later.first_position-earlier.first_position
          -result.first_velocity*options.dt),
      max_abs(result.later.second_position-earlier.second_position
          -result.second_velocity*options.dt));
  result.causal_speed_excess = std::max(0.0,std::abs(velocity)-options.speed);
  result.root_residual = std::abs(residual(p1));
  result.valid = finite(result.later.first_position)
      && finite(result.later.second_position)
      && finite(result.later.first_momentum)
      && finite(result.later.second_momentum);
  result.gates_pass = result.valid && result.converged
      && result.root_residual < 1e-13
      && result.energy_residual < options.gate_tolerance
      && result.momentum_residual < options.gate_tolerance
      && result.impulse_balance_residual < options.gate_tolerance
      && result.kinematic_residual < options.gate_tolerance
      && result.causal_speed_excess <= options.gate_tolerance;
  return result;
}

double relational_pair_state_max_difference(
    const RelationalPairState& lhs, const RelationalPairState& rhs) {
  if (lhs.first_polarity != rhs.first_polarity
      || lhs.second_polarity != rhs.second_polarity)
    return std::numeric_limits<double>::infinity();
  return std::max({max_abs(lhs.first_position-rhs.first_position),
      max_abs(lhs.second_position-rhs.second_position),
      max_abs(lhs.first_momentum-rhs.first_momentum),
      max_abs(lhs.second_momentum-rhs.second_momentum)});
}

}  // namespace ftd::eft

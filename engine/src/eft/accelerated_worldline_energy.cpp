#include "ftd/eft/accelerated_worldline_energy.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

long double energy(long double momentum,
                   long double rest_energy,
                   long double c_speed) {
  return std::sqrt(rest_energy * rest_energy
      + c_speed * c_speed * momentum * momentum);
}

long double velocity(long double momentum,
                     long double rest_energy,
                     long double c_speed) {
  return c_speed * c_speed * momentum
      / energy(momentum, rest_energy, c_speed);
}

long double secant_velocity(long double midpoint_momentum,
                            long double half_impulse,
                            long double rest_energy,
                            long double c_speed) {
  const long double scale = std::max(
      1.0L, std::abs(midpoint_momentum));
  if (std::abs(half_impulse)
      <= 64.0L * std::numeric_limits<long double>::epsilon() * scale) {
    return velocity(midpoint_momentum, rest_energy, c_speed);
  }
  return (energy(midpoint_momentum + half_impulse,
                 rest_energy, c_speed)
      - energy(midpoint_momentum - half_impulse,
               rest_energy, c_speed)) / (2.0L * half_impulse);
}

long double trajectory(long double tau,
                       long double midpoint_momentum,
                       long double half_impulse,
                       long double rest_energy,
                       long double c_speed,
                       long double temporal_scale) {
  const long double scale = std::max(
      1.0L, std::abs(midpoint_momentum));
  if (std::abs(half_impulse)
      <= 64.0L * std::numeric_limits<long double>::epsilon() * scale) {
    return temporal_scale * tau
        * velocity(midpoint_momentum, rest_energy, c_speed);
  }
  const long double momentum = midpoint_momentum
      - half_impulse + 2.0L * half_impulse * tau;
  return temporal_scale * (
      energy(momentum, rest_energy, c_speed)
      - energy(midpoint_momentum - half_impulse,
               rest_energy, c_speed)) / (2.0L * half_impulse);
}

bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

}  // namespace

AcceleratedWorldlineEnergyResult evaluate_accelerated_worldline_energy(
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double midpoint_momentum,
    double half_impulse,
    const Vec3& raw_direction) {
  AcceleratedWorldlineEnergyResult result;
  result.rest_energy = rest_energy;
  result.c_speed = c_speed;
  result.temporal_scale = temporal_scale;
  result.midpoint_momentum = midpoint_momentum;
  result.half_impulse = half_impulse;
  if (!(rest_energy > 0.0) || !std::isfinite(rest_energy)
      || !(c_speed > 0.0) || !std::isfinite(c_speed)
      || !(temporal_scale > 0.0) || !std::isfinite(temporal_scale)
      || !std::isfinite(midpoint_momentum)
      || !std::isfinite(half_impulse) || !finite(raw_direction)
      || !(raw_direction.mag2() > 0.0)) return result;
  result.direction = raw_direction * (1.0 / std::sqrt(raw_direction.mag2()));

  const long double M = rest_energy;
  const long double c = c_speed;
  const long double h = temporal_scale;
  const long double p = midpoint_momentum;
  const long double a = half_impulse;
  const long double p0 = p - a;
  const long double p1 = p + a;
  const long double H0 = energy(p0, M, c);
  const long double Hm = energy(p, M, c);
  const long double H1 = energy(p1, M, c);
  const long double vm = velocity(p, M, c);
  const long double vs = secant_velocity(p, a, M, c);
  const long double d_mid = h * vm;
  const long double d_exact = h * vs;
  const long double delta_h = H1 - H0;
  const long double work_mid = 2.0L * a * vm;
  const long double work_exact = 2.0L * a * vs;
  const long double defect_mid = delta_h - work_mid;
  const long double defect_exact = delta_h - work_exact;

  result.momentum_before = static_cast<double>(p0);
  result.momentum_after = static_cast<double>(p1);
  result.energy_before = static_cast<double>(H0);
  result.energy_midpoint = static_cast<double>(Hm);
  result.energy_after = static_cast<double>(H1);
  result.midpoint_velocity = static_cast<double>(vm);
  result.secant_velocity = static_cast<double>(vs);
  result.midpoint_displacement = static_cast<double>(d_mid);
  result.exact_displacement = static_cast<double>(d_exact);
  result.energy_change = static_cast<double>(delta_h);
  result.midpoint_work = static_cast<double>(work_mid);
  result.exact_work = static_cast<double>(work_exact);
  result.midpoint_work_defect = static_cast<double>(defect_mid);
  result.exact_work_defect = static_cast<double>(defect_exact);
  result.defect_identity_residual = static_cast<double>(std::abs(
      defect_mid - 2.0L * a * (vs - vm)));
  result.endpoint_residual = static_cast<double>(std::abs(
      trajectory(1.0L, p, a, M, c, h) - d_exact));

  constexpr long double tau = 0.371L;
  const long double pt = p - a + 2.0L * a * tau;
  const long double differentiated = h * velocity(pt, M, c);
  const long double analytic_derivative = h * c * c * pt
      / energy(pt, M, c);
  result.trajectory_derivative_residual = static_cast<double>(std::abs(
      differentiated - analytic_derivative));
  const long double exact_midpoint = trajectory(0.5L, p, a, M, c, h);
  result.midpoint_schedule_deviation = static_cast<double>(std::abs(
      exact_midpoint - 0.5L * d_exact));
  result.causal_speed_excess = static_cast<double>(std::max({
      0.0L,
      std::abs(velocity(p0, M, c)) - c,
      std::abs(velocity(p1, M, c)) - c,
      std::abs(vs) - c}));

  const long double reverse_velocity = secant_velocity(-p, a, M, c);
  result.reversal_velocity_residual = static_cast<double>(std::abs(
      reverse_velocity + vs));
  const long double forward_reversed = trajectory(
      1.0L - tau, p, a, M, c, h);
  const long double reverse_position = d_exact + trajectory(
      tau, -p, a, M, c, h);
  result.reversal_trajectory_residual = static_cast<double>(std::abs(
      reverse_position - forward_reversed));
  result.leading_cubic_term = static_cast<double>(
      -c * c * c * c * M * M * p
      / (Hm * Hm * Hm * Hm * Hm) * a * a * a);

  result.valid = std::isfinite(result.energy_before)
      && std::isfinite(result.energy_after)
      && std::isfinite(result.midpoint_velocity)
      && std::isfinite(result.secant_velocity)
      && std::isfinite(result.midpoint_displacement)
      && std::isfinite(result.exact_displacement)
      && std::isfinite(result.midpoint_work_defect)
      && std::isfinite(result.exact_work_defect)
      && std::isfinite(result.midpoint_schedule_deviation)
      && std::isfinite(result.leading_cubic_term);
  return result;
}

}  // namespace ftd::eft

#pragma once
/**
 * Raw-lattice causal kinematics — FTD-0402 source of truth.
 *
 * Stored particle velocity is always u in nodes/tick.  C_SPEED is therefore
 * explicit in every dimensionless speed and energy-momentum expression:
 *
 *   beta^2 = |u|^2 / C_SPEED^2
 *   f      = 1 - latency^2
 *   B      = beta^2 + latency^2 < 1
 *   dτ/dt  = sqrt(max(1 - B, 0))
 *
 * This implements the selected clock/bandwidth axiom.  It is not a substrate
 * derivation of special or general relativity.
 */

#include "constants.h"

#ifndef __CUDACC__
#include <cmath>
#endif

#ifdef __CUDACC__
#define FTD_CAUSAL_HD __host__ __device__ __forceinline__
#else
#define FTD_CAUSAL_HD inline
#endif

namespace ftd {

inline constexpr double CAUSAL_SENTINEL = 1.0e30;

FTD_CAUSAL_HD double causal_sqrt(double x) {
#ifdef __CUDACC__
  return sqrt(x);
#else
  return std::sqrt(x);
#endif
}

FTD_CAUSAL_HD bool causal_finite_nonnegative(double x) {
  // The comparison rejects NaN; the upper bound rejects infinities without
  // relying on host/device-specific isfinite overloads.
  return x >= 0.0 && x <= 1.7976931348623157e308;
}

FTD_CAUSAL_HD double lapse_f(double latency) {
  const double l2 = latency * latency;
  if (!causal_finite_nonnegative(l2)) return 0.0;
  return 1.0 - l2;
}

FTD_CAUSAL_HD double raw_beta2(double raw_speed2) {
  if (!causal_finite_nonnegative(raw_speed2)) return CAUSAL_SENTINEL;
  return raw_speed2 / (C_SPEED * C_SPEED);
}

FTD_CAUSAL_HD double causal_budget(double latency, double raw_speed2) {
  const double l2 = latency * latency;
  const double beta2 = raw_beta2(raw_speed2);
  if (!causal_finite_nonnegative(l2) || beta2 >= CAUSAL_SENTINEL)
    return CAUSAL_SENTINEL;
  return beta2 + l2;
}

FTD_CAUSAL_HD double bandwidth_fraction(double latency, double raw_speed2) {
  const double f = lapse_f(latency);
  if (f <= 0.0) return CAUSAL_SENTINEL;
  const double beta2 = raw_beta2(raw_speed2);
  if (beta2 >= CAUSAL_SENTINEL) return CAUSAL_SENTINEL;
  return beta2 / f;
}

FTD_CAUSAL_HD double proper_time_rate(double latency, double raw_speed2) {
  const double remaining = 1.0 - causal_budget(latency, raw_speed2);
  return remaining > 0.0 ? causal_sqrt(remaining) : 0.0;
}

FTD_CAUSAL_HD double transport_gamma(double latency, double raw_speed2) {
  const double rate = proper_time_rate(latency, raw_speed2);
  return rate > 0.0 ? 1.0 / rate : CAUSAL_SENTINEL;
}

// Force integration reconstructs the incoming specific momentum q=gamma*u.
// A directly mutated boundary/invalid state is kept finite here and is
// diagnosed/projected at movement entry; normally evolved states never use
// the floor branch.
FTD_CAUSAL_HD double momentum_input_gamma(double latency, double raw_speed2) {
  double remaining = 1.0 - causal_budget(latency, raw_speed2);
  if (!(remaining > BANDWIDTH_FLOOR)) remaining = BANDWIDTH_FLOOR;
  return 1.0 / causal_sqrt(remaining);
}

// Maps specific momentum q=P/M back to raw velocity under the selected
// transport budget: u=q*C*sqrt(f/(C^2+|q|^2)).
FTD_CAUSAL_HD double specific_momentum_velocity_scale(double latency,
                                                      double q2) {
  const double f = lapse_f(latency);
  if (f <= 0.0 || !causal_finite_nonnegative(q2)) return 0.0;
  const double scale =
      C_SPEED * causal_sqrt(f / (C_SPEED * C_SPEED + q2));
  // The analytic map is strictly interior for every finite q.  At very large
  // q, floating-point rounding can erase that strict inequality and leave a
  // value numerically on B=1.  Apply the same diagnostic margin used by the
  // movement-entry repair only in that limiting case; ordinary q is unchanged.
  const double mapped_speed2 = q2 * scale * scale;
  const double interior_speed2 =
      C_SPEED * C_SPEED * f * (1.0 - BANDWIDTH_FLOOR);
  if (mapped_speed2 >= interior_speed2 && q2 > 0.0)
    return causal_sqrt(interior_speed2 / q2);
  return scale;
}

FTD_CAUSAL_HD double max_raw_speed(double latency) {
  const double f = lapse_f(latency);
  return f > 0.0 ? C_SPEED * causal_sqrt(f) : 0.0;
}

// Returns a multiplicative scale in [0,1].  Values at or beyond the open
// causal boundary are placed a BANDWIDTH_FLOOR fraction inside it.  A zero
// result tells the caller to assign a literal zero vector (important for
// non-finite components, where multiplying infinity by zero would make NaN).
FTD_CAUSAL_HD double movement_projection_scale(double latency,
                                                double raw_speed2) {
  const double f = lapse_f(latency);
  if (f <= 0.0 || !causal_finite_nonnegative(raw_speed2)) return 0.0;
  const double target2 = C_SPEED * C_SPEED * f * (1.0 - BANDWIDTH_FLOOR);
  if (raw_speed2 < target2) return 1.0;
  if (raw_speed2 == 0.0) return 1.0;
  return causal_sqrt(target2 / raw_speed2);
}

FTD_CAUSAL_HD double flat_gamma(double raw_speed2) {
  const double remaining = 1.0 - raw_beta2(raw_speed2);
  return remaining > 0.0 ? 1.0 / causal_sqrt(remaining) : CAUSAL_SENTINEL;
}

FTD_CAUSAL_HD double flat_particle_energy(double raw_speed2) {
  const double gamma = flat_gamma(raw_speed2);
  return gamma >= CAUSAL_SENTINEL ? CAUSAL_SENTINEL : gamma * E_REST;
}

FTD_CAUSAL_HD double flat_particle_kinetic_energy(double raw_speed2) {
  const double gamma = flat_gamma(raw_speed2);
  return gamma >= CAUSAL_SENTINEL ? CAUSAL_SENTINEL
                                  : (gamma - 1.0) * E_REST;
}

FTD_CAUSAL_HD double born_infeld_core(double latency, double raw_speed2) {
  return -E_REST * proper_time_rate(latency, raw_speed2);
}

FTD_CAUSAL_HD double born_infeld_hamiltonian(double latency,
                                             double raw_speed2) {
  const double rate = proper_time_rate(latency, raw_speed2);
  const double f = lapse_f(latency);
  return rate > 0.0 && f > 0.0 ? E_REST * f / rate : CAUSAL_SENTINEL;
}

} // namespace ftd

#undef FTD_CAUSAL_HD

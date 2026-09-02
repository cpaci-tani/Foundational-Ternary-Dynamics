#pragma once
/**
 * Per-node state for the FTD render-bridge simulation.
 *
 * Each voxel carries ternary state, flux vector, velocity,
 * latency, and proper-time accumulator.
 */

#include "constants.h"
#include "causal_kinematics.h"
#include <array>
#include <cmath>

namespace ftd {

struct Vec3 {
  double x = 0.0, y = 0.0, z = 0.0;

  Vec3() = default;
  Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

  double mag2() const { return x * x + y * y + z * z; }
  double mag() const { return std::sqrt(mag2()); }

  // Dot product
  double dot(const Vec3 &o) const { return x * o.x + y * o.y + z * o.z; }

  // Cross product (needed for Lorentz force F = α·s·(v × B))
  static Vec3 cross(const Vec3 &a, const Vec3 &b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
  }

  Vec3 operator+(const Vec3 &o) const { return {x + o.x, y + o.y, z + o.z}; }
  Vec3 operator-(const Vec3 &o) const { return {x - o.x, y - o.y, z - o.z}; }
  Vec3 operator*(double s) const { return {x * s, y * s, z * s}; }
  Vec3 &operator+=(const Vec3 &o) {
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
  }
  Vec3 &operator-=(const Vec3 &o) {
    x -= o.x;
    y -= o.y;
    z -= o.z;
    return *this;
  }
  Vec3 &operator*=(double s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }
};

// Per-particle force breakdown for UI rendering and diagnostics.
// Stored in a separate buffer (RenderBridge::force_diag_) to keep Voxel
// cache-friendly for field sweeps that don't need force decomposition.
struct ForceDiag {
  Vec3 f_coulomb;
  Vec3 f_strong;
  Vec3 f_magnetic;
  Vec3 f_gravity;
  Vec3 f_exchange; // Fermi exchange (Pauli) repulsion
};

struct Voxel {
  // Ternary state: -1, 0, +1
  int8_t state = 0;

  // Flux vector (continuous field)
  Vec3 flux;

  // Momentum conjugate to flux: p = ∂J/∂t, the leapfrog momentum variable.
  // lagrangian.h proves wave_vel is the field's Legendre momentum. The name
  // "wave_vel" is historical and is retained; the field holds momentum, not
  // a velocity.
  Vec3 wave_vel;

  // ---- Dual-substrate fields ----
  // Paper: "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026)
  // Active when TermToggles::dual_substrate = true.
  // Observable: flux = flux_L + flux_R (maintained automatically).
  // Chirality is NOT phi = flux_L - flux_R -- that difference is never
  // computed anywhere in the engine. The actual chirality observable is
  // chirality_density() below: a TRANSVERSE difference |psi_L|^2 - |psi_R|^2
  // (projected perpendicular to the local velocity). No matter/antimatter
  // reading of it is asserted in code.
  Vec3 flux_L;     // Left substrate flux
  Vec3 flux_R;     // Right substrate flux
  Vec3 wave_vel_L; // Left substrate wave velocity
  Vec3 wave_vel_R; // Right substrate wave velocity

  // Chirality density: chi = |psi_L|^2 - |psi_R|^2
  // where psi_X is the complexified transverse component.
  // Replaces div(J) sign for manifestation polarity in dual-substrate mode.
  // Noether Audit (2026-06-02): upgraded to be coordinate-independent by projecting
  // perpendicular to the local velocity vector e_L when the particle is moving.
  double chirality_density() const {
    double speed2 = velocity.mag2();
    if (speed2 > 1e-12) {
      double speed = std::sqrt(speed2);
      Vec3 e_L(velocity.x / speed, velocity.y / speed, velocity.z / speed);
      double JL_dot = flux_L.dot(e_L);
      double JR_dot = flux_R.dot(e_L);
      double psiL2 = flux_L.mag2() - JL_dot * JL_dot;
      double psiR2 = flux_R.mag2() - JR_dot * JR_dot;
      return psiL2 - psiR2;
    } else {
      // If stationary, fall back to the legacy z-axis projection for backwards compatibility
      // with existing flavor and spin-statistics unit tests.
      double psiL2 = flux_L.x * flux_L.x + flux_L.y * flux_L.y;
      double psiR2 = flux_R.x * flux_R.x + flux_R.y * flux_R.y;
      return psiL2 - psiR2;
    }
  }

  // Lattice velocity (nodes per G*-tick)
  Vec3 velocity;

  // Sub-lattice position remainder
  Vec3 remainder;

  // Gravitational well depth (NOT the potential itself). The SOR solver
  // solves ∇²φ = 4πG(ρ−ρ̄) for φ (solve_latency_poisson_cpu in
  // poisson_solvers.cpp), subtracts the box mean of φ, and sets
  // latency = sqrt(max(−φ, 0)) — the square root of a mean-subtracted,
  // box-relative well depth. latency does NOT itself satisfy that Poisson
  // equation; φ does. Because of the mean subtraction, under-dense regions
  // are pinned to exactly zero rather than a negative "anti-well". It enters
  // the dynamics only as latency² (lapse f = 1−L², causal budget
  // B = β²+L² in causal_kinematics.h); "latency" never denotes a time delay
  // anywhere in the engine.
  // L ∈ [0, 0.999) — clamped below 1 to prevent horizon singularity.
  double latency = 0.0;

  // Proper time accumulator: dτ/dt = √max(1 - |u|²/C_SPEED² - L², 0).
  // Accumulated each tick for manifested particles when latency_field is ON.
  double tau = 0.0;

  // FTD-0271 (A5): de Broglie clock phase φ, advanced as dφ = ω₀·dτ when the
  // de_broglie_clock toggle is ON. FTD-0402 normalizes raw velocity to
  // C_SPEED in the selected clock/bandwidth axiom. This is an implementation
  // contract, not a substrate theorem of physical covariance. Read-only
  // diagnostic; NOT mixed into the golden state hash.
  double phase = 0.0;

  // Is this voxel part of a bound structure?
  bool locked = false;

  // Persistent particle identity (monotonically increasing, assigned at
  // genesis) Transferred during movement, cleared on evaporation/annihilation.
  // -1 = no particle.
  int32_t particle_id = -1;

  // Pair-production partner provenance bookkeeping. Particles born from the
  // same pair-production event share the same pair_id; -1 = none. Written
  // at injection, genesis, and movement (carried with the particle);
  // nothing in Scale 0 dynamics branches on it. It is read only for
  // provenance (CSV/VTK/telemetry export) and by Scale 1's particle-engine
  // cleanup pass (drops a stale reference once its partner is gone) — not
  // an entanglement link, since no Scale 0 physics reads or enforces any
  // correlation between paired particles.
  int pair_id = -1;

  // Spin-statistics fields (from DERIV_SPIN_STATISTICS_BRIDGE)
  // Spin: sign of the largest-magnitude component of curl(J) at genesis
  // (manifest_at() in phase_write.cpp), with a coin-flip fallback when the
  // curl is negligible. +1/-1 label the two curl-handedness classes, 0 = no
  // spin (void/boson). Its only dynamical consumer is the same-spin Fermi
  // exchange (Pauli) repulsion term in phase_forces.cpp: it never
  // precesses, never couples to a magnetic field, and never enters angular
  // momentum. (The earlier "ℤ₂ from lemniscate topology, 720° periodicity"
  // description has no code correlate.)
  int8_t spin = 0;

  // Color charge: Z/3Z from Lemniscate-Alpha's 3-lobe structure.
  //   0 = colorless, 1 = red, 2 = green, 3 = blue
  // Assigned at genesis from dominant flux axis (3 spatial dims → 3 colors).
  int8_t color = 0;

  // Flavor state for weak field interactions
  // 0 = none, 1 = e, 2 = mu, 3 = tau
  //
  // LOAD-BEARING despite looking dormant (revision 4.2 / 0.11 evidence —
  // an audit claimed "never read"; that was false). Live consumers:
  //   - GPU weak-field AUTO-ACTIVATION: gpu_engine.cu:746 gates the weak
  //     substrate kernels on `flavor != 0` (refresh_weak_field_active_from_host)
  //   - GPU SoA marshalling: gpu_buffers.cu uploads/downloads it
  //   - VTK research export: vtk_export.cpp writes it as a particle scalar
  // It is NOT in any golden-hash fold (golden-invisible) but IS
  // GPU-behavior-relevant. Do not remove or repurpose without a weak-field
  // activation redesign.
  int8_t flavor = 0;

  // Larmor radiation input: the RAW FORCE magnitude |F| (EM + gravity +
  // Lorentz; colour excluded — see the BH-F3 note at the assignment site in
  // phase_forces.cpp), recorded from the previous tick. This equals the
  // acceleration magnitude only at mass = 1 (the engine's convention). Its
  // sole consumer, the Larmor damping term in phase_write.cpp
  // (K_LARMOR * accel_mag^2), is therefore really F^2 = (mass*a)^2, not a^2
  // — it carries an extra factor of mass^2 relative to the true
  // acceleration-squared it is meant to model.
  double accel_mag = 0.0;

  // ---- Strong Substrate Field ----
  // Stella Octangula vertex-propagated gluonic field
  Vec3 flux_strong;
  Vec3 wave_vel_strong;

  // ---- Weak Substrate Field ----
  // Cuboctahedron edge-propagated weak field
  Vec3 flux_weak;
  Vec3 wave_vel_weak;

  // ---- Derived quantities ----

  double density() const { return flux.mag(); }

  double speed() const { return velocity.mag(); }

  // Fraction of the latency-selected transport allowance consumed by motion.
  // It reaches one at |u|=C_SPEED*sqrt(1-L²).
  double bandwidth_used() const {
    return bandwidth_fraction(latency, velocity.mag2());
  }

  double causal_budget() const {
    return ::ftd::causal_budget(latency, velocity.mag2());
  }

  double gamma_ftd() const {
    return transport_gamma(latency, velocity.mag2());
  }

  double born_infeld_core() const {
    return ::ftd::born_infeld_core(latency, velocity.mag2());
  }
};

} // namespace ftd

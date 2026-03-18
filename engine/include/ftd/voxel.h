#pragma once
/**
 * Per-node state for the FTD render-bridge simulation.
 *
 * Each voxel carries ternary state, flux vector, velocity,
 * latency, and proper-time accumulator.
 */

#include "constants.h"
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

  // Wave velocity (for flux propagation)
  Vec3 wave_vel;

  // ---- Dual-substrate fields ----
  // Paper: "The Algebraic Identity of Two Substrates" (Montanez & Claude, 2026)
  // Active when TermToggles::dual_substrate = true.
  // Observable: flux = flux_L + flux_R (maintained automatically).
  // Chirality: phi = flux_L - flux_R (encodes matter/antimatter).
  Vec3 flux_L;     // Left substrate flux
  Vec3 flux_R;     // Right substrate flux
  Vec3 wave_vel_L; // Left substrate wave velocity
  Vec3 wave_vel_R; // Right substrate wave velocity

  // Chirality density: chi = |psi_L|^2 - |psi_R|^2
  // where psi_X = J_Xx + i*J_Xy (complexified transverse component).
  // Replaces div(J) sign for manifestation polarity in dual-substrate mode.
  double chirality_density() const {
    double psiL2 = flux_L.x * flux_L.x + flux_L.y * flux_L.y;
    double psiR2 = flux_R.x * flux_R.x + flux_R.y * flux_R.y;
    return psiL2 - psiR2;
  }

  // Lattice velocity (nodes per G*-tick)
  Vec3 velocity;

  // Sub-lattice position remainder
  Vec3 remainder;

  // Gravitational potential: L = sqrt(r_s/r) from Poisson equation ∇²L = 4πGρ.
  // When latency_field toggle is ON, evolves via SOR Poisson solver.
  // L ∈ [0, 0.999) — clamped below 1 to prevent horizon singularity.
  double latency = 0.0;

  // Proper time accumulator: dτ/dt = √(f² - v²)/f where f = 1 - L².
  // Accumulated each tick for manifested particles when latency_field is ON.
  double tau = 0.0;

  // Is this voxel part of a bound structure?
  bool locked = false;

  // Persistent particle identity (monotonically increasing, assigned at
  // genesis) Transferred during movement, cleared on evaporation/annihilation.
  // -1 = no particle.
  int32_t particle_id = -1;

  // Entanglement pair ID (pair production partner tracking)
  // Particles from the same pair production event share the same pair_id.
  // -1 = no entanglement partner.
  int pair_id = -1;

  // Spin-statistics fields (from DERIV_SPIN_STATISTICS_BRIDGE)
  // Spin: ℤ₂ from lemniscate topology (720° periodicity).
  //   +1 = spin-up, -1 = spin-down, 0 = no spin (void/boson)
  // Assigned at genesis from curl(J) dominant component.
  int8_t spin = 0;

  // Color charge: Z/3Z from Lemniscate-Alpha's 3-lobe structure.
  //   0 = colorless, 1 = red, 2 = green, 3 = blue
  // Assigned at genesis from dominant flux axis (3 spatial dims → 3 colors).
  int8_t color = 0;

  // Larmor radiation: acceleration magnitude from previous tick
  double accel_mag = 0.0;

  // ---- Derived quantities ----

  double density() const { return flux.mag(); }

  double speed() const { return velocity.mag(); }

  // Bandwidth used: v²/f when latency active, else v².
  // When latency_field is ON, the effective speed limit is f = 1 - L²,
  // so bandwidth = v²/f measures fraction of available bandwidth consumed.
  double bandwidth_used() const {
    double v2 = speed() * speed();
    if (latency == 0.0) return v2; // fast path — no gravitational potential
    double f = 1.0 - latency * latency;
    return (f > 0.0) ? v2 / f : 1.0;
  }

  // Generalized Lorentz factor: γ = √f / √(f² - v²) when latency active.
  // Reduces to standard 1/√(1-v²) when L=0.
  double gamma_ftd() const {
    if (latency == 0.0) {
      double bw = speed() * speed();
      if (bw >= 1.0) return 1e30;
      return 1.0 / std::sqrt(1.0 - bw);
    }
    double f = 1.0 - latency * latency;
    if (f <= 0.0) return 1e30;
    double v2 = speed() * speed();
    double arg = f * f - v2;
    if (arg <= 0.0) return 1e30;
    return std::sqrt(f) / std::sqrt(arg);
  }

  // Born-Infeld core: -K_B · √(f² - v²)/√f when latency active.
  // Reduces to standard -K_B·√(1-v²) when L=0.
  double born_infeld_core() const {
    if (latency == 0.0) {
      double bw = speed() * speed();
      if (bw >= 1.0) return 0.0;
      return -K_B * std::sqrt(1.0 - bw);
    }
    double f = 1.0 - latency * latency;
    if (f <= 0.0) return 0.0;
    double v2 = speed() * speed();
    double arg = f * f - v2;
    if (arg <= 0.0) return 0.0;
    return -K_B * std::sqrt(arg) / std::sqrt(f);
  }
};

} // namespace ftd

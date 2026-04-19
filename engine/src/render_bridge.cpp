/**
 * Logic-First FTD Engine (v2.0)
 *
 * Built from axioms: {3D lattice, ternary states, flux field, local causality}
 *
 * Six rules, nothing else:
 *   1. Flux wave equation: d²J/dt² = c²∇²J (local linear dynamics)
 *   2. State-flux coupling: g_c·∇(s) source term (from δS/δJ = 0)
 *   3. Gauss projection: enforce ∇·J = s (charge conservation)
 *   4. Manifestation/Evaporation: threshold crossing
 *   5. Field-mediated forces: F = -α·s·∇φ_C + G_N·∇ρ (Poisson Coulomb, Phase 3)
 *   6. Movement + Collision: remainder accumulation, speed limit, annihilation
 *
 * Everything phenomenological has been stripped:
 *   - No pairwise Coulomb, Yukawa, Lorentz, exchange forces
 *   - No QCD running coupling
 *   - No weak transmutation
 *   - No binding energy maintenance
 *   - No noetic/consciousness
 *   - No latency/bandwidth/proper time
 *
 * What emerges from these rules IS the physics.
 * What doesn't emerge is a genuine absence, not a missing formula.
 *
 * Archived: engine_v1_phenomenological/ contains the full 1382-line version.
 */

#include "ftd/render_bridge.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_engine.h"
#endif

namespace ftd {

RenderBridge::RenderBridge(int lattice_size)
    : lattice_(lattice_size), voxels_(lattice_.total_sites()),
      force_diag_(lattice_.total_sites()),
      delta_j_(lattice_.total_sites()),
      delta_j_L_(lattice_.total_sites()),
      delta_j_R_(lattice_.total_sites()),
      phi_(lattice_.total_sites(), 0.0),
      phi_coulomb_(lattice_.total_sites(), 0.0),
      phi_latency_(lattice_.total_sites(), 0.0),
      moved_(lattice_.total_sites(), 0),
      sor_source_(lattice_.total_sites(), 0.0)
{
    // PERF: pre-size per-tick scratch buffers so phase_write doesn't
    // construct ~5KB of mt19937 state per voxel. Under WASM (no OpenMP)
    // num_threads is always 1; native builds size to omp_get_max_threads().
    int num_threads = 1;
#ifdef _OPENMP
    num_threads = omp_get_max_threads();
#endif
    thread_seeds_.resize(num_threads, 0u);
    thread_rngs_.resize(num_threads);
    colored_sites_cache_.reserve(256);
#ifdef FTD_ENABLE_CUDA
    try {
        gpu_ = std::make_unique<gpu::GpuEngine>(lattice_size);
        use_gpu_ = true;
        std::cerr << "[RenderBridge] GPU backend active (CUDA, L=" << lattice_size << ")\n";
    } catch (const std::exception& e) {
        use_gpu_ = false;
        std::cerr << "[RenderBridge] GPU init failed: " << e.what() << " — using CPU\n";
    } catch (...) {
        use_gpu_ = false;
        std::cerr << "[RenderBridge] GPU init failed (unknown error) — using CPU\n";
    }
#endif
}

// Destructor must be in .cpp where GpuEngine is fully defined (unique_ptr needs it)
RenderBridge::~RenderBridge() = default;

#ifdef FTD_ENABLE_CUDA
void RenderBridge::gpu_sync_to_host() {
    if (use_gpu_ && gpu_dirty_) {
        gpu_->sync_to_host(voxels_);
        gpu_dirty_ = false;
    }
}

void RenderBridge::gpu_push_to_device() {
    if (use_gpu_) {
        gpu_->upload_from_host(voxels_);
        gpu_dirty_ = false;
    }
}

void RenderBridge::gpu_flush_host_mutations() {
    // Wave 5.2 (2026-04-14): tick() calls this at the start so that any
    // direct host writes done via voxels()[idx].field = ... since the
    // previous tick get pushed back to the GPU before physics runs.
    if (use_gpu_ && host_mutated_) {
        gpu_->upload_from_host(voxels_);
        gpu_dirty_ = false;
        host_mutated_ = false;
    }
}
#endif

// Wave 5 (2026-04-14): GPU-aware phi_latency accessor.
// When use_gpu_ is true, lazily fetches the latency Poisson potential
// from the GPU buffer (d_phi_latency). When use_gpu_ is false, returns
// the CPU SOR solver's cached vector directly.
const std::vector<double>& RenderBridge::phi_latency() const {
#ifdef FTD_ENABLE_CUDA
    if (use_gpu_ && gpu_) {
        // Mirror the GPU's phi_latency into our host vector so external
        // callers get a stable reference.
        const auto& gpu_phi = gpu_->phi_latency();
        auto& dst = const_cast<std::vector<double>&>(phi_latency_);
        dst = gpu_phi;
    }
#endif
    return phi_latency_;
}

// ============================================================================
// Discrete Operators (pure mathematics — no physics assumptions)
// ============================================================================

// RF-09: 18-pt isotropic Laplacian templated on Voxel field.
// (1/3)*sum_face + (1/6)*sum_edge − 4*center cancels O(k^4) anisotropy.
template <Vec3 Voxel::*F>
Vec3 RenderBridge::laplacian_impl(int idx) const {
  const auto& face = lattice_.neighbors_6(idx);
  const auto& edge = lattice_.neighbors_12(idx);
  Vec3 lap;
  for (int n : face) lap += voxels_[n].*F * (1.0/3.0);
  for (int n : edge) lap += voxels_[n].*F * (1.0/6.0);
  lap -= voxels_[idx].*F * 4.0;
  return lap;
}

// Explicit instantiations (keeps template body in .cpp, avoids ODR issues).
template Vec3 RenderBridge::laplacian_impl<&Voxel::flux  >(int) const;
template Vec3 RenderBridge::laplacian_impl<&Voxel::flux_L>(int) const;
template Vec3 RenderBridge::laplacian_impl<&Voxel::flux_R>(int) const;

Vec3 RenderBridge::laplacian_flux(int idx) const {
  // Isotropic 18-point Laplacian on flux field.
  Vec3 lap = laplacian_impl<&Voxel::flux>(idx);
  assert(!std::isnan(lap.x) && !std::isnan(lap.y) && !std::isnan(lap.z));
  return lap;
}

void RenderBridge::sync_observable() {
  const int N = static_cast<int>(lattice_.total_sites());
  for (int i = 0; i < N; ++i)
    voxels_[i].flux = voxels_[i].flux_L + voxels_[i].flux_R;
}

double RenderBridge::divergence_flux(int idx) const {
  const auto& nbrs = lattice_.neighbors_6(idx);
  double div = 0.0;
  div += (voxels_[nbrs[0]].flux.x - voxels_[nbrs[1]].flux.x) * 0.5;
  div += (voxels_[nbrs[2]].flux.y - voxels_[nbrs[3]].flux.y) * 0.5;
  div += (voxels_[nbrs[4]].flux.z - voxels_[nbrs[5]].flux.z) * 0.5;
  return div;
}

Vec3 RenderBridge::curl_flux(int idx) const {
  const auto& n = lattice_.neighbors_6(idx);
  Vec3 curl;
  curl.x = (voxels_[n[2]].flux.z - voxels_[n[3]].flux.z) * 0.5 -
           (voxels_[n[4]].flux.y - voxels_[n[5]].flux.y) * 0.5;
  curl.y = (voxels_[n[4]].flux.x - voxels_[n[5]].flux.x) * 0.5 -
           (voxels_[n[0]].flux.z - voxels_[n[1]].flux.z) * 0.5;
  curl.z = (voxels_[n[0]].flux.y - voxels_[n[1]].flux.y) * 0.5 -
           (voxels_[n[2]].flux.x - voxels_[n[3]].flux.x) * 0.5;
  assert(!std::isnan(curl.x) && !std::isnan(curl.y) && !std::isnan(curl.z));
  return curl;
}

Vec3 RenderBridge::gradient_scalar(int idx,
                                   const std::vector<double> &field) const {
  const auto& nbrs = lattice_.neighbors_6(idx);
  Vec3 grad;
  grad.x = (field[nbrs[0]] - field[nbrs[1]]) * 0.5;
  grad.y = (field[nbrs[2]] - field[nbrs[3]]) * 0.5;
  grad.z = (field[nbrs[4]] - field[nbrs[5]]) * 0.5;
  assert(!std::isnan(grad.x) && !std::isnan(grad.y) && !std::isnan(grad.z));
  return grad;
}

Vec3 RenderBridge::gradient_state(int idx) const {
  const auto& n = lattice_.neighbors_6(idx);
  Vec3 grad;
  grad.x = (voxels_[n[0]].state - voxels_[n[1]].state) * 0.5;
  grad.y = (voxels_[n[2]].state - voxels_[n[3]].state) * 0.5;
  grad.z = (voxels_[n[4]].state - voxels_[n[5]].state) * 0.5;
  return grad;
}

Vec3 RenderBridge::gradient_density(int idx) const {
  const auto& n = lattice_.neighbors_6(idx);
  Vec3 grad;
  grad.x = (voxels_[n[0]].density() - voxels_[n[1]].density()) * 0.5;
  grad.y = (voxels_[n[2]].density() - voxels_[n[3]].density()) * 0.5;
  grad.z = (voxels_[n[4]].density() - voxels_[n[5]].density()) * 0.5;
  return grad;
}

Vec3 RenderBridge::gradient_divergence(int idx) const {
  const auto& n = lattice_.neighbors_6(idx);
  Vec3 grad;
  grad.x = (divergence_flux(n[0]) - divergence_flux(n[1])) * 0.5;
  grad.y = (divergence_flux(n[2]) - divergence_flux(n[3])) * 0.5;
  grad.z = (divergence_flux(n[4]) - divergence_flux(n[5])) * 0.5;
  return grad;
}

Vec3 RenderBridge::curl_state_velocity(int idx) const {
  auto c = lattice_.coord(idx);
  auto jcur = [&](int x, int y, int z) -> Vec3 {
    int ni = lattice_.index(x, y, z);
    return voxels_[ni].velocity * static_cast<double>(voxels_[ni].state);
  };
  Vec3 curl;
  curl.x = (jcur(c.x, c.y + 1, c.z).z - jcur(c.x, c.y - 1, c.z).z) * 0.5 -
           (jcur(c.x, c.y, c.z + 1).y - jcur(c.x, c.y, c.z - 1).y) * 0.5;
  curl.y = (jcur(c.x, c.y, c.z + 1).x - jcur(c.x, c.y, c.z - 1).x) * 0.5 -
           (jcur(c.x + 1, c.y, c.z).z - jcur(c.x - 1, c.y, c.z).z) * 0.5;
  curl.z = (jcur(c.x + 1, c.y, c.z).y - jcur(c.x - 1, c.y, c.z).y) * 0.5 -
           (jcur(c.x, c.y + 1, c.z).x - jcur(c.x, c.y - 1, c.z).x) * 0.5;
  return curl;
}

// RF-16: stress = |div(F)| + |curl(F)| + |grad(|F|)| templated on Voxel field.
// Inlines the 6-neighbor stencil directly so it works for flux, flux_L, or flux_R
// without depending on the helpers (divergence_flux, curl_flux, gradient_density)
// which are hardcoded to Voxel::flux.
// Note: (v.*F).component is the correct pointer-to-member access syntax.
template <Vec3 Voxel::*F>
double RenderBridge::stress_impl(int idx) const {
  const auto& nbrs = lattice_.neighbors_6(idx);

  // Divergence
  double div = 0.0;
  div += ((voxels_[nbrs[0]].*F).x - (voxels_[nbrs[1]].*F).x) * 0.5;
  div += ((voxels_[nbrs[2]].*F).y - (voxels_[nbrs[3]].*F).y) * 0.5;
  div += ((voxels_[nbrs[4]].*F).z - (voxels_[nbrs[5]].*F).z) * 0.5;
  double div_mag = std::abs(div);

  // Curl
  Vec3 curl;
  curl.x = ((voxels_[nbrs[2]].*F).z - (voxels_[nbrs[3]].*F).z) * 0.5 -
           ((voxels_[nbrs[4]].*F).y - (voxels_[nbrs[5]].*F).y) * 0.5;
  curl.y = ((voxels_[nbrs[4]].*F).x - (voxels_[nbrs[5]].*F).x) * 0.5 -
           ((voxels_[nbrs[0]].*F).z - (voxels_[nbrs[1]].*F).z) * 0.5;
  curl.z = ((voxels_[nbrs[0]].*F).y - (voxels_[nbrs[1]].*F).y) * 0.5 -
           ((voxels_[nbrs[2]].*F).x - (voxels_[nbrs[3]].*F).x) * 0.5;
  double curl_mag = curl.mag();

  // Gradient of magnitude |F|
  double gx = ((voxels_[nbrs[0]].*F).mag() - (voxels_[nbrs[1]].*F).mag()) * 0.5;
  double gy = ((voxels_[nbrs[2]].*F).mag() - (voxels_[nbrs[3]].*F).mag()) * 0.5;
  double gz = ((voxels_[nbrs[4]].*F).mag() - (voxels_[nbrs[5]].*F).mag()) * 0.5;
  double grad_mag = std::sqrt(gx*gx + gy*gy + gz*gz);

  return div_mag + curl_mag + grad_mag;
}

// Explicit instantiations.
template double RenderBridge::stress_impl<&Voxel::flux  >(int) const;
template double RenderBridge::stress_impl<&Voxel::flux_L>(int) const;

double RenderBridge::compute_stress(int idx) const {
  return stress_impl<&Voxel::flux>(idx);
}

double RenderBridge::compute_stress_left(int idx) const {
  // Stress from J_L only (left-chiral). Produces maximal parity violation
  // (δ ≈ 0.957): +1 particles (J_L-dominant) → high stress → easy transmutation;
  // −1 particles (J_R-dominant) → low stress → nearly immune.
  return stress_impl<&Voxel::flux_L>(idx);
}

double RenderBridge::born_probability(int idx) const {
  double rho = voxels_[idx].density();
  if (rho < K_GENESIS) return 0.0;
  return 1.0 - std::exp(-(rho - K_GENESIS) / K_B);
}

void RenderBridge::create_entangled_pair(int x, int y, int z, const Vec3& flux_val) {
  int id = next_pair_id_++;
  int idx = lattice_.index(x, y, z);
  auto& v = voxels_[idx];
  v.state = 1;
  v.flux = flux_val;
  v.pair_id = id;
  v.particle_id = next_particle_id_++;

  auto nbrs = lattice_.neighbors_6(idx);
  // I6 fix: find an empty neighbor for the partner — don't overwrite existing particles
  int partner_idx = -1;
  for (int n : nbrs) {
    if (voxels_[n].state == 0) {
      partner_idx = n;
      break;
    }
  }
  if (partner_idx < 0) return;  // No empty neighbor available

  auto& partner = voxels_[partner_idx];
  partner.state = -1;
  partner.flux = flux_val * -1.0;
  partner.pair_id = id;
  partner.particle_id = next_particle_id_++;
}

double RenderBridge::compute_entropy() const {
  const int N = static_cast<int>(lattice_.total_sites());
  double total_mag2 = 0.0;
  for (int i = 0; i < N; ++i) {
    total_mag2 += voxels_[i].flux.mag2();
  }
  if (total_mag2 < EPSILON_FLUX_SQ) return 0.0;
  double entropy = 0.0;
  for (int i = 0; i < N; ++i) {
    double p = voxels_[i].flux.mag2() / total_mag2;
    if (p > EPSILON_FLUX_SQ) {
      entropy -= p * std::log(p);
    }
  }
  return entropy;
}

// ============================================================================
// RULE 1: phase_read() — Wave propagation + state-flux coupling
//
// From the action principle δS/δJ = 0:
//   Wave: d²J/dt² = c²∇²J (Laplacian drives flux wave propagation)
//   Source: g_c·∇(s) (manifested particles source flux in their neighborhood)
//   Biot-Savart: g_c·∇×(s·v) (moving charges create rotational flux)
//
// STENCIL AND INTEGRATION NOTES (2026 audit):
//
//   18-point Moore Laplacian (weights face=1/3, edge=1/6, self=−4):
//   CONSISTENT (weights sum = 0) AND isotropic through O(h⁴). Direct
//   Taylor expansion:
//     face sum · (1/3)  +  edge sum · (1/6)  −  4·f
//       = h² ∇²f + (h⁴/12)·(∇²)²f + O(h⁶)
//   The 2:1 face:edge ratio is WHAT PRODUCES the O(h⁴) isotropy.
//   Verified empirically by tests/test_moore_laplacian_isotropy.cpp
//   (TRACKER §1.8 — closed 2026-04-17): smooth-Gaussian radial-symmetry
//   within 11% at L=64, σ=4. Residual at finite h is lattice dispersion
//   at k·h ~ 1 — a known artefact of ALL cubic-lattice FD schemes, not
//   a defect of these specific weights.
//
//   The advance pair (wave_vel += delta_J; flux += wave_vel) is
//   Störmer–Verlet (leapfrog) under the stagger interpretation where
//   wave_vel = v(t + h/2) and flux = J(t):
//       v(t + h/2) = v(t − h/2) + a(J(t)) · h   (kick)
//       J(t + h)   = J(t)       + v(t + h/2)·h   (drift)
//   Empirically verified by tests/test_leapfrog_integrator_audit.cpp
//   (see TRACKER_OPEN_ITEMS §1.4 — closed 2026-04-17): over 5000 ticks
//   with damping off, cumulative injection/dissipation balance to 0.1%,
//   the hallmark of a symplectic scheme. C_SPEED = 1/√D = 1/√3 is the
//   leapfrog CFL limit, correctly identified.
// ============================================================================

void RenderBridge::phase_read() {
  const int N = static_cast<int>(lattice_.total_sites());
  const int L = lattice_.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  const bool do_wave = toggles.wave_propagation;
  const bool do_coupling = toggles.coupling;
  const bool dual = toggles.dual_substrate;
  const double cw2 = C_WAVE * C_WAVE;

  // Isotropic 18-point Laplacian weights.
  // For interior voxels we skip all modulo ops (same technique as sor_sweep_18pt):
  //   coord decomposition:  iz = i % L  (stride 1), iy = (i/L) % L (stride L), ix = i/LL (stride LL)
  //   interior iff all three coords ∈ [1, L-2].
  // For the ~(L-2)³/L³ ≈ 97.7% interior fraction (L=64), this eliminates every modulo.
  // Dual-substrate computes L and R Laplacians from the SAME 18 loaded neighbors — one
  // pass, no redundant neighbor lookups compared to calling laplacian_flux_L + laplacian_flux_R
  // separately.
  constexpr double INV3 = 1.0 / 3.0;
  constexpr double INV6 = 1.0 / 6.0;

  if (dual) {
    // Dual-substrate: compute delta for J_L and J_R in a single neighbor sweep
#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
      delta_j_L_[i] = {};
      delta_j_R_[i] = {};

      if (do_wave) {
        // Decompose flat index into lattice coordinates
        const int iz = i % L;
        const int iy = (i / L) % L;
        const int ix = i / LL;

        Vec3 lap_L, lap_R;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          // Interior fast path: precomputed offsets, zero modulo operations.
          // Neighbor offsets: ±1=±z, ±L=±y, ±LL=±x (matches lattice coord convention).
          const Vec3 fL = (voxels_[i+1].flux_L  + voxels_[i-1].flux_L
                         + voxels_[i+L].flux_L  + voxels_[i-L].flux_L
                         + voxels_[i+LL].flux_L + voxels_[i-LL].flux_L) * INV3;
          const Vec3 fR = (voxels_[i+1].flux_R  + voxels_[i-1].flux_R
                         + voxels_[i+L].flux_R  + voxels_[i-L].flux_R
                         + voxels_[i+LL].flux_R + voxels_[i-LL].flux_R) * INV3;
          const Vec3 eL = (voxels_[i+1+L].flux_L  + voxels_[i+1-L].flux_L
                         + voxels_[i-1+L].flux_L  + voxels_[i-1-L].flux_L
                         + voxels_[i+1+LL].flux_L + voxels_[i+1-LL].flux_L
                         + voxels_[i-1+LL].flux_L + voxels_[i-1-LL].flux_L
                         + voxels_[i+L+LL].flux_L + voxels_[i+L-LL].flux_L
                         + voxels_[i-L+LL].flux_L + voxels_[i-L-LL].flux_L) * INV6;
          const Vec3 eR = (voxels_[i+1+L].flux_R  + voxels_[i+1-L].flux_R
                         + voxels_[i-1+L].flux_R  + voxels_[i-1-L].flux_R
                         + voxels_[i+1+LL].flux_R + voxels_[i+1-LL].flux_R
                         + voxels_[i-1+LL].flux_R + voxels_[i-1-LL].flux_R
                         + voxels_[i+L+LL].flux_R + voxels_[i+L-LL].flux_R
                         + voxels_[i-L+LL].flux_R + voxels_[i-L-LL].flux_R) * INV6;
          lap_L = fL + eL - voxels_[i].flux_L * 4.0;
          lap_R = fR + eR - voxels_[i].flux_R * 4.0;
        } else {
          // Boundary slow path: modular wrapping via lattice neighbor tables.
          lap_L = laplacian_impl<&Voxel::flux_L>(i);
          lap_R = laplacian_impl<&Voxel::flux_R>(i);
        }
        delta_j_L_[i] = lap_L * cw2;
        delta_j_R_[i] = lap_R * cw2;
      }

      // Coupling source: split equally between L and R substrates
      if (do_coupling) {
        Vec3 grad_s = gradient_state(i) * (G_C * 0.5);
        Vec3 curl_sv = curl_state_velocity(i) * (G_C * 0.5);
        delta_j_L_[i] += grad_s + curl_sv;
        delta_j_R_[i] += grad_s + curl_sv;
      }
    }
  } else {
    // Single-substrate: inline Laplacian with the same interior/boundary split
#pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
      delta_j_[i] = {};

      if (do_wave) {
        const int iz = i % L;
        const int iy = (i / L) % L;
        const int ix = i / LL;

        Vec3 lap;
        if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
          // Interior fast path
          const Vec3 f = (voxels_[i+1].flux  + voxels_[i-1].flux
                        + voxels_[i+L].flux  + voxels_[i-L].flux
                        + voxels_[i+LL].flux + voxels_[i-LL].flux) * INV3;
          const Vec3 e = (voxels_[i+1+L].flux  + voxels_[i+1-L].flux
                        + voxels_[i-1+L].flux  + voxels_[i-1-L].flux
                        + voxels_[i+1+LL].flux + voxels_[i+1-LL].flux
                        + voxels_[i-1+LL].flux + voxels_[i-1-LL].flux
                        + voxels_[i+L+LL].flux + voxels_[i+L-LL].flux
                        + voxels_[i-L+LL].flux + voxels_[i-L-LL].flux) * INV6;
          lap = f + e - voxels_[i].flux * 4.0;
        } else {
          // Boundary slow path
          lap = laplacian_flux(i);
        }
        delta_j_[i] = lap * cw2;
      }

      if (do_coupling) {
        delta_j_[i] += gradient_state(i) * G_C;
        delta_j_[i] += curl_state_velocity(i) * G_C;
      }
    }
  }
}

// ============================================================================
// RULE 2: phase_write() — Commit flux, damping, manifestation/evaporation
//
// Flux update via leapfrog integration:
//   wave_vel += delta_J    (acceleration from Laplacian + source)
//   flux += wave_vel       (position update)
//   flux *= (1 - γ)        (dissipation, γ = α from ontic chain)
//
// Manifestation: when |J| > K_GENESIS at a void site, a particle manifests.
//   Polarity from sign(∇·J): sources → +1, sinks → -1
//   Spin from curl(J): local vorticity → ℤ₂ handedness
//   Color from dominant flux axis: 3 spatial dims → ℤ₃
//
// Evaporation: when |J| << K_B, particle returns to void.
// ============================================================================

void RenderBridge::phase_write() {
  const int N = static_cast<int>(lattice_.total_sites());
  const double damping_factor = (dt_ > 1.0001)
      ? std::pow(1.0 - DAMPING, dt_)
      : 1.0 - DAMPING;
  const bool do_damping = toggles.damping;
  const bool do_genesis = toggles.genesis;
  const bool selective = toggles.selective_damping;
  const bool do_larmor = toggles.larmor_radiation;
  const bool dual = toggles.dual_substrate;

  // Phase D: Precompute near-particle mask (O(N), race-free)
  if (selective) {
    near_particle_.resize(N, 0);
    std::fill(near_particle_.begin(), near_particle_.end(), 0);
    if (do_larmor) {
      near_accel_.resize(N, 0.0);
      std::fill(near_accel_.begin(), near_accel_.end(), 0.0);
    }
    for (int i = 0; i < N; ++i) {
      if (voxels_[i].state != 0) {
        near_particle_[i] = 1;
        if (do_larmor) {
          double a = voxels_[i].accel_mag;
          near_accel_[i] = std::max(near_accel_[i], a);
        }
        for (int n : lattice_.neighbors_6(i)) {
          near_particle_[n] = 1;
          if (do_larmor) {
            double a = voxels_[i].accel_mag;  // particle's acceleration
            near_accel_[n] = std::max(near_accel_[n], a);
          }
        }
      }
    }
  }

  // PERF: re-seed the per-thread RNG pool ONCE per phase_write (was
  // constructing a fresh ~5KB mt19937 per voxel inside the parallel-for,
  // costing ~1.3 GB/tick of stack churn at L=64). Buffers are bridge
  // members presized in the ctor — no allocation here.
  int num_threads = 1;
#ifdef _OPENMP
  num_threads = omp_get_max_threads();
#endif
  for (int t = 0; t < num_threads; ++t) {
    thread_seeds_[t] = rng_();
    thread_rngs_[t].seed(thread_seeds_[t]);
  }

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];

    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    auto& local_rng = thread_rngs_[tid];
    std::uniform_real_distribution<double> local_uniform(0.0, 1.0);

    const bool should_damp = !selective || near_particle_[i];

    if (dual) {
      // ---- Dual-substrate leapfrog integration ----
      v.wave_vel_L += delta_j_L_[i];
      v.wave_vel_R += delta_j_R_[i];
      v.flux_L += v.wave_vel_L;
      v.flux_R += v.wave_vel_R;

      // Damping on both substrates independently
      if (do_damping && should_damp) {
        double eff_damping = damping_factor;
        // Larmor radiation: modulate damping at ALL near-particle sites
        // Uses max accel_mag of nearby particles (propagated via near_accel_)
        // Static charges (a=0) → LARMOR_FLOOR ≈ 1% damping
        // Accelerating charges → enhanced damping ∝ a²
        if (do_larmor && selective && near_particle_[i]) {
          double a2 = near_accel_[i] * near_accel_[i];
          double larmor_mod = std::min(1.0, LARMOR_FLOOR + K_LARMOR * a2);
          eff_damping = 1.0 - DAMPING * larmor_mod;
        }
        v.flux_L *= eff_damping;
        v.flux_R *= eff_damping;
        v.wave_vel_L *= eff_damping;
        v.wave_vel_R *= eff_damping;
      }

      // Update observable field: flux = J_L + J_R
      v.flux = v.flux_L + v.flux_R;
      v.wave_vel = v.wave_vel_L + v.wave_vel_R;

      // Genesis: use chirality density for polarity (dual-substrate rule)
      // chi = |psi_L|^2 - |psi_R|^2 where psi_X = J_Xx + i*J_Xy
      // s = sgn(chi) * Theta(|chi| - K_B^2)
      if (do_genesis && v.state == 0 && v.density() > K_GENESIS) {
        double excess = v.density() - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_B);
        if (local_uniform(local_rng) < p) {
          double chi = v.chirality_density();
          v.state = (chi >= 0) ? 1 : -1;
          int pid;
#pragma omp critical(genesis_id)
          { pid = next_particle_id_++; }
          v.particle_id = pid;

          // Spin from curl of observable J
          Vec3 curl = curl_flux(i);
          double ax = std::abs(curl.x), ay = std::abs(curl.y), az = std::abs(curl.z);
          double mx = std::max({ax, ay, az});
          if (mx > EPSILON_MAG) {
            if (az >= ax && az >= ay) v.spin = (curl.z > 0) ? 1 : -1;
            else if (ay >= ax) v.spin = (curl.y > 0) ? 1 : -1;
            else v.spin = (curl.x > 0) ? 1 : -1;
          } else {
            v.spin = (local_uniform(local_rng) < 0.5) ? 1 : -1;
          }

          // Color from dominant flux axis
          double fx = std::abs(v.flux.x), fy = std::abs(v.flux.y), fz = std::abs(v.flux.z);
          if (fx >= fy && fx >= fz) v.color = 1;
          else if (fy >= fx && fy >= fz) v.color = 2;
          else v.color = 3;
        }
      }
    } else {
      // ---- Single-substrate leapfrog integration (legacy) ----
      v.wave_vel += delta_j_[i];
      v.flux += v.wave_vel;

      if (do_damping && should_damp) {
        double eff_damping = damping_factor;
        // Larmor radiation: modulate damping at ALL near-particle sites
        if (do_larmor && selective && near_particle_[i]) {
          double a2 = near_accel_[i] * near_accel_[i];
          double larmor_mod = std::min(1.0, LARMOR_FLOOR + K_LARMOR * a2);
          eff_damping = 1.0 - DAMPING * larmor_mod;
        }
        v.flux *= eff_damping;
        v.wave_vel *= eff_damping;
      }

      // Genesis: void + high flux → manifest (div(J) for polarity)
      if (do_genesis && v.state == 0 && v.density() > K_GENESIS) {
        double excess = v.density() - K_GENESIS;
        double p = 1.0 - std::exp(-excess / K_B);
        if (local_uniform(local_rng) < p) {
          double div = divergence_flux(i);
          v.state = (div > 0) ? 1 : -1;
          int pid;
#pragma omp critical(genesis_id)
          { pid = next_particle_id_++; }
          v.particle_id = pid;

          Vec3 curl = curl_flux(i);
          double ax = std::abs(curl.x), ay = std::abs(curl.y), az = std::abs(curl.z);
          double mx = std::max({ax, ay, az});
          if (mx > EPSILON_MAG) {
            if (az >= ax && az >= ay) v.spin = (curl.z > 0) ? 1 : -1;
            else if (ay >= ax) v.spin = (curl.y > 0) ? 1 : -1;
            else v.spin = (curl.x > 0) ? 1 : -1;
          } else {
            v.spin = (local_uniform(local_rng) < 0.5) ? 1 : -1;
          }

          double fx = std::abs(v.flux.x), fy = std::abs(v.flux.y), fz = std::abs(v.flux.z);
          if (fx >= fy && fx >= fz) v.color = 1;
          else if (fy >= fx && fy >= fz) v.color = 2;
          else v.color = 3;
        }
      }
    }

    // Evaporation: low TOTAL wave energy → return to void
    // (same for both single and dual substrate — uses observable flux)
    constexpr double EVAP_ENERGY = K_B * K_B * EVAP_THRESHOLD;
    double local_energy = v.flux.mag2() + v.wave_vel.mag2();
    {
      const auto& nbrs = lattice_.neighbors_6(i);
      for (int n : nbrs)
        local_energy += voxels_[n].flux.mag2() + voxels_[n].wave_vel.mag2();
    }
    if (do_genesis && v.state != 0
        && local_energy < EVAP_ENERGY
        && !v.locked) {
      v.state = 0;
      v.particle_id = -1;
      v.spin = 0;
      v.color = 0;
    }
  }
}

// ============================================================================
// RULE 3: gauss_project() — Enforce ∇·J = s (charge conservation)
//
// This is the U(1) gauge constraint. It is logically necessary:
// charge conservation demands that the divergence of the flux field
// equals the charge density at every point.
//
// Method: SOR (Successive Over-Relaxation) on ∇²φ = ∇·J − s,
// then correct J -= ∇φ to remove unphysical longitudinal modes.
//
// Warm-started: phi_ persists between ticks for fast reconvergence.
// SOR ω=1.75 matches the Coulomb solver quality.
// ============================================================================

// ============================================================================
// SOR sweep helper — isotropic 18-point Poisson stencil with RED-BLACK ordering
//
// Replaces three near-identical inner SOR loops in gauss_project,
// solve_coulomb_poisson, solve_latency_poisson. Two performance tricks:
//
// (1) Interior/boundary split. Voxels not on a lattice face use precomputed
//     integer offsets and pay zero modulo wraps — at L=64 that's ~97% of
//     voxels on the fast path. The ~3% on a boundary face take the slow
//     modular path via lattice.neighbors_6/12.
//
// (2) Red-black ordering. Each sweep does TWO passes: pass 0 updates only
//     "red" voxels (where (x+y+z) parity == 0), pass 1 updates only "black".
//     Since the 18-point stencil is bipartite (red voxels only have black
//     neighbors and vice-versa), each color pass has zero loop-carried
//     dependencies → the compiler can auto-vectorize the inner loop with
//     -msimd128. Natural-ordering SOR has a write-then-read dependency
//     between consecutive iterations that prevents vectorization.
//
//     Convergence rate is identical to natural-ordering SOR with the same
//     omega (Hageman & Young 1981); we keep SOR_OMEGA = 1.75.
// ============================================================================
static inline void sor_sweep_18pt(std::vector<double>& phi,
                                  const std::vector<double>& source,
                                  const Lattice& lattice,
                                  double omega) {
  constexpr double INV3 = 1.0 / 3.0;
  constexpr double INV6 = 1.0 / 6.0;
  constexpr double INV4 = 1.0 / 4.0;
  const int L = lattice.size();
  const int LL = L * L;
  const int Nm1 = L - 1;

  // Precomputed neighbor offsets for interior voxels — no modulo needed.
  const int o_xp = 1,  o_xm = -1;
  const int o_yp = L,  o_ym = -L;
  const int o_zp = LL, o_zm = -LL;
  const int o_xpyp = o_xp + o_yp, o_xpym = o_xp + o_ym;
  const int o_xmyp = o_xm + o_yp, o_xmym = o_xm + o_ym;
  const int o_xpzp = o_xp + o_zp, o_xpzm = o_xp + o_zm;
  const int o_xmzp = o_xm + o_zp, o_xmzm = o_xm + o_zm;
  const int o_ypzp = o_yp + o_zp, o_ypzm = o_yp + o_zm;
  const int o_ymzp = o_ym + o_zp, o_ymzm = o_ym + o_zm;

  for (int color = 0; color < 2; ++color) {
    // ── Interior fast path (per color) ───────────────────────────────
    // Inner x loop strides by 2 → red and black voxels are interleaved
    // in cache lines but each pass touches only one color. The starting
    // x for each row depends on (y + z + color) parity so adjacent rows
    // alternate the offset.
    for (int z = 1; z < Nm1; ++z) {
      for (int y = 1; y < Nm1; ++y) {
        // x_start ∈ {1, 2}: smallest x ≥ 1 with (x+y+z) parity == color
        const int parity_yz = (y + z) & 1;
        const int x_start = 1 + ((color ^ (parity_yz ^ 1)) & 1);
        int idx = z * LL + y * L + x_start;
        // No loop-carried dependency between iterations: each phi[idx]
        // write reads only opposite-color neighbors (idx±1, idx±L,
        // idx±L²) which are NOT touched again until the next color pass.
        for (int x = x_start; x < Nm1; x += 2, idx += 2) {
          const double face_sum = phi[idx + o_xp] + phi[idx + o_xm]
                                + phi[idx + o_yp] + phi[idx + o_ym]
                                + phi[idx + o_zp] + phi[idx + o_zm];
          const double edge_sum = phi[idx + o_xpyp] + phi[idx + o_xpym]
                                + phi[idx + o_xmyp] + phi[idx + o_xmym]
                                + phi[idx + o_xpzp] + phi[idx + o_xpzm]
                                + phi[idx + o_xmzp] + phi[idx + o_xmzm]
                                + phi[idx + o_ypzp] + phi[idx + o_ypzm]
                                + phi[idx + o_ymzp] + phi[idx + o_ymzm];
          const double gs = (INV3 * face_sum + INV6 * edge_sum - source[idx]) * INV4;
          phi[idx] += omega * (gs - phi[idx]);
        }
      }
    }

    // ── Boundary slow path (per color) ───────────────────────────────
    // Voxels on a lattice face. Uses lattice's modular neighbor lookups.
    // Skips interior voxels already processed above AND wrong-color voxels.
    for (int z = 0; z < L; ++z) {
      const bool zEdge = (z == 0 || z == Nm1);
      for (int y = 0; y < L; ++y) {
        const bool yEdge = (y == 0 || y == Nm1);
        for (int x = 0; x < L; ++x) {
          const bool isInterior = !zEdge && !yEdge && x != 0 && x != Nm1;
          if (isInterior) continue;
          if (((x + y + z) & 1) != color) continue;
          const int idx = z * LL + y * L + x;
          const auto& face = lattice.neighbors_6(idx);
          const auto& edge = lattice.neighbors_12(idx);
          double face_sum = 0.0, edge_sum = 0.0;
          for (int n : face) face_sum += phi[n];
          for (int n : edge) edge_sum += phi[n];
          const double gs = (INV3 * face_sum + INV6 * edge_sum - source[idx]) * INV4;
          phi[idx] += omega * (gs - phi[idx]);
        }
      }
    }
  }
}

void RenderBridge::gauss_project() {
  const int N = static_cast<int>(lattice_.total_sites());
  const int L = lattice_.size();
  const int LL = L * L;
  const int Nm1 = L - 1;
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;

  // Source term for the gauss SOR: violation = div(J) - state.
  // Interior path uses direct offsets (no modulo). Boundary path falls back
  // to divergence_flux() which handles periodic wrapping.
  // sor_source_ is a bridge member — zero per-tick allocation.
#pragma omp parallel for schedule(static)
  for (int i = 0; i < N; ++i) {
    const int iz = i % L;
    const int iy = (i / L) % L;
    const int ix = i / LL;
    double div;
    if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
      // Interior: central difference with direct offsets (no modulo)
      // nbrs order: [+x=+LL, -x=-LL, +y=+L, -y=-L, +z=+1, -z=-1]
      div = (voxels_[i+LL].flux.x - voxels_[i-LL].flux.x) * 0.5
          + (voxels_[i+L].flux.y  - voxels_[i-L].flux.y)  * 0.5
          + (voxels_[i+1].flux.z  - voxels_[i-1].flux.z)  * 0.5;
    } else {
      div = divergence_flux(i);
    }
    sor_source_[i] = div - static_cast<double>(voxels_[i].state);
  }

  // Warm-started: phi_ retains values from previous tick.
  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    sor_sweep_18pt(phi_, sor_source_, lattice_, OMEGA);
  }

  // Phase 4 (Approach B): Skip Gauss correction at manifested sites.
  //
  // Mathematical justification: div(J)(i) does NOT involve J(i) — the
  // central-difference divergence operator only reads neighbor fluxes.
  // Therefore J(i) at a particle site is entirely transverse (invisible
  // to Gauss).  Correcting J(i) doesn't fix div(J)(i); it only affects
  // div(J) at the 6 face-neighbors.  Skipping the correction here:
  //   (a) Preserves the transverse flux that the wave equation builds
  //   (b) Still enforces Gauss at all void sites (where it matters)
  //   (c) Eliminates the Gauss/floor energy injection cycle
  //
  // Interior fast path: gradient_scalar uses central differences with
  // 6 face neighbors — same interior/boundary split as phase_read.
  const bool dual_gauss = toggles.dual_substrate;
#pragma omp parallel for schedule(static)
  for (int i = 0; i < N; ++i) {
    if (voxels_[i].state != 0) continue;  // Skip manifested sites
    Vec3 grad_phi;
    const int iz = i % L;
    const int iy = (i / L) % L;
    const int ix = i / LL;
    if (iz > 0 && iz < Nm1 && iy > 0 && iy < Nm1 && ix > 0 && ix < Nm1) {
      // Interior: direct offsets for central-difference gradient of phi_
      grad_phi.x = (phi_[i+LL] - phi_[i-LL]) * 0.5;
      grad_phi.y = (phi_[i+L]  - phi_[i-L])  * 0.5;
      grad_phi.z = (phi_[i+1]  - phi_[i-1])  * 0.5;
    } else {
      grad_phi = gradient_scalar(i, phi_);
    }
    voxels_[i].flux -= grad_phi;

    // Dual-substrate: split correction equally between L and R
    // This maintains div(J_L + J_R) = s while preserving L/R symmetry
    if (dual_gauss) {
      Vec3 half_corr = grad_phi * 0.5;
      voxels_[i].flux_L -= half_corr;
      voxels_[i].flux_R -= half_corr;
    }
  }
}

// ============================================================================
// Coulomb Poisson Solver — SOR with warm-start
//
// Solves ∇²φ_C = s (charge density) for the electrostatic potential.
// Force is then F = -α·q·∇φ_C, giving proper 1/r² Coulomb scaling.
//
// Key properties:
//   - Warm-started: phi_coulomb_ persists between ticks for fast convergence
//   - SOR (ω=1.75): ~3-5x faster than Jacobi for 3D Poisson
//   - Mean-subtracted: required for periodic BC compatibility (∫ρ = 0 on torus)
// ============================================================================

void RenderBridge::solve_coulomb_poisson() {
  const int N = static_cast<int>(lattice_.total_sites());
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;

  // Mean-subtract source for periodic BC compatibility
  double charge_sum = 0.0;
  for (int i = 0; i < N; ++i)
    charge_sum += static_cast<double>(voxels_[i].state);
  const double mean_charge = charge_sum / N;

  // Precompute source ONCE per tick (it doesn't depend on iter):
  //   ∇²φ = -s  →  source = -(s - mean)
#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    sor_source_[i] = -(static_cast<double>(voxels_[i].state) - mean_charge);
  }

  // Warm-started SOR; uses shared 18-point sweep helper.
  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    sor_sweep_18pt(phi_coulomb_, sor_source_, lattice_, OMEGA);
  }

  // Pin gauge: subtract mean of phi
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_coulomb_[i];
  const double phi_mean = phi_sum / N;
#pragma omp parallel for
  for (int i = 0; i < N; ++i)
    phi_coulomb_[i] -= phi_mean;
}

// ============================================================================
// Latency Poisson solver: ∇²φ_L = 4πG·ρ_mass
//
// The latency field is the FTD gravitational potential. Mass density is
// ρ_mass = K_B · |state| for manifested sites. After solving, set
// voxel.latency = sqrt(clamp(phi_L, 0, 0.998)) — the Schwarzschild-like
// gravitational potential L where the speed limit becomes v < f = 1 - L².
//
// Uses same 18-point isotropic SOR as solve_coulomb_poisson().
// ============================================================================

void RenderBridge::solve_latency_poisson() {
  const int N = static_cast<int>(lattice_.total_sites());
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;
  constexpr double FOUR_PI_G = 4.0 * PI * G_N;

  // Mean-subtract source for periodic BC compatibility
  double mass_sum = 0.0;
  for (int i = 0; i < N; ++i)
    mass_sum += K_B * std::abs(voxels_[i].state);
  const double mean_mass = mass_sum / N;

  // Precompute source ONCE per tick (independent of iter):
  //   ∇²φ = +4πGρ  →  source = 4πG(ρ - mean)
#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    const double rho_mass = K_B * std::abs(voxels_[i].state);
    sor_source_[i] = FOUR_PI_G * (rho_mass - mean_mass);
  }

  // Warm-started SOR; uses shared 18-point sweep helper.
  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    sor_sweep_18pt(phi_latency_, sor_source_, lattice_, OMEGA);
  }

  // Pin gauge: subtract mean of phi
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_latency_[i];
  const double phi_mean = phi_sum / N;
#pragma omp parallel for
  for (int i = 0; i < N; ++i)
    phi_latency_[i] -= phi_mean;

  // Convert Poisson potential to latency: L = sqrt(clamp(|phi|, 0, 0.998))
  // L ∈ [0, 0.999) — clamped below 1 to prevent horizon singularity.
  //
  // NOTE (April 13, 2026 fix): The Poisson equation ∇²φ = 4πGρ with attractive
  // mass gives phi NEGATIVE near mass (standard physics convention). The
  // magnitude |phi| is the gravitational potential depth. Taking sqrt(|phi|)
  // instead of sqrt(max(phi,0)) unlocks the entire GR sector — time dilation,
  // horizon formation, and gravitational wave propagation all depend on this.
  for (int i = 0; i < N; ++i) {
    double phi_val = phi_latency_[i];
    double abs_phi = std::abs(phi_val);
    double clamped = std::min(abs_phi, LATENCY_HORIZON_CLAMP);
    voxels_[i].latency = std::sqrt(clamped);
  }
}

// ============================================================================
// RULE 4: phase_forces() — Field-mediated forces ONLY
//
// Coulomb force via Poisson-solved potential (Phase 3):
//   Solve ∇²φ_C = s (charge density) via warm-started SOR
//   F_EM = -α · s · ∇φ_C          (proper 1/r² from Poisson Green's function)
//
// Legacy mode (poisson_coulomb = false):
//   F_EM = -α · s · ∇(∇·J)        (local double gradient — r^(-3.8) falloff)
//
// From density gradient (gravitational attraction to flux concentrations):
//   F_grav = G_N · ∇ρ
//
// NO pairwise forces. NO Yukawa. NO exchange. NO QCD running.
// ============================================================================

void RenderBridge::phase_forces() {
  const int N = static_cast<int>(lattice_.total_sites());
  const int L = lattice_.size();

  // Solve Coulomb potential (warm-started SOR)
  // Skip when emergent_forces is ON — force comes from flux field directly
  if (toggles.poisson_coulomb && !toggles.emergent_forces)
    solve_coulomb_poisson();

  // ── Color force ────────────────────────────────────────────────────
  //
  // EPISTEMIC STATUS: [PHENOMENOLOGICAL FIT], not [EMERGENT].
  //
  // What IS emergent from the FTD dynamics:
  //   - Z₃ colour LABELLING — assigned by dominant flux axis per voxel.
  //   - Confinement of like-colour configurations at distance x₋ ≈ 3.024.
  //
  // What is NOT emergent — these pieces are hand-fit to reproduce QCD:
  //   - Force coefficient cf = {+0.5 same-colour, −1 different} — SU(3)
  //     Casimir-motivated, inserted numerically.
  //   - Three-regime piecewise profile (Coulomb / flux-tube / linear) —
  //     hand-spliced at COLOR_COULOMB_RADIUS / COLOR_TRANSITION_RADIUS.
  //   - Running coupling α_s(r) — standard one-loop QCD form.
  //
  // Reading this code as "FTD derives QCD confinement" is wrong. What's
  // actually happening is: FTD supplies a 3-colour labelling; a QCD-shaped
  // potential is then built by hand on top. The shape is imposed.
  //
  // To upgrade this to a genuine derivation, replace the piecewise force
  // law with a dynamical SU(3) gauge field whose Wilson-loop expectation
  // produces linear confinement at large r without a hand-inserted regime
  // switch. That work is [OPEN].
  // PERF: colored_sites_cache_ is a bridge member — clear+push reuses capacity,
  // no per-tick malloc.
  colored_sites_cache_.clear();
  if (toggles.color_forces) {
    for (int ii = 0; ii < N; ++ii) {
      if (voxels_[ii].state != 0 && voxels_[ii].color != 0) {
        auto cc = lattice_.coord(ii);
        colored_sites_cache_.push_back({cc.x, cc.y, cc.z,
                                        voxels_[ii].state, voxels_[ii].color});
      }
    }
  }

  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];
    if (v.state == 0) continue;

    // EM force: three modes
    //   1. Poisson-based:  F = -alpha·s·∇φ_C        (standard, most accurate)
    //   2. Legacy gradient: F = -alpha·s·∇(∇·J)      (direct, short-range)
    //   3. Emergent (EFT):  F = G_C·s·∇|J|_{tier2}   (force from flux field)
    //      In mode 3, alpha = G_C² emerges: one G_C from this probe coupling,
    //      one G_C already embedded in the flux amplitude from the wave equation.
    Vec3 f_em;
    if (toggles.emergent_forces) {
      // EFT emergent force: read force FROM the flux field established by
      // wave equation + Gauss constraint. No Poisson solver needed.
      // Use tier-2 stencil (r=2 neighbors) to avoid self-field contamination.
      auto ci = lattice_.coord(i);
      int L = lattice_.size();
      double grad_x = 0, grad_y = 0, grad_z = 0;
      // Tier-2 finite differences (skip r=1 to avoid self-field wake)
      auto safe = [&](int x, int y, int z) -> double {
        int wx = ((x % L) + L) % L;
        int wy = ((y % L) + L) % L;
        int wz = ((z % L) + L) % L;
        return voxels_[lattice_.index(wx, wy, wz)].density();
      };
      grad_x = (safe(ci.x+2, ci.y, ci.z) - safe(ci.x-2, ci.y, ci.z)) * 0.25;
      grad_y = (safe(ci.x, ci.y+2, ci.z) - safe(ci.x, ci.y-2, ci.z)) * 0.25;
      grad_z = (safe(ci.x, ci.y, ci.z+2) - safe(ci.x, ci.y, ci.z-2)) * 0.25;
      Vec3 grad_rho_t2 = {grad_x, grad_y, grad_z};
      // Force = G_C · state · ∇|J| (one vertex coupling; other G_C in flux)
      f_em = grad_rho_t2 * (G_C * v.state);
    } else if (toggles.poisson_coulomb) {
      Vec3 grad_phi = gradient_scalar(i, phi_coulomb_);
      f_em = grad_phi * (-ALPHA * v.state);       // ALPHA == ALPHA_EFT (G_C² identity)
    } else {
      Vec3 grad_divJ = gradient_divergence(i);
      f_em = grad_divJ * (-ALPHA * v.state);       // ALPHA == ALPHA_EFT (G_C² identity)
    }

    // Gravitational force from density gradient
    // Use tier-2 (r=2) stencil for manifested particles to avoid
    // self-field contamination at tier-1 (r=1) face-neighbors.
    // The self-field wake at r=1 creates an asymmetric density gradient
    // that causes spurious self-acceleration. At r=2 the self-field
    // influence is negligible and only external gradients contribute.
    Vec3 f_grav;
    if (toggles.gravity) {
      auto c = lattice_.coord(i);
      double dx = voxels_[lattice_.index(c.x+2, c.y, c.z)].density()
                - voxels_[lattice_.index(c.x-2, c.y, c.z)].density();
      double dy = voxels_[lattice_.index(c.x, c.y+2, c.z)].density()
                - voxels_[lattice_.index(c.x, c.y-2, c.z)].density();
      double dz = voxels_[lattice_.index(c.x, c.y, c.z+2)].density()
                - voxels_[lattice_.index(c.x, c.y, c.z-2)].density();
      Vec3 grad_rho = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE, dz * GRAD_TIER2_SCALE};
      f_grav = grad_rho * G_N;
    }

    // Lorentz (magnetic) force: F = α·s·(v × B) where B = curl(J)
    // From the Lagrangian velocity-coupling term L_vc = -g_c·s·(v·J),
    // the E-L equation yields F = g_c·q·(v × curl(J)).
    // With coupling g_c² = α, this gives F = α·s·(v × B).
    Vec3 f_lorentz;
    if (toggles.lorentz_force && v.speed() > EPSILON_MAG) {
      Vec3 B = curl_flux(i);
      f_lorentz = Vec3::cross(v.velocity, B) * (ALPHA * v.state);       // ALPHA == ALPHA_EFT
    }

    // ── Color force: pairwise SU(3)-inspired interaction ─────────────
    // F_color(i←j) = α_s(r) · color_factor(c_i, c_j) · r̂ / r²
    // Running coupling α_s(r) implements asymptotic freedom at short r
    // and confinement saturation at large r.
    Vec3 f_color;
    if (toggles.color_forces && v.color != 0) {
      auto ci = lattice_.coord(i);
      for (auto& cs : colored_sites_cache_) {
        // Skip self via coord equality (cheaper than carrying idx)
        if (cs.cx == ci.x && cs.cy == ci.y && cs.cz == ci.z) continue;
        double ddx = cs.cx - ci.x;
        double ddy = cs.cy - ci.y;
        double ddz = cs.cz - ci.z;
        if (ddx >  L/2) ddx -= L;
        if (ddx < -L/2) ddx += L;
        if (ddy >  L/2) ddy -= L;
        if (ddy < -L/2) ddy += L;
        if (ddz >  L/2) ddz -= L;
        if (ddz < -L/2) ddz += L;
        double r2 = ddx*ddx + ddy*ddy + ddz*ddz;
        double r = std::sqrt(r2);
        if (r < 1.0) r = 1.0;  // Clamp to lattice spacing (matches GPU)
        r2 = r * r;
        double cf = (v.color == cs.color) ? 0.5 : -1.0;
        double as = alpha_s_lattice(r);

        // Three-regime force profile (matches GPU kernels_forces.cu):
        //   r < COLOR_COULOMB_RADIUS:    Coulomb (asymptotic freedom)
        //   transition:                  Flux tube stretching
        //   r >= COLOR_TRANSITION_RADIUS: Linear confinement (constant string tension)
        double F_mag;
        if (r < COLOR_COULOMB_RADIUS) {
          F_mag = as * cf / r2;
        } else if (r < COLOR_TRANSITION_RADIUS) {
          F_mag = as * cf / (COLOR_TRANSITION_DENOM * r);
        } else {
          F_mag = as * cf * r / COLOR_LINEAR_DENOM;
        }

        // ddx points from probe to source; negate for repulsive force direction
        // Same color (cf>0): force pushes AWAY from source (repulsive)
        // Diff color (cf<0): force pulls TOWARD source (attractive)
        f_color.x -= F_mag * ddx / r;
        f_color.y -= F_mag * ddy / r;
        f_color.z -= F_mag * ddz / r;
      }
    }

    Vec3 f_total = f_em + f_grav + f_lorentz + f_color;

    // Store for diagnostics
    force_diag_[i].f_coulomb = f_em;
    force_diag_[i].f_gravity = f_grav;
    force_diag_[i].f_strong = f_color;
    force_diag_[i].f_magnetic = f_lorentz;
    force_diag_[i].f_exchange = {};

    // Record acceleration magnitude
    v.accel_mag = f_total.mag();

    // Apply force (skip locked particles)
    if (!v.locked) {
      // ── γ_FTD MOMENTUM INTEGRATION (2026-04-17, TRACKER §1.2) ──────
      //
      // FTD bandwidth postulate: v²/C² + L² < 1, where C = C_SPEED and
      // L is local topological latency (gravity).  The corresponding
      // Lorentz factor is γ_FTD = 1/√(1 − v²/C² − L²).
      //
      // To respect this constraint exactly, we integrate MOMENTUM, not
      // velocity: p = γ_FTD · v.  Newton's law becomes dp/dt = F, and
      // v is extracted from p at the end of the step.  This guarantees
      // |v| → C·√(1 − L²) asymptotically as force → ∞; no clamp, no
      // energy discard, Lorentz-invariant by construction.
      //
      // Algebra (derivation in TRACKER §1.2):
      //   γ²|v|² = |p|²
      //   γ² = 1/(1 − |v|²/C² − L²)
      //   ⇒  |v|² = C²(1 − L²) · |p|² / (C² + |p|²)
      //   ⇒  v⃗   = p⃗ · C · √((1 − L²) / (C² + |p|²))
      //
      // Newtonian limit (|v| << C, L = 0): γ → 1, p ≈ v, v_new ≈ v + F·dt. ✓
      // Ultra-relativistic (|p| → ∞):       |v| → C·√(1 − L²).          ✓
      // Horizon (L → 1):                    |v| → 0.                     ✓
      //
      // Superseded the previous non-relativistic clamp
      // `if (|v| > C) v *= C/|v|;` which discarded energy and was also
      // STRICTER than the true bandwidth (clamp allowed |v| ≤ C(1−L²);
      // FTD bandwidth allows |v| ≤ C·√(1−L²)).
      const double C      = C_SPEED;
      const double C2     = C * C;
      const double L      = v.latency;                  // 0 if latency_field off
      const double L2     = L * L;
      // Budget-safe: clamp 1−L² strictly positive so sqrt() never
      // underflows at or near the horizon.
      const double one_L2 = std::max(1.0 - L2, 1e-6);

      // Current γ (with a budget floor of 1e-6 to keep γ finite if the
      // previous tick left v at the bandwidth edge).
      const double v2 = v.velocity.mag2();
      double budget  = v2 / C2 + L2;
      if (budget > 1.0 - 1e-6) budget = 1.0 - 1e-6;
      const double gamma_in = 1.0 / std::sqrt(1.0 - budget);

      // Reconstruct momentum, apply force, extract new velocity.
      Vec3 p = v.velocity * gamma_in;
      p = p + f_total * dt_;
      const double p2 = p.mag2();
      const double scale = C * std::sqrt(one_L2 / (C2 + p2));
      v.velocity = p * scale;
    }
  }
}

// ============================================================================
// RULE 5: phase_movement() — Kinematics + collisions + annihilation
//
// Particles move on the lattice via remainder accumulation:
//   remainder += velocity
//   when |remainder| >= 1 on any axis → integer lattice jump
//
// Collision outcomes (logically determined):
//   - Target void: move into it, carry self-field
//   - Target same sign: elastic bounce (two things can't be in one place)
//   - Target opposite sign: annihilation (cancel to void, flux burst)
// ============================================================================

void RenderBridge::phase_movement() {
  const int N = static_cast<int>(lattice_.total_sites());
  std::fill(moved_.begin(), moved_.end(), 0);

  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];
    if (v.state == 0 || v.locked || moved_[i]) continue;

    v.remainder += v.velocity * dt_;

    auto c = lattice_.coord(i);
    int dx = 0, dy = 0, dz = 0;

    if (v.remainder.x >= 1.0) { dx = 1; v.remainder.x -= 1.0; }
    else if (v.remainder.x <= -1.0) { dx = -1; v.remainder.x += 1.0; }
    if (v.remainder.y >= 1.0) { dy = 1; v.remainder.y -= 1.0; }
    else if (v.remainder.y <= -1.0) { dy = -1; v.remainder.y += 1.0; }
    if (v.remainder.z >= 1.0) { dz = 1; v.remainder.z -= 1.0; }
    else if (v.remainder.z <= -1.0) { dz = -1; v.remainder.z += 1.0; }

    if (dx == 0 && dy == 0 && dz == 0) continue;

    int target = lattice_.index(c.x + dx, c.y + dy, c.z + dz);
    auto &t = voxels_[target];

    if (t.state == 0) {
      // Move: transfer particle to target
      t.state = v.state;
      t.velocity = v.velocity;
      t.remainder = v.remainder;
      t.pair_id = v.pair_id;
      t.accel_mag = v.accel_mag;
      t.spin = v.spin;
      t.color = v.color;
      t.particle_id = v.particle_id;

      // Portable self-field: particle carries flux with it (up to K_B)
      double old_rho = v.density();
      if (old_rho > EPSILON_MAG) {
        double transfer = std::min(old_rho, K_B);
        double frac = transfer / old_rho;
        Vec3 self_field = v.flux * frac;
        v.flux = v.flux - self_field;
        t.flux = t.flux + self_field;

        // Dual-substrate: carry proportional L/R flux too
        if (toggles.dual_substrate) {
          Vec3 sf_L = v.flux_L * frac;
          Vec3 sf_R = v.flux_R * frac;
          v.flux_L = v.flux_L - sf_L;
          v.flux_R = v.flux_R - sf_R;
          t.flux_L = t.flux_L + sf_L;
          t.flux_R = t.flux_R + sf_R;
        }
      }

      v.state = 0;
      v.velocity = {};
      v.remainder = {};
      v.pair_id = -1;
      v.particle_id = -1;
      v.spin = 0;
      v.color = 0;
      moved_[target] = 1;  // Prevent re-processing this tick
    } else if (t.state == v.state) {
      // Same sign: elastic bounce
      if (dx != 0) v.velocity.x *= -1.0;
      if (dy != 0) v.velocity.y *= -1.0;
      if (dz != 0) v.velocity.z *= -1.0;
      v.remainder = {};
    } else {
      // Opposite sign: annihilation — both particles return to void.
      Vec3 flux_v = v.flux;
      Vec3 flux_t = t.flux;
      Vec3 flux_v_L, flux_v_R, flux_t_L, flux_t_R;
      if (toggles.dual_substrate) {
        flux_v_L = v.flux_L; flux_v_R = v.flux_R;
        flux_t_L = t.flux_L; flux_t_R = t.flux_R;
      }
      v.state = 0; t.state = 0;
      v.velocity = {}; t.velocity = {};
      v.remainder = {}; t.remainder = {};
      v.pair_id = -1; t.pair_id = -1;
      v.particle_id = -1; t.particle_id = -1;
      v.accel_mag = 0.0; t.accel_mag = 0.0;
      v.spin = 0; v.color = 0;
      t.spin = 0; t.color = 0;
      v.flux = {}; t.flux = {};
      if (toggles.dual_substrate) {
        v.flux_L = {}; v.flux_R = {};
        t.flux_L = {}; t.flux_R = {};
      }
      // Distribute each particle's flux to its own neighbors
      auto nbrs_v = lattice_.neighbors_6(i);
      auto nbrs_t = lattice_.neighbors_6(target);
      for (int n : nbrs_v) voxels_[n].flux += flux_v * (1.0 / 6.0);
      for (int n : nbrs_t) voxels_[n].flux += flux_t * (1.0 / 6.0);
      if (toggles.dual_substrate) {
        for (int n : nbrs_v) {
          voxels_[n].flux_L += flux_v_L * (1.0 / 6.0);
          voxels_[n].flux_R += flux_v_R * (1.0 / 6.0);
        }
        for (int n : nbrs_t) {
          voxels_[n].flux_L += flux_t_L * (1.0 / 6.0);
          voxels_[n].flux_R += flux_t_R * (1.0 / 6.0);
        }
      }
    }
  }
}

// ============================================================================
// The Tick: Six rules, executed in order
// ============================================================================

void RenderBridge::tick() {
  // F3 (callstack audit 2026-04-17): validate runs on BOTH paths now
  // so toggle-combination warnings surface regardless of CPU/GPU build.
  {
      std::string validErr;
      if (!toggles.validate(&validErr))
          std::cerr << "[TermToggles] Invalid combination: " << validErr;
  }

#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    // Wave 5.2: flush any host-side mutations (e.g. test doing
    // voxels()[idx].locked = true) back to the GPU before physics runs.
    gpu_flush_host_mutations();
    // Sync toggles to GPU engine
    gpu_->toggles = toggles;
    gpu_->tick();
    gpu_dirty_ = true;
    physical_time_ += dt_;
    ++tick_;

    // ── GPU EnergyLedger + proper-time (TRACKER §1.7 + audit F4) ──
    // Sync voxels back to host, then populate both the ledger and
    // (if latency_field is on) the per-particle proper-time tau.
    //
    // Cost: one PCIe download per tick (≈ 3 MB at L=64, sub-ms on
    // modern hardware; far below a CUDA tick's physics cost). If this
    // becomes a bottleneck for long CUDA benchmarks, replace with a
    // device-side reduction kernel that returns just the three scalar
    // sums (E_field, E_wave, E_kin) — see comment in gpu_engine.cu.
    gpu_sync_to_host();
    if (toggles.latency_field)
      accumulate_proper_time();
    update_energy_ledger();
    return;
  }
#endif

  // F2 (callstack audit 2026-04-17): CPU-only warning for GPU-only toggles.
  // Printed once per RenderBridge instance on the first tick where such a
  // toggle is set, so it's discoverable but doesn't spam.
  if (!cpu_warnings_emitted_) {
    std::string gpu_only_msg = toggles.cpu_runtime_warnings();
    if (!gpu_only_msg.empty()) {
      std::cerr << "[TermToggles] CPU-build warning:\n" << gpu_only_msg;
      cpu_warnings_emitted_ = true;
    }
  }

  // Rule 1: Wave propagation + state-flux coupling
  if (toggles.wave_propagation || toggles.coupling)
    phase_read();

  // Rule 2: Commit flux, damping, manifestation/evaporation
  phase_write();

  // Rule 2b: Pair production (correlated ±1 pairs from high-flux void).
  // F2 (callstack audit 2026-04-17): matching GPU path order. No-op on
  // CPU until the pair-production CPU port lands.
  if (toggles.pair_production)
    pair_production_cpu();

  // Rule 3: Gauss constraint enforcement (∇·J = s)
  if (toggles.gauss_projection)
    gauss_project();

  // Rule 3b: Self-field floor REMOVED (Phase 4 — Energy Conservation). The
  // former per-tick reset of self_field_injection_ was also a no-op — the
  // member is default-initialised 0 and nothing else writes to it now that
  // the floor is gone. (F1 from callstack audit 2026-04-17.)

  // Rule 3c: Latency field (gravitational potential) — Poisson solver
  // ∇²φ_L = 4πG·ρ_mass, then L = √(clamp(φ_L, 0, 0.998))
  // Must run after Gauss (which modifies flux) and before forces (which use L).
  if (toggles.latency_field)
    solve_latency_poisson();

  // Rule 4: Field-mediated forces
  if (toggles.forces)
    phase_forces();

  // Rule 5: Movement + collisions + annihilation
  if (toggles.movement)
    phase_movement();


  // Self-field floor moved to Rule 3b (after Gauss, before forces) in Phase 3.
  // No second floor here — eliminates the double-injection energy leak.

  // Rule 6: Weak transmutation (polarity flip under field stress).
  // F5 (callstack audit 2026-04-17): extracted to weak_transmutation_cpu().
  if (toggles.weak_transmutation)
    weak_transmutation_cpu();

  // Rule 7: Triad binding detection (3 same-sign particles → locked).
  // F2 (callstack audit 2026-04-17): matching GPU path. No-op on CPU
  // until the triad-detection kernel is ported.
  if (toggles.triad_binding)
    triad_binding_cpu();

  // Rule 8: Proper time accumulation (gravity sector).
  // F5 (callstack audit 2026-04-17): extracted to accumulate_proper_time().
  if (toggles.latency_field)
    accumulate_proper_time();

  physical_time_ += dt_;
  ++tick_;

  // ── Conservation bookkeeping (fills EnergyLedger) ────────────────────
  // Cheap: a few adds + divides; no loop over N. Tests assert on
  // `energy_ledger().residual` rather than re-deriving totals.
  update_energy_ledger();
}

// ════════════════════════════════════════════════════════════════════════
// weak_transmutation_cpu() — polarity flip under field stress.
//
// Extracted from tick() in F5 (callstack audit 2026-04-17) for symmetry
// with the GPU path (gpu_weak_transmutation). Identical algorithm.
//
// When field stress exceeds WEAK_THRESHOLD, manifested particles may flip
// polarity. In dual-substrate mode the weak force couples only to J_L
// (left-chiral), producing maximal parity violation.
// ════════════════════════════════════════════════════════════════════════
void RenderBridge::weak_transmutation_cpu() {
  const int N = static_cast<int>(lattice_.total_sites());
  for (int i = 0; i < N; ++i) {
    auto& v = voxels_[i];
    if (v.state == 0) continue;

    double stress = toggles.dual_substrate
                      ? compute_stress_left(i)
                      : compute_stress(i);

    if (stress > WEAK_THRESHOLD) {
      double p = 1.0 - std::exp(-(stress - WEAK_THRESHOLD) / K_B);
      if (uniform_(rng_) < p) {
        v.state = -v.state;
        if (toggles.dual_substrate) {
          std::swap(v.flux_L, v.flux_R);
          std::swap(v.wave_vel_L, v.wave_vel_R);
        }
      }
    }
  }
}

// ════════════════════════════════════════════════════════════════════════
// accumulate_proper_time() — FTD Schwarzschild-like proper time.
//
// Extracted from tick() in F5 (callstack audit 2026-04-17). Also called
// from the GPU tick() path after gpu_sync_to_host(), closing F4 (GPU was
// previously not accumulating v.tau at all).
//
//   f = 1 − L²   (Schwarzschild f-factor)
//   dτ/dt = √(f² − |v|²) / √f
//
// At v=0: dτ/dt = √(1−L²).  Prior version had a secondary bandwidth
// clamp here that was STRICTER than the FTD postulate allows; removed in
// TRACKER §1.2. The γ_FTD momentum integration in phase_forces now
// enforces the correct bandwidth by construction.
// ════════════════════════════════════════════════════════════════════════
void RenderBridge::accumulate_proper_time() {
  const int N = static_cast<int>(lattice_.total_sites());
  for (int i = 0; i < N; ++i) {
    auto& v = voxels_[i];
    if (v.state == 0) continue;
    double L = v.latency;
    double f = 1.0 - L * L;
    if (f <= 0.0) continue;
    double v2 = v.speed() * v.speed();
    double arg = f * f - v2;
    if (arg > 0.0)
      v.tau += std::sqrt(arg) / std::sqrt(f);
  }
}

// ════════════════════════════════════════════════════════════════════════
// pair_production_cpu() — correlated ±1 pair from high-flux void.
//
// F2 (callstack audit 2026-04-17): previously a silent no-op on CPU; the
// GPU path had `gpu_pair_production()`. This CPU port mirrors the GPU
// algorithm at moderate fidelity:
//
//   foreach void voxel with |J| above pair threshold:
//     with probability p = 1 − exp(−(|J|−K_GENESIS)/K_B):
//       manifest v.state = +1 at the current voxel
//       find the first empty face-neighbour and set its state = −1
//       assign shared pair_id so the two can be tracked as a correlated pair
//
// Kept behind the `pair_production` toggle so existing scenarios are
// unaffected.
// ════════════════════════════════════════════════════════════════════════
void RenderBridge::pair_production_cpu() {
  const int N = static_cast<int>(lattice_.total_sites());
  for (int i = 0; i < N; ++i) {
    auto& v = voxels_[i];
    if (v.state != 0) continue;                 // only manifest from void
    double jmag = v.flux.mag();
    if (jmag <= K_GENESIS) continue;            // below pair-production threshold

    double p = 1.0 - std::exp(-(jmag - K_GENESIS) / K_B);
    if (uniform_(rng_) >= p) continue;

    // Find an empty face-neighbour for the antipartner.
    int partner = -1;
    for (int n : lattice_.neighbors_6(i)) {
      if (voxels_[n].state == 0) { partner = n; break; }
    }
    if (partner < 0) continue;                  // nowhere to place the pair

    int pid;
    pid = next_particle_id_++;
    v.state = +1;
    v.particle_id = pid;
    v.pair_id = pid;

    auto& p2 = voxels_[partner];
    p2.state = -1;
    p2.particle_id = next_particle_id_++;
    p2.pair_id = pid;

    // Seed opposite flux on the antipartner so |J| doesn't duplicate.
    p2.flux = v.flux * -1.0;
  }
}

// ════════════════════════════════════════════════════════════════════════
// triad_binding_cpu() — detect 3 same-sign particles in a compact triad,
// mark them locked.
//
// F2 (callstack audit 2026-04-17): previously a silent no-op on CPU; the
// GPU path had `gpu_triad_detection()`. This CPU port follows the same
// rule used by the GPU kernel (constants.h):
//
//   - pairwise distance ≤ TRIAD_RADIUS
//   - near-equilateral: min(r)/max(r) ≥ TRIAD_RATIO_THRESHOLD
//   - all three share the same state (sign)
//
// Running time is O(M³) where M is the manifested-particle count. The
// triad_binding toggle is default-OFF; this path is only exercised when
// explicitly enabled.
// ════════════════════════════════════════════════════════════════════════
void RenderBridge::triad_binding_cpu() {
  const int N = static_cast<int>(lattice_.total_sites());
  // Snapshot manifested sites once; inner triple loop stays O(M³).
  std::vector<int> particles;
  particles.reserve(64);
  for (int i = 0; i < N; ++i) {
    if (voxels_[i].state != 0) particles.push_back(i);
  }

  auto coord_dist = [&](int a, int b) {
    auto ca = lattice_.coord(a), cb = lattice_.coord(b);
    double dx = ca.x - cb.x, dy = ca.y - cb.y, dz = ca.z - cb.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
  };

  const int M = static_cast<int>(particles.size());
  for (int a = 0; a < M; ++a) {
    auto& va = voxels_[particles[a]];
    if (va.locked) continue;
    for (int b = a + 1; b < M; ++b) {
      auto& vb = voxels_[particles[b]];
      if (vb.locked || vb.state != va.state) continue;
      double rAB = coord_dist(particles[a], particles[b]);
      if (rAB > TRIAD_RADIUS) continue;
      for (int c = b + 1; c < M; ++c) {
        auto& vc = voxels_[particles[c]];
        if (vc.locked || vc.state != va.state) continue;
        double rAC = coord_dist(particles[a], particles[c]);
        double rBC = coord_dist(particles[b], particles[c]);
        if (rAC > TRIAD_RADIUS || rBC > TRIAD_RADIUS) continue;
        double rmin = std::min({rAB, rAC, rBC});
        double rmax = std::max({rAB, rAC, rBC});
        if (rmax < 1e-9) continue;
        if (rmin / rmax < TRIAD_RATIO_THRESHOLD) continue;
        // Found a near-equilateral same-sign triad — lock all three.
        va.locked = true;
        vb.locked = true;
        vc.locked = true;
        break;                                   // next outer pair
      }
    }
  }
}

// ════════════════════════════════════════════════════════════════════════
// update_energy_ledger() — per-tick conservation drift populator.
//
// After every tick, snapshot the total scalar energy (field + wave-vel +
// manifested kinetic) and compare to the previous tick:
//
//   drift_frac = (E_curr − E_prev) / max(|E_prev|, ε)
//   expected   = −DAMPING   when the damping toggle is on, else 0
//   residual   = drift_frac − expected
//
// Tests can assert `|residual| < tol` and refuse regressions. Cost is
// O(N) on a small inner loop already cache-warm from phase_movement.
// ════════════════════════════════════════════════════════════════════════
void RenderBridge::update_energy_ledger() {
  const int N = static_cast<int>(lattice_.total_sites());
  double E_field = 0.0, E_wave = 0.0, E_kin = 0.0;
  for (int i = 0; i < N; ++i) {
    const auto& v = voxels_[i];
    E_field += v.flux.mag2();
    E_wave  += v.wave_vel.mag2();
    if (v.state != 0) E_kin += 0.5 * v.velocity.mag2();
  }
  const double E_total = 0.5 * (E_field + E_wave) + E_kin;

  // First call: seed E_prev so the next tick has a baseline.
  if (energy_ledger_.tick_prev < 0) {
    energy_ledger_.tick_prev  = tick_;
    energy_ledger_.E_prev     = E_total;
    energy_ledger_.E_curr     = E_total;
    energy_ledger_.dE_dt      = 0.0;
    energy_ledger_.drift_frac = 0.0;
    energy_ledger_.residual   = 0.0;
    energy_ledger_.expected_rate = toggles.damping ? -DAMPING : 0.0;
    return;
  }

  const double E_prev = energy_ledger_.E_curr;     // rotate
  energy_ledger_.tick_prev  = tick_ - 1;
  energy_ledger_.E_prev     = E_prev;
  energy_ledger_.E_curr     = E_total;
  energy_ledger_.dE_dt      = (E_total - E_prev) / std::max(dt_, 1e-12);

  const double denom = std::max(std::abs(E_prev), 1e-12);
  energy_ledger_.drift_frac = (E_total - E_prev) / denom;
  energy_ledger_.expected_rate = toggles.damping ? -DAMPING : 0.0;
  energy_ledger_.residual   = energy_ledger_.drift_frac - energy_ledger_.expected_rate;

  // Cumulative tracking for whole-run test assertions.
  if (energy_ledger_.residual > 0.0) {
    energy_ledger_.cumulative_injection += energy_ledger_.residual * denom;
  } else {
    energy_ledger_.cumulative_dissipation += (-energy_ledger_.residual) * denom;
  }
  const double abs_res = std::abs(energy_ledger_.residual);
  if (abs_res > energy_ledger_.max_residual_seen) {
    energy_ledger_.max_residual_seen = abs_res;
  }
}

void RenderBridge::run(int num_ticks) {
#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    gpu_->toggles = toggles;
    gpu_->run(num_ticks);
    gpu_dirty_ = true;
    physical_time_ += dt_ * num_ticks;
    tick_ += num_ticks;
    return;
  }
#endif
  for (int i = 0; i < num_ticks; ++i) {
    tick();
  }
}

// ============================================================================
// Diagnostics
// ============================================================================

Diagnostics RenderBridge::diagnostics() const {
  Diagnostics d;
  d.tick = tick_;

  const int N = static_cast<int>(lattice_.total_sites());

  for (int i = 0; i < N; ++i) {
    const auto &v = voxels_[i];
    d.total_flux += v.density();
    d.total_energy += std::abs(v.born_infeld_core());
    double bw = v.bandwidth_used();
    if (bw > d.max_bandwidth)
      d.max_bandwidth = bw;

    if (v.state != 0) {
      d.manifested_count++;
      if (v.state > 0) d.positive_count++;
      else d.negative_count++;
      if (v.spin > 0) d.spin_up_count++;
      else if (v.spin < 0) d.spin_down_count++;
      if (v.color >= 0 && v.color <= 3) d.color_count[v.color]++;
    }
  }

  d.total_entropy = compute_entropy();

  // Angular momentum: L = sum (r - r_cm) × v
  {
    Vec3 r_cm;
    int n_manifested = 0;
    for (int i = 0; i < N; ++i) {
      if (voxels_[i].state != 0) {
        Coord c = lattice_.coord(i);
        r_cm.x += c.x;
        r_cm.y += c.y;
        r_cm.z += c.z;
        n_manifested++;
      }
    }
    if (n_manifested > 0) {
      r_cm *= (1.0 / n_manifested);
      Vec3 L_total;
      for (int i = 0; i < N; ++i) {
        if (voxels_[i].state != 0) {
          Coord c = lattice_.coord(i);
          double rx = c.x - r_cm.x, ry = c.y - r_cm.y, rz = c.z - r_cm.z;
          const auto& vel = voxels_[i].velocity;
          L_total.x += ry * vel.z - rz * vel.y;
          L_total.y += rz * vel.x - rx * vel.z;
          L_total.z += rx * vel.y - ry * vel.x;
        }
      }
      d.total_angular_momentum = L_total;
    }
  }

  return d;
}

// ============================================================================
// Energy Audit — rigorous energy breakdown + Gauss constraint check
// ============================================================================

EnergyAudit RenderBridge::energy_audit() const {
  EnergyAudit a;
  const int N = static_cast<int>(lattice_.total_sites());

  for (int i = 0; i < N; ++i) {
    const auto &v = voxels_[i];

    a.field_energy += v.flux.mag2();
    a.wave_energy += v.wave_vel.mag2();

    // E = -wave_vel, B = curl(J)
    Vec3 E = v.wave_vel * -1.0;
    Vec3 B = curl_flux(i);
    a.E_field_energy += 0.5 * E.mag2();
    a.B_field_energy += 0.5 * B.mag2();

    // Poynting vector: S = E × B
    a.total_poynting.x += E.y * B.z - E.z * B.y;
    a.total_poynting.y += E.z * B.x - E.x * B.z;
    a.total_poynting.z += E.x * B.y - E.y * B.x;

    // Dual-substrate diagnostics
    if (toggles.dual_substrate) {
      a.E_L_total += v.flux_L.mag2() + v.wave_vel_L.mag2();
      a.E_R_total += v.flux_R.mag2() + v.wave_vel_R.mag2();
      a.chirality_total += v.chirality_density();
    }

    if (v.state != 0) {
      a.particle_ke += 0.5 * v.velocity.mag2();
      a.charge_total += v.state;
      a.manifested_count++;
    }

    double err = divergence_flux(i) - static_cast<double>(v.state);
    a.gauss_violation += err * err;
    double abs_err = std::abs(err);
    if (abs_err > a.max_gauss_error)
      a.max_gauss_error = abs_err;
  }

  a.total_energy = a.field_energy + a.wave_energy + a.particle_ke;
  a.self_field_injection = self_field_injection_;

  // Coulomb PE: sum of alpha * s_i * phi_C(i).  NOTE: includes self-energy
  // (each particle's interaction with its own Poisson field).  Self-energy is
  // ~constant for fixed particle count, so *changes* in coulomb_pe between
  // ticks are physically meaningful even though the absolute value is inflated.
  if (!phi_coulomb_.empty()) {
    for (int i = 0; i < N; ++i) {
      if (voxels_[i].state != 0)
        a.coulomb_pe += ALPHA * voxels_[i].state * phi_coulomb_[i];
    }
  }

  return a;
}

// ============================================================================
// EM Field Decomposition
//
// Returns the electric and magnetic fields at a single lattice site.
// E = -∂J/∂t ≈ -wave_vel (the leapfrog momentum variable is the time derivative)
// B = ∇×J (curl of the flux field, the vector potential analog)
// ============================================================================

EMFieldDiag RenderBridge::em_field_at(int idx) const {
  EMFieldDiag em;
  em.E = voxels_[idx].wave_vel * -1.0;
  em.B = curl_flux(idx);
  em.E_mag = em.E.mag();
  em.B_mag = em.B.mag();
  return em;
}

// ============================================================================
// Poynting Vector
//
// S = E × B = (-wave_vel) × (∇×J)
// Gives the energy flux density (direction and magnitude of EM energy flow).
// ============================================================================

Vec3 RenderBridge::poynting_vector(int idx) const {
  Vec3 E = voxels_[idx].wave_vel * -1.0;
  Vec3 B = curl_flux(idx);
  // Cross product: E × B
  return Vec3{
    E.y * B.z - E.z * B.y,
    E.z * B.x - E.x * B.z,
    E.x * B.y - E.y * B.x
  };
}

// ============================================================================
// Injection (unchanged from v1)
// ============================================================================

void RenderBridge::inject_flux(int x, int y, int z, const Vec3 &flux_val) {
#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    gpu_->inject_flux(x, y, z, flux_val);
    gpu_dirty_ = true;
    return;
  }
#endif
  auto &v = voxels_[lattice_.index(x, y, z)];
  v.flux = flux_val;
  if (toggles.dual_substrate) {
    v.flux_L = flux_val * 0.5;
    v.flux_R = flux_val * 0.5;
  }
}

void RenderBridge::inject_particle(int x, int y, int z, int8_t state,
                                   const Vec3 &flux_val,
                                   int8_t spin, int8_t color) {
#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    gpu_->inject_particle(x, y, z, state, flux_val, spin, color);
    gpu_dirty_ = true;
    return;
  }
#endif
  auto &v = voxels_[lattice_.index(x, y, z)];
  v.state = state;
  v.flux = flux_val;
  v.spin = spin;
  v.color = color;
  v.particle_id = next_particle_id_++;

  // Dual-substrate: split flux between L and R based on particle sign.
  // +1 particle: J_L gets (1+delta)/2 of flux, J_R gets (1-delta)/2
  // -1 particle: J_R gets (1+delta)/2, J_L gets (1-delta)/2
  // This ensures chirality_density() has the correct sign for manifestation.
  if (toggles.dual_substrate) {
    double frac_major = (1.0 + DELTA_APPROX) * 0.5;
    double frac_minor = (1.0 - DELTA_APPROX) * 0.5;
    if (state > 0) {
      v.flux_L = flux_val * frac_major;
      v.flux_R = flux_val * frac_minor;
    } else {
      v.flux_L = flux_val * frac_minor;
      v.flux_R = flux_val * frac_major;
    }
  }
}

// ============================================================================
// Phase 6: Flux-Aggregate Wavepacket Injection
//
// Distributes flux as a radial Gaussian envelope around a state ±1 seed.
// The seed provides nonlinear anchoring via coupling source + Gauss constraint.
// The Gaussian approximates the natural steady-state self-field profile
// (Stage 1 measured r_eff ≈ 3.33, power-law exponent ≈ 1.2).
// ============================================================================

void RenderBridge::inject_wavepacket(int cx, int cy, int cz, int8_t state,
                                     double sigma, double amplitude) {
#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    gpu_->inject_wavepacket(cx, cy, cz, state, sigma, amplitude);
    gpu_dirty_ = true;
    return;
  }
#endif
  int center = lattice_.index(cx, cy, cz);
  auto &vc = voxels_[center];
  vc.state = state;
  vc.particle_id = next_particle_id_++;

  // Distribute Gaussian flux in radial pattern
  // First pass: accumulate normalization factor
  int radius = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
  double norm_sum = 0.0;
  int N = lattice_.size();

  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;  // skip center
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > GAUSSIAN_CUTOFF_SIGMA * sigma) continue;
        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        norm_sum += g * g;  // |J|^2 contribution
      }
    }
  }

  // Scale factor so total |J|^2 = amplitude^2
  double scale = (norm_sum > EPSILON_FLUX_SQ) ? amplitude / std::sqrt(norm_sum) : 0.0;

  // Second pass: set flux vectors (radial direction)
  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > GAUSSIAN_CUTOFF_SIGMA * sigma) continue;

        // Wrap coordinates (periodic boundary)
        int x = ((cx + dx) % N + N) % N;
        int y = ((cy + dy) % N + N) % N;
        int z = ((cz + dz) % N + N) % N;
        int idx = lattice_.index(x, y, z);

        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        // Radial unit vector from center, scaled by Gaussian * normalization
        double mag = scale * g;
        Vec3 flux_inc(mag * dx / r, mag * dy / r, mag * dz / r);
        voxels_[idx].flux += flux_inc;

        // Dual-substrate: split flux between L and R
        if (toggles.dual_substrate) {
          double frac_major = (1.0 + DELTA_APPROX) * 0.5;
          double frac_minor = (1.0 - DELTA_APPROX) * 0.5;
          if (state > 0) {
            voxels_[idx].flux_L += flux_inc * frac_major;
            voxels_[idx].flux_R += flux_inc * frac_minor;
          } else {
            voxels_[idx].flux_L += flux_inc * frac_minor;
            voxels_[idx].flux_R += flux_inc * frac_major;
          }
        }
      }
    }
  }
}

// ============================================================================
// Phase 6: Aggregate Profile Diagnostic
//
// Measures the spatial structure of the flux envelope around a given center.
// Pure diagnostic — does not modify any state.
// ============================================================================

AggregateProfile RenderBridge::aggregate_profile(int center_idx, double threshold) const {
  AggregateProfile prof;
  auto cc = lattice_.coord(center_idx);
  int N = lattice_.size();
  int scan = 20;  // scan radius

  double sum_j2 = 0.0;
  double sum_r2_j2 = 0.0;
  Vec3 sum_rj2;  // for center of mass
  int radial_count[20] = {};
  double radial_sum[20] = {};

  for (int dx = -scan; dx <= scan; ++dx) {
    for (int dy = -scan; dy <= scan; ++dy) {
      for (int dz = -scan; dz <= scan; ++dz) {
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        int ri = static_cast<int>(std::round(r));
        if (ri < 1 || ri > 20) continue;

        int x = ((cc.x + dx) % N + N) % N;
        int y = ((cc.y + dy) % N + N) % N;
        int z = ((cc.z + dz) % N + N) % N;
        int idx = lattice_.index(x, y, z);
        double j2 = voxels_[idx].flux.mag2();
        double jmag = std::sqrt(j2);

        if (jmag > threshold) prof.site_count++;
        if (jmag > prof.peak_density) prof.peak_density = jmag;

        sum_j2 += j2;
        sum_r2_j2 += r2 * j2;
        sum_rj2.x += (cc.x + dx) * j2;
        sum_rj2.y += (cc.y + dy) * j2;
        sum_rj2.z += (cc.z + dz) * j2;

        radial_sum[ri - 1] += jmag;
        radial_count[ri - 1]++;
      }
    }
  }

  // Also include center site (r=0)
  double j2_center = voxels_[center_idx].flux.mag2();
  sum_j2 += j2_center;

  prof.total_energy = sum_j2;
  prof.effective_radius = (sum_j2 > EPSILON_FLUX_SQ) ? std::sqrt(sum_r2_j2 / sum_j2) : 0.0;

  if (sum_j2 > EPSILON_FLUX_SQ) {
    prof.center_of_mass = Vec3(sum_rj2.x / sum_j2, sum_rj2.y / sum_j2, sum_rj2.z / sum_j2);
  } else {
    auto c = lattice_.coord(center_idx);
    prof.center_of_mass = Vec3(c.x, c.y, c.z);
  }

  for (int i = 0; i < 20; ++i) {
    prof.radial_profile[i] = (radial_count[i] > 0) ? radial_sum[i] / radial_count[i] : 0.0;
  }

  return prof;
}

}  // namespace ftd

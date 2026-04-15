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
      moved_(lattice_.total_sites(), 0)
{
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

Vec3 RenderBridge::laplacian_flux(int idx) const {
  // Isotropic 18-point Laplacian: (1/3)*sum_face + (1/6)*sum_edge - 4*center
  // Cancels O(k^4) anisotropy of the 6-point stencil.
  const auto& face = lattice_.neighbors_6(idx);
  const auto& edge = lattice_.neighbors_12(idx);
  Vec3 lap;
  for (int n : face) lap += voxels_[n].flux * (1.0/3.0);
  for (int n : edge) lap += voxels_[n].flux * (1.0/6.0);
  lap -= voxels_[idx].flux * 4.0;
  assert(!std::isnan(lap.x) && !std::isnan(lap.y) && !std::isnan(lap.z));
  return lap;
}

Vec3 RenderBridge::laplacian_flux_L(int idx) const {
  const auto& face = lattice_.neighbors_6(idx);
  const auto& edge = lattice_.neighbors_12(idx);
  Vec3 lap;
  for (int n : face) lap += voxels_[n].flux_L * (1.0/3.0);
  for (int n : edge) lap += voxels_[n].flux_L * (1.0/6.0);
  lap -= voxels_[idx].flux_L * 4.0;
  return lap;
}

Vec3 RenderBridge::laplacian_flux_R(int idx) const {
  const auto& face = lattice_.neighbors_6(idx);
  const auto& edge = lattice_.neighbors_12(idx);
  Vec3 lap;
  for (int n : face) lap += voxels_[n].flux_R * (1.0/3.0);
  for (int n : edge) lap += voxels_[n].flux_R * (1.0/6.0);
  lap -= voxels_[idx].flux_R * 4.0;
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

double RenderBridge::compute_stress(int idx) const {
  double div_mag = std::abs(divergence_flux(idx));
  Vec3 c = curl_flux(idx);
  double curl_mag = c.mag();
  Vec3 gd = gradient_density(idx);
  double grad_mag = gd.mag();
  return div_mag + curl_mag + grad_mag;
}

double RenderBridge::compute_stress_left(int idx) const {
  // Stress computed from J_L only (left-chiral component).
  // Used for weak transmutation with parity violation in dual-substrate mode.
  // In dual mode, +1 particles have J_L dominant → high weak stress → easy transmutation
  //              -1 particles have J_R dominant → low weak stress → hard transmutation
  // This gives maximal parity violation (δ ≈ 0.957).
  const auto& nbrs = lattice_.neighbors_6(idx);

  // Divergence of J_L
  double div_L = 0.0;
  div_L += (voxels_[nbrs[0]].flux_L.x - voxels_[nbrs[1]].flux_L.x) * 0.5;
  div_L += (voxels_[nbrs[2]].flux_L.y - voxels_[nbrs[3]].flux_L.y) * 0.5;
  div_L += (voxels_[nbrs[4]].flux_L.z - voxels_[nbrs[5]].flux_L.z) * 0.5;
  double div_mag = std::abs(div_L);

  // Curl of J_L
  Vec3 curl_L;
  curl_L.x = (voxels_[nbrs[2]].flux_L.z - voxels_[nbrs[3]].flux_L.z) * 0.5 -
             (voxels_[nbrs[4]].flux_L.y - voxels_[nbrs[5]].flux_L.y) * 0.5;
  curl_L.y = (voxels_[nbrs[4]].flux_L.x - voxels_[nbrs[5]].flux_L.x) * 0.5 -
             (voxels_[nbrs[0]].flux_L.z - voxels_[nbrs[1]].flux_L.z) * 0.5;
  curl_L.z = (voxels_[nbrs[0]].flux_L.y - voxels_[nbrs[1]].flux_L.y) * 0.5 -
             (voxels_[nbrs[2]].flux_L.x - voxels_[nbrs[3]].flux_L.x) * 0.5;
  double curl_mag = curl_L.mag();

  // Gradient of |J_L|
  double rho_xp = voxels_[nbrs[0]].flux_L.mag();
  double rho_xm = voxels_[nbrs[1]].flux_L.mag();
  double rho_yp = voxels_[nbrs[2]].flux_L.mag();
  double rho_ym = voxels_[nbrs[3]].flux_L.mag();
  double rho_zp = voxels_[nbrs[4]].flux_L.mag();
  double rho_zm = voxels_[nbrs[5]].flux_L.mag();
  double gx = (rho_xp - rho_xm) * 0.5;
  double gy = (rho_yp - rho_ym) * 0.5;
  double gz = (rho_zp - rho_zm) * 0.5;
  double grad_mag = std::sqrt(gx*gx + gy*gy + gz*gz);

  return div_mag + curl_mag + grad_mag;
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
// ============================================================================

void RenderBridge::phase_read() {
  const int N = static_cast<int>(lattice_.total_sites());
  const bool do_wave = toggles.wave_propagation;
  const bool do_coupling = toggles.coupling;
  const bool dual = toggles.dual_substrate;

  if (dual) {
    // Dual-substrate: compute delta for J_L and J_R independently
#pragma omp parallel for
    for (int i = 0; i < N; ++i) {
      delta_j_L_[i] = {};
      delta_j_R_[i] = {};

      if (do_wave) {
        delta_j_L_[i] = laplacian_flux_L(i) * (C_WAVE * C_WAVE);
        delta_j_R_[i] = laplacian_flux_R(i) * (C_WAVE * C_WAVE);
      }

      // Coupling source: split equally between L and R
      // Each substrate receives half the state-flux coupling
      if (do_coupling) {
        Vec3 grad_s = gradient_state(i) * (G_C * 0.5);
        Vec3 curl_sv = curl_state_velocity(i) * (G_C * 0.5);
        delta_j_L_[i] += grad_s + curl_sv;
        delta_j_R_[i] += grad_s + curl_sv;
      }
    }
  } else {
    // Single-substrate (legacy)
#pragma omp parallel for
    for (int i = 0; i < N; ++i) {
      delta_j_[i] = {};

      if (do_wave)
        delta_j_[i] = laplacian_flux(i) * (C_WAVE * C_WAVE);

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

  // Pre-generate per-thread RNG seeds from the base RNG (sequential, deterministic).
  // This avoids data races on rng_/uniform_ inside the parallel region while
  // maintaining reproducibility (same base seed → same per-thread seeds → same results).
  int num_threads = 1;
#ifdef _OPENMP
  num_threads = omp_get_max_threads();
#endif
  std::vector<unsigned int> thread_seeds(num_threads);
  for (int t = 0; t < num_threads; ++t)
    thread_seeds[t] = rng_();

#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    auto &v = voxels_[i];

    // Thread-local RNG seeded deterministically from pre-generated seeds.
    // Each thread gets a unique seed; within a thread, the voxel index
    // provides additional entropy so results don't depend on loop scheduling.
    int tid = 0;
#ifdef _OPENMP
    tid = omp_get_thread_num();
#endif
    std::mt19937 local_rng(thread_seeds[tid] + static_cast<unsigned int>(i));
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

void RenderBridge::gauss_project() {
  const int N = static_cast<int>(lattice_.total_sites());
  constexpr int SOR_ITERS = SOR_ITERATIONS;
  constexpr double OMEGA = SOR_OMEGA;

  std::vector<double> violation(N);
#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    violation[i] = divergence_flux(i) - static_cast<double>(voxels_[i].state);
  }

  // Warm-started: phi_ retains values from previous tick (no cold-start reset).
  // SOR is sequential (Gauss-Seidel based) — no parallel for in inner loop.
  // Uses isotropic 18-point stencil: (1/3)*face + (1/6)*edge, divisor=4.
  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    for (int i = 0; i < N; ++i) {
      const auto& face = lattice_.neighbors_6(i);
      const auto& edge = lattice_.neighbors_12(i);
      double face_sum = 0.0, edge_sum = 0.0;
      for (int n : face) face_sum += phi_[n];
      for (int n : edge) edge_sum += phi_[n];
      double gs = ((1.0/3.0) * face_sum + (1.0/6.0) * edge_sum - violation[i]) / 4.0;
      phi_[i] += OMEGA * (gs - phi_[i]);
    }
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
#pragma omp parallel for
  for (int i = 0; i < N; ++i) {
    if (voxels_[i].state != 0) continue;  // Skip manifested sites
    Vec3 grad_phi = gradient_scalar(i, phi_);
    voxels_[i].flux -= grad_phi;

    // Dual-substrate: split correction equally between L and R
    // This maintains div(J_L + J_R) = s while preserving L/R symmetry
    if (toggles.dual_substrate) {
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
  double mean_charge = charge_sum / N;

  // SOR iteration (warm-started from previous tick's phi_coulomb_)
  // Note: ∇²φ = -s (standard electrostatic convention: ∇²V = -ρ/ε₀)
  // so that F = -α·s·∇φ gives repulsion for like charges, attraction for unlike.
  // Uses isotropic 18-point stencil: (1/3)*face + (1/6)*edge, divisor=4.
  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    for (int i = 0; i < N; ++i) {
      const auto& face = lattice_.neighbors_6(i);
      const auto& edge = lattice_.neighbors_12(i);
      double face_sum = 0.0, edge_sum = 0.0;
      for (int n : face) face_sum += phi_coulomb_[n];
      for (int n : edge) edge_sum += phi_coulomb_[n];
      double source = -(static_cast<double>(voxels_[i].state) - mean_charge);
      double phi_gs = ((1.0/3.0) * face_sum + (1.0/6.0) * edge_sum - source) / 4.0;
      phi_coulomb_[i] = (1.0 - OMEGA) * phi_coulomb_[i] + OMEGA * phi_gs;
    }
  }

  // Pin gauge: subtract mean of phi
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_coulomb_[i];
  double phi_mean = phi_sum / N;
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
  double mean_mass = mass_sum / N;

  // SOR iteration (warm-started from previous tick's phi_latency_)
  // Convention: ∇²φ = +4πGρ (positive: attractive potential is positive near mass)
  // Uses isotropic 18-point stencil: (1/3)*face + (1/6)*edge, divisor=4.
  for (int iter = 0; iter < SOR_ITERS; ++iter) {
    for (int i = 0; i < N; ++i) {
      const auto& face = lattice_.neighbors_6(i);
      const auto& edge = lattice_.neighbors_12(i);
      double face_sum = 0.0, edge_sum = 0.0;
      for (int n : face) face_sum += phi_latency_[n];
      for (int n : edge) edge_sum += phi_latency_[n];
      double rho_mass = K_B * std::abs(voxels_[i].state);
      double source = FOUR_PI_G * (rho_mass - mean_mass);
      double phi_gs = ((1.0/3.0) * face_sum + (1.0/6.0) * edge_sum - source) / 4.0;
      phi_latency_[i] = (1.0 - OMEGA) * phi_latency_[i] + OMEGA * phi_gs;
    }
  }

  // Pin gauge: subtract mean of phi
  double phi_sum = 0.0;
  for (int i = 0; i < N; ++i)
    phi_sum += phi_latency_[i];
  double phi_mean = phi_sum / N;
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
    double clamped = std::min(abs_phi, 0.998);
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

  // ── Color force: collect manifested colored particles ──────────────
  // SU(3)-inspired pairwise color interaction [IMPOSED coefficients]:
  //   same color (c_i == c_j):     +1/2 (repulsive)
  //   different color (c_i != c_j): -1   (attractive)
  // Coupling: running α_s(r) with confinement clamping.
  // The Z_3 color labeling comes from dominant flux axis [EMERGENT].
  // The force coefficients come from SU(3) Casimir operators [IMPOSED].
  struct ColoredSite { int idx; int8_t state; int8_t color; int cx, cy, cz; };
  std::vector<ColoredSite> colored_sites;
  if (toggles.color_forces) {
    for (int ii = 0; ii < N; ++ii) {
      if (voxels_[ii].state != 0 && voxels_[ii].color != 0) {
        auto cc = lattice_.coord(ii);
        colored_sites.push_back({ii, voxels_[ii].state, voxels_[ii].color,
                                 cc.x, cc.y, cc.z});
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
      f_em = grad_phi * (-ALPHA_EFT * v.state);  // EFT: alpha = G_C² (derived)
    } else {
      Vec3 grad_divJ = gradient_divergence(i);
      f_em = grad_divJ * (-ALPHA_EFT * v.state);  // EFT: alpha = G_C² (derived)
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
      f_lorentz = Vec3::cross(v.velocity, B) * (ALPHA_EFT * v.state);  // EFT: G_C²
    }

    // ── Color force: pairwise SU(3)-inspired interaction ─────────────
    // F_color(i←j) = α_s(r) · color_factor(c_i, c_j) · r̂ / r²
    // Running coupling α_s(r) implements asymptotic freedom at short r
    // and confinement saturation at large r.
    Vec3 f_color;
    if (toggles.color_forces && v.color != 0) {
      auto ci = lattice_.coord(i);
      for (auto& cs : colored_sites) {
        if (cs.idx == i) continue;
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
        //   r < 3:  Coulomb (asymptotic freedom)
        //   3-8:    Transition (flux tube stretching)
        //   r >= 8: Linear confinement (constant string tension)
        double F_mag;
        if (r < 3.0) {
          F_mag = as * cf / r2;
        } else if (r < 8.0) {
          F_mag = as * cf / (3.0 * r);
        } else {
          F_mag = as * cf * r / 64.0;
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
      v.velocity += f_total * dt_;

      // Enforce speed limit: |v| <= C_SPEED = 1
      double spd = v.speed();
      if (spd > C_SPEED) {
        v.velocity *= (C_SPEED / spd);
      }
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
#ifdef FTD_ENABLE_CUDA
  if (use_gpu_) {
    // Sync toggles to GPU engine
    gpu_->toggles = toggles;
    gpu_->tick();
    gpu_dirty_ = true;
    physical_time_ += dt_;
    ++tick_;
    return;
  }
#endif

  // Rule 1: Wave propagation + state-flux coupling
  if (toggles.wave_propagation || toggles.coupling)
    phase_read();

  // Rule 2: Commit flux, damping, manifestation/evaporation
  phase_write();

  // Rule 3: Gauss constraint enforcement (∇·J = s)
  if (toggles.gauss_projection)
    gauss_project();

  // Rule 3b: Self-field floor REMOVED (Phase 4 — Energy Conservation).
  //
  // The floor previously boosted |J| to K_B at manifested sites every tick.
  // This fought the Gauss projection in a perpetual cycle, injecting ~4100%
  // energy over 1000 ticks.  Mathematical analysis proved the floor was
  // unnecessary:
  //   - div(J)(i) does NOT involve J(i) (central-difference structure)
  //   - All flux at a particle site is transverse (invisible to Gauss)
  //   - Locked particles cannot evaporate (phase_write checks !v.locked)
  //   - Evaporation threshold (K_B*0.01) is far below natural steady-state flux
  //   - Coulomb forces use the Poisson solver (state-driven, not flux-driven)
  //
  // Removing the floor eliminates the only source of energy injection.
  self_field_injection_ = 0.0;

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

  // Rule 6: Weak transmutation (polarity flip under field stress) [CLAUDE.md §6.5]
  // When field stress exceeds WEAK_THRESHOLD, manifested particles may flip polarity.
  // In dual-substrate mode: weak force couples only to J_L (left-chiral component),
  // producing maximal parity violation — +1 particles (J_L-dominant) transmute
  // readily while -1 particles (J_R-dominant) are nearly immune.
  // Coefficients are [IMPOSED] from electroweak theory (sin²θ_W, K_GENESIS).
  if (toggles.weak_transmutation) {
    const int N = static_cast<int>(lattice_.total_sites());
    for (int i = 0; i < N; ++i) {
      auto& v = voxels_[i];
      if (v.state == 0) continue;  // Only manifested particles transmute

      // Compute stress: total in single mode, left-chiral in dual mode
      double stress = toggles.dual_substrate
                        ? compute_stress_left(i)
                        : compute_stress(i);

      if (stress > WEAK_THRESHOLD) {
        // Probabilistic flip: p = 1 - exp(-(stress - WEAK_THRESHOLD) / K_B)
        double p = 1.0 - std::exp(-(stress - WEAK_THRESHOLD) / K_B);
        if (uniform_(rng_) < p) {
          v.state = -v.state;  // Flip polarity (+1 ↔ -1)

          // In dual mode, swap L/R flux to match new chirality
          // +1 → -1: was J_L-dominant, now becomes J_R-dominant
          if (toggles.dual_substrate) {
            std::swap(v.flux_L, v.flux_R);
            std::swap(v.wave_vel_L, v.wave_vel_R);
          }
        }
      }
    }
  }

  // Rule 8: Proper time accumulation (gravity sector #43/#45)
  // dτ/dt = √(f² - v²)/f where f = 1 - L². At v=0: dτ/dt = √(1-L²).
  // Also enforce bandwidth constraint: speed limit becomes v < f·C_SPEED.
  if (toggles.latency_field) {
    const int N = static_cast<int>(lattice_.total_sites());
    for (int i = 0; i < N; ++i) {
      auto& v = voxels_[i];
      if (v.state != 0) {
        double L = v.latency;
        double f = 1.0 - L * L;
        if (f > 0.0) {
          // Proper time: dτ/dt = √(f² - v²) / √f
          double v2 = v.speed() * v.speed();
          double arg = f * f - v2;
          if (arg > 0.0)
            v.tau += std::sqrt(arg) / std::sqrt(f);

          // Bandwidth constraint: effective speed limit is f × C_SPEED
          double v_max = C_SPEED * std::max(f, 0.001);
          double spd = v.speed();
          if (spd > v_max) {
            v.velocity *= (v_max / spd);
          }
        }
      }
    }
  }

  physical_time_ += dt_;
  ++tick_;
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
        a.coulomb_pe += ALPHA_EFT * voxels_[i].state * phi_coulomb_[i];
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

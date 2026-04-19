/**
 * @file gpu_engine.cu
 * @brief GPU-accelerated FTD tick engine.
 *
 * [EXTENDED] Phase 1: Shell implementation. tick() delegates to CPU via download/upload
 * until GPU kernels are implemented in Phases 2-4.
 */

#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include <cuda_runtime.h>
#include <cufft.h>
#include <curand.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

#define CUFFT_CHECK(call) do { \
    cufftResult err = (call); \
    if (err != CUFFT_SUCCESS) { \
        fprintf(stderr, "cuFFT error at %s:%d: %d\n", \
                __FILE__, __LINE__, (int)err); \
        exit(1); \
    } \
} while(0)

#define CURAND_CHECK(call) do { \
    curandStatus_t err = (call); \
    if (err != CURAND_STATUS_SUCCESS) { \
        fprintf(stderr, "cuRAND error at %s:%d: %d\n", \
                __FILE__, __LINE__, (int)err); \
        exit(1); \
    } \
} while(0)

// Forward declarations of GPU kernel launchers (implemented in kernel files)
namespace ftd { namespace gpu { namespace kernels {
    void launch_phase_read(const GpuBuffers& bufs, bool do_wave, bool do_coupling);
    void launch_phase_write(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                            bool larmor_radiation, double damping_factor,
                            bool do_genesis, double dt);
    void launch_gauss_project(GpuBuffers& bufs,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_solve_coulomb(GpuBuffers& bufs,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_solve_latency(GpuBuffers& bufs,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_phase_forces(GpuBuffers& bufs, bool poisson_coulomb,
                             bool gravity, bool lorentz_force, double dt);
    void launch_phase_movement(GpuBuffers& bufs, double dt);
    // Dual-substrate launchers
    void launch_phase_read_dual(const GpuBuffers& bufs, bool do_wave, bool do_coupling);
    void launch_phase_write_dual(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                                  bool larmor_radiation, double damping_factor,
                                  bool do_genesis, double dt);
    void launch_gauss_sync_dual(GpuBuffers& bufs);

    // Fused wave update (single-substrate: replaces phase_read + phase_write)
    void launch_wave_update(GpuBuffers& bufs, bool do_wave, bool do_coupling,
                            bool do_damping, bool selective_damping,
                            bool larmor_radiation, double damping_factor,
                            bool do_genesis, double dt);

    // Extended physics launchers
    void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate);
    void launch_pair_production(GpuBuffers& bufs);
    void launch_build_particle_list(GpuBuffers& bufs);
    void launch_color_force(GpuBuffers& bufs, int num_particles, double dt);
    void launch_yukawa_force(GpuBuffers& bufs, int num_particles, double dt);
    void launch_exchange_force(GpuBuffers& bufs, int num_particles, double dt);
    void launch_triad_detection(GpuBuffers& bufs, int num_particles);
}}}

namespace ftd {
namespace gpu {

// ---------- Construction / Destruction ----------

GpuEngine::GpuEngine(int lattice_size)
    : size_(lattice_size), N_(lattice_size * lattice_size * lattice_size)
{
    // Allocate device buffers
    bufs_.allocate(lattice_size);

    // Precompute FFT Green's function
    bufs_.precompute_green_function();

    // Create cuFFT plans (3D double-precision Z2Z — kept for reference/fallback)
    CUFFT_CHECK(cufftPlan3d(&fft_plan_forward_, size_, size_, size_, CUFFT_Z2Z));
    CUFFT_CHECK(cufftPlan3d(&fft_plan_inverse_, size_, size_, size_, CUFFT_Z2Z));

    // Create cuFFT plans (3D single-precision C2C — primary, 2× faster)
    CUFFT_CHECK(cufftPlan3d(&fft_plan_forward_f_, size_, size_, size_, CUFFT_C2C));
    CUFFT_CHECK(cufftPlan3d(&fft_plan_inverse_f_, size_, size_, size_, CUFFT_C2C));

    // Create cuRAND generator
    CURAND_CHECK(curandCreateGenerator(&rng_, CURAND_RNG_PSEUDO_DEFAULT));
    CURAND_CHECK(curandSetPseudoRandomGeneratorSeed(rng_, 42ULL));

    // Initialize host shadow
    host_voxels_.resize(N_);
    host_phi_.resize(N_, 0.0);
    host_phi_coulomb_.resize(N_, 0.0);
    host_phi_latency_.resize(N_, 0.0);
    host_dirty_ = false;
}

GpuEngine::~GpuEngine() {
    if (fft_plan_forward_) cufftDestroy(fft_plan_forward_);
    if (fft_plan_inverse_) cufftDestroy(fft_plan_inverse_);
    if (fft_plan_forward_f_) cufftDestroy(fft_plan_forward_f_);
    if (fft_plan_inverse_f_) cufftDestroy(fft_plan_inverse_f_);
    if (rng_) curandDestroyGenerator(rng_);
    bufs_.free();
}

// ---------- Core Simulation ----------

void GpuEngine::tick() {
    // Phase 1+2: Wave update (Laplacian + coupling + leapfrog + damping + genesis/evaporation)
    // NOTE: Fusion of phase_read + phase_write into a single kernel (wave_update_kernel)
    // was attempted but has a race condition: thread i reads flux[neighbor_j] while thread j
    // may have already written its updated flux[j] in the same kernel launch. The separate
    // kernel approach provides the necessary global barrier between reading neighbors (phase_read)
    // and writing self (phase_write). Double-buffering could fix this in the future.
    gpu_phase_read();
    gpu_phase_write();

    // Phase 2b: Pair production (correlated ±1 pairs from high-flux void)
    if (toggles.pair_production) {
        gpu_pair_production();
    }

    // Phase 3: Gauss constraint projection (FFT-based Poisson)
    if (toggles.gauss_projection) {
        gpu_gauss_project();
        // Propagate Gauss correction back to L/R substrates
        if (toggles.dual_substrate) {
            kernels::launch_gauss_sync_dual(bufs_);
        }
    }

    // Phase 3b: Latency Poisson (gravitational potential → voxel.latency)
    // Wave 5 (2026-04-14): GPU now implements solve_latency_poisson().
    // Tests that enable toggles.latency_field no longer need force_cpu().
    if (toggles.latency_field) {
        gpu_solve_latency_poisson();
    }

    // Phase 4: Forces (Coulomb Poisson + EM/gravity/Lorentz)
    if (toggles.forces) {
        gpu_phase_forces();
    }

    // Phase 4b: Pairwise forces (color, Yukawa, exchange) — requires particle list
    bool need_plist = toggles.color_forces || toggles.strong_force
                   || toggles.exchange_force || toggles.triad_binding;
    if (need_plist) {
        gpu_build_particle_list();
        gpu_particle_forces();
    }

    // Phase 4c: Triad binding detection
    if (toggles.triad_binding) {
        gpu_triad_detection();
    }

    // Phase 5: Movement
    if (toggles.movement) {
        gpu_phase_movement();
    }

    // Phase 6: Weak transmutation (stress-threshold polarity flip)
    if (toggles.weak_transmutation) {
        gpu_weak_transmutation();
    }

    tick_++;
    host_dirty_ = true;
}

void GpuEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) {
        tick();
    }
}

// ---------- GPU Tick Sub-Phases ----------

void GpuEngine::gpu_phase_read() {
    if (toggles.dual_substrate) {
        kernels::launch_phase_read_dual(bufs_,
                                        toggles.wave_propagation,
                                        toggles.coupling);
    } else {
        kernels::launch_phase_read(bufs_,
                                   toggles.wave_propagation,
                                   toggles.coupling);
    }
}

void GpuEngine::gpu_phase_write() {
    // Generate random numbers for genesis (cuRAND fills d_random before kernels use it)
    if (toggles.genesis) {
        CURAND_CHECK(curandGenerateUniformDouble(rng_, bufs_.d_random, N_));
    }

    double damping = 1.0 - ALPHA;
    if (toggles.dual_substrate) {
        kernels::launch_phase_write_dual(bufs_,
                                         toggles.damping,
                                         toggles.selective_damping,
                                         toggles.larmor_radiation,
                                         damping,
                                         toggles.genesis,
                                         dt_);
    } else {
        kernels::launch_phase_write(bufs_,
                                    toggles.damping,
                                    toggles.selective_damping,
                                    toggles.larmor_radiation,
                                    damping,
                                    toggles.genesis,
                                    dt_);
    }
}

// C4: DEAD CODE — This fused kernel has a known race condition (neighbor reads
// while other threads write in the same pass). It is never called from tick().
// The safe split approach (gpu_phase_read + gpu_phase_write) is used instead.
// Kept for reference only — do not call without implementing double-buffering.
void GpuEngine::gpu_wave_update() {
    // Generate random numbers for genesis (cuRAND fills d_random before kernels use it)
    if (toggles.genesis) {
        CURAND_CHECK(curandGenerateUniformDouble(rng_, bufs_.d_random, N_));
    }

    double damping = 1.0 - ALPHA;
    kernels::launch_wave_update(bufs_,
                                toggles.wave_propagation,
                                toggles.coupling,
                                toggles.damping,
                                toggles.selective_damping,
                                toggles.larmor_radiation,
                                damping,
                                toggles.genesis,
                                dt_);
}

void GpuEngine::gpu_gauss_project() {
    kernels::launch_gauss_project(bufs_,
                                  fft_plan_forward_,
                                  fft_plan_inverse_,
                                  fft_plan_forward_f_,
                                  fft_plan_inverse_f_);
}

void GpuEngine::gpu_solve_coulomb() {
    kernels::launch_solve_coulomb(bufs_,
                                  fft_plan_forward_,
                                  fft_plan_inverse_,
                                  fft_plan_forward_f_,
                                  fft_plan_inverse_f_);
}

// Wave 5 (2026-04-14): GPU latency Poisson solver.
// Unblocks every test that previously had to call rb.force_cpu() because
// CUDA lacked this feature. Matches CPU RenderBridge::solve_latency_poisson
// exactly up to the FFT's automatic DC-mode cancellation (gauge freedom).
void GpuEngine::gpu_solve_latency_poisson() {
    kernels::launch_solve_latency(bufs_,
                                  fft_plan_forward_,
                                  fft_plan_inverse_,
                                  fft_plan_forward_f_,
                                  fft_plan_inverse_f_);
}

void GpuEngine::gpu_phase_forces() {
    // Solve Coulomb potential first (if Poisson mode)
    if (toggles.poisson_coulomb) {
        gpu_solve_coulomb();
    }
    kernels::launch_phase_forces(bufs_,
                                 toggles.poisson_coulomb,
                                 toggles.gravity,
                                 toggles.lorentz_force,
                                 dt_);
}

void GpuEngine::gpu_phase_movement() {
    kernels::launch_phase_movement(bufs_, dt_);
}

// ---------- Extended Physics Sub-Phases ----------

void GpuEngine::gpu_weak_transmutation() {
    // Fill random buffer for stochastic flip
    CURAND_CHECK(curandGenerateUniformDouble(rng_, bufs_.d_random, N_));
    kernels::launch_weak_transmutation(bufs_, toggles.dual_substrate);
}

void GpuEngine::gpu_pair_production() {
    // Fill random buffer for stochastic pair creation
    CURAND_CHECK(curandGenerateUniformDouble(rng_, bufs_.d_random, N_));
    kernels::launch_pair_production(bufs_);
}

void GpuEngine::gpu_build_particle_list() {
    kernels::launch_build_particle_list(bufs_);
    // Sync particle count to host for subsequent kernel launches
    CUDA_CHECK(cudaMemcpy(&host_num_particles_, bufs_.d_num_particles,
                          sizeof(int), cudaMemcpyDeviceToHost));
    if (host_num_particles_ > GpuBuffers::MAX_PARTICLES)
        host_num_particles_ = GpuBuffers::MAX_PARTICLES;
}

void GpuEngine::gpu_particle_forces() {
    if (host_num_particles_ <= 0) return;

    if (toggles.color_forces) {
        kernels::launch_color_force(bufs_, host_num_particles_, dt_);
    }
    if (toggles.strong_force) {
        kernels::launch_yukawa_force(bufs_, host_num_particles_, dt_);
    }
    if (toggles.exchange_force) {
        kernels::launch_exchange_force(bufs_, host_num_particles_, dt_);
    }
}

void GpuEngine::gpu_triad_detection() {
    if (host_num_particles_ <= 0) return;
    kernels::launch_triad_detection(bufs_, host_num_particles_);
}

// ---------- Diagnostics ----------

void GpuEngine::ensure_host_synced() {
    if (host_dirty_) {
        bufs_.download(host_voxels_, host_phi_, host_phi_coulomb_);
        // Wave 5: also download phi_latency for tests that read it directly
        bufs_.download_phi_latency(host_phi_latency_);
        host_dirty_ = false;
    }
}

void GpuEngine::push_to_device() {
    bufs_.upload(host_voxels_, host_phi_, host_phi_coulomb_);
    host_dirty_ = false;
}

Diagnostics GpuEngine::diagnostics() {
    ensure_host_synced();

    // Compute diagnostics on host (reuses RenderBridge logic)
    Diagnostics d;
    d.tick = tick_;
    for (int i = 0; i < N_; ++i) {
        const auto& v = host_voxels_[i];
        d.total_flux += v.flux.mag();
        d.total_energy += v.flux.mag2() + v.wave_vel.mag2();
        if (v.state != 0) {
            d.manifested_count++;
            if (v.state > 0) d.positive_count++;
            if (v.state < 0) d.negative_count++;
            if (v.spin > 0) d.spin_up_count++;
            if (v.spin < 0) d.spin_down_count++;
            if (v.color >= 0 && v.color <= 3) d.color_count[v.color]++;
        }
    }
    return d;
}

// ──────────────────────────────────────────────────────────────────
// ENERGY-LEDGER PERFORMANCE NOTE (TRACKER §1.7 closed 2026-04-17)
// ──────────────────────────────────────────────────────────────────
// RenderBridge::tick() currently populates the per-tick EnergyLedger
// on the GPU path by calling gpu_sync_to_host() + update_energy_ledger().
// That's one full-voxel download per tick (~3 MB at L=64).
//
// If this ever shows up in a profile as a bottleneck, replace with a
// device-side reduction kernel that returns just three scalars:
//
//     __global__ void reduce_energy_sums(
//         int N, const double* fx, const double* fy, const double* fz,
//                const double* wx, const double* wy, const double* wz,
//                const double* vx, const double* vy, const double* vz,
//                const int8_t* state,
//                double* out_E_field, double* out_E_wave, double* out_E_kin);
//
// Use a classic block-shared-memory reduction (256-thread blocks,
// warp-level intrinsics for the last 32 lanes). After the kernel, one
// cudaMemcpy of 3 doubles (24 bytes) replaces the 3 MB download.
// ──────────────────────────────────────────────────────────────────

EnergyAudit GpuEngine::energy_audit() {
    ensure_host_synced();

    EnergyAudit ea;
    for (int i = 0; i < N_; ++i) {
        const auto& v = host_voxels_[i];
        ea.field_energy += v.flux.mag2();
        ea.wave_energy  += v.wave_vel.mag2();
        if (v.state != 0) {
            ea.particle_ke += 0.5 * v.velocity.mag2();
            ea.manifested_count++;
            ea.charge_total += v.state;
        }
        // Dual-substrate diagnostics
        if (toggles.dual_substrate) {
            ea.E_L_total += v.flux_L.mag2();
            ea.E_R_total += v.flux_R.mag2();
            ea.wv_L_total += v.wave_vel_L.mag2();
            ea.wv_R_total += v.wave_vel_R.mag2();
            ea.chirality_total += v.chirality_density();
        }
    }
    ea.total_energy = ea.field_energy + ea.wave_energy + ea.particle_ke;
    return ea;
}

// ---------- Injection ----------

void GpuEngine::inject_flux(int x, int y, int z, const Vec3& flux_val) {
    ensure_host_synced();
    int idx = ((x % size_ + size_) % size_) * size_ * size_
            + ((y % size_ + size_) % size_) * size_
            + ((z % size_ + size_) % size_);
    host_voxels_[idx].flux = flux_val;
    if (toggles.dual_substrate) {
        host_voxels_[idx].flux_L = flux_val * 0.5;
        host_voxels_[idx].flux_R = flux_val * 0.5;
    }
    push_to_device();
}

void GpuEngine::inject_particle(int x, int y, int z, int8_t state,
                                const Vec3& flux_val,
                                int8_t spin, int8_t color) {
    ensure_host_synced();

    int idx = ((x % size_ + size_) % size_) * size_ * size_
            + ((y % size_ + size_) % size_) * size_
            + ((z % size_ + size_) % size_);

    auto& v = host_voxels_[idx];
    v.state = state;
    v.flux = flux_val;
    v.spin = spin;
    v.color = color;
    v.particle_id = next_particle_id_++;

    // Dual-substrate: split flux between L and R per chirality
    if (toggles.dual_substrate) {
        double fL = (state > 0) ? (1.0 + DELTA_APPROX) * 0.5
                                : (1.0 - DELTA_APPROX) * 0.5;
        double fR = 1.0 - fL;
        v.flux_L = flux_val * fL;
        v.flux_R = flux_val * fR;
    }

    push_to_device();
}

void GpuEngine::inject_wavepacket(int cx, int cy, int cz, int8_t state,
                                  double sigma, double amplitude) {
    ensure_host_synced();

    // Match CPU RenderBridge::inject_wavepacket exactly
    int radius = static_cast<int>(3.0 * sigma) + 1;

    // Set state at center first
    int cidx = ((cx % size_ + size_) % size_) * size_ * size_
             + ((cy % size_ + size_) % size_) * size_
             + ((cz % size_ + size_) % size_);
    host_voxels_[cidx].state = state;
    host_voxels_[cidx].particle_id = next_particle_id_++;

    // First pass: L2 normalization (sum of g²)
    double norm_sum = 0.0;
    for (int dx = -radius; dx <= radius; ++dx)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > 3.0 * sigma) continue;
        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        norm_sum += g * g;
    }

    double scale = (norm_sum > 1e-30) ? amplitude / std::sqrt(norm_sum) : 0.0;

    // Dual-substrate fractions
    double fL_frac = 0.5, fR_frac = 0.5;
    if (toggles.dual_substrate) {
        fL_frac = (state > 0) ? (1.0 + DELTA_APPROX) * 0.5
                               : (1.0 - DELTA_APPROX) * 0.5;
        fR_frac = 1.0 - fL_frac;
    }

    // Second pass: set radial flux
    for (int dx = -radius; dx <= radius; ++dx)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > 3.0 * sigma) continue;

        int x = ((cx + dx) % size_ + size_) % size_;
        int y = ((cy + dy) % size_ + size_) % size_;
        int z = ((cz + dz) % size_ + size_) % size_;
        int idx = x * size_ * size_ + y * size_ + z;

        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        double mag = scale * g;
        Vec3 dJ = { mag * dx / r, mag * dy / r, mag * dz / r };
        host_voxels_[idx].flux.x += dJ.x;
        host_voxels_[idx].flux.y += dJ.y;
        host_voxels_[idx].flux.z += dJ.z;

        if (toggles.dual_substrate) {
            host_voxels_[idx].flux_L.x += dJ.x * fL_frac;
            host_voxels_[idx].flux_L.y += dJ.y * fL_frac;
            host_voxels_[idx].flux_L.z += dJ.z * fL_frac;
            host_voxels_[idx].flux_R.x += dJ.x * fR_frac;
            host_voxels_[idx].flux_R.y += dJ.y * fR_frac;
            host_voxels_[idx].flux_R.z += dJ.z * fR_frac;
        }
    }

    push_to_device();
}

// ---------- Sync ----------

void GpuEngine::sync_to_host(std::vector<Voxel>& out) {
    ensure_host_synced();
    out = host_voxels_;
}

void GpuEngine::upload_from_host(const std::vector<Voxel>& voxels) {
    host_voxels_ = voxels;
    push_to_device();
}

}  // namespace gpu
}  // namespace ftd

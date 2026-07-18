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
#include <cstdio>
#include <cmath>
#include <algorithm>

#include "cuda_error.cuh"  // CUDA_CHECK (revision C1 consolidation)

#define CUFFT_CHECK(call) do { \
    cufftResult err = (call); \
    if (err != CUFFT_SUCCESS) { \
        fprintf(stderr, "cuFFT error at %s:%d: %d\n", \
                __FILE__, __LINE__, (int)err); \
        exit(1); \
    } \
} while(0)



// Forward declarations of GPU kernel launchers (implemented in kernel files)
namespace ftd { namespace gpu { namespace kernels {
    void launch_phase_read(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                            uint8_t bcc_stencil_mode,
                            bool do_db_clock, bool do_db_clock_coulomb, double omega0);
    void launch_phase_write(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                            bool larmor_radiation, double damping_factor,
                            bool do_genesis, bool do_evaporation, double dt, bool symplectic_leapfrog,
                            bool do_langevin, double langevin_gamma, double langevin_T,
                            uint8_t langevin_site_filter,
                            double kinetic_drain,
                            unsigned long long rng_seed, int tick);
    void launch_gauss_project(GpuBuffers& bufs,
                              double charge_coupling,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_solve_coulomb(GpuBuffers& bufs,
                              double charge_scale,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_solve_latency(GpuBuffers& bufs,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_phase_forces(GpuBuffers& bufs, bool poisson_coulomb,
                             bool emergent_forces,
                             bool gravity, bool lorentz_force, double dt);
    void launch_phase_movement(GpuBuffers& bufs, double dt, bool reflective_boundary,
                               bool dual_substrate);
    // Dual-substrate launchers
    void launch_phase_read_dual(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                                bool do_db_clock, bool do_db_clock_coulomb, double omega0);
    void launch_phase_write_dual(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                                  bool larmor_radiation, double damping_factor,
                                  bool do_genesis, bool do_evaporation, double dt, bool symplectic_leapfrog,
                                  unsigned long long rng_seed, int tick);
    void launch_gauss_sync_dual(GpuBuffers& bufs);

    // Extended physics launchers
    void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate,
                                   unsigned long long rng_seed, int tick);
    void launch_pair_production(GpuBuffers& bufs, unsigned long long rng_seed, int tick);
    void launch_build_particle_list(GpuBuffers& bufs);
    void launch_color_force(GpuBuffers& bufs, int num_particles, double dt);
    void launch_yukawa_force(GpuBuffers& bufs, int num_particles, double dt);
    void launch_exchange_force(GpuBuffers& bufs, int num_particles, double dt);
    void launch_triad_detection(GpuBuffers& bufs, int num_particles);
    void launch_strong_field_stencil(GpuBuffers& bufs, double damp);
    void launch_weak_field_stencil(GpuBuffers& bufs, double damp);
    void launch_gather_probe_flux(const double* d_flux_x, const double* d_flux_y,
                                  const double* d_flux_z, const int* d_probe_idx,
                                  double* d_out_x, double* d_out_y, double* d_out_z,
                                  int n_probe);
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

    set_rng_seed(toggles.langevin_seed);

    // Initialize host shadow
    host_voxels_.resize(N_);
    host_phi_.resize(N_, 0.0);
    host_phi_coulomb_.resize(N_, 0.0);
    host_phi_latency_.resize(N_, 0.0);
    // Pre-size force-diag mirror so GpuBackend::sync_to_host can scatter even
    // before the first tick has populated the device buffers (e.g. when a
    // test mutates voxels()[] right after construction and triggers a sync).
    host_force_diag_.coulomb_x.assign(N_, 0.0);
    host_force_diag_.coulomb_y.assign(N_, 0.0);
    host_force_diag_.coulomb_z.assign(N_, 0.0);
    host_force_diag_.strong_x.assign(N_, 0.0);
    host_force_diag_.strong_y.assign(N_, 0.0);
    host_force_diag_.strong_z.assign(N_, 0.0);
    host_force_diag_.magnetic_x.assign(N_, 0.0);
    host_force_diag_.magnetic_y.assign(N_, 0.0);
    host_force_diag_.magnetic_z.assign(N_, 0.0);
    host_force_diag_.gravity_x.assign(N_, 0.0);
    host_force_diag_.gravity_y.assign(N_, 0.0);
    host_force_diag_.gravity_z.assign(N_, 0.0);
    host_force_diag_.exchange_x.assign(N_, 0.0);
    host_force_diag_.exchange_y.assign(N_, 0.0);
    host_force_diag_.exchange_z.assign(N_, 0.0);
    host_dirty_ = false;
}

GpuEngine::~GpuEngine() {
    if (fft_plan_forward_) cufftDestroy(fft_plan_forward_);
    if (fft_plan_inverse_) cufftDestroy(fft_plan_inverse_);
    if (fft_plan_forward_f_) cufftDestroy(fft_plan_forward_f_);
    if (fft_plan_inverse_f_) cufftDestroy(fft_plan_inverse_f_);
    spectro_free();
    free_gauge_links();
    bufs_.free();
}

void GpuEngine::set_dt(double dt) {
    // Mirror RenderBridge::set_dt: dt<1 is honored ONLY with symplectic_leapfrog,
    // which permits a CFL-stable sub-step (the plain leapfrog hardcodes dt=1 and
    // is unstable for dt>1·CFL). Pre-2026-06-20 this unconditionally clamped to
    // 1.0, so the GPU silently ran dt=1 even when the symplectic integrator and a
    // dt<1 were requested (e.g. campaign_atomic_spectroscopy at ω₀=1.5, dt=0.5) —
    // exciting the unstable high-k mode and diverging. Now the GPU honors the same
    // dt<1 the CPU does. toggles are synced before each tick (GpuBackend::tick),
    // and GpuBackend::set_dt syncs them before forwarding, so this read is fresh.
    dt_ = (toggles.symplectic_leapfrog || dt >= 1.0) ? dt : 1.0;
}

void GpuEngine::set_rng_seed(unsigned int seed) {
    if (rng_seed_initialized_ && rng_seed_ == seed) return;
    rng_seed_ = seed;
    rng_seed_initialized_ = true;
}

// ---------- Spectroscopy Probe Facility (FTD-0281 rung-b) ----------

void GpuEngine::spectro_free() {
    if (d_probe_idx_) { cudaFree(d_probe_idx_); d_probe_idx_ = nullptr; }
    if (d_probe_jx_)  { cudaFree(d_probe_jx_);  d_probe_jx_  = nullptr; }
    if (d_probe_jy_)  { cudaFree(d_probe_jy_);  d_probe_jy_  = nullptr; }
    if (d_probe_jz_)  { cudaFree(d_probe_jz_);  d_probe_jz_  = nullptr; }
    n_probe_ = 0;
}

void GpuEngine::spectro_set_probes(const std::vector<int>& probe_indices) {
    spectro_free();
    n_probe_ = static_cast<int>(probe_indices.size());
    if (n_probe_ <= 0) return;
    // Ensure the device flux is current (the campaign injects via the host shadow,
    // then ticks once before snapshotting J(0); upload_from_host already ran on the
    // first tick — but if J(0) is captured before any tick, push host state here).
    CUDA_CHECK(cudaMalloc(&d_probe_idx_, n_probe_ * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_probe_jx_,  n_probe_ * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_probe_jy_,  n_probe_ * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_probe_jz_,  n_probe_ * sizeof(double)));
    CUDA_CHECK(cudaMemcpy(d_probe_idx_, probe_indices.data(),
                          n_probe_ * sizeof(int), cudaMemcpyHostToDevice));
    probe_jx_.assign(n_probe_, 0.0);
    probe_jy_.assign(n_probe_, 0.0);
    probe_jz_.assign(n_probe_, 0.0);
    // Capture J(0): gather current device flux into the host J0 reference.
    kernels::launch_gather_probe_flux(bufs_.d_flux_x, bufs_.d_flux_y, bufs_.d_flux_z,
                                      d_probe_idx_, d_probe_jx_, d_probe_jy_, d_probe_jz_,
                                      n_probe_);
    probe_j0x_.assign(n_probe_, 0.0);
    probe_j0y_.assign(n_probe_, 0.0);
    probe_j0z_.assign(n_probe_, 0.0);
    CUDA_CHECK(cudaMemcpy(probe_j0x_.data(), d_probe_jx_, n_probe_ * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(probe_j0y_.data(), d_probe_jy_, n_probe_ * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(probe_j0z_.data(), d_probe_jz_, n_probe_ * sizeof(double), cudaMemcpyDeviceToHost));
}

double GpuEngine::spectro_autocorr() {
    if (n_probe_ <= 0) return 0.0;
    kernels::launch_gather_probe_flux(bufs_.d_flux_x, bufs_.d_flux_y, bufs_.d_flux_z,
                                      d_probe_idx_, d_probe_jx_, d_probe_jy_, d_probe_jz_,
                                      n_probe_);
    CUDA_CHECK(cudaMemcpy(probe_jx_.data(), d_probe_jx_, n_probe_ * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(probe_jy_.data(), d_probe_jy_, n_probe_ * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(probe_jz_.data(), d_probe_jz_, n_probe_ * sizeof(double), cudaMemcpyDeviceToHost));
    // Fixed-order host sum (deterministic; matches the CPU campaign's probe loop
    // order exactly since probe_indices were built in the same x,y,z scan order).
    double ct = 0.0;
    for (int p = 0; p < n_probe_; ++p) {
        ct += probe_j0x_[p] * probe_jx_[p]
            + probe_j0y_[p] * probe_jy_[p]
            + probe_j0z_[p] * probe_jz_[p];
    }
    return ct;
}

// ---------- Core Simulation ----------

void GpuEngine::tick() {
    bufs_.reset_continuity_ledger();
    continuity_ledger_valid_ = true;
    // Phase 1+2: Wave update (Laplacian + coupling + leapfrog + damping + genesis/evaporation)
    // NOTE: Fusion of phase_read + phase_write into a single kernel (wave_update_kernel)
    // was attempted but has a race condition: thread i reads flux[neighbor_j] while thread j
    // may have already written its updated flux[j] in the same kernel launch. The separate
    // kernel approach provides the necessary global barrier between reading neighbors (phase_read)
    // and writing self (phase_write). Double-buffering could fix this in the future.
    //
    // ORDERING GUARANTEE (revision C7): both launches go to the same CUDA
    // stream (the default stream throughout this engine), and CUDA serializes
    // kernels on one stream in issue order — phase_write can NEVER begin
    // before every phase_read thread has retired. No explicit sync primitive
    // is needed or wanted here; do NOT move these onto different streams
    // without re-introducing the barrier explicitly.
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

    // Phase 7b (revision 0.9 option a): non-Abelian gauge-link relaxation —
    // mirrors the CPU tick's Rule 7b. [IMPOSED] Wilson-action staple sweep;
    // links are write-only w.r.t. the substrate (no kernel reads them), so
    // this cannot perturb the GPU golden or parity domains. Requires the
    // device buffers primed by upload_gauge_links() — GpuBackend::tick()
    // does that on the first gauge-enabled tick; if an embedder drives
    // GpuEngine directly without priming, the phase is skipped.
    if ((toggles.su2_gauge || toggles.su3_gauge) && gauge_links_device_) {
        gpu_gauge_relax();
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
    // FTD-0281 (GPU port, 2026-06-20): when db_clock_coulomb is on, pre-solve the
    // live Coulomb Poisson field so the diagonal KG term in phase_read can read
    // V(r) = −phi_C on the SAME tick. Mirrors the CPU pre-read solve in
    // render_bridge.cpp:620-621 (toggles.db_clock_coulomb → solve_coulomb_poisson()).
    // Forces are validation-conflicted with db_clock_coulomb (see TOGGLE_SPECS),
    // so this is the only Coulomb solve on this tick — no double-solve.
    if (toggles.db_clock_coulomb) {
        gpu_solve_coulomb();
    }

    if (toggles.dual_substrate) {
        kernels::launch_phase_read_dual(bufs_,
                                        toggles.wave_propagation,
                                        toggles.coupling,
                                        toggles.de_broglie_clock,
                                        toggles.db_clock_coulomb,
                                        toggles.omega0);
    } else {
        kernels::launch_phase_read(bufs_,
                                   toggles.wave_propagation,
                                   toggles.coupling,
                                   static_cast<uint8_t>(toggles.bcc_stencil),
                                   toggles.de_broglie_clock,
                                   toggles.db_clock_coulomb,
                                   toggles.omega0);
    }
}

void GpuEngine::gpu_phase_write() {
    // BH-F5/F8/F9 (2026-05-05): cuRAND prefills removed. Genesis + Langevin
    // now use deterministic SplitMix64 RNG (engine/include/ftd/voxel_rng.h)
    // computed per-thread inside the kernel from (seed, voxel_idx, tick, salt).
    // Bit-exact CPU↔GPU at unit mass.

    const auto rng_seed = static_cast<unsigned long long>(toggles.langevin_seed);
    const int  tick     = static_cast<int>(tick_);

    double damping = 1.0 - ALPHA;
    if (toggles.dual_substrate) {
        // Dual-substrate Langevin is not wired in this pass — user-facing
        // scope for FTD-0051 v1 is single-substrate only.
        kernels::launch_phase_write_dual(bufs_,
                                         toggles.damping,
                                         toggles.selective_damping,
                                         toggles.larmor_radiation,
                                         damping,
                                         toggles.genesis,
                                         toggles.evaporation,
                                         dt_,
                                         toggles.symplectic_leapfrog,
                                         rng_seed, tick);
    } else {
        kernels::launch_phase_write(bufs_,
                                    toggles.damping,
                                    toggles.selective_damping,
                                    toggles.larmor_radiation,
                                    damping,
                                    toggles.genesis,
                                    toggles.evaporation,
                                    dt_,
                                    toggles.symplectic_leapfrog,
                                    toggles.langevin,
                                    toggles.langevin_gamma,
                                    toggles.langevin_T,
                                    static_cast<uint8_t>(toggles.langevin_site_filter),
                                    toggles.kinetic_drain,
                                    rng_seed, tick);
    }

    // Step strong field stencil (Stella Octangula propagation)
    if (toggles.color_forces || toggles.strong_force) {
        kernels::launch_strong_field_stencil(bufs_, damping);
    }

    // Step weak field stencil only when flavor or weak-field state exists.
    // The previous unconditional launch was a full-grid kernel every tick even
    // for ordinary EM/QCD runs where flavor==0 and weak fields are zero.
    if (weak_field_active_) {
        kernels::launch_weak_field_stencil(bufs_, damping);
    }
}



void GpuEngine::gpu_gauss_project() {
    kernels::launch_gauss_project(bufs_,
                                  toggles.coulomb_charge_coupling,
                                  fft_plan_forward_,
                                  fft_plan_inverse_,
                                  fft_plan_forward_f_,
                                  fft_plan_inverse_f_);
}

void GpuEngine::gpu_solve_coulomb() {
    kernels::launch_solve_coulomb(bufs_,
                                  toggles.coulomb_source_scale,  // FTD-0281 Z (He+ well)
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
    // Reset force-diag mirror once per tick — matches the per-tick semantics
    // of CPU RenderBridge::phase_forces, which overwrites force_diag_[i] for
    // every state≠0 voxel. Voxels with state==0 stay zero, which is the
    // sensible default (CPU reads of those would return whatever the last
    // tick wrote — typically also zero).
    bufs_.reset_force_diag();
    // Solve Coulomb potential first (if Poisson mode and not emergent).
    // emergent_forces and poisson_coulomb are mutually exclusive per
    // toggles.validate(); the explicit && !emergent_forces guard mirrors
    // CPU phase_forces_solve_potentials() and is robust to validate
    // being silenced (e.g. WASM strict_validation=false path).
    if (toggles.poisson_coulomb && !toggles.emergent_forces) {
        gpu_solve_coulomb();
    }
    kernels::launch_phase_forces(bufs_,
                                 toggles.poisson_coulomb,
                                 toggles.emergent_forces,
                                 toggles.gravity,
                                 toggles.lorentz_force,
                                 dt_);
}

void GpuEngine::gpu_phase_movement() {
    kernels::launch_phase_movement(bufs_, dt_, toggles.reflective_boundary,
                                   toggles.dual_substrate);
}

// ---------- Extended Physics Sub-Phases ----------

void GpuEngine::gpu_weak_transmutation() {
    const auto rng_seed = static_cast<unsigned long long>(toggles.langevin_seed);
    const int  tick     = static_cast<int>(tick_);
    kernels::launch_weak_transmutation(bufs_, toggles.dual_substrate, rng_seed, tick);
}

void GpuEngine::gpu_pair_production() {
    const auto rng_seed = static_cast<unsigned long long>(toggles.langevin_seed);
    const int  tick     = static_cast<int>(tick_);
    kernels::launch_pair_production(bufs_, rng_seed, tick);
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
        // Force-diag mirror — populated each tick by the force kernels so
        // GpuBackend::sync_to_host can scatter it into RenderBridge::force_diag_.
        bufs_.download_force_diag(
            host_force_diag_.coulomb_x,  host_force_diag_.coulomb_y,  host_force_diag_.coulomb_z,
            host_force_diag_.strong_x,   host_force_diag_.strong_y,   host_force_diag_.strong_z,
            host_force_diag_.magnetic_x, host_force_diag_.magnetic_y, host_force_diag_.magnetic_z,
            host_force_diag_.gravity_x,  host_force_diag_.gravity_y,  host_force_diag_.gravity_z,
            host_force_diag_.exchange_x, host_force_diag_.exchange_y, host_force_diag_.exchange_z);
        host_dirty_ = false;
    }
}

void GpuEngine::push_to_device() {
    bufs_.upload(host_voxels_, host_phi_, host_phi_coulomb_);
    host_dirty_ = false;
}

Diagnostics GpuEngine::diagnostics() {
    ensure_host_synced();

    // 2026-05-04 fix: parity with engine/src/diagnostics_compute.cpp:38.
    // Pre-fix this used `flux.mag2() + wave_vel.mag2()` (no 0.5, no
    // Born-Infeld). CPU diagnostic uses |born_infeld_core()| which is
    // a non-trivial functional of flux, wave_vel, and latency. The two
    // returned different total_energy values for the same scenario.
    Diagnostics d;
    d.tick = tick_;
    for (int i = 0; i < N_; ++i) {
        const auto& v = host_voxels_[i];
        d.total_flux += v.density();                     // = flux.mag(), matches CPU
        d.total_energy += std::abs(v.born_infeld_core()); // matches CPU
        double bw = v.bandwidth_used();
        if (bw > d.max_bandwidth) d.max_bandwidth = bw;
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

    // Match the canonical 1/2 |·|² convention used by
    // engine/src/diagnostics_compute.cpp:98-99 (compute_energy_audit) and
    // engine/web/js/bridge/mock-diagnostics.js. Pre-2026-05-03 this kernel
    // dropped the 1/2 on every quadratic-energy diagnostic, making
    // GpuEngine::energy_audit() report 2× the RenderBridge value for the
    // same scenario — caught by test_gpu_parity GP2/GP3/GP4/GP5 all showing
    // an exact 2:1 mismatch and test_wavepacket WP1/WP3 showing 50% of the
    // expected K_B² normalization. Mirrors the same fix that was applied
    // to compute_energy_audit on 2026-04-27.
    EnergyAudit ea;
    for (int i = 0; i < N_; ++i) {
        const auto& v = host_voxels_[i];
        ea.field_energy += 0.5 * v.flux.mag2();
        ea.wave_energy  += 0.5 * v.wave_vel.mag2();
        if (v.state != 0) {
            ea.particle_ke += 0.5 * v.velocity.mag2();
            ea.manifested_count++;
            ea.charge_total += v.state;
        }
        // Dual-substrate diagnostics — same 1/2 |·|² convention.
        if (toggles.dual_substrate) {
            ea.E_L_total += 0.5 * v.flux_L.mag2();
            ea.E_R_total += 0.5 * v.flux_R.mag2();
            ea.wv_L_total += 0.5 * v.wave_vel_L.mag2();
            ea.wv_R_total += 0.5 * v.wave_vel_R.mag2();
            ea.chirality_total += v.chirality_density();
        }

        // Strong field diagnostic
        if (toggles.color_forces || toggles.strong_force) {
            ea.strong_energy += 0.5 * v.flux_strong.mag2();
        }

        // Weak field diagnostic
        ea.weak_energy += 0.5 * v.flux_weak.mag2();
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
                                int8_t spin, int8_t color, int8_t flavor) {
    ensure_host_synced();

    int idx = ((x % size_ + size_) % size_) * size_ * size_
            + ((y % size_ + size_) % size_) * size_
            + ((z % size_ + size_) % size_);

    auto& v = host_voxels_[idx];
    v.state = state;
    v.flux = flux_val;
    v.spin = spin;
    v.color = color;
    v.flavor = flavor;
    v.particle_id = next_particle_id_++;
    if (flavor != 0) weak_field_active_ = true;

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

eft::DualCellContinuity GpuEngine::continuity_step() const {
    if (!continuity_ledger_valid_) return eft::DualCellContinuity{};

    eft::DualCellContinuity out(size_);
    bufs_.download_continuity_ledger(out.rho_before,
                                     out.rho_after,
                                     out.reaction,
                                     out.current_x,
                                     out.current_y,
                                     out.current_z);
    return out;
}

void GpuEngine::upload_from_host(const std::vector<Voxel>& voxels) {
    // C5 (CUDA ticket): host_voxels_ currently mirrors the device SoA — it was
    // populated by the last ensure_host_synced()/sync_to_host() download, and
    // nothing mutates the device between that download and this call (this is
    // invoked at tick start via GpuBackend::flush_host_mutations, or from the
    // post-sync cluster-inertia push). So we diff the incoming host array
    // against host_voxels_ and upload ONLY the changed voxels — byte-identical
    // to the previous full push_to_device() because the device already holds
    // the correct bytes at every unchanged index. A single-voxel edit uploads
    // ~325 B instead of the whole ~40-array voxel image (85 MB at L=64).
    //
    // Cold start (host_voxels_ empty) falls back to a full upload inside
    // upload_voxels_delta. phi/phi_coulomb are intentionally NOT re-uploaded
    // here (unlike the old push_to_device()): they are device-authoritative and
    // already equal host_phi_/host_phi_coulomb_ from the last sync, so skipping
    // them is byte-identical AND keeps the single-voxel path off the two N-sized
    // potential arrays (2×2 MB at L=64) that would otherwise blow the <<1 MB
    // budget.
    bufs_.upload_voxels_delta(voxels, host_voxels_);
    host_voxels_ = voxels;
    refresh_weak_field_active_from_host();
    host_dirty_ = false;   // device now equals host_voxels_
    continuity_ledger_valid_ = false;
}

void GpuEngine::refresh_weak_field_active_from_host() {
    weak_field_active_ = false;
    for (const auto& v : host_voxels_) {
        if (v.flavor != 0 ||
            v.flux_weak.mag2() > 0.0 ||
            v.wave_vel_weak.mag2() > 0.0) {
            weak_field_active_ = true;
            return;
        }
    }
}

// ---------- Non-Abelian gauge link sector (revision 0.9 option a) ----------

// Double-buffered launchers defined in kernels_gauge.cu (extern "C" linkage).
extern "C" void launch_relax_su2_links(
    const SU2Link* src_x, const SU2Link* src_y, const SU2Link* src_z,
    SU2Link* dst_x, SU2Link* dst_y, SU2Link* dst_z,
    int L, double dt, double beta, cudaStream_t stream);
extern "C" void launch_relax_su3_links(
    const SU3Link* src_x, const SU3Link* src_y, const SU3Link* src_z,
    SU3Link* dst_x, SU3Link* dst_y, SU3Link* dst_z,
    int L, double dt, double beta, cudaStream_t stream);

void GpuEngine::upload_gauge_links(const std::vector<SU2Link>& su2_x,
                                   const std::vector<SU2Link>& su2_y,
                                   const std::vector<SU2Link>& su2_z,
                                   const std::vector<SU3Link>& su3_x,
                                   const std::vector<SU3Link>& su3_y,
                                   const std::vector<SU3Link>& su3_z) {
    const std::size_t bytes2 = static_cast<std::size_t>(N_) * sizeof(SU2Link);
    const std::size_t bytes3 = static_cast<std::size_t>(N_) * sizeof(SU3Link);
    if (!gauge_links_device_) {
        // Lazy allocation (mirrors CPU revision 4.1b): live + Jacobi scratch.
        for (int d = 0; d < 3; ++d) {
            CUDA_CHECK(cudaMalloc(&d_su2_[d],     bytes2));
            CUDA_CHECK(cudaMalloc(&d_su2_scr_[d], bytes2));
            CUDA_CHECK(cudaMalloc(&d_su3_[d],     bytes3));
            CUDA_CHECK(cudaMalloc(&d_su3_scr_[d], bytes3));
        }
        gauge_links_device_ = true;
    }
    const std::vector<SU2Link>* h2[3] = {&su2_x, &su2_y, &su2_z};
    const std::vector<SU3Link>* h3[3] = {&su3_x, &su3_y, &su3_z};
    for (int d = 0; d < 3; ++d) {
        CUDA_CHECK(cudaMemcpy(d_su2_[d], h2[d]->data(), bytes2, cudaMemcpyHostToDevice));
        CUDA_CHECK(cudaMemcpy(d_su3_[d], h3[d]->data(), bytes3, cudaMemcpyHostToDevice));
    }
}

void GpuEngine::download_gauge_links(std::vector<SU2Link>& su2_x,
                                     std::vector<SU2Link>& su2_y,
                                     std::vector<SU2Link>& su2_z,
                                     std::vector<SU3Link>& su3_x,
                                     std::vector<SU3Link>& su3_y,
                                     std::vector<SU3Link>& su3_z) const {
    if (!gauge_links_device_) return;
    const std::size_t bytes2 = static_cast<std::size_t>(N_) * sizeof(SU2Link);
    const std::size_t bytes3 = static_cast<std::size_t>(N_) * sizeof(SU3Link);
    std::vector<SU2Link>* h2[3] = {&su2_x, &su2_y, &su2_z};
    std::vector<SU3Link>* h3[3] = {&su3_x, &su3_y, &su3_z};
    for (int d = 0; d < 3; ++d) {
        if (h2[d]->size() != static_cast<std::size_t>(N_)) h2[d]->resize(N_);
        if (h3[d]->size() != static_cast<std::size_t>(N_)) h3[d]->resize(N_);
        CUDA_CHECK(cudaMemcpy(h2[d]->data(), d_su2_[d], bytes2, cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(h3[d]->data(), d_su3_[d], bytes3, cudaMemcpyDeviceToHost));
    }
}

void GpuEngine::gpu_gauge_relax() {
    if (!gauge_links_device_) return;
    // One Jacobi sweep per enabled group per tick, matching the CPU Rule 7b
    // (same GAUGE_RELAX_DT/GAUGE_RELAX_BETA constants). Default stream so the
    // sweep serializes with the rest of the tick's kernels (same-stream
    // ordering guarantee); src -> scratch, then pointer swap.
    if (toggles.su2_gauge) {
        launch_relax_su2_links(d_su2_[0], d_su2_[1], d_su2_[2],
                               d_su2_scr_[0], d_su2_scr_[1], d_su2_scr_[2],
                               size_, GAUGE_RELAX_DT, GAUGE_RELAX_BETA, 0);
        CUDA_CHECK(cudaGetLastError());
        for (int d = 0; d < 3; ++d) std::swap(d_su2_[d], d_su2_scr_[d]);
    }
    if (toggles.su3_gauge) {
        launch_relax_su3_links(d_su3_[0], d_su3_[1], d_su3_[2],
                               d_su3_scr_[0], d_su3_scr_[1], d_su3_scr_[2],
                               size_, GAUGE_RELAX_DT, GAUGE_RELAX_BETA, 0);
        CUDA_CHECK(cudaGetLastError());
        for (int d = 0; d < 3; ++d) std::swap(d_su3_[d], d_su3_scr_[d]);
    }
}

void GpuEngine::free_gauge_links() {
    for (int d = 0; d < 3; ++d) {
        if (d_su2_[d])     { cudaFree(d_su2_[d]);     d_su2_[d] = nullptr; }
        if (d_su2_scr_[d]) { cudaFree(d_su2_scr_[d]); d_su2_scr_[d] = nullptr; }
        if (d_su3_[d])     { cudaFree(d_su3_[d]);     d_su3_[d] = nullptr; }
        if (d_su3_scr_[d]) { cudaFree(d_su3_scr_[d]); d_su3_scr_[d] = nullptr; }
    }
    gauge_links_device_ = false;
}

}  // namespace gpu
}  // namespace ftd

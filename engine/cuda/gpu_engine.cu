/**
 * @file gpu_engine.cu
 * @brief GPU-accelerated FTD tick engine.
 *
 * [EXTENDED] Phase 1: Shell implementation. tick() delegates to CPU via download/upload
 * until GPU kernels are implemented in Phases 2-4.
 */

#include "ftd/gpu_engine.h"
#include "ftd/constants.h"
#include "ftd/volumetric_measure.h"
#include <cuda_runtime.h>
#include <cub/device/device_scan.cuh>
#include <thrust/iterator/transform_iterator.h>
#include <cufft.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

#define FTD_CUDA_ERROR_WANT_CUFFT
#include "cuda_error.cuh"  // CUDA_CHECK + recoverable CUFFT_CHECK



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
                             double genesis_threshold,
                             double manifest_scale,
                             unsigned long long rng_seed);
    void launch_gauss_project(GpuBuffers& bufs,
                              double charge_coupling,
                              bool exact_dual_gauss,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_solve_coulomb(GpuBuffers& bufs,
                              double charge_scale,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_solve_latency(GpuBuffers& bufs,
                              bool include_field_energy,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_phase_forces(GpuBuffers& bufs, bool poisson_coulomb,
                             bool emergent_forces,
                             bool gravity, bool lorentz_force, double dt);
    void launch_integrate_forces(GpuBuffers& bufs, double dt);
    void launch_phase_movement(GpuBuffers& bufs, double dt, bool reflective_boundary,
                               bool dual_substrate);
    void launch_ew_background_sweep(GpuBuffers& bufs, double drive,
                                    bool dual_substrate);
    void launch_absorbing_boundary(GpuBuffers& bufs);
    void launch_reflective_flux_boundary(GpuBuffers& bufs);
    void launch_dispersal_flux_boundary(GpuBuffers& bufs);
    // Dual-substrate launchers
    void launch_phase_read_dual(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                                bool do_db_clock, bool do_db_clock_coulomb, double omega0);
    void launch_phase_write_dual(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                                  bool larmor_radiation, double damping_factor,
                                  bool do_genesis, bool do_evaporation, double dt, bool symplectic_leapfrog,
                                  unsigned long long rng_seed);
    void launch_gauss_sync_dual(GpuBuffers& bufs);

    // Extended physics launchers
    void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate,
                                   unsigned long long rng_seed);
    void launch_pair_production(GpuBuffers& bufs, bool dual_substrate,
                                unsigned long long rng_seed);
    void launch_advance_device_tick(GpuBuffers& bufs);
    void launch_build_particle_list(GpuBuffers& bufs);
    void launch_color_force(GpuBuffers& bufs, double dt);
    void launch_yukawa_force(GpuBuffers& bufs, double dt);
    void launch_exchange_force(GpuBuffers& bufs, double dt);
    void launch_triad_detection(GpuBuffers& bufs);
    void launch_strong_field_stencil(GpuBuffers& bufs, double damp);
    void launch_weak_field_stencil(GpuBuffers& bufs, double damp);
    void launch_gather_probe_flux(const double* d_flux_x, const double* d_flux_y,
                                  const double* d_flux_z, const int* d_probe_idx,
                                  double* d_out_x, double* d_out_y, double* d_out_z,
                                  int n_probe, cudaStream_t stream);
    void launch_compact_diagnostics(GpuBuffers& bufs, int tick, bool movement,
                                    Diagnostics& out);
    void launch_compact_energy_audit(GpuBuffers& bufs,
                                     const TermToggles& toggles,
                                     EnergyAudit& out);
    void launch_compact_gravity_metric(GpuBuffers& bufs,
                                       const TermToggles& toggles,
                                       GravityMetricAgg& out);
    void launch_compact_lagrangian(GpuBuffers& bufs, LagrangianDiag& out);
    void launch_telemetry_snapshot(GpuBuffers& bufs, std::uint32_t groups,
                                   const TermToggles& toggles,
                                   cudaEvent_t ready_event);
    void decode_telemetry_snapshot(const GpuBuffers& bufs,
                                   const TelemetrySnapshotRequest& request,
                                   int tick, std::uint64_t state_version,
                                   bool gravity_requested,
                                   TelemetrySnapshot& out);
    void launch_compact_voxel(GpuBuffers& bufs, int index,
                              VoxelInspection& out);
    void launch_compact_force(GpuBuffers& bufs, int index, ForceDiag& out);
    void launch_accumulate_proper_time(GpuBuffers& bufs, bool update_phase,
                                       double omega0);
    void launch_inject_flux(GpuBuffers& bufs, int index, const Vec3& value,
                            bool dual, bool additive);
    void launch_inject_wave_velocity(GpuBuffers& bufs, int index,
                                     const Vec3& value, bool dual);
    void launch_inject_particle(GpuBuffers& bufs, int index, int8_t state,
                                const Vec3& flux, int8_t spin, int8_t color,
                                int8_t flavor, bool dual);
    void launch_inject_wavepacket(GpuBuffers& bufs, int cx, int cy, int cz,
                                  int8_t state, double sigma, double scale,
                                  int radius, bool dual);
    bool launch_inject_entangled_pair(
        GpuBuffers& bufs, int primary, const Vec3& flux, bool dual);
}}}

namespace ftd {
namespace gpu {

std::size_t g_gpu_visual_snapshot_download_bytes = 0;
std::size_t g_gpu_visual_snapshot_launches = 0;

namespace {

struct ParticleFlagToInt {
    __host__ __device__ int operator()(std::uint8_t flag) const {
        return static_cast<int>(flag);
    }
};

__global__ void visual_particle_flags_kernel(const std::int8_t* state,
                                             std::uint8_t* flags, int N) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < N) flags[index] = state[index] != 0 ? 1u : 0u;
}

__global__ void visual_particle_header_kernel(
    const std::uint8_t* flags, const std::int32_t* prefix, int N,
    std::uint32_t max_particles, VisualParticleStagingHeader* header) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    const std::uint32_t manifested = static_cast<std::uint32_t>(
        prefix[N - 1] + static_cast<std::int32_t>(flags[N - 1]));
    header->total_manifested = manifested;
    header->captured_count = manifested < max_particles
        ? manifested : max_particles;
}

__global__ void visual_particle_gather_kernel(
    const std::int8_t* state, const std::int8_t* spin,
    const std::int8_t* color, const double* remainder_x,
    const double* remainder_y, const double* remainder_z,
    const std::int32_t* prefix, int N,
    const VisualParticleStagingHeader* header,
    VisualParticleRecord* records) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= N || state[index] == 0) return;

    const std::uint32_t manifested = header->total_manifested;
    const std::uint32_t selected = header->captured_count;
    if (manifested == 0 || selected == 0) return;

    // Match the host visual selection accumulator exactly.  For one-based
    // manifested rank r, an item is retained iff floor(r*C/M) advances over
    // floor((r-1)*C/M), where M is total manifested and C is the bounded cap.
    // The prefix scan gives r-1 in ascending lattice-index order, so the
    // selected slot is deterministic without atomic insertion ordering.
    const std::uint64_t previous_rank = static_cast<std::uint64_t>(prefix[index]);
    const std::uint64_t current_rank = previous_rank + 1u;
    const std::uint64_t previous_bucket =
        previous_rank * static_cast<std::uint64_t>(selected) / manifested;
    const std::uint64_t current_bucket =
        current_rank * static_cast<std::uint64_t>(selected) / manifested;
    if (current_bucket == previous_bucket) return;

    const std::uint32_t slot = static_cast<std::uint32_t>(current_bucket - 1u);
    VisualParticleRecord record;
    record.index = index;
    record.state = state[index];
    record.spin = spin[index];
    record.color = color[index];
    record.remainder_x = static_cast<float>(remainder_x[index]);
    record.remainder_y = static_cast<float>(remainder_y[index]);
    record.remainder_z = static_cast<float>(remainder_z[index]);
    records[slot] = record;
}

void launch_visual_particle_capture(GpuBuffers& bufs,
                                    std::uint32_t requested_cap,
                                    cudaEvent_t ready_event) {
    const std::uint32_t cap = requested_cap == 0
        ? kMaxVisualParticleCapture
        : (std::min)(requested_cap, kMaxVisualParticleCapture);
    constexpr int threads = 256;
    const int blocks = (bufs.N + threads - 1) / threads;
    visual_particle_flags_kernel<<<blocks, threads>>>(
        bufs.d_state, bufs.d_pair_candidate_flags, bufs.N);
    CUDA_CHECK(cudaGetLastError());

    // `d_pair_candidate_flags` and `d_pair_candidate_indices` are persistent
    // serialized scratch owned by GpuBuffers.  Allocation reserved the larger
    // of this CUB scan and the pair/genesis select workspace at engine setup.
    const auto flags = thrust::make_transform_iterator(
        bufs.d_pair_candidate_flags, ParticleFlagToInt{});
    CUDA_CHECK(cub::DeviceScan::ExclusiveSum(
        bufs.d_pair_select_temp, bufs.pair_select_temp_bytes,
        flags, bufs.d_pair_candidate_indices, bufs.N));

    visual_particle_header_kernel<<<1, 1>>>(
        bufs.d_pair_candidate_flags, bufs.d_pair_candidate_indices, bufs.N,
        cap, bufs.d_visual_particle_header);
    CUDA_CHECK(cudaGetLastError());
    visual_particle_gather_kernel<<<blocks, threads>>>(
        bufs.d_state, bufs.d_spin, bufs.d_color,
        bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
        bufs.d_pair_candidate_indices, bufs.N,
        bufs.d_visual_particle_header, bufs.d_visual_particle_records);
    CUDA_CHECK(cudaGetLastError());

    // A fixed bounded copy keeps the entire request asynchronous: waiting for
    // a count first would require a host round trip before the record D2H can
    // be issued.  Polling is event-only and copies just the valid prefix into
    // the public vector once the fence is complete.
    CUDA_CHECK(cudaMemcpyAsync(
        bufs.h_visual_particle_header, bufs.d_visual_particle_header,
        sizeof(VisualParticleStagingHeader), cudaMemcpyDeviceToHost));
    constexpr std::size_t record_bytes =
        static_cast<std::size_t>(kMaxVisualParticleCapture)
        * sizeof(VisualParticleRecord);
    CUDA_CHECK(cudaMemcpyAsync(
        bufs.h_visual_particle_records, bufs.d_visual_particle_records,
        record_bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaEventRecord(ready_event));
    g_gpu_visual_snapshot_download_bytes +=
        sizeof(VisualParticleStagingHeader) + record_bytes;
    ++g_gpu_visual_snapshot_launches;
}

}  // namespace

// ---------- Construction / Destruction ----------

GpuEngine::GpuEngine(int lattice_size)
    : size_(lattice_size), N_(lattice_size * lattice_size * lattice_size)
{
    try {
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

    // Bind every plan to the engine stream so cufftExec* is stream-ordered
    // with the surrounding kernels and is capturable. cufftPlan3d allocates
    // the plan work area at creation time, so no allocation happens at exec
    // time (which capture would reject).
    CUFFT_CHECK(cufftSetStream(fft_plan_forward_,   bufs_.stream));
    CUFFT_CHECK(cufftSetStream(fft_plan_inverse_,   bufs_.stream));
    CUFFT_CHECK(cufftSetStream(fft_plan_forward_f_, bufs_.stream));
    CUFFT_CHECK(cufftSetStream(fft_plan_inverse_f_, bufs_.stream));

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
    } catch (...) {
        // A later cuFFT plan can fail after earlier plans and GpuBuffers have
        // succeeded. Destroy the completed plan prefix; GpuBuffers' destructor
        // releases its allocation prefix while the exception unwinds.
        if (fft_plan_forward_) cufftDestroy(fft_plan_forward_);
        if (fft_plan_inverse_) cufftDestroy(fft_plan_inverse_);
        if (fft_plan_forward_f_) cufftDestroy(fft_plan_forward_f_);
        if (fft_plan_inverse_f_) cufftDestroy(fft_plan_inverse_f_);
        fft_plan_forward_ = fft_plan_inverse_ = 0;
        fft_plan_forward_f_ = fft_plan_inverse_f_ = 0;
        throw;
    }
}

GpuEngine::~GpuEngine() {
    // A visual capture uses the default stream and reads the engine's SoA
    // source buffers.  NativeVisualScheduler must keep a bridge alive until
    // visual_snapshot_safe_to_replace() says its event has retired.  If an
    // owner violates that barrier, do not synchronize in a destructor (which
    // could freeze recovery); GpuBuffers::free() preserves the live source.
    // Only an actual CUDA event error becomes a terminal quarantine.
    if (visual_snapshot_pending_ && bufs_.visual_snapshot_ready) {
        const cudaError_t status = cudaEventQuery(bufs_.visual_snapshot_ready);
        if (status != cudaSuccess) {
            bufs_.free();  // nonblocking; NotReady is a barrier violation
            return;
        }
    }
    if (fft_plan_forward_) cufftDestroy(fft_plan_forward_);
    if (fft_plan_inverse_) cufftDestroy(fft_plan_inverse_);
    if (fft_plan_forward_f_) cufftDestroy(fft_plan_forward_f_);
    if (fft_plan_inverse_f_) cufftDestroy(fft_plan_inverse_f_);
    spectro_free();
    free_gauge_links();
    bufs_.free();
}

int GpuEngine::device_tick() const {
    int value = 0;
    CUDA_CHECK(cudaMemcpy(&value, bufs_.d_tick, sizeof(int),
                          cudaMemcpyDeviceToHost));
    return value;
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
                                      n_probe_, bufs_.stream);
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
                                      n_probe_, bufs_.stream);
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

    // CPU parity: the electroweak scenario's uniform +x drive is part of the
    // tick input, not a visualization mutation. Apply it before phase_read so
    // the same-tick wave/KG operators consume the driven field. In dual mode
    // the symmetric half split is essential because phase_read reads L/R.
    if (toggles.ew_background_sweep) {
        const double drive = (std::sin(static_cast<double>(tick_) * 0.01) + 1.0)
                           * 0.5 * 0.05;
        kernels::launch_ew_background_sweep(bufs_, drive,
                                            toggles.dual_substrate);
    }

    // Phase 1+2: Wave update (Laplacian + coupling + leapfrog + damping + genesis/evaporation)
    // NOTE: Fusion of phase_read + phase_write into a single kernel (wave_update_kernel)
    // was attempted but has a race condition: thread i reads flux[neighbor_j] while thread j
    // may have already written its updated flux[j] in the same kernel launch. The separate
    // kernel approach provides the necessary global barrier between reading neighbors (phase_read)
    // and writing self (phase_write). Double-buffering could fix this in the future.
    //
    // ORDERING GUARANTEE (revision C7): both launches go to the same CUDA
    // stream (the engine's dedicated stream, bufs_.stream), and CUDA
    // serializes kernels on one stream in issue order — phase_write can
    // NEVER begin before every phase_read thread has retired. No explicit
    // sync primitive is needed or wanted here; do NOT move these onto
    // different streams without re-introducing the barrier explicitly.
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

    const bool any_force = toggles.forces || toggles.color_forces
                        || toggles.strong_force || toggles.exchange_force;
    if (any_force) bufs_.reset_force_diag();

    // Phase 4: Force accumulation (Coulomb Poisson + EM/gravity/Lorentz)
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

    // FTD-0402: every active force channel feeds one momentum update.
    if (any_force) kernels::launch_integrate_forces(bufs_, dt_);

    // Phase 4c: Triad binding detection
    if (toggles.triad_binding) {
        gpu_triad_detection();
    }

    // Phase 5: Movement
    if (toggles.movement) {
        gpu_phase_movement();
    }

    // CPU parity: field boundaries run after the final ordinary flux writer
    // (Gauss/forces/movement), so projection cannot refill the damped shell.
    // The sponge and selected flux boundary are sequential and may coexist.
    if (toggles.absorbing_boundary) {
        kernels::launch_absorbing_boundary(bufs_);
    }
    if (toggles.flux_boundary == FluxBoundaryMode::Reflective) {
        kernels::launch_reflective_flux_boundary(bufs_);
    } else if (toggles.flux_boundary == FluxBoundaryMode::Dispersal) {
        kernels::launch_dispersal_flux_boundary(bufs_);
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

    // CPU Rule 8 parity without a full voxel mirror. tau and the optional
    // de-Broglie phase advance together on-device exactly once per tick.
    if (toggles.latency_field || toggles.de_broglie_clock) {
        accumulate_proper_time(toggles.de_broglie_clock, toggles.omega0);
    }

    // Advance the device mirror in the same place the host counter moves, so
    // a captured graph replays with a fresh RNG salt.
    kernels::launch_advance_device_tick(bufs_);

    tick_++;
    host_dirty_ = true;
    mark_device_state_changed();
}

void GpuEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) {
        tick();
    }
}

void GpuEngine::accumulate_proper_time(bool update_phase, double omega0) {
    kernels::launch_accumulate_proper_time(bufs_, update_phase, omega0);
    host_dirty_ = true;
    mark_device_state_changed();
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

    double damping = 1.0 - ALPHA;
    const double genesis_threshold = genesis_threshold_override > 0.0
        ? genesis_threshold_override : K_GENESIS;
    const double manifest_scale = manifest_use_temperature
        ? std::max(toggles.langevin_T, 1e-12)
        : (manifest_scale_override > 0.0
               ? manifest_scale_override : K_MANIFEST);
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
                                         rng_seed);
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
                                    genesis_threshold,
                                    manifest_scale,
                                    rng_seed);
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
                                  toggles.exact_dual_gauss,
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
                                  toggles.field_energy_gravity,
                                  fft_plan_forward_,
                                  fft_plan_inverse_,
                                  fft_plan_forward_f_,
                                  fft_plan_inverse_f_);
}

void GpuEngine::gpu_phase_forces() {
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
    kernels::launch_weak_transmutation(bufs_, toggles.dual_substrate, rng_seed);
}

void GpuEngine::gpu_pair_production() {
    const auto rng_seed = static_cast<unsigned long long>(toggles.langevin_seed);
    kernels::launch_pair_production(bufs_, toggles.dual_substrate, rng_seed);
}

void GpuEngine::gpu_build_particle_list() {
    // No host readback: the count stays on the device and the pairwise/triad
    // kernels bound themselves from it. Capacity overflow is a sticky device
    // flag surfaced by throw_if_particle_overflow() at the existing
    // synchronization boundaries (ensure_host_synced /
    // causal_projection_events), not by a blocking copy inside the tick.
    kernels::launch_build_particle_list(bufs_);
}

void GpuEngine::gpu_particle_forces() {
    if (toggles.color_forces) {
        kernels::launch_color_force(bufs_, dt_);
    }
    if (toggles.strong_force) {
        kernels::launch_yukawa_force(bufs_, dt_);
    }
    if (toggles.exchange_force) {
        kernels::launch_exchange_force(bufs_, dt_);
    }
}

void GpuEngine::gpu_triad_detection() {
    kernels::launch_triad_detection(bufs_);
}

// ---------- Diagnostics ----------

void GpuEngine::ensure_host_synced() {
    bufs_.throw_if_identity_error();
    bufs_.throw_if_particle_overflow();
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
    int32_t max_particle_id = -1;
    int32_t max_pair_id = -1;
    for (const auto& voxel : host_voxels_) {
        max_particle_id = std::max(max_particle_id, voxel.particle_id);
        max_pair_id = std::max(max_pair_id, voxel.pair_id);
    }
    if (max_particle_id == std::numeric_limits<int32_t>::max()
        || max_pair_id == std::numeric_limits<int32_t>::max()) {
        throw std::overflow_error(
            "host-staged identity leaves no representable GPU successor ID");
    }
    bufs_.upload(host_voxels_, host_phi_, host_phi_coulomb_);
    bufs_.raise_identity_counters(max_particle_id + 1, max_pair_id + 1);
    host_dirty_ = false;
    mark_device_state_changed();
}

Diagnostics GpuEngine::diagnostics() {
    Diagnostics d;
    kernels::launch_compact_diagnostics(bufs_, tick_, toggles.movement, d);
    return d;
}

// ──────────────────────────────────────────────────────────────────
// ENERGY-LEDGER PERFORMANCE NOTE (TRACKER §1.7 closed 2026-04-17)
// ──────────────────────────────────────────────────────────────────
// RenderBridge::tick() currently populates the per-tick EnergyLedger
// on the GPU path by calling gpu_sync_to_host() + update_energy_ledger().
// That used to require a full voxel download per tick (~87 MiB at L=64).
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
    EnergyAudit ea;
    kernels::launch_compact_energy_audit(bufs_, toggles, ea);
    return ea;
}

GravityMetricAgg GpuEngine::gravity_metric_agg() {
    GravityMetricAgg out;
    kernels::launch_compact_gravity_metric(bufs_, toggles, out);
    return out;
}

void GpuEngine::lagrangian_diagnostics(LagrangianDiag& out) {
    kernels::launch_compact_lagrangian(bufs_, out);
}

bool GpuEngine::begin_telemetry_snapshot(
    const TelemetrySnapshotRequest& requested) {
    if (telemetry_snapshot_pending_) return false;

    TelemetrySnapshotRequest request = requested;
    request.groups &= TELEMETRY_ALL;
    if (request.groups == 0) request.groups = TELEMETRY_DIAGNOSTICS;
    if (request.dt <= 0.0) request.dt = dt_;
    if (request.lattice_size <= 0) request.lattice_size = size_;

    // This launch is deliberately issued after all preceding default-stream
    // simulation work. It does not synchronize the host: the publisher owns
    // the event polling cadence and may queue the next tick after the copy.
    kernels::launch_telemetry_snapshot(bufs_, request.groups, toggles,
                                       bufs_.telemetry_snapshot_ready);
    telemetry_snapshot_request_ = request;
    telemetry_snapshot_state_version_ = state_version_;
    telemetry_snapshot_tick_ = tick_;
    telemetry_snapshot_gravity_requested_ = toggles.latency_field
                                         || toggles.field_energy_gravity;
    telemetry_snapshot_pending_ = true;
    return true;
}

bool GpuEngine::telemetry_snapshot_ready() const {
    if (!telemetry_snapshot_pending_) return false;
    const cudaError_t status = cudaEventQuery(bufs_.telemetry_snapshot_ready);
    if (status == cudaSuccess) return true;
    if (status == cudaErrorNotReady) return false;
    throw std::runtime_error(std::string("[GpuEngine] telemetry event query failed: ")
                             + cudaGetErrorString(status));
}

bool GpuEngine::poll_telemetry_snapshot(TelemetrySnapshot& out) {
    if (!telemetry_snapshot_ready()) return false;
    kernels::decode_telemetry_snapshot(
        bufs_, telemetry_snapshot_request_, telemetry_snapshot_tick_,
        telemetry_snapshot_state_version_, telemetry_snapshot_gravity_requested_, out);
    telemetry_snapshot_pending_ = false;
    return true;
}

void GpuEngine::wait_telemetry_snapshot(TelemetrySnapshot& out) {
    if (!telemetry_snapshot_pending_) {
        throw std::logic_error("[GpuEngine] no telemetry snapshot is pending");
    }
    CUDA_CHECK(cudaEventSynchronize(bufs_.telemetry_snapshot_ready));
    kernels::decode_telemetry_snapshot(
        bufs_, telemetry_snapshot_request_, telemetry_snapshot_tick_,
        telemetry_snapshot_state_version_, telemetry_snapshot_gravity_requested_, out);
    telemetry_snapshot_pending_ = false;
}

TelemetrySnapshot GpuEngine::telemetry_snapshot(
    const TelemetrySnapshotRequest& request) {
    if (!begin_telemetry_snapshot(request)) {
        throw std::logic_error(
            "[GpuEngine] cannot synchronously request telemetry while a snapshot is pending");
    }
    TelemetrySnapshot out;
    wait_telemetry_snapshot(out);
    return out;
}

bool GpuEngine::begin_visual_snapshot(
    const VisualSnapshotRequest& requested) {
    if (visual_snapshot_pending_
        || requested.kind != VisualCaptureKind::Particles) {
        return false;
    }

    VisualSnapshotRequest request = requested;
    if (request.max_particles == 0) {
        request.max_particles = kMaxVisualParticleCapture;
    } else {
        request.max_particles = (std::min)(request.max_particles,
                                           kMaxVisualParticleCapture);
    }
    if (request.dt <= 0.0) request.dt = dt_;
    if (request.lattice_size <= 0) request.lattice_size = size_;

    // All work is appended to the default stream after preceding simulation
    // work.  No host synchronization occurs here; source provenance is
    // captured now and decoded only after the visual event completes.
    launch_visual_particle_capture(bufs_, request.max_particles,
                                   bufs_.visual_snapshot_ready);
    visual_snapshot_request_ = request;
    visual_snapshot_state_version_ = state_version_;
    visual_snapshot_tick_ = tick_;
    visual_snapshot_pending_ = true;
    return true;
}

bool GpuEngine::visual_snapshot_ready() const {
    if (!visual_snapshot_pending_) return false;
    const cudaError_t status = cudaEventQuery(bufs_.visual_snapshot_ready);
    if (status == cudaSuccess) return true;
    if (status == cudaErrorNotReady) return false;
    throw std::runtime_error(std::string("[GpuEngine] visual capture event query failed: ")
                             + cudaGetErrorString(status));
}

bool GpuEngine::visual_snapshot_safe_to_replace() const {
    if (!visual_snapshot_pending_) return true;
    const cudaError_t status = cudaEventQuery(bufs_.visual_snapshot_ready);
    if (status == cudaSuccess) return true;
    if (status == cudaErrorNotReady) return false;
    throw std::runtime_error(std::string("[GpuEngine] visual capture source barrier failed: ")
                             + cudaGetErrorString(status));
}

bool GpuEngine::poll_visual_snapshot(VisualSnapshot& out) {
    if (!visual_snapshot_ready()) return false;
    if (visual_snapshot_request_.kind != VisualCaptureKind::Particles) {
        throw std::logic_error("[GpuEngine] unsupported completed visual capture kind");
    }

    const VisualParticleStagingHeader header = *bufs_.h_visual_particle_header;
    const std::uint32_t cap = (std::min)(
        visual_snapshot_request_.max_particles, kMaxVisualParticleCapture);
    const std::uint32_t count = (std::min)(header.captured_count, cap);

    out = {};
    out.kind = VisualCaptureKind::Particles;
    out.meta.epoch = visual_snapshot_request_.epoch;
    out.meta.state_version = visual_snapshot_state_version_;
    out.meta.tick = visual_snapshot_tick_;
    out.meta.physical_time = visual_snapshot_request_.physical_time;
    out.meta.dt = visual_snapshot_request_.dt;
    out.meta.lattice_size = visual_snapshot_request_.lattice_size;
    out.particles.total_manifested = header.total_manifested;
    out.particles.records.assign(bufs_.h_visual_particle_records,
                                 bufs_.h_visual_particle_records + count);
    visual_snapshot_pending_ = false;
    return true;
}

void GpuEngine::inspect_voxel(int index, VoxelInspection& out) {
    kernels::launch_compact_voxel(bufs_, index, out);
}

void GpuEngine::inspect_force(int index, ForceDiag& out) {
    kernels::launch_compact_force(bufs_, index, out);
}

// ---------- Injection ----------

void GpuEngine::inject_flux(int x, int y, int z, const Vec3& flux_val) {
    const auto wrap = [&](int value) {
        value %= size_;
        return value < 0 ? value + size_ : value;
    };
    const int idx = wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
    kernels::launch_inject_flux(bufs_, idx, flux_val,
                                toggles.dual_substrate, false);
    host_dirty_ = true;
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
}

void GpuEngine::inject_flux_add(int x, int y, int z,
                                const Vec3& flux_val) {
    const auto wrap = [&](int value) {
        value %= size_;
        return value < 0 ? value + size_ : value;
    };
    const int idx = wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
    kernels::launch_inject_flux(bufs_, idx, flux_val,
                                toggles.dual_substrate, true);
    host_dirty_ = true;
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
}

void GpuEngine::inject_wave_vel_add(int x, int y, int z,
                                    const Vec3& wave_vel) {
    const auto wrap = [&](int value) {
        value %= size_;
        return value < 0 ? value + size_ : value;
    };
    const int idx = wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
    kernels::launch_inject_wave_velocity(bufs_, idx, wave_vel,
                                         toggles.dual_substrate);
    host_dirty_ = true;
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
}

void GpuEngine::inject_particle(int x, int y, int z, int8_t state,
                                const Vec3& flux_val,
                                int8_t spin, int8_t color, int8_t flavor) {
    const auto wrap = [&](int value) {
        value %= size_;
        return value < 0 ? value + size_ : value;
    };
    const int idx = wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
    if (flavor != 0) weak_field_active_ = true;
    kernels::launch_inject_particle(bufs_, idx, state, flux_val,
                                    spin, color, flavor, toggles.dual_substrate);
    host_dirty_ = true;
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
}

void GpuEngine::inject_wavepacket(int cx, int cy, int cz, int8_t state,
                                  double sigma, double amplitude) {
    // Match CPU RenderBridge::inject_wavepacket exactly
    int radius = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;

    // First pass: L2 normalization (sum of g²)
    double norm_sum = 0.0;
    for (int dx = -radius; dx <= radius; ++dx)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dz = -radius; dz <= radius; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r > GAUSSIAN_CUTOFF_SIGMA * sigma) continue;
        double g = std::exp(-r2 / (2.0 * sigma * sigma));
        norm_sum += g * g;
    }

    double scale = (norm_sum > 1e-30) ? amplitude / std::sqrt(norm_sum) : 0.0;

    kernels::launch_inject_wavepacket(
        bufs_, cx, cy, cz, state, sigma, scale, radius,
        toggles.dual_substrate);
    host_dirty_ = true;
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
}

void GpuEngine::create_entangled_pair(int x, int y, int z,
                                      const Vec3& flux_val) {
    const auto wrap = [&](int value) {
        value %= size_;
        return value < 0 ? value + size_ : value;
    };
    const int idx = wrap(x) * size_ * size_ + wrap(y) * size_ + wrap(z);
    kernels::launch_inject_entangled_pair(bufs_, idx, flux_val,
                                          toggles.dual_substrate);
    host_dirty_ = true;
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
}

// ---------- Sync ----------

void GpuEngine::sync_to_host(std::vector<Voxel>& out) {
    ensure_host_synced();
    out = host_voxels_;
}

void GpuEngine::identity_counters(int32_t& next_particle_id,
                                  int32_t& next_pair_id) const {
    bufs_.throw_if_identity_error();
    bufs_.download_identity_counters(next_particle_id, next_pair_id);
}

void GpuEngine::raise_identity_counters(int32_t next_particle_id,
                                        int32_t next_pair_id) {
    bufs_.raise_identity_counters(next_particle_id, next_pair_id);
}

void GpuEngine::copy_visual_states(std::vector<std::int8_t>& out) const {
    bufs_.download_states(out);
}

void GpuEngine::copy_visual_flux_magnitude(std::vector<float>& out) {
    bufs_.download_flux_magnitude(out);
}

void GpuEngine::copy_visual_flux_magnitude_plane(
    int axis, int index, std::vector<float>& out) {
    bufs_.download_flux_magnitude_plane(axis, index, out);
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
    // ~333 B instead of the whole ~40-array voxel image (87 MiB at L=64).
    //
    // Cold start (host_voxels_ empty) falls back to a full upload inside
    // upload_voxels_delta. phi/phi_coulomb are intentionally NOT re-uploaded
    // here (unlike the old push_to_device()): they are device-authoritative and
    // already equal host_phi_/host_phi_coulomb_ from the last sync, so skipping
    // them is byte-identical AND keeps the single-voxel path off the two N-sized
    // potential arrays (2×2 MB at L=64) that would otherwise blow the <<1 MB
    // budget.
    int32_t max_particle_id = -1;
    int32_t max_pair_id = -1;
    for (const auto& voxel : voxels) {
        if (voxel.particle_id > max_particle_id)
            max_particle_id = voxel.particle_id;
        if (voxel.pair_id > max_pair_id)
            max_pair_id = voxel.pair_id;
    }
    if (max_particle_id == std::numeric_limits<int32_t>::max()
        || max_pair_id == std::numeric_limits<int32_t>::max()) {
        throw std::overflow_error(
            "host-staged identity leaves no representable GPU successor ID");
    }

    bufs_.upload_voxels_delta(voxels, host_voxels_);
    bufs_.raise_identity_counters(max_particle_id + 1, max_pair_id + 1);
    host_voxels_ = voxels;
    refresh_weak_field_active_from_host();
    host_dirty_ = false;   // device now equals host_voxels_
    continuity_ledger_valid_ = false;
    mark_device_state_changed();
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
    try {
        if (!gauge_links_device_) {
            // Lazy allocation (mirrors CPU revision 4.1b): live + Jacobi
            // scratch. Publish gauge_links_device_ only after every allocation
            // and upload succeeds; otherwise a partial cudaMalloc prefix would
            // leak and the next interactive tick would overwrite its pointers.
            for (int d = 0; d < 3; ++d) {
                CUDA_CHECK(cudaMalloc(&d_su2_[d],     bytes2));
                CUDA_CHECK(cudaMalloc(&d_su2_scr_[d], bytes2));
                CUDA_CHECK(cudaMalloc(&d_su3_[d],     bytes3));
                CUDA_CHECK(cudaMalloc(&d_su3_scr_[d], bytes3));
            }
        }

        const std::vector<SU2Link>* h2[3] = {&su2_x, &su2_y, &su2_z};
        const std::vector<SU3Link>* h3[3] = {&su3_x, &su3_y, &su3_z};
        for (int d = 0; d < 3; ++d) {
            CUDA_CHECK(cudaMemcpy(d_su2_[d], h2[d]->data(), bytes2, cudaMemcpyHostToDevice));
            CUDA_CHECK(cudaMemcpy(d_su3_[d], h3[d]->data(), bytes3, cudaMemcpyHostToDevice));
        }
        gauge_links_device_ = true;
    } catch (...) {
        // free_gauge_links() is null-safe and clears every pointer plus the
        // publication flag, making an allocation failure recoverable/retryable.
        free_gauge_links();
        throw;
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
    // (same GAUGE_RELAX_DT/GAUGE_RELAX_BETA constants). Engine stream so the
    // sweep serializes with the rest of the tick's kernels (same-stream
    // ordering guarantee); src -> scratch, then pointer swap.
    if (toggles.su2_gauge) {
        launch_relax_su2_links(d_su2_[0], d_su2_[1], d_su2_[2],
                               d_su2_scr_[0], d_su2_scr_[1], d_su2_scr_[2],
                               size_, GAUGE_RELAX_DT, GAUGE_RELAX_BETA, bufs_.stream);
        CUDA_CHECK(cudaGetLastError());
        for (int d = 0; d < 3; ++d) std::swap(d_su2_[d], d_su2_scr_[d]);
    }
    if (toggles.su3_gauge) {
        launch_relax_su3_links(d_su3_[0], d_su3_[1], d_su3_[2],
                               d_su3_scr_[0], d_su3_scr_[1], d_su3_scr_[2],
                               size_, GAUGE_RELAX_DT, GAUGE_RELAX_BETA, bufs_.stream);
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

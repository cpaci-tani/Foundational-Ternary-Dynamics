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
#include "ftd/interop_particle_record.h"
#include <cuda_runtime.h>
#include <cub/device/device_scan.cuh>
#include <thrust/iterator/transform_iterator.h>
#include <cufft.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <iostream>

#define FTD_CUDA_ERROR_WANT_CUFFT
#include "cuda_error.cuh"  // CUDA_CHECK + recoverable CUFFT_CHECK



// Forward declarations of GPU kernel launchers (implemented in kernel files)
namespace ftd { namespace gpu { namespace kernels {
    void launch_phase_read(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                            uint8_t bcc_stencil_mode,
                            bool do_db_clock, bool do_db_clock_coulomb, double omega0,
                            bool period2_floquet, bool bcc_time_floquet);
    void launch_phase_write(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                            bool larmor_radiation, double damping_factor,
                            bool do_genesis, bool do_evaporation, double dt,
                            bool symplectic_leapfrog, bool verlet_wave_integrator,
                            bool do_langevin, double langevin_gamma, double langevin_T,
                            uint8_t langevin_site_filter,
                            double kinetic_drain,
                            double genesis_threshold,
                            double manifest_scale,
                            unsigned long long rng_seed);
    void launch_verlet_second_half_kick(GpuBuffers& bufs, double dt, bool dual);
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
                              const double* strong_t00,
                              cufftHandle plan_fwd, cufftHandle plan_inv,
                              cufftHandle plan_fwd_f, cufftHandle plan_inv_f);
    void launch_phase_forces(GpuBuffers& bufs, bool poisson_coulomb,
                             bool emergent_forces,
                             bool gravity, bool geometric_gravity,
                             bool lorentz_force, double dt);
    void launch_integrate_forces(GpuBuffers& bufs, double dt);
    void launch_cluster_inertia(GpuBuffers& bufs, double dt);
    void launch_phase_movement(GpuBuffers& bufs, double dt, bool reflective_boundary,
                               bool dual_substrate, bool symmetric_movement_order,
                               unsigned long long langevin_seed);
    void launch_ew_background_sweep(GpuBuffers& bufs, bool dual_substrate);
    void launch_absorbing_boundary(GpuBuffers& bufs);
    void launch_reflective_flux_boundary(GpuBuffers& bufs);
    void launch_dispersal_flux_boundary(GpuBuffers& bufs);
    // Dual-substrate launchers
    void launch_phase_read_dual(const GpuBuffers& bufs, bool do_wave, bool do_coupling,
                                bool do_db_clock, bool do_db_clock_coulomb, double omega0,
                                bool period2_floquet, bool bcc_time_floquet);
    void launch_phase_write_dual(GpuBuffers& bufs, bool do_damping, bool selective_damping,
                                  bool larmor_radiation, double damping_factor,
                                  bool do_genesis, bool do_evaporation, double dt,
                                  bool symplectic_leapfrog, bool verlet_wave_integrator,
                                  unsigned long long rng_seed);
    void launch_gauss_sync_dual(GpuBuffers& bufs);

    // Extended physics launchers
    void launch_weak_transmutation(GpuBuffers& bufs, bool dual_substrate,
                                   unsigned long long rng_seed);
    void launch_pair_production(GpuBuffers& bufs, bool dual_substrate,
                                unsigned long long rng_seed);
    void launch_advance_device_tick(GpuBuffers& bufs);
    void launch_build_particle_list(GpuBuffers& bufs);
    void launch_color_force(GpuBuffers& bufs, double dt, bool linear_confinement,
                            bool continuous_remainder);
    void launch_begin_strong_energy(GpuBuffers& bufs, bool movement, bool config_valid);
    void launch_complete_strong_energy(GpuBuffers& bufs);
    void launch_strong_t00(GpuBuffers& bufs);
    void launch_matched_gauss_advance(GpuBuffers& bufs, double wave_speed, double dt);
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

// Interior-occlusion test for the visual cull (VisualSnapshotRequest::
// interior_cull_layers). A manifested site is "buried" when, along all 6 axis
// directions, the next `layers` voxels are all manifested and in-bounds; such a
// site is fully occluded by the shell around it and is dropped from the gather.
// `layers <= 0` short-circuits to false, so the cull is a no-op (flags reduce to
// state != 0, bit-identical to before). Index packing matches Lattice::index /
// interop_particle_gather_kernel: z=idx%L, y=(idx/L)%L, x=idx/L^2.
__device__ __forceinline__ bool visual_site_buried_d(
    const std::int8_t* __restrict__ state, int L, int index, int layers) {
    if (layers <= 0) return false;
    const int cz = index % L;
    const int cy = (index / L) % L;
    const int cx = index / (L * L);
    for (int d = 0; d < 6; ++d) {
        const int axis = d >> 1;                 // 0=x, 1=y, 2=z
        const int sgn = (d & 1) ? -1 : 1;
        for (int s = 1; s <= layers; ++s) {
            int nx = cx, ny = cy, nz = cz;
            if (axis == 0) nx += sgn * s;
            else if (axis == 1) ny += sgn * s;
            else nz += sgn * s;
            if (nx < 0 || nx >= L || ny < 0 || ny >= L || nz < 0 || nz >= L)
                return false;  // lattice edge → surface, keep
            if (state[(nx * L + ny) * L + nz] == 0) return false;  // void → keep
        }
    }
    return true;  // solid along all 6 axes for `layers` steps → buried
}

__global__ void visual_particle_flags_kernel(const std::int8_t* state,
                                             std::uint8_t* flags, int N,
                                             int L, int cull_layers) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index >= N) return;
    flags[index] = (state[index] != 0
                    && !visual_site_buried_d(state, L, index, cull_layers))
                       ? 1u : 0u;
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
    const std::int8_t* state, const std::uint8_t* flags, const std::int8_t* spin,
    const std::int8_t* color, const double* remainder_x,
    const double* remainder_y, const double* remainder_z,
    const std::int32_t* prefix, int N,
    const VisualParticleStagingHeader* header,
    VisualParticleRecord* records) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    // Gate on `flags` (manifested AND not culled), not raw state: the prefix scan
    // and header count are over the flagged pool, so a buried voxel must skip here
    // too or its slot math would be wrong. flags==0 ⊇ state==0.
    if (index >= N || flags[index] == 0) return;

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

// Mirrors NativeEngineSession::capture()'s per-particle color assignment
// (engine/native/src/engine_session.cpp) and Lattice::coord()'s
// (engine/include/ftd/lattice.h) world-position decode exactly, so
// interop-rendered and CPU-rendered particles are visually identical -- this
// is the fact Task 11's validation test checks. Lattice::index(x,y,z) packs
// x*L^2 + y*L + z, so the inverse (Lattice::coord()) is z = idx % L,
// y = (idx / L) % L, x = idx / (L * L) -- x is the SLOWEST-varying digit,
// not the fastest, so it is deliberately decoded last below.
__global__ void interop_particle_gather_kernel(
    const std::int8_t* __restrict__ state,
    const std::uint8_t* __restrict__ flags,
    const double* __restrict__ remainder_x,
    const double* __restrict__ remainder_y,
    const double* __restrict__ remainder_z,
    const std::int32_t* __restrict__ prefix, int N, int L,
    const VisualParticleStagingHeader* __restrict__ header,
    InteropParticleRecord* __restrict__ records,
    InteropParticleHeader* __restrict__ interop_header) {
    const int index = blockIdx.x * blockDim.x + threadIdx.x;

    if (index == 0) {
        // Same bounded cap as the existing header kernel writes for the
        // shared path -- captured_count there is already min(visible, cap).
        // This write precedes the flags gate so it still runs when flags[0]==0.
        interop_header->captured_count = header->captured_count;
    }

    // Gate on `flags` (manifested AND not culled) — see visual_particle_gather_kernel.
    if (index >= N || flags[index] == 0) return;
    const std::uint32_t manifested = header->total_manifested;
    const std::uint32_t selected = header->captured_count;
    if (manifested == 0 || selected == 0) return;

    const std::uint64_t previous_rank = static_cast<std::uint64_t>(prefix[index]);
    const std::uint64_t current_rank = previous_rank + 1u;
    const std::uint64_t previous_bucket =
        previous_rank * static_cast<std::uint64_t>(selected) / manifested;
    const std::uint64_t current_bucket =
        current_rank * static_cast<std::uint64_t>(selected) / manifested;
    if (current_bucket == previous_bucket) return;
    const std::uint32_t slot = static_cast<std::uint32_t>(current_bucket - 1u);

    // Decode lattice index -> integer coord -> world position exactly as
    // Lattice::coord()/NativeEngineSession::capture() do on the CPU today:
    // z = index % L, y = (index / L) % L, x = index / (L*L); voxel center is
    // coord + 0.5, plus the sub-voxel remainder.
    const int cz = index % L;
    const int cy = (index / L) % L;
    const int cx = index / (L * L);

    InteropParticleRecord rec;
    rec.x = static_cast<float>(cx) + 0.5f + static_cast<float>(remainder_x[index]);
    rec.y = static_cast<float>(cy) + 0.5f + static_cast<float>(remainder_y[index]);
    rec.z = static_cast<float>(cz) + 0.5f + static_cast<float>(remainder_z[index]);
    rec.size = 0.55f;
    if (state[index] >= 0) {
        rec.r = 0.29f; rec.g = 0.87f; rec.b = 0.50f;
    } else {
        rec.r = 0.97f; rec.g = 0.44f; rec.b = 0.44f;
    }
    records[slot] = rec;
}

void launch_visual_particle_capture(GpuBuffers& bufs,
                                    std::uint32_t requested_cap,
                                    int lattice_size,
                                    std::uint16_t cull_layers,
                                    cudaEvent_t ready_event) {
    const std::uint32_t cap = requested_cap == 0
        ? kMaxVisualParticleCapture
        : (std::min)(requested_cap, kMaxVisualParticleCapture);
    constexpr int threads = 256;
    const int blocks = (bufs.N + threads - 1) / threads;
    visual_particle_flags_kernel<<<blocks, threads>>>(
        bufs.d_state, bufs.d_pair_candidate_flags, bufs.N,
        lattice_size, static_cast<int>(cull_layers));
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
        bufs.d_state, bufs.d_pair_candidate_flags, bufs.d_spin, bufs.d_color,
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

void launch_interop_particle_gather(GpuBuffers& bufs, int lattice_size,
                                    std::uint32_t requested_cap,
                                    std::uint16_t cull_layers) {
    // The write cap must never exceed the ACTUAL element capacity of the
    // imported D3D12 buffer -- kMaxVisualParticleCapture alone bounds the
    // unrelated CPU-decode staging array (d_visual_particle_records), not
    // this call's destination (bufs.d_interop_particle_buffer, a mapped view
    // whose real size is bufs.interop_particle_capacity, set by
    // GpuEngine::import_d3d12_particle_buffer() from the D3D12-side
    // resource's exact byte size). Clamping against both means a caller
    // that passes requested_cap==0 (or any value larger than the imported
    // buffer) can never make the gather kernel write past the end of memory
    // shared with D3D12. If nothing has been imported yet,
    // interop_particle_capacity is 0 and this clamps the effective cap to 0,
    // so the header reports zero captured particles and no record write
    // happens at all -- the safe outcome for a call that should not have
    // reached here (GpuEngine::interop_gather_particles() already refuses to
    // call this before an import succeeds; this clamp is defense in depth).
    const std::uint32_t import_cap =
        (std::min)(bufs.interop_particle_capacity, kMaxVisualParticleCapture);
    const std::uint32_t cap = requested_cap == 0
        ? import_cap
        : (std::min)(requested_cap, import_cap);
    constexpr int threads = 256;
    const int blocks = (bufs.N + threads - 1) / threads;

    // Same flags -> prefix-scan -> header sequence as
    // launch_visual_particle_capture -- deliberately duplicated rather than
    // factored into a shared helper, because this project's engine code
    // favors readable duplication over a helper with hidden branching in a
    // hot path (matches the existing style already in this file).
    //
    // IMPORTANT: this sequence writes into the SAME scratch
    // launch_visual_particle_capture uses -- bufs.d_visual_particle_header
    // (via the same visual_particle_header_kernel) and the same
    // d_pair_candidate_flags / d_pair_candidate_indices / d_pair_select_temp
    // CUB workspace. The two launchers do NOT have different header scratch:
    // d_interop_header is a separate, ADDITIONAL 4-byte host-visible summary
    // that only interop_particle_gather_kernel's own final `if (index==0)`
    // branch populates, by copying FROM d_visual_particle_header's
    // captured_count -- it is not an alternate destination for this shared
    // sequence.
    //
    // Reusing that scratch across two independently-callable launchers is
    // race-free TODAY only because bufs.stream is created with plain
    // cudaStreamCreate (blocking, not cudaStreamNonBlocking -- see
    // gpu_buffers.cu) while launch_visual_particle_capture's kernels run on
    // the legacy default stream (no stream argument): CUDA's
    // synchronizing-stream semantics serialize a blocking stream's work
    // against the default stream in host issue order (the same invariant
    // GpuBuffers::stream's own doc comment in gpu_buffers.h states for
    // Component A generally). That is an incidental consequence of
    // bufs.stream's creation flags, not an explicit synchronization
    // primitive (no event, no stream wait) between these two functions. It
    // breaks silently -- corrupting the selection/count of either or both
    // captures, no crash to signal it -- if launch_visual_particle_capture
    // ever moves off the default stream, if bufs.stream is ever created with
    // cudaStreamNonBlocking (the natural choice for a dedicated
    // graph-capture stream), or if the CPU-decode capture path and this
    // interop gather are ever invoked concurrently (plausible once a future
    // task wires interop into the render/debug loop). Whoever changes either
    // side of that must give the two launchers independent scratch or an
    // explicit ordering primitive.
    visual_particle_flags_kernel<<<blocks, threads, 0, bufs.stream>>>(
        bufs.d_state, bufs.d_pair_candidate_flags, bufs.N,
        lattice_size, static_cast<int>(cull_layers));
    CUDA_CHECK(cudaGetLastError());

    const auto flags = thrust::make_transform_iterator(
        bufs.d_pair_candidate_flags, ParticleFlagToInt{});
    CUDA_CHECK(cub::DeviceScan::ExclusiveSum(
        bufs.d_pair_select_temp, bufs.pair_select_temp_bytes,
        flags, bufs.d_pair_candidate_indices, bufs.N, bufs.stream));

    visual_particle_header_kernel<<<1, 1, 0, bufs.stream>>>(
        bufs.d_pair_candidate_flags, bufs.d_pair_candidate_indices, bufs.N,
        cap, bufs.d_visual_particle_header);
    CUDA_CHECK(cudaGetLastError());

    interop_particle_gather_kernel<<<blocks, threads, 0, bufs.stream>>>(
        bufs.d_state, bufs.d_pair_candidate_flags,
        bufs.d_remainder_x, bufs.d_remainder_y, bufs.d_remainder_z,
        bufs.d_pair_candidate_indices, bufs.N, lattice_size,
        bufs.d_visual_particle_header,
        static_cast<InteropParticleRecord*>(bufs.d_interop_particle_buffer),
        bufs.d_interop_header);
    CUDA_CHECK(cudaGetLastError());

    // Only the 4-byte count needs a host round trip -- this is the whole
    // point of the interop path: no per-particle download, no per-particle
    // CPU decode. Compare to launch_visual_particle_capture's D2H of the
    // full kMaxVisualParticleCapture-sized record array.
    CUDA_CHECK(cudaMemcpyAsync(bufs.h_interop_header, bufs.d_interop_header,
                              sizeof(InteropParticleHeader),
                              cudaMemcpyDeviceToHost, bufs.stream));
    CUDA_CHECK(cudaEventRecord(bufs.interop_gather_ready, bufs.stream));
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
    // Graph execs reference the stream and the device buffers; retire them
    // before GpuBuffers::free() drains and destroys the stream. The
    // visual_snapshot_pending_ quarantine branch above (`bufs_.free(); return;`)
    // skips this call, same as it already skips the fft_plan_*/spectro_free/
    // free_gauge_links cleanup below it for the identical reason: that path
    // hands the whole allocation off to CUDA-context replacement rather than
    // resolve it here. The graph cache leaking there is consistent with the
    // rest of this destructor's established quarantine behavior, not a new
    // gap.
    destroy_graph_cache();
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

// Unlike almost every other call in this file, this function soft-fails
// (returns false) instead of going through CUDA_CHECK and throwing. That is
// deliberate, not an oversight: this is a capability probe callers use to
// decide WHETHER to attempt D3D12/CUDA interop, not a hot-path tick
// operation where a hard failure should abort — throwing here would make it
// unusable as a "can I do this?" check. Both CUDA-call failure branches
// below call cudaGetLastError() to consume the runtime's per-thread sticky
// last-error state before returning; without that, a later, unrelated
// CUDA_CHECK(cudaGetLastError()) elsewhere in the tick path could pick up
// and misattribute an error this function already handled.
bool GpuEngine::device_luid(char out_luid[8]) const {
    int device = 0;
    if (cudaGetDevice(&device) != cudaSuccess) {
        cudaGetLastError();  // clear sticky error; this is a soft failure
        return false;
    }
    cudaDeviceProp prop{};
    if (cudaGetDeviceProperties(&prop, device) != cudaSuccess) {
        cudaGetLastError();  // clear sticky error; this is a soft failure
        return false;
    }
    // Checked as a heuristic signal that the LUID is unpopulated (non-WDDM).
    // CUDA's own docs only say this field's value is undefined on
    // TCC/non-Windows platforms, not that it's guaranteed zero there — this
    // project's native app (engine/native) is WIN32-only with WDDM-mode consumer
    // GPUs, where zero is the observed no-LUID signal in practice.
    if (!prop.luidDeviceNodeMask) return false;
    std::memcpy(out_luid, prop.luid, sizeof(prop.luid));
    return true;
}

// Unlike device_luid() above, this is NOT a repeatable, side-effect-free
// "can I do X?" query -- it is a one-time setup step that later interop work
// (the gather kernel a subsequent task adds) depends on actually succeeding.
// A false return here means "interop is unavailable this session", not "try
// again": on the first-call failure path nothing was imported, and on the
// second-call failure path the partially-created external memory object is
// explicitly torn down (cudaDestroyExternalMemory) before returning, so this
// object never holds a half-imported handle either way.
//
// It still soft-fails (bool, no throw) rather than going through CUDA_CHECK,
// matching this codebase's established pattern for one-time D3D12/CUDA setup
// calls the native app's probes rather than hard-crashes on (see
// D3D12Presenter::create_shared_particle_buffer(), which returns nullptr on
// failure for the same reason: a missing/mismatched adapter or an
// interop-hostile driver is a real, recoverable-by-the-caller outcome, not a
// programming error).
//
// The sticky-error-clearing concern from device_luid() DOES still apply here,
// for a reason independent of the "is this a probe?" question: this function
// runs CUDA API calls on the same host thread that will go on to do
// unrelated tick work afterward (kernel launches immediately followed by
// CUDA_CHECK(cudaGetLastError()), the codebase's standard post-launch-error
// pattern -- see kernels_forces.cu, kernels_poisson.cu, etc.). If a failed
// import here left the runtime's per-thread sticky error set, the very next
// unrelated CUDA_CHECK(cudaGetLastError()) in the tick path would pick it up
// and misattribute this function's stale failure to a completely unrelated
// kernel launch. Both failure branches below clear it for that reason.
//
// Safe to call more than once (e.g. to re-import after a D3D12-side buffer
// resize) -- mirrors D3D12Presenter::create_shared_particle_buffer()'s own
// "safe to call more than once" contract on the exporting side. Any
// previously-imported external memory object is torn down (which also
// invalidates the previously-mapped bufs_.d_interop_particle_buffer view)
// before the new cudaImportExternalMemory call, so a second call never
// leaks the first import's driver-level reference to its D3D12 resource.
// Like device_luid(), this must be called from the same OS thread that
// owns this GpuEngine's CUDA context -- cudaImportExternalMemory and
// cudaExternalMemoryGetMappedBuffer both operate against the calling
// thread's current CUDA context.
bool GpuEngine::import_d3d12_particle_buffer(void* nt_handle, std::uint64_t byte_count) {
    if (!nt_handle || byte_count == 0) return false;

    // A real (re-)import attempt below invalidates whatever a PRIOR import's
    // gather state meant, regardless of whether THIS attempt goes on to
    // succeed or fail: bufs_.interop_particle_capacity described the OLD
    // buffer's size (about to be torn down), and interop_gather_launched_
    // tracked whether a gather had ever completed against that OLD buffer.
    // Resetting both here closes the stale-state window a caller could
    // otherwise observe by polling interop_gather_ready()/
    // interop_particle_count() after a fresh import (e.g. the lattice-resize
    // re-import path already exercised by test_cuda_import_shared_buffer.cpp)
    // but before the next interop_gather_particles() call -- without this,
    // interop_gather_ready() would still report the OLD import's completed
    // event as "ready" and interop_particle_count() would still return the
    // OLD import's last captured_count for a buffer nothing has gathered
    // into yet.
    interop_gather_launched_.store(false, std::memory_order_relaxed);
    bufs_.interop_particle_capacity = 0;

    // Tear down a prior import before creating the new one -- otherwise the
    // assignments below simply overwrite bufs_.interop_external_memory /
    // bufs_.d_interop_particle_buffer and the first import's external memory
    // object (and its underlying driver-level reference to the D3D12
    // resource) leaks for the process lifetime.
    if (bufs_.interop_external_memory) {
        cudaDestroyExternalMemory(bufs_.interop_external_memory);
        bufs_.interop_external_memory = nullptr;
        bufs_.d_interop_particle_buffer = nullptr;
    }

    // This call has two distinct failure sources, both handled identically
    // (soft bool failure, no throw) since the caller only needs to know
    // "did the import succeed", not which CUDA call rejected it:
    //   1. cudaImportExternalMemory() below -- the NT handle/description was
    //      rejected (e.g. adapter mismatch, driver does not support D3D12
    //      resource import).
    //   2. cudaExternalMemoryGetMappedBuffer() further below -- the memory
    //      object imported but could not be mapped as a flat buffer.
    cudaExternalMemoryHandleDesc mem_desc{};
    mem_desc.type = cudaExternalMemoryHandleTypeD3D12Resource;
    mem_desc.handle.win32.handle = nt_handle;
    mem_desc.size = byte_count;
    mem_desc.flags = cudaExternalMemoryDedicated;
    if (cudaImportExternalMemory(&bufs_.interop_external_memory, &mem_desc) != cudaSuccess) {
        cudaGetLastError();  // clear sticky error; see function-level comment
        return false;
    }

    cudaExternalMemoryBufferDesc buf_desc{};
    buf_desc.offset = 0;
    buf_desc.size = byte_count;
    buf_desc.flags = 0;
    if (cudaExternalMemoryGetMappedBuffer(&bufs_.d_interop_particle_buffer,
                                          bufs_.interop_external_memory,
                                          &buf_desc) != cudaSuccess) {
        cudaGetLastError();  // clear sticky error; see function-level comment
        cudaDestroyExternalMemory(bufs_.interop_external_memory);
        bufs_.interop_external_memory = nullptr;
        // cudaExternalMemoryGetMappedBuffer() is this call's own out-param
        // target; the CUDA Runtime API does not document that it leaves the
        // pointer untouched on failure, so null it explicitly rather than
        // relying on driver behavior. Otherwise a caller that bypasses
        // NativeEngineSession's interop_enabled_ gate (e.g. a direct
        // GpuEngine test caller) could observe a non-null, potentially
        // driver-written-garbage bufs_.d_interop_particle_buffer here.
        bufs_.d_interop_particle_buffer = nullptr;
        return false;
    }
    // Record how many InteropParticleRecord slots the mapped view actually
    // has room for -- this is the D3D12-side buffer's exact size
    // (D3D12Presenter::create_shared_particle_buffer() allocates precisely
    // max_particles * sizeof(InteropParticleRecord) bytes, no padding), so
    // integer division recovers max_particles exactly for any byte_count a
    // real caller passes. launch_interop_particle_gather() clamps its write
    // cap to this value so the gather kernel can never write past the end of
    // the imported memory. A byte_count that is not an exact multiple of
    // sizeof(InteropParticleRecord) floors to the largest fully-contained
    // record count, which is the correct conservative bound either way.
    bufs_.interop_particle_capacity =
        static_cast<std::uint32_t>(byte_count / sizeof(InteropParticleRecord));
    return true;
}

// Cross-API GPU-timeline fence (Task 7). Imports a D3D12_FENCE_FLAG_SHARED
// fence (D3D12Presenter::create_shared_fence()) as a CUDA external
// semaphore so interop_signal_fence() below can signal it on bufs_.stream
// after the gather kernel -- the D3D12-side wait_shared_fence() then blocks
// the render queue (not the CPU) until that signal retires. Same
// handle-lifetime contract as import_d3d12_particle_buffer(): the caller
// closes nt_handle after this call returns, success or failure. Safe to
// call more than once (e.g. to re-import after a presenter reset): any
// previously-imported semaphore is torn down first so a second call never
// leaks the prior import's driver-level reference. Like
// import_d3d12_particle_buffer(), must be called from the same OS thread
// that owns this GpuEngine's CUDA context -- cudaImportExternalSemaphore
// operates against the calling thread's current CUDA context.
bool GpuEngine::import_d3d12_fence(void* nt_handle) {
    if (!nt_handle) return false;
    if (bufs_.interop_fence) {
        cudaDestroyExternalSemaphore(bufs_.interop_fence);
        bufs_.interop_fence = nullptr;
    }
    cudaExternalSemaphoreHandleDesc desc{};
    desc.type = cudaExternalSemaphoreHandleTypeD3D12Fence;
    desc.handle.win32.handle = nt_handle;
    desc.flags = 0;
    if (cudaImportExternalSemaphore(&bufs_.interop_fence, &desc) != cudaSuccess) {
        cudaGetLastError();  // clear sticky error; mirrors import_d3d12_particle_buffer()
        return false;
    }
    return true;
}

// Signals bufs_.interop_fence to `value` on bufs_.stream. A no-op (false)
// when no fence has been imported -- callers that never called
// import_d3d12_fence() (e.g. tests exercising only the gather path) can
// call interop_gather_particles() freely without this branch doing
// anything.
//
// Precondition: like import_d3d12_fence(), must be called from the same OS
// thread that owns this GpuEngine's CUDA context --
// cudaSignalExternalSemaphoresAsync operates against the calling thread's
// current CUDA context, not a property of this GpuEngine instance.
//
// On a cudaSignalExternalSemaphoresAsync() failure (e.g. a caller-supplied
// `value` that does not monotonically increase relative to the fence's
// current value -- the public API does nothing to prevent that), this
// clears the sticky per-thread CUDA error before returning, for the exact
// reason import_d3d12_particle_buffer() and import_d3d12_fence() clear it on
// their own failure paths (see the function-level comment above
// import_d3d12_particle_buffer()): this call runs on the same host thread
// that goes on to do unrelated tick work afterward via
// interop_gather_particles() (adjacent to real kernel launches immediately
// followed by CUDA_CHECK(cudaGetLastError())), so an uncleared sticky error
// here would be misattributed to a completely unrelated kernel launch by the
// very next such check.
bool GpuEngine::interop_signal_fence(std::uint64_t value) {
    if (!bufs_.interop_fence) return false;
    cudaExternalSemaphoreSignalParams params{};
    params.params.fence.value = value;
    if (cudaSignalExternalSemaphoresAsync(&bufs_.interop_fence, &params, 1,
                                          bufs_.stream) != cudaSuccess) {
        cudaGetLastError();  // clear sticky error; see function-level comment
        return false;
    }
    return true;
}

void GpuEngine::set_dt(double dt) {
    // Mirror RenderBridge::set_dt: dt<1 is honored with symplectic_leapfrog
    // or verlet_wave_integrator. The plain leapfrog hardcodes dt=1.
    // FTD-0408/0411 exact monodromies lock the unit tick.
    if (toggles.lorentz_period2_floquet || toggles.lorentz_bcc_time_floquet) {
        dt_ = 1.0;
    } else {
        dt_ = (toggles.symplectic_leapfrog
               || toggles.verlet_wave_integrator
               || dt >= 1.0) ? dt : 1.0;
    }
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

void GpuEngine::record_tick_body() {
    bufs_.reset_continuity_ledger();

    // CPU parity: the electroweak scenario's uniform +x drive is part of the
    // tick input, not a visualization mutation. Apply it before phase_read so
    // the same-tick wave/KG operators consume the driven field. In dual mode
    // the symmetric half split is essential because phase_read reads L/R.
    // The drive is computed ON-DEVICE from bufs_.d_tick (not passed by value)
    // so a captured graph replay recomputes it from the current tick instead
    // of baking in the capture-time value — same hazard class Task 7 fixed
    // for the RNG-salted kernels.
    if (toggles.ew_background_sweep) {
        kernels::launch_ew_background_sweep(bufs_, toggles.dual_substrate);
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
    if (!toggles.matched_gauss_dynamics) {
        gpu_phase_read();
        gpu_phase_write();
    }

    // E1 / FTD-0337: KDK second half-kick at the post-drift field, before
    // pair production and Gauss — same operator split as RenderBridge::tick.
    if (toggles.verlet_wave_integrator) {
        gpu_phase_read();
        kernels::launch_verlet_second_half_kick(
            bufs_, dt_, toggles.dual_substrate);
    }

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
    if (toggles.strong_stress_energy) {
        gpu_build_particle_list();
        const bool projection_ok =
            toggles.color_forces && toggles.forces && toggles.movement
            && !toggles.damping && !toggles.genesis && !toggles.evaporation
            && !toggles.pair_production && !toggles.poisson_coulomb
            && !toggles.emergent_forces && !toggles.gravity
            && !toggles.latency_field && !toggles.lorentz_force
            && !toggles.strong_force && !toggles.exchange_force
            && !toggles.weak_transmutation && !toggles.triad_binding
            && !toggles.absorbing_boundary && !toggles.reflective_boundary;
        kernels::launch_begin_strong_energy(
            bufs_, toggles.movement, projection_ok);
    }

    if (toggles.latency_field) {
        if (toggles.strong_stress_energy) {
            kernels::launch_strong_t00(bufs_);
        }
        gpu_solve_latency_poisson();
    }

    const bool any_force = toggles.forces || toggles.color_forces
                        || toggles.strong_force || toggles.exchange_force
                        || toggles.cluster_inertia;
    if (any_force) bufs_.reset_force_diag();

    // Phase 4: Force accumulation (Coulomb Poisson + EM/gravity/Lorentz)
    if (toggles.forces) {
        gpu_phase_forces();
    }

    // Phase 4b: Pairwise forces (color, Yukawa, exchange) — requires particle list
    bool need_plist = toggles.color_forces || toggles.strong_force
                   || toggles.exchange_force;
    if (need_plist) {
        gpu_build_particle_list();
        gpu_particle_forces();
    }

    // FTD-0402: every active force channel feeds one momentum update.
    if (any_force) kernels::launch_integrate_forces(bufs_, dt_);

    // Rigid-body cluster inertia: after integrate, before movement — CPU Rule 4.
    if (toggles.cluster_inertia) {
        kernels::launch_cluster_inertia(bufs_, dt_);
    }

    // Phase 5: Movement
    if (toggles.movement) {
        gpu_phase_movement();
    }

    if (toggles.matched_gauss_dynamics) {
        kernels::launch_matched_gauss_advance(bufs_, C_SPEED, dt_);
    }

    if (toggles.strong_stress_energy && toggles.movement) {
        gpu_build_particle_list();
        kernels::launch_complete_strong_energy(bufs_);
    }
    if (toggles.strong_stress_energy) {
        if (!toggles.movement) gpu_build_particle_list();
        kernels::launch_strong_t00(bufs_);
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

    // Phase 7: Triad binding — after movement + weak, matching CPU Rule 7.
    // Rebuild the particle list on post-movement sites.
    if (toggles.triad_binding) {
        gpu_build_particle_list();
        gpu_triad_detection();
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
}

// ---------- Graph eligibility / key ----------

bool GpuEngine::graph_eligible() const {
    // ew_background_sweep's drive is now device-resident (see commit
    // 7dc754bc, made during Task 7's code review) and could technically be
    // graph-eligible, but it remains excluded here because it's a research
    // scenario outside this task's tested profiles (none of
    // test_gpu_graph_capture.cpp's Profile values enable it) — revisit if a
    // future profile needs it.
    if (toggles.ew_background_sweep) return false;
    // gpu_gauge_relax()'s ping-pong buffer swap (std::swap(d_su2_[d],
    // d_su2_scr_[d]) / the su3 equivalent) is plain host C++, not a CUDA API
    // call, so stream capture never records it. A captured graph would keep
    // replaying the one kernel launch topology recorded at capture time —
    // src=d_su2_[d]/dst=d_su2_scr_[d] with those exact pointers baked in —
    // forever, instead of alternating which buffer is "current" the way the
    // CPU reference does every tick. Confirmed via test_gauge_gpu_parity's P1
    // regressing from PASS to a 2.141e-02 CPU/GPU divergence once
    // graph_capture_enabled defaulted to true. Excluded here rather than
    // reworked into a capture-safe device-resident ping-pong, matching how
    // ew_background_sweep is handled above.
    if (toggles.su2_gauge || toggles.su3_gauge) return false;
    return true;
}

std::uint64_t GpuEngine::graph_key() const {
    std::uint64_t h = 1469598103934665603ULL;
    const auto mix = [&h](const void* p, std::size_t n) {
        const auto* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < n; ++i) {
            h ^= b[i];
            h *= 1099511628211ULL;
        }
    };
    const auto mix_bool = [&mix](bool v) { const unsigned char c = v ? 1u : 0u; mix(&c, 1); };
    const auto mix_int  = [&mix](int v)  { mix(&v, sizeof(v)); };
    const auto mix_dbl  = [&mix](double v) { mix(&v, sizeof(v)); };

    // Topology: every toggle that adds, removes or reshapes a launch.
    mix_bool(toggles.wave_propagation);
    mix_bool(toggles.coupling);
    mix_bool(toggles.damping);
    mix_bool(toggles.selective_damping);
    mix_bool(toggles.larmor_radiation);
    mix_bool(toggles.genesis);
    mix_bool(toggles.evaporation);
    mix_bool(toggles.langevin);
    mix_bool(toggles.symplectic_leapfrog);
    mix_bool(toggles.verlet_wave_integrator);
    mix_bool(toggles.lorentz_period2_floquet);
    mix_bool(toggles.lorentz_bcc_time_floquet);
    mix_bool(toggles.pair_production);
    mix_bool(toggles.gauss_projection);
    mix_bool(toggles.dual_substrate);
    mix_bool(toggles.latency_field);
    mix_bool(toggles.field_energy_gravity);
    mix_bool(toggles.forces);
    mix_bool(toggles.gravity);
    mix_bool(toggles.lorentz_force);
    mix_bool(toggles.poisson_coulomb);
    mix_bool(toggles.emergent_forces);
    mix_bool(toggles.color_forces);
    mix_bool(toggles.strong_force);
    mix_bool(toggles.exchange_force);
    mix_bool(toggles.triad_binding);
    mix_bool(toggles.cluster_inertia);
    mix_bool(toggles.movement);
    mix_bool(toggles.symmetric_movement_order);
    mix_bool(toggles.confinement);
    mix_bool(toggles.strong_stress_energy);
    mix_bool(toggles.matched_gauss_dynamics);
    mix_bool(toggles.reflective_boundary);
    mix_bool(toggles.absorbing_boundary);
    mix_int(static_cast<int>(toggles.flux_boundary));
    mix_bool(toggles.weak_transmutation);
    mix_bool(toggles.su2_gauge);
    mix_bool(toggles.su3_gauge);
    mix_bool(toggles.de_broglie_clock);
    mix_bool(toggles.db_clock_coulomb);
    mix_bool(toggles.exact_dual_gauss);
    mix_bool(toggles.ew_background_sweep);
    mix_int(static_cast<int>(toggles.bcc_stencil));
    mix_int(static_cast<int>(toggles.langevin_site_filter));

    // Non-toggle latches that gate whole phases.
    mix_bool(weak_field_active_);
    mix_bool(gauge_links_device_);

    // Scalar kernel arguments. These are NOT topology, but CUDA bakes them
    // into node parameters, so a change must force a recapture just as a
    // toggle change does.
    mix_dbl(dt_);
    mix_dbl(toggles.omega0);
    mix_dbl(toggles.langevin_T);
    mix_dbl(toggles.langevin_gamma);
    mix_dbl(toggles.kinetic_drain);
    mix_dbl(toggles.coulomb_charge_coupling);
    mix_dbl(toggles.coulomb_source_scale);
    mix_dbl(genesis_threshold_override);
    mix_dbl(manifest_scale_override);
    mix_bool(manifest_use_temperature);
    mix_int(static_cast<int>(toggles.langevin_seed));
    return h;
}

// Safety of destroying a possibly still-executing exec: ticks are async by
// design (no cudaStreamSynchronize between them), so on eviction the exec
// from the immediately preceding tick may still be running on the device
// when this runs. No sync precedes this call, and none is needed.
//
// CUDA graph execs use the same deferred-destruction model as streams and
// events: the destroy call returns immediately and the driver defers actual
// resource release until in-flight work referencing the object completes.
// This is documented, not assumed:
//   - cuGraphExecDestroy (driver API, include/cuda.h, CUDA 13.0 local
//     headers): "Destroys the executable graph specified by hGraphExec...
//     If the executable graph is in-flight, it will not be terminated, but
//     rather freed asynchronously on completion." — an explicit statement of
//     exactly this case, for the function cudaGraphExecDestroy wraps.
//   - cudaGraphExecDestroy (runtime API, include/cuda_runtime_api.h) carries
//     the same \note_destroy_ub tag as cudaStreamDestroy and cudaEventDestroy,
//     both of which spell out the identical asynchronous-release contract in
//     the same header ("the function will return immediately and the
//     resources ... will be released automatically once the device has
//     completed all work"; "the call does not block on completion ... any
//     associated resources will automatically be released asynchronously").
// Empirically stress-tested (Task 8 code review, 2026-08-18): forcing
// destroy_graph_cache() to fire on a still-in-flight exec by cycling 24
// distinct graph_key() topologies back-to-back with no synchronization
// (test_gpu_graph_capture.cpp G7) ran clean under both
// compute-sanitizer --tool memcheck and --tool synccheck, zero errors.
void GpuEngine::destroy_graph_cache() {
    for (auto& entry : graph_cache_) {
        if (entry.second) cudaGraphExecDestroy(entry.second);
    }
    graph_cache_.clear();
}

// ---------- Tick dispatch ----------

void GpuEngine::tick() {
    continuity_ledger_valid_ = true;
    if (toggles.matched_gauss_dynamics && !matched_gauss_ready_) {
        throw std::logic_error(
            "[FTD-0428] matched_gauss_dynamics requires explicit initialization");
    }

    if (!graph_capture_enabled || !graph_eligible()) {
        record_tick_body();
    } else {
        const std::uint64_t key = graph_key();
        const auto it = graph_cache_.find(key);
        if (it != graph_cache_.end()) {
            if (it->second) {
                CUDA_CHECK(cudaGraphLaunch(it->second, bufs_.stream));
                ++graph_replays_;
            } else {
                // Known-uncapturable key: permanent direct-launch fallback.
                record_tick_body();
            }
        } else {
            if (graph_cache_.size() >= MAX_GRAPH_CACHE) destroy_graph_cache();

            // Capture RECORDS without executing, so this pass performs zero
            // device work; launching the instantiated graph immediately below
            // is what makes the capturing tick a normal tick. No scratch
            // buffers and no state rollback are involved. Thread-local capture
            // mode is deliberate but currently defensive rather than load-
            // bearing: today's only caller (engine/native/src/app/main.cpp)
            // runs tick() and capture()/snapshot polling strictly sequentially
            // on one thread, so nothing actually issues legacy-stream work
            // concurrently with a capture in progress. ThreadLocal is chosen
            // anyway because it is strictly safer than Global at zero cost —
            // it scopes the capture restriction to this thread instead of
            // process-wide, so a future multi-threaded caller (e.g. a snapshot
            // poller on its own thread) cannot be silently broken by legacy-
            // stream work issued elsewhere while this thread is capturing.
            cudaGraph_t graph = nullptr;
            bool captured = false;
            const cudaError_t begin_status = cudaStreamBeginCapture(
                bufs_.stream, cudaStreamCaptureModeThreadLocal);
            if (begin_status == cudaSuccess) {
                bool recorded = true;
                try {
                    record_tick_body();
                } catch (...) {
                    recorded = false;
                }
                const cudaError_t end_status =
                    cudaStreamEndCapture(bufs_.stream, &graph);
                captured = recorded && end_status == cudaSuccess && graph;
                if (!captured && graph) {
                    cudaGraphDestroy(graph);
                    graph = nullptr;
                }
            }

            cudaGraphExec_t exec = nullptr;
            if (captured) {
                const cudaError_t inst_status =
                    cudaGraphInstantiate(&exec, graph, 0);
                cudaGraphDestroy(graph);
                if (inst_status != cudaSuccess) exec = nullptr;
            }

            graph_cache_[key] = exec;
            if (exec) {
                // The recorded work never ran; run it now as the graph.
                CUDA_CHECK(cudaGraphLaunch(exec, bufs_.stream));
                ++graph_captures_;
                ++graph_replays_;
            } else {
                // Capture failed: the recorded work was discarded, so this
                // tick has done nothing yet. Re-run it with direct launches.
                ++graph_capture_failures_;
                cudaGetLastError();   // clear any sticky capture error
                record_tick_body();
            }
        }
    }

    tick_++;
    host_dirty_ = true;
    mark_device_state_changed();
    if (toggles.matched_gauss_dynamics) {
        int valid = 0;
        CUDA_CHECK(cudaMemcpy(&valid, bufs_.d_matched_valid, sizeof(int),
                              cudaMemcpyDeviceToHost));
        matched_gauss_last_valid_ = valid != 0;
        if (!matched_gauss_last_valid_) {
            throw std::logic_error(
                "[FTD-0428] movement history is not conservative and routable");
        }
    }
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
                                        toggles.omega0,
                                        toggles.lorentz_period2_floquet,
                                        toggles.lorentz_bcc_time_floquet);
    } else {
        kernels::launch_phase_read(bufs_,
                                   toggles.wave_propagation,
                                   toggles.coupling,
                                   static_cast<uint8_t>(toggles.bcc_stencil),
                                   toggles.de_broglie_clock,
                                   toggles.db_clock_coulomb,
                                   toggles.omega0,
                                   toggles.lorentz_period2_floquet,
                                   toggles.lorentz_bcc_time_floquet);
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
                                         toggles.verlet_wave_integrator,
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
                                    toggles.verlet_wave_integrator,
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
                                  toggles.strong_stress_energy ? bufs_.d_strong_t00 : nullptr,
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
                                 toggles.geometric_gravity,
                                 toggles.lorentz_force,
                                 dt_);
}

void GpuEngine::gpu_phase_movement() {
    kernels::launch_phase_movement(bufs_, dt_, toggles.reflective_boundary,
                                   toggles.dual_substrate,
                                   toggles.symmetric_movement_order,
                                   static_cast<unsigned long long>(toggles.langevin_seed));
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
        kernels::launch_color_force(bufs_, dt_, toggles.confinement,
                                    toggles.strong_stress_energy);
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
    launch_visual_particle_capture(bufs_, request.max_particles, size_,
                                   request.interior_cull_layers,
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

// Native-desktop D3D12 interop (Component B). Runs the same
// flags/prefix-scan/header selection pass as begin_visual_snapshot() above,
// but gathers fully-decoded InteropParticleRecords straight into the buffer
// import_d3d12_particle_buffer() imported, instead of raw
// VisualParticleRecords for later CPU decode -- see
// launch_interop_particle_gather()'s own comment for why the two paths stay
// duplicated rather than sharing a helper. Everything here runs on
// bufs_.stream (the Component-A dedicated stream), unlike
// begin_visual_snapshot()'s default-stream launches, so it can be ordered
// with (and graph-captured alongside) the rest of the tick if a future task
// folds it into Component A's capture.
bool GpuEngine::interop_gather_particles(std::uint32_t max_particles,
                                         std::uint64_t fence_value,
                                         std::uint16_t cull_layers) {
    if (!bufs_.d_interop_particle_buffer) return false;
    launch_interop_particle_gather(bufs_, size_, max_particles, cull_layers);
    // Marks that at least one gather has actually been launched against the
    // CURRENTLY-imported buffer, so interop_gather_ready() below can tell
    // "the event has never been recorded" apart from "the event retired".
    // cudaEventQuery on an event that has never had cudaEventRecord() called
    // on it returns cudaSuccess (there is no outstanding work to wait for),
    // so without this flag interop_gather_ready() would report READY before
    // the first gather ever ran, and interop_particle_count() would then
    // read bufs_.h_interop_header->captured_count while it is still
    // uninitialized pinned memory (cudaHostAlloc does not zero it).
    // import_d3d12_particle_buffer() clears this flag on every (re-)import,
    // so it also cannot outlive the buffer it was measured against.
    interop_gather_launched_.store(true, std::memory_order_relaxed);
    // Cross-API GPU-timeline fence (Task 7): signal AFTER the gather kernel
    // launch above so the signal is ordered on bufs_.stream behind the
    // gather's writes into d_interop_particle_buffer. A no-op when no fence
    // has been imported (interop_signal_fence() checks bufs_.interop_fence
    // itself). A real signal failure here means the gather itself may have
    // succeeded, but the cross-API handoff that makes the buffer safely
    // consumable by D3D12 did not -- without a signaled fence the render
    // thread's wait_shared_fence() would block on a value that is never
    // reached, so this is reported as a failure of the whole call.
    if (bufs_.interop_fence) {
        if (!interop_signal_fence(fence_value)) {
            std::cerr << "[GpuEngine] interop_gather_particles: "
                         "interop_signal_fence(" << fence_value
                      << ") failed; D3D12 side will not see this gather"
                      << std::endl;
            return false;
        }
    }
    return true;
}

bool GpuEngine::interop_gather_ready() const {
    // Mirrors visual_snapshot_ready()'s own pending-flag-before-event-query
    // pattern (see poll_visual_snapshot() above): a flag gate is required
    // because cudaEventQuery on a not-yet-recorded event reports "ready" by
    // definition (no work to wait for), which would otherwise make this
    // function return true before interop_gather_particles() has ever run.
    if (!interop_gather_launched_.load(std::memory_order_relaxed) ||
        !bufs_.interop_gather_ready) return false;
    const cudaError_t status = cudaEventQuery(bufs_.interop_gather_ready);
    if (status == cudaSuccess) return true;
    if (status == cudaErrorNotReady) return false;
    throw std::runtime_error(std::string("[GpuEngine] interop gather event query failed: ")
                             + cudaGetErrorString(status));
}

std::uint32_t GpuEngine::interop_particle_count() const {
    // Only safe to read bufs_.h_interop_header->captured_count once
    // interop_gather_ready() is true (its doc comment states this contract
    // explicitly): before the first gather, the pinned host header is
    // genuinely uninitialized memory; between a gather launch and its event
    // retiring, the in-flight cudaMemcpyAsync D2H copy is still writing it.
    // Gating here -- rather than trusting every caller to check
    // interop_gather_ready() first -- matches this file's own established
    // pattern for the analogous visual-snapshot path, where
    // poll_visual_snapshot() (not the caller) enforces the same precondition
    // via visual_snapshot_ready().
    return interop_gather_ready() ? bufs_.h_interop_header->captured_count : 0u;
}

// TEST-ONLY (see gpu_engine.h doc comment). A synchronous, blocking
// cudaMemcpy straight off the imported interop buffer -- never called from
// any production path, which exists specifically to avoid this download
// (Task 6). Used only by test_interop_visual_parity to compare the interop
// gather kernel's output against the pre-interop CPU capture path.
void GpuEngine::debug_read_interop_records(std::vector<InteropParticleRecord>& out,
                                           std::uint32_t count) const {
    // Clamp against the imported buffer's real capacity, the same way
    // launch_interop_particle_gather() clamps its writes -- otherwise a
    // caller-supplied count larger than bufs_.interop_particle_capacity
    // would read past the end of the imported device buffer below.
    // interop_particle_capacity is 0 whenever d_interop_particle_buffer is
    // null (see import_d3d12_particle_buffer()), so this clamp is safe to
    // apply unconditionally, before the buffer-validity guard.
    count = (std::min)(count, bufs_.interop_particle_capacity);
    out.assign(count, InteropParticleRecord{});
    if (!bufs_.d_interop_particle_buffer || count == 0) return;
    CUDA_CHECK(cudaMemcpy(out.data(), bufs_.d_interop_particle_buffer,
                          static_cast<std::size_t>(count) * sizeof(InteropParticleRecord),
                          cudaMemcpyDeviceToHost));
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

void GpuEngine::upload_matched_gauss(const eft::MatchedGaussDynamics& src) {
    if (!src.initialized() || src.size() != size_) {
        throw std::logic_error(
            "[FTD-0428] matched Gauss upload requires an initialized field of matching size");
    }
    bufs_.ensure_matched_gauss();
    const std::size_t bytes = static_cast<std::size_t>(N_) * sizeof(double);
    CUDA_CHECK(cudaMemcpy(bufs_.d_matched_ex, src.electric().x.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(bufs_.d_matched_ey, src.electric().y.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(bufs_.d_matched_ez, src.electric().z.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(bufs_.d_matched_bx, src.magnetic_half().x.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(bufs_.d_matched_by, src.magnetic_half().y.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(bufs_.d_matched_bz, src.magnetic_half().z.data(), bytes, cudaMemcpyHostToDevice));
    matched_gauss_ready_ = true;
    matched_gauss_last_valid_ = true;
    host_dirty_ = true;
    mark_device_state_changed();
}

void GpuEngine::download_matched_gauss(eft::MatchedGaussDynamics& dst) {
    bufs_.ensure_matched_gauss();
    eft::MatchedFaceFlux electric(size_);
    eft::MatchedEdgeField magnetic(size_);
    const std::size_t bytes = static_cast<std::size_t>(N_) * sizeof(double);
    CUDA_CHECK(cudaMemcpy(electric.x.data(), bufs_.d_matched_ex, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(electric.y.data(), bufs_.d_matched_ey, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(electric.z.data(), bufs_.d_matched_ez, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(magnetic.x.data(), bufs_.d_matched_bx, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(magnetic.y.data(), bufs_.d_matched_by, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(magnetic.z.data(), bufs_.d_matched_bz, bytes, cudaMemcpyDeviceToHost));
    eft::MatchedWaveStep step;
    step.valid = matched_gauss_last_valid_;
    dst.adopt_state(std::move(electric), std::move(magnetic), step);
}

void GpuEngine::download_strong_stress(std::vector<StrongStressCell>& out) {
    bufs_.ensure_strong_stress();
    out.assign(static_cast<std::size_t>(N_), {});
    std::vector<double> t00(static_cast<std::size_t>(N_));
    std::vector<double> xx(static_cast<std::size_t>(N_));
    std::vector<double> yy(static_cast<std::size_t>(N_));
    std::vector<double> zz(static_cast<std::size_t>(N_));
    std::vector<double> xy(static_cast<std::size_t>(N_));
    std::vector<double> xz(static_cast<std::size_t>(N_));
    std::vector<double> yz(static_cast<std::size_t>(N_));
    const std::size_t bytes = static_cast<std::size_t>(N_) * sizeof(double);
    CUDA_CHECK(cudaMemcpy(t00.data(), bufs_.d_strong_t00, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(xx.data(), bufs_.d_strong_sxx, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(yy.data(), bufs_.d_strong_syy, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(zz.data(), bufs_.d_strong_szz, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(xy.data(), bufs_.d_strong_sxy, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(xz.data(), bufs_.d_strong_sxz, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(yz.data(), bufs_.d_strong_syz, bytes, cudaMemcpyDeviceToHost));
    for (int i = 0; i < N_; ++i) {
        auto& cell = out[static_cast<std::size_t>(i)];
        cell.energy_density = t00[static_cast<std::size_t>(i)];
        cell.stress_xx = xx[static_cast<std::size_t>(i)];
        cell.stress_yy = yy[static_cast<std::size_t>(i)];
        cell.stress_zz = zz[static_cast<std::size_t>(i)];
        cell.stress_xy = xy[static_cast<std::size_t>(i)];
        cell.stress_xz = xz[static_cast<std::size_t>(i)];
        cell.stress_yz = yz[static_cast<std::size_t>(i)];
    }
}

void GpuEngine::download_strong_step_diagnostics(StrongEnergyStepDiagnostics& out) {
    bufs_.ensure_strong_stress();
    GpuBuffers::StrongStepDevice step{};
    CUDA_CHECK(cudaMemcpy(&step, bufs_.d_strong_step, sizeof(step),
                          cudaMemcpyDeviceToHost));
    out.h_before = step.h_before;
    out.h_after = step.h_after;
    out.residual = step.residual;
    out.lambda = step.lambda;
    out.momentum_before = {step.mx_before, step.my_before, step.mz_before};
    out.momentum_after = {step.mx_after, step.my_after, step.mz_after};
    out.projection_events = step.projection_events;
    out.projection_failures = step.projection_failures;
    out.topology_failures = step.topology_failures;
    out.projected_particles = step.projected_particles;
}

}  // namespace gpu
}  // namespace ftd

/**
 * SoA device buffer management for FTD GPU engine.
 * Handles allocation, deallocation, and AoS↔SoA conversions.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include <cuda_runtime.h>
#include <cub/device/device_select.cuh>
#include <cub/device/device_scan.cuh>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/transform_iterator.h>
#include <algorithm>
#include <cufft.h>
#include <cstdio>
#include <cmath>
#include <cstring>   // std::memcpy — bit-exact double compare in the C5 delta path
#include <stdexcept>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Check CUDA errors with file/line info
#include "cuda_error.cuh"  // CUDA_CHECK (revision C1 consolidation)

namespace ftd {
namespace gpu {

namespace {

// The visual particle path scans the existing byte flag scratch into the
// existing int32 prefix scratch.  CUB needs matching input/output value types,
// so make the conversion explicit without materializing an N-sized int flag
// array.  Pair/genesis compaction now runs on the engine's dedicated stream
// (GpuBuffers::stream); the visual-capture consumer of this same scratch
// (launch_visual_particle_capture in gpu_engine.cu) still runs on the legacy
// default stream.  Correctness holds via the blocking-stream implicit sync
// documented on the GpuBuffers::stream member itself, not because the two
// consumers share a stream.  The scratch remains persistent for the
// lifetime of GpuBuffers.
struct ByteFlagToInt {
    __host__ __device__ int operator()(std::uint8_t flag) const {
        return static_cast<int>(flag);
    }
};

}  // namespace

// ---------- Allocation ----------

void GpuBuffers::allocate(int lattice_size) {
    L = lattice_size;
    N = L * L * L;

    try {

    // Engine execution stream. Default (blocking) flags — see gpu_buffers.h.
    CUDA_CHECK(cudaStreamCreate(&stream));

    // State
    CUDA_CHECK(cudaMalloc(&d_state, N * sizeof(int8_t)));

    // Flux (3 components)
    CUDA_CHECK(cudaMalloc(&d_flux_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_z, N * sizeof(double)));

    // Wave velocity (3 components)
    CUDA_CHECK(cudaMalloc(&d_wave_vel_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_z, N * sizeof(double)));

    // Particle velocity (3 components)
    CUDA_CHECK(cudaMalloc(&d_velocity_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_velocity_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_velocity_z, N * sizeof(double)));

    // Remainder (3 components)
    CUDA_CHECK(cudaMalloc(&d_remainder_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_remainder_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_remainder_z, N * sizeof(double)));

    // Scalar fields
    CUDA_CHECK(cudaMalloc(&d_locked, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_particle_id, N * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_next_particle_id, sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_next_pair_id, sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_identity_allocation_base, sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_identity_error, sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_spin, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_color, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_flavor, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_accel_mag, N * sizeof(double)));

    // Solver potentials
    CUDA_CHECK(cudaMalloc(&d_phi, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_phi_coulomb, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_phi_latency, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_latency, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_tau, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_phase, N * sizeof(double)));
    // Zero-initialize the latency fields (warm-start = 0)
    CUDA_CHECK(cudaMemset(d_phi_latency, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_latency, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_tau, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_phase, 0, N * sizeof(double)));

    // Read-phase temporaries
    CUDA_CHECK(cudaMalloc(&d_delta_j_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_z, N * sizeof(double)));

    // Dual-substrate fields (18 arrays)
    CUDA_CHECK(cudaMalloc(&d_flux_L_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_L_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_L_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_R_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_R_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_R_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_L_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_L_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_L_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_R_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_R_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_R_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_L_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_L_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_L_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_R_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_R_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta_j_R_z, N * sizeof(double)));

    // Strong field (Stella Octangula)
    CUDA_CHECK(cudaMalloc(&d_flux_strong_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_strong_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_strong_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_strong_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_strong_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_strong_z, N * sizeof(double)));

    // Weak field (Cuboctahedron)
    CUDA_CHECK(cudaMalloc(&d_flux_weak_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_weak_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_flux_weak_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_weak_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_weak_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_wave_vel_weak_z, N * sizeof(double)));

    // Selective damping mask + Larmor acceleration
    CUDA_CHECK(cudaMalloc(&d_near_particle, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_near_accel, N * sizeof(double)));

    // Force-diagnostic mirrors (see gpu_buffers.h header note)
    CUDA_CHECK(cudaMalloc(&d_fd_coulomb_x,  N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_coulomb_y,  N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_coulomb_z,  N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_strong_x,   N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_strong_y,   N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_strong_z,   N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_magnetic_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_magnetic_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_magnetic_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_gravity_x,  N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_gravity_y,  N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_gravity_z,  N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_exchange_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_exchange_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_fd_exchange_z, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_causal_projection_events, sizeof(unsigned long long)));

    // Constant-size interactive diagnostic reduction scratch.  Keeping this
    // in GpuBuffers gives it the same exception-safe lifecycle as the lattice
    // arrays and avoids cudaMalloc/cudaFree on every dashboard poll.
    CUDA_CHECK(cudaMalloc(&d_compact_diagnostics,
                          COMPACT_DIAGNOSTIC_SCALARS * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_compact_charge_sum, sizeof(long long)));
    // A telemetry publisher writes this buffer once per scheduler epoch then
    // begins a pinned async D2H copy. Keep it disjoint from
    // d_compact_diagnostics so a synchronous inspector cannot overwrite an
    // unread snapshot result.
    CUDA_CHECK(cudaMalloc(&d_telemetry_snapshot,
                          COMPACT_TELEMETRY_SCALARS * sizeof(double)));
    CUDA_CHECK(cudaHostAlloc(reinterpret_cast<void**>(&h_telemetry_snapshot),
                             COMPACT_TELEMETRY_SCALARS * sizeof(double),
                             cudaHostAllocPortable));
    CUDA_CHECK(cudaEventCreateWithFlags(&telemetry_snapshot_ready,
                                        cudaEventDisableTiming));

    // A visual capture always transfers the same bounded staging capacity.
    // This avoids a host-side count round trip between device scan/gather and
    // D2H submission, and (unlike legacy per-frame vectors) requires no
    // request-path CUDA allocation.
    CUDA_CHECK(cudaMalloc(&d_visual_particle_header,
                          sizeof(VisualParticleStagingHeader)));
    CUDA_CHECK(cudaMalloc(&d_visual_particle_records,
                          kMaxVisualParticleCapture * sizeof(VisualParticleRecord)));
    CUDA_CHECK(cudaHostAlloc(reinterpret_cast<void**>(&h_visual_particle_header),
                             sizeof(VisualParticleStagingHeader),
                             cudaHostAllocPortable));
    CUDA_CHECK(cudaHostAlloc(reinterpret_cast<void**>(&h_visual_particle_records),
                             kMaxVisualParticleCapture * sizeof(VisualParticleRecord),
                             cudaHostAllocPortable));
    CUDA_CHECK(cudaEventCreateWithFlags(&visual_snapshot_ready,
                                        cudaEventDisableTiming));

    // Native-desktop D3D12 interop (Component B; see gpu_buffers.h). Only the
    // owned pieces (header + event) are allocated here.
    // d_interop_particle_buffer / interop_external_memory are NOT allocated
    // here -- they start null and are populated later by
    // import_d3d12_particle_buffer(), since the D3D12 buffer doesn't exist
    // until the presenter creates it, which happens after this allocate().
    CUDA_CHECK(cudaMalloc(&d_interop_header, sizeof(InteropParticleHeader)));
    CUDA_CHECK(cudaHostAlloc(reinterpret_cast<void**>(&h_interop_header),
                             sizeof(InteropParticleHeader), cudaHostAllocDefault));
    CUDA_CHECK(cudaEventCreateWithFlags(&interop_gather_ready,
                                        cudaEventDisableTiming));

    // Poisson mean-charge scratch (persistent; see gpu_buffers.h)
    CUDA_CHECK(cudaMalloc(&d_poisson_charge_sum, sizeof(long long)));
    CUDA_CHECK(cudaMalloc(&d_poisson_mean_charge, sizeof(double)));

    // FFT workspace
    CUDA_CHECK(cudaMalloc(&d_fft_buf, N * sizeof(cufftDoubleComplex)));
    CUDA_CHECK(cudaMalloc(&d_fft_buf_f, N * sizeof(cufftComplex)));
    CUDA_CHECK(cudaMalloc(&d_green, N * sizeof(double)));



    // Particle list
    CUDA_CHECK(cudaMalloc(&d_plist_idx, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_num_particles, sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_particle_overflow, sizeof(int)));
    // Deterministic particle-list compaction scratch (see gpu_buffers.h).
    CUDA_CHECK(cudaMalloc(&d_particle_flags, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_particle_candidate_indices, N * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_particle_candidate_count, sizeof(int32_t)));

    // Device tick mirror
    CUDA_CHECK(cudaMalloc(&d_tick, sizeof(int)));

    // Pair production tracking
    CUDA_CHECK(cudaMalloc(&d_pair_id, N * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_pair_candidate_flags, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_pair_candidate_indices, N * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_pair_candidate_count, sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_movement_moved, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMalloc(&d_movement_order, N * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_movement_rank, N * sizeof(int32_t)));
    // Stable compaction is shared by genesis identity ranking and pair
    // candidates.  The visual particle capture also uses its persistent byte
    // flags/int32 prefix arrays after a tick; everything is serialized on the
    // default stream.  Size the one CUB workspace for the larger of select or
    // scan so no interactive capture performs cudaMalloc/cudaFree.
    thrust::counting_iterator<int32_t> pair_indices(0);
    CUDA_CHECK(cub::DeviceSelect::Flagged(
        nullptr, pair_select_temp_bytes, pair_indices,
        d_pair_candidate_flags, d_pair_candidate_indices,
        d_pair_candidate_count, N));
    std::size_t visual_scan_temp_bytes = 0;
    const auto visual_flags = thrust::make_transform_iterator(
        d_pair_candidate_flags, ByteFlagToInt{});
    CUDA_CHECK(cub::DeviceScan::ExclusiveSum(
        nullptr, visual_scan_temp_bytes, visual_flags,
        d_pair_candidate_indices, N));
    pair_select_temp_bytes = (std::max)(pair_select_temp_bytes,
                                        visual_scan_temp_bytes);
    // launch_build_particle_list (kernels_forces.cu) reuses this same
    // workspace for a third cub::DeviceSelect::Flagged call over the
    // particle-list flags/candidate buffers. That call has the identical
    // (thrust::counting_iterator<int32_t> indices, uint8_t* flags,
    // int32_t* output, int32_t* num_selected, N) template shape as the
    // pair-candidate select above — CUB's required temp storage is a
    // function of exactly those types and N, not of the specific pointer
    // values — so it is EXPECTED to demand an identical byte count. Rather
    // than assume that, size it explicitly here too and fold it into the
    // max, so pair_select_temp_bytes is provably sufficient for all three
    // call sites regardless of whether that expectation holds.
    std::size_t particle_select_temp_bytes = 0;
    thrust::counting_iterator<int32_t> particle_indices(0);
    CUDA_CHECK(cub::DeviceSelect::Flagged(
        nullptr, particle_select_temp_bytes, particle_indices,
        d_particle_flags, d_particle_candidate_indices,
        d_particle_candidate_count, N));
    pair_select_temp_bytes = (std::max)(pair_select_temp_bytes,
                                        particle_select_temp_bytes);
    CUDA_CHECK(cudaMalloc(&d_pair_select_temp, pair_select_temp_bytes));

    // Native EFT continuity event ledger
    CUDA_CHECK(cudaMalloc(&d_ledger_rho_before, N * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_ledger_reaction, N * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_ledger_current_x, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_ledger_current_y, N * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_ledger_current_z, N * sizeof(double)));

    // Zero-initialize all buffers
    CUDA_CHECK(cudaMemset(d_state, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_flux_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_velocity_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_velocity_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_velocity_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_remainder_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_remainder_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_remainder_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_locked, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_particle_id, 0xFF, N * sizeof(int32_t))); // -1
    CUDA_CHECK(cudaMemset(d_next_particle_id, 0, sizeof(int32_t)));
    CUDA_CHECK(cudaMemset(d_next_pair_id, 0, sizeof(int32_t)));
    CUDA_CHECK(cudaMemset(d_identity_allocation_base, 0, sizeof(int32_t)));
    CUDA_CHECK(cudaMemset(d_identity_error, 0, sizeof(int32_t)));
    CUDA_CHECK(cudaMemset(d_pair_candidate_flags, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_pair_candidate_count, 0, sizeof(int32_t)));
    CUDA_CHECK(cudaMemset(d_particle_flags, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_particle_candidate_count, 0, sizeof(int32_t)));
    CUDA_CHECK(cudaMemset(d_movement_moved, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_spin, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_color, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_flavor, 0, N * sizeof(int8_t)));
    CUDA_CHECK(cudaMemset(d_accel_mag, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_phi, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_phi_coulomb, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_L_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_L_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_L_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_R_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_R_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_R_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_L_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_L_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_L_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_R_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_R_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_R_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_L_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_L_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_L_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_R_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_R_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_delta_j_R_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_strong_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_strong_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_strong_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_strong_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_strong_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_strong_z, 0, N * sizeof(double)));

    CUDA_CHECK(cudaMemset(d_flux_weak_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_weak_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_flux_weak_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_weak_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_weak_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_wave_vel_weak_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_near_particle, 0, N * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemset(d_near_accel, 0, N * sizeof(double)));
    // Force-diagnostic mirrors
    CUDA_CHECK(cudaMemset(d_fd_coulomb_x,  0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_coulomb_y,  0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_coulomb_z,  0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_strong_x,   0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_strong_y,   0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_strong_z,   0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_magnetic_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_magnetic_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_magnetic_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_gravity_x,  0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_gravity_y,  0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_gravity_z,  0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_exchange_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_exchange_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_fd_exchange_z, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_causal_projection_events, 0, sizeof(unsigned long long)));

    CUDA_CHECK(cudaMemset(d_poisson_charge_sum, 0, sizeof(long long)));
    CUDA_CHECK(cudaMemset(d_poisson_mean_charge, 0, sizeof(double)));
    CUDA_CHECK(cudaMemset(d_plist_idx, 0, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_num_particles, 0, sizeof(int)));
    CUDA_CHECK(cudaMemset(d_particle_overflow, 0, sizeof(int)));
    CUDA_CHECK(cudaMemset(d_tick, 0, sizeof(int)));
    CUDA_CHECK(cudaMemset(d_pair_id, 0xFF, N * sizeof(int32_t))); // -1
    CUDA_CHECK(cudaMemset(d_ledger_rho_before, 0, N * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_ledger_reaction, 0, N * sizeof(int)));
    CUDA_CHECK(cudaMemset(d_ledger_current_x, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_ledger_current_y, 0, N * sizeof(double)));
    CUDA_CHECK(cudaMemset(d_ledger_current_z, 0, N * sizeof(double)));
    } catch (...) {
        // GpuBuffers owns raw CUDA pointers. Its destructor is not entered if
        // allocation is called from a constructor that has not completed, so
        // release every successfully allocated prefix before propagating.
        free();
        throw;
    }
}

void GpuBuffers::free() {
    if (visual_capture_quarantined) return;
    // Check the visual event before freeing *any* SoA source buffer: a capture
    // kernel reads state/remainder/spin/color, so releasing those first would
    // either be unsafe or make cudaFree implicitly wait for a hung stream.
    // A pending event is handled as a source-lifecycle barrier below; a real
    // event failure is terminal and recovered by CUDA-context replacement.
    if (visual_snapshot_ready) {
        const cudaError_t status = cudaEventQuery(visual_snapshot_ready);
        if (status == cudaErrorNotReady) {
            // This is a source-replacement lifecycle violation, not a CUDA
            // fault.  The native visual scheduler must observe
            // visual_snapshot_safe_to_replace()==true before it releases the
            // bridge.  Do not synchronize here: a stuck GPU must not turn a
            // desktop recovery into a freeze.  The object is intentionally
            // left intact for its owning context/process to retire.
            return;
        }
        if (status != cudaSuccess) {
            // A real CUDA event failure is terminal for this source.  Avoid
            // every dependent cudaFree/host unpin (which may wait forever)
            // and let context/process replacement reclaim the allocations.
            visual_capture_quarantined = true;
            return;
        }
    }
    // Drain and release the engine stream BEFORE any device pointer it may
    // still reference is freed. The quarantine/pending-visual early returns
    // above (`if (visual_capture_quarantined) return;` and the
    // cudaErrorNotReady/event-failure branches) skip this destroy, exactly
    // as they already skip every cudaFree below for the same reason: those
    // paths hand the allocation off to CUDA-context replacement rather than
    // resolve it here, deliberately. The stream leaks in that case along
    // with every other GPU buffer already does — this is the established
    // pattern in this function, not a gap introduced by this task.
    if (stream) {
        cudaStreamSynchronize(stream);
        cudaStreamDestroy(stream);
        stream = nullptr;
    }
    if (d_state)         { cudaFree(d_state); d_state = nullptr; }
    if (d_flux_x)        { cudaFree(d_flux_x); d_flux_x = nullptr; }
    if (d_flux_y)        { cudaFree(d_flux_y); d_flux_y = nullptr; }
    if (d_flux_z)        { cudaFree(d_flux_z); d_flux_z = nullptr; }
    if (d_wave_vel_x)    { cudaFree(d_wave_vel_x); d_wave_vel_x = nullptr; }
    if (d_wave_vel_y)    { cudaFree(d_wave_vel_y); d_wave_vel_y = nullptr; }
    if (d_wave_vel_z)    { cudaFree(d_wave_vel_z); d_wave_vel_z = nullptr; }
    if (d_velocity_x)    { cudaFree(d_velocity_x); d_velocity_x = nullptr; }
    if (d_velocity_y)    { cudaFree(d_velocity_y); d_velocity_y = nullptr; }
    if (d_velocity_z)    { cudaFree(d_velocity_z); d_velocity_z = nullptr; }
    if (d_remainder_x)   { cudaFree(d_remainder_x); d_remainder_x = nullptr; }
    if (d_remainder_y)   { cudaFree(d_remainder_y); d_remainder_y = nullptr; }
    if (d_remainder_z)   { cudaFree(d_remainder_z); d_remainder_z = nullptr; }
    if (d_locked)        { cudaFree(d_locked); d_locked = nullptr; }
    if (d_particle_id)   { cudaFree(d_particle_id); d_particle_id = nullptr; }
    if (d_next_particle_id) {
        cudaFree(d_next_particle_id);
        d_next_particle_id = nullptr;
    }
    if (d_next_pair_id) {
        cudaFree(d_next_pair_id);
        d_next_pair_id = nullptr;
    }
    if (d_identity_allocation_base) {
        cudaFree(d_identity_allocation_base);
        d_identity_allocation_base = nullptr;
    }
    if (d_identity_error) {
        cudaFree(d_identity_error);
        d_identity_error = nullptr;
    }
    if (d_spin)          { cudaFree(d_spin); d_spin = nullptr; }
    if (d_color)         { cudaFree(d_color); d_color = nullptr; }
    if (d_flavor)        { cudaFree(d_flavor); d_flavor = nullptr; }
    if (d_accel_mag)     { cudaFree(d_accel_mag); d_accel_mag = nullptr; }
    if (d_phi)           { cudaFree(d_phi); d_phi = nullptr; }
    if (d_phi_coulomb)   { cudaFree(d_phi_coulomb); d_phi_coulomb = nullptr; }
    if (d_phi_latency)   { cudaFree(d_phi_latency); d_phi_latency = nullptr; }
    if (d_latency)       { cudaFree(d_latency); d_latency = nullptr; }
    if (d_tau)           { cudaFree(d_tau); d_tau = nullptr; }
    if (d_phase)         { cudaFree(d_phase); d_phase = nullptr; }
    if (d_delta_j_x)     { cudaFree(d_delta_j_x); d_delta_j_x = nullptr; }
    if (d_delta_j_y)     { cudaFree(d_delta_j_y); d_delta_j_y = nullptr; }
    if (d_delta_j_z)     { cudaFree(d_delta_j_z); d_delta_j_z = nullptr; }
    if (d_flux_L_x)      { cudaFree(d_flux_L_x); d_flux_L_x = nullptr; }
    if (d_flux_L_y)      { cudaFree(d_flux_L_y); d_flux_L_y = nullptr; }
    if (d_flux_L_z)      { cudaFree(d_flux_L_z); d_flux_L_z = nullptr; }
    if (d_flux_R_x)      { cudaFree(d_flux_R_x); d_flux_R_x = nullptr; }
    if (d_flux_R_y)      { cudaFree(d_flux_R_y); d_flux_R_y = nullptr; }
    if (d_flux_R_z)      { cudaFree(d_flux_R_z); d_flux_R_z = nullptr; }
    if (d_wave_vel_L_x)  { cudaFree(d_wave_vel_L_x); d_wave_vel_L_x = nullptr; }
    if (d_wave_vel_L_y)  { cudaFree(d_wave_vel_L_y); d_wave_vel_L_y = nullptr; }
    if (d_wave_vel_L_z)  { cudaFree(d_wave_vel_L_z); d_wave_vel_L_z = nullptr; }
    if (d_wave_vel_R_x)  { cudaFree(d_wave_vel_R_x); d_wave_vel_R_x = nullptr; }
    if (d_wave_vel_R_y)  { cudaFree(d_wave_vel_R_y); d_wave_vel_R_y = nullptr; }
    if (d_wave_vel_R_z)  { cudaFree(d_wave_vel_R_z); d_wave_vel_R_z = nullptr; }
    if (d_delta_j_L_x)   { cudaFree(d_delta_j_L_x); d_delta_j_L_x = nullptr; }
    if (d_delta_j_L_y)   { cudaFree(d_delta_j_L_y); d_delta_j_L_y = nullptr; }
    if (d_delta_j_L_z)   { cudaFree(d_delta_j_L_z); d_delta_j_L_z = nullptr; }
    if (d_delta_j_R_x)   { cudaFree(d_delta_j_R_x); d_delta_j_R_x = nullptr; }
    if (d_delta_j_R_y)   { cudaFree(d_delta_j_R_y); d_delta_j_R_y = nullptr; }
    if (d_delta_j_R_z)   { cudaFree(d_delta_j_R_z); d_delta_j_R_z = nullptr; }
    if (d_flux_strong_x)     { cudaFree(d_flux_strong_x); d_flux_strong_x = nullptr; }
    if (d_flux_strong_y)     { cudaFree(d_flux_strong_y); d_flux_strong_y = nullptr; }
    if (d_flux_strong_z)     { cudaFree(d_flux_strong_z); d_flux_strong_z = nullptr; }
    if (d_wave_vel_strong_x) { cudaFree(d_wave_vel_strong_x); d_wave_vel_strong_x = nullptr; }
    if (d_wave_vel_strong_y) { cudaFree(d_wave_vel_strong_y); d_wave_vel_strong_y = nullptr; }
    if (d_wave_vel_strong_z) { cudaFree(d_wave_vel_strong_z); d_wave_vel_strong_z = nullptr; }

    if (d_flux_weak_x)     { cudaFree(d_flux_weak_x); d_flux_weak_x = nullptr; }
    if (d_flux_weak_y)     { cudaFree(d_flux_weak_y); d_flux_weak_y = nullptr; }
    if (d_flux_weak_z)     { cudaFree(d_flux_weak_z); d_flux_weak_z = nullptr; }
    if (d_wave_vel_weak_x) { cudaFree(d_wave_vel_weak_x); d_wave_vel_weak_x = nullptr; }
    if (d_wave_vel_weak_y) { cudaFree(d_wave_vel_weak_y); d_wave_vel_weak_y = nullptr; }
    if (d_wave_vel_weak_z) { cudaFree(d_wave_vel_weak_z); d_wave_vel_weak_z = nullptr; }
    if (d_near_particle) { cudaFree(d_near_particle); d_near_particle = nullptr; }
    if (d_near_accel)    { cudaFree(d_near_accel); d_near_accel = nullptr; }
    if (d_fd_coulomb_x)  { cudaFree(d_fd_coulomb_x);  d_fd_coulomb_x  = nullptr; }
    if (d_fd_coulomb_y)  { cudaFree(d_fd_coulomb_y);  d_fd_coulomb_y  = nullptr; }
    if (d_fd_coulomb_z)  { cudaFree(d_fd_coulomb_z);  d_fd_coulomb_z  = nullptr; }
    if (d_fd_strong_x)   { cudaFree(d_fd_strong_x);   d_fd_strong_x   = nullptr; }
    if (d_fd_strong_y)   { cudaFree(d_fd_strong_y);   d_fd_strong_y   = nullptr; }
    if (d_fd_strong_z)   { cudaFree(d_fd_strong_z);   d_fd_strong_z   = nullptr; }
    if (d_fd_magnetic_x) { cudaFree(d_fd_magnetic_x); d_fd_magnetic_x = nullptr; }
    if (d_fd_magnetic_y) { cudaFree(d_fd_magnetic_y); d_fd_magnetic_y = nullptr; }
    if (d_fd_magnetic_z) { cudaFree(d_fd_magnetic_z); d_fd_magnetic_z = nullptr; }
    if (d_fd_gravity_x)  { cudaFree(d_fd_gravity_x);  d_fd_gravity_x  = nullptr; }
    if (d_fd_gravity_y)  { cudaFree(d_fd_gravity_y);  d_fd_gravity_y  = nullptr; }
    if (d_fd_gravity_z)  { cudaFree(d_fd_gravity_z);  d_fd_gravity_z  = nullptr; }
    if (d_fd_exchange_x) { cudaFree(d_fd_exchange_x); d_fd_exchange_x = nullptr; }
    if (d_fd_exchange_y) { cudaFree(d_fd_exchange_y); d_fd_exchange_y = nullptr; }
    if (d_fd_exchange_z) { cudaFree(d_fd_exchange_z); d_fd_exchange_z = nullptr; }
    if (d_tick) { cudaFree(d_tick); d_tick = nullptr; }
    if (d_particle_overflow) {
        cudaFree(d_particle_overflow);
        d_particle_overflow = nullptr;
    }
    if (d_causal_projection_events) {
        cudaFree(d_causal_projection_events);
        d_causal_projection_events = nullptr;
    }
    if (d_poisson_charge_sum) {
        cudaFree(d_poisson_charge_sum);
        d_poisson_charge_sum = nullptr;
    }
    if (d_poisson_mean_charge) {
        cudaFree(d_poisson_mean_charge);
        d_poisson_mean_charge = nullptr;
    }
    if (d_fft_buf)       { cudaFree(d_fft_buf); d_fft_buf = nullptr; }
    if (d_fft_buf_f)     { cudaFree(d_fft_buf_f); d_fft_buf_f = nullptr; }
    if (d_green)         { cudaFree(d_green); d_green = nullptr; }
    if (d_visual_flux_magnitude) {
        cudaFree(d_visual_flux_magnitude);
        d_visual_flux_magnitude = nullptr;
    }
    if (d_visual_flux_plane) {
        cudaFree(d_visual_flux_plane);
        d_visual_flux_plane = nullptr;
    }
    if (d_compact_diagnostics) {
        cudaFree(d_compact_diagnostics);
        d_compact_diagnostics = nullptr;
    }
    if (d_compact_charge_sum) {
        cudaFree(d_compact_charge_sum);
        d_compact_charge_sum = nullptr;
    }
    // A telemetry copy is issued on the default stream. Synchronize its
    // per-engine fence before releasing pinned memory; unlike a raw device
    // pointer, the host target is not protected by cudaFree's stream order.
    if (telemetry_snapshot_ready) {
        cudaEventSynchronize(telemetry_snapshot_ready);
        cudaEventDestroy(telemetry_snapshot_ready);
        telemetry_snapshot_ready = nullptr;
    }
    if (h_telemetry_snapshot) {
        cudaFreeHost(h_telemetry_snapshot);
        h_telemetry_snapshot = nullptr;
    }
    if (d_telemetry_snapshot) {
        cudaFree(d_telemetry_snapshot);
        d_telemetry_snapshot = nullptr;
    }
    // At entry the event was queried successfully, before any source buffer
    // release.  It is therefore safe to free its fixed staging without a
    // stream synchronization.  Pending/faulted captures returned early above
    // and leave the whole engine quarantined for context/process replacement.
    if (visual_snapshot_ready) {
        cudaEventDestroy(visual_snapshot_ready);
        visual_snapshot_ready = nullptr;
    }
    if (h_visual_particle_records) cudaFreeHost(h_visual_particle_records);
    if (h_visual_particle_header) cudaFreeHost(h_visual_particle_header);
    if (d_visual_particle_records) cudaFree(d_visual_particle_records);
    if (d_visual_particle_header) cudaFree(d_visual_particle_header);
    h_visual_particle_records = nullptr;
    h_visual_particle_header = nullptr;
    d_visual_particle_records = nullptr;
    d_visual_particle_header = nullptr;

    // This is a MAPPED VIEW, not an allocation -- do not cudaFree() it.
    // Destroying the parent external memory object (below) invalidates it.
    d_interop_particle_buffer = nullptr;
    if (interop_external_memory) {
        cudaDestroyExternalMemory(interop_external_memory);
        interop_external_memory = nullptr;
    }
    // Unlike its closest analogues -- visual_snapshot_ready and
    // telemetry_snapshot_ready, both explicitly drained above -- this event
    // is destroyed unconditionally, with no pending-check guard. That is
    // safe because its only producer, launch_interop_particle_gather()
    // (gpu_engine.cu), records it exclusively on bufs_.stream, and
    // bufs_.stream has already been fully drained via
    // cudaStreamSynchronize(stream) and destroyed earlier in this same
    // function -- so by the time this line runs, no D2H copy referencing
    // h_interop_header can still be in flight.
    if (interop_gather_ready) {
        cudaEventDestroy(interop_gather_ready);
        interop_gather_ready = nullptr;
    }
    if (h_interop_header) cudaFreeHost(h_interop_header);
    if (d_interop_header) cudaFree(d_interop_header);
    h_interop_header = nullptr;
    d_interop_header = nullptr;

    // Cross-API GPU-timeline fence (Task 7). Same "safe unconditional
    // destroy" reasoning as interop_gather_ready above: the only stream that
    // ever signals it, bufs_.stream, is already drained+destroyed earlier in
    // this function.
    if (interop_fence) {
        cudaDestroyExternalSemaphore(interop_fence);
        interop_fence = nullptr;
    }

    if (d_plist_idx)     { cudaFree(d_plist_idx); d_plist_idx = nullptr; }
    if (d_num_particles) { cudaFree(d_num_particles); d_num_particles = nullptr; }
    if (d_particle_flags) {
        cudaFree(d_particle_flags);
        d_particle_flags = nullptr;
    }
    if (d_particle_candidate_indices) {
        cudaFree(d_particle_candidate_indices);
        d_particle_candidate_indices = nullptr;
    }
    if (d_particle_candidate_count) {
        cudaFree(d_particle_candidate_count);
        d_particle_candidate_count = nullptr;
    }
    if (d_pair_id)       { cudaFree(d_pair_id); d_pair_id = nullptr; }
    if (d_pair_candidate_flags) {
        cudaFree(d_pair_candidate_flags);
        d_pair_candidate_flags = nullptr;
    }
    if (d_pair_candidate_indices) {
        cudaFree(d_pair_candidate_indices);
        d_pair_candidate_indices = nullptr;
    }
    if (d_pair_candidate_count) {
        cudaFree(d_pair_candidate_count);
        d_pair_candidate_count = nullptr;
    }
    if (d_movement_moved) {
        cudaFree(d_movement_moved);
        d_movement_moved = nullptr;
    }
    if (d_movement_order) {
        cudaFree(d_movement_order);
        d_movement_order = nullptr;
    }
    if (d_movement_rank) {
        cudaFree(d_movement_rank);
        d_movement_rank = nullptr;
    }
    if (d_pair_select_temp) {
        cudaFree(d_pair_select_temp);
        d_pair_select_temp = nullptr;
    }
    pair_select_temp_bytes = 0;
    if (d_ledger_rho_before) { cudaFree(d_ledger_rho_before); d_ledger_rho_before = nullptr; }
    if (d_ledger_reaction)   { cudaFree(d_ledger_reaction); d_ledger_reaction = nullptr; }
    if (d_ledger_current_x)  { cudaFree(d_ledger_current_x); d_ledger_current_x = nullptr; }
    if (d_ledger_current_y)  { cudaFree(d_ledger_current_y); d_ledger_current_y = nullptr; }
    if (d_ledger_current_z)  { cudaFree(d_ledger_current_z); d_ledger_current_z = nullptr; }
    auto free_d = [](auto*& p) {
        if (p) { cudaFree(p); p = nullptr; }
    };
    free_d(d_matched_ex); free_d(d_matched_ey); free_d(d_matched_ez);
    free_d(d_matched_bx); free_d(d_matched_by); free_d(d_matched_bz);
    free_d(d_matched_cx); free_d(d_matched_cy); free_d(d_matched_cz);
    if (d_matched_valid) { cudaFree(d_matched_valid); d_matched_valid = nullptr; }
    free_d(d_strong_t00); free_d(d_strong_sxx); free_d(d_strong_syy);
    free_d(d_strong_szz); free_d(d_strong_sxy); free_d(d_strong_sxz);
    free_d(d_strong_syz);
    if (d_strong_idx) { cudaFree(d_strong_idx); d_strong_idx = nullptr; }
    if (d_strong_id) { cudaFree(d_strong_id); d_strong_id = nullptr; }
    if (d_strong_begin_id) { cudaFree(d_strong_begin_id); d_strong_begin_id = nullptr; }
    if (d_strong_color) { cudaFree(d_strong_color); d_strong_color = nullptr; }
    free_d(d_strong_px); free_d(d_strong_py); free_d(d_strong_pz);
    free_d(d_strong_mx); free_d(d_strong_my); free_d(d_strong_mz);
    if (d_strong_count) { cudaFree(d_strong_count); d_strong_count = nullptr; }
    if (d_strong_step) { cudaFree(d_strong_step); d_strong_step = nullptr; }
    N = 0;
    L = 0;
}

// ---------- AoS → SoA Upload ----------

void GpuBuffers::upload(const std::vector<Voxel>& host_voxels,
                        const std::vector<double>& host_phi,
                        const std::vector<double>& host_phi_coulomb) {
    upload_voxels(host_voxels);
    CUDA_CHECK(cudaMemcpy(d_phi, host_phi.data(), N * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_phi_coulomb, host_phi_coulomb.data(), N * sizeof(double), cudaMemcpyHostToDevice));
}

// ─── C5: upload instrumentation + delta-diff helpers ───────────────────────
std::size_t g_gpu_upload_bytes      = 0;
bool        g_gpu_force_full_upload = false;
std::size_t g_gpu_full_voxel_download_bytes = 0;
std::size_t g_gpu_full_voxel_download_calls = 0;
std::size_t g_gpu_identity_counter_download_bytes = 0;
std::size_t g_gpu_identity_counter_download_calls = 0;

namespace {
// Bitwise inequality of two doubles' object representations. We compare BITS,
// not values, so the delta path reproduces a full upload byte-for-byte:
// +0.0/-0.0 and distinct NaN payloads are genuinely different device bytes and
// must be re-uploaded if they differ (value-equality `==` would wrongly treat
// -0.0 == +0.0 as unchanged and let the device drift from the full-upload
// result).
inline bool bits_differ(double a, double b) {
    std::uint64_t ua, ub;
    std::memcpy(&ua, &a, sizeof(ua));
    std::memcpy(&ub, &b, sizeof(ub));
    return ua != ub;
}
inline bool vec_differ(const Vec3& a, const Vec3& b) {
    return bits_differ(a.x, b.x) || bits_differ(a.y, b.y) || bits_differ(a.z, b.z);
}
// True iff any field that upload_voxels_range() uploads differs between a and b.
// MUST stay in lockstep with the field set copied in upload_voxels_range()
// below — if a field is uploaded there but omitted here, the delta path can
// silently skip a changed voxel and diverge from a full upload.
inline bool uploaded_fields_differ(const Voxel& a, const Voxel& b) {
    return a.state != b.state
        || a.color != b.color
        || a.flavor != b.flavor
        || a.spin != b.spin
        || a.locked != b.locked
        || a.particle_id != b.particle_id
        || a.pair_id != b.pair_id
        || bits_differ(a.accel_mag, b.accel_mag)
        || bits_differ(a.latency, b.latency)
        || bits_differ(a.tau, b.tau)
        || bits_differ(a.phase, b.phase)
        || vec_differ(a.flux, b.flux)
        || vec_differ(a.wave_vel, b.wave_vel)
        || vec_differ(a.velocity, b.velocity)
        || vec_differ(a.remainder, b.remainder)
        || vec_differ(a.flux_L, b.flux_L)
        || vec_differ(a.flux_R, b.flux_R)
        || vec_differ(a.wave_vel_L, b.wave_vel_L)
        || vec_differ(a.wave_vel_R, b.wave_vel_R)
        || vec_differ(a.flux_strong, b.flux_strong)
        || vec_differ(a.wave_vel_strong, b.wave_vel_strong)
        || vec_differ(a.flux_weak, b.flux_weak)
        || vec_differ(a.wave_vel_weak, b.wave_vel_weak);
}
}  // namespace

void GpuBuffers::upload_voxels(const std::vector<Voxel>& host_voxels) {
    upload_voxels_range(host_voxels, 0, N);
}

void GpuBuffers::upload_voxels_range(const std::vector<Voxel>& host_voxels,
                                     int lo, int count) {
    if (count <= 0) return;

    // Scatter the AoS fields for [lo, lo+count) into per-field host staging,
    // then upload each staging array to the matching offset in its device SoA
    // array. Field set here is the single source of truth (see
    // uploaded_fields_differ).
    std::vector<int8_t>  h_state(count);
    std::vector<double>  h_fx(count), h_fy(count), h_fz(count);
    std::vector<double>  h_wvx(count), h_wvy(count), h_wvz(count);
    std::vector<double>  h_vx(count), h_vy(count), h_vz(count);
    std::vector<double>  h_rx(count), h_ry(count), h_rz(count);
    std::vector<uint8_t> h_locked(count);
    std::vector<int32_t> h_pid(count);
    std::vector<int8_t>  h_spin(count), h_color(count), h_flavor(count);
    std::vector<double>  h_accel(count);
    std::vector<int32_t> h_pair_id(count);
    std::vector<double>  h_latency(count);
    std::vector<double>  h_tau(count);
    std::vector<double>  h_phase(count);
    // Dual-substrate staging
    std::vector<double>  h_fLx(count), h_fLy(count), h_fLz(count);
    std::vector<double>  h_fRx(count), h_fRy(count), h_fRz(count);
    std::vector<double>  h_wvLx(count), h_wvLy(count), h_wvLz(count);
    std::vector<double>  h_wvRx(count), h_wvRy(count), h_wvRz(count);
    // Strong field staging
    std::vector<double>  h_fsx(count), h_fsy(count), h_fsz(count);
    std::vector<double>  h_wvsx(count), h_wvsy(count), h_wvsz(count);
    // Weak field staging
    std::vector<double>  h_fwx(count), h_fwy(count), h_fwz(count);
    std::vector<double>  h_wvwx(count), h_wvwy(count), h_wvwz(count);

    for (int k = 0; k < count; ++k) {
        const auto& v = host_voxels[lo + k];
        h_state[k]  = v.state;
        h_color[k]  = v.color;
        h_flavor[k] = v.flavor;
        h_fx[k]     = v.flux.x;
        h_fy[k]     = v.flux.y;
        h_fz[k]     = v.flux.z;
        h_wvx[k]    = v.wave_vel.x;
        h_wvy[k]    = v.wave_vel.y;
        h_wvz[k]    = v.wave_vel.z;
        h_vx[k]     = v.velocity.x;
        h_vy[k]     = v.velocity.y;
        h_vz[k]     = v.velocity.z;
        h_rx[k]     = v.remainder.x;
        h_ry[k]     = v.remainder.y;
        h_rz[k]     = v.remainder.z;
        h_locked[k] = v.locked ? 1 : 0;
        h_pid[k]    = v.particle_id;
        h_spin[k]   = v.spin;
        h_accel[k]  = v.accel_mag;
        h_pair_id[k] = v.pair_id;
        h_latency[k] = v.latency;
        h_tau[k]     = v.tau;
        h_phase[k]   = v.phase;
        // Dual-substrate
        h_fLx[k]  = v.flux_L.x;
        h_fLy[k]  = v.flux_L.y;
        h_fLz[k]  = v.flux_L.z;
        h_fRx[k]  = v.flux_R.x;
        h_fRy[k]  = v.flux_R.y;
        h_fRz[k]  = v.flux_R.z;
        h_wvLx[k] = v.wave_vel_L.x;
        h_wvLy[k] = v.wave_vel_L.y;
        h_wvLz[k] = v.wave_vel_L.z;
        h_wvRx[k] = v.wave_vel_R.x;
        h_wvRy[k] = v.wave_vel_R.y;
        h_wvRz[k] = v.wave_vel_R.z;
        // Strong field
        h_fsx[k]  = v.flux_strong.x;
        h_fsy[k]  = v.flux_strong.y;
        h_fsz[k]  = v.flux_strong.z;
        h_wvsx[k] = v.wave_vel_strong.x;
        h_wvsy[k] = v.wave_vel_strong.y;
        h_wvsz[k] = v.wave_vel_strong.z;
        // Weak field
        h_fwx[k]  = v.flux_weak.x;
        h_fwy[k]  = v.flux_weak.y;
        h_fwz[k]  = v.flux_weak.z;
        h_wvwx[k] = v.wave_vel_weak.x;
        h_wvwy[k] = v.wave_vel_weak.y;
        h_wvwz[k] = v.wave_vel_weak.z;
    }

    std::size_t bytes = 0;
    // Upload a staging array into dev[lo .. lo+count). Accumulates transfer
    // volume so both the full and delta paths are measured identically.
    #define FTD_UPLOAD_RANGE(dev, src, T)                                        \
        do {                                                                     \
            CUDA_CHECK(cudaMemcpy((dev) + lo, (src).data(),                      \
                                  static_cast<std::size_t>(count) * sizeof(T),   \
                                  cudaMemcpyHostToDevice));                      \
            bytes += static_cast<std::size_t>(count) * sizeof(T);                \
        } while (0)

    FTD_UPLOAD_RANGE(d_state, h_state, int8_t);
    FTD_UPLOAD_RANGE(d_flux_x, h_fx, double);
    FTD_UPLOAD_RANGE(d_flux_y, h_fy, double);
    FTD_UPLOAD_RANGE(d_flux_z, h_fz, double);
    FTD_UPLOAD_RANGE(d_wave_vel_x, h_wvx, double);
    FTD_UPLOAD_RANGE(d_wave_vel_y, h_wvy, double);
    FTD_UPLOAD_RANGE(d_wave_vel_z, h_wvz, double);
    FTD_UPLOAD_RANGE(d_velocity_x, h_vx, double);
    FTD_UPLOAD_RANGE(d_velocity_y, h_vy, double);
    FTD_UPLOAD_RANGE(d_velocity_z, h_vz, double);
    FTD_UPLOAD_RANGE(d_remainder_x, h_rx, double);
    FTD_UPLOAD_RANGE(d_remainder_y, h_ry, double);
    FTD_UPLOAD_RANGE(d_remainder_z, h_rz, double);
    FTD_UPLOAD_RANGE(d_locked, h_locked, uint8_t);
    FTD_UPLOAD_RANGE(d_particle_id, h_pid, int32_t);
    FTD_UPLOAD_RANGE(d_spin, h_spin, int8_t);
    FTD_UPLOAD_RANGE(d_color, h_color, int8_t);
    FTD_UPLOAD_RANGE(d_flavor, h_flavor, int8_t);
    FTD_UPLOAD_RANGE(d_accel_mag, h_accel, double);
    FTD_UPLOAD_RANGE(d_pair_id, h_pair_id, int32_t);
    FTD_UPLOAD_RANGE(d_latency, h_latency, double);
    FTD_UPLOAD_RANGE(d_tau, h_tau, double);
    FTD_UPLOAD_RANGE(d_phase, h_phase, double);
    // Dual-substrate
    FTD_UPLOAD_RANGE(d_flux_L_x, h_fLx, double);
    FTD_UPLOAD_RANGE(d_flux_L_y, h_fLy, double);
    FTD_UPLOAD_RANGE(d_flux_L_z, h_fLz, double);
    FTD_UPLOAD_RANGE(d_flux_R_x, h_fRx, double);
    FTD_UPLOAD_RANGE(d_flux_R_y, h_fRy, double);
    FTD_UPLOAD_RANGE(d_flux_R_z, h_fRz, double);
    FTD_UPLOAD_RANGE(d_wave_vel_L_x, h_wvLx, double);
    FTD_UPLOAD_RANGE(d_wave_vel_L_y, h_wvLy, double);
    FTD_UPLOAD_RANGE(d_wave_vel_L_z, h_wvLz, double);
    FTD_UPLOAD_RANGE(d_wave_vel_R_x, h_wvRx, double);
    FTD_UPLOAD_RANGE(d_wave_vel_R_y, h_wvRy, double);
    FTD_UPLOAD_RANGE(d_wave_vel_R_z, h_wvRz, double);
    // Strong field
    FTD_UPLOAD_RANGE(d_flux_strong_x, h_fsx, double);
    FTD_UPLOAD_RANGE(d_flux_strong_y, h_fsy, double);
    FTD_UPLOAD_RANGE(d_flux_strong_z, h_fsz, double);
    FTD_UPLOAD_RANGE(d_wave_vel_strong_x, h_wvsx, double);
    FTD_UPLOAD_RANGE(d_wave_vel_strong_y, h_wvsy, double);
    FTD_UPLOAD_RANGE(d_wave_vel_strong_z, h_wvsz, double);
    // Weak field
    FTD_UPLOAD_RANGE(d_flux_weak_x, h_fwx, double);
    FTD_UPLOAD_RANGE(d_flux_weak_y, h_fwy, double);
    FTD_UPLOAD_RANGE(d_flux_weak_z, h_fwz, double);
    FTD_UPLOAD_RANGE(d_wave_vel_weak_x, h_wvwx, double);
    FTD_UPLOAD_RANGE(d_wave_vel_weak_y, h_wvwy, double);
    FTD_UPLOAD_RANGE(d_wave_vel_weak_z, h_wvwz, double);

    #undef FTD_UPLOAD_RANGE
    g_gpu_upload_bytes += bytes;
}

void GpuBuffers::upload_voxels_delta(const std::vector<Voxel>& host_voxels,
                                     const std::vector<Voxel>& shadow) {
    // Fall back to a full upload when we cannot trust a partial one:
    //   - g_gpu_force_full_upload: test knob capturing the pre-C5 reference;
    //   - shadow not a valid device mirror of size N (cold start / resize).
    if (g_gpu_force_full_upload ||
        static_cast<int>(shadow.size()) != N ||
        static_cast<int>(host_voxels.size()) != N) {
        upload_voxels(host_voxels);
        return;
    }

    // Collect the indices whose uploaded fields changed vs the device shadow.
    std::vector<int> dirty;
    for (int i = 0; i < N; ++i) {
        if (uploaded_fields_differ(host_voxels[i], shadow[i])) dirty.push_back(i);
    }
    if (dirty.empty()) return;  // device already equals host_voxels — no-op

    // If a large fraction changed, one contiguous full upload is cheaper than
    // many small strided copies (and identical in effect). N/4 is a heuristic;
    // correctness holds for any threshold.
    if (static_cast<std::size_t>(dirty.size()) * 4 > static_cast<std::size_t>(N)) {
        upload_voxels(host_voxels);
        return;
    }

    // Coalesce the ascending dirty indices into maximal contiguous runs and
    // upload each run. Byte-identical to a full upload: the caller guarantees
    // device == shadow at every non-dirty index, so writing exactly the dirty
    // runs makes the device equal host_voxels everywhere.
    int run_lo = dirty[0];
    int run_hi = dirty[0];
    for (std::size_t k = 1; k < dirty.size(); ++k) {
        const int idx = dirty[k];
        if (idx == run_hi + 1) { run_hi = idx; continue; }
        upload_voxels_range(host_voxels, run_lo, run_hi - run_lo + 1);
        run_lo = idx;
        run_hi = idx;
    }
    upload_voxels_range(host_voxels, run_lo, run_hi - run_lo + 1);
}

namespace {

void raise_identity_counter(int32_t* device_counter, int32_t requested_next) {
    if (requested_next <= 0) return;
    int32_t current = 0;
    CUDA_CHECK(cudaMemcpy(&current, device_counter, sizeof(current),
                          cudaMemcpyDeviceToHost));
    if (requested_next <= current) return;
    CUDA_CHECK(cudaMemcpy(device_counter, &requested_next, sizeof(requested_next),
                          cudaMemcpyHostToDevice));
}

}  // namespace

void GpuBuffers::raise_identity_counters(int32_t next_particle_id,
                                         int32_t next_pair_id) {
    raise_identity_counter(d_next_particle_id, next_particle_id);
    raise_identity_counter(d_next_pair_id, next_pair_id);
}

void GpuBuffers::download_identity_counters(int32_t& next_particle_id,
                                            int32_t& next_pair_id) const {
    CUDA_CHECK(cudaMemcpy(&next_particle_id, d_next_particle_id,
                          sizeof(next_particle_id), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(&next_pair_id, d_next_pair_id,
                          sizeof(next_pair_id), cudaMemcpyDeviceToHost));
    g_gpu_identity_counter_download_bytes +=
        sizeof(next_particle_id) + sizeof(next_pair_id);
    ++g_gpu_identity_counter_download_calls;
}

void GpuBuffers::throw_if_identity_error() const {
    int32_t error = 0;
    CUDA_CHECK(cudaMemcpy(&error, d_identity_error, sizeof(error),
                          cudaMemcpyDeviceToHost));
    if (error != 0) {
        throw std::overflow_error(
            "GPU particle/pair identity namespace exhausted");
    }
}

// ---------- SoA → AoS Download ----------

void GpuBuffers::download(std::vector<Voxel>& host_voxels,
                          std::vector<double>& host_phi,
                          std::vector<double>& host_phi_coulomb) const {
    download_voxels(host_voxels);
    host_phi.resize(N);
    host_phi_coulomb.resize(N);
    CUDA_CHECK(cudaMemcpy(host_phi.data(), d_phi, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(host_phi_coulomb.data(), d_phi_coulomb, N * sizeof(double), cudaMemcpyDeviceToHost));
}

void GpuBuffers::download_states(std::vector<std::int8_t>& out) const {
    out.resize(static_cast<std::size_t>(N));
    CUDA_CHECK(cudaMemcpy(out.data(), d_state,
                          static_cast<std::size_t>(N) * sizeof(std::int8_t),
                          cudaMemcpyDeviceToHost));
}

__global__ void pack_visual_flux_magnitude_kernel(
    const double* flux_x,
    const double* flux_y,
    const double* flux_z,
    float* magnitude,
    int count) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= count) return;
    const double x = flux_x[i];
    const double y = flux_y[i];
    const double z = flux_z[i];
    magnitude[i] = static_cast<float>(sqrt(x * x + y * y + z * z));
}

void GpuBuffers::download_flux_magnitude(std::vector<float>& out) {
    if (!d_visual_flux_magnitude) {
        CUDA_CHECK(cudaMalloc(&d_visual_flux_magnitude,
                              static_cast<std::size_t>(N) * sizeof(float)));
    }

    constexpr int block = 256;
    const int grid = (N + block - 1) / block;
    pack_visual_flux_magnitude_kernel<<<grid, block, 0, stream>>>(
        d_flux_x, d_flux_y, d_flux_z, d_visual_flux_magnitude, N);
    CUDA_CHECK(cudaGetLastError());

    out.resize(static_cast<std::size_t>(N));
    CUDA_CHECK(cudaMemcpy(out.data(), d_visual_flux_magnitude,
                          static_cast<std::size_t>(N) * sizeof(float),
                          cudaMemcpyDeviceToHost));
}

__global__ void pack_visual_flux_plane_kernel(
    const double* flux_x,
    const double* flux_y,
    const double* flux_z,
    float* magnitude,
    int axis,
    int plane_index,
    int L) {
    const int q = blockIdx.x * blockDim.x + threadIdx.x;
    const int count = L * L;
    if (q >= count) return;
    const int a = q / L;
    const int b = q - a * L;
    int x, y, z;
    if (axis == 0) {
        x = plane_index; y = a; z = b;
    } else if (axis == 1) {
        x = a; y = plane_index; z = b;
    } else {
        x = a; y = b; z = plane_index;
    }
    const int i = x * L * L + y * L + z;
    const double fx = flux_x[i], fy = flux_y[i], fz = flux_z[i];
    magnitude[q] = static_cast<float>(sqrt(fx * fx + fy * fy + fz * fz));
}

void GpuBuffers::download_flux_magnitude_plane(int axis, int index,
                                                std::vector<float>& out) {
    axis = axis == 0 ? 0 : (axis == 1 ? 1 : 2);
    index %= L;
    if (index < 0) index += L;
    const int count = L * L;
    if (!d_visual_flux_plane) {
        CUDA_CHECK(cudaMalloc(&d_visual_flux_plane,
                              static_cast<std::size_t>(count) * sizeof(float)));
    }
    constexpr int block = 256;
    const int grid = (count + block - 1) / block;
    pack_visual_flux_plane_kernel<<<grid, block, 0, stream>>>(
        d_flux_x, d_flux_y, d_flux_z, d_visual_flux_plane,
        axis, index, L);
    CUDA_CHECK(cudaGetLastError());

    out.resize(static_cast<std::size_t>(count));
    CUDA_CHECK(cudaMemcpy(out.data(), d_visual_flux_plane,
                          static_cast<std::size_t>(count) * sizeof(float),
                          cudaMemcpyDeviceToHost));
}

void GpuBuffers::download_phi_latency(std::vector<double>& out) const {
    out.resize(N);
    CUDA_CHECK(cudaMemcpy(out.data(), d_phi_latency, N * sizeof(double),
                          cudaMemcpyDeviceToHost));
}

__global__ void reset_continuity_ledger_kernel(
    const int8_t* __restrict__ state,
    int* __restrict__ rho_before,
    int* __restrict__ reaction,
    double* __restrict__ current_x,
    double* __restrict__ current_y,
    double* __restrict__ current_z,
    int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    rho_before[i] = static_cast<int>(state[i]);
    reaction[i] = 0;
    current_x[i] = 0.0;
    current_y[i] = 0.0;
    current_z[i] = 0.0;
}

void GpuBuffers::throw_if_particle_overflow() const {
    if (!d_particle_overflow) return;
    int flag = 0;
    CUDA_CHECK(cudaMemcpy(&flag, d_particle_overflow, sizeof(int),
                          cudaMemcpyDeviceToHost));
    // Acknowledge-and-clear: the flag is sticky only UNTIL a caller observes
    // it here, not forever. Without this reset, a single transient overflow
    // (e.g. a manifestation burst that annihilation later brings back under
    // MAX_PARTICLES) would poison every future call for the rest of this
    // GpuBuffers' lifetime, even once the particle count is legitimately
    // back in bounds. The blocking cudaMemcpy immediately above already
    // forces completion of everything previously enqueued on `stream`
    // (this class's established invariant), so this async reset is safely
    // ordered before the next tick's finalize_particle_list_kernel launch
    // (also always issued on `stream`) without an extra synchronization.
    // Resetting an already-zero flag is a harmless no-op, so no branch on
    // `flag` is needed here.
    CUDA_CHECK(cudaMemsetAsync(d_particle_overflow, 0, sizeof(int), stream));
    if (flag != 0) {
        throw std::runtime_error(
            "[GpuEngine] manifested particle count exceeded the CUDA "
            "pairwise/triad capacity " + std::to_string(MAX_PARTICLES)
            + "; refusing to report partial color/Yukawa/exchange/triad "
              "physics");
    }
}

void GpuBuffers::reset_continuity_ledger() {
    constexpr int block = 256;
    const int grid = (N + block - 1) / block;
    reset_continuity_ledger_kernel<<<grid, block, 0, stream>>>(
        d_state, d_ledger_rho_before, d_ledger_reaction,
        d_ledger_current_x, d_ledger_current_y, d_ledger_current_z, N);
    CUDA_CHECK(cudaGetLastError());
    // The former cudaDeviceSynchronize() here was the single largest source
    // of per-tick host stalls: it ran first thing on EVERY tick. It is not
    // needed for correctness — the reset and every later phase are issued to
    // the same stream and CUDA serializes one stream in issue order — and it
    // is illegal inside a stream capture. Consumers of the ledger
    // (download_continuity_ledger) copy on the legacy stream, which
    // implicitly synchronizes with this blocking stream.
}

void GpuBuffers::download_continuity_ledger(
    std::vector<int>& rho_before,
    std::vector<int>& rho_after,
    std::vector<int>& reaction,
    std::vector<double>& current_x,
    std::vector<double>& current_y,
    std::vector<double>& current_z) const {
    rho_before.resize(N);
    rho_after.resize(N);
    reaction.resize(N);
    current_x.resize(N);
    current_y.resize(N);
    current_z.resize(N);

    std::vector<int8_t> state_after(static_cast<size_t>(N));
    CUDA_CHECK(cudaMemcpy(rho_before.data(), d_ledger_rho_before,
                          N * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(state_after.data(), d_state,
                          N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(reaction.data(), d_ledger_reaction,
                          N * sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(current_x.data(), d_ledger_current_x,
                          N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(current_y.data(), d_ledger_current_y,
                          N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(current_z.data(), d_ledger_current_z,
                          N * sizeof(double), cudaMemcpyDeviceToHost));

    for (int i = 0; i < N; ++i) {
        rho_after[static_cast<size_t>(i)] =
            static_cast<int>(state_after[static_cast<size_t>(i)]);
    }
}

void GpuBuffers::download_voxels(std::vector<Voxel>& host_voxels) const {
    // Exact byte count of the arrays copied below: 333 bytes/site.  Potential
    // and force mirrors are intentionally tracked separately from this core
    // counter; a compact diagnostics request must leave this value unchanged.
    constexpr std::size_t BYTES_PER_SITE = 333u;
    g_gpu_full_voxel_download_bytes += static_cast<std::size_t>(N) * BYTES_PER_SITE;
    ++g_gpu_full_voxel_download_calls;
    host_voxels.resize(N);

    // Download SoA arrays to host staging
    std::vector<int8_t>  h_state(N);
    std::vector<double>  h_fx(N), h_fy(N), h_fz(N);
    std::vector<double>  h_wvx(N), h_wvy(N), h_wvz(N);
    std::vector<double>  h_vx(N), h_vy(N), h_vz(N);
    std::vector<double>  h_rx(N), h_ry(N), h_rz(N);
    std::vector<uint8_t> h_locked(N);
    std::vector<int32_t> h_pid(N);
    std::vector<int8_t>  h_spin(N), h_color(N), h_flavor(N);
    std::vector<double>  h_accel(N);
    std::vector<int32_t> h_pair_id_dl(N);
    std::vector<double>  h_latency(N);
    std::vector<double>  h_tau(N);
    std::vector<double>  h_phase(N);

    CUDA_CHECK(cudaMemcpy(h_state.data(), d_state, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_latency.data(), d_latency, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_tau.data(), d_tau, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_phase.data(), d_phase, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fx.data(), d_flux_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fy.data(), d_flux_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fz.data(), d_flux_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvx.data(), d_wave_vel_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvy.data(), d_wave_vel_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvz.data(), d_wave_vel_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vx.data(), d_velocity_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vy.data(), d_velocity_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vz.data(), d_velocity_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_rx.data(), d_remainder_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_ry.data(), d_remainder_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_rz.data(), d_remainder_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_locked.data(), d_locked, N * sizeof(uint8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pid.data(), d_particle_id, N * sizeof(int32_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_spin.data(), d_spin, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_color.data(), d_color, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_flavor.data(), d_flavor, N * sizeof(int8_t), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_accel.data(), d_accel_mag, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pair_id_dl.data(), d_pair_id, N * sizeof(int32_t), cudaMemcpyDeviceToHost));

    // Download dual-substrate arrays
    std::vector<double> h_fLx(N), h_fLy(N), h_fLz(N);
    std::vector<double> h_fRx(N), h_fRy(N), h_fRz(N);
    std::vector<double> h_wvLx(N), h_wvLy(N), h_wvLz(N);
    std::vector<double> h_wvRx(N), h_wvRy(N), h_wvRz(N);

    // Strong field
    std::vector<double> h_fsx(N), h_fsy(N), h_fsz(N);
    std::vector<double> h_wvsx(N), h_wvsy(N), h_wvsz(N);
    // Weak field
    std::vector<double> h_fwx(N), h_fwy(N), h_fwz(N);
    std::vector<double> h_wvwx(N), h_wvwy(N), h_wvwz(N);

    CUDA_CHECK(cudaMemcpy(h_fLx.data(), d_flux_L_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fLy.data(), d_flux_L_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fLz.data(), d_flux_L_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fRx.data(), d_flux_R_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fRy.data(), d_flux_R_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fRz.data(), d_flux_R_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvLx.data(), d_wave_vel_L_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvLy.data(), d_wave_vel_L_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvLz.data(), d_wave_vel_L_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvRx.data(), d_wave_vel_R_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvRy.data(), d_wave_vel_R_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvRz.data(), d_wave_vel_R_z, N * sizeof(double), cudaMemcpyDeviceToHost));

    // Strong field
    CUDA_CHECK(cudaMemcpy(h_fsx.data(), d_flux_strong_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fsy.data(), d_flux_strong_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fsz.data(), d_flux_strong_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvsx.data(), d_wave_vel_strong_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvsy.data(), d_wave_vel_strong_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvsz.data(), d_wave_vel_strong_z, N * sizeof(double), cudaMemcpyDeviceToHost));

    // Weak field
    CUDA_CHECK(cudaMemcpy(h_fwx.data(), d_flux_weak_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fwy.data(), d_flux_weak_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fwz.data(), d_flux_weak_z, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvwx.data(), d_wave_vel_weak_x, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvwy.data(), d_wave_vel_weak_y, N * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_wvwz.data(), d_wave_vel_weak_z, N * sizeof(double), cudaMemcpyDeviceToHost));

    // Gather into AoS
    for (int i = 0; i < N; ++i) {
        auto& v = host_voxels[i];
        v.state       = h_state[i];
        v.flux        = {h_fx[i], h_fy[i], h_fz[i]};
        v.wave_vel    = {h_wvx[i], h_wvy[i], h_wvz[i]};
        v.velocity    = {h_vx[i], h_vy[i], h_vz[i]};
        v.remainder   = {h_rx[i], h_ry[i], h_rz[i]};
        v.locked      = h_locked[i] != 0;
        v.particle_id = h_pid[i];
        v.spin        = h_spin[i];
        v.color       = h_color[i];
        v.flavor      = h_flavor[i];
        v.accel_mag   = h_accel[i];
        // Dual-substrate fields
        v.flux_L      = {h_fLx[i], h_fLy[i], h_fLz[i]};
        v.flux_R      = {h_fRx[i], h_fRy[i], h_fRz[i]};
        v.wave_vel_L  = {h_wvLx[i], h_wvLy[i], h_wvLz[i]};
        v.wave_vel_R  = {h_wvRx[i], h_wvRy[i], h_wvRz[i]};
        // Strong field
        v.flux_strong      = {h_fsx[i], h_fsy[i], h_fsz[i]};
        v.wave_vel_strong  = {h_wvsx[i], h_wvsy[i], h_wvsz[i]};
        // Weak field
        v.flux_weak        = {h_fwx[i], h_fwy[i], h_fwz[i]};
        v.wave_vel_weak    = {h_wvwx[i], h_wvwy[i], h_wvwz[i]};
        // Pair ID from device
        v.pair_id     = h_pair_id_dl[i];
        // Latency field + proper time from GPU (when latency_field toggle is on)
        v.latency     = h_latency[i];
        v.tau         = h_tau[i];
        v.phase       = h_phase[i];
    }
}

// ---------- Green's Function Precomputation ----------

__global__ void kernel_precompute_green(double* green, int L) {
    int kx = blockIdx.x * blockDim.x + threadIdx.x;
    int ky = blockIdx.y * blockDim.y + threadIdx.y;
    int kz = blockIdx.z * blockDim.z + threadIdx.z;
    if (kx >= L || ky >= L || kz >= L) return;

    int idx = kx * L * L + ky * L + kz;  // X-major (matches CPU)

    // Isotropic 18-point Laplacian eigenvalue for periodic BC:
    // G(k) = (2/3)(cx+cy+cz-3) + (2/3)(cx*cy+cy*cz+cz*cx-3)
    // Cancels O(k^4) anisotropic term of the 6-point stencil.
    // Real-space: face weight=1/3, edge weight=1/6, center=-4
    double cx = cos(2.0 * M_PI * kx / L);
    double cy = cos(2.0 * M_PI * ky / L);
    double cz = cos(2.0 * M_PI * kz / L);
    double face = cx + cy + cz - 3.0;
    double edge = cx*cy + cy*cz + cz*cx - 3.0;
    double G = (2.0/3.0) * face + (2.0/3.0) * edge;

    // Store 1/G for spectral division. DC component (k=0,0,0) → 0 (gauge freedom)
    green[idx] = (kx == 0 && ky == 0 && kz == 0) ? 0.0 : 1.0 / G;
}

// ---------- Force-Diag Reset / Download ----------

void GpuBuffers::reset_force_diag() {
    // Zero all 15 component arrays so the next tick's force kernels see a
    // clean slate — matches the per-tick semantics callers expect when they
    // read force_diag_at(...) after tick(). State==0 voxels stay zero, which
    // is the natural default.
    CUDA_CHECK(cudaMemsetAsync(d_fd_coulomb_x, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_coulomb_y, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_coulomb_z, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_strong_x, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_strong_y, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_strong_z, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_magnetic_x, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_magnetic_y, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_magnetic_z, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_gravity_x, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_gravity_y, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_gravity_z, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_exchange_x, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_exchange_y, 0, N * sizeof(double), stream));
    CUDA_CHECK(cudaMemsetAsync(d_fd_exchange_z, 0, N * sizeof(double), stream));
}

unsigned long long GpuBuffers::download_causal_projection_events() const {
    unsigned long long value = 0;
    CUDA_CHECK(cudaMemcpy(&value, d_causal_projection_events,
                          sizeof(value), cudaMemcpyDeviceToHost));
    return value;
}

void GpuBuffers::download_force_diag(
    std::vector<double>& fc_x, std::vector<double>& fc_y, std::vector<double>& fc_z,
    std::vector<double>& fs_x, std::vector<double>& fs_y, std::vector<double>& fs_z,
    std::vector<double>& fm_x, std::vector<double>& fm_y, std::vector<double>& fm_z,
    std::vector<double>& fg_x, std::vector<double>& fg_y, std::vector<double>& fg_z,
    std::vector<double>& fe_x, std::vector<double>& fe_y, std::vector<double>& fe_z) const {
    fc_x.resize(N); fc_y.resize(N); fc_z.resize(N);
    fs_x.resize(N); fs_y.resize(N); fs_z.resize(N);
    fm_x.resize(N); fm_y.resize(N); fm_z.resize(N);
    fg_x.resize(N); fg_y.resize(N); fg_z.resize(N);
    fe_x.resize(N); fe_y.resize(N); fe_z.resize(N);
    const size_t bytes = N * sizeof(double);
    CUDA_CHECK(cudaMemcpy(fc_x.data(), d_fd_coulomb_x,  bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fc_y.data(), d_fd_coulomb_y,  bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fc_z.data(), d_fd_coulomb_z,  bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fs_x.data(), d_fd_strong_x,   bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fs_y.data(), d_fd_strong_y,   bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fs_z.data(), d_fd_strong_z,   bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fm_x.data(), d_fd_magnetic_x, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fm_y.data(), d_fd_magnetic_y, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fm_z.data(), d_fd_magnetic_z, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fg_x.data(), d_fd_gravity_x,  bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fg_y.data(), d_fd_gravity_y,  bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fg_z.data(), d_fd_gravity_z,  bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fe_x.data(), d_fd_exchange_x, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fe_y.data(), d_fd_exchange_y, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(fe_z.data(), d_fd_exchange_z, bytes, cudaMemcpyDeviceToHost));
}

void GpuBuffers::precompute_green_function() {
    dim3 block(4, 8, 8);  // 256 threads — better SM occupancy
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);
    kernel_precompute_green<<<grid, block, 0, stream>>>(d_green, L);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
}

void GpuBuffers::ensure_matched_gauss() {
    if (d_matched_ex) return;
    const std::size_t bytes = static_cast<std::size_t>(N) * sizeof(double);
    CUDA_CHECK(cudaMalloc(&d_matched_ex, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_ey, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_ez, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_bx, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_by, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_bz, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_cx, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_cy, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_cz, bytes));
    CUDA_CHECK(cudaMalloc(&d_matched_valid, sizeof(int)));
    CUDA_CHECK(cudaMemset(d_matched_ex, 0, bytes));
    CUDA_CHECK(cudaMemset(d_matched_ey, 0, bytes));
    CUDA_CHECK(cudaMemset(d_matched_ez, 0, bytes));
    CUDA_CHECK(cudaMemset(d_matched_bx, 0, bytes));
    CUDA_CHECK(cudaMemset(d_matched_by, 0, bytes));
    CUDA_CHECK(cudaMemset(d_matched_bz, 0, bytes));
}

void GpuBuffers::ensure_strong_stress() {
    if (d_strong_t00) return;
    const std::size_t bytes = static_cast<std::size_t>(N) * sizeof(double);
    const std::size_t pbytes =
        static_cast<std::size_t>(MAX_PARTICLES) * sizeof(double);
    CUDA_CHECK(cudaMalloc(&d_strong_t00, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_sxx, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_syy, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_szz, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_sxy, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_sxz, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_syz, bytes));
    CUDA_CHECK(cudaMalloc(&d_strong_idx, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_strong_id, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_strong_begin_id, MAX_PARTICLES * sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_strong_color, MAX_PARTICLES * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_strong_px, pbytes));
    CUDA_CHECK(cudaMalloc(&d_strong_py, pbytes));
    CUDA_CHECK(cudaMalloc(&d_strong_pz, pbytes));
    CUDA_CHECK(cudaMalloc(&d_strong_mx, pbytes));
    CUDA_CHECK(cudaMalloc(&d_strong_my, pbytes));
    CUDA_CHECK(cudaMalloc(&d_strong_mz, pbytes));
    CUDA_CHECK(cudaMalloc(&d_strong_count, sizeof(int)));
    CUDA_CHECK(cudaMalloc(&d_strong_step, sizeof(StrongStepDevice)));
}

}  // namespace gpu
}  // namespace ftd

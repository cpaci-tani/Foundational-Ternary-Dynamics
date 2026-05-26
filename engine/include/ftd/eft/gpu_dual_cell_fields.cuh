#pragma once
/**
 * @file ftd/eft/gpu_dual_cell_fields.cuh
 * @brief Device-side data structures for GPU-native EFT calculations.
 */

#include <cuda_runtime.h>

namespace ftd {
namespace eft {
namespace gpu {

struct GpuDualCellFields {
    int L = 0;
    int N = 0;                  // L^3
    int* d_rho_cell = nullptr;  // Device memory state {-1, 0, +1}
    double* d_phi_x = nullptr;  // Device memory face flux (average)
    double* d_phi_y = nullptr;
    double* d_phi_z = nullptr;

    void allocate(int size) {
        L = size;
        N = size * size * size;
        cudaMalloc(&d_rho_cell, N * sizeof(int));
        cudaMalloc(&d_phi_x, N * sizeof(double));
        cudaMalloc(&d_phi_y, N * sizeof(double));
        cudaMalloc(&d_phi_z, N * sizeof(double));
    }

    void free() {
        if (d_rho_cell) cudaFree(d_rho_cell);
        if (d_phi_x) cudaFree(d_phi_x);
        if (d_phi_y) cudaFree(d_phi_y);
        if (d_phi_z) cudaFree(d_phi_z);
        d_rho_cell = nullptr;
        d_phi_x = nullptr;
        d_phi_y = nullptr;
        d_phi_z = nullptr;
    }
};

struct GpuSnapshotPair {
    GpuDualCellFields before;
    GpuDualCellFields after;

    int L() const { return before.L; }
    int total_sites() const { return before.N; }
};

// Forward declaration of actual GpuBuffers structure
namespace gpu {
    struct GpuBuffers;
}

// --- GPU EFT Orchestration Interfaces ---

/**
 * @brief Converts GPU buffers (from active simulation) to GpuDualCellFields.
 */
void gpu_render_bridge_to_dual_cell_fields(const ftd::gpu::GpuBuffers& bufs, GpuDualCellFields& out);

/**
 * @brief Coarse-grains a fine snapshot into a coarse snapshot on-device (b=2).
 */
void gpu_block_dual_cell_b2(const GpuDualCellFields& fine, GpuDualCellFields& coarse);

/**
 * @brief Computes all 10 EFT operator averages on-device on a snapshot pair,
 *        writing the final 10 double-precision values to out_means.
 *        Out_means must point to a host array of size 10.
 */
void gpu_compute_eft_means(const GpuSnapshotPair& p, double* out_means);

} // namespace gpu
} // namespace eft
} // namespace ftd



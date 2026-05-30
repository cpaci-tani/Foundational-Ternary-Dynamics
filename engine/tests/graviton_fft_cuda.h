#pragma once
/**
 * graviton_fft_cuda.h — GPU (cuFFT) backend for the per-tick 3D FFTs of
 * campaign_graviton_tt_correlator.cpp.
 *
 * PURPOSE — pure, result-preserving performance optimization.
 * ─────────────────────────────────────────────────────────────────────────
 * The graviton-TT campaign FFTs ~15 real-loaded complex L³ grids every tick
 * (6 flux-quadrupole components + 6 stress components + 3 raw flux
 * components). On the CPU radix-2 path (ftd/spectral.h fft3d) this is the
 * campaign's wall-clock bottleneck: single-threaded, ~15 forward 3D FFTs per
 * tick, which makes the canonical L=128 run take 12+ hours.
 *
 * This header declares a thin GPU FFT service that computes EXACTLY the same
 * discrete Fourier transforms — double-precision (cuFFT Z2Z), unnormalized
 * forward transform, identical row-major [nx][ny][nz] index layout — only on
 * the GPU. It is a drop-in replacement for the campaign's CPU `fft3d`; no
 * physics, no operators, no projector, no decision logic changes.
 *
 * WHY Z2Z (double precision): the campaign's grids are std::complex<double>.
 * cuFFT Z2Z is the double-precision complex-to-complex transform; its memory
 * layout (cufftDoubleComplex = {double x, double y}) is bit-compatible with
 * std::complex<double>. Single-precision (C2C) would change rounding and is
 * NOT acceptable for a result-preserving swap.
 *
 * INDEX LAYOUT — identical to the campaign's CPU fft3d:
 *   The campaign lays a scalar field at flat index  x*L*L + y*L + z  (X-major,
 *   z is the fastest/contiguous axis). The CPU fft3d transforms along z
 *   (stride 1), then y (stride L), then x (stride L*L). A cuFFT 3D plan
 *   cufftPlan3d(L,L,L) / cufftPlanMany(rank=3, n={L,L,L}) treats the buffer
 *   as row-major [n0][n1][n2] with n2 the contiguous axis — i.e. exactly
 *   (x,y,z) with z contiguous. So a single batched 3D Z2Z forward transform
 *   over this buffer reproduces the CPU fft3d result bit-for-bit (modulo the
 *   usual floating-point reassociation of any FFT — the campaign's FFT-peak ω
 *   is a discrete bin and is robust to it; see the campaign's validation
 *   table).
 *
 * BATCHING + PLAN REUSE: the 15 same-size grids are transformed in ONE
 * cufftExecZ2Z call via a batched cufftPlanMany plan. The plan and the device
 * buffer are allocated once (per L) in the constructor and reused for every
 * tick — no per-tick planning, no per-tick device alloc.
 *
 * This file builds only when CUDA is available (the campaign is GPU_HEAVY).
 */

#include <complex>
#include <cstddef>
#include <vector>

#ifndef FTD_ENABLE_CUDA
#include <cassert>
#include <iostream>
#include <algorithm>
#include "ftd/spectral.h"
#endif

namespace ftd {
namespace graviton {

/**
 * Persistent batched double-precision 3D FFT service backed by cuFFT.
 *
 * Construct once with the lattice size L; call forward_batch() once per tick
 * with the set of L³ complex grids to transform in place. The cuFFT plan and
 * the device scratch buffer are allocated in the constructor and freed in the
 * destructor — there is no per-tick allocation or planning.
 */
class Fft3dBatchGpu {
public:
    /**
     * @param L           lattice edge length (grids are L³ complex values).
     * @param max_batch   maximum number of grids transformed in one call.
     *
     * Allocates a device buffer sized L³ * max_batch * sizeof(cufftDoubleComplex)
     * and a batched cufftPlanMany Z2Z plan. Aborts (CUDA/cuFFT error path) on
     * device failure — consistent with the engine's CUDA_CHECK convention.
     */
    Fft3dBatchGpu(int L, int max_batch);
    ~Fft3dBatchGpu();

    Fft3dBatchGpu(const Fft3dBatchGpu&)            = delete;
    Fft3dBatchGpu& operator=(const Fft3dBatchGpu&) = delete;

    /**
     * Forward, unnormalized 3D FFT of every grid in `grids`, in place.
     *
     * Each entry of `grids` points at an L^3 contiguous std::complex<double>
     * array laid out as x*L*L + y*L + z (the campaign's layout). On return,
     * each array holds its 3D DFT in the same index order. This matches the
     * campaign's old CPU fft3d called with inverse=false.
     *
     * grids.size() must be <= max_batch (asserted). The grids are uploaded to
     * the device buffer, transformed by a single batched cufftExecZ2Z, and
     * downloaded back into the same host arrays.
     */
    void forward_batch(const std::vector<std::complex<double>*>& grids);

private:
    int          L_         = 0;
    int          max_batch_ = 0;
    std::size_t  n_per_grid_ = 0;   // L³
    void*        d_buf_      = nullptr;  // cufftDoubleComplex* (device)
    // cufftHandle is an int typedef; stored as int to keep this header
    // free of the CUDA toolkit headers (the .cu owns those).
    int          plan_      = 0;
    bool         plan_made_ = false;
};

#ifndef FTD_ENABLE_CUDA
inline Fft3dBatchGpu::Fft3dBatchGpu(int L, int max_batch)
    : L_(L), max_batch_(max_batch) {
    assert(L > 0 && "lattice size must be positive");
    assert(max_batch > 0 && "batch size must be positive");
    n_per_grid_ = static_cast<std::size_t>(L_) * L_ * L_;
    std::cerr << "[graviton_fft_cuda] CPU fallback mode ready: L=" << L_ 
              << ", batch=" << max_batch_ << "\n";
}

inline Fft3dBatchGpu::~Fft3dBatchGpu() {}

inline void Fft3dBatchGpu::forward_batch(const std::vector<std::complex<double>*>& grids) {
    const int batch = static_cast<int>(grids.size());
    if (batch == 0) return;
    assert(batch <= max_batch_ && "more grids than plan max_batch");

    std::vector<std::complex<double>> temp(L_);

    for (int b = 0; b < batch; ++b) {
        std::complex<double>* grid = grids[b];
        assert(grid != nullptr && "null grid pointer");

        // 1. FFT along z-direction (stride 1)
        for (int x = 0; x < L_; ++x) {
            for (int y = 0; y < L_; ++y) {
                std::size_t offset = (static_cast<std::size_t>(x) * L_ + y) * L_;
                for (int z = 0; z < L_; ++z) {
                    temp[z] = grid[offset + z];
                }
                ftd::fft_1d(temp, false);
                for (int z = 0; z < L_; ++z) {
                    grid[offset + z] = temp[z];
                }
            }
        }

        // 2. FFT along y-direction (stride L)
        for (int x = 0; x < L_; ++x) {
            for (int z = 0; z < L_; ++z) {
                std::size_t offset_xz = static_cast<std::size_t>(x) * L_ * L_ + z;
                for (int y = 0; y < L_; ++y) {
                    temp[y] = grid[offset_xz + static_cast<std::size_t>(y) * L_];
                }
                ftd::fft_1d(temp, false);
                for (int y = 0; y < L_; ++y) {
                    grid[offset_xz + static_cast<std::size_t>(y) * L_] = temp[y];
                }
            }
        }

        // 3. FFT along x-direction (stride L*L)
        for (int y = 0; y < L_; ++y) {
            for (int z = 0; z < L_; ++z) {
                std::size_t offset_yz = static_cast<std::size_t>(y) * L_ + z;
                for (int x = 0; x < L_; ++x) {
                    temp[x] = grid[offset_yz + static_cast<std::size_t>(x) * L_ * L_];
                }
                ftd::fft_1d(temp, false);
                for (int x = 0; x < L_; ++x) {
                    grid[offset_yz + static_cast<std::size_t>(x) * L_ * L_] = temp[x];
                }
            }
        }
    }
}
#endif // FTD_ENABLE_CUDA

}  // namespace graviton
}  // namespace ftd

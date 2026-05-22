/**
 * graviton_fft_cuda.cu — cuFFT (GPU) implementation of the batched
 * double-precision 3D FFT service declared in graviton_fft_cuda.h.
 *
 * Pure, result-preserving performance optimization for
 * campaign_graviton_tt_correlator.cpp — see the header for the full rationale
 * and the bit-layout / index-layout correctness argument.
 *
 * The transform is double-precision (cufftExecZ2Z), unnormalized, forward,
 * 3D, batched over the campaign's ~15 same-size grids in a single exec call.
 * The cuFFT plan and the device buffer are created once per L and reused.
 */

#include "graviton_fft_cuda.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <cuda_runtime.h>
#include <cufft.h>

// Match the engine's CUDA error-handling convention (see cuda/kernels_poisson.cu).
#define G_CUDA_CHECK(call) do {                                              \
    cudaError_t _e = (call);                                                 \
    if (_e != cudaSuccess) {                                                 \
        std::fprintf(stderr, "[graviton_fft_cuda] CUDA error at %s:%d: %s\n", \
                     __FILE__, __LINE__, cudaGetErrorString(_e));            \
        std::exit(1);                                                        \
    }                                                                        \
} while (0)

#define G_CUFFT_CHECK(call) do {                                             \
    cufftResult _e = (call);                                                 \
    if (_e != CUFFT_SUCCESS) {                                               \
        std::fprintf(stderr, "[graviton_fft_cuda] cuFFT error at %s:%d: %d\n",\
                     __FILE__, __LINE__, (int)_e);                           \
        std::exit(1);                                                        \
    }                                                                        \
} while (0)

// std::complex<double> and cufftDoubleComplex are both two contiguous doubles
// {real, imag} / {x, y} with no padding; the campaign relies on this when it
// hands raw std::complex<double> arrays to this service. Verify at compile time.
static_assert(sizeof(std::complex<double>) == sizeof(cufftDoubleComplex),
              "std::complex<double> must be bit-compatible with cufftDoubleComplex");
static_assert(sizeof(std::complex<double>) == 2 * sizeof(double),
              "std::complex<double> must be exactly two doubles");

namespace ftd {
namespace graviton {

Fft3dBatchGpu::Fft3dBatchGpu(int L, int max_batch)
    : L_(L), max_batch_(max_batch) {
    assert(L > 0 && "lattice size must be positive");
    assert(max_batch > 0 && "batch size must be positive");

    n_per_grid_ = static_cast<std::size_t>(L) * static_cast<std::size_t>(L)
                * static_cast<std::size_t>(L);

    // Persistent device scratch: L³ * max_batch complex<double> values.
    const std::size_t bytes =
        n_per_grid_ * static_cast<std::size_t>(max_batch_) * sizeof(cufftDoubleComplex);
    G_CUDA_CHECK(cudaMalloc(&d_buf_, bytes));

    // Persistent batched 3D Z2Z plan: `max_batch_` independent 3D transforms
    // of an L×L×L grid laid out row-major (z contiguous), tightly packed
    // (idist = odist = L³). This is exactly the campaign's grid layout.
    int n[3] = {L_, L_, L_};
    cufftHandle plan = 0;
    G_CUFFT_CHECK(cufftPlanMany(
        &plan,
        /*rank   =*/3,
        /*n      =*/n,
        /*inembed=*/n, /*istride=*/1, /*idist=*/static_cast<int>(n_per_grid_),
        /*onembed=*/n, /*ostride=*/1, /*odist=*/static_cast<int>(n_per_grid_),
        /*type   =*/CUFFT_Z2Z,
        /*batch  =*/max_batch_));
    plan_      = static_cast<int>(plan);
    plan_made_ = true;

    std::fprintf(stderr,
        "[graviton_fft_cuda] cuFFT Z2Z 3D plan ready: L=%d, batch=%d, "
        "device buffer %.1f MiB\n",
        L_, max_batch_, bytes / (1024.0 * 1024.0));
}

Fft3dBatchGpu::~Fft3dBatchGpu() {
    if (plan_made_) {
        cufftDestroy(static_cast<cufftHandle>(plan_));
        plan_made_ = false;
    }
    if (d_buf_) {
        cudaFree(d_buf_);
        d_buf_ = nullptr;
    }
}

void Fft3dBatchGpu::forward_batch(
    const std::vector<std::complex<double>*>& grids) {
    const int batch = static_cast<int>(grids.size());
    if (batch == 0) return;
    assert(batch <= max_batch_ && "more grids than plan max_batch");

    cufftDoubleComplex* d_buf = static_cast<cufftDoubleComplex*>(d_buf_);

    // Upload each host grid into its slot of the contiguous device buffer.
    for (int b = 0; b < batch; ++b) {
        assert(grids[b] != nullptr && "null grid pointer");
        G_CUDA_CHECK(cudaMemcpy(
            d_buf + static_cast<std::size_t>(b) * n_per_grid_,
            grids[b],
            n_per_grid_ * sizeof(cufftDoubleComplex),
            cudaMemcpyHostToDevice));
    }

    // One batched, forward, unnormalized 3D Z2Z transform over all `batch`
    // grids. cuFFT's forward transform applies no 1/N scaling — this matches
    // the campaign's CPU fft3d called with inverse=false. When batch <
    // max_batch_, transforming the full plan would touch stale slots, so the
    // plan is sized to max_batch_ and the campaign always submits max_batch_
    // grids; the assert above guards the contract.
    G_CUFFT_CHECK(cufftExecZ2Z(static_cast<cufftHandle>(plan_),
                               d_buf, d_buf, CUFFT_FORWARD));

    // Download the transformed grids back into the same host arrays.
    for (int b = 0; b < batch; ++b) {
        G_CUDA_CHECK(cudaMemcpy(
            grids[b],
            d_buf + static_cast<std::size_t>(b) * n_per_grid_,
            n_per_grid_ * sizeof(cufftDoubleComplex),
            cudaMemcpyDeviceToHost));
    }
}

}  // namespace graviton
}  // namespace ftd

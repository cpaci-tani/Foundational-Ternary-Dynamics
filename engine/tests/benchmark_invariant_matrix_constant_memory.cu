// benchmark_invariant_matrix_constant_memory.cu
//
// Standalone benchmark for the CUDA constant-memory invariant pattern
// established by ADR-0014. Exercises:
//   - upload_invariant_matrix() populating c_A and c_consts
//   - apply_invariant_A() kernel reading c_A
//   - read_constants_for_check() kernel reading c_consts
//
// Inputs are bespoke seeded random buffers; no engine voxel state.
// Verification is element-wise parity against a CPU reference within
// 1e-14 relative error.
//
// Exit code 0 on PASS, non-zero on any mismatch.

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include "../cuda/cuda_invariants.cuh"
#include "ftd/constants.h"

#define CUDA_CHECK(call)                                                    \
    do {                                                                    \
        cudaError_t err__ = (call);                                         \
        if (err__ != cudaSuccess) {                                         \
            std::fprintf(stderr, "CUDA error %s:%d: %s\n",                  \
                         __FILE__, __LINE__, cudaGetErrorString(err__));    \
            std::exit(EXIT_FAILURE);                                        \
        }                                                                   \
    } while (0)

int main() {
    using namespace ftd::cuda;

    constexpr int N = 1 << 20;                  // 1,048,576 state vectors
    constexpr int blockSize = 256;
    const int gridSize = (N + blockSize - 1) / blockSize;

    const double Gstar    = ftd::ontic::G_STAR;
    const double varpi    = ftd::ontic::VARPI;
    const double invGstar = 1.0 / Gstar;

    // Inputs (deterministic seed for reproducibility).
    std::vector<double> hX(N), hY(N), hZ(N);
    std::mt19937_64 rng(0xC0FFEEULL);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (int i = 0; i < N; ++i) {
        hX[i] = dist(rng);
        hY[i] = dist(rng);
        hZ[i] = dist(rng);
    }

    // CPU reference: apply A to each vector in the SAME operation order as the
    // kernel, so the comparison is bit-exact at double precision (no FMA).
    std::vector<double> rX(N), rY(N), rZ(N);
    for (int i = 0; i < N; ++i) {
        const double x = hX[i], y = hY[i], z = hZ[i];
        rX[i] =   Gstar  * x +    0.0    * y +  (-varpi)   * z;
        rY[i] = (-varpi) * x +    1.0    * y +    0.0      * z;
        rZ[i] =    0.0   * x + (-varpi)  * y +  invGstar   * z;
    }

    // Device buffers.
    double *dX = nullptr, *dY = nullptr, *dZ = nullptr;
    double *oX = nullptr, *oY = nullptr, *oZ = nullptr;
    const size_t bytes = static_cast<size_t>(N) * sizeof(double);
    CUDA_CHECK(cudaMalloc(&dX, bytes));
    CUDA_CHECK(cudaMalloc(&dY, bytes));
    CUDA_CHECK(cudaMalloc(&dZ, bytes));
    CUDA_CHECK(cudaMalloc(&oX, bytes));
    CUDA_CHECK(cudaMalloc(&oY, bytes));
    CUDA_CHECK(cudaMalloc(&oZ, bytes));
    CUDA_CHECK(cudaMemcpy(dX, hX.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dY, hY.data(), bytes, cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(dZ, hZ.data(), bytes, cudaMemcpyHostToDevice));

    // Upload constant memory.
    upload_invariant_matrix();

    // Self-check c_consts.
    double *dCheck = nullptr;
    CUDA_CHECK(cudaMalloc(&dCheck, 3 * sizeof(double)));
    read_constants_for_check<<<1, 1>>>(dCheck);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    double hCheck[3] = {0.0, 0.0, 0.0};
    CUDA_CHECK(cudaMemcpy(hCheck, dCheck, 3 * sizeof(double),
                          cudaMemcpyDeviceToHost));
    if (hCheck[0] != Gstar || hCheck[1] != varpi || hCheck[2] != invGstar) {
        std::fprintf(stderr,
                     "FAIL: c_consts mismatch "
                     "(got %.17g, %.17g, %.17g; expected %.17g, %.17g, %.17g)\n",
                     hCheck[0], hCheck[1], hCheck[2], Gstar, varpi, invGstar);
        return 1;
    }
    cudaFree(dCheck);

    // Timed matrix-application launch.
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));
    apply_invariant_A<<<gridSize, blockSize>>>(dX, dY, dZ, oX, oY, oZ, N);
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    CUDA_CHECK(cudaGetLastError());

    // Pull results.
    std::vector<double> oXh(N), oYh(N), oZh(N);
    CUDA_CHECK(cudaMemcpy(oXh.data(), oX, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(oYh.data(), oY, bytes, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(oZh.data(), oZ, bytes, cudaMemcpyDeviceToHost));

    // Parity check (per-element max relative error).
    double maxRel = 0.0;
    int firstFail = -1;
    for (int i = 0; i < N; ++i) {
        const double ref = std::max({std::abs(rX[i]),
                                     std::abs(rY[i]),
                                     std::abs(rZ[i]),
                                     1e-300});
        const double dx = std::abs(oXh[i] - rX[i]) / ref;
        const double dy = std::abs(oYh[i] - rY[i]) / ref;
        const double dz = std::abs(oZh[i] - rZ[i]) / ref;
        const double m  = std::max({dx, dy, dz});
        if (m > maxRel) {
            maxRel = m;
            if (m > 1e-14 && firstFail < 0) firstFail = i;
        }
    }

    // Throughput + bandwidth report (read 3 doubles, write 3 doubles per state).
    const double states_per_sec =
        static_cast<double>(N) / (static_cast<double>(ms) * 1e-3);
    const double gbytes =
        (static_cast<double>(N) * 6.0 * sizeof(double))
        / (1024.0 * 1024.0 * 1024.0);
    const double gbps = gbytes / (static_cast<double>(ms) * 1e-3);

    std::printf("max_rel_error = %.3e\n", maxRel);
    std::printf("throughput    = %.3e states/sec\n", states_per_sec);
    std::printf("bandwidth     = %.3f GB/s\n", gbps);

    cudaFree(dX); cudaFree(dY); cudaFree(dZ);
    cudaFree(oX); cudaFree(oY); cudaFree(oZ);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    if (maxRel > 1e-14) {
        std::fprintf(stderr,
                     "FAIL: max relative error %.3e exceeds tolerance 1e-14 "
                     "(first fail i=%d)\n",
                     maxRel, firstFail);
        return 1;
    }
    std::printf("PASS\n");
    return 0;
}

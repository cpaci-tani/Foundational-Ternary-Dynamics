#pragma once
/**
 * @file cuda_error.cuh — shared CUDA / cuFFT error-check macros (revision C1).
 *
 * Consolidated from the definitions previously duplicated across 11 .cu TUs.
 * Host-side failures throw std::runtime_error so allocation/command boundaries
 * can recover and report the CUDA failure; kernels must not include this in
 * device code paths.
 *
 * CUFFT_CHECK is guarded so TUs that do not link cuFFT can include this
 * header without pulling <cufft.h>: define FTD_CUDA_ERROR_WANT_CUFFT before
 * including (kernels_poisson.cu does).
 */

#include <cstdio>
#include <stdexcept>
#include <string>
#include <cuda_runtime.h>

namespace ftd::gpu::detail {

[[noreturn]] inline void throw_cuda_error(cudaError_t err,
                                          const char* file,
                                          int line) {
    throw std::runtime_error(
        std::string("CUDA error at ") + file + ":" + std::to_string(line) +
        ": " + cudaGetErrorString(err));
}

}  // namespace ftd::gpu::detail

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        ::ftd::gpu::detail::throw_cuda_error(err, __FILE__, __LINE__); \
    } \
} while(0)

#ifdef FTD_CUDA_ERROR_WANT_CUFFT
#include <cufft.h>
namespace ftd::gpu::detail {

[[noreturn]] inline void throw_cufft_error(cufftResult err,
                                           const char* file,
                                           int line) {
    throw std::runtime_error(
        std::string("cuFFT error at ") + file + ":" + std::to_string(line) +
        ": code " + std::to_string(static_cast<int>(err)));
}

}  // namespace ftd::gpu::detail

#define CUFFT_CHECK(call) do { \
    cufftResult err = (call); \
    if (err != CUFFT_SUCCESS) { \
        ::ftd::gpu::detail::throw_cufft_error(err, __FILE__, __LINE__); \
    } \
} while(0)
#endif

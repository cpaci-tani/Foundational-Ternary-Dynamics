#pragma once
/**
 * @file cuda_error.cuh — shared CUDA / cuFFT error-check macros (revision C1).
 *
 * Byte-identical to the definitions previously duplicated across 11 .cu TUs
 * (verified by hash before consolidation) — a fix here now propagates
 * everywhere, per the ADR-0007 shared-header pattern. Host-side only
 * (fprintf/exit): kernels must not include this in device code paths.
 *
 * CUFFT_CHECK is guarded so TUs that do not link cuFFT can include this
 * header without pulling <cufft.h>: define FTD_CUDA_ERROR_WANT_CUFFT before
 * including (kernels_poisson.cu does).
 */

#include <cstdio>
#include <cstdlib>
#include <cuda_runtime.h>

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

#ifdef FTD_CUDA_ERROR_WANT_CUFFT
#include <cufft.h>
#define CUFFT_CHECK(call) do { \
    cufftResult err = (call); \
    if (err != CUFFT_SUCCESS) { \
        fprintf(stderr, "cuFFT error at %s:%d: %d\n", \
                __FILE__, __LINE__, (int)err); \
        exit(1); \
    } \
} while(0)
#endif

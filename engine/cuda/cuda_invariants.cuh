#pragma once
//
// cuda_invariants.cuh — CUDA constant-memory pattern for small read-only
// invariants (matrices and companion scalars).
//
// See docs/adr/0014-cuda-constant-memory-for-invariants.md for the pattern.
//
// Usage:
//     ftd::cuda::upload_invariant_matrix();              // ontic defaults
//     ftd::cuda::upload_invariant_matrix(Gstar, varpi);  // explicit
//     ftd::cuda::apply_invariant_A<<<grid, block>>>(...);
//
// The pattern is dormant for the existing engine — c_A and c_consts are
// only read by the kernels declared here. No production TU references
// them; the golden-tick hash and gpu_parity_complete sweep are unaffected.
//

#include <cuda_runtime.h>

namespace ftd {
namespace cuda {

// Row-major 3x3 invariant matrix in CUDA constant memory.
// Storage: c_A[3*i + j] is A_{ij}.
//
// Reference matrix populated by upload_invariant_matrix():
//   [  G*,    0,    -varpi  ]
//   [ -varpi, 1,     0      ]
//   [  0,   -varpi,  1/G*   ]
extern __constant__ double c_A[9];

// Companion constants {G_STAR, VARPI, invGstar = 1/G_STAR}.
//
// G_STAR = Gamma(1/4) / Gamma(3/4) ~ 2.9587  (lemniscatic constant)
// VARPI  = Gamma(1/4)^2 / (2*sqrt(2*pi))
//        = sqrt(2) * K(1/sqrt(2))   ~ 2.6221  (lemniscate constant)
//        NOTE: VARPI is NOT 2*K(1/sqrt(2)); see FTD-0117.
extern __constant__ double c_consts[3];

// Upload A and c_consts to device constant memory.
// Both arrays are populated synchronously via cudaMemcpyToSymbol.
// invGstar is computed host-side as 1.0 / Gstar.
void upload_invariant_matrix(double Gstar, double varpi);

// Convenience overload: uses ftd::ontic::G_STAR and ftd::ontic::VARPI.
void upload_invariant_matrix();

// Element-wise application of A to N state vectors in SoA layout.
// All inputs are device pointers of length n; output pointers may
// not alias input pointers.
__global__ void apply_invariant_A(
    const double* __restrict__ inX,
    const double* __restrict__ inY,
    const double* __restrict__ inZ,
    double* __restrict__ outX,
    double* __restrict__ outY,
    double* __restrict__ outZ,
    int n);

// Tiny self-check kernel: writes c_consts[0..2] into a 3-element device buffer.
// Exists so the benchmark can verify c_consts uploads correctly even though
// apply_invariant_A does not read c_consts directly.
__global__ void read_constants_for_check(double* out3);

}  // namespace cuda
}  // namespace ftd

// cuda_invariants.cu — Implementation of the CUDA constant-memory invariant
// pattern declared in cuda_invariants.cuh. See ADR-0014 for rationale.
//
// Defines:
//   c_A         — 3x3 row-major invariant matrix in __constant__ memory
//   c_consts    — {G_STAR, VARPI, invGstar} companion array
//   upload_invariant_matrix(double, double)
//   upload_invariant_matrix()           overload using ontic defaults
//   apply_invariant_A(...)              SoA element-wise A*v kernel
//   read_constants_for_check(double*)   tiny self-check kernel for c_consts

#include "cuda_invariants.cuh"

#include <cuda_runtime.h>

#include "ftd/constants.h"

namespace ftd {
namespace cuda {

// Constant-memory storage. Symbol prefix `c_` distinguishes device
// __constant__ from host ftd::ontic::* constexpr symbols (see ADR-0014).
__constant__ double c_A[9];
__constant__ double c_consts[3];

void upload_invariant_matrix(double Gstar, double varpi) {
    const double invGstar = 1.0 / Gstar;

    // Row-major: h_A[3*i + j] is A_{ij}.
    //   A_00 = Gstar,   A_01 = 0,      A_02 = -varpi
    //   A_10 = -varpi,  A_11 = 1,      A_12 = 0
    //   A_20 = 0,       A_21 = -varpi, A_22 = invGstar
    const double h_A[9] = {
         Gstar,   0.0,    -varpi,
        -varpi,   1.0,     0.0,
         0.0,    -varpi,   invGstar,
    };
    const double h_consts[3] = {Gstar, varpi, invGstar};

    cudaMemcpyToSymbol(c_A,      h_A,      sizeof(h_A));
    cudaMemcpyToSymbol(c_consts, h_consts, sizeof(h_consts));
}

void upload_invariant_matrix() {
    upload_invariant_matrix(ftd::ontic::G_STAR, ftd::ontic::VARPI);
}

__global__ void apply_invariant_A(
    const double* __restrict__ inX,
    const double* __restrict__ inY,
    const double* __restrict__ inZ,
    double* __restrict__ outX,
    double* __restrict__ outY,
    double* __restrict__ outZ,
    int n)
{
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;

    const double x = inX[i];
    const double y = inY[i];
    const double z = inZ[i];

    outX[i] = c_A[0] * x + c_A[1] * y + c_A[2] * z;
    outY[i] = c_A[3] * x + c_A[4] * y + c_A[5] * z;
    outZ[i] = c_A[6] * x + c_A[7] * y + c_A[8] * z;
}

__global__ void read_constants_for_check(double* out3) {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        out3[0] = c_consts[0];
        out3[1] = c_consts[1];
        out3[2] = c_consts[2];
    }
}

}  // namespace cuda
}  // namespace ftd

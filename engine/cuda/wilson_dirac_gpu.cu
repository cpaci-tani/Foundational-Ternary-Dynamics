/**
 * Wilson-Dirac GPU kernel (Phase II.2-E).
 *
 * Mirrors engine/src/wilson_dirac.cpp exactly: same operator, same constant
 * shift (m + 3r/a for 3D spatial), same chiral-basis gamma matrices, same
 * X-major site index. Used to validate that CPU and GPU produce numerically
 * identical results to ~1e-12.
 *
 * Per-site complexity: 6 stencil neighbours, 4 spinor components each, ~30
 * complex multiplies per site. Trivially memory-bound. One thread per site.
 */

#include "ftd/wilson_dirac_gpu.h"

#include <cuComplex.h>
#include <cuda_runtime.h>

#include <cstdio>
#include <cstdlib>

#include "cuda_index.cuh"

namespace ftd {
namespace wilson_dirac {

namespace {

using c2 = cuDoubleComplex;

__device__ __forceinline__ c2 cmake(double r, double i) { return make_cuDoubleComplex(r, i); }
__device__ __forceinline__ c2 cadd(c2 a, c2 b) { return cmake(a.x + b.x, a.y + b.y); }
__device__ __forceinline__ c2 csub(c2 a, c2 b) { return cmake(a.x - b.x, a.y - b.y); }
__device__ __forceinline__ c2 cmul(c2 a, c2 b) {
    return cmake(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}
__device__ __forceinline__ c2 cscalar(double s, c2 a) { return cmake(s * a.x, s * a.y); }
__device__ __forceinline__ c2 cconj(c2 a) { return cmake(a.x, -a.y); }

// pauli_i applied to (v0, v1):
//   sigma_x: (v1, v0)
//   sigma_y: (-i v1,  i v0)  ->  (v1.y, -v1.x), (-v0.y, v0.x)
//   sigma_z: (v0, -v1)
__device__ __forceinline__ void apply_pauli(int i, c2 v0, c2 v1, c2& o0, c2& o1) {
    if (i == 0) {
        o0 = v1; o1 = v0;
    } else if (i == 1) {
        o0 = cmake(v1.y, -v1.x);   // -i * v1
        o1 = cmake(-v0.y, v0.x);   // +i * v0
    } else {
        o0 = v0; o1 = csub(cmake(0.0, 0.0), v1);
    }
}

// Layout: psi has 4 cdouble components per site, packed as
//   psi_flat[site * 4 + k], k in {0,1,2,3}.
// Gauge: U[mu * N + site] -> one cdouble per direction per site.
__global__ void wilson_dirac_kernel(c2* __restrict__ out,
                                     const c2* __restrict__ psi,
                                     const c2* __restrict__ U,
                                     int L,
                                     double m, double r, double a) {
    const int N = L * L * L;
    const int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= N) return;

    int x, y, z;
    decode_xyz(idx, L, x, y, z);

    const double diag = m + 3.0 * r / a;
    const double off = 1.0 / (2.0 * a);

    c2 result[4];
    for (int k = 0; k < 4; ++k) result[k] = cscalar(diag, psi[idx * 4 + k]);

    for (int mu = 0; mu < 3; ++mu) {
        const int dx = (mu == 0) ? 1 : 0;
        const int dy = (mu == 1) ? 1 : 0;
        const int dz = (mu == 2) ? 1 : 0;
        const int idx_p = idx3d(x + dx, y + dy, z + dz, L);
        const int idx_m = idx3d(x - dx, y - dy, z - dz, L);

        const c2 U_n_mu = U[mu * N + idx];
        const c2 U_minus_dag = cconj(U[mu * N + idx_m]);

        c2 psi_xp[4], psi_xm[4];
        for (int k = 0; k < 4; ++k) {
            psi_xp[k] = cmul(U_n_mu, psi[idx_p * 4 + k]);
            psi_xm[k] = cmul(U_minus_dag, psi[idx_m * 4 + k]);
        }

        // (r - gamma^mu) psi_xp:  upper = r u + sigma_mu * lower; lower = r l - sigma_mu * upper
        c2 sl0, sl1, su0, su1;
        apply_pauli(mu, psi_xp[2], psi_xp[3], sl0, sl1);
        apply_pauli(mu, psi_xp[0], psi_xp[1], su0, su1);
        c2 rmg[4];
        rmg[0] = cadd(cscalar(r, psi_xp[0]), sl0);
        rmg[1] = cadd(cscalar(r, psi_xp[1]), sl1);
        rmg[2] = csub(cscalar(r, psi_xp[2]), su0);
        rmg[3] = csub(cscalar(r, psi_xp[3]), su1);

        // (r + gamma^mu) psi_xm:  upper = r u - sigma_mu * lower; lower = r l + sigma_mu * upper
        apply_pauli(mu, psi_xm[2], psi_xm[3], sl0, sl1);
        apply_pauli(mu, psi_xm[0], psi_xm[1], su0, su1);
        c2 rpg[4];
        rpg[0] = csub(cscalar(r, psi_xm[0]), sl0);
        rpg[1] = csub(cscalar(r, psi_xm[1]), sl1);
        rpg[2] = cadd(cscalar(r, psi_xm[2]), su0);
        rpg[3] = cadd(cscalar(r, psi_xm[3]), su1);

        for (int k = 0; k < 4; ++k) {
            c2 sum = cadd(rmg[k], rpg[k]);
            result[k] = cadd(result[k], cscalar(-off, sum));
        }
    }

    for (int k = 0; k < 4; ++k) out[idx * 4 + k] = result[k];
}

void check_cuda(cudaError_t err, const char* what) {
    if (err != cudaSuccess) {
        std::fprintf(stderr, "CUDA error in %s: %s\n", what, cudaGetErrorString(err));
        std::abort();
    }
}

}  // namespace

void apply_wilson_dirac_gpu(SpinorField& out,
                            const SpinorField& psi,
                            const GaugeLinks& links,
                            const Lattice& lattice,
                            const WilsonDiracParams& params) {
    const int L = lattice.size();
    const int N = L * L * L;

    // Each site holds 4 cdoubles. cuDoubleComplex is layout-compatible with
    // std::complex<double> for our purposes (re-im pair of doubles).
    const std::size_t spinor_bytes = static_cast<std::size_t>(N) * 4 * sizeof(c2);
    const std::size_t links_bytes = static_cast<std::size_t>(N) * 3 * sizeof(c2);

    c2* d_psi = nullptr;
    c2* d_out = nullptr;
    c2* d_U = nullptr;

    check_cuda(cudaMalloc(&d_psi, spinor_bytes), "cudaMalloc d_psi");
    check_cuda(cudaMalloc(&d_out, spinor_bytes), "cudaMalloc d_out");
    check_cuda(cudaMalloc(&d_U,   links_bytes), "cudaMalloc d_U");

    // Pack psi.
    static_assert(sizeof(Spinor) == 4 * sizeof(c2),
                  "Spinor must pack as 4 cuDoubleComplex (8 doubles)");
    check_cuda(cudaMemcpy(d_psi, psi.data.data(), spinor_bytes, cudaMemcpyHostToDevice),
               "cudaMemcpy psi -> device");

    // Pack U: layout [mu * N + site].
    {
        std::vector<c2> U_flat(static_cast<std::size_t>(N) * 3);
        for (int mu = 0; mu < 3; ++mu) {
            for (int i = 0; i < N; ++i) {
                const auto& u = links.U[mu][static_cast<std::size_t>(i)];
                U_flat[static_cast<std::size_t>(mu) * N + i] = make_cuDoubleComplex(u.real(), u.imag());
            }
        }
        check_cuda(cudaMemcpy(d_U, U_flat.data(), links_bytes, cudaMemcpyHostToDevice),
                   "cudaMemcpy U -> device");
    }

    const int block = 128;
    const int grid = (N + block - 1) / block;
    wilson_dirac_kernel<<<grid, block>>>(d_out, d_psi, d_U, L, params.m, params.r, params.a);
    check_cuda(cudaGetLastError(), "wilson_dirac_kernel launch");
    check_cuda(cudaDeviceSynchronize(), "wilson_dirac_kernel sync");

    check_cuda(cudaMemcpy(out.data.data(), d_out, spinor_bytes, cudaMemcpyDeviceToHost),
               "cudaMemcpy out -> host");

    cudaFree(d_psi);
    cudaFree(d_out);
    cudaFree(d_U);
}

}  // namespace wilson_dirac
}  // namespace ftd

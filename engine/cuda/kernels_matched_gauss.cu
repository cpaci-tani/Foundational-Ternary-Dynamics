/**
 * Native CUDA advance for FTD-0428 matched_gauss_dynamics.
 *
 * Isolated sector: Faraday, Ampere, conservative current subtract, then
 * centered E written into flux. Init CG stays on the host.
 */

#include "ftd/gpu_buffers.h"
#include "ftd/constants.h"
#include "../cuda/cuda_index.cuh"
#include "cuda_error.cuh"

#include <cuda_runtime.h>
#include <cmath>

namespace ftd {
namespace gpu {
namespace kernels {
namespace {

__device__ __forceinline__ int wrap_i(int v, int L) {
    return ::ftd::wrap(v, L);
}

__device__ __forceinline__ int idx(int x, int y, int z, int L) {
    return ::ftd::idx3d(x, y, z, L);
}

__global__ void matched_curl_adjoint_kernel(
    const double* __restrict__ ex,
    const double* __restrict__ ey,
    const double* __restrict__ ez,
    double* __restrict__ cx,
    double* __restrict__ cy,
    double* __restrict__ cz,
    int L) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    const int i = idx(x, y, z, L);
    const int xp = idx(x + 1, y, z, L);
    const int yp = idx(x, y + 1, z, L);
    const int zp = idx(x, y, z + 1, L);
    cx[i] = ez[yp] - ez[i] - ey[zp] + ey[i];
    cy[i] = ex[zp] - ex[i] - ez[xp] + ez[i];
    cz[i] = ey[xp] - ey[i] - ex[yp] + ex[i];
}

__global__ void matched_curl_kernel(
    const double* __restrict__ bx,
    const double* __restrict__ by,
    const double* __restrict__ bz,
    double* __restrict__ cx,
    double* __restrict__ cy,
    double* __restrict__ cz,
    int L) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    const int i = idx(x, y, z, L);
    const int xm = idx(x - 1, y, z, L);
    const int ym = idx(x, y - 1, z, L);
    const int zm = idx(x, y, z - 1, L);
    cx[i] = bz[i] - bz[ym] - by[i] + by[zm];
    cy[i] = bx[i] - bx[zm] - bz[i] + bz[xm];
    cz[i] = by[i] - by[xm] - bx[i] + bx[ym];
}

__global__ void matched_saxpy3_kernel(
    double* __restrict__ x,
    double* __restrict__ y,
    double* __restrict__ z,
    const double* __restrict__ ax,
    const double* __restrict__ ay,
    const double* __restrict__ az,
    double scale,
    int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    x[i] += scale * ax[i];
    y[i] += scale * ay[i];
    z[i] += scale * az[i];
}

__global__ void matched_current_and_valid_kernel(
    double* __restrict__ ex,
    double* __restrict__ ey,
    double* __restrict__ ez,
    const int* __restrict__ rho_before,
    const int8_t* __restrict__ state,
    const int* __restrict__ reaction,
    const double* __restrict__ jx,
    const double* __restrict__ jy,
    const double* __restrict__ jz,
    int* __restrict__ valid,
    double tolerance,
    int L,
    int N) {
    if (blockIdx.x != 0 || threadIdx.x != 0) return;
    int reaction_l1 = 0;
    double residual = 0.0;
    double current_l1 = 0.0;
    for (int i = 0; i < N; ++i) {
        const int r = reaction[i];
        reaction_l1 += r < 0 ? -r : r;
        current_l1 += fabs(jx[i]) + fabs(jy[i]) + fabs(jz[i]);
    }
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int i = idx(x, y, z, L);
                const double divj =
                    (jx[i] - jx[idx(x - 1, y, z, L)]) +
                    (jy[i] - jy[idx(x, y - 1, z, L)]) +
                    (jz[i] - jz[idx(x, y, z - 1, L)]);
                const double cont =
                    static_cast<double>(static_cast<int>(state[i]) - rho_before[i])
                    + divj - static_cast<double>(reaction[i]);
                residual = fmax(residual, fabs(cont));
            }
        }
    }
    if (reaction_l1 != 0 || residual > tolerance) {
        *valid = 0;
        return;
    }
    for (int i = 0; i < N; ++i) {
        ex[i] -= jx[i];
        ey[i] -= jy[i];
        ez[i] -= jz[i];
    }
    double gauss = 0.0;
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int i = idx(x, y, z, L);
                const double divE =
                    (ex[i] - ex[idx(x - 1, y, z, L)]) +
                    (ey[i] - ey[idx(x, y - 1, z, L)]) +
                    (ez[i] - ez[idx(x, y, z - 1, L)]);
                gauss = fmax(gauss, fabs(divE - static_cast<double>(state[i])));
            }
        }
    }
    *valid = (gauss <= 10.0 * tolerance) ? 1 : 0;
    (void)current_l1;
}

__global__ void matched_center_to_flux_kernel(
    const double* __restrict__ ex,
    const double* __restrict__ ey,
    const double* __restrict__ ez,
    double* __restrict__ fx,
    double* __restrict__ fy,
    double* __restrict__ fz,
    double* __restrict__ wvx,
    double* __restrict__ wvy,
    double* __restrict__ wvz,
    int L) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    const int z = blockIdx.z * blockDim.z + threadIdx.z;
    if (x >= L || y >= L || z >= L) return;
    const int i = idx(x, y, z, L);
    fx[i] = 0.5 * (ex[i] + ex[idx(x - 1, y, z, L)]);
    fy[i] = 0.5 * (ey[i] + ey[idx(x, y - 1, z, L)]);
    fz[i] = 0.5 * (ez[i] + ez[idx(x, y, z - 1, L)]);
    wvx[i] = 0.0;
    wvy[i] = 0.0;
    wvz[i] = 0.0;
}

}  // namespace

void launch_matched_gauss_advance(GpuBuffers& bufs, double wave_speed, double dt) {
    bufs.ensure_matched_gauss();
    const int L = bufs.L;
    const int N = bufs.N;
    const double scale = wave_speed * dt;
    const cudaStream_t stream = bufs.stream;
    dim3 block(4, 8, 8);
    dim3 grid((L + 3) / 4, (L + 7) / 8, (L + 7) / 8);
    const int t1 = 256;
    const int g1 = (N + t1 - 1) / t1;

    CUDA_CHECK(cudaMemsetAsync(bufs.d_matched_valid, 0, sizeof(int), stream));

    matched_curl_adjoint_kernel<<<grid, block, 0, stream>>>(
        bufs.d_matched_ex, bufs.d_matched_ey, bufs.d_matched_ez,
        bufs.d_matched_cx, bufs.d_matched_cy, bufs.d_matched_cz, L);
    CUDA_CHECK(cudaGetLastError());
    matched_saxpy3_kernel<<<g1, t1, 0, stream>>>(
        bufs.d_matched_bx, bufs.d_matched_by, bufs.d_matched_bz,
        bufs.d_matched_cx, bufs.d_matched_cy, bufs.d_matched_cz, -scale, N);
    CUDA_CHECK(cudaGetLastError());
    matched_curl_kernel<<<grid, block, 0, stream>>>(
        bufs.d_matched_bx, bufs.d_matched_by, bufs.d_matched_bz,
        bufs.d_matched_cx, bufs.d_matched_cy, bufs.d_matched_cz, L);
    CUDA_CHECK(cudaGetLastError());
    matched_saxpy3_kernel<<<g1, t1, 0, stream>>>(
        bufs.d_matched_ex, bufs.d_matched_ey, bufs.d_matched_ez,
        bufs.d_matched_cx, bufs.d_matched_cy, bufs.d_matched_cz, scale, N);
    CUDA_CHECK(cudaGetLastError());
    matched_current_and_valid_kernel<<<1, 1, 0, stream>>>(
        bufs.d_matched_ex, bufs.d_matched_ey, bufs.d_matched_ez,
        bufs.d_ledger_rho_before, bufs.d_state, bufs.d_ledger_reaction,
        bufs.d_ledger_current_x, bufs.d_ledger_current_y, bufs.d_ledger_current_z,
        bufs.d_matched_valid, 1e-12, L, N);
    CUDA_CHECK(cudaGetLastError());
    matched_center_to_flux_kernel<<<grid, block, 0, stream>>>(
        bufs.d_matched_ex, bufs.d_matched_ey, bufs.d_matched_ez,
        bufs.d_flux_x, bufs.d_flux_y, bufs.d_flux_z,
        bufs.d_wave_vel_x, bufs.d_wave_vel_y, bufs.d_wave_vel_z, L);
    CUDA_CHECK(cudaGetLastError());
}

}  // namespace kernels
}  // namespace gpu
}  // namespace ftd

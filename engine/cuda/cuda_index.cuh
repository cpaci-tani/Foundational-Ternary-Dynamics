#ifndef FTD_CUDA_INDEX_CUH
#define FTD_CUDA_INDEX_CUH

// Shared device-side index helpers for CUDA kernels.
//
// CRIT-3 (callstack audit): match the CPU X-major flat layout
//     i = ix*L*L + iy*L + iz
// across every kernel that touches the lattice. Previously each .cu file
// re-defined wrap_d / idx3d_d / decode_xyz_d / periodic_delta_d locally,
// which made geometry-affecting bug fixes (Z-major -> X-major) easy to miss
// in one TU. This header is the single source of truth.
//
// All helpers are __device__ __forceinline__ — zero-overhead inlining at
// kernel call sites; safe to include from any .cu translation unit.

#include <cstdint>

namespace ftd {

__device__ __forceinline__
int wrap(int x, int N) {
    return ((x % N) + N) % N;
}

__device__ __forceinline__
int idx3d(int x, int y, int z, int N) {
    // X-major flat layout: i = ix*N*N + iy*N + iz.
    return wrap(x, N) * N * N + wrap(y, N) * N + wrap(z, N);
}

__device__ __forceinline__
void decode_xyz(int idx, int N, int& x, int& y, int& z) {
    x = idx / (N * N);
    y = (idx / N) % N;
    z = idx % N;
}

// Shortest-path delta on a periodic N^3 lattice. Returns d in (-N/2, N/2].
__device__ __forceinline__
int periodic_delta(int a, int b, int N) {
    int d = a - b;
    if (d >  N/2) d -= N;
    if (d < -N/2) d += N;
    return d;
}

} // namespace ftd

#endif // FTD_CUDA_INDEX_CUH

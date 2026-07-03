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


#if defined(__CUDACC__)
// Byte-level atomicCAS (revision C3 consolidation — was duplicated verbatim
// in kernels_forces.cu and kernels_aux.cu as atomicCAS_byte_stencil; the
// old "keep local to avoid cross-TU device-symbol resolution" rationale is
// satisfied by header-inlining, same as wrap/idx3d above — ADR-0007).
// CUDA only supports 32-bit+ atomicCAS, so operate on the containing word.
__device__ __forceinline__
int8_t atomicCAS_byte(int8_t* addr, int8_t compare, int8_t val) {
    unsigned int* word_addr = reinterpret_cast<unsigned int*>(
        reinterpret_cast<size_t>(addr) & ~3ULL);
    unsigned int byte_offset = (reinterpret_cast<size_t>(addr) & 3) * 8;
    unsigned int byte_mask = 0xFFu << byte_offset;

    unsigned int old_word = *word_addr;
    unsigned int assumed;
    do {
        assumed = old_word;
        unsigned int old_byte = (assumed >> byte_offset) & 0xFF;
        if (old_byte != static_cast<unsigned char>(compare))
            return static_cast<int8_t>(old_byte);
        unsigned int new_word = (assumed & ~byte_mask)
                              | (static_cast<unsigned int>(static_cast<unsigned char>(val)) << byte_offset);
        old_word = atomicCAS(word_addr, assumed, new_word);
    } while (old_word != assumed);
    return compare;  // Success: old value was indeed `compare`
}
#endif  // __CUDACC__

} // namespace ftd

#endif // FTD_CUDA_INDEX_CUH

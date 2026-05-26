#pragma once
/**
 * @file ftd/eft/gpu_discrete_universe.h
 * @brief Standalone, hyper-optimized GPU Discrete Universe simulation engine prototype.
 */

#include <cstdint>
#include <cuda_fp16.h>

namespace ftd {
namespace eft {
namespace gpu {

/**
 * @brief Discrete Universe GPU buffer representation.
 *        Packs 5 ternary states (trits) into 1 byte (1.6 bits/cell).
 *        Uses FP16 for vector fields (flux & wave velocity) for maximum VRAM density.
 */
struct GpuDiscreteUniverse {
    int L = 0;                  // Side length (power of 2)
    int N = 0;                  // L^3
    int num_bytes_state = 0;    // Ceil(N / 5) bytes

    // Device memory pointers
    uint8_t* d_packed_states = nullptr;  // Packed 5-trits-per-byte array
    half* d_flux_x = nullptr;            // FP16 x-flux
    half* d_flux_y = nullptr;            // FP16 y-flux
    half* d_flux_z = nullptr;            // FP16 z-flux
    half* d_wave_vel_x = nullptr;        // FP16 x-wave-velocity
    half* d_wave_vel_y = nullptr;
    half* d_wave_vel_z = nullptr;

    // Red-Black SOR solver potentials
    half* d_phi = nullptr;               // Local scalar potential

    void allocate(int size);
    void free();
};

// C++ Driver Benchmark APIs
void run_discrete_universe_benchmark(int lattice_size, int ticks);

} // namespace gpu
} // namespace eft
} // namespace ftd

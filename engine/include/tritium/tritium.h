#pragma once
// tritium.h — Single-include header for the Tritium ternary compute library
//
// Tritium provides efficient balanced ternary {-1, 0, +1} computation:
//   - Hybrid storage: 2-bit compute format (32 trits/uint64) + 5-per-byte storage
//   - Parallel arithmetic and logic on packed trit words
//   - ReLU-like thresholding (hard + stochastic/FTD Born rule)
//   - TritVector/TritMatrix with popcount-based dot products
//   - 1D/2D/3D convolution with trit kernels
//   - SIMD acceleration (AVX2/SSE4.2 with scalar fallback)
//
// Usage:
//   #include <tritium/tritium.h>
//   using namespace tritium;

#include "trit.h"
#include "packing.h"
#include "arithmetic.h"
#include "logic.h"
#include "threshold.h"
#include "trit_vector.h"
#include "trit_matrix.h"
#include "convolution.h"
#include "simd.h"

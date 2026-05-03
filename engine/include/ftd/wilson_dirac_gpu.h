#pragma once
/**
 * Wilson-Dirac GPU host-side API (Phase II.2-E).
 *
 * One-shot GPU implementation of the spatial Wilson-Dirac operator. Allocates,
 * uploads, runs, downloads, frees on each call. Used for CPU/GPU parity testing.
 * Production runs that need persistent device buffers should be migrated to a
 * GpuBuffers-style RAII wrapper (left to a later refactor; not on the critical
 * path for II.2-E -- bit-exact-enough parity validates the kernel logic).
 *
 * Conventions: identical to engine/include/ftd/wilson_dirac.h.
 *   - 3D spatial Wilson-Dirac, dimension-consistent mass shift = m + 3r/a
 *   - Chiral (Weyl) basis gamma matrices
 *   - X-major linear index i = x*L*L + y*L + z (matches Lattice::index and cuda_index.cuh)
 */

#include "ftd/wilson_dirac.h"

namespace ftd {
namespace wilson_dirac {

// CPU-managed Spinor-field-shaped wrapper that copies psi -> device, runs the
// kernel, copies result -> out. Implementation in engine/cuda/wilson_dirac_gpu.cu.
void apply_wilson_dirac_gpu(SpinorField& out,
                            const SpinorField& psi,
                            const GaugeLinks& links,
                            const Lattice& lattice,
                            const WilsonDiracParams& params);

}  // namespace wilson_dirac
}  // namespace ftd

# engine/cuda — CUDA kernel library

**Purpose.** GPU-accelerated implementations of the FTD engine's hot phases.
Compiled into `ftd_cuda` static library; linked into `ftd_core` when
`FTD_ENABLE_CUDA` is on. CPU/GPU parity is enforced via shared constants
(`constants_gpu.cuh`) and tested via `test_gpu_parity_complete.cpp`.

## Public API

CUDA kernels are launched from `engine/cuda/gpu_engine.cu`. Consumers of
the engine (CPU or web) interact only via `RenderBridge::tick()`, which
dispatches to GPU automatically when CUDA is available and toggled on.

## Internal structure

### Kernel libraries (multi-kernel TUs)

| File | LOC | Kernels | Role |
|---|---:|---:|---|
| `kernels_stencil.cu` | 1530 | 14 | Wave equation (single + dual substrate), genesis, evaporation, transmutation, pair production. **Phase 5 will split into 3 TUs.** |
| `kernels_forces.cu` | 934 | 7 | EM + gravity + Lorentz + color/strong/exchange + particle movement |
| `kernels_poisson.cu` | 478 | 10 | FFT-based Poisson solvers (Coulomb, latency, Gauss); pack/unpack helpers |
| `gpu_buffers.cu` | 778 | 3 | SoA allocator + uploaders + downloaders; small special-case kernels (continuity ledger reset, Green precompute) |
| `gpu_engine.cu` | 684 | 0 | Tick orchestration; launches kernels; manages streams |
| `atom_engine_gpu.cu` | 361 | 1 | Scale-2 O(N²) pairwise ionic + vdW |
| `particle_engine_gpu.cu` | ~200 | 1 | Scale-1 Barnes-Hut tree force |

### Shared headers

| Header | Role |
|---|---|
| `cuda_index.cuh` | Shared `__device__ __forceinline__` helpers: `idx3d`, `wrap`, `decode_xyz`, `periodic_delta` |
| `../include/ftd/constants_gpu.cuh` | Device-side `__constant__` mirrors of host constants |
| `../include/ftd/gpu_buffers.h`, `gpu_engine.h` | Public C++ API consumed by `render_bridge.cpp` |

## Dependencies

- **CUDA Toolkit** (probed via `find_package(CUDAToolkit)`)
- **cuFFT** (Poisson solvers)
- **cuBLAS** (atom-engine pair forces)

## How to run

```bash
# Build (with CUDA auto-detected)
cmake -S engine -B engine/build -DFTD_ENABLE_CUDA=ON
cmake --build engine/build --target ftd_cuda

# Run a GPU benchmark (WSL2 only per CLAUDE.md)
wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
    engine/build_wsl/test_gpu_parity_complete"
```

**Important: GPU campaigns route through WSL2.** Windows-native CUDA
(`engine/build/`) is acceptable for compile-time checks and single-tick
correctness; multi-seed campaigns or measurement runs go through
`engine/build_wsl/`.

## CPU/GPU parity contract

For every host-side phase in `engine/src/render_bridge.cpp` there is a
device-side equivalent kernel. Parity is enforced by:

- Shared constants via `engine/include/ftd/constants_gpu.cuh` (mirrors
  `constants.h` to bit precision via static_asserts)
- Shared geometry helpers via `cuda_index.cuh` (no per-kernel local copies)
- `test_gpu_parity_complete.cpp` (and similar) running identical scenarios
  on both backends and asserting agreement

Known divergences are tracked in `AUDIT_LEDGER.md` Track B (GPU/CPU
parity) and tagged for WSL2 verification before merge.

## How to extend

### Adding a new kernel
1. Place in the appropriate kernel library (`kernels_stencil.cu` for wave/state,
   `kernels_forces.cu` for forces, etc.). Phase 5 may add `kernels_aux.cu`.
2. Use `__device__ __forceinline__` helpers from `cuda_index.cuh`; do NOT
   redefine `idx3d` etc. locally (drift risk).
3. If the kernel needs a host-side launcher, add it to `gpu_engine.cu`.
4. If it needs new constants, add to `constants_gpu.cuh` (and host-side
   `constants.h` for parity).
5. Write a CPU mirror in `engine/src/render_bridge.cpp` or an extracted
   phase TU.
6. Add a parity test that compares CPU vs GPU output on a deterministic
   seed.

### Adding a new helper header
1. Create `cuda/<name>.cuh` with `__device__ __forceinline__` helpers.
2. Document in this README under "Shared headers."
3. Update kernels to include the new header instead of local copies.

## Invariants

- All `__global__` kernels accept the same launch geometry as their CPU
  mirror (same loop bounds, same neighbor offsets)
- All shared constants flow through `constants_gpu.cuh`; literal numeric
  values in kernel bodies are a bug
- Index computations use `cuda_index.cuh` helpers; local copies are a bug

## Related docs

- [CONTRACTS.md §7](../../CONTRACTS.md#7--constants-chain-contract) (constants chain)
- [docs/adr/0007-cuda-helper-consolidation.md](../../docs/adr/0007-cuda-helper-consolidation.md)
- [docs/adr/0008-r1-r5-phase-extraction.md](../../docs/adr/0008-r1-r5-phase-extraction.md) (CPU-side analogue)
- [engine/SPEC_ENGINE.md](../SPEC_ENGINE.md)
- CPU mirror: `engine/src/render_bridge.cpp`
- Constants: `engine/include/ftd/constants_gpu.cuh`

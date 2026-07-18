# engine/cuda — CUDA kernel library

**Purpose.** GPU-accelerated implementations of the FTD engine's hot phases.
Compiled into `ftd_cuda` static library; linked into `ftd_core` when
`FTD_ENABLE_CUDA` is on. CPU/GPU parity is enforced via shared constants
(`constants_shared.h`) and tested via `test_gpu_parity_complete.cpp`.

## Public API

CUDA kernels are launched from `engine/cuda/gpu_engine.cu`. Consumers of
the engine (CPU or web) interact only via `RenderBridge::tick()`, which
dispatches to GPU automatically when CUDA is available and toggled on.

## Internal structure

### Kernel libraries (multi-kernel TUs)

Phase 5 split (commits 2db67ca…87158ae): `kernels_stencil.cu` (1530 LOC) was
deleted and replaced by 3 TUs along the single/dual substrate seam plus
auxiliary kernels.

| File | LOC | Role |
|---|---:|---|
| `kernels_stencil_single.cu` | 759 | Single-substrate kernels: phase_read, compute_near_particle, phase_write, wave_update, genesis, evaporation |
| `kernels_stencil_dual.cu` | 565 | Dual-substrate kernels: phase_read_dual, strong_field_stencil, weak_field_stencil, phase_write_dual, genesis_dual, gauss_sync_dual |
| `kernels_aux.cu` | 286 | Auxiliary kernels: weak_transmutation, pair_production |
| `kernels_forces.cu` | 934 | EM + gravity + Lorentz + color/strong/exchange + particle movement (unchanged by Phase 5) |
| `kernels_poisson.cu` | 478 | FFT-based Poisson solvers (Coulomb, latency, Gauss); pack/unpack helpers (unchanged) |
| `gpu_buffers.cu` | 778 | SoA allocator + uploaders + downloaders; small special-case kernels (unchanged) |
| `gpu_engine.cu` | 684 | Tick orchestration; launches kernels; manages streams (unchanged) |
| `atom_engine_gpu.cu` | 361 | Scale-2 O(N²) pairwise ionic + vdW (unchanged) |
| `particle_engine_gpu.cu` | ~200 | Scale-1 Barnes-Hut tree force (unchanged) |

### Shared headers

| Header | Role |
|---|---|
| `kernels_stencil_common.cuh` | 82 LOC — shared device helpers used by both single-/dual-substrate stencil TUs: `wrap`, `idx3d`, `effective_damping`, `scale_field_pair` |
| `cuda_index.cuh` | Shared `__device__ __forceinline__` helpers: `idx3d`, `wrap`, `decode_xyz`, `periodic_delta`, `atomicCAS_byte` (revision C3 — was duplicated in kernels_forces/kernels_aux) |
| `cuda_error.cuh` | Shared `CUDA_CHECK` / `CUFFT_CHECK` error macros (revision C1 — was duplicated verbatim across 11 TUs) |
| `../include/ftd/constants_shared.h` | `inline constexpr` mirrors of host constants — compiles under both g++ and nvcc; not `__constant__` memory (those live in `cuda_invariants.cu`, ADR-0014) |
| `../include/ftd/gpu_buffers.h`, `gpu_engine.h` | Public C++ API consumed by `render_bridge.cpp` |

## Dependencies

- **CUDA Toolkit** (probed via `find_package(CUDAToolkit)`)
- **cuFFT** (Poisson solvers)
- **cuBLAS** (atom-engine pair forces)

## How to run

```bash
# Build (CUDA ON by default; pins MSVC 14.44 -- VS 18's default 14.51
# toolset crashes CUDA 13.0's cudafe++)
engine\build_native.bat build --target ftd_cuda

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

- Shared constants via `engine/include/ftd/constants_shared.h` (mirrors
  `constants.h` to bit precision via static_asserts)
- Shared geometry helpers via `cuda_index.cuh` (no per-kernel local copies)
- `test_gpu_parity_complete.cpp` (and similar) running identical scenarios
  on both backends and asserting agreement

Known divergences are tracked in `engine/CHECKLIST_ENGINE.md` or the
relevant archived audit under `docs/audits/`, and tagged for WSL2
verification before merge.

**Phase 5 GPU runtime parity status:** the Phase 5 stencil split is
compile-verified and CPU-deterministic-verified only. Full L=64 GPU
runtime parity is **deferred to a WSL2 follow-up session** (the local
build environment has Windows-native CUDA only; campaign-grade GPU
verification routes through `engine/build_wsl/` per CLAUDE.md).

## How to extend

### Adding a new kernel
1. Place in the appropriate kernel library:
   - `kernels_stencil_single.cu` for single-substrate wave/state kernels
   - `kernels_stencil_dual.cu` for dual-substrate variants
   - `kernels_aux.cu` for transmutation / pair production / other auxiliary kernels
   - `kernels_forces.cu` for force kernels
   - `kernels_poisson.cu` for Poisson / FFT solvers
2. Use `__device__ __forceinline__` helpers from `cuda_index.cuh` and
   `kernels_stencil_common.cuh`; do NOT redefine `idx3d`, `wrap`,
   `effective_damping`, etc. locally (drift risk).
3. If the kernel needs a host-side launcher, add it to `gpu_engine.cu`.
4. If it needs new constants, add to `constants_shared.h` (and host-side
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
- All shared constants flow through `constants_shared.h`; literal numeric
  values in kernel bodies are a bug
- Index computations use `cuda_index.cuh` helpers; local copies are a bug

## Related docs

- [CONTRACTS.md §7](../../CONTRACTS.md#7--constants-chain-contract) (constants chain)
- [docs/adr/0007-cuda-helper-consolidation.md](../../docs/adr/0007-cuda-helper-consolidation.md)
- [docs/adr/0008-r1-r5-phase-extraction.md](../../docs/adr/0008-r1-r5-phase-extraction.md) (CPU-side analogue)
- [docs/adr/0012-golden-tick-regression-gate.md](../../docs/adr/0012-golden-tick-regression-gate.md) — bit-exact regression gate (hash `0xb604d81a3d79366e` @ L=17) covering physics-touching kernel changes
- [engine/SPEC_ENGINE.md](../SPEC_ENGINE.md)
- CPU mirror: `engine/src/render_bridge.cpp` (545 LOC) + `engine/src/render_bridge_phases/{phase_read,phase_write,phase_forces,phase_movement}.cpp`
- Constants: `engine/include/ftd/constants_shared.h`

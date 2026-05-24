# 0014 — CUDA Constant Memory for Invariants

**Status:** Accepted
**Date:** 2026-05-23
**Author:** session 2026-05-23 (Plan: `use-cuda-constant-memory-adaptive-church`)

## Context

Small read-only matrices and companion scalars that every thread reads
identically are a canonical CUDA constant-memory use case: broadcast-cached,
persistent for the device context, settable from host via `cudaMemcpyToSymbol`.
Before this ADR, the engine had **zero** `__constant__` symbols — every
physics constant lived as `inline constexpr double` in `ftd::ontic::*` headers
and was baked into kernels at compile time. That works for stencil weights
participating in many fused multiplies; it does not work for invariant
*matrices* indexed at runtime.

## Decision

Adopt the pattern in `engine/cuda/cuda_invariants.{cuh,cu}` for any new
device-visible invariant:

1. **Definition** in a dedicated `.cu` TU under `engine/cuda/`:
   `__constant__ double c_NAME[K];` (K ≤ 64).
2. **Declaration** in a sibling `.cuh`: `extern __constant__ double c_NAME[K];`.
3. **Upload helper** alongside the symbol, using `cudaMemcpyToSymbol`. Default
   overload pulls from `ftd::ontic::*` so the single-source-of-truth is preserved.
4. Symbol prefix `c_` to distinguish device storage from host constexpr.
5. **Separable compilation REQUIRED on consumer targets** — `CUDA_SEPARABLE_COMPILATION ON`
   plus `CUDA_RESOLVE_DEVICE_SYMBOLS ON` — or nvcc treats the `extern` declaration
   as a static definition and the consumer TU silently gets its own copy.

Reference implementation populates a 3×3 matrix touching `G_STAR`, `VARPI`,
`1/G_STAR`. Validated by `tests/benchmark_invariant_matrix_constant_memory.cu`
against a CPU reference within 1e-14 relative error.

## Consequences

New invariants declared once, uploaded once, broadcast-cached at kernel
runtime. Constant memory is 64 KiB per context; current usage < 100 bytes.
Pattern is dormant: `test_render_bridge_golden` hash `0xcd957b601d47868a`
unchanged; `test_gpu_parity_complete` 70/0 across 20 physics domains.
Footgun for future work — consumer TUs without `CUDA_SEPARABLE_COMPILATION`
silently get a zero copy of the symbol.

## Alternatives considered

- **Keep `inline constexpr`.** Works for scalars; fails for matrices indexed
  at runtime — the compiler cannot place an array index into immediates.
- **Pass the matrix as a kernel argument.** Wastes one constant-memory cache
  hit per call and bloats the kernel signature; loses the broadcast cache.
- **Stash the matrix in global memory.** Costs a cache-miss path on the first
  warp; the matrix is never written, so this is strictly worse than constant.
- **Texture/surface memory.** Designed for spatially-coherent reads, not
  uniform broadcast; overkill for 9-element matrices.

## References

- Files: `engine/cuda/cuda_invariants.{cuh,cu}`,
  `engine/tests/benchmark_invariant_matrix_constant_memory.cu`,
  `engine/cuda/CMakeLists.txt`, `engine/CMakeLists.txt`.
- Cross-refs: ADR-0007 (CUDA helper consolidation — sibling `.cuh` pattern),
  ADR-0012 (golden-tick regression gate — pattern is dormant w.r.t. this hash).
- Plan: `~/.claude/plans/use-cuda-constant-memory-adaptive-church.md`.
- Math note: `ϖ = √2·K(1/√2)`, NOT `2·K(1/√2)`; see FTD-0117 for the G\*/ϖ
  confusion history. Codebase already uses the correct form
  (`scripts/constants.py:108`).

# 0007 — CUDA helper consolidation (single shared header pattern)

**Status:** Accepted
**Date:** 2026-04 (retroactive, formalized 2026-04-27)
**Author:** codified during Wave 2 audit sweep

## Context

`kernels_stencil.cu` and `kernels_forces.cu` had local copies of `idx3d`,
`wrap`, `decode_xyz`, `periodic_delta` device-side helpers. The two copies
had drifted at least once (the X-major/Z-major fix at `kernels_stencil.cu:40`
was applied to one TU and forgotten in the other). Such silent drift
produces visually-correct-looking but physically-wrong simulations.

## Decision

Consolidate shared device helpers into single headers under `engine/cuda/`,
included by every kernel TU that needs them. First instance:
`engine/cuda/cuda_index.cuh` (`__device__ __forceinline__ idx3d`, `wrap`,
`decode_xyz`, `periodic_delta`).

Future consolidation candidates (Phase 5):
- `engine/cuda/lap18.cuh` — 18-point isotropic Laplacian (shared by
  single + dual substrate stencil kernels)
- `engine/cuda/field_gradients.cuh` — tier-1 / tier-2 gradient helpers
  (shared by force kernels)

## Consequences

- (+) One source of truth per helper; drift bug class eliminated
- (+) Kernels become thinner and more readable
- (+) Compile-time inlining preserves performance
- (−) Need to know which header to include; mitigated by `engine/cuda/README.md`

## Alternatives considered

- Per-kernel local copies — rejected: actual drift bug confirmed (CRIT-3
  fix at `kernels_stencil.cu:40`).
- Macro-based helpers — rejected: harder to debug; loses type checking.

## References

- Files: `engine/cuda/cuda_index.cuh`, `engine/cuda/kernels_*.cu`
- Cross-refs: ADR-0008 (R1-R5 phase extraction; CPU-side analogue)

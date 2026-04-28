# 0012 — Golden-tick regression gate (Phase 4 physics-touching extractions)

**Status:** Accepted
**Date:** 2026-04-27
**Author:** refactor sweep Phase 4 pre-flight (commit 8afc8be)

## Context

Phase 4 of the refactor sweep decomposed the four fat phase methods of
`RenderBridge` (`phase_write` 286 LOC, `phase_forces` 239 LOC, `phase_read`
142 LOC, `phase_movement` 114 LOC) into focused free-function TUs under
`engine/src/render_bridge_phases/`. This is the highest-risk JS-or-C++
extraction in the sweep — these methods carry the load-bearing physics
of the engine. A subtle reordering or off-by-one would silently change
energy conservation, force directions, particle motion, etc. Test suites
that look for "obvious" failures wouldn't catch a 10⁻⁹ drift accumulating
over 100 ticks into a visible bug elsewhere.

## Decision

Before any extraction begins, land a **bit-exact byte-hash regression
test** that fingerprints 100 ticks of a deterministic scenario. Every
subsequent commit must reproduce the hash exactly.

`engine/tests/test_render_bridge_golden.cpp`:
- `RenderBridge(L=16)`, force CPU, seed RNG with 42
- Fixed toggle profile (Logic6-like) honoring `validate()` deps
- Inject 3 manifested particles + 1 flux pulse at known coords
- Run exactly 100 ticks
- 64-bit FNV-1a hash over: every voxel's state + flux + wave_vel +
  velocity, every `EnergyAudit` field, manifested-site state
- Assert `hash == 0xcd957b601d47868aULL`

CTest label `golden`; ~0.20s wall.

## Consequences

- (+) Phases 4a/4b/4c each extracted hundreds of LOC of physics code
  with bit-exact preservation, verified at commit time
- (+) Phases 5/6/7 also held the gate (CUDA split, toggle table refactor,
  test infra extraction)
- (+) Regression target for any future physics-adjacent change
- (−) Adding new physics requires a deliberate gate-rebaseline commit
  (capture new hash → freeze) before extraction work; this is by design

## Alternatives considered

- Hand-rolled per-quantity assertions — rejected: hashes catch
  permutation bugs and byte-level corruption that field-level checks miss
- GPU determinism — rejected: cuRAND non-determinism and floating-point
  reduction order leak in. CPU forcing with deterministic SplitMix64 RNG
  is the only stable substrate for a strict gate.
- Larger lattice (L=64) — rejected: 0.20s wall at L=16 keeps the gate
  cheap enough to run on every commit

## References

- Files: `engine/tests/test_render_bridge_golden.cpp`,
  `engine/CMakeLists.txt` (`ftd_add_test render_bridge_golden LABELS unit;golden`)
- Cross-refs: CONTRACTS.md §12, ADR-0008 (R1-R5 phase extraction precedent)

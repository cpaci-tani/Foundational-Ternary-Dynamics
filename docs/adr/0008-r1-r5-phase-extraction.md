# 0008 — R1-R5 phase extraction (render_bridge.cpp decomposition)

**Status:** Accepted (Phase 4 of refactor sweep will continue this)
**Date:** 2026-04 (retroactive)
**Author:** codified 2026-04-27

## Context

`engine/src/render_bridge.cpp` accumulated to 1231 LOC with fat phase
methods: phase_write (286 LOC), phase_forces (239 LOC), phase_read (142 LOC),
plus orchestration. Each phase mixed multiple concerns (e.g., phase_write
combined damping selection, selective-damping mask build, Larmor radiation,
Langevin OU thermostat, dual-substrate genesis, single-substrate genesis,
particle-id post-pass). Reasoning about a single-substrate fix required
scrolling past the dual-substrate path; physics audits became
disproportionately expensive.

## Decision

Incrementally extract phases into focused `engine/src/<phase>.cpp` TUs:

- **R1** `poisson_solvers.cpp` — SOR Poisson chain (Coulomb, latency, gauss)
- **R2** `transmutation_phases.cpp` — weak / pair / triad / proper-time
- **R3** `energy_ledger_compute.cpp` — per-tick conservation bookkeeping
- **R4** `diagnostics_compute.cpp` — `diagnostics()`, `energy_audit()`,
  EM fields, Poynting, entropy
- **R5** `injection.cpp` — `inject_flux*`, `inject_particle`, `inject_wavepacket`,
  `aggregate_profile`

Each extracted TU exposes free functions; `RenderBridge` methods become
thin delegators (`return ::ftd::xxx_cpu(*this);`). Phase 4 of the current
refactor sweep will continue this pattern with phase_write, phase_forces,
phase_read, phase_movement decomposition.

## Consequences

- (+) Each phase concern is independently editable and testable
- (+) Build incremental on phase changes touches only the relevant TU
- (+) Bug fixes localize to one TU
- (−) Some indirection (free function called via thin method)

## Alternatives considered

- Keep monolithic — rejected: scaling problem confirmed.
- Class-per-phase — rejected: state ownership stays on RenderBridge,
  classes would just duplicate plumbing.

## References

- Files: `engine/src/{poisson_solvers,transmutation_phases,energy_ledger_compute,diagnostics_compute,injection}.cpp`
- Cross-refs: ADR-0007 (CUDA-side analogue), Phase 4 plan

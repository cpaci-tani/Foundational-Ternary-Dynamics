# 0016 — Experimental-module quarantine

**Status:** Accepted

## Context

Three experimental modules are kept for provenance/future work but sit
outside the production tick path: DagEngine + SparseVoxelDAG (sparse octree
prototype, deprecated ticket W6; WASM binding removed in the 2026
consolidation sweep), the `ftd_cognition` sidecar, and the CUDA
discrete-universe prototype (standalone binary, no test consumers). Their
build membership was implicit — nothing marked the boundary, and audits kept
re-discovering them as "dead code?" (engine revision program 0.11/3.7).

## Decision

CMake option `FTD_BUILD_EXPERIMENTAL` (default **ON** — zero change to any
existing build) gates: `src/dag_engine.cpp` in `ftd_core`, the
`ftd_cognition` library, their tests/benchmarks, and
`experimental_discrete_universe`. The `source_lint` CTest additionally fails
if any production TU includes a quarantined header
(`dag_engine.h`/`dag_lattice.h`/`cognition/…`). Code is KEPT, never deleted
(project quarantine convention).

## Consequences

- (+) The experimental boundary is explicit, CI-visible, and lint-enforced
- (+) `-DFTD_BUILD_EXPERIMENTAL=OFF` gives a lean core for deployments
- (−) Two extra CMake conditionals to maintain
- Revisit: 2026-Q4 — DAG unblock criterion is sparse L>64 void-field work

## Alternatives considered

- Deleting the modules — rejected: provenance convention; DagEngine has a
  documented future purpose and a structure-validation test.
- Archiving to `archive/` — rejected: the code still compiles against live
  headers; archived copies rot silently.

## References

- Files: `engine/CMakeLists.txt` (option + gated blocks),
  `engine/cmake/FtdSourceLint.cmake` (boundary lint),
  `engine/src/dag_engine.cpp` (status banner), `engine/src/cognition/`
- Cross-refs: ADR-0012 (goldens unaffected — the modules are off the tick
  path), engine/CHECKLIST_ENGINE.md revision-0.11 evidence table

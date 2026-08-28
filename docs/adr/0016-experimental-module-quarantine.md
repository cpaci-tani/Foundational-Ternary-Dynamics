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

CMake option `FTD_BUILD_EXPERIMENTAL` (default **OFF** — explicit opt-in)
gates: `src/dag_engine.cpp` in `ftd_core`, the
`ftd_cognition` library, their tests/benchmarks, and
`experimental_discrete_universe`. The `source_lint` CTest additionally fails
if any production TU includes a quarantined header
(`dag_engine.h`/`dag_lattice.h`/`cognition/…`). Code is KEPT, never deleted
(project quarantine convention).

## Consequences

- (+) The experimental boundary is explicit, CI-visible, and lint-enforced
- (+) Normal builds and deployments contain only maintained production engines
- (+) `-DFTD_BUILD_EXPERIMENTAL=ON` keeps the quarantined modules buildable and tested on demand
- (−) Two extra CMake conditionals to maintain
- Revisit: 2026-Q4 — DAG unblock criterion is sparse L>64 void-field work

**Default changed 2026-08-27.** The original default-ON decision minimized
behavior change while the quarantine boundary was introduced. Once the engine
inventory proved these targets had no production consumers, leaving them in
every normal build made the boundary nominal rather than operational. Their
sources and opt-in tests remain intact; only default build membership changed.

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

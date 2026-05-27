# Scope: Engine Architecture Mapping and Documentation

## Architecture
- Target directory: `c:\Users\cpaci\Desktop\ftd\engine\`
- Major directories in engine: `include/ftd/`, `src/`, `cuda/`, `wasm/`, `tests/`
- Target outputs:
  - Architecture map (R1)
  - Dependency graph (R2)
  - Gap analysis & structural documentation (R3)

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Exploration & Inventory | Complete mapping of `engine/` subdirectories and boundaries, identifying the role of every header and source file. | None | DONE |
| M2 | Dependency & Flow Analysis | Granular dependency analysis of compile-time header inclusions, runtime execution pipelines, and CPU/GPU data transfer boundaries. | M1 | DONE |
| M3 | Structural Documentation & Gap Analysis | Synthesizing architectural documentation and stubs/gap catalog into a final markdown report. | M2 | DONE |

## Interface Contracts
- **Input Context**: Verbatim `engine/` source code and headers.
- **Output Artifacts**: Comprehensive documentation written to `docs/theory/01_reference/MAP_ENGINE_ARCHITECTURE.md` (or another path chosen for structural maps). Let's write the report to a clear path like `docs/theory/01_reference/MAP_ENGINE_ARCHITECTURE.md`.

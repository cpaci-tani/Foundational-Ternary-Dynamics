# Project: FTD Web Dashboard Refactoring

## Architecture
- The FTD web dashboard is a Three.js-based simulation dashboard spanning 8 scales (lattice, particles, atoms, molecules, planetary, cosmic, meta, reference frame context).
- Composition root: `app_dag.js`.
- Core renderers: `field-renderer.js`, `flux-renderer.js`, `topology-sheet-renderer.js`, `molecular-renderer.js`, `particle-renderer.js`, `planetary-renderer.js`, `cosmic-renderer.js`.

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| 1 | Exploratory Sweep | Sweep `engine/web/js/` to find modular duplication, components without proper lifecycle methods (`mount`, `update`, `destroy`), and WebGL resources without explicit `.dispose()` calls. | None | DONE |
| 2 | Unified Lifecycle Design | Design a clean, unified lifecycle interface (`mount`, `update`, `destroy` / `dispose`) with a BaseLifecycleController class. | M1 | DONE |
| 3 | Module Refactoring | Refactor UI modules, view controllers, and renderers (focusing on `field-renderer.js`, `flux-renderer.js`, `topology-sheet-renderer.js`, and scale controllers) to adhere to the component lifecycle and dispose of all Three.js geometries, materials, textures, render targets, and global event listeners. | M2 | DONE |
| 4 | Playwright Testing | Run Playwright test suite to ensure no regressions and 146/146 test completion. Verify leak prevention. | M3 | POSTPONED (by User; deferred to victory audit) |
| 5 | Forensic Audit & Handoff | Conduct integrity audit and deliver handoff report (`handoff.md`) to the Sentinel. | M4 | BLOCKED (Integrating Entrypoint Fix) |

## Interface Contracts
- Components/Renderers must implement unified lifecycle methods: `mount()`, `update()`, and `destroy()`/`dispose()`.
- Upon `destroy()` / `dispose()`, all Three.js resources (geometries, materials, textures, render targets) must be explicitly disposed of, and all global event listeners and timers must be cleared/removed.

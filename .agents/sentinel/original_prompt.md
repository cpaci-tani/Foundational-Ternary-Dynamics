## 2026-05-27T04:03:57Z

Refactor the FTD web dashboard codebase to ensure exceptional modularity, DRY compliance, clear lifecycle management, and zero memory/computation leaks (both in JS heap and WebGL context).

Working directory: c:\Users\cpaci\Desktop\ftd\engine\web

## Requirements

### R1. Modular & DRY Dashboard Sweep
Conduct a thorough sweep of the frontend modules (under `engine/web/js/`) to consolidate duplicate utility routines, eliminate visual-rendering redundancy, and decouple DOM operations from business logic.

### R2. Strict CRUD Lifecycle & Component Lifecycle Management
Organize all UI modules, views, and controllers to follow a explicit component lifecycle contract (e.g., standard `mount()`, `update()`, and `destroy()` / `unmount()` routines), ensuring clean initialization, telemetry binding, and termination.

### R3. WebGL Resource & JS Memory Leak Mitigation
Audit the Three.js viewport renderers (including `field-renderer.js`, `flux-renderer.js`, and `topology-sheet-renderer.js`) to guarantee that all WebGL geometries, materials, textures, and render targets are explicitly `.dispose()`'d upon container unmount or lattice resize events, and all global event listeners are detached.

### R4. Automated Regression Testing
All changes must be validated against the comprehensive Playwright test suite in `engine/web/tests/` to guarantee that all 146 tests (including scale switching, timeline buffers, panel mounts, and performance baselines) pass with 100% correctness.

## Acceptance Criteria

### Modularity and Lifecycle Quality
- [ ] UI components cleanly implement and call `mount()`, `update()`, and `destroy()` lifecycle hooks.
- [ ] No hardcoded global variable leaks or redundant cross-component direct mutations exist.

### Memory Integrity & Leak Audit
- [ ] Visual inspection or automated checks confirm that Three.js memory allocations (`renderer.info.memory.geometries`, `textures`) do not grow unboundedly during repeated scale-switching or lattice-resizing actions.
- [ ] Global event listeners (e.g., keyboard shortcuts, resize hooks) are cleanly detached during scale switches or panel collapses.

### Zero Regression
- [ ] The complete Playwright test suite (`npx playwright test`) passes with 100% success inside `engine/web/tests/`.
- [ ] Zero console errors are thrown during flagship-scenario or Scale 5 cosmic simulation runs.

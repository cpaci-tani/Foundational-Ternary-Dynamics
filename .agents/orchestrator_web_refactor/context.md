# Context — FTD Web Dashboard Refactoring

## Active Work & Observations
- The codebase is located at `engine/web/`.
- Entry point is `engine/web/js/app_dag.js`.
- Renderers of interest:
  - `field-renderer.js`
  - `flux-renderer.js`
  - `topology-sheet-renderer.js`
  - `molecular-renderer.js`
  - `particle-renderer.js`
- Test suite is located under `engine/web/tests/`. Run via `npm test` or `npx playwright test`.

## Technical References
- Three.js resource disposal guide:
  - Geometries: `geometry.dispose()`
  - Materials: `material.dispose()` (remember that textures mapped to materials must be disposed of separately)
  - Textures: `texture.dispose()`
  - Render Targets: `renderTarget.dispose()`
  - Group/Mesh children: Must recursively traverse and dispose of their geometries and materials.
- DOM Event Listeners: Must be tracked and removed on `destroy()`.
- Timers / Intervals: Must be tracked and cleared on `destroy()`.

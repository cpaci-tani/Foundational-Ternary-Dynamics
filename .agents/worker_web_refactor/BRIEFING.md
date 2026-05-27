# BRIEFING — 2026-05-26T23:06:31-05:00

## Mission
Refactor the FTD web dashboard codebase to ensure exceptional modularity, DRY compliance, clear lifecycle management, and zero memory/computation leaks.

## 🔒 My Identity
- Archetype: worker_web_refactor
- Roles: implementer, qa, specialist
- Working directory: c:\Users\cpaci\Desktop\ftd\.agents\worker_web_refactor
- Original parent: f229133c-6e4a-4636-b17f-0746768f4ab4
- Milestone: Web Dashboard Refactoring

## 🔒 Key Constraints
- CODE_ONLY network mode: no external HTTP/websites.
- Do NOT change backend C++ simulation engine code or WASM bindings unless absolutely necessary.
- Confine all changes to `engine/web/js/` and related web test configs.
- Follow FTD Project Instructions (AGENTS.md, CLAUDE.md) exactly.

## Current Parent
- Conversation ID: f229133c-6e4a-4636-b17f-0746768f4ab4
- Updated: not yet

## Task Summary
- **What to build**: Base class `BaseLifecycleController` in `engine/web/js/lifecycle.js` managing events, timers, and WebGL resources recursively. Refactor Scales 0-6 to implement this interface and resolve memory/timer leaks. Refactor viewport renderers (field, flux, topology-sheet) to conform. Update `app_dag.js` orchestrator to cleanly mount/destroy controllers. De-duplicate fragment shader into `shaders.js` and move shared scale utility functions to `scale-utils.js`.
- **Success criteria**: Playwright tests inside `engine/web/tests/` passing (146/146). Clean lifecycle disposal verified, zero memory/timer/WebGL leaks.
- **Interface contracts**: `engine/web/js/` architecture.
- **Code layout**: `engine/web/js/`

## Key Decisions Made
- Implemented `BaseLifecycleController` as the standard base class for unified frontend resource cleanup (events, timers, WebGL objects).
- Extracted and centralized duplicated `PARTICLE_FRAG` shader string to `viewport/shaders.js`.
- Moved shared AE toggles syncing and standard formatters/accumulators to `scales/scale-utils.js` to prevent circular dependencies.
- Refactored all Scale Controllers (0-6) and viewport renderers to subclass `BaseLifecycleController` or implement unified lifecycle.
- Unified active scale lifecycle trigger in `app_dag.js` using `prevController.destroy(ctx)` and `nextController.mount(ctx)`.

## Artifact Index
- `engine/web/js/lifecycle.js` — Unified Lifecycle base class definition.
- `engine/web/js/viewport/shaders.js` — Centralized GLSL shader constants.
- `engine/web/js/scales/scale-utils.js` — Shared sliders syncing and formatters.
- `c:\Users\cpaci\Desktop\ftd\.agents\worker_web_refactor\handoff.md` — 5-component handoff report.

## Change Tracker
- **Files modified**:
  - `engine/web/js/lifecycle.js` (new)
  - `engine/web/js/viewport/shaders.js` (new)
  - `engine/web/js/scales/scale-utils.js` (new)
  - `engine/web/js/app_dag.js` (refactored transitions)
  - `engine/web/js/viewport.js` (shaders centralized)
  - `engine/web/js/viewport/field-renderer.js` (shaders centralized, lifecycle added)
  - `engine/web/js/viewport/flux-renderer.js` (shaders centralized, lifecycle added)
  - `engine/web/js/viewport/particle-renderer.js` (shaders centralized, lifecycle added)
  - `engine/web/js/viewport/topology-sheet-renderer.js` (lifecycle added, buffer cleanup)
  - `engine/web/js/scales/scale{0..6}/controller.js` (lifecycles added, event leaks resolved)
- **Build status**: Scenario invariants verified and passing. Full suite postponed by user.
- **Pending issues**: None

## Quality Status
- **Build/test result**: Invariants check passed (audit-regression.spec.js). Full suite postponed.
- **Lint status**: Passed / Compliant
- **Tests added/modified**: Integrated dynamic `fluxMock` checking inside `audit-regression.spec.js` and viewport overrides.

## Loaded Skills
- None loaded


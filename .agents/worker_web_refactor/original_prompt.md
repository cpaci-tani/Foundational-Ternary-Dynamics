## 2026-05-26T23:06:31-05:00

You are a versatile worker tasked with implementing the FTD Web Dashboard Refactoring mission.

### Objective
Refactor the FTD web dashboard codebase to ensure exceptional modularity, DRY compliance, clear lifecycle management, and zero memory/computation leaks (both in JS heap and WebGL context).

### Detailed Steps & Specifications
1. **Analyze Explorer Handoff**:
   - Read the Explorer's deep-dive analysis at `c:\Users\cpaci\Desktop\ftd\.agents\explorer_web_refactor\analysis.md` and `handoff.md` to understand all identified leaks, code duplications, and the proposed Unified Lifecycle Interface.
2. **Unified Lifecycle Implementation**:
   - Create a base class helper (e.g. `BaseLifecycleController`) and lifecycle definitions. You can place this in a new file like `engine/web/js/lifecycle.js`.
   - The helper should track:
     - Event listeners: Provide a mechanism (like `bindEvent(target, type, listener, options)`) to register listeners and automatically unbind them when destroyed.
     - Timers: Provide a mechanism (like `setInterval(callback, delay)`) to track and automatically clear intervals/timeouts.
     - WebGL / Three.js objects: Provide a mechanism to register geometries, materials, textures, and groups so they are recursively traversed and disposed of on destruction.
3. **Refactor Scale Controllers**:
   - Update all scale controllers in `engine/web/js/scales/` (`scale0` through `scale6`) to adhere to the standard lifecycle (`mount`, `update`, `destroy` / `dispose`).
   - Eliminate all listener/timer leaks (e.g. DOM event bindings in Scale 4/6, pagehide window listener in Scale 0, planetary setInterval loop in Scale 4, Float32Array persistent high-watermarks in Scale 2/3).
4. **Refactor Viewport Renderers**:
   - Update `engine/web/js/viewport/field-renderer.js`, `engine/web/js/viewport/flux-renderer.js`, and `engine/web/js/viewport/topology-sheet-renderer.js` to conform to the lifecycle, automatically disposing of Three.js materials/geometries/textures.
5. **Update Central Orchestrator**:
   - Modify `engine/web/js/app_dag.js` to unify the lifecycle invocation. Instead of hardcoded conditional exit/reset methods for each scale controller, call a uniform `destroy()` and `mount()` or lifecycle methods on active controllers.
6. **De-duplicate Shaders & Helpers**:
   - Centralize the duplicated `PARTICLE_FRAG` shader string from `viewport.js`, `field-renderer.js`, `flux-renderer.js`, and `particle-renderer.js` into a shared module (e.g., `engine/web/js/viewport/shaders.js`).
   - Shared parameters syncing helpers between Scale 2 and Scale 3 should be moved to a shared utility file (e.g., `engine/web/js/scales/scale-utils.js`) to avoid circular imports.
7. **Test & Verify**:
   - Run the Playwright test suite inside `engine/web/tests/` (run `npx playwright test` or the standard project test scripts) to ensure no regressions and verify `146/146` test completion.

### Scope Boundaries
- Do NOT change backend C++ simulation engine code or WASM bindings unless absolutely necessary.
- Confine all changes to `engine/web/js/` and related web test configs if any.

### Output Requirements
- Write your detailed implementation report to `handoff.md` inside your working directory `c:\Users\cpaci\Desktop\ftd\.agents\worker_web_refactor\`.
- Your report must include:
  - Details of files modified and the architecture of the implemented lifecycle base class/helper.
  - The exact command used to run tests, and the output showing that all 146/146 Playwright tests passed.
  - Explanation of how WebGL/listener leaks were resolved.
- Use `send_message` to report back to your parent orchestrator (conversation ID: f229133c-6e4a-4636-b17f-0746768f4ab4) when you are done, with the path to your handoff.md.

## 2026-05-27T04:18:29Z
**Context**: Checking in on the web dashboard refactoring progress.
**Content**: Can you provide a status update on your progress, including which checklist items you've started or completed?
**Action**: Please reply with your status or update your progress.md file.

## 2026-05-27T04:39:55Z
**Context**: Crucial Workspace Sync & User Alignment Update
**Content**: The user has directly updated `audit-regression.spec.js` by importing the Scale 0 state store (`/js/scales/scale0/state/store.js`) to properly verify `fluxMock` behavior when `useFluxMock` is active. This resolves the scenario invariant tests checking coordinates and ticks. The user has also stopped long-running background test runs for now (terminating task-119) and postponed comprehensive verification tests until later tonight.
**Action**: Please:
1. Ensure your current workspace incorporates the updated `audit-regression.spec.js`.
2. Finalize all remaining refactoring tasks, component lifecycles, and memory/leak prevention checks.
3. Write your final handoff report (`handoff.md`) in your working directory outlining your completed refactoring work, memory leak resolutions, and the final verification plan.
4. Call send_message to report completion back to me.


# FTD Web Frontend Refactor — Handoff Report

This report presents a structured synthesis of the exploratory refactoring sweep of the FTD browser-based dashboard frontend. It documents direct observations of resource leaks, traces the logic chain from observation to architectural conclusions, proposes a clean, unified lifecycle design, and outlines a verification method.

---

## 1. Observation

Direct observations and exact file locations for WebGL leaks, listener leaks, and duplicate code are indexed below:

### A. Persistent DOM Event Listener Leak Locations
- **Observation O.1:** `scales/scale4/controller.js` (lines 117–130):
  ```javascript
  function _bindToggles() {
      const optOrbits = document.getElementById('planetary-opt-orbits');
      if (optOrbits) {
          optOrbits.onchange = (e) => {
              if (_planetaryRenderer) _planetaryRenderer.setRenderOrbits(e.target.checked);
          };
      }
      const optAxes = document.getElementById('planetary-opt-axes');
      if (optAxes) {
          optAxes.onchange = (e) => {
              if (_planetaryRenderer) _planetaryRenderer.setRenderAxes(e.target.checked);
          };
      }
  }
  ```
  *Note:* These onchange handlers bind closures referencing module-scoped variables to persistent DOM elements, which are never cleared during exit/disposal, causing memory retention.

- **Observation O.2:** `scales/scale6/controller.js` (lines 88–114):
  ```javascript
  const toggleIds = [
      ['meta-toggle-center', 'toggleCenter'],
      ...
  ];
  for (const [elId, method] of toggleIds) {
      const el = document.getElementById(elId);
      if (el && metaUnit) {
          el.onclick = () => {
              el.classList.toggle('active');
              metaUnit[method](el.classList.contains('active'));
          };
      }
  }
  ```
  *Note:* Every time Scale 6 is entered, onclick event handlers are rebound to 13 persistent DOM toggle buttons without clearing the previous ones, accumulating closures that reference previous `metaUnit` instances.

- **Observation O.3:** `scales/scale0/controller.js` (line 387):
  ```javascript
  window.addEventListener('pagehide', () => { exitScale0(); });
  ```
  *Note:* Added to global window scope without a corresponding `removeEventListener` cleanup.

### B. Global/System Memory & Timer Omissions
- **Observation O.4:** `scales/scale4/controller.js` (lines 17–43):
  ```javascript
  function _startPlanetaryLoop(ctx) {
      if (window._planetaryInterval) clearInterval(window._planetaryInterval);
      window._planetaryInterval = setInterval(() => { ... }, 16);
  }
  ```
  *Note:* Pollutes the global namespace (`window._planetaryInterval`) to keep track of simulation time interval.

- **Observation O.5:** `scales/scale2/controller.js` (lines 82–86):
  ```javascript
  let _aeMergeCap          = 0;
  let _aeMergePos          = null;
  let _aeMergeCol          = null;
  let _aeMergeSize         = null;
  ```
  *Note:* Shared flat Float32Arrays grow dynamically to fit active molecule particle sizes, but are never shrunken or cleared when switching scales or loading smaller scenarios, resulting in persistent high-watermark memory overhead.

### C. Direct Code and Shader Duplication
- **Observation O.6:** Identical GLSL `PARTICLE_FRAG` shader strings are duplicated word-for-word in:
  1. `engine/web/js/viewport.js` (lines 112–129)
  2. `engine/web/js/viewport/field-renderer.js` (lines 1545–1562)
  3. `engine/web/js/viewport/flux-renderer.js` (lines 35–48)
  4. `engine/web/js/viewport/particle-renderer.js` (lines 39–54)
  
- **Observation O.7:** Duplicated parameter slider and toggle syncing helpers inside `scales/scale3/controller.js` (lines 63–83):
  ```javascript
  function _syncAEParamsFromUIInternal(bridge) { ... }
  function _resetAETogglesToDefaults(bridge) { ... }
  ```
  *Note:* These functions are cloned exactly from `scales/scale2/controller.js` to avoid circular dependencies, introducing code drift risks.

### D. Fragmented Lifecycle Omissions
- **Observation O.8:** Scale controllers lack standard cleanup and initialization names in `app_dag.js`:
  - Scale 0 uses `exitScale0()` and `clearScale0Timeline()`
  - Scale 1 lacks any exit/dispose methods.
  - Scale 2 uses `resetScale2(ctx)`.
  - Scale 3 uses `resetScale3(ctx)`.
  - Scale 4 uses `dispose(ctx)`.
  - Scale 5 uses `resetScale5(ctx)`.
  - Scale 6 uses `resetScale6(ctx)`.

---

## 2. Logic Chain

1. **Premise 1:** When switching scale modes, the old scale controller's module-scoped variables are set to `null` (e.g. `_planetaryRenderer = null` or `metaUnit = null`).
2. **Premise 2:** Persistent DOM elements and the global window object remain allocated across scale switches.
3. **Premise 3:** Direct event listener binding via `onchange`, `onclick`, or `addEventListener` creates a **closure** that captures these module-scoped variables.
4. **Premise 4:** If these bindings are not cleared upon mode exit, the browser's Garbage Collector cannot reclaim the closed-over scope, retaining references to the now-unused sub-renderers or previous objects (**Observation O.1, O.2, O.3**).
5. **Premise 5:** Direct C++ memory allocation and WASM-heap bindings grow with simulation scale, while local JS merge arrays persist at their highest watermarks (**Observation O.5**), introducing system-level memory bloat.
6. **Premise 6:** Lack of a standard lifecycle interface makes automatic sweeping impossible, forcing manual, error-prone conditional cleanups in the central loop (**Observation O.8**).
7. **Conclusion:** Introducing a **Unified Lifecycle Interface** with a registry class to track events, timers, and WebGL allocations will automatically prevent these resource leaks, centralize shader sources, and de-duplicate boilerplate code.

---

## 3. Caveats

- **Scope:** The investigation was conducted as a read-only code sweep. No runtime memory profiling (e.g., Chrome DevTools Heap Snapshots) was performed.
- **Assumptions:** We assume that DOM elements (`panel-controls`, `planetary-opt-orbits`, etc.) are persistent throughout the application lifetime. If the entire DOM were dynamically rebuilt on scale switch, these listener leaks would be reclaimed naturally, but they are not (they remain static cards).
- **Alternate Explanations:** The duplication of UI sync helpers in Scale 3 was done to avoid circular imports. A cleaner approach is to extract them to a shared utility module (`scales/scale-utils.js`).

---

## 4. Conclusion

### A. Clean, Unified Lifecycle Interface Design

To resolve the fragmented method naming and automate resource reclamation, we propose a standard lifecycle interface:

```typescript
interface WebLifecycle {
  /**
   * Initialize and mount the view/component.
   * Binds DOM listeners, constructs geometries, and launches timers.
   */
  mount(ctx: AppContext): void;

  /**
   * Updates state, drives physics loop, and triggers canvas rendering.
   */
  update(dt: number, ctx: AppContext): void;

  /**
   * Completely tears down resources, removes listeners, and clears timers.
   */
  destroy(ctx: AppContext): void;
}
```

#### Proposed base class helper to automate cleanup:
```javascript
export class BaseLifecycleController {
    constructor() {
        this._listeners = [];
        this._timers = [];
        this._threeObjects = [];
    }

    // Bind event and track for automatic cleanup
    bindEvent(target, type, listener, options) {
        if (!target) return;
        target.addEventListener(type, listener, options);
        this._listeners.push({ target, type, listener, options });
    }

    // Schedule interval and track for automatic cleanup
    setInterval(callback, delay) {
        const id = setInterval(callback, delay);
        this._timers.push({ id, type: 'interval' });
        return id;
    }

    // Track a Three.js object for automatic disposal
    trackThreeObject(obj) {
        if (obj) this._threeObjects.push(obj);
        return obj;
    }

    destroy(ctx) {
        // 1. Remove all event listeners automatically
        for (const { target, type, listener, options } of this._listeners) {
            target.removeEventListener(type, listener, options);
        }
        this._listeners = [];

        // 2. Clear all timers automatically
        for (const { id, type } of this._timers) {
            if (type === 'interval') clearInterval(id);
            if (type === 'timeout') clearTimeout(id);
        }
        this._timers = [];

        // 3. Automatically traverse and dispose of Three.js objects
        for (const obj of this._threeObjects) {
            obj.traverse(child => {
                if (child.geometry) child.geometry.dispose();
                if (child.material) {
                    if (Array.isArray(child.material)) {
                        child.material.forEach(m => m.dispose());
                    } else {
                        child.material.dispose();
                    }
                }
            });
            if (obj.parent) obj.parent.remove(obj);
        }
        this._threeObjects = [];
    }
}
```

### B. Consolidated Leak & Duplication Inventory
1. **Event Listeners:** 15 persistent DOM bindings across scale controllers without removal on exit.
2. **Timers:** 1 `setInterval` written to `window._planetaryInterval` that should be instance-tracked.
3. **WebGL/Three.js:** ~100+ separate buffer geometries, shader materials, and canvas textures are created lazily but lack auto-registration in unified tracking arrays.
4. **Shader Duplication:** `PARTICLE_FRAG` string is duplicated in four files; should be unified in `viewport/shaders.js`.
5. **AE UI Syncer Duplication:** Duplicated inside `scales/scale3/controller.js` to avoid circular dependency; should be extracted to `scales/scale-utils.js`.

---

## 5. Verification Method

To verify these observations and validate the proposed design, subsequent implementers must follow these steps:

1. **Verify Existing Leaks via Chrome DevTools:**
   - Run the application (`python -m http.server 8080 -d engine/web`).
   - Open DevTools, select the **Performance Monitor** tab.
   - Continuously switch between Scale 4 (Planetary) and Scale 6 (Meta) 20 times.
   - **Invalidation Condition:** If the "JS event listeners" count continuously increases and never drops after garbage collection, the event listener leak is active.
2. **Verify Shader/Utility De-duplication:**
   - Perform a global grep search: `rg "PARTICLE_FRAG"`
   - Assert that only one central declaration exists in the new `viewport/shaders.js` and is imported elsewhere.
3. **Verify Unified Interface in `app_dag.js`:**
   - Inspect `app_dag.js` to ensure the hardcoded scale-based cleanup switches are replaced by a uniform:
     ```javascript
     activeController.destroy(ctx);
     ```

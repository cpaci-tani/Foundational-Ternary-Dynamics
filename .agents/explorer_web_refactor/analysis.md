# FTD Web Frontend Refactor — Deep-Dive Investigation & Analysis

This report documents the findings from a read-only exploratory sweep of the FTD browser-based dashboard frontend (`engine/web/js/`). The investigation focused on identifying **modular duplication**, **lifecycle omissions**, and **potential WebGL, event listener, and timer leaks** across:
- `viewport/field-renderer.js`
- `viewport/flux-renderer.js`
- `viewport/topology-sheet-renderer.js`
- `app_dag.js`
- The scale controllers in `scales/`

---

## 1. WebGL & Memory Resource Leak Inventory

While the renderers have manually written cleanup/dispose logic, several critical leak vectors and architectural patterns pose severe memory accumulation risks, particularly during long-lived browser sessions, dynamic lattice resizing, or continuous scale-switching.

### A. Stale Event Listeners and Closure Capture (Scale Controllers)
Scale controllers are written as ES modules that maintain local, module-scoped private variables. When a user switches between scales, the active controller binds event handlers directly to persistent DOM elements using standard assignment (`onclick`, `onchange`).

1. **Stale Callback Closures in Scale 4 (Planetary)**
   - **File:** `scales/scale4/controller.js`
   - **Lines:** 117–130 (`_bindToggles()`)
   - **Code:**
     ```javascript
     optOrbits.onchange = (e) => {
         if (_planetaryRenderer) _planetaryRenderer.setRenderOrbits(e.target.checked);
     };
     ```
   - **Mechanism:** Assigning `onchange` replaces previous callbacks on that DOM element, but leaves the *closure* pointing to the stale reference. The callback closes over module-scoped variables (`_planetaryRenderer`), preventing memory reclamation even after `_planetaryRenderer` is nulled out. When switching away from Scale 4, `dispose()` is called but these `onchange` assignments on persistent DOM elements (`planetary-opt-orbits`, `planetary-opt-axes`) are **never** cleared.
   
2. **Stale Callback Closures in Scale 6 (Meta)**
   - **File:** `scales/scale6/controller.js`
   - **Lines:** 88–114 (`loadMetaScenario`)
   - **Code:**
     ```javascript
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
   - **Mechanism:** Upon re-entering Scale 6, the `onclick` handlers of 13 geometric toggle buttons are re-assigned. Since the buttons are persistent in the DOM, leaving Scale 6 does **not** clear these handlers. The closures keep closing over the previous `metaUnit` reference until it's overwritten on next re-entry. If the user clicks these buttons when in another mode, or if a global cleanup sweeps the DOM, the event reactions continue to reference nulled or stale resources.

3. **Accumulating Window Listeners in Scale 0**
   - **File:** `scales/scale0/controller.js`
   - **Line:** 387
   - **Code:**
     ```javascript
     window.addEventListener('pagehide', () => { exitScale0(); });
     ```
   - **Mechanism:** This window-level event listener is bound during module load. If the application is ever re-initialized, or in hot-reloading testing environments, these listeners accumulate on the `window` object indefinitely. There is no corresponding `removeEventListener` call.

### B. WebGL / Three.js Resource Disposal
Dynamic recreation of Three.js objects can easily exhaust the GPU's memory (VRAM) if geometries, materials, and textures are not explicitly disposed of via `dispose()`.

1. **Lazy Reconstruction in Topology Sheet Renderer**
   - **File:** `viewport/topology-sheet-renderer.js`
   - **Lines:** 345–491
   - **Mechanism:** The topology sheet renderer employs an intensive scatter/blur pipeline (`_scatterHeights`) utilizing custom `ShaderMaterial` instances and `DataTexture` layers. While it successfully handles geometry and material disposal during lattice resizing, it allocates multiple intermediate float buffers and lookup tables. If these arrays are not bounded or cached, it causes garbage collection (GC) thrashing.
   
2. **Scale 2/3 Electron Orbital Cloud Buffers**
   - **File:** `scales/scale2/controller.js`
   - **Lines:** 82–86 (`_aeMergePos`, `_aeMergeCol`, `_aeMergeSize`)
   - **Mechanism:** When merging bonding electron clouds with nuclear core clouds, the controller allocates large flat `Float32Array` buffers. To prevent memory leaks, these grow dynamically (`_aeMergeCap = Math.max(mergedCount, _aeMergeCap * 2)`) but are **never shrunken or freed**. In a session where the user loads a massive molecule (e.g. caffeine) and then switches to a simple atom (e.g., hydrogen), the maximum buffer size remains allocated in system memory.

### C. Timers & Intervals
1. **Planetary Interval Management**
   - **File:** `scales/scale4/controller.js`
   - **Lines:** 17–43 (`_startPlanetaryLoop`)
   - **Mechanism:** Scale 4 creates a planetary simulation loop using `setInterval` at a 16ms rate. It writes to the global `window._planetaryInterval`. While `dispose()` successfully performs `clearInterval(window._planetaryInterval)`, writing to a global window property creates a namespace pollution risk and is vulnerable to external overrides or state divergence.

---

## 2. Modular Duplication & Copy-Paste Patterns

Several key sub-systems exhibit substantial modular duplication and copy-paste code, primarily because the renderers and controllers were developed in isolation.

### A. Shader Source Duplication
The custom shaders for radial soft particles are duplicated word-for-word across four separate files.

- **Duplicated Shader String (`PARTICLE_FRAG`):**
  ```javascript
  const PARTICLE_FRAG = `
      uniform vec3 color;
      uniform float opacity;
      varying float vAlpha;
      void main() {
          vec2 center = gl_PointCoord - vec2(0.5);
          float dist = length(center);
          if (dist > 0.5) discard;
          float intensity = smoothstep(0.5, 0.0, dist);
          float alpha = intensity * intensity * opacity * vAlpha;
          gl_FragColor = vec4(color * (intensity * 0.5 + 0.5), alpha);
      }
  `;
  ```
- **Occurrences:**
  1. `engine/web/js/viewport.js`
  2. `engine/web/js/viewport/field-renderer.js`
  3. `engine/web/js/viewport/flux-renderer.js`
  4. `engine/web/js/viewport/particle-renderer.js`

Centralizing these shaders in a unified utility module (e.g. `viewport/shaders.js`) would eliminate over 100 lines of duplicated GLSL strings and allow global optimizations (e.g., adjusting the soft edge drop-off or color mapping) in a single place.

### B. Renderer Lifecycle Boilerplate
Every viewport renderer (`FieldRenderer`, `FluxRenderer`, `TopologySheetRenderer`) implements its own manually maintained `onLatticeSizeChanged(size, halfN)` and `dispose()` method.
- **Copy-Paste Pattern:**
  - Standard mesh array iteration to call `disposeMesh` or call `.geometry.dispose()` and `.material.dispose()`.
  - Manual tracking of WebGL resources via lists of keys (e.g., `simpleMeshFields` in `field-renderer.js`, line 2312).
- **Consequence:** If a developer adds a new overlay or helper geometry, they must remember to manually add its key to `simpleMeshFields` or the local disposal list. This is highly error-prone and a major leak vector.

### C. UI Sync and Toggle Management (Scale 2 vs. Scale 3)
Scale 2 (Atoms) and Scale 3 (Molecules) share the same underlying Atom Engine (AE) C++ bridge. However, the scenario initialization and parameter synchronization logic is duplicated between them.
- **Occurrence:**
  - `scale3/controller.js` duplicates `_syncAEParamsFromUIInternal` and `_resetAETogglesToDefaults` (lines 57–84) from `scale2/controller.js` to avoid circular dependencies.
- **Consequence:** Modifying a parameter toggle in Scale 2 requires double-updating the logic in Scale 3, otherwise the parameters will drift and exhibit inconsistent physics behavior.

---

## 3. Class & View Lifecycle Omissions

A major architectural finding is that **none** of the scale controllers or view components share a unified lifecycle interface.

```
                  ┌───────────────────────┐
                  │      app_dag.js       │
                  │  (Central Orchestrator)│
                  └───────────┬───────────┘
         ┌────────────────────┼────────────────────┐
         ▼                    ▼                    ▼
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│Scale1Controller │  │Scale4Controller │  │Scale6Controller │
│ (No dispose/exit│  │ (Has dispose()  │  │ (Has resetScale6│
│  or standard    │  │  uses global    │  │  custom name    │
│  interface)     │  │  interval)      │  │  no standard)   │
└─────────────────┘  └─────────────────┘  └─────────────────┘
```

- **Scale 0 Controller:** Has `enter()`, `exit()`, `step()`, but relies on `exitScale0()` (from `scenario-loader.js`) and `clearScale0Timeline()` for cleanup.
- **Scale 1 Controller:** Lacks any `enter()` or `exit()` / `dispose()` methods entirely.
- **Scale 2 Controller:** Uses `resetScale2(ctx)` for cleanup.
- **Scale 3 Controller:** Uses `resetScale3(ctx)` for cleanup.
- **Scale 4 Controller:** Uses `dispose(ctx)` for cleanup.
- **Scale 5 Controller:** Uses `resetScale5(ctx)` for cleanup.
- **Scale 6 Controller:** Uses `resetScale6(ctx)` for cleanup.

### The Impact
Because every scale controller cleans up using a different function name (`dispose`, `resetScale*`, `exit`), the central orchestrator `app_dag.js` must implement highly custom, hardcoded conditional checks when switching modes:
```javascript
// Hardcoded cleanup logic in app_dag.js:
if (mode !== 'cosmic') Scale5Controller.resetScale5(_makeCtx());
if (mode !== 'planetary') Scale4Controller.dispose({ ... });
if (mode !== 'meta') Scale6Controller.resetScale6(_makeCtx());
```
This hardcoding makes the system fragile and prevents the modular addition of new scales. It also leaves scale controllers vulnerable to partial cleanup, resulting in the stale event listener and timer leaks identified above.

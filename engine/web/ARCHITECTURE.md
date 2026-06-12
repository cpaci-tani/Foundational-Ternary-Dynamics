# FTD Web Engine Architecture

**Status:** active architecture map, refreshed after the June 2026 cleanup pass.
**Scope:** `engine/web/` browser runtime, UI shell, bridges, scale controllers,
rendering, docs, and tests. The C++ engine is documented in
`engine/SPEC_ENGINE.md`.

If this file disagrees with code, the code wins. Update this file and
`docs/INDEX.md` in the same cleanup when a major boundary changes.

---

## Current Runtime Shape

The web engine is a native ES-module browser app. There is no bundler: the sole
HTML app entry point is `index.html`, which loads `js/app.js`; Three.js is
provided through the import map in `index.html`.

At runtime:

```text
index.html
  -> js/app.js                         composition root
     -> AppShell                       toolbar, tabs, panels, responsive shell
     -> bridge selection               native WebSocket -> WASM -> JS mock
     -> Viewport                       shared Three.js scene facade
     -> per-scale controller           scale lifecycle + animation
     -> telemetryHub                   shared telemetry buffers
```

The active dashboard scales are:

| Scale | Mode value | Controller | Role |
|---|---|---|---|
| 0 | `lattice` | `js/scales/scale0/controller.js` | substrate, flux, scenarios, overlays |
| 1 | `particles` | `js/scales/scale1/controller.js` | particle engine |
| 2 | `atoms` | `js/scales/scale2/controller.js` | atom engine UI |
| 3 | `molecules` | `js/scales/scale3/controller.js` | molecular UI over AE-style runtime |
| 4 | `planetary` | `js/scales/scale4/controller.js` | planetary sandbox |
| 5 | `cosmic` | `js/scales/scale5/controller.js` | cosmic sandbox |
| Meta | `meta` | `js/scales/scale6/controller.js` with `scale12` UI registration | existential unit / Moore geometry view |

Scale 11 / reference-frame-context mode is no longer a live scale in the web
tree. Older audit and refactor documents that mention it are historical.

---

## Directory Hierarchy

```text
engine/web/
├── index.html                 browser entry
├── serve.py                   dev/test server with COOP/COEP headers
├── ARCHITECTURE.md            this file
├── docs/
│   ├── INDEX.md               documentation navigation
│   ├── SPEC_*.md              active specs
│   ├── PLAN_*.md              active implementation plans
│   ├── audits/                point-in-time audits
│   ├── historical/            tracked provenance, no longer active guidance
│   └── adr/                   architecture decision records
├── css/
│   ├── tokens.css             semantic tokens
│   ├── ui/primitives/         buttons, fields, tabs, toggles, etc.
│   ├── ui/components/         shell and component CSS
│   ├── ui/panels/             panel CSS
│   ├── ui/scales/             scale-specific CSS
│   └── themes/                theme overrides
├── js/
│   ├── app.js                 composition root and cross-scale wiring
│   ├── telemetry-hub.js       shared telemetry buffers
│   ├── bridge-init.js         bridge re-export + capability installation
│   ├── bridge/                bridge implementations and scenario seeds
│   ├── physics/               physics harness adapters
│   ├── scales/                per-scale controllers
│   ├── ui/                    shell, panels, components, charts
│   └── viewport/              focused renderer modules
├── tests/                     Playwright + node web tests
└── wasm/                      generated Emscripten artifacts consumed by WASM bridge
```

Generated doc renders (`docs/*.html`, `docs/*_files/`) are local artifacts and
are ignored. Markdown is the tracked source of truth.

---

## Layer Responsibilities

### Application Shell

`js/app.js` is still the composition root. It owns bridge initialization, global
play state, mode switching, top-level service construction, and the shared
`requestAnimationFrame` dispatch.

`js/ui/shell/app-shell.js` owns the modern shell surface: toolbar registration,
panel registry validation, responsive state, panel docking, mobile behavior,
tooltips, knowledge base, FAQ, and keyboard help.

Architectural state: healthy direction, but `app.js` remains a large legacy root
and should continue shrinking as scale-specific control wiring moves into each
scale package.

### Scale Controllers

Controllers are leaves imported by `app.js`; they do not import `app.js`.
Scale 0 is the most mature package:

```text
js/scales/scale0/
├── controller.js
├── runtime/          tick, frame sync, overlays, diagnostics, scenario loader
├── state/            state store and dirty flags
├── ui/               bindings, controls, overlays, toolbar registration
├── analysis/         analysis helpers
├── scenario-registry.js
└── viewport-adapter.js
```

Scale 1-5 and Meta are more mixed: they have controllers and some registered UI
modules, but still rely more heavily on the app root and shared viewport APIs.

### Bridge Layer

The bridge is the physics boundary. Current implementations:

| Bridge | Path | Role |
|---|---|---|
| `WebSocketBridge` | `js/ws-bridge.js` | native server bridge when available |
| `WasmBridge` | `js/bridge/wasm-bridge.js` | canonical in-browser C++ bridge |
| `MockBridge` | `js/bridge/mock-bridge.js` | JS fallback/reference bridge |
| `MockBridgeProxy` | `js/bridge/mock-bridge-proxy.js` | worker-backed JS bridge for Scale 0 flux/mock scenarios |

All bridge consumers should prefer `bridge.capabilities.scaleN.*` surfaces.
Direct bridge reads are allowed only when listed by the bridge contract or when a
panel deliberately uses a debug/global path.

Authoritative bridge details live in
`docs/SPEC_SCALE0_BRIDGE_ARCHITECTURE.md` and `js/bridge/README.md`.

### Rendering

`js/viewport.js` is the public facade used by controllers. It delegates a large
amount of work to focused modules under `js/viewport/`, especially:

- `scene-core.js`
- `particle-renderer.js`
- `flux-renderer.js`
- `field-renderer.js`
- `molecular-renderer.js`
- `topology-sheet-renderer.js`

Architectural state: partially decomposed. `field-renderer.js` is still a large
module and remains a prime candidate for focused decomposition once visual/test
coverage is sufficient.

### Telemetry

`js/telemetry-hub.js` is the dashboard telemetry source of truth. Scale 0
runtime collection happens through `js/scales/scale0/runtime/diagnostics.js`.
Panels and charts should read hub buffers/descriptors rather than ad hoc bridge
queries unless the panel is intentionally live-sampling an active owner.

The Scale-0 telemetry catalog lives at `docs/TELEMETRY_CATALOG_SCALE0.md`.

---

## Main Loop

The app root owns the primary `requestAnimationFrame` loop and dispatches to the
active scale. Scale 0 has a five-stage pipeline:

```text
advanceSimulation
  -> syncRenderableData
  -> updateFieldOverlays
  -> renderFrame
  -> updateDiagnosticsAndPanels
```

Floating/low-frequency panels use `js/lib/raf-coordinator.js` rather than each
owning a separate rAF loop.

Scale 4/5 have specialized physics cadence inside their controllers; check the
controller before assuming Scale-0 timing semantics.

---

## Documentation Map

Start with `docs/INDEX.md`.

Primary active references:

- `docs/SPEC_SCALE0_BRIDGE_ARCHITECTURE.md`
- `docs/SPEC_SCALE0_RUNTIME_PIPELINE.md`
- `docs/SPEC_SCALE0_SCENARIO_ARCHITECTURE.md`
- `docs/USER_GUIDE.md`
- `docs/TOGGLE_REGISTRY.md`
- `docs/TELEMETRY_CATALOG_SCALE0.md`
- root `CONTRACTS.md` for cross-project interface summaries

Historical/provenance docs live in `docs/historical/` or `docs/audits/`.
Do not use historical docs as active implementation guidance without checking
the current code.

---

## Known Architectural Debt

1. `js/app.js` still owns too much direct DOM wiring and cross-scale glue.
2. Scenario definitions still span registry, metadata, JS seed bodies, C++
   seed bodies, and toggle profiles. ADR `docs/adr/0002-scenario-architecture.md`
   remains the decision point.
3. Large modules remain: `viewport/field-renderer.js`, `bridge/mock-bridge.js`,
   `scales/scale0/ui/overlays/p1-observables-panel.js`, and
   `bridge/wasm-bridge.js`.
4. Static tooling is light. Tests are strong, but there is no lint/type/import
   boundary gate yet.
5. Debug globals (`window.__ftd*`, `window._ftdBridge`) are useful but should be
   treated as escape hatches, not normal module boundaries.

---

## Test Architecture

The Playwright suite under `tests/` is the main web regression harness. It
covers scale switching, worker path, lifecycle, panel rendering, scenario
parity, all-scenario Scale-0 telemetry, and several physics/protocol checks.

Run from `engine/web/tests/`:

```bash
npm test
npx playwright test scale0-scenario-telemetry-contract.spec.js
```

The configured server is `python serve.py 8081 --cache --quiet` from
`engine/web/`; it sends COOP/COEP headers so SharedArrayBuffer and the worker
path are available.

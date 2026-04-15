# FTD Web Engine — Architecture

This document describes the architecture of the browser-based FTD
dashboard under `engine/web/`. It is maintained alongside the code;
if something here contradicts the code, update the doc in the same
commit.

The engine itself (C++ physics, CUDA kernels, CTest suite) is
documented in `engine/SPEC_ENGINE.md`. This document is specifically
about the web-facing presentation layer.

---

## 1. Entry point

```
engine/web/
├── index.html               ← single module-script loader
├── css/
│   ├── tokens.css           layout / colors / themes
│   ├── layout.css
│   ├── components.css
│   ├── scale-visibility.css controls which UI is visible per mode
│   ├── charts.css
│   └── themes/{abyss,light,parchment,nord}.css
├── js/
│   ├── app_dag.js           main application controller (~2.5k LOC)
│   ├── constants.js         single source of truth for all constants
│   ├── viewport.js          Three.js scene graph (see ADR 0001)
│   ├── wasm-bridge-dag.js   WASM facade + MockBridge fallback
│   ├── bridge/
│   │   ├── bridge-factory-dag.js
│   │   ├── mock-scale4.js   (planetary N-body)
│   │   └── mock-scale5.js   (cosmic N-body)
│   ├── core/
│   │   ├── BaseRenderer.js  (cosmic + planetary share this)
│   │   └── event-bus.js     (Scale 4 only)
│   ├── scales/
│   │   ├── scale-utils.js   (shared helpers — see §4)
│   │   ├── scale0/controller.js   lattice
│   │   ├── scale1/controller.js   particles (PE engine)
│   │   ├── scale2/controller.js   atoms (AE engine)
│   │   ├── scale3/controller.js   molecules (aliases Scale 2)
│   │   ├── scale4/controller.js   planetary
│   │   ├── scale5/controller.js   cosmic
│   │   ├── scale6/controller.js   meta (existential unit)
│   │   └── scale11/controller.js  consciousness
│   └── (utilities: dom-utils, diagnostics, charts, inspector,
│       pe-telemetry, lagrangian, units, fields, fieldlines, ...)
├── docs/
│   └── adr/                 architectural decision records
│       └── 0001-viewport-decomposition.md
└── tests/
    ├── playwright.config.js
    ├── scales.spec.js       (12 smoke tests)
    └── README.md
```

`index.html` loads exactly one module script:

```html
<script type="module" src="js/app_dag.js?v=..."></script>
```

Everything else is pulled in via ES module imports at load time. There
is no bundler step. Three.js is served from a CDN via importmap.

---

## 2. The DAG variant

The file suffix `_dag` appears on `app_dag.js`, `wasm-bridge-dag.js`,
and `bridge/bridge-factory-dag.js`. It marks the post-refactor
variant of files whose legacy counterparts were deleted in the
April 2026 "Phase A" cleanup. The acronym is from the pattern of
per-scale controllers being a directed acyclic graph rooted at the
main app (no controller imports `app_dag.js`; controllers are leaves).

There are **no** pre-DAG variants live anywhere. If you find an
`app.js` or `wasm-bridge.js` without the `-dag` suffix, it was
introduced after April 2026 and is a regression — these filenames
were deleted and should not come back.

---

## 3. The main loop

`app_dag.js` owns the primary `requestAnimationFrame` loop at
`animate()`. Each frame:

1. Schedule next rAF **first** (unconditional — the loop survives
   even if a mode-specific animator throws).
2. Dispatch to the mode-specific animator based on `engineMode`:
   - `lattice` → `Scale0Controller.animateLattice(ctx)`
   - `particles` → `animatePE(now)` (inline in app_dag)
   - `atoms` / `molecules` → `animateAE(now)` (inline)
   - `planetary` → no-op here; Scale 4 uses its own `setInterval`
     inside `Scale4Controller.loadScenario`
   - `cosmic` → `Scale5Controller.animateCosmic(ctx)`
   - `meta` → `Scale6Controller.updateMeta(ctx, 1/60)`
   - `consciousness` → `Scale11Controller.animateConsciousness(...)`
3. Update `BackgroundManager` and any floating UI trackers.
4. Increment `frameCount`; every ~1 s, publish FPS to the status bar.

Scale 4 (planetary) and pre-Phase-B.1 Scale 5 (cosmic) used an
independent `setInterval` for physics. The cosmic setInterval was
removed in Phase B.1; Scale 4 still uses one and is tracked
separately.

---

## 4. Scale controller contract

Every scale controller exports a subset of the following API. The
exact set depends on the scale, but the naming convention is
consistent:

| Export              | Called by                               |
|---------------------|-----------------------------------------|
| `animate{Mode}(ctx)` | main rAF loop (once per frame)          |
| `load{Mode}Scenario(ctx, name)` | scenario selector, mode switch |
| `step(ctx)`         | Step button or `s` keybinding           |
| `reset{Scale}(ctx)` | main app when switching AWAY from mode  |
| (mode-specific setters) | UI wiring in `app_dag.js`          |

The `ctx` object is built by `_makeCtx()` in `app_dag.js` and holds
everything a controller needs: `bridge`, `viewport`, `running`,
`ticksPerFrame`, `frameCount`, `inspector`, panels, UI-update
callbacks. `ctx.frameCount` is a getter that always returns the
current module-level value, so controllers can freely read it each
frame without stale-copy issues.

**Reset semantics.** `resetScale{N}(ctx)` is called when switching
AWAY from a scale, not when switching INTO one. It disposes
GPU-resident resources (Three.js geometries, materials, textures)
owned by that scale and clears any per-scale module state.
Persistent state that the scale wants to retain across re-entries
(Scale 11's `_csPedagogy`, listener bags, prepared DOM) lives in
module-level variables that `reset` does NOT touch — see Phase B.2
in the April 2026 refactor plan for the canonical example.

### Shared helpers in `scales/scale-utils.js`

Scale controllers pull the following from `scale-utils.js`:

- `createTickAccumulator()` — fractional-tick accumulator for sub-1
  simulation speeds. Every controller uses this.
- `formatSI(n)` — K/M/G/T suffixed number formatter for status bars.
- `formatNumber(n)` — fixed/exponential dispatcher for diagnostics.
- `throttleBySize(L, table)` — lattice-size-dependent throttle lookup.
- `createStatusBarCache()` — deduplicating DOM text writer.
- `createListenerBag()` — trackable `addEventListener` bag for clean
  teardown in `reset*` functions.

New shared patterns that appear in 2+ controllers should land here.

---

## 5. Bridge hierarchy

```
                    ┌──────────────────────┐
                    │   Scale controllers  │
                    │  (scale{0..11}/...)  │
                    └──────────┬───────────┘
                               │ bridge.peTick() / aeTick() / getFluxVectorSampled() / ...
                               ▼
                    ┌──────────────────────┐
                    │     _ftdBridge       │ ← app_dag module-level `bridge` variable
                    └──────────┬───────────┘
                               │ created by createBridge()
                               ▼
        ┌────────────────┬─────────────────┬─────────────────┐
        │                │                 │                 │
        ▼                ▼                 ▼                 ▼
┌──────────────┐  ┌─────────────┐   ┌─────────────┐   ┌──────────────┐
│  WasmBridge  │  │  MockBridge │   │CosmicMock-  │   │ PlanetaryMock│
│  (real wasm) │  │  (pure JS)  │   │   Bridge    │   │    Bridge    │
│              │  │             │   │             │   │              │
│wasm-bridge-  │  │wasm-bridge- │   │bridge/      │   │bridge/       │
│dag.js        │  │dag.js       │   │mock-scale5  │   │mock-scale4   │
└──────────────┘  └─────────────┘   └─────────────┘   └──────────────┘
```

**Selection policy.** `createBridge()` tries the WebSocket native-GPU
bridge first (if `ws://localhost:9100` is reachable), then
`WasmBridge.init()`, then falls back to `MockBridge`. The
WebSocket bridge is a dev-time optional path for running against
a standalone C++ server; in the browser-only path it simply fails
its connection attempt and the exponential backoff emits the
`[ws-bridge] Reconnecting in Ns...` logs you see in the console.

**Scale 4 and Scale 5** each bring their own mock bridge instead of
sharing `_ftdBridge`. They are independent N-body simulators with
their own data structures (`mock-scale4.js` and `mock-scale5.js`).
They are re-exported from `wasm-bridge-dag.js` so callers can
`import { CosmicMockBridge } from './wasm-bridge-dag.js'`, but the
canonical source is `bridge/mock-scale{4,5}.js` — scale controllers
can and should import directly from there.

**Scale 11** temporarily replaces `ctx.bridge` with a flux-only
`MockBridge(32)` on entry (saving the original in `_savedBridge`) so
the consciousness lattice can run its own wave dynamics without
interference from whatever the user had active in Scale 0. The
original bridge is restored in `resetScale11`. This is the only
place in the engine where `ctx.bridge` mutates mid-session.

---

## 6. Constants contract

`engine/web/js/constants.js` is the **single source of truth** for
all physics constants in the web engine. Every other module that
needs a constant imports it from there by name.

There are two categories:

- **FTD-derived constants** (the "framework scale"): `K_B`, `ALPHA`,
  `G_N`, `G_STAR`, `VARPI`, `X_PLUS`, `X_MINUS`, `N_C`, `N_BASE`,
  `B_3`, `N_EFF`, `MU_RATIO`, `TAU_RATIO`, `PROTON_RATIO`,
  `M_PROTON`, `M_HIGGS`, `V_HIGGS`, ... These are the outputs of
  the ontic derivation chain and are computed at module load.

- **PDG experimental reference values** (the "physical scale"):
  `M_E_PHYS`, `M_MU_PHYS`, `M_TAU_PHYS`, `M_P_PHYS`, `M_N_PHYS`,
  `M_PI_CH_PHYS`, `M_K_CH_PHYS`, `M_SIGMA_PHYS`, `M_OMEGA_PHYS`,
  `M_DELTA_PHYS`, `M_W_PHYS`, ... These are the measured values from
  the Particle Data Group, used when comparing FTD predictions
  against experiment or when populating the particle catalog with
  observed masses.

**Do not unify the two categories.** `M_PROTON` (FTD-derived ≈ 1798
MeV, the framework mass scale for the proton at the FTD coupling)
differs from `M_P_PHYS` (PDG ≈ 938 MeV, the observed proton rest
mass) by design. See [CLAUDE.md](../../CLAUDE.md) "Epistemic
Discipline" for the underlying reasoning.

**If you find a hardcoded physics value in any web-engine file,
move it to `constants.js` and import it from there.** The only
exceptions are:

1. Rounding-fragile natural-unit computations that need a
   deterministic fork of the rounding (e.g., `atomic-energy.js`
   uses its own local `K_B = M_P_PHYS_electron` to keep SEMF
   output formats stable across refactors).
2. String-identified config tables where the literal value is
   part of a developer-facing schema (e.g., scenario default
   lattice sizes in `config/toggles.js`).

---

## 7. Testing

The Playwright smoke suite under `engine/web/tests/` covers:

- Every engine mode loads without console errors or 404s.
- Bridge initializes within 15 s.
- Scale 5 cosmic does not leak `window._cosmicInterval`
  (Phase B.1 regression guard).
- Scale 11 consciousness does not leak event listeners across 5
  re-entries (Phase B.2 regression guard).
- `constants.js` exports `K_B`, `ALPHA`, `G_STAR` as named exports
  and `K_B === 0.511`.

See `tests/README.md` for how to run. Physics correctness lives in
the C++ CTest and Python pytest suites elsewhere in the project.

---

## 8. Where the bodies are buried

- `viewport.js` is a 3,400-LOC god object. See
  [ADR 0001](docs/adr/0001-viewport-decomposition.md).

- `wasm-bridge-dag.js` is ~4,900 LOC and is the largest live file.
  It is a facade over several scenario loaders plus the WASM
  bindings; the size is mostly scenario-specific initialization
  code (particle layouts, lattice patterns). Decomposing it is a
  future initiative if it keeps growing.

- `app_dag.js` at ~2,500 LOC is the main application controller. It
  still holds a lot of UI wiring that could live in per-scale
  modules, but the trend is clearly in that direction — every new
  scale controller moves ~500 LOC out of `app_dag.js`.

- `ws-bridge.js` continuously tries to reconnect to a native GPU
  server on `ws://localhost:9100` and emits "Reconnecting in Ns..."
  / "Disconnected" / `Aborted()` logs in the console. This is
  benign noise in the browser-only path. If you want the logs to
  stop, either run a `ws_server.exe` on port 9100 or simply ignore
  them.

---

## 9. How to add a new scale

1. Create `js/scales/scale{N}/controller.js`.
2. Export `animate{Mode}(ctx)` (called every rAF frame),
   `load{Mode}Scenario(ctx, name)` (called on mode switch), and
   `reset{Scale}(ctx)` (called when switching away).
3. In `app_dag.js`, add an `import * as Scale{N}Controller from
   './scales/scale{N}/controller.js'` and wire it into the
   `animate()` dispatcher and `switchEngineMode()`.
4. Add the mode to the `engine-mode` select in `index.html`.
5. Use `scale-utils.js` for tick accumulation, number formatting,
   listener tracking — don't re-implement.
6. Add a row to `scales.spec.js` so the new mode is covered by
   the scale-sweep test.
7. Update this document with where the controller lives and which
   bridge it uses.

Keep new controllers under 1,000 LOC. If one grows larger, split
before the next feature lands.

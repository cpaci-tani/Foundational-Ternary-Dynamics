# FTD Maintainability Guide

Field manual for making changes to this project without breaking
sibling systems. Complements — does not replace — `CLAUDE.md`,
`CONTRIBUTING.md`, the layered `ARCHITECTURE.md` files, and the
`docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md` ledger.

**Who this is for:** a maintainer (human or agent) who has skimmed
`README.md` and `CLAUDE.md` and now wants to do work without
accidentally violating a project invariant.

---

## Table of Contents

- [Part 1 — Architecture at a glance](#part-1--architecture-at-a-glance)
  - [1.1 Layer map](#11-layer-map)
  - [1.2 Where a new X goes](#12-where-a-new-x-goes)
  - [1.3 Module boundaries](#13-module-boundaries)
- [Part 2 — Hazards and Recipes](#part-2--hazards-and-recipes)
  - [2.1 Hazards](#21-hazards)
    - [H1 — Panel-registry ↔ DOM-stub contract](#h1--panel-registry--dom-stub-contract)
    - [H2 — renderMathInHtml(escapeHtml(x)) rule](#h2--rendermathinhtmlescapehtmlx-rule)
    - [H3 — Epistemic-tag discipline](#h3--epistemic-tag-discipline)
    - [H4 — Constants single source of truth](#h4--constants-single-source-of-truth)
    - [H5 — Cross-renderer scope](#h5--cross-renderer-scope)
    - [H6 — System-clock / date fixtures](#h6--system-clock--date-fixtures)
    - [H7 — KaTeX CDN fallback](#h7--katex-cdn-fallback)
    - [H8 — Scale-gated toggles](#h8--scale-gated-toggles)
  - [2.2 Recipes](#22-recipes)
    - [R1 — Add a Scale 0 diagnostic row](#r1--add-a-scale-0-diagnostic-row)
    - [R2 — Add a Scale 0 chart series](#r2--add-a-scale-0-chart-series)
    - [R3 — Add a Scale 0 scenario](#r3--add-a-scale-0-scenario)
    - [R4 — Add a new panel](#r4--add-a-new-panel)
    - [R5 — Add a Verify manifest row](#r5--add-a-verify-manifest-row)
    - [R6 — Add a FAQ entry](#r6--add-a-faq-entry)
    - [R7 — Add a Scene control](#r7--add-a-scene-control)
    - [R8 — Add a viewport-overlay toggle](#r8--add-a-viewport-overlay-toggle)
    - [R9 — Add a physics constant](#r9--add-a-physics-constant)
    - [R10 — Rename a physics constant](#r10--rename-a-physics-constant)
    - [R11 — Add a Playwright spec](#r11--add-a-playwright-spec)
    - [R12 — Add a Python proof](#r12--add-a-python-proof)
    - [R13 — Add a theory doc](#r13--add-a-theory-doc)
    - [R14 — Update CHANGELOG](#r14--update-changelog)
    - [R15 — Commit workflow](#r15--commit-workflow)
- [Part 3 — Technical-debt ledger](#part-3--technical-debt-ledger)
  - [3.1 Live debt](#31-live-debt)
  - [3.2 Deferred features](#32-deferred-features)
  - [3.3 Cross-reference with TRACKER_OPEN_ITEMS.md](#33-cross-reference-with-tracker_open_itemsmd)

---

## Part 1 — Architecture at a glance

### 1.1 Layer map

```
┌────────────────────────────────────────────────────────────────┐
│  docs/theory/          (~115 markdown docs, physics claims)    │
│  ├── 01_reference/     (SPEC_FTD, REF_CLAIMS_MATRIX, …)         │
│  ├── 02_foundations/…  (epistemic-tagged derivations + audits)  │
│  └── 07_assessment/    (AUDIT_*.md, TRACKER_OPEN_ITEMS.md)      │
└────────────────────────────────────────────────────────────────┘
                              │
                              ▼ cites
┌────────────────────────────────────────────────────────────────┐
│  scripts/                                                      │
│  ├── constants.py      ← SINGLE SOURCE OF TRUTH for physics    │
│  │                      constants (G*, α, N_c, K_B, …)         │
│  ├── proofs/           (pytest: proof_complete_sm.py etc.)     │
│  ├── verification/     (40 derivation-verification scripts)    │
│  ├── audit/            (build_verify_manifest.py, others)      │
│  └── tests/            (7-tier comprehensive framework)        │
└────────────────────────────────────────────────────────────────┘
                     │                            │
              mirrors │                            │ writes
                     ▼                            ▼
┌──────────────────────────────┐   ┌──────────────────────────────┐
│  engine/include/ftd/ontic.h  │   │  engine/web/data/            │
│  (C++ mirror of constants.py)│   │    verify-manifest.json      │
└──────────────┬───────────────┘   └────────────┬─────────────────┘
               │                                 │
               ▼ compiled into                   │
┌────────────────────────────────┐               │
│  engine/ (C++ simulation +     │               │
│  CUDA + WASM)                  │               │
│  ├── src/         (6 .cpp)     │               │
│  ├── cuda/        (GPU path)   │               │
│  ├── wasm/        (Emscripten) │               │
│  └── tests/       (169 CTest)  │               │
└──────────────┬─────────────────┘               │
               │                                 │
         built ▼ as WASM                         │
┌────────────────────────────────────────────────┴────────────────┐
│  engine/web/ (Three.js dashboard)                              │
│  ├── js/constants.js       ← mirrors scripts/constants.py      │
│  ├── js/telemetry-hub.js   (RingBuffer hub, hub.s0.*, …)       │
│  ├── js/viewport.js        (shared Three.js scene for 0-3)     │
│  ├── js/scales/*/          (per-scale controllers)             │
│  ├── js/ui/panels/*        (descriptor-driven panels)          │
│  │   └── scene-panel/                                          │
│  └── js/verify-panel/      (evidence scoreboard)               │
└────────────────────────────────────────────────────────────────┘
                              │
                              ▼ disseminated via
┌────────────────────────────────────────────────────────────────┐
│  dissemination/  (manuscript v2, book, whitepaper, interactive)│
└────────────────────────────────────────────────────────────────┘
```

**Reading rule.** Constants flow top-down: Python canonical → C++
header mirror → WASM runtime → JS dashboard. Anything that changes
physics changes `scripts/constants.py` first; the mirrors follow.
Theory documents cite constants by name; they don't produce values.

### 1.2 Where a new X goes

For each common addition, the files you touch and their order:

| You want to add | Files (in order) |
|---|---|
| Scale-0 diagnostic row | 1) `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js` — add the row. 2) The producer that writes to `hub.s0.diag` in `scales/scale0/controller.js`. 3) `engine/web/docs/TELEMETRY_CATALOG_SCALE0.md` — document it. |
| Scale-0 chart series | `engine/web/js/ui/panels/charts-panel/descriptors/scale0.js` + a `RingBuffer` in `engine/web/js/telemetry-hub.js` if one doesn't exist. |
| Scale-0 scenario | `engine/web/js/config/scenarios.js` with a new seed entry + `engine/web/js/scales/scale0/scenario-registry.js` to make it selectable. |
| New panel | `PANEL_REGISTRY` entry in `engine/web/js/ui/scale-registry/panel-registry.js` + `<div class="panel" id="panel-X">` stub in `engine/web/index.html` + `engine/web/js/ui/panels/X-panel/{component,template}.js` + `engine/web/css/ui/panels/X-panel.css` + `<link>` in `index.html` + re-export in `panels/index.js` + `initXPanel({...})` call in `app_dag.js`. |
| Verify manifest row | 1) `engine/web/data/measurements.json` — add measurement with citation + URL. 2) `_ftd_rows_from_constants()` in `scripts/proofs/build_verify_manifest.py` — add matching FTD row (pick a tier). 3) Run `python -m scripts.proofs.build_verify_manifest` — regenerate the manifest. 4) Commit all three. |
| FAQ entry | `engine/web/js/ui/components/faq/data.js` — add the entry under the correct section with all four required keys (`problem`, `mainstreamStruggle`, `ftdAngle`, `stillOpen`); every `ftdAngle` item needs an epistemic tag. |
| Scene control | `engine/web/js/ui/panels/scene-panel/adapter.js` setter + `template.js` row + `component.js` `CONTROL_META` entry — all three in one commit. |
| Viewport-overlay toggle | `engine/web/js/scales/scale0/ui/overlays/template.js` + binding handler in `scales/scale0/ui/bindings.js` + CSS in `css/ui/components/viewport-overlays.css` if a new style is needed. |
| Physics constant | 1) `scripts/constants.py` — define. 2) `engine/web/js/constants.js` — mirror. 3) `engine/include/ftd/ontic.h` — mirror. 4) Run `constants-sentinel` agent to verify no drift. 5) Update `docs/SPEC_FTD.md` if the value is in any public-facing list. |
| Rename a physics constant | Same three mirrors as above + grep `engine/tests/**` and `engine/web/tests/**` for the old name + update `docs/theory/01_reference/REF_SYMBOL_GLOSSARY.md`. |
| Theory document | `docs/theory/<NN>_<category>/` — use the correct semantic prefix (`DERIV_`, `FOUND_`, `AUDIT_`, `EXPLR_`, `REF_`). Add to `docs/theory/META_INDEX.md`. Apply epistemic tags correctly — see H3. |
| Playwright spec | `engine/web/tests/<feature>.spec.js`. Follow the pattern of `faq.spec.js`: `bootShell(page)` helper, wait for `data-shell-ready`, click tab via `page.evaluate(...)` to bypass pointer-event hit-testing. |
| Python proof | `scripts/proofs/<proof_name>.py`. If it produces a value that flows into the Verify panel, expose it via `_ftd_rows_from_constants` in `build_verify_manifest.py`. |

### 1.3 Module boundaries

These are the don't-reach-into rules. Violating any of them is a
code-review flag.

- **Panels never import from the retired `js/verification/` tree.**
  That tree was deleted; any re-import means the commit missed an
  update.
- **Content renderers always wrap body text as
  `renderMathInHtml(escapeHtml(x))`; attribute values stay bare
  `escapeHtml`.** Inverted order (render-then-escape) kills the math
  rendering; skipping `escapeHtml` on attributes opens injection
  holes.
- **C++ never writes to JS UI state.** Cross-boundary flow is JS
  pulling from the bridge (`bridge.getDiagnostics()`), never the
  other way.
- **JS never hardcodes physics constants.** Always `import` from
  `engine/web/js/constants.js`. If a value isn't there, add it
  (which means `scripts/constants.py` gets it first).
- **`SceneAdapter` is the only JS file outside `viewport.js` that
  imports `three`.** Other panels / components that need Three.js
  behavior go through the adapter or add methods to `Viewport`.
- **Tests never mock the bridge if they can help it.** Playwright
  specs use the real WASM bridge; pytest uses real `constants.py`.
  Mocks are a warning signal that the test is testing wiring, not
  physics.

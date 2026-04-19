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

---

## Part 2 — Hazards and Recipes

### 2.1 Hazards

Each hazard has three lines: **Symptom** (what you see when you've
hit it), **Cause** (the project invariant being violated), **Fix**
(the exact corrective action). Linked to a recent commit so you can
read the pattern in context.

#### H1 — Panel-registry ↔ DOM-stub contract

- **Symptom.** Your new tab doesn't appear in the tab bar, or the
  tab appears but clicking it shows an empty panel. Playwright gets
  `document.getElementById('panel-X')` returning `null`.
- **Cause.** Adding a panel requires THREE things to line up:
  (1) an entry in `PANEL_REGISTRY` in
  `engine/web/js/ui/scale-registry/panel-registry.js`;
  (2) a pre-rendered stub `<div class="panel" id="panel-X"></div>`
  inside `#panel-area` in `engine/web/index.html`;
  (3) a wrapper component that populates the stub at init time.
  Missing any one silently drops the panel.
- **Fix.** All three in the same commit. Validation in
  `AppShell.init` warns `[ui-shell] Panel registry validation
  errors: panel-X in DOM but not registry` or the reverse — read
  the browser console on first load. See commit `966e684` (Scene
  panel) for a canonical example.

#### H2 — renderMathInHtml(escapeHtml(x)) rule

- **Symptom.** Math displays as literal `\(\alpha\)` in the rendered
  UI, or HTML tags leak through as clickable text.
- **Cause.** Every user-text body position must be wrapped
  `renderMathInHtml(escapeHtml(x))`. Attribute values must stay
  bare `escapeHtml(x)` — don't wrap attributes with the math
  renderer because the delimiter regex runs on the entire string,
  not just visible content.
- **Fix.** In each renderer (`faq/reader.js`, `knowledge-base/reader.js`,
  `verify-panel/row.js`, `tooltips/component.js`,
  `lagrangian-panel/term-row.js`), every `${escapeHtml(x)}` in a
  body position becomes `${renderMathInHtml(escapeHtml(x))}`. The
  Playwright coverage spec
  `engine/web/tests/math-formatting.spec.js` asserts no raw `\(` /
  `\[` strings leak.

#### H3 — Epistemic-tag discipline

- **Symptom.** A claim in theory docs, manuscript, Verify rows, or
  FAQ entries gets reviewer pushback for overclaim. The pattern:
  "SM formula with FTD numbers" dressed up as a THEOREM.
- **Cause.** The tag taxonomy (AXIOM / THEOREM / SELECTION /
  PARAMETRIC / CONJECTURE / IMPOSED / EMERGENT / OPEN) maps to
  review expectations. Mislabeling is the #1 overclaim pattern the
  project audits against — see `AUDIT_EPISTEMIC_AUDIT.md` and
  `CLAUDE.md` "Epistemic Discipline".
- **Fix.** Run the tag check on every new claim: **if the formula
  is standard-model and the inputs are FTD values, the tag is
  PARAMETRIC, not THEOREM**. THEOREM requires a proof from
  postulates (D=3, ternary states, varpi). SELECTION requires a
  consistency argument. CONJECTURE is acceptable but must be
  labeled as such. See commit `6ca091f` for a recent downgrade
  example (Born rule and dark-matter 17/27).

#### H4 — Constants single source of truth

- **Symptom.** α value, `N_c`, or a mass derivation gives different
  numbers in Python proofs vs. C++ engine vs. JS dashboard.
- **Cause.** `scripts/constants.py` is canonical. It has two
  mirrors: `engine/web/js/constants.js` and
  `engine/include/ftd/ontic.h`. Drift between them is a
  constants-sentinel-level bug.
- **Fix.** Always edit `scripts/constants.py` first, then
  regenerate the Verify manifest
  (`python -m scripts.proofs.build_verify_manifest`), then
  hand-mirror into `js/constants.js` and `ontic.h`. Finish with
  the `constants-sentinel` agent to verify zero drift.

#### H5 — Cross-renderer scope

- **Symptom.** A new Scene / camera / overlay feature works on
  Scale 0 but silently does nothing on Scale 5 (cosmic).
- **Cause.** `Viewport` serves Scales 0-3. Scale 4 is
  `PlanetaryRenderer`, Scale 5 is `CosmicRenderer`, Scale 11 uses
  `Viewport` with forced post-processing overrides. Each has its
  own camera, scene graph, and lifecycle.
- **Fix.** Declare the scale bucket in the panel-registry entry
  (`scales: ['0','1','2','3']`) and check against it before
  shipping. If you need cross-scale reach, write a `SceneAdapter`-
  style shim that dispatches on `engineMode`. Current Scene panel
  is scoped to Viewport-only as a deliberate first-pass.

#### H6 — System-clock / date fixtures

- **Symptom.** A test that passed yesterday fails today with a
  date-comparison assertion, or the Verify panel's build stamp
  reads tomorrow.
- **Cause.** Assertions on CODATA 2022 dates, spec-file timestamps,
  or `build_stamp.timestamp` are clock-sensitive. The manifest
  regenerates `datetime.now(timezone.utc).isoformat()` each build.
- **Fix.** Use stable `last_checked` / `date` fields in data files,
  not `datetime.now()`. In Playwright tests, assert that the
  timestamp *exists and matches a format*, not that it equals a
  specific value. In Python tests, freeze the clock or compare
  against a date range.

#### H7 — KaTeX CDN fallback

- **Symptom.** `\(\alpha\)` renders as raw text after an offline
  deploy or a CDN outage.
- **Cause.** KaTeX is loaded via CDN (`cdn.jsdelivr.net`). If
  `window.katex` is absent, `renderMathInHtml` leaves the LaTeX
  source visible — a graceful degrade, but not what you see in the
  usual dev loop.
- **Fix.** Acceptable fallback behavior — no code change needed
  most of the time. Tests should assert **presence of `.katex`
  elements**, not absence of `\(...\)` delimiters (the coverage
  spec already does this). If you need offline reliability, host
  KaTeX locally in `js/ui/charts/vendor/` and update the `<link>`
  / `<script>` in `index.html`.

#### H8 — Scale-gated toggles

- **Symptom.** A toggle shows on the wrong scale, or it shows on
  every scale when it should be Scale-0-only.
- **Cause.** Two overlapping mechanisms:
  (1) `scales: [...]` on the `PANEL_REGISTRY` entry gates the tab.
  (2) `.scaleN-only` CSS classes inside panels hide / show
  elements based on the active `#app[data-active-scale="N"]`.
  Using both on the same toggle leads to double-gating that breaks
  in subtle ways.
- **Fix.** Pick one. Scale-level gating goes on the registry.
  Sub-scale gating (e.g. "this toggle only in Scale 0 with flux
  scenarios") uses `.scale0-only` CSS. Never mix.

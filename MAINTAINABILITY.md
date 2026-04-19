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

### 2.2 Recipes

Each recipe is a checklist: files touched, steps, verification.
Deliberately terse — more detail in the linked examples.

#### R1 — Add a Scale 0 diagnostic row

**Files:** `engine/web/js/ui/panels/diagnostics-panel/descriptors/scale0.js` + producer writing to `hub.s0.diag` + `engine/web/docs/TELEMETRY_CATALOG_SCALE0.md`.

**Steps:**
1. Add a row object to the appropriate section in the descriptor:
   `{ id: 'new-metric', label: 'New metric', unit: 'E*', source: 's0.diag.newMetric', trend: 'newMetric' }`.
2. In the producer (`engine/web/js/scales/scale0/controller.js` or its `populateDiagnostics` path), assign `hub.s0.diag.newMetric = value` each tick.
3. If you want a sparkline, add a `RingBuffer` in `engine/web/js/telemetry-hub.js` and push to it in the producer.
4. Add a catalog row to `TELEMETRY_CATALOG_SCALE0.md` under the right section.

**Verify:** `cd engine/web/tests && npx playwright test scales.spec.js --reporter=list`. Open the Diagnostics tab in a running preview and confirm the row shows a non-dash value under any scenario.

#### R2 — Add a Scale 0 chart series

**Files:** `engine/web/js/ui/panels/charts-panel/descriptors/scale0.js` + buffer in `telemetry-hub.js`.

**Steps:**
1. Add `{ key: 'newSeries', label: 'New', color: 'var(--chart-new, #…)', buffer: 'newBuffer' }` to the `series` array of the relevant chart.
2. Ensure `hub.newBuffer = new RingBuffer(500)` exists.
3. Push values to the buffer from the producer.

**Verify:** `cd engine/web/tests && npx playwright test scales.spec.js --reporter=list`. Open Charts tab, confirm the series appears in the legend and draws.

#### R3 — Add a Scale 0 scenario

**Files:** `engine/web/js/config/scenarios.js` + `engine/web/js/scales/scale0/scenario-registry.js`.

**Steps:**
1. Add a scenario block to `scenarios.js` under `SCALE0_SCENARIOS` with `id`, `title`, `desc` (promote math to LaTeX per H2), and any seed metadata.
2. Register it in `scenario-registry.js` so `populateScale0ScenarioSelect` picks it up.
3. Ensure the bridge supports the scenario id (`bridge.setupScenario(id)` in `wasm-bridge-dag.js`) — add a branch if new.

**Verify:** reload, select the scenario from the dropdown, run it, confirm no bridge errors in console. `grep scenarios.js -nE '<your-id>'` returns exactly the expected hits.

#### R4 — Add a new panel

**Files:** registry + stub + component + template + CSS + `<link>` + re-export + app_dag wiring.

**Steps:**
1. Add descriptor to `PANEL_REGISTRY` in `engine/web/js/ui/scale-registry/panel-registry.js` with `{id, label, icon, scales}`.
2. Add stub `<div class="panel" id="panel-X"></div>` in `engine/web/index.html` inside `#panel-area`.
3. Create `engine/web/js/ui/panels/X-panel/{component,template}.js`. Template returns INNER content (no outer wrapper). Component queries `#panel-X` and sets `innerHTML`.
4. Create `engine/web/css/ui/panels/X-panel.css` and add `<link rel="stylesheet" href="css/ui/panels/X-panel.css?v=1">` to `index.html`.
5. Re-export `initXPanel` from `engine/web/js/ui/panels/index.js`.
6. Call `initXPanel({...})` from `app_dag.js` in the panel-init block.

**Verify:** `cd engine/web/tests && npx playwright test scales.spec.js --reporter=list`. Scene panel commit `966e684` is the canonical reference.

#### R5 — Add a Verify manifest row

**Files:** `engine/web/data/measurements.json` + `scripts/proofs/build_verify_manifest.py` + regenerated manifest JSON.

**Steps:**
1. Add the measurement to `measurements.json` with `id`, `quantity`, `sector`, `value`, `sigma`, `units`, `source`, `url`, `date`.
2. Add a matching FTD row in `_ftd_rows_from_constants()` in `build_verify_manifest.py`. Pick a tier:
   - `hard` — derive from first principles; requires `inputs_used` array naming every input.
   - `parametric` — SM formula with FTD inputs; requires `formula_source: 'SM'` and `ftd_inputs`.
   - `unpredicted` — FTD makes no claim; must NOT carry `ftd_value`.
3. Run `python -m scripts.proofs.build_verify_manifest`. Commit the updated `engine/web/data/verify-manifest.json`.

**Verify:** `cd engine/web/tests && npx playwright test verify-panel.spec.js --reporter=list`. Visit Verify tab, confirm the row renders with appropriate tier styling.

#### R6 — Add a FAQ entry

**Files:** `engine/web/js/ui/components/faq/data.js`.

**Steps:**
1. Add the entry object to the appropriate `FAQ_SECTIONS[i].entries` array.
2. Required keys (validator enforces): `id`, `question`, `problem`, `mainstreamStruggle`, `ftdAngle`, `stillOpen`, optional `shortQuestion`, `theoryRefs`.
3. Every `ftdAngle` item is `{tag, text}` where `tag` must be one of `THEOREM` / `SELECTION` / `PARAMETRIC` / `CONJECTURE` / `OPEN`. Apply H3 discipline.

**Verify:** `cd engine/web/tests && npx playwright test faq.spec.js --reporter=list`. Confirm 6/6 pass including the "no claim-making verbs" assertion.

#### R7 — Add a Scene control

**Files:** `engine/web/js/ui/panels/scene-panel/{adapter,template,component}.js`.

**Steps:**
1. Add a `setXxx(value)` method to `SceneAdapter`. Also add an entry to `DEFAULTS` with the default value.
2. Add a DOM row to `template.js` with `data-scene-control="xxx"` and appropriate `<input>` type (range / color / checkbox).
3. Add `CONTROL_META.xxx = {type, fmt, apply: 'setXxx'}` in `component.js`.

**Verify:** `cd engine/web/tests && npx playwright test scene-panel.spec.js --reporter=list`. Confirm total-control count updates and reload-persistence test still passes.

#### R8 — Add a viewport-overlay toggle

**Files:** `engine/web/js/scales/scale0/ui/overlays/template.js` + `scales/scale0/ui/bindings.js` + `css/ui/components/viewport-overlays.css` (if styling needed).

**Steps:**
1. Add the button to the template with `id="toggle-newthing"` and the appropriate column.
2. Add the handler in `bindings.js` `FIELD_TOGGLE_BINDINGS` with a `fieldKey` mapping to the state-store key.
3. Push the render logic in `viewport-adapter.js` if the toggle shows a new overlay type.

**Verify:** toggle the button in a running preview, confirm overlay appears. No automated test currently covers this — add one if the toggle carries non-trivial behavior.

#### R9 — Add a physics constant

**Files:** `scripts/constants.py` + `engine/web/js/constants.js` + `engine/include/ftd/ontic.h` + possibly `docs/SPEC_FTD.md`.

**Steps:**
1. Define in `scripts/constants.py` with a docstring tagging its epistemic status and unit.
2. Mirror in `engine/web/js/constants.js` with the exact same value and a matching comment.
3. Mirror in `engine/include/ftd/ontic.h` as `constexpr`.
4. Run `constants-sentinel` agent to verify no drift.
5. If the constant is in any public-facing list in `docs/SPEC_FTD.md`, update that too.

**Verify:** `python -m pytest scripts/tests/ -k constants` — constants tests should still pass. `cd engine/build && ctest --output-on-failure -C Release -R ontic` — C++ side should still compile and match.

#### R10 — Rename a physics constant

**Files:** the three mirrors from R9 + all call-sites across Python / C++ / JS / tests.

**Steps:**
1. Do R9 steps 1-3 with the new name.
2. Grep all call-sites: `grep -r OLD_NAME scripts/ engine/ docs/`.
3. Replace in each, minding JSON keys in `verify-manifest.json` and scenario metadata.
4. Update `docs/theory/01_reference/REF_SYMBOL_GLOSSARY.md` if the symbol is listed.
5. Keep the old name as an `@deprecated` alias for one release cycle if external consumers might reference it.

**Verify:** full test suite — `python -m pytest scripts/tests/`, `cd engine/build && ctest -C Release`, `cd engine/web/tests && npx playwright test`. All must be green.

#### R11 — Add a Playwright spec

**Files:** `engine/web/tests/<feature>.spec.js`.

**Steps:**
1. Copy `engine/web/tests/faq.spec.js` as a template.
2. Use the `bootShell(page)` helper: `await page.goto('/')`, wait for `data-shell-ready === 'true'`, wait for the loading overlay to hide.
3. Click tabs via `page.evaluate(() => document.querySelector('#tab-bar .tab[data-panel="X"]')?.click())` — not `locator.click()` — to bypass pointer-event hit-testing on the tab-strip scroll container.
4. Prefer stable id selectors (`#panel-X`, `#btn-Y`) over class selectors.

**Verify:** `cd engine/web/tests && npx playwright test <feature>.spec.js --reporter=list`. Also run the full suite (`npx playwright test --reporter=list`) to confirm no regression; baseline is 122 passing as of commit `966e684`.

#### R12 — Add a Python proof

**Files:** `scripts/proofs/<proof_name>.py` + unit tests under `scripts/tests/` if the proof is non-trivial.

**Steps:**
1. Import canonical values from `scripts.constants`. Never hardcode `α = 1/137.035999...`.
2. Emit a structured result at the end: `print(f"RESULT: {name} = {value:.12f}")` so other scripts can grep it.
3. If the proof produces a Verify-manifest-facing value, add a row via R5.
4. Add to `docs/theory/META_INDEX.md` if it's new derivation work worth linking.

**Verify:** `python -m scripts.proofs.<proof_name>`. Exit code 0 and result line present.

#### R13 — Add a theory doc

**Files:** `docs/theory/<NN>_<category>/<PREFIX>_<NAME>.md` + `docs/theory/META_INDEX.md`.

**Steps:**
1. Pick the right category (01_reference, 02_foundations, 03_derivations, …) and prefix (`DERIV_`, `FOUND_`, `AUDIT_`, `EXPLR_`, `REF_`, `THEOREM_`, `SPEC_`).
2. Apply epistemic tags on every claim per H3.
3. Cross-reference related docs with relative-path links.
4. Update `docs/theory/META_INDEX.md` in the same commit.

**Verify:** `grep -c "\[THEOREM\]\|\[SELECTION\]\|\[PARAMETRIC\]\|\[CONJECTURE\]\|\[OPEN\]" docs/theory/<path>/<name>.md` — expect non-zero for any doc that makes claims. Epistemic-auditor agent can vet the tag choices.

#### R14 — Update CHANGELOG

**Files:** `CHANGELOG.md`.

**Steps:**
1. Add a section under the next unreleased version describing the change in user-facing terms.
2. Group by category: Added / Changed / Fixed / Deprecated / Removed.
3. Reference the commit hash(es) if the change spans multiple commits.

**Verify:** eyeball — changelog entries don't have automated verification; adjacent project audits would catch omissions.

#### R15 — Commit workflow

**Files:** whatever you're changing.

**Steps:**
1. `git status --short` — confirm the only files staged are the ones your task touched.
2. Craft a commit message: short imperative subject line (≤ 72 chars), blank line, body explaining the *why* plus verification evidence (test counts, review outcomes).
3. End the body with the standard `Co-Authored-By` trailer if an AI agent wrote significant portions of the change.
4. Prefer many small commits over one giant commit when changes are separable.
5. Never force-push to `main`. Ask before any destructive git operation.

**Verify:** `git log --oneline -1` — subject line parses cleanly; `git show HEAD --stat` — scope matches intent.

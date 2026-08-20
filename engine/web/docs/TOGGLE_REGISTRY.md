# Web Toggle Registry (canonical map)

**Ticket:** W3-1 · **Scope:** the browser dashboard (`engine/web/`) toggle surface only.
**Companion test:** [`engine/web/tests/toggle-coverage.spec.js`](../tests/toggle-coverage.spec.js) (W3-2) — proves every user-facing toggle is wired (clicking it changes state) against a live Chromium.

> **Why this doc exists.** A web audit flagged a long list of Scale-0 field toggles as "orphans / dead code." That audit grepped `index.html` for `id="toggle-..."` literals and found **zero** — so it concluded the buttons did not exist. They *do* exist; they are **dynamically rendered at runtime** (see [Source 3](#source-3--dom-template-dynamically-rendered)), not authored into static HTML. This registry is the verify-don't-assume answer: it maps every toggle to its real source-of-truth, and the companion spec proves wiring with a live click test rather than a grep. **Result: 0 orphans, 0 broken toggles — every Scale-0 field toggle is wired (see [Findings](#findings)).**

---

## TL;DR

| Scale | Toggle family | Count | Wired? |
|------|----------------|-------|--------|
| Scale 0 (Lattice) | **Field/overlay** toggles (`fieldFlags`, render-side) | **32** | All 32 verified by live click test |
| Scale 0 (Lattice) | **Physics-term** toggles (`bridge.setToggle`, sim-side) | 18 (+scenario overrides) | Wired via `SCALE0_TOGGLES` → bridge |
| Scale 0 (Lattice) | Viewport volume/slice (not in `fieldFlags`) | 2 | Wired via `bindings.js` (`setFluxVolumeVisible` / `setFluxSliceVisible`) |
| Scale 2/3 (Atoms/Molecules) | Atom-engine physics toggles | 11 | Wired via `SCALE2_TOGGLES` → bridge |

The **field/overlay toggles are the W3-2 deliverable's focus** — they are the family the audit mis-flagged, and the family with a clean DOM-id  state-key mapping that a click test can exercise exhaustively.

---

## The three scattered sources (the consolidation finding)

Web toggle state-of-truth is currently split across **three files** with **three different dispatch mechanisms**. None is wrong, but the split is why a single literal grep mis-mapped the surface.

### Source 1 — physics-term toggles → the engine bridge
**File:** [`engine/web/js/config/toggles.js`](../js/config/toggles.js)
`SCALE0_TOGGLES`, `SCALE2_TOGGLES`, and the `*_SCENARIO_OVERRIDES` tables. Each row is `[toggleKey, defaultValue, domElementId]`. These drive `bridge.setToggle(key, value)` — they change the **simulation** (which physics terms run). They mirror the C++ `TermToggles` struct field names. DOM ids here use the `t-*` prefix (e.g. `t-wave`, `t-gravity`) and live in the physics-toggles control card, not the overlay panel.

### Source 2 — field/overlay state store (the render-side flags)
**File:** [`engine/web/js/scales/scale0/state/store.js`](../js/scales/scale0/state/store.js)
`FIELD_TOGGLE_KEYS` (the canonical key list) + `fieldFlags` bag + `setFieldToggle(key, value)` mutator. These are **render-only** flags — they decide which overlay (E-field arrows, Poynting vectors, ψ², horizon isosurface, …) is drawn. They do **not** go to the bridge; they gate viewport overlay visibility. `createFieldFlags()` derives the all-off defaults bag *programmatically* from `FIELD_TOGGLE_KEYS`, so the key list is the single source of truth for this family (a prior hand-maintained mirror drifted; that hazard is now removed in-code).

### Source 3 — DOM template (dynamically rendered)
**File:** [`engine/web/js/scales/scale0/ui/overlays/template.js`](../js/scales/scale0/ui/overlays/template.js)
Builds a `<div id="viewport-overlay">` via `document.createElement` and an HTML string containing **34 `<button class="view-toggle ...">` elements** (32 `field-toggle` + 2 volume/slice). **This is the audit's blind spot:** these buttons exist only after this template runs at Scale-0 boot — `index.html` contains none of them.
The **DOM-id  state-key mapping** is declared in [`engine/web/js/scales/scale0/ui/dom.js`](../js/scales/scale0/ui/dom.js) as `FIELD_TOGGLE_BINDINGS` (32 `[buttonId, fieldKey]` pairs). The **click wiring** is in [`engine/web/js/scales/scale0/ui/bindings.js`](../js/scales/scale0/ui/bindings.js): for each `[buttonId, fieldKey]` it attaches a click handler that calls `setToggleState(buttonId, fieldKey, on)`, which `setButtonActive(...)` + `setFieldToggle(fieldKey, on)` + syncs viewport overlay visibility. The `on` value is `!readButtonActive(buttonId)` — i.e. the click flips relative to the button's current `.active` class.

> **Cross-reference (not duplicated here): the C++ side.** The simulation's authoritative toggle table is `TOGGLE_SPECS[]` in [`engine/include/ftd/term_toggles.h`](../../include/ftd/term_toggles.h) (27 boolean toggles + 5 non-bool config fields, table-driven per ADR-0013). Source 1 (`SCALE0_TOGGLES`) is the **JS whitelist subset** of that table that the dashboard surfaces; `config/toggles.js` documents in-line which `TermToggles` fields are deliberately omitted (research controls: `triad_binding`, `pair_production`, `latency_field`, `exact_dual_gauss`, `emergent_forces`, `langevin`, `langevin_site_filter`, `bcc_stencil`, `strict_validation`). This doc maps the **web** surface; for the engine-truth table read `term_toggles.h`.

### Recommendation (documentation only — do **not** implement here)
The field/overlay family is already in good shape: `FIELD_TOGGLE_KEYS` (store) and `FIELD_TOGGLE_BINDINGS` (dom) are kept in **exact 1:1 lockstep** (verified: 32 = 32, no orphan either way). The remaining fragmentation is the **three-prefix / three-dispatch** split:
- Field toggles use the `toggle-*` id prefix + `setFieldToggle` (render store).
- Physics toggles use the `t-*` id prefix + `bridge.setToggle` (sim bridge).
- The idkey pairing for field toggles lives in `dom.js`, but the button **HTML** lives in `template.js` and the key **list** lives in `store.js` — three files must stay consistent for one family.

A future (separate-ticket) consolidation could co-locate the field-toggle triplet — emit `template.js` buttons *from* `FIELD_TOGGLE_BINDINGS` rather than hand-authoring 32 `<button>` lines — so adding an overlay is one edit (a key in `store.js` + a binding row) instead of three. That would also make the audit's grep blind-spot structurally impossible to recur. **No behavior change is made by this doc or its companion test.**

---

## Scale 0 — Field / overlay toggles (the 32)

DOM id  state key  column, all sourced from `dom.js::FIELD_TOGGLE_BINDINGS` (idkey), `store.js::FIELD_TOGGLE_KEYS` (key canon), `template.js` (button HTML), `presets.js::COL_TO_TOGGLES` (column grouping). **Source of truth = `store.js` key list; rendered + wired as described above.** "Wired" is asserted by the companion live click test, not by this table.

| # | DOM id | State key (`fieldFlags.*`) | Column | Wired |
|---|--------|----------------------------|--------|-------|
| 1 | `#toggle-e-field` | `showEField` | fields | ✓ |
| 2 | `#toggle-b-field` | `showBField` | fields | ✓ |
| 3 | `#toggle-poynting` | `showPoynting` | fields | ✓ |
| 4 | `#toggle-div-field` | `showDivField` | volume | ✓ |
| 5 | `#toggle-flux-lines` | `showFluxLines` | volume | ✓ |
| 6 | `#toggle-force-em` | `showForceEM` | forces | ✓ |
| 7 | `#toggle-force-gravity` | `showForceGravity` | forces | ✓ |
| 8 | `#toggle-force-strong` | `showForceStrong` | forces | ✓ |
| 9 | `#toggle-force-weak` | `showForceWeak` | forces | ✓ |
| 10 | `#toggle-dual-substrate` | `showDualSubstrate` | phenomena | ✓ |
| 11 | `#toggle-chirality` | `showChirality` | phenomena | ✓ |
| 12 | `#toggle-dark-halo` | `showDarkMatterHalo` | phenomena | ✓ |
| 13 | `#toggle-damping-zones` | `showDampingZones` | phenomena | ✓ |
| 14 | `#toggle-genesis-iso` | `showGenesisIsosurface` | phenomena | ✓ |
| 15 | `#toggle-confinement` | `showConfinement` | phenomena | ✓ |
| 16 | `#toggle-psi-squared` | `showPsiSquared` | quantum | ✓ |
| 17 | `#toggle-phase` | `showPhase` | quantum | ✓ |
| 18 | `#toggle-lagrangian-density` | `showLagrangianDensity` | quantum | ✓ |
| 19 | `#toggle-entropy-density` | `showEntropyDensity` | quantum | ✓ |
| 20 | `#toggle-grav-potential` | `showGravPotential` | topology | ✓ |
| 21 | `#toggle-em-energy` | `showEmEnergy` | topology | ✓ |
| 22 | `#toggle-charge-density` | `showChargeDensity` | topology | ✓ |
| 23 | `#toggle-vorticity` | `showVorticity` | topology | ✓ |
| 24 | `#toggle-horizon` | `showHorizon` | phenomena | ✓ |
| 25 | `#toggle-e-pressure` | `showEPressure` | stress-energy | ✓ |
| 26 | `#toggle-b-pressure` | `showBPressure` | stress-energy | ✓ |

**id  key derivation rule:** the key is the kebab-cased id with the `toggle-` prefix stripped and `show` prepended in camelCase — *almost*. It is **not** algorithmically derivable in every case (`toggle-genesis-iso → showGenesisIsosurface`, `toggle-dark-halo → showDarkMatterHalo`, `toggle-div-field → showDivField`). The authoritative mapping is therefore the **explicit `FIELD_TOGGLE_BINDINGS` table in `dom.js`** — the companion test reads that table directly rather than reconstructing ids, so it never drifts from the code.

### State-only field flags (no DOM button)
**None.** Every one of the 32 `FIELD_TOGGLE_KEYS` has a corresponding `FIELD_TOGGLE_BINDINGS` row and a rendered button. (Programmatically verified: `FIELD_TOGGLE_KEYS \ keys(FIELD_TOGGLE_BINDINGS) = ∅`.) The companion test keeps a state-only branch anyway, so if a future key is added without a button the test self-detects it and exercises it via `setFieldToggle` directly.

---

## Scale 0 — Physics-term toggles (sim-side, `SCALE0_TOGGLES`)

These are render-independent: they switch which physics terms the engine integrates, via `bridge.setToggle(key, value)`. DOM ids use the `t-*` prefix and live in the physics-toggles control card (separate from the overlay panel). Defaults and scenario overrides are in `config/toggles.js`. Not the focus of the W3-2 click test (that test targets the field/overlay family), but listed here for the canonical map.

| DOM id | Toggle key | Default | C++ field (`TermToggles::*`) |
|--------|-----------|---------|------------------------------|
| `t-wave` | `wave_propagation` | on | `wave_propagation` |
| `t-coupling` | `coupling` | on | `coupling` |
| `t-damping` | `damping` | on | `damping` |
| `t-genesis` | `genesis` | on | `genesis` |
| `t-gauss` | `gauss_projection` | on | `gauss_projection` |
| `t-forces` | `forces` | on | `forces` |
| `t-gravity` | `gravity` | **off** | `gravity` |
| `t-movement` | `movement` | on | `movement` |
| `t-poisson` | `poisson_coulomb` | on | `poisson_coulomb` |
| `t-lorentz` | `lorentz_force` | **off** | `lorentz_force` |
| `t-selective` | `selective_damping` | on | `selective_damping` |
| `t-larmor` | `larmor_radiation` | **off** | `larmor_radiation` |
| `t-dual` | `dual_substrate` | **off** | `dual_substrate` |
| `t-confinement` | `confinement` | **off** | `confinement` (linear colour string; requires `color_forces`) |
| `t-color-forces` | `color_forces` | **off** | `color_forces` |
| `t-strong-force` | `strong_force` | **off** | `strong_force` (GPU-only) |
| `t-exchange` | `exchange_force` | **off** | `exchange_force` (GPU-only) |
| `t-weak` | `weak_transmutation` | **off** | `weak_transmutation` |

---

## Scale 0 — Viewport volume / slice (not field flags)

Buttons in the overlay panel (`template.js`) that are **not** `fieldFlags` keys and are wired separately in `bindings.js` to the viewport adapter:

| DOM id | Action | Wiring |
|--------|--------|--------|
| `#toggle-flux-volume` | show/hide flux volume render | `bindings.js` → `viewportAdapter.setFluxVolumeVisible(on)` |
| `#toggle-flux-slice` | show/hide flux slice overlay (all enabled mid-planes) | `bindings.js` → `viewportAdapter.setFluxSliceVisible(on)` |
| `#flux-slice-axis-xy` | enable/disable the xy mid-plane (z=L/2) of the slice | `bindings.js` → `viewportAdapter.setFluxSliceAxisEnabled(2, on)` |
| `#flux-slice-axis-xz` | enable/disable the xz mid-plane (y=L/2) of the slice | `bindings.js` → `viewportAdapter.setFluxSliceAxisEnabled(1, on)` |
| `#flux-slice-axis-yz` | enable/disable the yz mid-plane (x=L/2) of the slice | `bindings.js` → `viewportAdapter.setFluxSliceAxisEnabled(0, on)` |

`#toggle-flux-volume` / `#toggle-flux-slice` participate in the `volume` column's clear-button / badge accounting (`COL_TO_TOGGLES.volume`) but use button `.active` class as their only state (no store key). The three `#flux-slice-axis-*` buttons are sub-modifiers of the slice (default all-on) and are deliberately **excluded** from `COL_TO_TOGGLES` so they neither inflate the column badge nor get swept by the column clear. The Flux Volume card's Opacity / Shape / Point Size / Threshold controls (`wire.js::wireFluxVolume`) fan out to the slice mesh too (`viewport.setFluxSlice{Opacity,Shape,PointScale,Threshold}`). The W3-2 test scopes to `fieldFlags`-backed toggles and so does not assert these; `flux-slice-axes.spec.js` covers the slice + axis buttons + shared controls.

---

## Scale 2/3 — Atom/molecule physics toggles (`SCALE2_TOGGLES`)

Sim-side toggles for the atom engine (`AtomToggles`), dispatched via the Scale-2 bridge. DOM ids use the `aeSet*` convention. Listed for the canonical map; out of scope for the Scale-0 W3-2 test.

| DOM id | Toggle key | Default |
|--------|-----------|---------|
| `aeSetIonic` | `ae-ionic` | on |
| `aeSetVdw` | `ae-vdw` | on |
| `aeSetBondsForce` | `ae-bonds-force` | on |
| `aeSetBonding` | `ae-bonding` | on |
| `aeSetDamping` | `ae-damping` | off |
| `aeSetSpeedLimit` | `ae-speed-limit` | on |
| `aeSetHBonds` | `ae-hbonds` | off |
| `aeSetAngleStrain` | `ae-angle` | off |
| `aeSetDipoleDipole` | `ae-dipole` | off |
| `aeSetThermostat` | `ae-thermostat` | off |
| `aeSetElectronegativity` | `ae-electronegativity` | off |

---

## Findings

1. **An earlier audit over-counted Scale-0 field toggles as orphans.** Root cause: the buttons are dynamically rendered by `overlays/template.js` at boot, not authored in `index.html`; a literal grep of `index.html` returns zero matches. They are real, rendered, and wired.
2. **All 32 Scale-0 field/overlay toggles are wired.** `FIELD_TOGGLE_KEYS` (store, 32) and `FIELD_TOGGLE_BINDINGS` (dom, 32) are in exact 1:1 lockstep — no orphan button, no buttonless key, no duplicate id. The companion [`toggle-coverage.spec.js`](../tests/toggle-coverage.spec.js) proves each one flips its `fieldFlags` key on click and flips back on second click, with zero real console errors across the full sweep.
3. **Three scattered sources** (physics-term whitelist in `config/toggles.js`; field-flag store in `state/store.js`; button HTML in `ui/overlays/template.js` + idkey map in `ui/dom.js` + click wiring in `ui/bindings.js`). The field-toggle idkey map is already lockstep-verified; the residual fragmentation is the per-family three-file spread, for which a future single-source-render consolidation is recommended (not implemented here).

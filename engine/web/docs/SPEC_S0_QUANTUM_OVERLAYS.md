# Scale 0 Quantum & Physics Overlay Spec

Status: `[SELECTION]` design + implementation spec for new Scale 0 viewport overlays
Version: 1.0 (2026-04-16)
Scope: `engine/web/js/scales/scale0/**`, `engine/web/css/ui/**`

---

## 1. Motivation

The current Scale 0 overlay has 18 toggles covering flux volume, fields (E/B/Poynting/Light), forces (EM/Gravity/Strong/Weak), and phenomena (Dual J / Chirality / DM Halo / Genesis / Damping / Confinement). The audit in the Gemini conversation flagged **17 quantum & physics quantities** that the FTD lattice exposes but that the UI doesn't yet surface.

This spec:

1. Catalogs every proposed overlay with physical definition, formula, implementation notes, and priority tier.
2. Specifies the **file-by-file checklist** to add each overlay (state, bindings, renderer, template, CSS).
3. Ships Tier 1 end-to-end (UI infrastructure + a CPU-side sampler so toggles fire and emit values; full renderer work tagged per toggle).
4. Defers Tier 2/3 to their own tickets, each fully described here.

---

## 2. Architecture recap — how a Scale 0 overlay is wired

An overlay toggle at Scale 0 flows through **six** files. Every new overlay must touch the ones relevant to it:

| # | File | Role |
|---|------|------|
| A | [state/store.js](../js/scales/scale0/state/store.js) | Declares `fieldFlags.<key>`, `FIELD_TOGGLE_KEYS` |
| B | [ui/dom.js](../js/scales/scale0/ui/dom.js) | Maps DOM id → field key via `FIELD_TOGGLE_BINDINGS` |
| C | [ui/overlays/template.js](../js/scales/scale0/ui/overlays/template.js) | Button markup with swatch + tooltip |
| D | [css/ui/scales/scale0/toolbar.css](../css/ui/scales/scale0/toolbar.css) | `.field-swatch-<name>` color chip |
| E | [viewport-adapter.js](../js/scales/scale0/viewport-adapter.js) | Proxies to renderer methods |
| F | [runtime/field-overlays.js](../js/scales/scale0/runtime/field-overlays.js) + renderer | Per-tick data computation → visualization |

---

## 3. Tier 1 — Quantum column (ship now)

**Goal:** give users the canonical QM view (Born density, phase, action, entropy) plus the gravity *potential* that matches the existing gravity force overlay. All five are CPU-computable from the current flux field in one pass.

### 3.1 `|ψ|²` — Born probability density

- **Physics:** probability density = normalized squared flux magnitude. Under the FTD dual-substrate mapping `ψ ≡ J_L + i J_R`, we have `|ψ|² = |J_L|² + |J_R|²`. When dual-substrate is not active, `|ψ|² = |J|²`.
- **Formula (CPU):**
  ```
  for each voxel v: psi2[v] = J_L[v].lenSq() + J_R[v].lenSq()     // if dual
                   psi2[v] = J[v].lenSq()                          // otherwise
  normalize(psi2)  // so integral sums to 1
  ```
- **Visualization:** recolor the Flux Volume point cloud by `|ψ|²` → viridis colormap. Falls back gracefully to magnitude if renderer doesn't implement `updatePsiSquaredField`.
- **Toggle id:** `toggle-psi-squared` · **Field key:** `showPsiSquared` · **Swatch:** viridis gradient (`linear-gradient(90deg, #440154, #21918c, #fde725)`).
- **Label:** `|ψ|²` · **Tooltip:** "Born probability density = |J_L|² + |J_R|² (or |J|² if dual substrate off). Where the particle *is*, probabilistically."

### 3.2 `Phase φ` — complex phase angle

- **Physics:** arg(ψ) ∈ [-π, π]. Reveals interference fringes, double-slit patterns, Aharonov-Bohm phase loops. Requires Dual Substrate to be active (otherwise J is real and φ = 0/π trivially).
- **Formula (CPU):** `phase[v] = atan2(J_R[v].mag(), J_L[v].mag())`.
- **Visualization:** cyclic HSL colormap (hue = phase, saturation = |ψ|). Point cloud recolored.
- **Toggle id:** `toggle-phase` · **Field key:** `showPhase` · **Swatch:** cyclic rainbow (`conic-gradient(red, yellow, lime, cyan, blue, magenta, red)` simplified).
- **Label:** `Phase φ` · **Tooltip:** "Complex phase arg(J_L + i·J_R). Interference fringes, Aharonov-Bohm loops. Requires Dual Substrate."

### 3.3 `ℒ(x)` — Lagrangian density

- **Physics:** integrand of the Lagrangian chart — ℒ = (1/2)|∂_t J|² − (1/2)|∇J|² − V(s,J). Shows spatially **where** action is accumulating. Positive regions are dynamic (kinetic-dominated); negative are potential-dominated.
- **Formula (CPU):**
  ```
  kinetic = 0.5 * dJ_dt.lenSq()
  gradient = 0.5 * (|∂_x J|² + |∂_y J|² + |∂_z J|²)
  coupling = g_c * s * J        // manifestation coupling term
  lagrangian[v] = kinetic - gradient - coupling
  ```
  Uses the same decomposition as [lagrangian.js](../js/lagrangian.js) but per-voxel instead of summed.
- **Visualization:** diverging blue→white→red colormap. Volumetric cloud.
- **Toggle id:** `toggle-lagrangian-density` · **Field key:** `showLagrangianDensity` · **Swatch:** `linear-gradient(90deg, #2166ac, #f7f7f7, #b2182b)`.
- **Label:** `ℒ(x)` · **Tooltip:** "Lagrangian density per voxel. Blue = potential-dominated, red = kinetic-dominated. Spatial view of the action density chart."

### 3.4 `Entropy s(x)` — local Shannon entropy

- **Physics:** Shannon entropy of the ternary state distribution in the 3×3×3 Moore neighborhood of each voxel. High entropy = disorder, low = ordered/crystallized.
- **Formula (CPU):**
  ```
  for each voxel v:
      count[-1], count[0], count[+1] = 0,0,0
      for each of 27 Moore neighbors n: count[s[n]]++
      p_k = count[k] / 27  for k in {-1, 0, +1}
      entropy[v] = -Σ p_k log(p_k)    // max = log(3) ≈ 1.0986
  ```
- **Visualization:** grayscale heatmap (black = ordered, white = maximally disordered).
- **Toggle id:** `toggle-entropy-density` · **Field key:** `showEntropyDensity` · **Swatch:** `linear-gradient(90deg, #000, #fff)`.
- **Label:** `Entropy s` · **Tooltip:** "Local Shannon entropy of state distribution in 3×3×3 Moore neighborhood. White = disordered, black = crystallized."

### 3.5 `Φ(x)` — gravitational potential

- **Physics:** The existing Gravity force overlay shows `−∇Φ`. Users can't see the *wells and peaks* that create the force. Φ is the Newtonian potential from the density gradient.
- **Formula (CPU):** Φ satisfies `∇²Φ = 4πG ρ` where ρ = smoothed |J|². The engine already runs a Poisson solver for the Coulomb potential (when `t-poisson` is on); a parallel gravity-Poisson solver emits Φ.
- **Visualization:** contour-map (equipotential surfaces) OR a volumetric scalar field with a pressure-style colormap (deep blue = well, yellow = peak).
- **Toggle id:** `toggle-grav-potential` · **Field key:** `showGravPotential` · **Swatch:** `linear-gradient(90deg, #000033, #0066cc, #ffff00)`.
- **Label:** `Φ potential` · **Tooltip:** "Gravitational potential Φ — the wells that generate the Gravity force vectors. Deep blue = mass well, yellow = saddle/peak."

### 3.6 Tier 1 checklist

- [x] Pattern documented (this file)
- [ ] **State store** — add 5 keys to `FIELD_TOGGLE_KEYS`, `createFieldFlags`
- [ ] **Bindings** — add 5 `[id, key]` pairs to `FIELD_TOGGLE_BINDINGS`
- [ ] **Overlay template** — new "Quantum" column with 5 buttons + swatches + tooltips
- [ ] **CSS swatches** — add `.field-swatch-{psi-squared,phase,lagrangian,entropy,grav-potential}`
- [ ] **Viewport adapter** — add 5 `NON_FORCE_OVERLAYS` entries + 5 `apply*` methods (no-op tolerant)
- [ ] **Field-overlays runtime** — add CPU-side computation for each (sampling loop, normalization, buffer handoff)
- [ ] **Renderer stubs** — in `mock-bridge.js` / `viewport.js`, add `togglePsiSquaredField`, `updatePsiSquaredField`, etc. (initial implementation: no-op with console probe so the UI toggle fires and is testable)
- [ ] **Verification** — each toggle flips `.active`, sets `fieldFlags.<key>`, and the runtime loop detects it and logs/applies via adapter

---

## 4. Tier 2 — FTD-ontology-specific (next)

These reveal structures unique to FTD and aren't standard QFT.

### 4.1 Master-quadratic domain map

- **Physics:** The master quadratic `x² − kx + 1 = 0` has three regimes: Real (k=16, physics), Degenerate (k≈1.35, measurement boundary), Complex (k=½, reference frame context). Sample `k(x)` per voxel from local |J|/K_C ratio.
- **Visualization:** 3-color map (physics blue / boundary purple / reference frame context gold).
- **Toggle id:** `toggle-master-domain` · **Field key:** `showMasterDomain`.
- **Tooltip:** "Master-quadratic regime per voxel: physics (k=16), measurement boundary (k≈1.35), reference frame context (k=½)."

### 4.2 Dual-substrate coupling `|J_L · J_R|`

- **Physics:** Overlap between left and right chirality. Non-zero regions are where mass arises in the FTD ontology (chiralities must couple to generate manifested particles).
- **Formula:** `coupling[v] = |J_L[v] · J_R[v]|`.
- **Visualization:** volumetric cloud, warm→cool purple gradient.
- **Toggle id:** `toggle-dual-coupling` · **Field key:** `showDualCoupling`.

### 4.3 Ontic-slot index

- **Physics:** Per-voxel classification of which of the 27 Moore-neighborhood sites is "active" (has max |J| contribution). Ties directly to Moore Layer Theorem (SC+FCC+BCC decomposition).
- **Visualization:** 27-color categorical map, grouped by layer (6 face-centered = SC, 12 edge-centered = FCC, 8 corner = BCC, 1 center).
- **Toggle id:** `toggle-ontic-slot` · **Field key:** `showOnticSlot`.

### 4.4 Color charge density

- **Physics:** SU(3) Casimir density per voxel. Three color channels (R/G/B) with the existing `#ff4d33 / #4d80e6 / #4de673` legend.
- **Visualization:** three superimposed isosurfaces or a vector-field of color flux.
- **Toggle id:** `toggle-color-charge` · **Field key:** `showColorCharge`.

### 4.5 Bell / entanglement pair map

- **Physics:** Links between `createEntangledPair()` sites, drawn as glowing connections. Shows non-local correlations without requiring them to act at distance.
- **Visualization:** line segments between paired voxels, opacity = |Bell correlation|.
- **Toggle id:** `toggle-bell-pairs` · **Field key:** `showBellPairs`.

### 4.6 Tier 2 checklist

- [ ] Same six-file extension pattern as Tier 1, applied to 5 toggles
- [ ] Engine-side hooks: expose paired-particle list (bridge already has it from `createEntangledPair`), ontic-slot index (new), color-charge field (existing strong-force solver provides components)
- [ ] New CSS column `.s0-overlay-col-phenomena` becomes 6 items → split into `Phenomena` + `Topology` or merge with FTD in a 5th column

---

## 5. Tier 3 — Diagnostic (specialised)

Short reference. Each is a one-pass CPU computation from existing fields and can be added following the Tier 1 pattern.

| Toggle | Formula | Swatch | Priority |
|---|---|---|---|
| `\|E\|² − \|B\|²` | Lorentz scalar | diverging blue↔red | Medium |
| `E·B` | Lorentz pseudoscalar; CP-violation indicator | cyclic violet↔orange | Medium |
| **Momentum density** | `p = -iℏ∇ψ` (real part from dual substrate) | vector arrows | Medium |
| **Angular momentum** | `L = r × p` | vector arrows, cyan | Low |
| **Temperature T(x)** | Local ⟨KE⟩ over neighborhood | thermal (blue→red) | Medium |
| **Coherence map** | Spatial autocorrelation of J | grayscale alpha | Low |
| **Phase vortices** | ∮∇φ = 2πn loops | point markers at n≠0 | High for QFT pedagogy |
| **Time dilation γ(x)** | `1/√(1 − 2Φ/c²)` from Φ | blue→yellow | High for GR |
| **Ricci scalar R** | Derived from metric fluctuation around Minkowski | diverging | Low (expensive) |
| **Higgs VEV \|v(x)\|** | If FTD models EWSB via a scalar condensate | yellow intensity | Low |
| **Topological charge** | `F ∧ F` integrand | gold spots | Medium |
| **Annihilation events** | Event markers (last N ticks) | red flash | UX gem |
| **Genesis events** | Event markers paired with Genesis isosurface | green flash | UX gem |
| **Locked voxels** | Where s is locked by inspector / scenario | outline overlay | UX aid |

### 5.1 Tier 3 checklist

- [ ] Each toggle: file-by-file pattern (§2)
- [ ] Renderer extensions: some (γ, R, topological charge) require new Three.js materials/shaders
- [ ] Consider a 5th overlay column `Diagnostics` if total exceeds 6 per column

---

## 6. Column layout after Tier 1

```
VOLUME      FIELDS          FORCES          QUANTUM          PHENOMENA
──────      ──────          ──────          ──────           ──────────
Flux        E Field         [Style ▸]       |ψ|² density     Dual J
Slice       B Field         EM              Phase φ          Chirality
Lines       Poynting S      Gravity         ℒ(x)             DM Halo
∇·J         Light           Strong          Entropy s        Genesis
                            Weak            Φ potential      Damping
                                                              Confinement
```

Column grid becomes 5-wide on desktop (minmax 130px), 2×3 on ≤1199px, single column on ≤767px. CSS already parametrized via `.s0-overlay-panel { grid-template-columns: repeat(5, …); }`.

---

## 7. File-level changes required for Tier 1

### 7.1 `state/store.js`

```js
export const FIELD_TOGGLE_KEYS = [
    ...previous,
    'showPsiSquared',
    'showPhase',
    'showLagrangianDensity',
    'showEntropyDensity',
    'showGravPotential',
];

function createFieldFlags() {
    return {
        ...previous,
        showPsiSquared: false,
        showPhase: false,
        showLagrangianDensity: false,
        showEntropyDensity: false,
        showGravPotential: false,
    };
}
```

### 7.2 `ui/dom.js`

```js
export const FIELD_TOGGLE_BINDINGS = [
    ...previous,
    ['toggle-psi-squared',        'showPsiSquared'],
    ['toggle-phase',              'showPhase'],
    ['toggle-lagrangian-density', 'showLagrangianDensity'],
    ['toggle-entropy-density',    'showEntropyDensity'],
    ['toggle-grav-potential',     'showGravPotential'],
];
```

### 7.3 `ui/overlays/template.js`

Add a fifth column between **Forces** and **Phenomena**:

```html
<div class="s0-overlay-col">
  <div class="s0-overlay-col-head">Quantum</div>
  <button class="view-toggle field-toggle" id="toggle-psi-squared" title="Born probability density |ψ|² …">
    <span class="field-swatch field-swatch-psi-squared"></span>|ψ|&sup2;
  </button>
  <!-- 4 more -->
</div>
```

### 7.4 `scales/scale0/toolbar.css`

```css
.field-swatch-psi-squared   { background: linear-gradient(90deg, #440154, #21918c, #fde725); }
.field-swatch-phase         { background: conic-gradient(from 0deg, #ff0055, #ffcc00, #00ff88, #00aaff, #9900ff, #ff0055); }
.field-swatch-lagrangian    { background: linear-gradient(90deg, #2166ac, #f7f7f7, #b2182b); }
.field-swatch-entropy       { background: linear-gradient(90deg, #000, #fff); }
.field-swatch-grav-potential{ background: linear-gradient(90deg, #000033, #0066cc, #ffff00); }
```

### 7.5 `viewport-adapter.js`

```js
const NON_FORCE_OVERLAYS = {
    ...previous,
    showPsiSquared:        'togglePsiSquaredField',
    showPhase:             'togglePhaseField',
    showLagrangianDensity: 'toggleLagrangianDensityField',
    showEntropyDensity:    'toggleEntropyDensityField',
    showGravPotential:     'toggleGravPotentialField',
};

// In the returned adapter:
applyPsiSquared(data)       { viewport?.updatePsiSquaredField?.(data); },
applyPhase(data)            { viewport?.updatePhaseField?.(data); },
applyLagrangianDensity(d)   { viewport?.updateLagrangianDensityField?.(d); },
applyEntropyDensity(d)      { viewport?.updateEntropyDensityField?.(d); },
applyGravPotential(d)       { viewport?.updateGravPotentialField?.(d); },
```

### 7.6 `runtime/field-overlays.js`

Add sampler functions that compute each quantity from the current flux buffer each tick when the corresponding flag is active:

```js
function computePsiSquared(fluxL, fluxR) { /* |J_L|² + |J_R|² */ }
function computePhase(fluxL, fluxR)      { /* atan2 of vector magnitudes */ }
function computeLagrangian(flux, dJdt, gradJ, s) { /* ... */ }
function computeEntropy(state)           { /* per-voxel Moore-neighborhood Shannon H */ }
function computeGravPotential(density)   { /* bridge.getGravPotential?.() or new solver */ }

export function updateFieldOverlays(ctx, state, adapter) {
    // existing field overlays
    if (state.fieldFlags.showPsiSquared) {
        const data = computePsiSquared(...);
        adapter.applyPsiSquared(data);
    }
    // ...
}
```

### 7.7 Renderer stubs (`viewport.js` / `mock-bridge.js`)

```js
// Minimal: accept the data, do nothing destructive. Upgrade later to full
// volumetric rendering.
togglePsiSquaredField(on) { this._showPsi2 = !!on; this._markDirty(); }
updatePsiSquaredField(data) { this._psi2Data = data; this._markDirty(); }
// Same pattern for phase, lagrangian, entropy, grav potential.
```

When `_showPsi2` is true and flux-volume is rendered, the point-cloud shader substitutes `|ψ|²` for `|J|` magnitude (fragment recolor). Later PRs extend with dedicated volumetrics.

---

## 8. Acceptance criteria

- [ ] Five new toggles appear in the new **Quantum** column
- [ ] Each toggle flips `.active`, `fieldFlags.<key>` updates, `bridge.setToggle` fires
- [ ] The runtime overlay loop probes each flag and emits a data buffer per tick
- [ ] Renderer methods at least no-op without throwing (verified in browser eval)
- [ ] Column layout holds on desktop, tablet (2×3), mobile (stacked)
- [ ] Theme-switch preserves swatch visibility on all 5 themes (swatches are data colors — do not theme)
- [ ] Contrast probe (§7 of [SPEC_THEME_DESIGN.md](./SPEC_THEME_DESIGN.md)) still passes
- [ ] SPEC_S0_OVERLAYS tier-2/3 tickets filed with links back to this spec

---

## 9. Revision log

| Version | Date | Change |
|---|---|---|
| 1.0 | 2026-04-16 | Initial 17-item catalog; Tier 1 implementation started |

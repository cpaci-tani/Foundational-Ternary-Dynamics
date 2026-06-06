# SPEC — Scale-0 Lattice Spectroscopy (Spectrum panel redesign) + Hierarchy panel removal

**Status:** `[DESIGN — approved direction; details delegated "do what makes the most sense — this is a science tool"]`
**Date:** 2026-06-06 · **Scope:** `engine/web` Scale-0 — replace the particle-mass "Spectrum Scanner"
with a field-spectroscopy instrument; remove the Hierarchy panel.

This is a **scientific instrument**, so the math is pinned here, not left to implementation. Every readout
carries an honesty tag: **[M]** measured directly (audit/sampler), **[D]** derived/computed (FFT, topology),
**[≈]** approximate (downsampled / band-limited live view).

---

## 1. Part 1 — Remove the Hierarchy panel

The Hierarchy panel is a near-empty stub (`HierarchyPanelComponent` just stamps a dataset attribute;
content came from `onticPanel.updateHierarchyPanel`). Removal scope:

- `js/ui/scale-registry/panel-registry.js` — delete the `{ id: 'hierarchy', … }` entry.
- `index.html` — delete the `#panel-hierarchy` DOM node (and its tab if statically present).
- `js/ui/panels/hierarchy-panel/` — delete the component dir; drop its import/registration in
  `js/ui/panels/index.js`.
- Wiring: remove `updateHierarchyPanel` calls + `ctx.updateHierarchyPanel` (app.js `_makeCtx` + the tab
  handler `handlePanelActivated`), the `case 'hierarchy'` in `diagnostics.js`, and the
  `onticPanel.updateHierarchyPanel` definition (app-ontic / ontic-observatory) if unused elsewhere.
- `panel-resources` template hierarchy content (if any).
- Tests: `validatePanelRegistry` stays green (it cross-checks registry↔DOM, so both sides must drop
  together); update `scales.spec.js` / any spec asserting the hierarchy tab.

**Guard:** after removal, `validatePanelRegistry(panelArea)` must return `ok:true` (no registry entry
without a DOM panel and no DOM panel without a registry entry).

---

## 2. Part 2 — Lattice Spectroscopy: concept + niche

Characterize **the lattice field itself**: what spatial scales hold the energy, what topological structure
is present, how the field metrics are distributed, and how the energy partitions. Distinct from its
neighbors: the **conservation micropanel** tracks energy/Gauss *over time*; **P1-observables** probes the
Coulomb field at a point. This panel answers *"what is the field's structure right now?"*

Four sections, top to bottom: **① E(k) energy spectrum (hero) · ② Topology · ③ Field metrics +
distributions · ④ Energy partition.**

Data sources (all already on the capability surface):
- `getScale0FieldSamples({kind:'fluxVector', stride})` → **J** components on a strided regular sub-grid
  (`{positions, vectors, count}`) — the spectrum input.
- `getScale0FieldSamples({kind})` for `divJ`, `curlJ`/`b`, `gaussResidual`, `vorticity`, `helicity`,
  `coherence`, `fisher`, `kretschmann` (`{positions, values}` / `{…, vectors}` point sets).
- `getScale0EnergyAudit()` → `fieldEnergy`, `waveEnergy`, `eFieldEnergy`, `bFieldEnergy`, `totalPoynting`,
  `gaussViolation`, `maxGaussError`, `chiralityTotal`, `ELTotal`/`ERTotal`, `wvLTotal`/`wvRTotal`,
  `energyDrift`.
- `getScale0Diagnostics()` → `entropy`, `totalFlux`, `manifested`, charges.

---

## 3. ① E(k) — field energy spectrum (the hero)  **[D]**

**Definition.** For the flux field **J**(x) on the L³ periodic lattice, the energy spectrum is the
shell-summed power of its Fourier transform:

```
Ĵᵢ(k) = FFT3(Jᵢ)            (i = x,y,z ; periodic lattice ⇒ DFT is exact)
E(k) = Σ_{ |k'| ∈ [k, k+Δk) }  Σᵢ |Ĵᵢ(k')|²          (radially binned)
```

with physical wavevector `|k| = (2π/L)·√(nₓ'² + n_y'² + n_z'²)`, signed frequency
`nᵢ' = nᵢ (nᵢ ≤ M/2) else nᵢ − M`, grid side M (power-of-2, the sub-grid padded). Plotted **log–log**:
E(k) vs k. This is the standard turbulence-style energy spectrum (Parseval ⇒ Σ_k E(k) = Σ_x |J(x)|²).

**Self-validation (the science check).** With the unitary normalization, `Σ_k E(k)` must equal
`Σ_x |J(x)|²` (Parseval) which ties to the audit's `fieldEnergy` (= ½Σ|J|² over the same support). The
panel computes the ratio `ΣE(k) / (2·fieldEnergy)` and displays it as a **consistency check** (≈1.00 ⇒
the instrument is correct); a drift flags a bug or a support/scaling mismatch. Tag **[M↔D]**.

**Derived readouts** under the plot:
- **Peak mode k\*** = argmaxₖ E(k); **dominant wavelength λ\* = 2π/k\*** (in voxels). **[D]**
- **Spectral slope p**: least-squares fit of `log E(k)` vs `log k` over the resolved inertial range
  (exclude k=0 and the top decade near Nyquist). `E(k) ~ kᵖ`; p<0 ⇒ energy at large scales (IR-dominated),
  p>0 ⇒ UV/small-scale buildup. **[D]**
- **Total spectral power** Σ E(k) + the Parseval ratio above. **[D/M]**
- Optional overlay: **E_J(k)** (flux) vs **E_WV(k)** (wave-velocity / E-field) — the audit splits energy
  into field vs wave; the spectrum splits the same way, so the user can see which channel holds which scales.

**Live vs Deep (scientifically meaningful, not just perf):**
- **Live** uses the strided sub-grid (stride s ⇒ M ≈ ⌈L/s⌉). Its Nyquist is `k < π/s` — it resolves the
  **large-scale (IR) modes only**; the UV tail is not present. Tagged **[≈ band-limited, k<π/s]**.
- **Deep Measure** uses **stride = 1** (full field) ⇒ Nyquist `k < π` — the **full k-range** including UV.
  This is why Deep Measure exists: it is the rigorous, full-bandwidth spectrum, not a cosmetic upgrade.

---

## 4. ② Topology  (invariants from the field)

- **Gauss-law violation** `[M]`: `gaussViolation` (= Σ(∇·E−ρ)²) and `maxGaussError` from the audit; show
  Σ, max, and a pass badge (`gv < 1e-4`). This is the constraint-satisfaction / topological-charge
  consistency measure.
- **Defect / monopole count** `[D]`: from the `divJ` sampler — voxels with `|∇·J| > τ·max|∇·J|`
  (τ≈0.5) are sources/sinks; report **#sources, #sinks, net = Σ sign**. Honest label: "div·J extrema above
  threshold" (a monopole *proxy*, not a quantized charge).
- **Flux-tube / string count** `[D]`: connected components of `{ |J| > τ_J }` (6-neighbour union-find on
  the strided grid; `τ_J` ≈ a fraction of max|J|). Report **#components + largest size** — the confinement
  strings / coherent flux structures.
- **Chirality / L–R asymmetry** `[M]`: `chiralityTotal`, plus `A = (ELTotal−ERTotal)/(ELTotal+ERTotal)`
  and the wave-channel analogue `(wvLTotal−wvRTotal)/(…)`. The dual-substrate handedness.

---

## 5. ③ Field metrics + distributions

For each metric — **vorticity, helicity, coherence, Fisher information, Kretschmann curvature** (samplers)
and **entropy** (diagnostics) — show the live **mean / rms / max** **[M]** *and* a **spatial histogram of
the sampled values** **[D]** (so the user sees the distribution/structure, not just a mean). A compact
metric strip (one row each: name · value · mini-histogram), with a click-to-expand larger distribution for
the selected metric. Histograms bin the sampler's `values` array (fixed bin count, auto-range).

---

## 6. ④ Energy partition

A compact stacked bar of **E-field / B-field / wave / field** energy (audit `eFieldEnergy`, `bFieldEnergy`,
`waveEnergy`, `fieldEnergy`) **[M]**, the **Poynting flux** magnitude `|totalPoynting|` + direction **[M]**,
and **energy drift %** (`energyDrift`) **[M]**. Kept compact and complementary to the conservation
micropanel (which owns the time-series); this is the instantaneous partition.

---

## 7. Read model + architecture

- **Live:** `rafCoordinator.subscribe(PANEL_ID, { hz: 2, cb: update })`. `update` reads the strided
  `fluxVector` sub-grid + the metric samplers + the cached audit (all main-thread, cheap), recomputes the
  band-limited spectrum + topology + metric histograms. Stride auto-scales: `s = clamp(round(L/24), 2, 8)`
  (~24³ sub-grid).
- **Deep Measure (button):** full-resolution.
  - **Worker path (default for flux-*):** post a `deepMeasure` command → the worker computes the full-res
    spectrum + topology from its own `_fluxJ` (off the render thread, no cap) via the shared analysis utils
    → posts a `deepMeasureResult` → the proxy stores it → the panel renders it. A "measuring…" state shows
    until the result arrives.
  - **Non-worker path:** compute on the main thread from the full `fluxVector` (stride 1) with the
    "measuring…" state (one-time, on click).
- **Shared analysis utils** (pure, no DOM, importable by both the panel and the worker):
  - `js/scales/scale0/analysis/lattice-spectrum.js` — radix-2 Cooley–Tukey 1D FFT, separable 3D wrapper
    (FFT along x, then y, then z), power-of-2 zero-pad, radial binning, physical-k normalization, Parseval
    ratio, peak/slope extraction. The one genuinely new algorithm.
  - `js/scales/scale0/analysis/lattice-topology.js` — div·J extrema count, flux-tube union-find connected
    components, chirality/asymmetry helpers, histogram helper.
- **Panel:** `js/scales/scale0/ui/overlays/spectrum-panel.js` — full rewrite (4 sections, SVG/canvas charts
  matching the existing style, live + deep wiring, honesty tags). Registry id/label unchanged (`spectrum`,
  scale 0). New CSS in the panel CSS file.
- **Bridge:** `mock-bridge.worker.js` (+`deepMeasure` command → analysis utils → `deepMeasureResult`),
  `mock-bridge-proxy.js` (`deepMeasure()` poster + `_lastDeepMeasure` store + accessor). The worker imports
  the analysis utils (pure modules, no DOM — safe in a worker).

---

## 8. Testing

`tests/scale0-spectrum.spec.js`:
- Panel mounts under `#panel-spectrum`; sections render.
- On a flux scenario (e.g. `flux-pulse`, running): E(k) is non-empty, finite, monotone-k bins; **Parseval
  ratio ∈ [0.5, 2]** (the correctness check — the spectrum total tracks the audit field energy).
- Deep Measure yields **more k-bins** than live (finer/full-band) and stays finite.
- Topology counts are integers ≥ 0; metric histograms populate (non-empty on a live field).
- Energy partition bars sum to a positive total.
- Registry/hierarchy: `validatePanelRegistry` ok after removal; `scales.spec.js` green.

---

## 9. Risks

- **FFT cost:** capped by power-of-2 pad of the sub-grid (live ~32³ → trivial). Deep full-res worker-offloaded
  on the default path; main-thread deep at large L is one-time on click with a spinner.
- **Strided aliasing:** the live spectrum is band-limited and **labeled as such** ([≈ k<π/s]) — honest, not
  hidden. Deep Measure is the unaliased reference.
- **Parseval mismatch:** if the ratio strays from 1, it's surfaced as a visible consistency number (a
  feature: the tool validates itself) rather than silently wrong.
- **Worker protocol:** the `deepMeasure` command/result is small and gated behind the button; the
  non-worker fallback keeps the feature available everywhere.

# PLAN — Port the Web Scale-0 Overlay System to the Native D3D12 Renderer

**Status:** discovery + planning (read-only). No code changed. Uncommitted, for review.
**Scope:** every Scale-0 field / phenomena visualization the web dashboard offers, mapped to a port plan for `engine/native` (RmlUi + D3D12 rebuild).
**Author intent (from the request):** "all of the physics phenomena" — flux lines, E/B fields, flux volume, flux slices, confinement strings, dark-matter halo, dual substrate, genesis threshold, charge density, potential, Gauss viz, topology sheets, etc.

---

## 0. Executive summary

- **Total distinct Scale-0 overlays catalogued: 33** (plus 4 force render-styles that cross-cut the 4 force overlays, plus the Flux-Volume / Flux-Slice appearance modifiers).
- **Native-status breakdown (primary status per overlay):**
  - **COVERED — 11.** Map directly to the existing native path (`append_field_vectors` arrows / `append_field_scalars` points) via one of the 18 `VisualFieldKind`s, needing at most a colour-ramp tweak.
  - **EXTEND — 13.** Native already has (or can trivially derive) the data, but the web uses a draw technique the native app lacks (plane point-slices, wireframe boxes, needle segments, pair-links, gaussian sprites, instanced cones, dense magnitude grid, derived scalars).
  - **NEW — 9.** Need a genuinely new subsystem: **6 rubber-sheet surfaces** (a triangle-mesh vertex-colour PSO the native app does not have) and **3 streamline overlays** (RK4 field-line integration the native app does not have).
- **Force render-styles (cross-cutting):** Arrows = COVERED, Heatmap = EXTEND, Glyphs = EXTEND, Flow = NEW (streamlines).
- **The single biggest architectural gap** is not any one overlay — it is that the native app renders **exactly one overlay at a time** (`SetFieldOverlay` replaces the ambient flux cloud), whereas the web composites **all active overlays simultaneously** via a per-frame job scheduler. Tranche 1 is mostly this lift.
- **Recommended first tranche:** (1) multi-overlay compositing on the native side, then (2) all COVERED point/arrow overlays, then (3) the derived-scalar and line-PSO EXTENDs that reuse the two primitives the native app already has. This lights up ~24 of 33 overlays with **zero new PSOs**.

### Native rendering capability (ground truth for the verdicts)

The native presenter (`engine/native/src/d3d12_presenter.cpp`) has exactly **three PSOs**:

| PSO | Topology | Used for | Web overlays it can serve |
|-----|----------|----------|----------------------------|
| `pso` (sprite) | `TRIANGLE` (instanced camera-facing quad, 6 verts/instance) | particles + `frame.flux` point cloud | every "colored point cloud" overlay |
| `pso_lines` | `LINELIST` | wireframe box + `frame.field_lines` | vector arrows, streamlines, wireframe boxes, pair-links, needles |
| `pso_interop` | `TRIANGLE` | CUDA-interop particle path | (particle fast-path only) |

`NativeFrame` (`engine/native/include/native/native_frame.h`) carries `particles` (sprites), `flux` (sprites), and `field_lines` (`NativeLine`: two coloured endpoints). **There is no triangle-mesh-with-vertex-colour surface PSO** (only billboarded quads) → rubber sheets are NEW. **There is no volumetric raymarch** — but the web "Flux Volume" is itself only an additive sprite point cloud, so it maps to `pso` (COVERED), not a raymarcher. **There is no field-line integrator** → streamlines are NEW.

### Data plumbing that already exists on the native side

Every web overlay sources data through one of four bridge calls; three of the four are already wired natively:

| Web bridge call | Native equivalent | Status |
|-----------------|-------------------|--------|
| `getScale0FieldSamples({kind, stride})` | `RenderBridge::copy_visual_field_sample(VisualFieldKind, stride, out)` | present; all 18 kinds |
| `getScale0ForceField('em'\|'gravity'\|'strong', stride)` | `VisualFieldKind::{EmForce, GravityForce, StrongForce}` (kinds 14/15/16) via same call | present |
| `getScale0FluxVolume()` (dense N³ or FTV2 `{data,latticeSize,stride,axisCount}` magnitude grid) | **none** — must assemble from `copy_visual_field_sample(FluxVector)` magnitude, or add a dense-grid path | new data-assembly |
| `getParticleData()` / `getScale0ParticleFrame()` | native visual snapshot particles (`capture()`) | present |

`VisualFieldKind` (`engine/include/ftd/visual_field_sample.h`) already covers: Electric(0), Magnetic(1), Poynting(2), Divergence(3), FluxVector(4), Vorticity(5), Helicity(6), Kretschmann(7), Latency(8), Fisher(9), Coherence(10), Curl(11), State(12), GaussResidual(13), EmForce(14), GravityForce(15), StrongForce(16), PoissonLatency(17).

---

## 1. Complete overlay inventory

Columns: **Overlay** (web UI label) · **Physics** · **Web render technique** · **Data source** (exact) · **Key params** · **Web Three.js impl** · **Native status** · **D3D12 technique needed**.

Web renderer modules referenced (all under `engine/web/js/`):
`viewport/flux-renderer.js`, `viewport/field-em-renderer.js`, `viewport/field-force-renderer.js`, `viewport/field-quantum-renderer.js`, `viewport/field-topology-renderer.js`, `viewport/topology-sheet-renderer.js`, `fieldlines.js`, and the Scale-0 orchestration in `scales/scale0/runtime/field-overlays.js` (+ `overlay-frames.js`, `field-sample-cache.js`).

### 1a. VOLUME column

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **Flux Volume** | flux magnitude \|J\| of the whole substrate | Additive soft-sprite **point cloud** (peak-hold normalized); NOT a true raymarch | `getScale0FluxVolume()` dense N³ or FTV2 magnitude grid | fractional-stride sample grid (`FLUX_MAX_AXIS_POINTS=53`, ≤149K pts), threshold slider (`0.005`), organic ±½-cell jitter, additive glow bloom, peak-hold decay `0.985` | `THREE.Points` + `FLUX_VOL_VERT`/`PARTICLE_FRAG` `ShaderMaterial`, `AdditiveBlending`, `DynamicDrawUsage`; `flux-renderer.js` `updateFluxVolume` | **COVERED** (this IS the native `append_flux` ambient cloud) | reuse `pso` sprites. **EXTEND** to add: 3D hash jitter (organic), additive glow uniform, threshold slider, peak-hold-with-decay normalizer |
| **Flux Slice** | \|J\| on the 3 lattice mid-planes (xy@z=L/2, xz@y=L/2, yz@x=L/2) | **Colored points on cut-planes** (magnitude only, sign-blind) | flux magnitude on mid-planes (gathered by `frame-sync.js`; from the same magnitude grid) | per-axis enable (xy/xz/yz), shared opacity / shape / point-size / threshold with Flux Volume | `THREE.Points` sized `3·N²`; `field-em-renderer.js` `updateFluxSlices` | **EXTEND** | reuse `pso` sprites; add a plane-slice sampler (emit points only for the enabled mid-plane indices) + per-axis toggles |
| **Flux Lines** | J-field flow direction (streamlines) | **Traced RK4 streamlines**, colored by LOCAL \|J\| at each vertex | `getScale0FieldSamples({kind:'fluxVector'})` → `VisualFieldKind::FluxVector` | importance-sampled seeds (∝\|J\|^1.5), bidirectional, `stepSize 0.5`, `maxSteps 100`, `maxLines 200`; flux colormap | `computeStreamlines` (`fieldlines.js`) → pooled line buffer → `LineSegments`; `flux-renderer.js` `updateFluxStreamlines` | **NEW** | line PSO exists; **needs the RK4 streamline integrator** + importance-sampling seed generator + per-vertex magnitude coloring |
| **∇·J** (Divergence) | divergence of flux = charge sources (red) / sinks (blue) | **Colored points** (signed red/blue), additive | `getScale0FieldSamples({kind:'divJ'})` → `VisualFieldKind::Divergence` (3) | threshold `1%` of max, additive blend | `THREE.Points` particle-frag; `field-em-renderer.js` `updateDivergenceField` | **COVERED** | `append_field_scalars(Divergence)` already draws signed points |
| **State s** | ternary manifestation field s∈{−1,0,+1} [AXIOM] | **Colored points** (s=−1 blue, s=+1 red; void invisible) | `getScale0FieldSamples({kind:'state'}, stride 1)` → `VisualFieldKind::State` (12) | stride-1 always, soft-disc sprite, additive | `THREE.Points` soft-disc `PointsMaterial`; `field-em-renderer.js` `updateStateField` | **COVERED** | `append_field_scalars(State)`; tweak ramp to blue/red sign split |

### 1b. FIELDS column

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **Radiative E** | inductive E = −∂J/∂t (streamlines) | **Traced RK4 streamlines**, cyan fade | `getScale0FieldSamples({kind:'e'})` → `VisualFieldKind::Electric` (0) + particle frame for seeds | particle-anchored seeds (6/particle, offset 2) when particles exist, else importance seeds; bidirectional | `computeStreamlines` → shared streamline mesh; `field-em-renderer.js` `updateEFieldLines` (cyan, alpha fade, optional per-knot hue) | **NEW** | **needs RK4 integrator** + particle-anchored + importance seed generators |
| **B Field** | magnetic B = ∇×J (closed-loop streamlines) | **Traced RK4 streamlines**, green fade | `getScale0FieldSamples({kind:'b'})` → `VisualFieldKind::Magnetic` (1) + particle frame | ring seeds (8/particle, radius 4, perpendicular to flux) or perpendicular importance seeds; `maxSteps×1.5` to close loops | same mesh factory; `updateBFieldLines` (green) | **NEW** | RK4 integrator + B-specific loop seeding (Gram-Schmidt perpendicular rings) |
| **Poynting S** | energy flux S = E×B | **Vector arrows** (yellow→orange), additive | `getScale0FieldSamples({kind:'poynting'})` → `VisualFieldKind::Poynting` (2) | threshold `5%`, `log` length scale, 32768-arrow cap | `LineSegments` arrow pairs; `field-em-renderer.js` `updatePoyntingVectors` | **COVERED** | `append_field_vectors(Poynting)` |

### 1c. FORCES column (each overlay renders in one of 4 styles: Arrows / Heatmap / Flow / Glyphs)

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **EM** | electrostatic Coulomb force [PARAMETRIC] | arrows / gaussian heatmap / animated flow / instanced cones | `getScale0ForceField('em', stride)` → `VisualFieldKind::EmForce` (14) | force-stride finer at small L; per-style params below | `field-force-renderer.js` `updateForceVolume` + style renderers | **COVERED** (arrows) | `append_field_vectors(EmForce)` for arrows |
| **Gravity** | density-gradient attraction G_N·∇\|J\| [SELECTION] | same 4 styles | `getScale0ForceField('gravity')` → `VisualFieldKind::GravityForce` (15) | as above | `updateGravityField` | **COVERED** (arrows) | `append_field_vectors(GravityForce)` |
| **Strong** | pairwise flux-tube force [SELECTION] | same 4 styles | `getScale0ForceField('strong')` → `VisualFieldKind::StrongForce` (16) | as above | `updateStrongForceField` | **COVERED** (arrows) | `append_field_vectors(StrongForce)` |
| **∇×J pseudovector** ("weak") | curl ∇×J parity-even (axial) pseudovector [PROXY], scaled by `DUAL_DELTA` | arrows (soft sprites) / heatmap / flow / glyphs | `getScale0FieldSamples({kind:'curlJ'})` → `VisualFieldKind::Curl` (11), ×`DUAL_DELTA≈0.957` | threshold `8%`, weak palette | `field-force-renderer.js` `updateWeakField` (soft additive sprites) | **COVERED** (arrows/points) | `append_field_vectors(Curl)` (or scalar points of \|∇×J\|) |
| **— Force style: Arrows** | (render style) | base→tip line pairs, `log` length | (as force above) | 32768 cap | `_writeArrowFieldIntoMesh` | **COVERED** | line PSO (== native arrow path) |
| **— Force style: Heatmap** | (render style) | gaussian additive **sprite points**, size ∝ log(mag) | (as force) | `exp(-r²·16)` frag, per-force palette | custom `ShaderMaterial` in `updateForceHeatmap` | **EXTEND** | reuse `pso` sprites + a gaussian-falloff fragment (new frag or shader param) |
| **— Force style: Flow** | (render style) | **animated dashed streamlines** | (as force) | flow seeds ∝\|force\|, 40% length, dash-offset animation | `computeStreamlines` + `LineDashedMaterial`; `updateForceStreamlines` | **NEW** | RK4 integrator + dashed-line rendering + per-frame dash animation |
| **— Force style: Glyphs** | (render style) | **instanced oriented cones** | (as force) | `ConeGeometry(0.3,1,6)`, per-instance transform+color, 8000 cap | `InstancedMesh`; `updateForceGlyphs` | **EXTEND** | **new instanced-cone triangle PSO** (per-instance matrix + color) — or reuse the interop instanced path |

### 1d. QUANTUM column

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **\|ψ\|²** | flux energy density \|J\|², max-normalized [PROXY Born analogue] | **Viridis point cloud**, breathing opacity | derived from `fluxVector` via `computePsiSquaredFrame` (`overlay-frames.js`) | threshold `0.02`, viridis ramp, `0.85+0.15·sin` pulse | `field-quantum-renderer.js` `_populateQuantumField` (soft-disc points) | **EXTEND** (derive scalar) | reuse `pso` sprites; derive \|J\|² from `FluxVector`, viridis ramp, opacity pulse |
| **Phase φ** | complex phase arg(J_L+iJ_R) [PROXY; needs Dual Substrate] | **Oriented line-segment needles** (cyclic-HSL colored) | `computePhaseFrame` from `fluxVector` + dual L/R vecs | needle length `1.2`, `rampCyclicHSL`, threshold `0.02` | `LineSegments` needles; `field-em-renderer.js` `updatePhaseField` | **EXTEND** | line PSO exists; emit one short oriented segment per voxel + cyclic-HSL color |
| **ℒ(x)** Lagrangian density | ½\|E\|²−½(∇·J)² [PROXY] | **Diverging (RdBu) point cloud**, signed | `computeLagrangianDensityFrame` from `fluxVector`+`poynting`+`divergence`+`e` | signed, `rampDivergingRdBu`, threshold `0.10` | `_populateQuantumField` signed; `updateLagrangianDensityField` | **EXTEND** (derive scalar) | reuse `pso` sprites; derive the composite scalar; diverging ramp |
| **Entropy s** | disorder proxy 4p(1−p), p=\|J\|/\|J\|max [PROXY] | **Jittering sparkle points**, grayscale | `computeEntropyDensityFrame` from `fluxVector` | jitter `s·0.8`, `rampGrayscale`, threshold `0.04` | `field-quantum-renderer.js` `updateEntropyDensityField` | **EXTEND** (derive scalar) | reuse `pso` sprites; derive scalar + per-point jitter offset |

### 1e. TOPOLOGY column (rubber sheets, except Latency / Gauss which are point clouds)

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **Φ potential** | gravitational potential stand-in −\|J\|² (blurred) [PROXY] | **Deformable rubber sheet** (mesh + wireframe), wells dip | `computeGravPotentialFrame` from `fluxVector` | 40×40 `PlaneGeometry`, depth `0.25·N`, signed, `rampGravWell` | `topology-sheet-renderer.js` `updateGravPotential` (scatter→grid→2-pass box-blur→bilinear deform) | **NEW** | **new triangle-mesh vertex-color PSO** (double-sided, + wireframe pass) + heightfield scatter/blur/deform (CPU or compute) + dynamic vertex buffer |
| **EM energy u** | ½\|E\|²+(c²/2)\|B\|² Maxwell energy density | **Rubber sheet** | `computeEmEnergyFrame` from `e`+`b` (`VisualFieldKind` Electric+Magnetic) | yFrac 0.05, depth 0.08N, `rampEmEnergy`, peak-hold | `TopologySheetRenderer.update('emEnergy')` | **NEW** | same mesh PSO + scatter/blur/deform (shared once built) |
| **Charge ρ** | ρ = ∇·J signed [SELECTION] | **Rubber sheet** (signed: red hills / blue wells) | `computeChargeDensityFrame` from `divJ` → `VisualFieldKind::Divergence` | yFrac 0.87, depth 0.08N, signed, `rampCharge` | `update('chargeDensity')` | **NEW** | shared mesh PSO |
| **Vorticity ω** | \|∇×J\| swirl magnitude | **Rubber sheet** (thin band) | `getScale0FieldSamples({kind:'vorticity'})` → `VisualFieldKind::Vorticity` (5) | yFrac 0.97, depth 0.03N, `rampVorticity` | `update('vorticity')` | **NEW** | shared mesh PSO (data is COVERED; only the sheet technique is NEW) |
| **Latency L** | normalized flux density √(\|J\|²/\|J\|²max) [PROXY] | **Colored point cloud** (blue→red) | `getScale0FieldSamples({kind:'latency'})` → `VisualFieldKind::Latency` (8); **PoissonLatency (17)** for native mass-gravity scenarios | threshold `0.02`, blue-low/red-high | `field-quantum-renderer.js` `_updateScalarCloud('latency')` | **COVERED** | `append_field_scalars(Latency)`; kind override to 17 for mass-gravity scenarios |
| **Gauss resid.** | r = ∇·J − s [MEASURED residual] | **Signed point cloud** (red/blue) | `getScale0FieldSamples({kind:'gaussResidual'}, stride 1)` → `VisualFieldKind::GaussResidual` (13) | signed, threshold `0.05`, stride-1 | `_updateScalarCloud('gaussResidual')` | **COVERED** | `append_field_scalars(GaussResidual)` |

### 1f. STRESS-ENERGY column (rubber sheets)

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **P_E (electric)** | ½\|E\|² electric pressure | **Rubber sheet** | `computeEPressureFrame` from `e` → `VisualFieldKind::Electric` | yFrac 0.35, depth 0.08N, `rampEPressure` | `TopologySheetRenderer.update('ePressure')` | **NEW** | shared mesh PSO + derive ½\|E\|² |
| **P_B (magnetic)** | (c²/2)\|B\|² magnetic pressure, c=`C_SPEED` | **Rubber sheet** | `computeBPressureFrame` from `b` → `VisualFieldKind::Magnetic` | yFrac 0.45, depth 0.08N, `rampBPressure` | `update('bPressure')` | **NEW** | shared mesh PSO + derive (c²/2)\|B\|² |

### 1g. PHENOMENA column

| Overlay | Physics | Web render technique | Data source | Key params | Web Three.js impl | Native status | D3D12 technique |
|---|---|---|---|---|---|---|---|
| **Dual J** | amplitude split J_L=J(1+δ)/2, J_R=J(1−δ)/2 [PROXY], δ=`DUAL_DELTA` | **L/R colored point cloud** (L warm, R cool) | derived from `fluxVector` (collinear split) in `buildDerivedSubstrateData` | threshold `2%`, additive | `field-quantum-renderer.js` `updateDualFluxVolume` | **EXTEND** (derive L/R) | reuse `pso` sprites; emit two colored point sets from `FluxVector`·(1±δ)/2 |
| **Chirality** | \|J\|·δ non-negative chiral amplitude [PROXY] | **Colored point cloud** (signed red/blue) | derived from `fluxVector` in `buildDerivedSubstrateData` | threshold `2%` | `updateChiralityField` | **EXTEND** (derive scalar) | reuse `pso` sprites; derive \|J\|·δ scalar |
| **DM Halo** | sub-threshold flux envelope 0.003<\|J\|<K_GENESIS [PROXY] | **Colored point cloud** (band-selected) | `getScale0FluxVolume()` dense \|J\| grid + `getScale0ParticleFrame()` | band `[0.003, K_GENESIS]`, `K_GENESIS≈1.533`, additive | `field-topology-renderer.js` `updateDarkMatterHalo` | **EXTEND** (dense grid) | reuse `pso` sprites; **needs a dense \|J\| magnitude grid** (from `FluxVector` stride-1) + band select |
| **Genesis** | genesis frontier shell \|J\|≈K_GENESIS [SELECTION] | **Isosurface shell as a point cloud** (band, NOT marching cubes) | `getScale0FluxVolume()` dense \|J\| grid, threshold `K_GENESIS` | band `kGenesis·0.15`, additive | `field-topology-renderer.js` `updateGenesisIsosurface` | **EXTEND** (dense grid) | reuse `pso` sprites; dense \|J\| grid + shell-band select (NOT true isosurface — points suffice) |
| **Color charge** | recolor particles by genesis color axis (argmax\|J_axis\| ∈ {r,g,b}) [not SU(3)] | **Particle recolor mode** (not a separate mesh) | per-particle color-charge (genesis-assigned) | recolors existing particle sprites | `viewport` `toggleColorChargeRender`; state key `showColorCharge` (special-cased, not in `FIELD_TOGGLE_KEYS`) | **EXTEND** | recolor the existing particle sprites; **needs per-particle color-charge in the visual snapshot** |
| **Damping** | selective-damping zones (7-cell von-Neumann footprint) | **Wireframe boxes** (3-voxel cube edges) around particles | `getScale0ParticleFrame()` positions | 12 edges/particle, red, `maxSegments 1200` | `field-topology-renderer.js` `updateDampingZones` (`LineSegments`) | **EXTEND** | line PSO exists; emit 12 box edges per particle |
| **Confinement** | pair-proximity links, 1<r<√120≈10.95 [PROXY] | **Line segments between particle pairs** | `bridge.getParticleData()` | dist² threshold `CONFINEMENT_PAIR_DIST2`, color = separation direction, additive | `field-topology-renderer.js` `updateConfinementStrings` (spatial-hash pair scan → `LineSegments`) | **EXTEND** | line PSO exists; emit a link segment per qualifying pair (spatial-hash the pair scan) |
| **Horizon** | voxels where latency L≥0.95 [PROXY] | **Point cloud** (threshold-selected) | `getScale0FieldSamples({kind:'latency'})` via `computeHorizonFrame` | L≥0.95 select | `field-quantum-renderer.js` `updateHorizonField` (dark points) | **COVERED** | `append_field_scalars(Latency)` + threshold select |

### 1h. Auxiliary / not in the main overlay menu

| Overlay | Physics | Web render technique | Data source | Native status | D3D12 technique |
|---|---|---|---|---|---|
| **Knot zones** | field-line knots (clumps of crossing E/B streamlines) | **Wireframe boxes** (per-knot, E and B families, per-knot hue) | field-line knot tracker (`field-line-knots.js`, analysis over the E/B streamlines) — gated by the Knots panel + `knotTracking` (NOT a menu toggle, NOT in `anyFieldActive`) | `field-topology-renderer.js` `updateKnotZones` | **EXTEND** (rendering) + **NEW** (knot-detection analysis is a whole subsystem, and depends on the streamline integrator being ported first) | line PSO for the boxes; port the knot clustering/tracking analysis |
| **Ambient flux cloud** | default \|J\| cloud when no overlay active | sprite point cloud | `copy_visual_field_sample(FluxVector)` | **COVERED** — already implemented (`append_flux`) | — |
| **Event horizon sphere** | Scale-1 black-hole horizon (sphere + accretion ring) | `SphereGeometry` + `TorusGeometry` meshes | Scale-1 scenario | Scale-1, out of scope here | — |

### 1i. Appearance modifiers (not standalone overlays)

- **Flux Volume:** Organic (3D jitter) · Glow (additive bloom) · Opacity · Shape · Point Size · Threshold sliders (`ui/controls/flux-volume.js` → `wire.js::wireFluxVolume` → `flux-renderer.js` `setFlux*`).
- **Flux Slice:** xy / xz / yz axis toggles + shared opacity/shape/point-size/threshold (fan out to the slice mesh too).
- **Force render-style selector:** Arrows / Heatmap / Flow / Glyphs (applies to all 4 force overlays).

---

## 2. Phased port plan

### Tranche 0 — Prerequisite: multi-overlay compositing (architectural)

The native `SetFieldOverlay` command holds **one** `{enabled, kind}` and `capture()` replaces the flux cloud with that single field. The web composites **all** active overlays each frame through an amortized job scheduler (`field-overlays.js` `updateFieldOverlays`). Before porting individual overlays, lift the native side to hold **a set of active overlays** and append each into `NativeFrame` (a set of `field_lines` groups + `flux`/points groups + future mesh groups). This is the load-bearing change; nearly every later overlay depends on it.

- Replace `overlay_enabled_/overlay_kind_` (`scale0_adapter.h`) with a small set of enabled overlay descriptors.
- Keep the ambient flux cloud only when the set is empty (matches web `anyFieldActive`).
- Optional but recommended: port the web's per-frame **budget scheduler** later (amortize streamlines/rubber sheets across frames) — not needed until the heavy overlays land.

### Tranche 1 — COVERED overlays (no new PSO, no new data)

Direct `VisualFieldKind` → existing arrows/points path. Ship these first; they exercise the multi-overlay lift end-to-end.

- **Points (scalar):** ∇·J (Divergence), State s, Latency L (+ PoissonLatency override for mass-gravity scenarios), Gauss resid., Horizon (Latency + threshold).
- **Arrows (vector):** Poynting S; EM / Gravity / Strong / ∇×J-weak forces (Arrows style).
- **Sprite cloud:** Flux Volume (core) — already the ambient cloud; just make it a first-class toggle.

Deliverable: **11 overlays live** using only `pso` (sprites) + `pso_lines` (arrows).

### Tranche 2 — EXTEND overlays that reuse the two existing primitives

No new PSO; add derivation code, new point/line emission patterns, or a dense grid.

- **Derived scalars → sprite points:** \|ψ\|² (viridis + pulse), ℒ Lagrangian (diverging), Entropy s (jitter), Chirality (\|J\|·δ), Dual J (L/R split, two point sets).
- **Dense \|J\| grid → sprite points:** DM Halo (band 0.003–K_GENESIS), Genesis (K_GENESIS shell band). Add a `getScale0FluxVolume`-equivalent: `copy_visual_field_sample(FluxVector, stride)` → per-voxel magnitude buffer.
- **Line-PSO emission:** Damping (wireframe boxes), Confinement (pair-link segments, spatial-hashed), Phase φ (oriented needles).
- **Flux Volume refinements:** organic jitter, glow uniform, threshold slider, peak-hold-decay normalization.
- **Particle recolor:** Color charge (needs per-particle color axis added to the visual snapshot).

Deliverable: **~13 more overlays live**, still zero new PSOs (Color charge and the dense grid need new *data* plumbing, flagged below).

### Tranche 3 — EXTEND overlays needing a modest new pipeline

- **Force Heatmap style:** reuse `pso` sprites with a gaussian-falloff fragment (small new frag / shader param).
- **Force Glyphs style:** **new instanced-cone triangle PSO** (per-instance transform + color) — or reuse the `pso_interop` instanced path.
- **Flux Slice:** plane sampler + per-axis toggles (sprite points on the 3 mid-planes).

### Tranche 4 — NEW: streamlines (RK4 integrator)

The heaviest shared subsystem. Once the integrator exists, four overlays light up.

- Port `computeStreamlines` (RK4 over a nearest-sample spatial index, bidirectional) + the seed generators (`generateEFieldSeeds`, `generateBFieldSeeds`, `generateImportanceSeeds`, `generateBImportanceSeeds`) from `fieldlines.js`. **CPU port is the low-risk path**; a compute-shader version is a later optimization.
- Overlays: **Flux Lines, Radiative E, B Field, Force Flow** (Flow adds dashed-line animation).
- Renders through the existing `pso_lines` (LINELIST) — no new PSO, but a **larger dynamic line vertex buffer** and per-vertex color.
- **Unblocks Knot zones** (the knot detector consumes the streamline output).

### Tranche 5 — NEW: rubber-sheet surfaces (new mesh PSO)

- **New triangle-mesh vertex-color PSO** (double-sided, transparent, + a wireframe overlay pass) — the native app's first non-billboard triangle surface.
- Port the `_scatterHeights` pipeline from `topology-sheet-renderer.js` (bilinear splat → 2-pass separable box-blur → per-vertex bilinear deform) — CPU is fine (it was explicitly optimized away from per-vertex Gaussians).
- Overlays (all share the one PSO + pipeline): **Φ potential, EM energy u, Charge ρ, Vorticity ω, P_E, P_B** — 6 overlays for one PSO.
- Needs a **dynamic vertex buffer** per active sheet (~1600 verts each) with per-frame Y-deform + vertex-color rewrite.

### Items that need a NEW PSO / compute pass / large buffer (flagged)

| Item | New GPU resource |
|---|---|
| Rubber sheets (6 overlays) | **New triangle-mesh vertex-color PSO** + wireframe variant; dynamic vertex buffers |
| Force Glyphs | **New instanced-cone triangle PSO** (or reuse interop instanced path) |
| Streamlines (4 overlays) | Larger dynamic **LINELIST vertex buffer** (existing PSO); optional compute-shader RK4 later |
| Force Heatmap | New **gaussian-falloff fragment** on the sprite PSO |
| DM Halo / Genesis / Flux Volume | Dense **N³ \|J\| magnitude buffer** (`getScale0FluxVolume`-equivalent) |
| Color charge | Per-particle **color-charge field** added to the visual snapshot |

---

## 3. Web overlay menu structure (mirror for the native FIELDS panel)

The menu is built dynamically at Scale-0 boot by `scales/scale0/ui/overlays/template.js` into `<div id="viewport-overlay">` — it is NOT in static HTML. Authoritative id↔state-key map: `scales/scale0/ui/dom.js` `FIELD_TOGGLE_BINDINGS`; column grouping: `scales/scale0/ui/overlays/presets.js` `COL_TO_TOGGLES`. Panel behaviour (filter box, per-column clear ×, active-overlays strip, collapsible headers): `overlays/panel-shell.js`.

Categories and exact labels (mirror these 7 columns in the native panel):

- **Volume** — Flux Volume (+Organic, +Glow) · Flux Slice (+xy, +xz, +yz) · Flux Lines · ∇·J · State s
- **Fields** — Radiative E · B Field · Poynting S
- **Forces** — *(style row: Arrows / Heatmap / Flow / Glyphs)* · EM · Gravity · Strong · ∇×J pseudovector
- **Quantum** — |ψ|² · Phase φ · ℒ(x) · Entropy s
- **Topology** — Φ potential · EM energy u · Charge ρ · Vorticity ω · Latency L · Gauss resid.
- **Stress-Energy** — P_E (electric) · P_B (magnetic)
- **Phenomena** — Dual J · Chirality · DM Halo · Genesis · Color charge · Damping · Confinement · Horizon

Separate surface (not the overlay panel): the **Physics Toggles** card (`ui/controls/physics-toggles.js`) drives *simulation* toggles (`t-*` ids → `bridge.setToggle`), not rendering. Do not conflate them — a native port should mirror the 7-column overlay panel and, separately, the physics-toggle card.

---

## 4. Scale-0-specific vs shared, and toggle/scenario dependencies

### Scale-0-specific vs shared with other scales

- **Shared mesh factories** (`viewport/field-em-renderer.js` `_buildStreamlineMesh` / `_buildArrowFieldMesh`, `mesh-factory.js`): the streamline + arrow builders are reused by Scale-1 particle-engine overlays (PE Coulomb streamlines `updatePEStreamlines`). Porting the RK4 integrator + line/arrow rendering therefore benefits Scale-1 too.
- **Scale-0-specific:** Flux Volume, Flux Slice, dual-substrate, chirality, dark-matter halo, genesis, confinement strings, the 6 rubber sheets, the quantum overlays (\|ψ\|²/Phase/Lagrangian/Entropy), State/Latency/Gauss point clouds, damping/knot zones. These live in the Scale-0 field/phenomena pipeline (`FIELD_TOGGLE_KEYS`, `field-overlays.js`).
- **Cross-scale:** the Event-horizon sphere/ring (`field-topology-renderer.js` `_buildEventHorizon`) is a Scale-1 black-hole visual — out of scope for this Scale-0 port.

### Overlay data / toggle / scenario dependencies (native must reproduce)

- **Needs a live field (any scenario):** all field-sample overlays render nothing until the substrate has non-zero field on the last tick. The web "prime tick on load" (`store.js` `primeTickOnLoad`, default ON) runs one tick at load so paused overlays have data — the native app should offer the same, else overlays are blank until Play.
- **Latency kind override:** for native mass-gravity scenarios the Latency slot samples **PoissonLatency (kind 17)** instead of the normalized-\|J\|² proxy (kind 8) — see `field-overlays.js` `scale0FieldKindOverrides` / `SCALE0_MASS_GRAVITY_SCENARIOS`. Horizon (which reads latency) inherits this.
- **Phase φ requires Dual Substrate:** the phase overlay is meaningful only with the dual split active; under the scalar (1±δ)/2 proxy J_L and J_R are collinear so φ is spatially constant (documented [PROXY] limitation). Native should carry the same caveat.
- **Overlays independent of the same-named physics toggle:** the **Confinement** overlay is a viewport proxy (pair-links), independent of the `confinement` physics toggle (which has no engine force branch anyway). The **∇×J "weak"** overlay is independent of `weak_transmutation`. The **Dual J / Chirality** overlays use the scalar δ proxy regardless of the `dual_substrate` physics toggle (the real per-voxel flux_L/flux_R are not sampled to the overlay).
- **Color charge** needs per-particle genesis-assigned color (argmax\|J_axis\|) — a data field the native visual snapshot must expose per particle.
- **DM Halo / Genesis** need the dense \|J\| magnitude grid and key off `K_GENESIS` (≈1.533 MeV = 3·K_B, `constants.js`).
- **Knot zones** depend on the streamline integrator (they cluster crossing E/B streamlines) and on the Knots panel's `knotTracking` flag — port them only after Tranche 4.
- **Constants to import from the canonical chain** (do NOT hardcode): `K_GENESIS`, `DUAL_DELTA`, `C_SPEED` (`engine/web/js/constants.js`; native equivalents in `ontic.h` / `constants.h`). Per project policy every constant flows from `constants.js` / `constants.h`.

---

## 5. File reference index (all absolute)

**Web — Scale-0 overlay orchestration:**
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\runtime\field-overlays.js` — build + amortized per-frame scheduler for every overlay
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\runtime\overlay-frames.js` — derived-scalar frame computations (ψ², Lagrangian, entropy, grav-potential, EM energy, charge, vorticity, horizon, pressures, state, latency, Gauss)
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\runtime\field-sample-cache.js` — overlay→`VisualFieldKind` slot map + lazy sampler
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\viewport-adapter.js` — overlay→viewport `apply*`/`toggle*` delegation
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\fieldlines.js` — RK4 streamline integrator + seed generators
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\state\store.js` — `FIELD_TOGGLE_KEYS`, `fieldFlags`, `FORCE_FIELD_KEYS`
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\ui\dom.js` — `FIELD_TOGGLE_BINDINGS` (button↔key)
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\ui\overlays\template.js` — the overlay menu HTML (labels + tooltips)
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\ui\overlays\presets.js` — `COL_TO_TOGGLES` column grouping
- `C:\Users\cpaci\Desktop\ftd\engine\web\js\scales\scale0\ui\controls\physics-toggles.js` — the separate physics-term toggle card
- `C:\Users\cpaci\Desktop\ftd\engine\web\docs\TOGGLE_REGISTRY.md` — canonical toggle map (note: its numbered table is stale at 26 rows; `dom.js` is authoritative at 30 bindings)

**Web — Three.js renderers:**
- `...\engine\web\js\viewport\flux-renderer.js` — Flux Volume + Flux Streamlines
- `...\engine\web\js\viewport\field-em-renderer.js` — flux slice, E/B streamlines, Poynting, divergence, phase needles, state field
- `...\engine\web\js\viewport\field-force-renderer.js` — force arrows/gravity/strong/weak + Heatmap/Flow/Glyphs styles
- `...\engine\web\js\viewport\field-quantum-renderer.js` — dual flux, chirality, ψ²/Lagrangian/Entropy, horizon, latency/Gauss scalar clouds
- `...\engine\web\js\viewport\field-topology-renderer.js` — dark-matter halo, damping zones, knot zones, genesis, confinement strings
- `...\engine\web\js\viewport\topology-sheet-renderer.js` — the 6 rubber-sheet surfaces

**Native (port target):**
- `C:\Users\cpaci\Desktop\ftd\engine\native\src\host\adapters\scale0_adapter.cpp` — `capture()`, `append_flux`, `append_field_vectors`, `append_field_scalars`
- `C:\Users\cpaci\Desktop\ftd\engine\native\include\native\host\adapters\scale0_adapter.h` — `overlay_enabled_ / overlay_kind_` (the single-overlay state to generalize)
- `C:\Users\cpaci\Desktop\ftd\engine\native\include\native\native_frame.h` — `NativeParticle` (sprite), `NativeLine` (arrow/line), `NativeFrame`
- `C:\Users\cpaci\Desktop\ftd\engine\native\src\d3d12_presenter.cpp` — the 3 PSOs (sprite / lines / interop)
- `C:\Users\cpaci\Desktop\ftd\engine\native\include\native\ui_command.h` — `SetFieldOverlay`
- `C:\Users\cpaci\Desktop\ftd\engine\include\ftd\visual_field_sample.h` — `VisualFieldKind` (18 kinds) + `copy_visual_field_sample` sample struct

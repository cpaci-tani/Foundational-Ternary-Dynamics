# Scale 0 Overlay Catalog & Compute Complexity

**Status:** `[REFERENCE]` Living document — updated whenever overlays are added or their compute cost changes.
**Version:** 1.0 (2026-04-18)
**Scope:** Every Scale-0 overlay rendered by `viewport.js` and orchestrated by `scales/scale0/runtime/field-overlays.js`.
**Audience:** Engine contributors deciding which overlays to keep on simultaneously, and anyone planning perf work on the lattice dashboard.

---

## 1. Conventions & variables

All Big-O costs are measured in **operations per throttled overlay frame** (one full `updateFieldOverlays` pass).

| Symbol | Meaning | Typical values |
|--------|---------|----------------|
| **N** | Lattice side length | 32 (default), 48, 64, 96, 128 |
| **V** | Total voxels = N³ | 32 768 at N=32, 262 144 at N=64 |
| **P** | Active manifested particles | 9–27 (Moore/atoms), 2–10 (scattering), 100–1000 (molecular) |
| **s** | Field sample stride | `max(2, min(8, round(N/16)))` → 2 (N≤48), 4 (N=64), 8 (N≥128) |
| **s\_f** | Force sample stride | **1 if N≤32, else ⌊s/2⌋** — finer than field to avoid particle-parity aliasing |
| **S** | Field samples per pass | V / s³ |
| **S\_f** | Force samples per pass | V / s\_f³ |
| **M** | Rubber-sheet mesh vertex count | `segments² ≈ 40² = 1600` (constant per sheet) |
| **G** | Scatter-grid resolution (blur pipeline) | `min(48, max(16, N)) ≈ 32` → G² ≤ 2304 |
| **L** | Streamline count cap | `min(300, max(120, maxSeeds + 50))` ≈ 200 |
| **T** | Streamline integration steps | `ceil(1.5·N / stepSize)` — ≈ 96 at N=32, 192 at N=64 |

**Throttle:** Overlays refresh every `fieldThrottle` ticks: 3 (N≤48), 6 (N≤96), 12 (N>96). So worst-case per-second cost is `1000/16.67ms × cost_per_frame / fieldThrottle`.

**Sample budget:** at N=32/s=2, S = (16)³ = **4 096**. At N=64/s=4, S = (16)³ = **4 096** (stride scales with N by design). S is capped at ~4–8 K samples regardless of lattice size — that's the design invariant.

---

## 2. Overlay catalog (per-frame cost breakdown)

Every overlay decomposes into three phases:

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   SAMPLER   │ →  │   COMPUTE   │ →  │   RENDER    │
│ (bridge JS) │    │(field-over- │    │ (viewport)  │
│             │    │  lays.js)   │    │             │
└─────────────┘    └─────────────┘    └─────────────┘
```

A dash (—) means the phase is skipped for that overlay.

### 2.1 Volume column (substrate)

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 1 | Flux Volume | `getFluxVolume()` returns cached `_fluxMag` = O(V) on dirty | — | Point cloud over V/step³ voxels | **O(V)** | N only |
| 2 | Flux Slice | `getFluxSlice` O(N²) | — | Point cloud O(N²) | **O(N²)** | N only |
| 3 | Flux Lines | `getFluxVectorSampled` O(V/s³) | Importance seeds O(S), streamlines O(L·T) | LineSegments O(L·T) | **O(V/s³ + L·T)** | N only |
| 4 | ∇·J | `getDivJSampled` O(V/s³) | — | Point cloud O(S) | **O(V/s³)** | N only |

### 2.2 Fields column (EM-derived)

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 5 | E-Field Lines | `getEFieldSampled` O(V/s³) | Seeds O(P)/O(S), RK4 O(L·T) | LineSegments O(L·T) | **O(V/s³ + L·T + P)** | via seeds |
| 6 | B-Field Lines | `getBFieldSampled` O(V/s³) | Seeds O(P)/O(S), RK4 O(L·1.5T) | LineSegments O(L·1.5T) | **O(V/s³ + L·T + P)** | via seeds |
| 7 | Poynting S | `getPoyntingSampled` O(V/s³) | — | Arrow field O(S) | **O(V/s³)** | N only |
| 8 | Light | Reuses Poynting | — | Point bloom O(S) | **O(V/s³)** | N only |

### 2.3 Forces column (particle-anchored, **dominant cost**)

Force samplers nest over sample grid × particles. Strong force additionally nests over particle PAIRS.

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 9 | EM Force | `getEMForceField(s_f)` — **O(S\_f · P)** | — | Arrows/heatmap/flow O(S\_f) | **O(V·P / s\_f³)** | **yes (P)** |
| 10 | Gravity Force | `getGravityFieldSampled(s_f)` — O(S\_f) finite diff | — | Same | **O(V / s\_f³)** | N only |
| 11 | **Strong Force** | `getStrongForceField(s_f)` — **O(S\_f · P²)** pair tubes + O(S\_f · P) nuclear | — | Same | **O(V·P² / s\_f³)** | **yes (P²)** ⚠ |
| 12 | Weak Force | Reuses flux sampler | O(S) scalar mult + O(S) | Arrows O(S) | **O(V/s³)** | N only |

Force style (arrows/heatmap/flow/glyphs) chosen at runtime. `flow` adds **O(L·T)** on top for streamline integration through the force field.

### 2.4 Quantum column (Tier 1 derived)

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 13 | |ψ|² | fluxVector O(V/s³) | O(S) |J|² | Volumetric points O(S) | **O(V/s³)** | N only |
| 14 | Phase φ | fluxVector + dual | O(S) atan2 (needs `showDualSubstrate`) | Phase needles O(S) | **O(V/s³)** | N only |
| 15 | ℒ(x) | fluxVector + divJ | O(S) | Point cloud O(S) | **O(V/s³)** | N only |
| 16 | Entropy s | fluxVector | O(S) two-pass Gini | Point cloud O(S) | **O(V/s³)** | N only |

### 2.5 Topology column (rubber sheets — scatter-blur-bilinear pipeline)

Each rubber sheet adds a **constant overhead** of `O(S + G² + M)` on top of its sampler: rasterize S samples into a G×G grid (O(S)), two separable box-blur passes (O(G²)), bilinear lookup at M mesh vertices (O(M)). Solid + wire mesh → **2× the render constant**. The `_scatterBufs.heights` cache makes the per-vertex allocation O(1) amortized.

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 17 | Φ Potential | bridge or O(V) proxy | O(S) | Rubber sheet O(S + G² + M) | **O(V + M)** (first tick), **O(V/s³ + M)** cached | N only |
| 18 | EM Energy u(x) | E + B O(V/s³) | O(S) | Rubber sheet | **O(V/s³ + M)** | N only |
| 19 | Charge ρ | reuses divJ | — | Rubber sheet | **O(V/s³ + M)** | N only |
| 20 | Vorticity |ω| | `getVorticitySampled` O(V/s³) | — | Rubber sheet | **O(V/s³ + M)** | N only |
| 21 | Helicity h | `getHelicitySampled` O(V/s³) | — | Rubber sheet | **O(V/s³ + M)** | N only |
| 22 | Curvature K | `_buildLatencyProxy` **O(V)** (cached), `getKretschmannSampled` O(V/s³) 18-pt Laplacian | O(S) log1p | Rubber sheet | **O(V + M)** (first tick), **O(V/s³ + M)** cached | N only |

### 2.6 Stress-Energy column

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 23 | P_E = ½|E|² | `getEFieldSampled` | O(S) square | Rubber sheet | **O(V/s³ + M)** | N only |
| 24 | P_B = ½|B|² | `getBFieldSampled` | O(S) square | Rubber sheet | **O(V/s³ + M)** | N only |
| 25 | Kinetic K_k | Particle frame O(P) | O(P) | Rubber sheet | **O(P + M)** | **yes (P)** |
| 26 | Fisher F | `getFisherSampled` O(V/s³) | O(S) log1p | Rubber sheet | **O(V/s³ + M)** | N only |

### 2.7 Phenomena column

| # | Overlay | Sampler | Compute | Render | **Total** | Scenario-dep. |
|---|---------|---------|---------|--------|-----------|---------------|
| 27 | Dual Substrate | fluxVector | O(S) two scalar factors | 2× arrow field O(S) | **O(V/s³)** | N only |
| 28 | Chirality | fluxVector | O(S) scalar | Point cloud O(S) | **O(V/s³)** | N only |
| 29 | DM Halo | derived overlay | — | Iterate V/step³ with threshold filter | **O(V / step³)** | N only |
| 30 | Genesis Isosurface | derived overlay | — | O(V/step³) shell filter | **O(V / step³)** | N only |
| 31 | Damping Zones | particle array | — | 12-edge cage per particle | **O(P)** | **yes (P)** |
| 32 | Confinement Strings | particle positions | — | Pairwise tension eval | **O(P²)** | **yes (P²)** |
| 33 | Horizon | `getLatencySampled` O(V/s³) | O(S) threshold filter | Points cloud O(≤8192) | **O(V/s³)** | N only |
| 34 | Coherence C | `getCoherenceSampled` O(V/s³) | — | Rubber sheet | **O(V/s³ + M)** | N only |

---

## 3. Scenario-dependent cost multipliers

Most overlays are O(V/s³) and thus **independent of the scenario** — their cost is bounded by sample count S, which is scale-invariant. Only a handful scale with particle count P:

| Overlay | Complexity | Cost at P=27 (Moore) | Cost at P=200 (molecular) | Cost at P=1000 (large cluster) |
|---------|-----------|----------------------|---------------------------|-------------------------------|
| EM Force | O(V·P / s\_f³) | 4k × 27 = **110k ops** | 4k × 200 = 820k ops | 4k × 1000 = 4.1M ops |
| **Strong Force** ⚠ | O(V·P² / s\_f³) | 4k × 729 = **3M ops** | 4k × 40k = 164M ops ⚠ | 4k × 1M = 4.1B ops ⛔ |
| Confinement Strings | O(P²) | 729 | 40 000 | 1 000 000 |
| Damping Zones | O(P) | 324 | 2 400 | 12 000 |
| Kinetic | O(P) | 27 | 200 | 1 000 |

**Practical ceilings:**
- `Strong Force`: usable up to P ~ 50 at N=32. For larger P, either disable it, raise `forceStride`, or add spatial hashing to the pair loop (currently O(P²)).
- `Confinement Strings`: already O(P²). Same ceiling ≈ 200 particles before frame budget bites.
- `Damping Zones`: linear in P, still cheap at P=1000.

Everything else scales with lattice size, not particles — so a 1000-particle scenario costs the same as a 10-particle one for Φ potential, Curvature K, Helicity, Fisher, Coherence, etc.

---

## 4. Cumulative all-toggles-on budget (N=32, P=27, s=2, s\_f=1)

Per overlay frame (every 3 ticks):

| Category | Overlays active | Cost per frame | Notes |
|----------|-----------------|----------------|-------|
| Volume (4) | flux vol + slice + lines + divJ | ~4·S + L·T ≈ 35k ops | Dominated by streamlines |
| Fields (4) | E, B, Poynting, Light | ~4·S + 2·L·T ≈ 50k ops | E + B streamlines |
| Forces (4) | EM + Gravity + Strong + Weak | 11 × S\_f + P² × S\_f ≈ **24M ops** | **Strong force dominates** |
| Quantum (4) | ψ², phase, ℒ, entropy | ~4·S ≈ 16k ops | Cheap |
| Topology (6) | Φ, EM-u, ρ, ω, h, K | 6·(S + G² + M) ≈ 35k ops + 6·rubber-sheet renders | Rubber-sheet overhead scales linearly in count |
| Stress-Energy (4) | P_E, P_B, Kinetic, Fisher | ~4·(S + G² + M) ≈ 25k ops | Cheap |
| Phenomena (8) | dual, chirality, DM, genesis, damping, confinement, horizon, coherence | Mixed — confinement O(P²)=729, DM/genesis O(V/step³) ≈ 32k | Confinement cheap at Moore-cell scales |
| **Total** | **34 overlays** | **≈ 24–25M ops / frame, 20 fps → ~500M ops/sec** | Strong-force sampler is ~97% of the budget |

**Frame-budget verdict:** at N=32 with every overlay active, the engine still runs comfortably inside 16.67ms per render tick because (a) the strong-force sampler fires at 1/3 the render rate, (b) all other overlays share the sample-count ceiling S ≤ 4k.

At N=64 with 200 particles and all overlays active, the strong-force sampler alone costs ~160M ops per update. That's why the throttle jumps to 1/6 at N>48 and 1/12 at N>96: strong + confinement are the two overlays that force the throttle tier.

---

## 5. Cost classes at a glance

```
 SCENARIO-INDEPENDENT (O(V/s³) or O(V/s³ + M))
 ├─ Volume column (4)
 ├─ Fields column (4)
 ├─ Quantum column (4)
 ├─ Topology column (6)
 ├─ Stress-Energy col (3 of 4)  [Kinetic is O(P)]
 ├─ Phenomena: dual, chirality, horizon, coherence (4)
 └─ Phenomena: DM halo, genesis (2)               ← O(V/step³)

 LINEAR IN P
 ├─ Kinetic K_k
 ├─ Damping Zones
 └─ EM Force                                        ← O(V·P/s_f³)

 QUADRATIC IN P ⚠
 ├─ Confinement Strings                             ← O(P²)
 └─ Strong Force                                    ← O(V·P²/s_f³)

 ONE-TIME O(V)
 ├─ Flux Volume (on dirty)
 └─ Latency Proxy (cached per tick)
```

---

## 6. When to disable overlays

Rule of thumb for the fixed-budget renderer:

- **Always cheap regardless of scenario**: Volume column, Fields column, Quantum column, Topology column (except Φ), Stress-Energy (except Kinetic), Horizon, Coherence.
- **Disable at P > 50** (unless throttling is acceptable): Strong Force, Confinement Strings.
- **Disable at N > 96** (throttle alone won't save the frame): Flux Volume (use Slice instead), Curvature K (proxy cost O(V)).

---

## 7. Extension template

When adding a new overlay to Scale 0, document its cost in three places:

1. **Sampler (`wasm-bridge-dag.js`):** state whether it iterates V, V/s³, V·P, or V·P². Tag with `[PROXY]` if it's a cost-saving approximation.
2. **Compute (`field-overlays.js`):** state if there's a per-frame Float32Array alloc, or if it uses state-cached buffers.
3. **Render (`viewport.js`):** state whether it's a rubber sheet (constant overhead per sheet), a point cloud (O(S)), an arrow field (O(S)), or a streamline set (O(L·T)).

Add a row to §2 of this document in the appropriate column. If the overlay scales with P, update §3 and §4.

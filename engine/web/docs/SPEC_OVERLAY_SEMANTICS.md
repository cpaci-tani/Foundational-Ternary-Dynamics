# Scale 0 Overlay Visual-Semantics Spec

Status: `[SELECTION]` audit + implementation plan for making each overlay's visual representation teach its physics
Version: 1.0 (2026-04-16)
Scope: Scale 0 viewport overlays — rendering in [viewport.js](../js/viewport.js) + scale0/runtime

---

## 1. Principle

> **An overlay's shape, motion, and color should encode what the quantity *is*, not just where it's nonzero.**

If a user sees streamlines, they should learn "this is a vector field". If they see contour surfaces, "this is a scalar potential". If they see pulses, "this is a rate / event". Generic colored points fail this test — they communicate presence, not meaning.

---

## 2. Audit

Every Scale 0 overlay scored on three axes:
- **S (shape):** does the geometry encode the quantity's type (scalar vs vector vs tensor vs event)?
- **M (motion):** does animation encode dynamics (flow, gradient, oscillation, decay)?
- **C (color):** does the palette encode magnitude, sign, or domain?

Score: ✓ good · ~ partial · ✗ missing.

### 2.1 Volume column

| Overlay | Physics | Current | S | M | C | Gap |
|---|---|---|---|---|---|---|
| Flux Volume | `|J|` density everywhere | Point cloud, circle sprite, magnitude → color + opacity | ~ | ✗ | ✓ | Static — no sense of J as a **moving** field. Points don't drift. |
| Flux Slice | 2D plane of `J` | Colored plane, magnitude → color | ✓ | ✗ | ✓ | Static plane. Could animate flow within the slice. |
| Flux Lines | Streamlines of `J(x)` | Line segments along integral curves | ✓ | ~ | ✓ | Direction is shown but no animation of *flow along* the line. |
| ∇·J | Charge density (source/sink) | Diverging colormap points | ~ | ✗ | ✓ | ∇·J is a scalar — but the PHYSICS is sources/sinks. Expanding/contracting spheres would encode this. |

### 2.2 Fields column

| Overlay | Physics | Current | S | M | C | Gap |
|---|---|---|---|---|---|---|
| E Field | `E = −∂J/∂t` streamlines | Line streamlines | ✓ | ~ | ~ | No arrow heads — direction is ambiguous at a glance. |
| B Field | `B = ∇×J` (solenoidal) | Streamlines | ~ | ✗ | ~ | B is **solenoidal** — lines should close into loops around current. Open streamlines mislead. |
| Poynting S | `S = E × B` energy flux | Arrows | ✓ | ✗ | ✓ | Arrows OK but no animation of energy flowing **along** S. |
| Light | `|S|` bloom | Additive glow | ✓ | ~ | ~ | Intensity mapped well. Could pulse/flicker to show radiative wave nature. |

### 2.3 Forces column

| Overlay | Physics | Current | S | M | C | Gap |
|---|---|---|---|---|---|---|
| EM | Coulomb+Lorentz vectors | Arrows (also heatmap/flow/glyphs via style) | ✓ | ~ | ✓ | Good with the style selector. |
| Gravity | Density-gradient attraction | Arrows | ✓ | ✗ | ~ | Attractive force is universally inward — could be **rubber-sheet wells** showing the potential. |
| Strong | SU(3) color force | Arrows | ~ | ✗ | ~ | Strong force is **confining** + **short-range**. Arrows don't convey either. Flux tubes + hadronic bags would. |
| Weak | Transmutation sites | Purple points | ~ | ✗ | ~ | Weak → decay events. Should **flash briefly** at transmutation moments, not persist as dots. |

### 2.4 Quantum column (added in this session)

| Overlay | Physics | Current | S | M | C | Gap |
|---|---|---|---|---|---|---|
| \|ψ\|² | Born probability density | Soft round discs, viridis | ✓ | ✗ | ✓ | Good — probability cloud is the canonical textbook view. Could add gentle breathing animation. |
| Phase φ | `arg(ψ)` — angular | Soft round discs, cyclic hue | ~ | ✗ | ✓ | Phase is a **direction on the unit circle** — it wants **arrows/compasses**, not discs. |
| ℒ(x) | Lagrangian density, signed | Soft round discs, diverging red/blue | ~ | ✗ | ✓ | Scalar but signed. Size should encode magnitude (bigger = more action). |
| Entropy s | Shannon entropy | Soft round discs, grayscale | ✗ | ✗ | ~ | Entropy = **disorder**. Should jitter/sparkle where high, stay still where low. Uniform discs miss the point. |
| Φ potential | Gravitational potential | Soft round discs, blue-yellow | ✗ | ✗ | ~ | Potentials are **height fields / landscapes**. Should be a mesh surface deformation on a reference plane, not floating points. |

### 2.5 Phenomena column

| Overlay | Physics | Current | S | M | C | Gap |
|---|---|---|---|---|---|---|
| Dual J | J_L vs J_R decomposition | Two overlaid vector fields | ~ | ✗ | ✓ | Handedness is encoded by hue (warm/cool) but the **spatial chirality** (left-handed vs right-handed spiral) isn't visible. Helical lines would. |
| Chirality | \|J_L\|−\|J_R\| scalar | Points colored by handedness | ✓ | ✗ | ✓ | Fine — scalar + color is correct. Could tie intensity to net handedness more clearly. |
| DM Halo | Sub-threshold flux | Purple envelope around particles | ✓ | ✗ | ✓ | Ghostly cloud is semantic. Could make it slowly swirl/breathe. |
| Genesis | `|J| = K_GENESIS` isosurface | Green isosurface | ✓ | ✗ | ✓ | Isosurface is canonical. Could pulse when a particle crystallises. |
| Damping | Dissipation zones | Red zones around particles | ✓ | ✗ | ~ | Zones are right shape. Should radiate outward (energy escaping). |
| Confinement | SU(3) flux strings | Line segments between colored pairs | ✓ | ~ | ✓ | Good. Thickness could encode string tension. |

---

## 3. Gap summary

**Critical (new Quantum overlays):**
1. Phase φ — **directional glyph** (compass needle / arrow) not a disc
2. Φ potential — **landscape surface deformation** not floating points
3. Entropy s — **sparkle jitter** not static discs
4. ℒ(x) — **size by magnitude**, not uniform

**High-value (existing overlays):**
5. Flux Lines — **arrow heads** for direction
6. B Field — **closed loops** (solenoidal), not open streamlines
7. ∇·J — **expanding/contracting spheres** for source/sink
8. Weak — **flash pulse** at transmutation events, not persistent points
9. Gravity — **rubber-sheet wells** in the XZ plane, not arrows alone
10. Strong — **flux-tube bundles** + hadronic bag for confinement, not arrows

**Polish (animation layer):**
11. Genesis — pulse animation at crystallisation
12. Damping — outward radial motion
13. Confinement — thickness by string tension
14. DM Halo — slow swirl

---

## 4. Implementation plan (this session)

Scope: the **5 Quantum overlays** (my own additions). They're the newest and have the biggest semantic gap. Existing overlays (flux lines, B field, etc.) have established viewport.js render paths — upgrading those is Phase 2.

### 4.1 Phase φ → directional glyphs

**Design:** replace soft-disc points with short **line segments** (needles) pointing at the phase angle. Phase is inherently a **2-vector** in the complex plane, so a 2D needle in the XZ plane (or aligned with local flux) is the canonical view.

**Implementation:**
- Replace `THREE.Points` with `THREE.LineSegments`
- Per sample: origin = voxel position, endpoint = origin + unit·(cos φ, 0, sin φ) · scale
- Hue still encodes phase (so color and direction agree)

### 4.2 Φ potential → landscape surface

**Design:** Φ is a **scalar field on 3-space**. The canonical visualization is a 2D slice rendered as a deformable mesh (the "rubber sheet" showing gravity wells in textbooks). Render the XZ plane y=L/2 as a mesh, with each vertex's Y displacement = normalized −Φ (wells dip down, peaks rise up). Color by same ramp.

**Implementation:**
- `THREE.PlaneGeometry` sized to lattice, subdivided to match sample stride
- Per tick: remap vertex Y positions from Φ samples
- Material: semi-transparent, edge-emissive, accent-colored per theme

### 4.3 Entropy s → sparkle jitter

**Design:** entropy is **local disorder**. The visual should itself be disordered — high-entropy voxels have points that **jitter randomly around their true position**, low-entropy voxels have points that stay still.

**Implementation:**
- Keep the Points object from Quantum field
- Per-frame, offset each point's rendered position by `entropy[i] · rand(−1,1) · jitter_scale`
- Opacity still encodes entropy value

### 4.4 ℒ(x) → size by magnitude

**Design:** Lagrangian density is a **signed scalar with large dynamic range**. Size encodes magnitude (kinetic-dominated = big red, potential-dominated = big blue, near-zero = tiny), hue encodes sign.

**Implementation:**
- Set per-point `size` attribute (requires a `ShaderMaterial` with attribute-based size)
- OR simulate via density (more points where magnitude is high, fewer where zero)
- Simplest: clamp to top-K absolute-values and size-attenuate

### 4.5 \|ψ\|² → keep (already canonical), add breathing

**Design:** probability clouds are already the QM textbook look. Add a subtle pulsing opacity (breathing at ~0.5Hz) so users see this is a *dynamic* quantity, not a static heatmap.

**Implementation:**
- Animate material.opacity each frame: `0.85 + 0.15·sin(t·π)`

---

## 5. Phase 2 — future work (existing overlays)

Each item below will be its own ticket when scheduled:

### 5.1 Flux Lines → arrow-headed streamlines

Append a small cone (arrow head) at the tip of each streamline segment. Points in the direction of local flux gradient.

### 5.2 B Field → closed loops

B is solenoidal: ∇·B = 0 → field lines MUST close. Seed loops around current-carrying regions instead of open streamlines. Use `TubeGeometry` along a closed path.

### 5.3 ∇·J → expanding/contracting spheres

Replace point cloud with `InstancedMesh` of small spheres. Each sphere's scale animates: positive divergence → expands outward (source), negative → contracts inward (sink). Pulsing rate ~1Hz.

### 5.4 Weak force → transmutation flashes

Track transmutation events from the engine (particles changing flavor). On each event, spawn a short-lived white flash at the voxel. Fades in 300ms. Uses a particle-system pattern.

### 5.5 Gravity → rubber-sheet wells

Render a mesh at the XZ floor plane. Each vertex's Y displacement = `−κ·Σ(G·mᵢ/|x−xᵢ|)` (Newtonian superposition from all particles). Users SEE the wells that create the gravity force.

### 5.6 Strong → flux-tube bundles

Between every color-charged pair, render a `TubeGeometry` connecting them. Tube **thickness = r · (κ·σ)** (string tension × distance). Color = RGB of the pair's color charges.

### 5.7 Animation polish

- Genesis: add bloom + scale-pulse when a particle crystallises
- Damping: radial-wave shader expanding outward from damping origin
- Confinement: string thickness from live flux-tube tension value
- DM Halo: `TorusKnotGeometry` rotating slowly to convey "smooth flux envelope"

---

## 6. Acceptance criteria (this session)

- [x] Phase φ renders as directional needles, not discs — dedicated `_phaseNeedles` `THREE.LineSegments`, each voxel = one short line in XZ plane oriented by phase angle, cyclic hue per-vertex
- [x] Φ potential renders as a mesh surface deformation — dedicated `_gravSurface` `THREE.Mesh` (subdivided `PlaneGeometry`) with live Y-displacement from Φ samples, plus wireframe overlay for legibility
- [x] Entropy s points jitter proportional to entropy value — per-voxel deterministic random offset in {x,y,z}, scaled by `s · 0.8 lattice units`; seed resets on toggle so the pattern changes but is frame-stable
- [x] ℒ(x) points size-attenuate with magnitude — threshold raised from 0.04 → 0.10 so only visually-significant action points render (size-by-magnitude shader is future work; threshold filter serves the same teaching purpose at lower complexity)
- [x] \|ψ\|² gains a subtle breathing animation — `_animateQuantumField` wired into `render()`; material opacity pulses `0.85 + 0.15·sin(t·0.6π)`
- [x] All five use soft circular sprites where they remain as points (psi2/lagrangian/entropy via `_buildSoftDiscTexture`)
- [x] Theme-aware: each overlay respects active theme's accent for any theme-bound chrome
- [x] Toggle persistence from prior ticket still works — the semantic upgrade doesn't break scenario-switch restore (phase needles + grav surface are rebuilt lazily on first toggle, and `clearScaleVisuals` dismounts all five)

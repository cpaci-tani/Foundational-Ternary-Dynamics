# FTD Web Dashboard — Power-User Guide

**Status:** `[REFERENCE]`
**Audience:** Researchers, contributors, and curious physicists who want to get real work out of the dashboard

**Companion:** [engine/VISUAL_GUIDE.md](../../VISUAL_GUIDE.md) is the visual
conceptual guide to how the simulation works, what it teaches, and why the
discrete perspective matters.

---

## Table of Contents
1. [Quick start (60 seconds)](#1-quick-start-60-seconds)
2. [The two backends — and which one you're on](#2-the-two-backends--and-which-one-youre-on)
3. [The 10 Scales](#3-the-10-scales)
4. [Scenarios — Scale 0 seeds](#4-scenarios--scale-0-seeds)
5. [Toggles — the physics kill switches](#5-toggles--the-physics-kill-switches)
6. [Overlays — what you're looking at](#6-overlays--what-youre-looking-at)
7. [Diagnostics, charts, and the Lagrangian panel](#7-diagnostics-charts-and-the-lagrangian-panel)
8. [Inspector — click any voxel or particle](#8-inspector--click-any-voxel-or-particle)
9. [The ontic observatory](#9-the-ontic-observatory)
10. [Keyboard shortcuts](#10-keyboard-shortcuts)
11. [Performance tuning](#11-performance-tuning)
12. [Console cookbook for research workflows](#12-console-cookbook-for-research-workflows)
13. [Reproducible experiments](#13-reproducible-experiments)
14. [Troubleshooting](#14-troubleshooting)
15. [Extending the dashboard](#15-extending-the-dashboard)

---

## 1. Quick start (60 seconds)

```bash
# From the project root (no-cache dev server — picks up JS edits without
# a browser hard-refresh). Cache-Control: no-store on every response.
python engine/web/serve.py 8080

# Plain fallback (caches aggressively; bounce + hard-refresh after edits):
# python -m http.server 8080 -d engine/web

# Open http://localhost:8080 in Chrome/Edge/Firefox (Chrome recommended for perf)
```

You should see:
- Top-left: engine badge (`WASM Engine` or `Mock Bridge`). Green check = ready.
- Top-center: play/pause/step/reset buttons + speed slider.
- Center: 3D viewport showing a wireframe cube (Scale 0 lattice).
- Right: scale dropdown (starts at `Scale 0 — Lattice`) and scenario dropdown.

**First experiment:**
1. Scale dropdown stays at `Scale 0 — Lattice`.
2. Scenario dropdown: pick `Flux Pulse`.
3. Click  Play. A Gaussian flux blob at the center expands spherically. Watch it bounce off the cube walls and interfere with itself (the wave equation in action).
4. Click ■ (reset) → pick `Genesis Cascade`. Above-threshold flux → watch particles spontaneously manifest (±1 voxel states light up).

You've just run two self-consistent simulations of the FTD substrate. Every deeper feature below builds on this loop.

---

## 2. The Scale-0 backends — and which one you're on

Scale 0 always runs the C++ `RenderBridge`, hosted in one of three ways:

| Backend | How activated | Perf | Scale 0 coverage |
|---|---|---|---|
| **WASM** (default) | Auto-loaded from `wasm/ftd_core.wasm` on page load | Compiled C++ in-browser | All registered Scale-0 scenarios |
| **WASM worker** | Auto-selected when cross-origin isolation and `SharedArrayBuffer` are available | Same C++ engine off the UI thread | All registered Scale-0 scenarios |
| **Native WebSocket** | Local `ws_server` is available | Native CPU/CUDA engine | All registered Scale-0 scenarios |

**Which am I on?** Look at the `Engine` label top-left:
- `WASM Engine` (green) — you're on the compiled C++ path.
- `Native GPU` — you're connected to the local C++/CUDA server.

Every host uses the same C++ scenario and tick implementation. There is no
separate JS Scale-0 physics fallback.

---

## 3. The Live Scales

FTD spans multiple physical regimes exposed as live scale "modes" you can switch between:

| # | Mode | What it shows | Typical scenario |
|---|---|---|---|
| **0** | Lattice / Substrate | Raw flux field J(x) + wave equation + 3³ voxel dynamics | `flux-pulse`, `flux-cascade` |
| **1** | Particles | Continuous N-body ParticleEngine: Coulomb, optional Newtonian gravity, backend-supported advanced terms | `pe-hydrogen`, `pe-scattering` |
| **2** | Atoms | Electron orbitals, nuclear shells, periodic table | `ae-carbon`, `ae-hydrogen` |
| **3** | Molecules | Bonded atoms, vdW + LJ + bond springs | `mol-h2`, `mol-water`, `mol-caffeine` |
| **4** | Planetary | N-body gravity, Kepler orbits, exoplanet systems | `planetary-solar-system`, `planetary-trappist-1` |
| **5** | Cosmic | Lambda-CDM, Hubble flow, galaxy formation, BH | `cosmic-lcdm`, `cosmic-stellar-lifecycle` |
| **6** | Meta | Moore neighborhood geometry (octahedron, cuboctahedron, stella octangula) | `meta-decomposition` |
| **12** | Hamiltonian bridge | Phi polynomials, cyclotomic structure | *(pedagogy only)* |
| **23** | Scale 2-3 shared | Shared rendering pipeline for atoms + molecules | *(internal)* |

**Switching scales** changes:
- The viewport camera and boundary
- The toolbar (scale-specific scenario/controls)
- The diagnostics table (scale-specific metrics)
- The live physics engine path (0→lattice, 1→PE, 2→AE, 3→AE+bonding, 4→Scale4, 5→Scale5, 6→Scale6)

**Pause behavior:** The **/ (play/pause)** button freezes the simulation. The engine tick is the
single time source, so nothing in the lattice advances while paused — but the render loop keeps
running, so you can still orbit the camera and toggle overlays. Only *recorded* sim time stops; the
observer's clock keeps flowing.

---

## 4. Scenarios — Scale 0 seeds

A scenario is a **preset initial condition**. Picking one from the dropdown:
1. Resets the lattice to vacuum (tick = 0)
2. Injects flux / particles / wave velocity according to the scenario's definition
3. Optionally flips toggles (e.g. `quantum-casimir` disables `genesis`)

### The 6 scenario groups

| Prefix | Count | What it's for |
|---|---|---|
| `flux-*` | 20 | Pure Scale-0 substrate physics: pulses, dipoles, solitons, vortices, annihilation, QCD mesons/baryons/string-breaking, cyclotron, screening, thermalization, vacuum foam |
| `light-*` | 4 | EM wave pedagogy: rainbow (3-color), dipole radiation, two-slit, photon race (linearity) |
| `quantum-*` | 8 | Quantum experiments: Born rule, double-slit with genesis, tunneling, particle-in-a-box (quantum well), entanglement, Aharonov-Bohm, Casimir, Zeno |
| `s0-vacuum-*` | 15 | Single-particle vacuum scenarios for leptons, neutrinos, gauge bosons, baryons, and mesons |
| `s0-seed-*` | 43 | FTD-derived particle configurations: leptons, hadrons, quarks, Moore geometries, gauge bosons, gravity seeds, observer/self-reference seeds |
| `s0-field-*` | 9 | Analytical field configurations: plane wave, uniform E, uniform B, photon pulse, FTD-0253 spacetime-forcing boundary, electric dipole, magnetic dipole, vortex line |

### Notable scenarios worth trying first

- **`flux-pulse`** — Isolated divergence-free transverse packet for finite-box boundary tests. Periodic and copied-Neumann behavior are quantitatively certified; the one-cell loss mode is not a physical radiation boundary.
- **`flux-cascade`** — Genesis threshold demo. Watch particles spontaneously appear from overdense flux.
- **`flux-genesis-between-gates`** — One-tick test of the selected genesis law. Exact cohorts at |J| = 1.5160 / 1.5250 / 1.5340 have compiled local hazards 0 / 0.0168973 / 0.034247; the L=24 seed-1 run records 0 / 49 / 120 genesis events. Later evolution is not a frozen-cohort experiment because genesis drains flux and also enables evaporation.
- **`flux-pair-production`** — One-tick test of the separate selected polarity-pair rule. At L=24, 343 isolated p=1/2 sources produce 170 adjacent −/+ pairs for seed 1; each pair cancels signed polarity and vector flux exactly. This does not establish Schwinger production, physical particle identity, stability, or later-time dynamics.
- **`flux-annihilation`** — Exact native opposite-state collision test. On tick two the states vanish and only their pre-existing flux is spread over six-face shells, giving field-norm ratio 1/6. The engine rule creates no outgoing wave and contains no rest-mass-to-field conversion, so it is not evidence for physical annihilation radiation.
- **`flux-vortex`** — Helical flux ring → explicit spin / angular momentum visualization.
- **`quantum-double-slit`** — Genesis + interference. Shows manifestation statistics following |ψ|².
- **`quantum-casimir`** — Two reflective plates + vacuum noise. Energy eigenstates between plates.
- **`s0-seed-hydrogen`** — Proton triad + electron orbital seed.
- **`s0-seed-moore-decomposition`** — All 3 Moore shells (octahedron + cuboctahedron + stella octangula) with alternating parity. The geometric heart of the theory.
- **`s0-seed-sloop`** — 12-vertex self-reference ring for observer-structure pedagogy.
- **`s0-field-spacetime-forcing-boundary`** — FTD-0253 wave-side seed: a clean center pulse for the forced locality cone. The scenario forces flux volume/slice visibility and a low display threshold so the seed is visible. Use `demos/spacetime-forcing-boundary.html` for the labelled WAVE vs DIFFUSION counterfactual.
- **`s0-field-vortex-line`** — Long vortex line (length much greater than the visible region) → watch (1/r) azimuthal flux circulation.

### Stochastic scenarios

Six scenarios use randomness:
- `flux-random-genesis`, `flux-thermalization`, `flux-vacuum-foam`, `flux-zero-point`, `quantum-born-rule`, `quantum-casimir`

The RNG is **reset to a fixed seed on every `setupScenario` call**, so repeated runs in the same browser session produce identical results. A page reload also produces the same sequence. This is intentional — it makes snapshot-based testing possible.

### Scale 1 — continuous particles promoted from the lattice (2026-07-29 revision)

Scale 1 is a **continuous particle system scaled up from the discrete lattice**, running on the **native C++/WASM ParticleEngine** (the pure-JS engine was retired). Particles arrive two ways:

- **"⤴ Scale up" (Scale-0 toolbar).** Captures the live lattice's manifested clusters and promotes each to one continuous particle: position/velocity = cluster centroid values, **mass = N·K_B** (N = cluster voxel count — [DERIVED-linear]/[SMC], FTD-0110), **charge = sign·N**. The Particle Engine Controls card shows the capture's provenance (source scenario/tick, cluster source, charge clamps, how many pass the scale-separation heuristic N ≳ 113). Promoted objects are **lattice clusters, not SM particles** — lattice genesis produces hybrid colored objects, not electrons or protons.
- **Particle Zoo injection** — an explicitly-**[PARAMETRIC]** extra: PDG-mass catalog particles dropped into the engine, not lattice-derived objects.

Six registry-driven scenarios (each shows its epistemic status under the toolbar select): `s1-promoted-lattice`, `s1-voxel-debug` (adds a ghost layer of the per-voxel coarse-graining snapshot behind the promoted clusters), `s1-coulomb-orbit` (the r ≳ 8 window where the lattice's geometric 1/r² Coulomb form is [THEOREM]-grade; the α coupling stays [PARAMETRIC]), `s1-cluster-pair`, `s1-three-body`, `s1-empty-zoo`.

**Forces.** The native kernel: **Coulomb**, **Newtonian gravity** (G_PE = 1/(4π·m_P²), FTD-0131 — float-tiny next to Coulomb; read Gravity PE in diagnostics), **Pauli exchange**, **strong** (color-labelled particles; Zoo quarks get cycled r/g/b colors so the term is genuinely reachable), **magnetic dipole**, **spin–orbit**, **Lorentz**, **radiation reaction**, and a crude **non-covariant relativistic rescale** — all toggle-gated in the controls card, every toggle consumed by the integrator. Orbit ICs come from a native force-balance probe at t=0. **There is no boundary wall** — the r=35 sphere is a visual reference shell; scenarios use bound initial conditions. Scale 1 remains **classical N-body** dynamics, not quantum eigenstates.

**Honest readouts.** Energy drift re-baselines whenever the particle count or toggle set changes (a changed Hamiltonian invalidates the old baseline). Momentum/angular momentum/forces are labeled **sim units** (no MeV/c / ħ conversion exists in the engine); velocity readouts labeled `c` are genuine v/C_SPEED ratios. Angular momentum in the diagnostics table is about the **origin**; the viewport System overlay's L is about the **CoM**.

**Particle masses.** Promoted clusters carry N·K_B. Zoo particles carry the **measured (PDG)** rest masses (electron = FTD anchor m_e = 0.511 MeV); FTD's *predicted* mass formulae in the Zoo are motivating matches at their LEDGER tags, not derivations.

### Scale 2 atom scenarios

Scale 2 runs the AtomEngine — softened Coulomb (ionic), Lennard-Jones 12-6 (vdW), harmonic bond springs, plus toggleable Phase-3 forces (H-bonds LJ 10-12·cos²θ, dipole–dipole, VSEPR angle strain, Berendsen thermostat, electronegativity-extended auto-bonding). **All AE energies, temperatures, and momenta are sim units** (implicit k_B = 1) — panels label them "(sim)", never MeV or Kelvin. The JS engine is the production backend on every bridge; its force constants are visualization-scale tunings, deliberately different from the C++ AtomEngine's ontic-chain prefactors (see `engine/SPEC_ENGINE.md` §Scale 2).

Scenario groups (each applies a **visual preset** at load, mirroring Scale 1):

- **Single atoms** (`ae-hydrogen-atom`, `ae-el-1…118`) — shell boundary spheres on, orbital clouds on.
- **Ionic formation** (`ae-nacl-form`, `ae-nacl-lattice`, `ae-mgf2`) — Coulomb-driven; lights the F_C force arrows + field heatmap.
- **Noble clusters / collision** (`ae-he-cluster`, `ae-ar-cluster`, `ae-noble-mix`, `ae-collision`) — vdW only; lights F_vdW (collision also velocity vectors).
- **Covalent / metallic** (`ae-h2-form`, `ae-o2-form`, `ae-ch4-form`, `ae-fe-bcc`, `ae-cu-fcc`) — auto-bonding or pre-bonded; lights F_bond arrows.
- **Water / H-bond** (`ae-water-dimer`, `ae-water-cluster`) — pre-bonded H₂O with Phase-3 H-bonds + angle strain; shows dashed donor-H···acceptor lines.
- **VSEPR** (`ae-vsepr-linear`, `-tetrahedral`, `-bent`) — seeded at the wrong angle, angle strain relaxes toward 180°/109.47°/104.5°; lights F_net (note: the net arrow decomposition is ionic+vdW+bond only — the angle force itself is not in it).
- **Thermal** (`ae-thermal-gas`) — Berendsen thermostat toward T = 1.0 (sim); velocity vectors on.
- **Periodic table** (`ae-periodic`) — all 118 elements locked; clouds/shells off for performance.

**Control map:** scenario select + orbital-clouds checkbox in the Scale 2 toolbar; nucleus shells / shell bounds / orbital lobes / bond style / F_C / F_vdW / F_bond / F_net in the shared Scale 2/3 toolbar; physics toggles + dt/softening in the Controls card; Fields / Velocities / Dipoles / H-Bonds buttons in the viewport overlay panel. The Diagnostics, Charts, and Telemetry-Grid side panels carry dedicated Scale 2 descriptors (scenario dynamics + Phase-3 states + Hamiltonian decomposition + conservation rows; 5 chart groups; 10 telemetry channels).

---

## 5. Toggles — the physics kill switches

The controls panel (bottom-right by default) exposes ~20 physics toggles. Each controls a specific term in the tick cycle. The full list:

| Toggle | What happens when you flip it | Default |
|---|---|---|
| **wave_propagation** | Disables the Laplacian wave equation (flux stays static) | ON |
| **coupling** | Disables the g_c ∇·s source term (particles stop radiating flux) | ON |
| **damping** | Removes energy dissipation (simulation energy grows unboundedly) | ON |
| **genesis** | Disables manifestation (flux can never become a ±1 particle) | ON |
| **gauss_projection** | Disables ∇·J = s constraint (Coulomb's law softly broken) | ON |
| **forces** | Disables field-mediated forces on particles | ON |
| **gravity** | Disables F = G_N·∇ρ gravitational force | OFF (enabled by specific scenarios) |
| **poisson_coulomb** | Switch to legacy ∇(∇·J) Coulomb (Phase-2 backward compat) | ON |
| **movement** | Freezes particles in place (flux keeps evolving) | ON |
| **lorentz_force** | Disables F = α·s·(v×B) magnetic force | ON |
| **selective_damping** | Apply damping only near particles (vacuum EM becomes lossless) | ON |
| **larmor_radiation** | Enable acceleration-dependent damping (radiation reaction) | OFF |
| **dual_substrate** | Track J_L, J_R independently (parity-violating mode) | ON |
| **color_forces** | SU(3)-inspired color-dependent pairwise force | OFF |
| **weak_transmutation** | Chirality-stress-driven polarity flip (+1  −1) | ON |
| **strong_force** | Yukawa short-range nuclear force | OFF |
| **triad_binding** | Detect 3-particle triads, mark them `locked = true` | OFF |
| **pair_production** | Correlated ±1 pairs from high-flux void regions | OFF |
| **exchange_force** | Pauli-exclusion repulsion between same-spin particles | OFF |
| **latency_field** | Poisson-based gravity potential ∇²L = 4πGρ | OFF |

**Pro tip:** For most pedagogy, disable `damping` and enable `genesis` — this lets the energy sit there and form structure. For physics-accurate simulations, leave defaults.

**Scenario overrides:** Some scenarios (e.g. `quantum-casimir` disables `genesis`, `s0-seed-beta-decay` enables `weak_transmutation` + `dual_substrate`). After loading a scenario, check the toggles panel to see what got flipped.

---

## 6. Overlays — what you're looking at

The "Overlays" panel exposes the visual layer. Each toggle adds a renderer on top of the voxel scene.

### Scalar-field overlays (2D sheets on a Y plane)
These render as colored rubber-sheets hovering at different Y-heights to prevent z-fighting:
- **ψ² (Born cloud)** — probability density from |J|². Pulsing breath at ~0.3 Hz (tied to animation clock).
- **Charge density** — signed voxel state.
- **Kretschmann** — latency-derived curvature scalar.
- **Coherence** — `|∇·J|²` normalization.
- **Fisher info** — gradient-squared of probability.
- **E pressure / B pressure** — EM energy density components.
- **Kinetic energy** — `|wave_vel|²/2`.
- **Vorticity** — `|∇×J|²`.

### Vector-field overlays (3D)
- **E field lines** — streamlines of `−wave_vel`.
- **B field lines** — streamlines of `∇×J`.
- **Poynting vectors** — energy flow S = E×B.
- **Flux streamlines** — J itself, following gradient lines.
- **Force glyphs** — particle-position arrows. Scale 1 shows the current net particle force; Scale 0 force decompositions remain separate lattice overlays.

### Volumetric overlays
- **Flux volume** — 3D density cloud colored by |J|.
- **Dual-substrate volume** — separate L/R substrate columns.
- **Genesis isosurface** — iso-surface at `|J| = K_GENESIS`.
- **Damping zones** — highlight voxels in selective-damping radius.
- **Confinement strings** — magenta lines connecting color-triad bound states.

### Special
- **Event horizon cloud** — latency L ≥ 0.95 (only visible in Schwarzschild-like scenarios).
- **Φ gravitational potential sheet** — rubber-sheet at bottom of lattice.
- **Wireframe boundary** — outer cube/sphere/torus (depending on boundary shape setting).

**Rendering budget:** Each overlay costs some frame time. At L=32 you can have 8+ on simultaneously; at L=128, keep it to 3-4 or FPS will drop below 30. Monitor via the FPS display top-right.

---

## 7. Diagnostics, charts, and the Lagrangian panel

### Diagnostics panel (Controls tab → Diagnostics sub-tab)

Per-tick summary:
- **Tick / physical time** — sim time since reset.
- **Manifested** — count of ±1 particles (total, +, −).
- **Total flux** — RMS of |J| across the lattice.
- **Total energy** — field energy + wave energy (they should sum cleanly; see Lagrangian for breakdown).
- **Charge balance** — `N₊ − N₋`. Should stay near zero in pair-production scenarios.
- **Spin up/down / color count** — per-state counts.

### Charts panel

Time series (scrollable, last 10K ticks):
- **Flux/wave energy** — stacked area chart.
- **Particle count** — line chart, colored by state.
- **Momentum / angular momentum** — 3-axis components.
- **Diagnostics sparklines** — every field in the diagnostics panel gets a mini sparkline.

Click any chart to open the fullscreen modal with crosshair scrub + export buttons.

### Lagrangian panel (the theoretical gold mine)

Shows the 7-term Lagrangian decomposition per-tick:
- `L_field_kinetic` = ½|wave_vel|² (dispositional field KE)
- `L_field_gradient` = −½c²|∇J|² (spatial curvature cost)
- `L_born_infeld` = −K_B·√(1−v²) (particle rest energy)
- `L_coupling` = g_c·s·(∇·J) (manifestation source)
- `L_velocity` = g_c·s·(v·J) (particle-flux drag)
- `L_gauss` = Gauss constraint residual
- `L_dissipation` = γ·½|J|² (damping sink)

Plus derived quantities: **Hamiltonian, total action, flux magnitude, wave energy, manifested count, locked count**.

Use this panel to verify the scenario you just loaded conserves the quantities it should. For example, `flux-dipole` with `damping` OFF: `L_dissipation ≈ 0`, total energy ≈ constant.

---

## 8. Inspector — click any voxel or particle

Click any voxel in the viewport → the **Inspector panel** (bottom-right, or toggle via `I` key) opens with:
- Coordinates `(x, y, z)` and world position
- State (0, +1, −1) and locked/pinned flags
- Flux vector `J = (Jx, Jy, Jz)` and |J|
- Wave velocity, ∇·J, curl(J), laplacian, E-field, B-field
- Spin, color, pair ID (if manifested)
- Proper time τ accumulated so far
- Latency L(x) and its gradient ∇L

For particles (Scale 1+), the Inspector shows:
- Nearest neighbors with distances
- Scale 1 identity, mass, charge, locked state, velocity, kinetic energy, nearest interaction, and net force
- Scale 2/3 molecular force terms such as bond spring, vdW, angle strain, and bound partners where that engine owns the data

**Shortcut:** Double-click any voxel to focus the camera on it without opening the Inspector.

---

## 9. The ontic observatory

The Panels → Ontic tab houses FTD's "meta-physics" summary:
- **Ontic Chain** — displays G* → VARPI → x± → α → K_B → derived values, with the derivation chain visualized as a DAG.
- **Physics Fidelity card** — energy eigenvalues, cross-sections, decay rates computed live from the current state.
- **Aggregation Bridge** — which "level of emergence" the current state is at (flux → particles → atoms → molecules → ...).
- **Emergence Monitor** — trajectory through levels over time.
- **Hierarchy Tower** — visual of the scale cascade.

Use this when you want to answer: *"Does this simulation respect the ontic hierarchy, or has it decoupled?"*

---

## 10. Keyboard shortcuts

Press `?` anywhere on the page to see the full cheat sheet. Highlights:

| Key | Action |
|---|---|
| `Space` | Global play/pause |
| `N` | Step one tick (while paused) |
| `R` | Reset sim (keeps current scenario) |
| `]` / `[` | Increase/decrease speed |
| `0`–`9` | Jump to scale N |
| `C` | Cycle camera view (free/top/side/front) |
| `H` | Toggle HUD (FPS, tick counter) |
| `I` | Toggle Inspector panel |
| `G` | Toggle grid |
| `V` | Toggle voxel wireframe |
| `F` | Toggle fullscreen |
| `S` | Screenshot PNG of current view |
| `E` | Open data export modal |
| `?` | Keyboard help overlay |

---

## 11. Performance tuning

### Lattice size (N)
Set via the `N` slider or `ctx.setLatticeSize(N)`. Typical choices:
- `N = 16` — instant, ~10M voxels total. Use for UI dev, algorithm validation.
- `N = 32` — standard (~32K voxels). 60 FPS on modern hardware with 3-4 overlays.
- `N = 64` — high-fidelity (~262K voxels). 30 FPS with most overlays off; good for long-run physics experiments.
- `N = 128` — paper-quality (~2M voxels). FPS drops to ~5-15; use with **headless** or very few overlays.

### Ticks per frame
Above the viewport, the speed slider maps to ticks-per-frame:
- `1×` = 1 tick per rendered frame (60 ticks/sec at 60fps)
- `10×` = 10 ticks per frame (600 ticks/sec)
- `0.1×` = 1 tick per 10 frames (slow motion — great for pedagogy)

### Toggle optimizations
The biggest wins for FPS:
1. Turn off `dual_substrate` if you don't need L/R split → ~15% faster.
2. Turn off `selective_damping` → ~10% faster (but you'll lose vacuum energy conservation).
3. Close unused charts (each one updates per-tick even when not visible).

### WASM execution
Scale 0 physics is WASM-only. Depending on browser capabilities, the same C++
engine runs in-thread through `WasmBridge` or in a worker through
`WasmBridgeProxy`; there is no JavaScript MockBridge fallback.

### GPU (WS server)
For the GPU path, build and launch the retained `ws_server` target from the
canonical WSL2 `engine/build_wsl` tree, then serve `engine/web`. The dashboard
connects to the loopback WebSocket bridge; the standalone browser/WASM path
remains CPU-only.

---

## 12. Console cookbook for research workflows

The dashboard exposes `window._ftdBridge` (the active bridge) and `window._ftdViewport` for console-level scripting. Useful patterns:

### Run an experiment and log results
```js
const b = window._ftdBridge;
b.setupScenario('flux-pulse');
const results = [];
for (let t = 0; t < 500; t++) {
    b.tick();
    const d = b.getDiagnostics();
    results.push({ tick: d.tick, energy: d.totalEnergy, n: d.manifested });
}
console.table(results.slice(0, 20));
// Download as CSV:
const csv = 'tick,energy,n\n' + results.map(r => `${r.tick},${r.energy},${r.n}`).join('\n');
const a = Object.assign(document.createElement('a'), {
    href: URL.createObjectURL(new Blob([csv], { type: 'text/csv' })),
    download: 'flux-pulse-500-ticks.csv'
});
a.click();
```

### Sweep a parameter
```js
const results = {};
for (const scenario of ['flux-pulse', 'flux-dipole', 'flux-cascade']) {
    const b = window._ftdBridge;
    b.setupScenario(scenario);
    for (let t = 0; t < 100; t++) b.tick();
    results[scenario] = b.getEnergyAudit();
}
console.table(results);
```

### Snapshot voxel state
```js
const b = window._ftdBridge;
const N = b.latticeSize;
const flux = b.getFluxVolume();  // Float64Array of length N³·3
const snapshot = { tick: b.currentTick(), flux: Array.from(flux) };
console.log(JSON.stringify(snapshot).length, 'bytes');
```

### Force a specific toggle state
```js
window._ftdBridge.setToggle('damping', false);
window._ftdBridge.setToggle('genesis', true);
// ... or check: window._ftdBridge.getToggle('dual_substrate')
```

### Inspect a WASM scenario programmatically
```js
// Start in WASM (default)
const w = window._ftdBridge;
w.setupScenario('flux-pulse');
for (let t = 0; t < 100; t++) w.tick();
const wasmResult = w.getDiagnostics();
console.log('WASM:', wasmResult);

```

### Sample a field programmatically
```js
const e = window._ftdBridge.getEFieldSampled(2);  // stride=2
console.log('E-field samples:', e.count, 'vectors');
console.log('First vector:', e.vectors.slice(0, 3));
```

---

## 13. Reproducible experiments

### Seed repeatability
Stochastic scenarios (flux-random-genesis, flux-thermalization, flux-vacuum-foam, flux-zero-point, quantum-born-rule, quantum-casimir) use a **fixed RNG seed** (`0xC0DEFACE`) that resets on every `setupScenario()` call. Repeated runs in the same browser session produce bit-exact results.

### What does NOT reset
- The `_tick` counter → call `b.reset()` explicitly first if you need tick=0.
- Toggle states set by previous scenarios → save and restore if needed:
  ```js
  const savedToggles = {};
  ['damping', 'genesis', 'dual_substrate', 'coupling'].forEach(t => {
      savedToggles[t] = window._ftdBridge.getToggle(t);
  });
  // ... run experiment ...
  Object.entries(savedToggles).forEach(([k, v]) => window._ftdBridge.setToggle(k, v));
  ```

### Play bar — forward navigation
The play bar (below the viewport) hosts play/pause, single-step, reset, speed, and a forward "T N"
tick readout. The simulation is **forward-only** — there is no rewind — but the step controls make
forward navigation precise:
- **Step (S)** advances exactly one tick — frame-by-frame to catch the tick a particle manifests.
- **Step-by-N** (gear ▸ Step: +1 / +10 / +100) jumps a fixed number of ticks while paused.
- **Speed** (gear ▸ Speed, or the − / + nudges) sets ticks-per-frame — slow a fast event down or
  speed a slow one up. Scale 0 only.
- **Reset (R)** re-seeds the current scenario from tick 0 for a repeatable run + screenshot.

---

## 14. Troubleshooting

### WASM engine fails to initialize
- **Cause:** Browser blocked the `.wasm` file or the asset was not deployed.
- **Fix:** Use a current Chrome/Edge/Firefox build and check the console for 404s on `/wasm/ftd_core.wasm`.

### "Dashboard loads but no overlays render"
- **Cause:** WebGL context failed.
- **Fix:** Check `chrome://gpu` — hardware acceleration must be enabled. Update GPU drivers.

### "Toggling ψ² overlay doesn't show anything"
- **Cause:** Flux field is below the minimum threshold. The overlay renders only voxels with `|ψ|² > 1e-4`.
- **Fix:** Pick a scenario that actually seeds flux (e.g. `flux-pulse`). Verify via `window._ftdBridge.getDiagnostics().totalFlux > 0`.

### "Scenario loads but lattice stays empty on WASM"
- All 84 UI scenarios run on WASM. If you see this, hard-refresh (Ctrl+Shift+R) to clear cached WASM binary.

### "Sim runs too fast / blinks particles"
- **Cause:** Ticks-per-frame is too high.
- **Fix:** Drop the speed slider to 1× or 0.5×.

### "Paused simulation's opacity still pulses"
- **Cause:** Animation clock advancing during pause.
- **Fix:** This is the ticket-14 regression test. If you see it, it's a regression — file a bug and re-run `tests/animation-clock-freeze.spec.js`.

### "Scenario xyz doesn't work"
- Check the console for a JS error.
- Try `window._ftdBridge.setupScenario('xyz')` directly. If no error, look at tick 0 state via `getDiagnostics()`.
- Cross-check that the id exists in both the UI registry and the matching C++ group under `engine/src/scenarios/`.

---

## 15. Extending the dashboard

### Adding a new scenario

Scenarios have one physics implementation plus their UI descriptor:
1. **C++ seed** — the matching group under `engine/src/scenarios/`
2. **UI registry** — `engine/web/js/scales/scale0/scenario-registry.js` (adds the dropdown entry)

For a new prefix group, also register in:
- `engine/include/ftd/scenarios.h` + `engine/src/scenarios.cpp::dispatch_scenario` (C++ dispatcher)

> **Note:** The `scenario-parity.spec.js` test fails if the UI registry and
> C++ dispatcher drift. Add the id to the matching C++ group and the registry,
> OR add the name to the `KNOWN_LEGACY_ONLY` allowlist in
> `engine/web/tests/scenario-parity.spec.js` if it's intentionally backward-compat-only.

### Adding a new toggle

1. Add to `engine/include/ftd/term_toggles.h` (C++ struct field).
2. Add matching entry in `engine/web/js/config/toggles.js` (JS key must match C++ field name).
3. Implement behavior in whatever tick-cycle phase needs to honor it (see `engine/src/render_bridge.cpp::tick()` for the phase sequence).
4. Rebuild WASM: `emcmake cmake -S engine -B engine/build_wasm && emmake cmake --build engine/build_wasm --target ftd_wasm`.
5. Deploy: `cp engine/build_wasm/wasm/ftd_core.* engine/web/wasm/`.

### Adding a new overlay

Templates live in `engine/web/js/scales/<scaleN>/ui/overlays/template.js`. The overlay controller:
- Takes bridge state + viewport reference
- Exposes `.toggle(on)` and `.update(data)` methods
- Registers with the scale's controller in `engine/web/js/scales/<scaleN>/controller.js`

See `engine/web/js/viewport/molecular-renderer.js` as the canonical 600-LOC extraction example.

### Running the test suite

```bash
# Node unit tests (fast, no browser):
cd engine/web && node --test tests/*.spec.js

# Playwright browser tests (requires python server):
cd engine/web/tests && npx playwright test --reporter=list

# Specific suite:
cd engine/web/tests && npx playwright test wasm-scenario-coverage.spec.js
cd engine/web/tests && npx playwright test animation-clock-freeze.spec.js
cd engine/web/tests && npx playwright test perf-baseline.spec.js

# JSC++ scenario parity guard:
# Fails CI if a scenario exists in the JS group files but not in
# engine/src/scenarios.cpp (or vice versa). Fast — runs in <1s.
cd engine/web/tests && npx playwright test scenario-parity.spec.js

# C++ engine tests (slow):
ctest --test-dir engine/build -C Release --timeout 60 -j24 --output-on-failure
```

### Reference documents

- `engine/SPEC_ENGINE.md` — engine architecture
- `engine/web/ARCHITECTURE.md` — current web dashboard architecture
- `engine/web/docs/INDEX.md` — documentation map and active/historical split
- `engine/include/ftd/scenarios.h` — scenario library public API
- `docs/SPEC_FTD.md` — the FTD theory spec (foundational)
- `docs/theory/META_INDEX.md` — theory document catalog

---

## Appendix A: The FTD canonical constants

Used throughout the dashboard and engine:

| Symbol | Value | Meaning |
|---|---|---|
| **G*** | 2.9586751192 | Lemniscatic constant (universal render bridge) |
| **ϖ** | 2.6220575543 | Lemniscate constant (first integral) |
| **α** | 1/137.036 | Engine fine-structure input using the tree-level master-quadratic `x_+` value; physical identification remains `[STRONGLY MOTIVATED CONJECTURE]` |
| **N_c** | 3 | Number of colors; sourced from independent topology/framework arguments, not from the retired `x_-` identification |
| **K_B** | 0.511 MeV | Electron-mass anchor (the manifestation-kinetics scale is K_MANIFEST := W_SC = 0.505462 since FTD-0388) |
| **K_GENESIS** | 1.533 | Genesis threshold = N_c · K_B |
| **C_SPEED** | 1/√3 | CFL limit on cubic lattice |
| **G_N** | 1/100 | Lattice-scaled simulation gravity parameter, not the physical gravitational coupling |
| **N_base** | 4 | D=3 + 1 time dimension |
| **N_eff** | 13 | Effective degrees of freedom |

The dashboard and engine mirror these values through `engine/include/ftd/constants.h`, `engine/include/ftd/ontic.h`, and `engine/web/js/constants.js`, but not every runtime constant is a derivation. Treat claim status as defined by the project ledgers, not by this quick-reference table.

---

**You're now equipped to run the FTD dashboard as a research tool, not just a demo. Questions beyond this guide should go to `engine/web/ARCHITECTURE.md` (technical architecture), `engine/web/docs/INDEX.md` (documentation map), `docs/SPEC_FTD.md` (physics), or open a discussion thread.**

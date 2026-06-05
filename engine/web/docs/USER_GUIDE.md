# FTD Web Dashboard — Power-User Guide

**Status:** `[REFERENCE]`
**Updated:** 2026-04-18
**Audience:** Researchers, contributors, and curious physicists who want to get real work out of the dashboard

---

## Table of Contents
1. [Quick start (60 seconds)](#1-quick-start-60-seconds)
2. [The two backends — and which one you're on](#2-the-two-backends--and-which-one-youre-on)
3. [The 10 Scales](#3-the-10-scales)
4. [Scenarios — 84 ways to seed Scale 0](#4-scenarios--84-ways-to-seed-scale-0)
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
3. Click ▶ Play. A Gaussian flux blob at the center expands spherically. Watch it bounce off the cube walls and interfere with itself (the wave equation in action).
4. Click ■ (reset) → pick `Genesis Cascade`. Above-threshold flux → watch particles spontaneously manifest (±1 voxel states light up).

You've just run two self-consistent simulations of the FTD substrate. Every deeper feature below builds on this loop.

---

## 2. The two backends — and which one you're on

The dashboard has **two simulation backends**:

| Backend | How activated | Perf | Scale 0 coverage |
|---|---|---|---|
| **WASM** (default) | Auto-loaded from `wasm/ftd_core.wasm` on page load | ~2× faster than Mock; uses SIMD + LTO | 84/84 scenarios (as of 2026-04-18) |
| **MockBridge** (JS fallback) | Used if WASM fails to load, OR you toggle via `?dev=1` URL param | Slower but modifiable without rebuild | 83 scenarios |

**Which am I on?** Look at the `Engine` label top-left:
- `WASM Engine` (green) — you're on the compiled C++ path.
- `Mock Bridge` (amber) — you're on the JS fallback.

**Force MockBridge for development:** append `?dev=1` to the URL. Useful when iterating on scenario authoring (edit `engine/web/js/bridge/scenarios/*.js`, refresh page, changes live — no WASM rebuild needed).

**Why two backends exist:** The WASM backend runs real FTD physics at native speed. MockBridge exists so new scenario authors can iterate in pure JS, and so CI can test the UI without an Emscripten toolchain. Both implementations were audited for parity in April 2026 — every UI scenario produces equivalent behavior on both.

---

## 3. The 10 Scales

FTD spans 11 orders of magnitude of physical phenomena, exposed as 10 scale "modes" you can switch between:

| # | Mode | What it shows | Typical scenario |
|---|---|---|---|
| **0** | Lattice / Substrate | Raw flux field J(x) + wave equation + 3³ voxel dynamics | `flux-pulse`, `flux-cascade` |
| **1** | Particles | N-body Coulomb + gravity (PE engine) | `pe-hydrogen`, `pe-coulomb-scattering` |
| **2** | Atoms | Electron orbitals, nuclear shells, periodic table | `ae-carbon`, `ae-hydrogen` |
| **3** | Molecules | Bonded atoms, vdW + LJ + bond springs | `mol-h2`, `mol-water`, `mol-caffeine` |
| **4** | Planetary | N-body gravity, Kepler orbits, exoplanet systems | `planetary-solar-system`, `planetary-trappist-1` |
| **5** | Cosmic | Lambda-CDM, Hubble flow, galaxy formation, BH | `cosmic-lcdm`, `cosmic-stellar-lifecycle` |
| **6** | Meta | Moore neighborhood geometry (octahedron, cuboctahedron, stella octangula) | `meta-decomposition` |
| **11** | Reference frame context | sLoop self-reference, master quadratic θ_C | `reference frame context-sloop` |
| **12** | Hamiltonian bridge | Phi polynomials, cyclotomic structure | *(pedagogy only)* |
| **23** | Scale 2-3 shared | Shared rendering pipeline for atoms + molecules | *(internal)* |

**Switching scales** changes:
- The viewport camera and boundary
- The toolbar (scale-specific scenario/controls)
- The diagnostics table (scale-specific metrics)
- The LIVE physics engine path (0→lattice, 1→PE, 2→AE, 3→AE+bonding, 4→Scale4, 5→Scale5, 6→Scale6, 11→Scale11)

**Pause behavior:** The **▶/⏸ (play/pause)** button freezes the simulation. The engine tick is the
single time source, so nothing in the lattice advances while paused — but the render loop keeps
running, so you can still orbit the camera and toggle overlays. Only *recorded* sim time stops; the
observer's clock keeps flowing.

---

## 4. Scenarios — 84 ways to seed Scale 0

A scenario is a **preset initial condition**. Picking one from the dropdown:
1. Resets the lattice to vacuum (tick = 0)
2. Injects flux / particles / wave velocity according to the scenario's definition
3. Optionally flips toggles (e.g. `quantum-casimir` disables `genesis`)

### The 5 scenario groups

| Prefix | Count | What it's for |
|---|---|---|
| `flux-*` | 20 | Pure Scale-0 substrate physics: pulses, dipoles, solitons, vortices, annihilation, QCD mesons/baryons/string-breaking, cyclotron, screening, thermalization, vacuum foam |
| `light-*` | 4 | EM wave pedagogy: rainbow (3-color), dipole radiation, two-slit, photon race (linearity) |
| `quantum-*` | 8 | Quantum experiments: Born rule, double-slit with genesis, tunneling, particle-in-a-box (quantum well), entanglement, Aharonov-Bohm, Casimir, Zeno |
| `s0-seed-*` | 43 | FTD-derived particle configurations: leptons, hadrons, quarks, Moore geometries, gauge bosons, gravity seeds, reference frame context seeds |
| `s0-field-*` | 8 | Analytical field configurations: plane wave, uniform E, uniform B, electric dipole, magnetic dipole, vortex line |

### Notable scenarios worth trying first

- **`flux-pulse`** — 3D Gaussian → spherical wave. The "hello world" of Scale 0.
- **`flux-cascade`** — Genesis threshold demo. Watch particles spontaneously appear from overdense flux.
- **`flux-vortex`** — Helical flux ring → explicit spin / angular momentum visualization.
- **`flux-annihilation`** — 4 ±1 particles on collision courses. Genesis → annihilation pair production.
- **`quantum-double-slit`** — Genesis + interference. Shows manifestation statistics following |ψ|².
- **`quantum-casimir`** — Two reflective plates + vacuum noise. Energy eigenstates between plates.
- **`s0-seed-hydrogen`** — Proton triad + electron orbital seed.
- **`s0-seed-moore-decomposition`** — All 3 Moore shells (octahedron + cuboctahedron + stella octangula) with alternating parity. The geometric heart of the theory.
- **`s0-seed-sloop`** — 12-vertex self-reference ring for reference frame context pedagogy.
- **`s0-field-vortex-line`** — Long vortex line (length much greater than the visible region) → watch (1/r) azimuthal flux circulation.

### Stochastic scenarios

Five scenarios use randomness:
- `flux-random-genesis`, `flux-thermalization`, `flux-vacuum-foam`, `quantum-born-rule`, `quantum-casimir`

The RNG is **reset to a fixed seed on every `setupScenario` call** (as of 2026-04-18), so repeated runs in the same browser session produce identical results. A page reload also produces the same sequence. This is intentional — it makes snapshot-based testing possible.

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
| **weak_transmutation** | Chirality-stress-driven polarity flip (+1 ↔ −1) | ON |
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
- **Force glyphs** — stacked arrows at particle positions (EM + gravity + strong, each a separate color).

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
- Force decomposition (Coulomb, gravity, strong, weak, bond spring, vdW, angle strain)
- Bound partners (for molecular scales)

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

### WASM vs Mock
WASM is ~2× faster than MockBridge for Scale 0 physics. If you're running long experiments, always use WASM (default).

### GPU (WS server)
For truly heavy work, launch `engine/start_gpu_dashboard.bat` which runs the native CUDA-accelerated engine and streams to the browser via WebSocket. Expect 20-50× speedups at N=128.

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

### Compare JS-fallback vs WASM on a scenario
```js
// Start in WASM (default)
const w = window._ftdBridge;
w.setupScenario('flux-pulse');
for (let t = 0; t < 100; t++) w.tick();
const wasmResult = w.getDiagnostics();
console.log('WASM:', wasmResult);

// Reload with ?dev=1 to force MockBridge, then:
// const m = window._ftdBridge; m.setupScenario('flux-pulse');
// for (let t = 0; t < 100; t++) m.tick();
// console.log('Mock:', m.getDiagnostics());
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
Stochastic scenarios (flux-random-genesis, flux-thermalization, flux-vacuum-foam, quantum-born-rule, quantum-casimir) use a **fixed RNG seed** (`0xC0DEFACE`) that resets on every `setupScenario()` call. Repeated runs in the same browser session produce bit-exact results.

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

### "Mock Bridge" shown instead of "WASM Engine"
- **Cause:** Browser blocked the .wasm file (common in IE/old Edge), or `?dev=1` is in URL.
- **Fix:** Use Chrome/Edge/Firefox; remove `?dev=1`; check console for 404s on `/wasm/ftd_core.wasm`.

### "Dashboard loads but no overlays render"
- **Cause:** WebGL context failed.
- **Fix:** Check `chrome://gpu` — hardware acceleration must be enabled. Update GPU drivers.

### "Toggling ψ² overlay doesn't show anything"
- **Cause:** Flux field is below the minimum threshold. The overlay renders only voxels with `|ψ|² > 1e-4`.
- **Fix:** Pick a scenario that actually seeds flux (e.g. `flux-pulse`). Verify via `window._ftdBridge.getDiagnostics().totalFlux > 0`.

### "Scenario loads but lattice stays empty on WASM (not Mock)"
- **As of 2026-04-18 this is fixed.** All 84 UI scenarios now run on WASM. If you still see this, hard-refresh (Ctrl+Shift+R) to clear cached WASM binary.

### "Sim runs too fast / blinks particles"
- **Cause:** Ticks-per-frame is too high.
- **Fix:** Drop the speed slider to 1× or 0.5×.

### "Paused simulation's opacity still pulses"
- **Cause:** Animation clock advancing during pause.
- **Fix (as of 2026-04-18):** This is the ticket-14 regression test. If you see it, it's a regression — file a bug and re-run `tests/animation-clock-freeze.spec.js`.

### "Scenario xyz doesn't work"
- Check the console for a JS error.
- Try `window._ftdBridge.setupScenario('xyz')` directly. If no error, look at tick 0 state via `getDiagnostics()`.
- Cross-check: does the scenario exist in BOTH `engine/web/js/bridge/scenarios/*.js` (JS side) AND `engine/src/scenarios.cpp` (C++ side)? If only one, that's the bug.

---

## 15. Extending the dashboard

### Adding a new scenario

Scenarios live in **two places** that must stay in sync:
1. **JS side** — `engine/web/js/bridge/scenarios/<group>-scenarios.js`
2. **C++ side** — `engine/src/scenarios.cpp` (mirror the JS body in corresponding `setup_<group>_scenario` function)
3. **UI registry** — `engine/web/js/scales/scale0/scenario-registry.js` (adds the dropdown entry)

For a new prefix group, also register in:
- `engine/web/js/bridge/scenarios/index.js` (JS dispatcher)
- `engine/include/ftd/scenarios.h` + `engine/src/scenarios.cpp::dispatch_scenario` (C++ dispatcher)

> **Note:** The `scenario-parity.spec.js` test will fail CI if you add a scenario
> to one side and forget the other. Add it to BOTH the JS group file AND
> `engine/src/scenarios.cpp`'s corresponding `setup_<group>_scenario` function,
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

# JS↔C++ scenario parity guard (new 2026-04-19):
# Fails CI if a scenario exists in the JS group files but not in
# engine/src/scenarios.cpp (or vice versa). Fast — runs in <1s.
cd engine/web/tests && npx playwright test scenario-parity.spec.js

# C++ engine tests (slow):
cd engine/build && ctest -C Release --timeout 60 -j4
```

### Reference documents

- `engine/SPEC_ENGINE.md` — engine architecture
- `engine/web/ARCHITECTURE.md` — web dashboard architecture
- `engine/web/docs/SPEC_REFACTOR_LARGE_FILES.md` — the recent large-file split spec (14 tickets across 3 waves)
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
| **α** | 1/137.036 | Fine structure constant (from master quadratic x₊) |
| **N_c** | 3 | Number of colors (from master quadratic x₋) |
| **K_B** | 0.511 MeV | Manifestation threshold (electron mass) |
| **K_GENESIS** | 1.533 | Genesis threshold = N_c · K_B |
| **C_SPEED** | 1/√3 | CFL limit on cubic lattice |
| **G_N** | 1/100 | Gravitational coupling = 1/(b₃ + N_c)² |
| **N_base** | 4 | D=3 + 1 time dimension |
| **N_eff** | 13 | Effective degrees of freedom |

All are live in `engine/include/ftd/constants.h` (C++) and `engine/web/js/constants.js` (JS). They agree to floating-point precision and are derived from D=3 + ϖ alone.

---

**You're now equipped to run the FTD dashboard as a research tool, not just a demo. Questions beyond this guide should go to `engine/web/docs/SPEC_REFACTOR_LARGE_FILES.md` (technical architecture), `docs/SPEC_FTD.md` (physics), or open a discussion thread.**

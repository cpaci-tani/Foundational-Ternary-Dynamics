# FTD Simulation Engine Reference

**Living document for AI agents and developers.**
**Last updated:** 2026-04-27 (post 8-phase modular refactor sweep — see §"April 27, 2026 — Modular refactor sweep" below)
**Engine version:** 2.17.0 (post-refactor: viewport + bridge + render_bridge + CUDA stencil all decomposed)
**Test count:** 257 C++ test source files (211 active CMake targets after 2026-05-04 trim-the-fat round 4) + 17 Playwright specs + 23 Python test files. CTest LABELS scheme (`unit`/`physics`/`golden`/`slow`/`gpu`). GPU conditional on `FTD_ENABLE_CUDA`.

### April 27, 2026 — Modular refactor sweep (8 phases, 17 commits)

The engine was decomposed across 8 phases following the plan in
`.claude/plans/i-want-to-try-crispy-charm.md` (closed). Driven by the
audit observation that 5 specific files had accumulated structural
bloat. Bit-exact physics preservation enforced by a 100-tick
deterministic byte-hash gate (`test_render_bridge_golden`, hash
`0xcd957b601d47868a`) that held across every commit.

**Cumulative LOC reductions:**

| File | Before | After | Δ |
|---|---:|---:|---:|
| `engine/web/js/viewport.js` | 3953 | 1256 | **−68%** |
| `engine/web/js/wasm-bridge-dag.js` | 2395 | 42 | **−98%** |
| `engine/src/render_bridge.cpp` | 1231 | 545 | **−56%** |
| `engine/cuda/kernels_stencil.cu` | 1530 | 0 (deleted, split into 3 TUs) | **−100%** |
| `engine/include/ftd/render_bridge.h` | 506 | 369 | **−27%** |
| `engine/include/ftd/test_telemetry.h` | 412 | 154 | **−63%** |

**New infrastructure created:**
- 4 viewport sub-renderer modules (4948 LOC across 5 files): `scene-core.js`, `flux-renderer.js`, `particle-renderer.js`, `field-renderer.js`
- 7 bridge layer modules: `mock-bridge.js`, `wasm-bridge.js`, `capabilities/{install,scale0,scale1,scale2}.js`
- 5 `render_bridge_phases/` TUs: `phase_write.cpp`, `phase_forces.cpp`, `phase_read.cpp`, `phase_movement.cpp` (Phase 4) + R1-R5 prior
- 3 CUDA TUs: `kernels_stencil_single.cu`, `kernels_stencil_dual.cu`, `kernels_aux.cu` + shared `kernels_stencil_common.cuh`
- `ftd_test_support` library (test_telemetry impl + bridge_fixtures)
- 4 new ADRs: 0010 (cascade callback), 0011 (mesh-factory callback), 0012 (golden-tick gate), 0013 (toggle TOGGLE_SPECS[])

**Key patterns established:**
- **Cascade callback** (ADR-0010): every sub-renderer exposes `onLatticeSizeChanged`, `setBoundaryShape`, `setEngineMode`, `dispose`; orchestrator dispatches unconditionally.
- **Mesh-factory callback** (ADR-0011): single canonical home + ctor-bound callbacks for cross-sub-renderer helpers.
- **Golden-tick regression gate** (ADR-0012): `test_render_bridge_golden.cpp` hashes 100 ticks; bit-exact preservation required.
- **TOGGLE_SPECS[] table-driven** (ADR-0013): adding a toggle = 2-place edit (was 5-place).

See [docs/audits/AUDIT_2026-04_refactor-sweep.md](../docs/audits/AUDIT_2026-04_refactor-sweep.md)
for the full ledger including commit hashes, deferred items, and lessons learned.

**Outstanding deferral:** Phase 5 GPU runtime parity at L=64 — kernels_stencil split is host-compile-verified + CPU-deterministic-verified, but bit-exact GPU-stencil parity needs a WSL2 follow-up session (per CLAUDE.md GPU-via-WSL2 mandate).

---

### April 27, 2026 — Lattice cleanup pass + plumbing leak plugs (pre-refactor)

**Web engine architecture additions:**
- **PhysicsHarness layer** (`engine/web/js/physics/`) — single canonical
  read/write surface across MockBridge and WasmBridge. Lazy-attached
  per bridge; exposes `getParticleCharge`,
  `findOppositeChargePairFromList`, `sampleEFieldAlongRay`, particle
  injection, scenario dispatch. Retired the JS migrated-scenario
  registry and mirror-bridge plumbing — both bridges own their
  scenario libraries directly.
- **Bridge contract typedef** (`engine/web/js/bridge/bridge-contract.js`)
  — `@typedef ScaleBridge` documents the 16-method symmetric surface;
  both `MockBridge` and `WasmBridge` carry `@implements` annotations.
- **C-3 inversion**: `harness.setupScenario` defers to
  `bridge.setupScenario` (C++ canonical when `isWasm=true`,
  MockBridge native JS otherwise).
- **Lazy fluxMock allocation**: scenario-loader only allocates the
  parallel JS MockBridge when `shouldUseFluxMock` returns true. Saves
  ~21 MB / quantum-* / light-* scenario load.

**Scenario library DRY (JS + C++):**
- New shared `engine/web/js/bridge/scenarios/_helpers.js` exporting
  `injectRadialEnvelope`, `injectParticleFull`, `injectDressedParticle`,
  `injectTriad`, `TRIAD_ANGLES`. Six bespoke radial-Gaussian loops in
  `s0-seed-scenarios.js` collapsed to helper calls.
- `1/sqrt(3)` literals removed from JS (light, s0-field) and C++
  (light, s0_field) scenarios in favor of imported `C_SPEED`.
- `SCN_PI` shadow dropped from C++ `engine/src/scenarios/_helpers.h`;
  callsites use `ftd::PI` directly.
- Toggle-whitelist contract documented in `scenario-loader.js` and
  `engine/include/ftd/scenarios.h`.

**Plumbing + memory leak plugs (21 tickets across two audit passes):**
- WasmBridge gained `dispose()` symmetric with `MockBridge.dispose()`;
  `reset()` now cleans `_pe` / `_ae` / `_aeFallback` / harness key
  before destroying the C++ RenderBridge.
- `MockBridge.dispose()` extended to null `_stateGrid`,
  `_selectiveDampMask`, `_boundaryMask`, `_latencyProxy`, `_peEngine`,
  `_aeEngine`.
- `physics-harness.sampleEFieldAlongRay` position-index Map cache
  moved off the bridge-emitted `efs` object onto `harness._efsIndex`.
- Two leaked `BoxGeometry`s in `viewport.js` (voxelHighlight,
  symHighlights) now disposed after `EdgesGeometry` construction.
- `cosmic-renderer._cleanGeometries` now disposes `_nebulaCloud`.
- `chart-fullscreen` active-card stack handles concurrent
  fullscreen requests.
- `scrub-bar.mount()` idempotent; step-by-N chain generation-tagged.
- `rafCoordinator` auto-unsubscribes callbacks that throw 10 frames
  in a row; new `clear()` API for HMR / test teardown.
- Cross-cutting `window.__ftd*Panel` singleton retention fixed.
- P1 observables panel migrated from raw recursive
  `requestAnimationFrame` to `rafCoordinator.subscribe`; per-frame
  listener pattern replaced with single panel-level click delegation;
  full `dispose()` returned from api.
- Scale 11 `disableAudio()` now closes the AudioContext.
- `pagehide` hook releases the lazy fluxMock on bfcache freeze.

**C++ engine:**
- `RenderBridge::tick` strict_validation `throw` guarded with
  `#ifdef __EMSCRIPTEN__` → `std::cerr` + `std::abort` fallback so
  the WASM build (`-fno-exceptions`) doesn't abort the module
  silently on configuration bugs.

**Tooling:**
- `engine/web/serve.py` — no-cache dev server (Cache-Control:
  no-store) so JS edits hit the browser without manual hard-refresh.
- `engine/build_wasm.bat` — Windows wrapper around emcmake/emmake.
- `.githooks/commit-msg` — enforces no-`Co-Authored-By` policy.
- New `engine/web/docs/REF_DEBUG_GLOBALS.md` catalogues the 10
  `window.__ftd*` debug globals.

**Cumulative LOC delta:** ~−380 LOC across harness + scenario libraries;
~+250 LOC of new shared primitives + typedef + helpers; 21 plumbing
tickets closed; 9 infrastructure tickets closed. WASM rebuilt twice;
both clean.

### April 17, 2026 — Engine cleanup sweep (6 tracker items closed)

Six TRACKER_OPEN_ITEMS §1 items resolved in one pass, in dependency-ordered sequence. Summary:

| § | Title | Verdict | What changed |
|---|---|---|---|
| 1.4 | Symplectic leapfrog integrator | Already symplectic | Corrected comments + new audit test |
| 1.8 | Moore-Laplacian isotropy | Already isotropic (Taylor proof) | Corrected comments + new isotropy test |
| 1.5 | `ALPHA_PRECISION` rollout | Wiring needed | `ALPHA`, `G_C`, JS mirror all upgraded; `ALPHA_TREE` retained as reference |
| 1.2 | γ_FTD momentum integration | Real physics change | Velocity clamp replaced with `p = γmv` in `phase_forces`; removed the over-strict secondary clamp in latency block |
| 1.7 | GPU-path `EnergyLedger` | Hook needed | `tick()`'s GPU path now calls `gpu_sync_to_host()` + `update_energy_ledger()` |
| 1.9 | Muon / tau spatial seeds | JS feature | Two new scale-0 scenarios (`s0-seed-muon`, `s0-seed-tau`) with full epistemic metadata |

**All six viable engine opens are now ✅ CLOSED.** Three remaining §1 items are explicitly `[BLOCKED]` (DagEngine stubs, dynamical SU(3), δ_c closed form) on upstream work. See [`docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`](../docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md) for the full ledger.

### April 17, 2026: Open items tracker + cleanup sweep
- **`docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md`** — new canonical ledger of every `[OPEN]` across engine code, theory derivations, foundations, particles, consciousness, math connections, and bridges. 275 occurrences across 83 files, organised + auto-refreshable.
- **Dead-code removal:** `vec3Str` / `fmtForce` in `engine/web/js/inspector.js` (unused, superseded by `units.js`).
- See [CHANGELOG.md](../CHANGELOG.md) → "Open Items Tracker + Cleanup Sweep" for full list.

### April 17, 2026: Consolidation sweep
- **`DagEngine` marked EXPERIMENTAL.** Banner at the top of `dag_engine.h` and `dag_engine.cpp` makes clear that `gauss_project` / `phase_forces` / `phase_movement` are `[OPEN]` stubs — the production physics path is `RenderBridge`.
- **DagEngine WASM binding removed.** The web engine never called it; the Emscripten export was dead weight inviting users into an unfinished code path.
- **`engine/README.md`** got a new "Engine files — what's production, what's experimental" table.
- **`EnergyLedger`** auto-populated via `RenderBridge::update_energy_ledger()` at the end of every CPU-path `tick()`. Tests can now assert on `|residual| < tol` and refuse energy-drift regressions. GPU-path caveat documented (host voxels stale between syncs).

### April 17, 2026: Honesty sweep
- **`X_PLUS_PRECISION = 137.035999177`** and **`ALPHA_PRECISION = 1/X_PLUS_PRECISION`** added to `ontic.h` and re-exported. α derivation now first-class in engine headers, not just in docs. Engine force paths still use tree-level `ALPHA` (3.8 ppm wider than CODATA — below every benchmark's resolution). Swap when a benchmark needs < 1 ppm.
- **`ALPHA_EFT = G_C²` re-framed.** G_C was *defined* as √α, so `G_C² = α` is an identity by construction — a consistency check, not a derivation. The real α derivation is the master quadratic. (Engine behavior unchanged.)
- **Colour force re-tagged** `[PHENOMENOLOGICAL FIT]` (was `[EMERGENT]`). Colour labelling is emergent; the three-regime force law is imposed. Genuine SU(3) derivation tracked in `TRACKER_OPEN_ITEMS.md` §1.3 + §2.4.
- **Velocity clamp re-tagged** `[APPROXIMATION — NON-RELATIVISTIC CLAMP]`. Proper `γ_FTD` momentum integration is `[OPEN]`.
- **Gravity regime banner** at `G_N` in `ontic.h`: explicit that engine runs at lattice-toy strength (~10³⁷× physical). Every gravity-benchmark claim must state the regime.
- **Integration-scheme notes** added in `phase_read` header: the Moore Laplacian is consistent but not isotropic at O(h²); the advance pair is forward Euler, not symplectic leapfrog.

### April 17, 2026: Dashboard UX refresh
- **Panels redesign** (`docs/superpowers/specs/2026-04-16-panels-redesign-design.md`): diagnostics / charts / lagrangian tabs rebuilt on vendored uPlot 1.6.30 + a shared descriptor-driven table primitive. 27 diagnostic rows with physics-accurate units, 20 inline sparklines, chip-picker chart grid, stacked-area Lagrangian.
- **Playback timeline** (`docs/superpowers/specs/2026-04-16-playback-timeline-design.md`): floating scrub bar absorbs the play / local / step / reset / speed controls. Reverse-scrub backed by an LOD-tiered `TimelineBuffer` (working-memory analogue — snapshots block-average as they age). `Render 30s` button pre-computes a scrubbable clip via main-thread slicing + cancellable progress chip.
- **Weak force visualization**: shader swap → `PointsMaterial` + radial-gradient `CanvasTexture` sprite with additive blending; flow-style streamlines bumped to 320 seeds / full-length lines.
- **Overlay panel collapsible**: Scale 0 visualization panel gets a header chevron; per-scale collapse state in localStorage.

### April 13, 2026: Engine-Theory Bridge, EFT, GR Unlocked, 3 Theorem Papers
- **20-benchmark suite** (`benchmark_engine_theory.cpp`): first quantitative engine-to-theory comparison
- **EFT reconstruction**: `ALPHA_EFT = G_C * G_C` — alpha derived as coupling squared, not independent input
- **Emergent forces toggle**: computes force from flux field gradient without Poisson solver
- **6 emergence experiments** (`benchmark_emergent_alpha.cpp`): self-energy, interaction potential, emergent force, bound state, null baseline, EFT force
- **Budget equation** (`benchmark_budget_equation.cpp`): x/K + G*/x = 1 verified to 0.2% on lattice
- **Wilson loops** (`benchmark_wilson_loops.cpp`): 12/17 pass, flux tube collimation detected, area law sigma > 0
- **Gluon dynamics** (`campaign_gluon_dynamics.cpp`): 7/11 pass, linear E(r), E/r ~ constant
- **Einstein equations** (`test_einstein_equations.cpp`): gravitational superposition to 0.08%, **time dilation 0.004% match (after latency fix)**
- **BH thermodynamics** (`benchmark_black_hole_thermo.cpp`): **L_peak=0.62, proper time dilation** (after latency fix), Smarr S*T=M/2 exact
- **LATENCY FIX** (one line): `sqrt(max(phi,0))` -> `sqrt(|phi|)` in render_bridge.cpp line 735. Unlocks entire GR sector.
- **Three theorem papers** (conditional [THEOREM] upgrades):
  - `DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` — x+ = 1/alpha
  - `DERIV_SINGLET_FROM_VOID_EVENT.md` — Bell loop via void event
  - `DERIV_NC_FROM_TOPOLOGY.md` — N_c = 3 from 4 independent routes
- **WASM rebuilt and deployed**: ftd_core.js + ftd_core.wasm now in engine/web/wasm/
- **DagEngine fixed**: added 4 missing pure virtual overrides (current_tick, dt, set_dt, entity_count). **Update 2026-04-17:** DagEngine is now explicitly EXPERIMENTAL — see top-of-file April 17 consolidation entry.
- **6 SM visualization scenarios**: Particle Zoo, Higgs Field, Higgs Mechanism, Electroweak, Three Generations, QCD Vacuum
- **Scientific status: C+ -> B+** (20 benchmarks + 4 physics domains + GR + 3 theorems)
- Consolidated index.html (removed index_dag.html), fixed dag_engine.h missing include

### April 10-11, 2026 Session Additions
- **Stellar Lifecycle scenario** (Scale 5 JS): cloud collapse -> star formation -> death -> WD/NS/BH + Hawking evaporation
- **Fuel tracking and stellar evolution** in mock-scale5.js (fuel_fraction, fuel_stage, supernova ejecta)
- **Fuel-stage-aware rendering** in cosmic-renderer.js (red giants, late burners, dying stars)
- **Master verification script**: `scripts/proofs/proof_master_verification.py` (54/54 checks across 10 domains)

---

## 1. Architecture: Logic-First Engine (v2.0)

The engine was rewritten from ~1382 lines of phenomenological code to a logic-first design. Only behaviors derivable from the axioms {3D lattice, ternary states, flux field, local causality, action principle} remain. Everything else was archived to `archive/engine_v1_phenomenological/`.

**Six rules, nothing else:**

1. **Flux wave equation**: dJ/dt = c^2 nabla^2 J (only possible local linear dynamics for a vector field)
2. **State-flux coupling**: source term g_c * grad(s) + g_c * curl(s*v) (from dS/dJ = 0)
3. **Gauss projection**: enforce div(J) = s each tick (charge conservation -- logical necessity)
4. **Manifestation/Evaporation**: |J| > K_GENESIS -> manifest; neighborhood energy < K_B^2 * 1e-6 -> evaporate (7-site check: particle + 6 face-neighbors)
5. **Field-mediated forces**: F = -alpha * s * grad(phi_C) + G_N * grad(rho) + alpha * s * (v x B) where B = curl(J) (Poisson Coulomb + Lorentz magnetic + gravity)
6. **Movement + Collision**: remainder accumulation, speed limit C_SPEED = C_WAVE = 1/sqrt(3), annihilation on contact

**What was removed** (archived in `archive/engine_v1_phenomenological/`):
- Pairwise Coulomb, Yukawa, exchange, Lorentz forces
- QCD running coupling, color Yukawa
- Weak transmutation, binding/triad locking, noetic/consciousness coupling
- Latency/bandwidth/proper-time system

**Toggle-gated extensions** (default OFF, for pedagogy and exploration):
- Larmor radiation: acceleration-dependent damping (v2.11)
- Dual substrate: J_L + J_R chirality physics
- Color forces, strong force, weak transmutation, triad binding, pair production, exchange force

### Scale 5: Cosmic Engine (v2.12)

N-body + SPH cosmic simulation with Barnes-Hut octree gravity. All physics driven by FTD-derived constants (zero free parameters):
- **9 body types**: Dark matter, gas, stars, neutron stars, black holes, quasars, nebulae, white dwarfs, dark energy field
- **18-phase cosmic tick cycle**: octree build, gravity, SPH density/forces, Friedmann expansion, dark energy, accretion, jets, star formation, stellar evolution, magnetic fields, radiation pressure, gravitational waves, Verlet integration
- **14 toggles**: gravity, sph_gas, hubble_expansion (core ON); dark_energy, dark_matter_halos, black_hole_accretion, cosmic_radiation, star_formation, stellar_evolution, galaxy_mergers, magnetic_fields, radiation_pressure, relativistic_jets, gravitational_waves (extensions OFF)
- **FTD constants**: G_N=0.01, Omega_Lambda=2/3, DM_frac=17/27, gamma=5/3, c=1/sqrt(3)

### Abstract Base Class: ScaleEngine (v2.12)

All scale engines (ParticleEngine, CosmicEngine) inherit from `ScaleEngine`, providing:
- Unified `tick()`, `run()`, `current_tick()`, `dt()`, `set_dt()` interface
- String-based `get_toggle(name)` / `set_toggle(name, value)` for unified registry
- `base_diagnostics()` returning common metrics across all scales
- `scale_level()` and `scale_name()` for runtime type identification

### Scaling and Performance Constraints

**Lattice Engine (Scale 0)**
Forces are $O(N)$ field-mediated (single loop over manifested particles summing their interactions with the local lattice neighborhood) instead of $O(N^2)$ explicit pairwise. Inherently faster for large particle counts processing raw flux.

**Macro Engines (Scales 1, 2, 5)**
The `ParticleEngine`, `AtomEngine`, and `CosmicEngine` all rely on a dynamically re-calculated **Barnes-Hut Octree** (see `barnes_hut.h`) to approximate macroscopic limits of long-range $1/r^2$ isotropic potentials (e.g. Gravity and Coulomb).
- Achieves $\mathcal{O}(N \log N)$ computation scaling by terminating monopole traversals at a critical opening angle threshold ($\theta < 0.5$).
- `AtomEngine`'s discrete covalent interactions traverse a fully pre-separated $O(N)$ topographical linked-list ensuring that discrete bounds like `Angle Strain` do not invoke continuous $O(N^2)$ matrices.
---

## 2. Directory Layout

```
engine/
  CMakeLists.txt              # Build system -- all targets and test registration
  SPEC_ENGINE.md              # This document
  print_ontic.py              # Utility to print ontic chain values
  include/ftd/
    scale_engine.h            # [v2.12] Abstract base class for all scale engines (111L)
    ontic.h                   # Ontic derivation chain (9+ layers), D=3 + varpi -> all constants (1221L)
    constants.h               # Re-exports ontic + engine-specific constants (279L)
    constants_gpu.cuh         # GPU-side constants mirror (device __constant__ memory)
    voxel.h                   # Vec3, ForceDiag, Voxel struct (203L)
    lattice.h                 # Lattice class -- 3D cubic grid with periodic boundaries (59L)
    render_bridge.h           # RenderBridge -- main engine API, tick(), diagnostics() (239L)
    lagrangian.h              # 4-term Lagrangian + Rayleigh dissipation (218L)
    term_toggles.h            # 20 runtime toggles for pedagogy system (62L)
    csv_export.h              # Header-only CSV export (flux field, density slice, timeseries) (385L)
    particle_engine.h         # ParticleEngine : ScaleEngine -- Scale 1 particles (247L)
    atom_engine.h             # AtomEngine -- Scale 2 composite atoms + bonds (327L)
    cosmic_engine.h           # [v2.12] CosmicEngine : ScaleEngine -- Scale 5 N-body+SPH (523L)
    scale.h                   # OnticEntity + scale bridge declarations (83L)
    scenarios.h               # [NEW Apr 2026] Public dispatch_scenario() -- C++ port of JS scenario library
    correlations.h            # Correlation function analysis (205L)
    ensemble.h                # Statistical ensemble infrastructure (200L)
    spectral.h                # Spectral analysis utilities (195L)
    tracker.h                 # Particle trajectory tracking (173L)
    hilbert.h                 # Hilbert space utilities (209L)
    barnes_hut.h              # Octree for long-range 1/r^2 forces (used by PE/AE/CE)
    constructors.h            # Scenario/state constructors reused across engines
    dag_engine.h              # DagEngine [EXPERIMENTAL] -- gauss_project/phase_forces/phase_movement stubs
    dag_lattice.h             # Lattice variant used by DagEngine
    engine_select.h           # Runtime switch between logic-first and DAG paths
    test_telemetry.h          # Shared telemetry helpers used by CTests
    gpu_engine.h              # GpuEngine -- CUDA GPU drop-in for RenderBridge (115L)
    gpu_buffers.h             # SoA device memory layout (124L)
    gpu_atom_engine.h         # GPU AtomEngine bindings
    gpu_particle_engine.h     # GPU ParticleEngine bindings
  src/
    render_bridge.cpp         # Logic-first engine -- 6-phase tick cycle (1538L)
    lagrangian.cpp            # compute_lagrangian_diagnostics() -- 4 active terms (166L)
    main.cpp                  # CLI entry point (scenarios A-K) (937L)
    particle_engine.cpp       # ParticleEngine: Velocity Verlet + analytical forces (394L)
    atom_engine.cpp           # AtomEngine: ionic + vdW + covalent forces (762L)
    cosmic_engine.cpp         # [v2.12] CosmicEngine: Barnes-Hut + SPH + Friedmann (900L)
    scale_bridge.cpp          # Scale 0<->1<->2<->5 coarsen/refine round-trip (283L)
    scenarios.cpp             # [NEW Apr 2026] 83 scenarios from JS ported to C++ (flux-/light-/quantum-/s0-seed-/s0-field-)
    constructors.cpp          # Shared scenario/state constructor helpers
    dag_engine.cpp            # DagEngine [EXPERIMENTAL] -- see banner in header
    ontic_audit.cpp           # Ontic-chain self-audit (prints derivations and consistency checks)
    ws_server.cpp             # Optional native WebSocket bridge server (consumed by ws-bridge.js)
  cuda/
    gpu_buffers.cu            # SoA device allocation, upload, download (445L)
    gpu_engine.cu             # GpuEngine tick loop, host<->device sync (496L)
    kernels_stencil.cu        # GPU phase_read + phase_write + near_particle + dual-substrate (1172L)
    kernels_poisson.cu        # FFT Poisson solver (cuFFT spectral) (328L)
    kernels_forces.cu         # GPU forces + movement + color/strong/weak/exchange kernels (737L)
    CMakeLists.txt            # CUDA build rules (35L)
  config/                     # [v2.12] Data-driven configuration
    toggles.json              # Unified toggle registry -- 48 toggles across all scales
    scenarios/                # Scenario manifests per scale (JSON)
      scale0.json             # 36 lattice scenarios
      scale1.json             # 25 particle scenarios
      scale2.json             # 20 atom scenarios + 118 element entries
      scale3.json             # 27 molecule scenarios
      scale4.json             # 10 consciousness scenarios + 12 figures
      scale5.json             # 4 cosmic scenarios + camera presets
      scale6.json             # Meta scenario + 13 toggle controls
  tests/
    257 test files            # 211 active CMake targets after 2026-05-04 trim-the-fat round 4
  wasm/
    ftd_wasm.cpp              # Emscripten Embind bindings -- full engine API (1512L)
    CMakeLists.txt            # WASM build rules (Emscripten-only)
  web/
    index.html                # Browser dashboard (structural HTML, no inline CSS) (1888L)
    css/                      # [v2.12] Modular CSS architecture (10 files)
      tokens.css              # Design tokens, reset, base styles
      layout.css              # App grid, toolbar, viewport, status bar
      components.css          # Cards, tabs, panels, toggles, modals, settings
      scale-visibility.css    # Per-mode show/hide rules (48 selectors)
      charts.css              # Chart + diagnostic component styles
      themes/                 # 5 theme override files
        midnight.css           abyss.css           light.css
        parchment.css          nord.css
    js/                       # [v2.12] Modular JS architecture (~40 modules)
      app.js                  # Main coordinator: init, frame loop, scale dispatch
      constants.js            # JS mirror of ontic.h derivation chain
      core/                   # Shared infrastructure
        state.js              # Centralized runtime state singleton
        event-bus.js           # Pub/sub for decoupled module communication
        bridge.js              # UnifiedBridge -- scale-agnostic simulation interface
      config/                 # Extracted configuration data
        toggles.js            # Toggle definitions + scenario override maps
        scenarios.js          # Consciousness scenario descriptions
      bridge/                 # Simulation bridge layer
        bridge-factory.js     # createBridge() factory (WASM -> MockBridge fallback)
        mock-scale5.js        # CosmicMockBridge (JS-only N-body for dev)
      scales/                 # Per-scale controllers (each owns its own state)
        scale0/controller.js  # Lattice: animateLattice, loadScenario, field viz (702L)
        scale1/controller.js  # Particles: animatePE, cloud rendering, trails (912L)
        scale2/controller.js  # Atoms: animateAE, orbital clouds, force arrows (1056L)
        scale3/controller.js  # Molecules: loadMolecule, reuses Scale 2 animate (217L)
        scale4/controller.js  # Consciousness: sLoop, Mandelbrot, hologram (443L)
        scale5/controller.js  # Cosmic: N-body, galaxy rendering (193L)
        scale6/controller.js  # Meta: existential unit, geometry toggles (150L)
      viewport.js             # Three.js 3D: particles, bonds, orbitals, fields, camera
      wasm-bridge.js          # WasmBridge + MockBridge (auto-fallback)
      cosmic-renderer.js      # [v2.12] Photorealistic cosmic body rendering
      consciousness.js        # ConsciousnessEngine (sLoop, measurement cascade)
      consciousness-pedagogy.js  # Pedagogical visualizations (Canvas 2D)
      consciousness-figure.js    # Holographic figure (Three.js)
      meta-unit.js            # MetaUnit (3x3x3 Moore neighborhood)
      meta-pedagogy.js        # Meta info/inspect panels
      [+ 15 additional library modules: elements, orbitals, molecules, fields, etc.]
    wasm/
      ftd_core.js             # Emscripten JS loader (generated)
      ftd_core.wasm           # WebAssembly binary (generated)
  build/                      # CPU build directory
  build_wasm/                 # WASM build directory
  build_cuda/                 # CUDA build directory (when FTD_ENABLE_CUDA=ON)
```

### Source line totals

| Component | Lines |
|-----------|-------|
| Headers (`include/ftd/*.h`) | ~5,500 |
| Sources (`src/*.cpp`) | ~5,000 |
| CUDA (`cuda/*.cu + CMakeLists`) | 3,218 |
| WASM bindings | 1,512 |
| Web CSS (external) | ~2,000 |
| Web JS (all modules) | ~25,000 |
| Config (JSON) | ~600 |
| **Total engine C++** | **~15,200** |
| **Total web frontend** | **~28,500** |

### Archived Components
```
archive/engine_v1_phenomenological/
  render_bridge.cpp       # Original ~1382-line phenomenological engine
  lagrangian.cpp          # 9-term Lagrangian diagnostics
  lagrangian.h            # 9-term Lagrangian definitions
  term_toggles.h          # 14-toggle system

archive/qt_gui/           # Qt6 native GUI (28 files, replaced by web UI)
```

---

## 3. Build and Run

### Tests only

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release
cd engine/build && ctest --output-on-failure -C Release
```

### WASM build (browser dashboard)

```bash
# Requires Emscripten SDK installed
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm
# Outputs: engine/build_wasm/wasm/ftd_core.js + ftd_core.wasm
# Copy to: engine/web/wasm/
cp engine/build_wasm/wasm/ftd_core.{js,wasm} engine/web/wasm/
# Serve:
python -m http.server 8080 -d engine/web
# Open: http://localhost:8080
```

### CLI simulation
```bash
./engine/build/Release/ftd_sim.exe [scenario] [lattice_size] [num_ticks]
```
Scenarios: `A` (Coulomb electron-proton), `B` (pair production from flux), `D` (locked particle stability), `E` (helium atom), `F` (gravitational cluster), `G` (scale stress test), `H`/`I`/`J` (CSV export variants), `K` (force law profile).

---

## 4. The Tick Cycle

Each call to `RenderBridge::tick()` executes these phases in order.
Every phase is gated by the corresponding `TermToggles` boolean.

```
tick() {
  1.  phase_read()          [wave_propagation || coupling]
  2.  phase_write()         [always runs; damping/genesis gated internally]
  3.  gauss_project()       [gauss_projection]
  4.  phase_forces()        [forces]
  5.  phase_movement()      [movement]
  6.  ++tick_
}
```

### Phase details

| Phase | Toggle | What it does |
|-------|--------|-------------|
| `phase_read` | `wave_propagation`, `coupling` | Computes delta_J: Laplacian wave equation (c^2 nabla^2 J) + state-flux coupling (g_c grad(s)) + Biot-Savart (g_c curl(s*v)). Dual-substrate path when enabled: independent Laplacians for J_L and J_R |
| `phase_write` | `damping`, `genesis`, `selective_damping`, `larmor_radiation` | Leapfrog: wave_vel += delta_J, flux += wave_vel. Damping: uniform (default), selective (near-particle only), or Larmor-modulated (acceleration-dependent). Genesis: \|J\| > K_GENESIS -> manifest (polarity from div(J), spin from curl(J), color from dominant axis). Evaporation: 7-site neighborhood energy < K_B^2 * 1e-6 -> void. Dual-substrate: independent leapfrog for L/R, observable sync |
| `gauss_project` | `gauss_projection` | SOR Poisson solver (omega=1.75, 30 iterations, warm-started): violation = div(J)-state, solve nabla^2 phi = violation, then J -= grad(phi) at **void sites only** (manifested sites skipped -- Phase 4 Approach B). Dual-substrate: Gauss sync propagates correction to J_L and J_R equally |
| `phase_forces` | `forces`, `gravity`, `lorentz_force`, `poisson_coulomb` | **Field-mediated only**: F_EM = -alpha*s*grad(phi_C) (Poisson, default) or -alpha*s*grad(div(J)) (legacy). Poisson solver: SOR omega=1.75, 30 iterations, warm-started. F_Lorentz = alpha*s*(v x B) where B=curl(J). F_grav = G_N*grad(rho) (tier-2 stencil). Optional: color_forces, strong_force, exchange_force (toggle-gated, default OFF). Per-particle force breakdown stored in `ForceDiag` |
| `phase_movement` | `movement` | Clears `moved_` flag buffer. Remainder accumulation, integer moves when remainder >= 1. Collisions: void->move, same-sign->bounce, opposite-sign->annihilate. Speed bounded by the γ_FTD integrator upstream (in `phase_forces`), which guarantees `v² /C² + L² < 1` by construction — no clamp here. Self-field and particle_id carried to new site. |

---

## 5. Constants Hierarchy

All physics constants derive from two inputs: **D = 3** (spatial dimensions) and **varpi** (lemniscate constant).
The derivation chain lives in `ontic.h` (9+ layers). `constants.h` re-exports everything into `ftd::`.

### Ontic chain summary (ontic.h)

| Layer | Constants | Source |
|-------|-----------|--------|
| -1 | `EULER_E` | Self-referential seed (e) |
| 0 | `EULER_GAMMA`, `GAMMA_QUARTER` | Transcendental seeds |
| 0b | `NOME_LEMNISCATIC`, `THETA_LEMNISCATIC` | Modular selection |
| 1 | `VARPI`, `GAUSS_CONSTANT_M`, `PI` | Elliptic geometry |
| 2 | `PF`, `G_STAR`, `SQRT_GSTAR` | Universal operator: G* = Gamma(1/4)/Gamma(3/4) ≈ 2.9587 |
| 2b | `K_CRIT`, `X_BORN` | Euler's identity / emergence of i |
| 3 | `COEFFICIENT` (16 G*^2), `X_PLUS` (137.036 = 1/alpha), `X_MINUS` (3.024 ~ N_c) | Master quadratic |
| 3b | `DELTA_SQ`, `DELTA_APPROX` | Dual-substrate splitting: delta^2 = (4G*-1)/(4G*) |
| 4 | `D_SPATIAL`=3, `N_C`=3, `N_GEN`=3, `N_F`=6, `N_BASE`=4, `B_3`=7, `N_EFF`=13 | Framework integers |
| 5 | `ALPHA`, `G_C`, `G_N`=0.01, `SIN2_WEINBERG` | Coupling constants |
| 6 | `K_B`=0.511, `K_GENESIS`=1.533 | Mass scale |
| 7 | Mass ratios, mixing angles, CP violation | Particle physics |
| 8 | Cosmological parameters, consciousness | Extended hierarchy |
| sim | `C_SPEED`=`C_WAVE`=1/sqrt(3), `DAMPING`=alpha | Simulation parameters |

### Active vs reference constants

**Active (used in engine kernels)**:

| Constant | Value | Used in |
|----------|-------|---------|
| `ALPHA` | 0.00729 (1/X_PLUS, tree-level) | Coulomb force, damping, exchange force |
| `ALPHA_EFT` | `G_C²` (≡ ALPHA by construction) | Same two-vertex force paths; consistency alias |
| `K_B` | 0.511 | Evaporation threshold, wavepacket amplitude, Larmor scale |
| `G_C` | sqrt(ALPHA) | State-flux coupling (phase_read) |
| `G_N` | 0.01 (lattice toy — see §5 gravity banner) | Gravitational force |
| `C_WAVE` | 1/sqrt(3) | Wave propagation speed (Laplacian coefficient) |
| `C_SPEED` | 1/sqrt(3) | Movement speed limit |
| `K_GENESIS` | 3 * K_B | Genesis threshold |
| `DAMPING` | alpha | Flux dissipation rate |
| `PHI` | 1.618... | Binding energy (triad detection) |
| `DELTA_APPROX` | 0.9568 | Dual-substrate splitting |
| `WEAK_THRESHOLD` | K_GENESIS | Weak transmutation stress threshold |
| `K_LARMOR` | 4/(3*K_B) | Larmor radiation modulation |
| `LARMOR_FLOOR` | 0.01 | Minimum Larmor factor |
| `ALPHA_S` | varies | Strong coupling (Yukawa force) |
| `YUKAWA_RANGE` | varies | Strong force range |
| `N_C` | 3 | Color charge count |

**Reference-only (computed in ontic.h, not read by engine kernels yet)**:

| Constant | Purpose |
|----------|---------|
| `X_PLUS_PRECISION` | 4-term corrected 1/α = 137.035999177 (matches CODATA). Opt in to swap from tree-level `X_PLUS`. |
| `ALPHA_PRECISION` | 1 / X_PLUS_PRECISION — use when benchmark precision surpasses 1 ppm. |
| `ALPHA_G_APPROX` | 5.9e-39 — *physical* gravitational coupling. Engine uses `G_N = 0.01` instead (see §5 gravity banner). |
| `MU_RATIO`, `TAU_RATIO`, etc. | Mass ratios (used by ParticleEngine / AtomEngine, not lattice) |
| `THETA_W`, `THETA_12`, `THETA_13`, `THETA_23` | Mixing angles (theoretical reference) |
| `DELTA_CP` | CP violation phase (theoretical reference) |
| `G_STAR`, `PF`, `X_PLUS`, `X_MINUS` | Master quadratic intermediates |
| `THETA_C`, `PHI_C` | Consciousness parameters (theoretical reference) |
| `LAMBDA_COSMO` | Cosmological constant (theoretical reference) |
| `EULER_E`, `EULER_GAMMA`, `GAMMA_QUARTER` | Mathematical seeds |

---

## 6. Voxel Structure

Each lattice site is represented by the `Voxel` struct (`voxel.h`, 175L):

### Core fields

| Field | Type | Description |
|-------|------|-------------|
| `state` | int8_t | Ternary: -1, 0, +1 |
| `flux` | Vec3 | Continuous vector field |
| `wave_vel` | Vec3 | Wave velocity (flux propagation) |
| `velocity` | Vec3 | Lattice velocity (nodes per G*-tick) |
| `remainder` | Vec3 | Sub-lattice position remainder |
| `particle_id` | int32_t | Persistent identity (-1 = no particle) |
| `pair_id` | int | Entanglement partner ID (-1 = none) |
| `spin` | int8_t | Z_2 from lemniscate topology (+1/-1/0) |
| `color` | int8_t | Z/3Z from 3-lobe structure (0-3) |
| `locked` | bool | Part of a bound structure? |
| `accel_mag` | double | Acceleration magnitude (for Larmor) |

### Dual-substrate fields (active when `dual_substrate = true`)

| Field | Type | Description |
|-------|------|-------------|
| `flux_L` | Vec3 | Left substrate flux |
| `flux_R` | Vec3 | Right substrate flux |
| `wave_vel_L` | Vec3 | Left substrate wave velocity |
| `wave_vel_R` | Vec3 | Right substrate wave velocity |

Observable: `flux = flux_L + flux_R`. Chirality: `chirality_density() = |psi_L|^2 - |psi_R|^2`.

### Deprecated fields (kept for binary compatibility)

`latency`, `tau`, `drag`, `attention`, `sloop_depth`, `is_sloop` -- always zero in v2.0+.

### Derived quantities

| Method | Formula |
|--------|---------|
| `density()` | `|flux|` |
| `speed()` | `|velocity|` |
| `bandwidth_used()` | `speed^2 + latency^2` |
| `gamma_ftd()` | `1/sqrt(1 - bandwidth_used)` |
| `born_infeld_core()` | `-K_B * sqrt(1 - bandwidth_used)` |

### ForceDiag struct

Per-particle force breakdown stored in a separate buffer (`force_diag_`) for UI diagnostics:

| Field | Type | Description |
|-------|------|-------------|
| `f_coulomb` | Vec3 | Electromagnetic (Poisson Coulomb) |
| `f_strong` | Vec3 | Strong nuclear (Yukawa) |
| `f_magnetic` | Vec3 | Lorentz magnetic (v x B) |
| `f_gravity` | Vec3 | Gravitational (grad rho) |
| `f_exchange` | Vec3 | Fermi exchange (Pauli) repulsion |

---

## 7. Force Computation

Forces are computed in `phase_forces()` as **field-mediated** interactions. No pairwise forces exist in the core engine.

### Force pipeline (per manifested particle)

1. **Electromagnetic (Coulomb-like)** -- two modes controlled by `toggles.poisson_coulomb`:

   **Poisson mode (default)**: `F_EM = -ALPHA * state * gradient_scalar(idx, phi_coulomb_)`
   - Solves nabla^2 phi_C = -s via warm-started SOR (omega=1.75, 30 iterations)
   - Measured exponent: **-2.25** (ideal: -2.0). GPU: **-2.067**
   - Isotropy ratio: **1.0** at r=5

   **Legacy mode** (`poisson_coulomb = false`): `F_EM = -ALPHA * state * gradient_divergence(idx)`

2. **Gravitational**: `F_grav = G_N * gradient_density(idx)` (tier-2 stencil, r=2)

3. **Lorentz (magnetic)** -- gated by `toggles.lorentz_force`:
   `F_Lorentz = ALPHA * state * cross(velocity, B)` where `B = curl(J)`

### Toggle-gated extensions (default OFF)

| Force | Toggle | Formula |
|-------|--------|---------|
| Color | `color_forces` | SU(3)-inspired pairwise color force |
| Strong | `strong_force` | Yukawa short-range nuclear force |
| Exchange | `exchange_force` | Pauli exclusion (same-spin repulsion) |

### E/B Field Diagnostics

`em_field_at(idx)` returns `{E, B}` where:
- **E = -wave_vel**: Electric field (negative time-derivative of flux)
- **B = curl(J)**: Magnetic field (curl of flux)

`poynting_vector(idx)` returns S = E x B. `EnergyAudit` includes `e_field_energy`, `b_field_energy`, `total_poynting`.

---

## 8. TermToggles

The `TermToggles` struct provides **20 runtime booleans** for the pedagogy system. Core rules default ON; extensions default OFF.

### Core toggles (logic-derived, default ON)

| Toggle | Gates |
|--------|-------|
| `wave_propagation` | Laplacian wave equation in phase_read |
| `coupling` | g_c * grad(s) source term in phase_read |
| `damping` | Dissipation flux *= (1-alpha) in phase_write |
| `genesis` | Manifestation + evaporation in phase_write |
| `gauss_projection` | Gauss constraint div(J) = s (SOR solver) |
| `forces` | Field-mediated EM + gravity |
| `gravity` | F_grav = G_N * grad(rho) in phase_forces |
| `movement` | Velocity integration + collision handling |
| `poisson_coulomb` | Poisson-based Coulomb (default). false = legacy grad(div J) |
| `lorentz_force` | Magnetic Lorentz force F = alpha*s*(v x B) |

### Extension toggles (default ON — promoted from OFF based on physics validation)

| Toggle | Default | Description |
|--------|---------|-------------|
| `selective_damping` | ON | Only damp sites near particles; vacuum waves propagate without loss |
| `dual_substrate` | ON | Split flux into J_L + J_R substrates with chirality |
| `weak_transmutation` | ON | Stress-threshold polarity flip (+1 <-> -1) |

### Extension toggles (default OFF — for exploration)

| Toggle | Description |
|--------|-------------|
| `larmor_radiation` | Acceleration-dependent damping (requires `selective_damping`) |
| `color_forces` | SU(3)-inspired color-dependent pairwise force |
| `strong_force` | Yukawa short-range nuclear force |
| `triad_binding` | Detect 3-particle triads, set locked=true |
| `pair_production` | Correlated +1/-1 pairs from high-flux void |
| `exchange_force` | Pauli exclusion repulsion (same-spin) |
| `latency_field` | Poisson-based latency field for gravity potential |

`enable_all()` enables core toggles; extensions remain OFF. `disable_all()` turns everything OFF.

---

## 9. Lagrangian System

The 4-term Lagrangian (in `lagrangian.h`) provides the variational foundation:

| Term | Expression | Physics |
|------|-----------|---------|
| L_BI | -K_B sqrt(1 - v^2) | Rest mass, special relativity |
| L_COUPLING | -g_c s div(J) | Electric (Coulomb-like) force |
| L_VELOCITY | -g_c s (v * J) | Magnetic (Lorentz-like) force |
| L_GAUSS | -lambda_G (div(J) - rho)^2 | Charge conservation, U(1) gauge |
| R (dissipation) | (alpha/2) \|wave_vel\|^2 | Vacuum drag |

`compute_lagrangian_diagnostics()` returns `LagrangianDiag` with per-term sums, Gauss violation, conservation checks.

---

## 10. Three Simulation Scales

### Scale 0: Voxel (RenderBridge)

The lattice engine. Each site is a Voxel with ternary state + continuous flux. Forces are field-mediated via discrete differential operators. Tick cycle: phase_read -> phase_write -> gauss_project -> phase_forces -> phase_movement.

### Scale 1: Particle (ParticleEngine)

Lattice-free engine with continuous positions and analytical forces. All constants from `ontic.h`.

**Force convention** (matches Scale 0 Poisson solver):
```
F_EM   = -alpha * q_i * q_j * r_hat / (4pi * (r^2 + soft^2))
F_grav = +G_N * m_i * m_j * r_hat / (r^2 + soft^2)
```

**Velocity Verlet** (symplectic): half-kick -> drift -> recompute -> half-kick. dt configurable, softening=1.0.

Files: `particle_engine.h` (108L), `particle_engine.cpp` (234L).

### Scale 2: Atom (AtomEngine)

Composite atoms with inter-atomic forces and covalent bonding. Three forces:
- **Ionic** (Coulomb): F = -alpha * Q_i * Q_j * r_hat / (4pi * r^2_soft)
- **Van der Waals** (LJ 12-6): 24 eps [2(sigma/r)^12 - (sigma/r)^6] / r
- **Covalent** (harmonic spring): -k * (r - r_eq) * r_hat

Automatic bond formation (r < 1.2 sigma_avg) and breaking (r > 2 r_eq). `compute_atomic_properties(Z, N)` derives all parameters from ontic constants.

Files: `atom_engine.h` (215L), `atom_engine.cpp` (427L).

### Scale Bridge

`coarsen()` extracts particles from lattice voxels. `refine()` calls `inject_wavepacket()` to reconstruct lattice state. Round-trip fidelity: position error = 0, velocity exact, energy error ~7e-13%.

`coarsen_to_atoms()` / `refine_to_particles()` for Scale 1 <-> 2.

Files: `scale.h` (68L), `scale_bridge.cpp` (202L).

---

## 11. Test Catalog

### Summary

| Category | Files | Checks |
|----------|-------|--------|
| Unit tests (test_*) | 108 | ~600+ |
| Campaign tests (campaign_*) | 47 | ~400+ |
| Five Minds campaigns | 5 | 15 |
| **Total** | **175+** | **1000+** |

All tests registered as CTests (170 CPU+GPU + 5 Five Minds campaigns). GPU tests (4 files) conditional on `FTD_ENABLE_CUDA`. GPU parity: 21/21 PASS. Five Minds campaigns: 15/15 PASS.

### Test categories

**Core infrastructure:**
- `constants` -- Ontic chain values, alpha precision, G* verification
- `lorentz` -- Lorentz factor, bandwidth limit, speed capping
- `lattice` -- Periodic wrapping, neighbor enumeration
- `voxel_properties` -- Voxel derived quantities (density, speed, bandwidth, gamma, Born-Infeld)
- `lattice_operators` -- Lattice topology, corner wrapping, neighbor symmetry, coord round-trip
- `discrete_operators` -- Laplacian, divergence, curl, gradient accuracy and symmetry
- `bridge_dynamics` -- RenderBridge tick cycle integration (vacuum stability, injection, propagation)

**Lagrangian verification:**
- `born_infeld`, `energy`, `gauss`, `stress_energy`, `thermodynamics`, `lagrangian`

**Ontic physics:**
- `ontic_chain`, `genesis`, `gravity_dynamics`, `annihilation`, `annihilation_conservation`, `wave_collapse`

**Wave and field:**
- `wave_speed`, `interference`, `gauge`, `polarization`, `momentum`, `magnetic`, `flux_mediated`, `entanglement`

**Lagrangian forces:**
- `variational_coulomb`, `magnetic_lagrangian`, `dissipation`, `complete_lagrangian`, `constant_activation`, `portable_field`

**Perfected Electromagnetism:**
- `maxwell` -- 6 sections (M1-M6): div(B)=0, Faraday, E perp B, Coulomb 1/r^2, wave equation, Ampere-Maxwell
- `em_energy_conservation` -- Vacuum EM energy conserved (drift < 0.01% over 2000 ticks)
- `continuity` -- Charge conservation exact through all dynamics
- `poynting` -- Poynting vector S = E x B verified (direction, magnitude, symmetry)
- `larmor` -- Acceleration-dependent damping (power proportional to a^2)
- `em_fields` -- E/B field diagnostics, E perp B for propagating waves
- `lorentz_force` -- Zero work, correct direction, toggle safety
- `selective_damping` -- Vacuum wave preservation, near-particle damping

**Poisson Coulomb (Phase 3):**
- `poisson_coulomb`, `energy_tracking`

**Energy Conservation (Phase 4):**
- `energy_conservation` (12 checks), `annihilation_conservation`

**Free Dynamics (Phase 5):**
- `campaign_free_dynamics` (10 checks), `particle_lifetime`

**Flux-Aggregate Particles (Phase 6):**
- `selffield_profile`, `wavepacket`, `campaign_aggregate_interaction`

**Multi-Scale (Phase 7):**
- `particle_engine` (22 checks), `scale_bridge` (9), `hydrogen_scale1` (6)
- `campaign_cross_scale`, `campaign_born_ensemble`

**Atom Engine (Phase 8):**
- `atom_engine` (16 checks), `atom_scale_bridge`, `campaign_h2_molecule`

**Dual Substrate:**
- `dual_substrate` -- Identity, chirality, conservation, backward compatibility

**Comprehensive logic engine:**
- `test_logic_engine` -- **42 checks** across 6 sections (Field Dynamics, Manifestation, Forces, Movement, Emergence, Lagrangian)

**10-Phase Proof-Out** (125+ checks):
- Phase 1: `campaign_statistical_convergence`
- Phase 2: `campaign_dispersion_convergence`, `campaign_coulomb_convergence`, `campaign_wave_isotropy`
- Phase 3: `campaign_bell_substrate`, `campaign_epr_correlation`, `campaign_born_rule`
- Phase 4: `campaign_hydrogen_binding`, `campaign_triad_energy`, `campaign_inertial_mass`, `campaign_structure_stability`
- Phase 5: `campaign_color_force`, `campaign_color_neutral`, `campaign_confinement`, `campaign_baryon_formation`
- Phase 6: `campaign_weak_transmutation`, `campaign_parity_violation`, `campaign_weak_decay`
- Phase 7: `campaign_gravitational_wave`, `campaign_gravity_profile`, `campaign_gravity_hierarchy`
- Phase 8: `campaign_triad_binding`, `campaign_neutrino_sector`
- Phase 9: `campaign_cosmological_predictions`
- Phase 10: `campaign_novel_predictions`

**Scientific Validation (Phase 11):**
- `test_falsifiability` (12 checks) -- Wrong parameters produce wrong physics
- `campaign_integer_sweep` (7 checks) -- {3,4,7,13} is unique among 315 combinations
- `campaign_hydrogen_spectrum` (8 checks) -- Quantitative hydrogen orbit (radius 0.0004% error)
- `campaign_two_slit` (7 checks) -- Interference fringes from two coherent sources

**GPU/CUDA** (conditional on `FTD_ENABLE_CUDA`):
- `gpu_parity` -- 21 checks: SoA round-trip, vacuum wave parity, energy parity (21/21 PASS)
- `gpu_benchmark` -- Performance timing ( at 64^3)
- `gpu_physics` -- 26 campaigns, 100+ checks: GP-COULOMB, GP-GAUSS, GP-WAVE-SPEED, GP-ENERGY-LONG, GP-GRAVITY, GP-ANNIHILATION, GP-MAXWELL-AMPERE, GP-EM-ENERGY, GP-CONTINUITY, GP-KCOMP-SHELL, GP-WEAK, GP-COLOR, GP-STRONG, GP-TRIAD, GP-PAIRS, GP-EXCHANGE, GP-BOUNCE, GP-DUAL-SUBSTRATE
- `gpu_experiments` -- Extended GPU experiments (timeout: 1800s)

**Five Minds Campaign Tests** (15/15 PASS):
- `campaign_plato` -- Ontological faithfulness (dispositional ratio, genesis threshold, void energy)
- `campaign_einstein` -- Conservation & covariance (energy conservation, Lorentz contraction, gravitational redshift)
- `campaign_vonneumann` -- Computational convergence (Coulomb scaling, wave speed, hydrogen binding)
- `campaign_wigner` -- Symmetry (octahedral O_h, parity violation, CPT invariance)
- `campaign_grothendieck` -- Structural universality (color force running, scale bridge, alpha from scattering)

---

## 12. Key Design Decisions

1. **Field-mediated forces ONLY**: F = -alpha*s*grad(phi_C) + G_N*grad(rho) (Poisson, default). No pairwise formulas. Whatever emerges IS the physics.

2. **Damping hierarchy**: Default: uniform flux decay at rate alpha. With `selective_damping`: only near-particle sites damp. With `larmor_radiation` (requires `selective_damping`): acceleration-modulated damping proportional to a^2 (correct Larmor scaling).

3. **No self-field floor (Phase 4)**: Particles are naturally stable via coupling source g_c*grad(s). Removing the floor eliminated ~4146% energy injection.

4. **K_GENESIS = 3 * K_B**: Genesis threshold at 3x evaporation, derived from N_c = 3.

5. **CFL-derived wave speed**: C_WAVE = 1/sqrt(3), the CFL stability limit for 6-neighbor Laplacian on 3D cubic lattice. DERIVED from D=3, not a free parameter.

6. **Tier-2 gravity gradient**: F_grav uses r=2 stencil to avoid self-field contamination.

7. **Neighborhood energy evaporation**: 7-site check (particle + 6 face-neighbors) for monotonically decreasing measure despite leapfrog oscillation.

8. **Gauss exclusion at particle sites**: Gauss projection skips manifested sites -- physically correct since div(J)(i) doesn't involve J(i).

9. **Poisson-based Coulomb**: SOR warm-started solver gives 1/r^2 force (exponent -2.25, isotropy 1.0). Replaces legacy double-gradient (exponent -3.8, isotropy 0.40).

10. **Sequential movement with moved_ guard**: Prevents double-processing after index-order moves.

11. **Lorentz magnetic force**: F = alpha*s*(v x B) does zero work (v*F = 0). Toggle-gated.

12. **E/B field decomposition**: E = -wave_vel, B = curl(J). Poynting vector S = E x B for energy flow diagnostics.

13. **Backward compatibility**: Removed phase functions exist as no-op stubs. Removed toggles exist as deprecated fields. Removed Lagrangian terms return 0.

14. **Double damping is intentional (Rayleigh dissipation)**: Both `flux` and `wave_vel` are damped by `(1-ALPHA)` each tick in `phase_write`. This is deliberate Rayleigh dissipation -- it damps both the position-like degree of freedom (flux) and the velocity-like degree of freedom (wave_vel). Damping only one would leave undamped oscillatory modes. The dual damping ensures monotonic energy decay in the field, which is required for stable self-field buildup and physically correct radiation loss.

15. **Speed limit enforced by γ_FTD momentum integration in phase_forces()**: As of 2026-04-17 (TRACKER §1.2), the velocity update in `phase_forces` uses `p = γmv` dynamics. Momentum reconstructs from `v + latency`, Newton's law updates `p`, and the new `v` extracts from `p` via `v = p · C · √((1−L²)/(C²+|p|²))`. This respects the FTD bandwidth `v²/C² + L² < 1` by construction — `|v|` asymptotes to `C·√(1−L²)`, never crosses. No clamp needed anywhere downstream; `phase_movement` receives an already-bounded velocity. Previous implementation used a non-relativistic clamp that discarded energy and was Lorentz-violating; the γ-integration replaces it cleanly.

---

## 13. RenderBridge Public API

### Core

| Method | Description |
|--------|-------------|
| `tick()` | Advance one tick (all 5 phases) |
| `diagnostics()` | Returns `Diagnostics` struct (counts, flux totals, charge) |
| `energy_audit()` | Returns `EnergyAudit` (field/wave/KE/PE breakdown, Gauss violation) — one-shot snapshot |
| `energy_ledger()` | Returns `const EnergyLedger&` — per-tick conservation drift (auto-populated on CPU path). Tests assert `abs(.residual) < tol` to refuse energy-drift regressions. GPU: call `update_energy_ledger()` manually after a device→host sync. |
| `update_energy_ledger()` | Populate the ledger (called automatically by `tick()` on CPU path) |
| `inject_particle(x,y,z, state)` | Inject single particle at lattice site |
| `inject_wavepacket(x,y,z, state, sigma, amplitude)` | Inject Gaussian wavepacket |
| `inject_flux(x,y,z, fx,fy,fz)` | Raw flux injection (overwrites site) |
| `inject_flux_add(x,y,z, flux_val)` | **[NEW Apr 2026]** Additive flux injection — accumulates instead of overwriting. Required by ported JS scenarios that sum overlapping Gaussians. |
| `inject_wave_vel_add(x,y,z, wv_val)` | **[NEW Apr 2026]** Additive wave-velocity injection — same additive semantics, for wave-equation initial conditions. |
| `create_entangled_pair(x,y,z, dx,dy,dz)` | Pair production with partner tracking |

### Diagnostics

| Method | Returns |
|--------|---------|
| `force_diag(idx)` | `ForceDiag` -- per-particle force breakdown |
| `em_field_at(idx)` | `EMFieldDiag {E, B}` |
| `poynting_vector(idx)` | `Vec3` (S = E x B) |
| `aggregate_profile(center, threshold)` | `AggregateProfile` (CoM, energy, r_eff, radial profile) |

### Configuration

| Method | Description |
|--------|-------------|
| `physical_time()` | Current tick * dt |
| `dt()` / `set_dt(val)` | Get/set timestep |
| `seed_rng(seed)` | Set RNG seed for reproducibility |
| `toggles` | Public `TermToggles` struct (20 booleans) |

### Scenario library

**[NEW April 2026]** `ftd::dispatch_scenario(RenderBridge& rb, const std::string& name)`
(declared in `include/ftd/scenarios.h`, implemented in `src/scenarios.cpp`,
~1240 LOC) is the public C++ entry point for scenario setup. It is a
straight port of the browser-side JS scenario library under
`engine/web/js/bridge/scenarios/` — the two code paths stay in lockstep
so that WASM, CLI, and native hosts all seed the lattice identically.

Dispatch tries five prefix groups in order and returns `true` on the
first match:

1. `flux-*` — pure-flux field initial conditions
2. `light-*` — photon-like wavepackets and coherent-state probes
3. `quantum-*` — superposition, entanglement, and measurement setups
4. `s0-seed-*` — Scale-0 manifested-particle seeds
5. `s0-field-*` — Scale-0 background-field presets

Returning `false` means no prefix matched; `wasm/ftd_wasm.cpp` falls
through to its legacy scenario `switch` for backward-compatibility with
older scenario names still referenced by UI code. The scenarios use the
new additive injectors (`inject_flux_add`, `inject_wave_vel_add`)
because many of them accumulate overlapping Gaussians and cannot use
the overwriting `inject_flux`.

---

## 14. CUDA GPU Engine

The GPU engine (`GpuEngine`) is a drop-in alternative to `RenderBridge`. All field data resides on the device; host transfers only diagnostics.

### Architecture

```
Host (CPU)                          Device (GPU)
inject_particle()  ---upload--->    d_state, d_flux_*, ...
inject_wavepacket()                 d_wave_vel_*, d_velocity_*
                   <--download---
diagnostics()                       tick() loop:
energy_audit()                        1. phase_read
sync_to_host()                        2. phase_write
                                      3. gauss (FFT)
                                      4. coulomb (FFT)
                                      5. forces
                                      6. movement
```

### FFT Poisson Solver

Replaces CPU's iterative SOR with spectral method via cuFFT:
- **Exact**: Gauss violation = 0.0 (vs CPU SOR ~ 1.14)
- **Single-pass**: No iteration count to tune
- Precomputed Green's function reused every tick

**Numerical parity note (F6 callstack audit 2026-04-17):** CPU and GPU
solve the SAME Poisson equation but with different numerical methods
(SOR iterative vs FFT spectral). CPU output carries a residual ≤ 10⁻⁴
at the default `SOR_ITERATIONS = 6`; GPU output is exact to floating-
point roundoff. Benchmarks comparing CPU vs GPU Poisson-dependent
quantities (Coulomb force, gauss_project, latency field) should account
for this ~10⁻⁴ systematic difference and not treat it as a regression.

### SoA Memory Layout

~200 bytes/voxel (26+ separate device arrays for coalesced access). At 128^3: ~400 MB.

### Build

```bash
cmake -S engine -B engine/build_cuda -DFTD_ENABLE_CUDA=ON -G Ninja \
      -DCMAKE_CUDA_FLAGS="--allow-unsupported-compiler"
cmake --build engine/build_cuda --config Release
```

Requirements: CUDA 13.0+, compute capability >= 8.9. Target architectures: "89;120" (Ada + Blackwell).

### Benchmarks (GPU)

| Lattice | CPU (ms/tick) | GPU (ms/tick) | Speedup |
|---------|---------------|---------------|---------|
| 16^3 | -- | -- | 18.6x |
| 32^3 | -- | -- | 41x |
| 48^3 | -- | -- | 193x |
| 64^3 | 134 | 0.37 | **** |

### GPU Physics Campaigns

26 campaigns, 100+ checks validating GPU parity at large lattice sizes:

| Campaign | Lattice | Key Result |
|----------|---------|------------|
| GP-COULOMB | 128^3 | Force exponent -2.067, R^2=0.9999 |
| GP-GAUSS | 128^3 | FFT violation = 0.0, charge exact 1000 ticks |
| GP-WAVE-SPEED | 128^3 | Axial 0.700 voxel/tick (1.21x CFL) |
| GP-ENERGY-LONG | 64^3 | 50K ticks, max drift 4.96%, charge exact |
| GP-GRAVITY | 128^3 | 20 particles, RMS shrinkage 12.6% |
| GP-ANNIHILATION | 64^3 | 20->2 particles, Q=0 exact |
| GP-MAXWELL-AMPERE | 128^3 | Standing wave E/B verification |
| GP-EM-ENERGY | 64^3 | Undamped vacuum bounded oscillation |
| GP-CONTINUITY | 128^3 | 10 pairs, Q=0 at all checkpoints |
| GP-DUAL-SUBSTRATE | 64^3 | Identity 3e-16, partition, backward compat |
| GP-KCOMP-SHELL | 128^3 | K_comp volumetric shell 10/10 |
| GP-BOUNCE | 64^3 | Same-sign elastic bounce verified |
| GP-WEAK/COLOR/STRONG/TRIAD/PAIRS/EXCHANGE | 64^3 | Toggle-gated physics extensions |

### Files

| File | Lines | Content |
|------|-------|---------|
| `gpu_engine.h` | 115 | GpuEngine class |
| `gpu_buffers.h` | 124 | SoA device memory layout |
| `gpu_buffers.cu` | 445 | Allocation, AoS<->SoA transfer |
| `gpu_engine.cu` | 496 | Tick loop, host<->device sync |
| `kernels_stencil.cu` | 1172 | Phase read/write + dual-substrate |
| `kernels_poisson.cu` | 328 | FFT Poisson solver |
| `kernels_forces.cu` | 737 | Forces + movement + extensions |
| `cuda/CMakeLists.txt` | 35 | Build rules |

---

## 15. Web UI (Browser Dashboard)

The C++ engine compiles to WASM via Emscripten. The browser dashboard provides zero-install access with Three.js 3D visualization.

### Architecture

```
ftd_core (C++ library)
    |
    +-- WASM Bindings (wasm/ftd_wasm.cpp, Embind)
    |       |
    |       +-- Browser Frontend (web/)
    |           +-- Three.js 3D viewport
    |           +-- Canvas 2D charts
    |           +-- Vanilla JS (ES modules, zero build step)
    |
    +-- CLI (src/main.cpp, native)
```

### Dashboard Layout (April 2026 refresh)

```
+----------------------------------------------------------------+
|  FTD Engine v2.14     [Engine ▼]                     [⚙]       |  Toolbar
+----------------------------------------------------------------+
|                                    [Visualization ▾]           |  Overlay (collapsible)
|                                     VOLUME  FIELDS  FORCES     |
|                                     QUANTUM PHENOMENA          |
|                                                                 |
|              Three.js 3D Viewport                               |  ~60%
|         (particles, wireframe, field overlays)                  |
|                                                                 |
|   ┌──────────────────── Scrub Bar ────────────────────┐         |
|   │ [▶] [▷] [⏵] [↺] │ Speed─●─ │ ⟲ [──timeline──] t  │         |
|   │  global  local            │       Render ⚙     │         |
|   └─────────────────────────────────────────────────────┘      |
+----+----+-----+----+----+----+----+----+-----------------------+
| Ctrl|Diag|Chart|Lag |Insp|Zoo |Hrk |QL  | Dock tabs            |
+----+----+-----+----+----+----+----+----+-----------------------+
|                Active Tab Panel                                 |  ~35%
+----------------------------------------------------------------+
| Running | Tick: 1,234 | Particles: 12 | 60 fps                  |  Status
+----------------------------------------------------------------+
```

Key changes from v2.11:
- **Toolbar** now hosts only branding, the Engine (scale) selector, and Settings. All playback controls moved to the floating scrub bar.
- **Floating Scrub Bar** (`js/ui/components/scrub-bar/`) — a 44-px glass pill at the viewport bottom with four semantic sections:
  1. *Controls*: global play (pill, accent fill) · local play (outline square, pulses when local-paused-global-running) · step · reset. Captions `global` / `local` beneath.
  2. *Speed*: uppercase `SPEED` · 90-px range slider · mono tick-per-frame readout.
  3. *Timeline*: reset-playhead button · LOD-shaded memory strip (sharp / blurry / static) · green render band on the right when a clip is present · time badge.
  4. *Actions*: `● Render` button and a settings kebab.
- **Overlay panel** (visualization toggles) has a chevron collapse affordance in its header that persists per-scale in localStorage (`ftd.overlay.scale0.collapsed`, etc.).
- **Panel dock** (bottom tabs) supports `data-panel-mount="bottom|left|right"` and `data-panel-width="narrow|normal|wide"` via the pre-paint hydration script in `<body>`.

### Playback Timeline (working-memory + render mode)

The scrub bar is backed by two capture strategies that share a single `TimelineBuffer` primitive (`js/scales/scale0/timeline/`):

- **MemoryRecorder** — live rolling window with LOD-tiered age decay. Snapshots enter at LOD 0 and are progressively block-averaged to LOD 1 (2× downsample) / LOD 2 (4×) / LOD 3 (audit-only) as they age across tier boundaries. Tier schedule auto-derives from a user-configurable byte budget (default 30 MB, ≈ 27 s of window at a 32³ lattice).
- **RenderController** — offline dense capture. User clicks the Render button; the controller runs ticks in ≤ 12 ms idle slices (`setTimeout(0)`) while sampling every `sampleEveryTicks = 4` ticks (15 fps @ 60 TPS). A budget-aware LOD picker selects the coarsest LOD (0 / 1 / 2) whose byte-cost × sample-count fits the render budget, then the whole clip is captured at that LOD — guaranteeing a dense, uniformly-sampled buffer for smooth forward and backward scrubbing. Emits `start / progress / done / cancel / error`. Cancellation restores the original engine state; partial clips are discarded.

Hydration uses two Scale 0 bridge capabilities:
- `getScale0Snapshot()` → `{ tick, lod, lattice, flux, wave, particles, audit }` (copies of MockBridge's `_stateGrid`, `_fluxJ`, `_fluxWV`, `_particles`).
- `loadScale0Snapshot(s)` — writes arrays back into the engine buffers. Accepts **any LOD**; LOD 1/2 inputs are upsampled nearest-neighbor to N³ before write (the JS-side `timeline/lod.js#upsampleScalar / upsampleVec3` helpers are published on `window.__ftdTimelineLod`). LOD 3 is telemetry-only and rejected.

Scrubbing is a pure "load, don't re-simulate" operation: `hydrateToTick(tick)` picks the nearest snapshot by tick from the render buffer (if an active clip exists) else the memory buffer, and loads it directly. No fast-forward ticks run during a drag, so the cost per scrub frame is one upsample + one buffer write — latency is independent of scrub distance. Pointer moves are coalesced to one hydrate per animation frame via `requestAnimationFrame`, so 240 Hz trackpads cannot saturate the loader. Live simulation resumes on pointerup (`onScrubEnd`).

### Panels Redesign (April 2026)

The three Scale 0 dashboard tabs were rebuilt on a shared chart/table primitive set:

- **Charts primitives** (`js/ui/charts/`): vendored uPlot 1.6.30, a theme reader that maps CSS custom properties into uPlot config, and three primitive classes:
  - `UPlotChart` — line/area with preallocated Float64Array buffers, DPR + ResizeObserver handling, localStorage-persisted series-hidden state.
  - `Sparkline` — axis-free micro chart for table Trend cells.
  - `StackedAreaChart` — custom `paths` renderer that cumulatively sums same-x points across series.
- **Diagnostics panel** (`js/ui/panels/diagnostics-panel/`): descriptor-driven `<table>` sections with `Metric | Value | Unit | Trend` columns, tabular-nums typography, zebra striping, digit-change pulse animation, and inline sparklines per row. The single Scale 0 descriptor declares 5 sections × 27 rows with physics-accurate units (`ct`, `E*`, `|J|`, `nat`, `|S|`, `ℏ`, `E*²`, `|w|²`).
- **Charts panel** (`js/ui/panels/charts-panel/`): horizontally-scrollable chip picker + auto-fit card grid. Chip toggles fully destroy / recreate chart cards — no leaked uPlot instances. Active-chart set persists in localStorage (`ftd.charts.active`).
- **Lagrangian panel** (`js/ui/panels/lagrangian-panel/`): StackedAreaChart with 7 bands · term-row checkboxes that two-way sync with the uPlot legend · `Action & Constraints` + `Ontic Constants` sidecar tables reusing `DiagnosticsTable`.

All three panels read live data from `TelemetryHub` (`js/telemetry-hub.js`), which now also exposes per-audit-field ring buffers under `hub.aud.*` (E-field, B-field, Poynting magnitude, particle KE, Coulomb PE, E_L, E_R, chirality, wave L/R, max-Gauss, self-field).

### Scenarios (23+)

**Scale 0 (Lattice):** Flux Pulse, Dipole, Proton+Electron, Genesis Cascade, Damping Demo, 4-Source Interference, Flux Vortex, Particle Collision, Pair Production, Hydrogen Atom, Gravity Cluster, Random Genesis, Rainbow, Lattice Prism, Dipole Radiation, Two-Slit, Photon Race, Dual Substrate, Entangled Pair, Annihilation, Force Law Profile

**Scale 1 (ParticleEngine):** Leptons: Hydrogen, Helium, Positronium, Muonium, True Muonium, Tauonium, Tauonic Hydrogen. Exotic Atoms: Pionic H, Kaonic H, Σ⁺ Atom, Protonium. Hadrons: Pionium, Kaonium, Δ⁺⁺ System, Ω⁻ Scattering. Nuclear: Deuteron, Tritium, Helion. Bosons: W⁺W⁻ Pair. Scattering: p-e, Three-body, π⁺-p, μ⁻-p. Custom. (23 scenarios)

**Scale 2 (AtomEngine):** Individual elements (118), Periodic Table. Noble Gas Clusters: He/Ar/Mix. Ionic Formation: NaCl/MgF₂/Lattice. Covalent Formation: H₂/O₂/CH₄. H-Bonding: Water Dimer/Pentamer. VSEPR Geometry: CO₂/CH₄/H₂O. Thermal Dynamics: Gas/Collision. Metallic Clusters: Fe BCC/Cu FCC. Custom. Phase 3 forces (JS MockBridge): H-bonds, angle strain, dipole-dipole, thermostat, electronegativity. Scale 3 molecules: 25-molecule library + NaCl Crystal

### Field Visualization Overlays (5 categorical groups)

The Scale 0 overlay panel is organised into five semantic columns; each column groups related toggles so the flat "9 keys" layout no longer scales. Hidden by default behind a collapse chevron; state persists per scale in `ftd.overlay.<scale>.collapsed`.

| Column | Toggles |
|--------|---------|
| **Volume** | Flux Volume (points), Flux Slice (XZ plane), Flux Lines (streamlines), ∇·J (divergence source/sink heatmap) |
| **Fields** | E Field, B Field, Poynting S, Light (photon bloom from \|S\|) |
| **Forces** | Force style selector (Arrows / Heatmap / Flow / Glyphs) applied to: EM, Gravity, Strong, Weak |
| **Quantum** | \|ψ\|², Phase φ, ℒ(x), Entropy s, Φ potential |
| **Phenomena** | Dual J, Chirality, DM Halo, Genesis, Damping, Confinement |

The Weak force shares the force-style selector but its "Arrows" mode renders additive-blended radial sprites (`PointsMaterial` + CanvasTexture gradient), not arrows — transmutation sites pulse along the intensity palette.

### Scale 2/3 Atom & Molecule Visualization (6 features)

Enhanced pedagogical visualization for Scale 2 (atoms) and Scale 3 (molecules):

| Feature | Implementation | Controls |
|---------|---------------|----------|
| **Enhanced nucleus** | Denser proton/neutron clouds (8 pts/nucleon), white center glow, larger radius | Always on |
| **Strong force shells** | Translucent orange InstancedMesh spheres (100 pool), AdditiveBlending, radius = 0.5 × cbrt(A) × 1.8 | Shells checkbox (default ON) |
| **Thick styled bonds** | CylinderGeometry InstancedMesh (1500 pool) with single/double/triple order support, CPK-blended colors | Bond style dropdown (Thick/Thin/Off) |
| **Bonding electron clouds** | Gaussian ellipsoidal point clouds along bond axes (8 × order points per bond, light cyan) | Clouds checkbox |
| **Orbital shell boundaries** | Translucent spheres per principal quantum number using Slater Z_eff (n=1 blue, n=2 green, n=3 orange, n=4+ pink) | Bounds checkbox (default OFF) |
| **Shaped orbital lobes** | Elongated ellipsoid InstancedMesh (2000 pool) for p/d/f valence orbitals, AdditiveBlending | Lobes checkbox (default OFF) |
| **Per-atom force arrows** | 4 LineSegments sets: Coulomb (red), vdW (green), Bond (orange), Net (white), log-compressed scaling | F_C / F_vdW / F_B / F_net toggle buttons |

Force decomposition computed via `aeGetForceDecomposition()` in MockBridge (ionic, vdW, bond, net). Arrows updated every 2nd frame for performance. All features auto-hidden on Scale 0/1 transitions via CSS `scale23-only` class and `setEngineMode()` cleanup.

### Boundary Containment (7 shapes)

Cube (periodic), Sphere, Octahedron, Dodecahedron, Icosahedron, Cylinder, Torus, None.

### Environment Backgrounds (6)

None, Star Field (default), Nebula, Quantum Foam, The Beyond, Flux Storm.

---

## 16. Dual-Substrate Mode

When `toggles.dual_substrate = true`, the single flux field J is replaced by two independent substrates J_L and J_R:

- **Observable**: psi = J_L + J_R (maintained automatically)
- **Chirality**: phi = J_L - J_R
- **Splitting**: delta^2 = (4G*-1)/(4G*) ≈ 0.9155; DELTA_APPROX ≈ 0.9568

**CPU implementation**: Independent Laplacians and leapfrog for L/R in phase_read/write. Gauss sync distributes correction equally.

**GPU implementation**: Dedicated dual kernels (`phase_read_dual_kernel`, `phase_write_dual_kernel`, `gauss_sync_dual_kernel`). Identity J = J_L + J_R maintained to machine precision (3.19e-16).

---

## 17. 10-Phase Proof-Out Scorecard

All 10 phases pass with 125+ individual checks:

| Phase | Campaign | Checks | Result |
|-------|----------|--------|--------|
| 1 | Statistical convergence | 5/5 | PASS |
| 2 | Continuum limit | 15/15 | PASS |
| 3 | Bell test & Born rule | 18/18 | PASS |
| 4 | Mass spectrum | 20/20 | PASS |
| 5 | Color dynamics | 16/16 | PASS |
| 6 | Weak sector | 12/12 | PASS |
| 7 | Gravitational sector | 13/13 | PASS |
| 8 | Particle Zoo | 13/13 | PASS |
| 9 | Cosmological predictions | 6/6 | PASS |
| 10 | Novel predictions & falsifiability | 7/7 | PASS |

### Key Results

| Observable | FTD Prediction | Measured | Precision |
|------------|---------------|----------|-----------|
| 4-term 1/alpha | 137.035999177 | 137.035999177(21) | **0.325 ppt** |
| Spectral index n_s | 0.9645 | 0.9649 +/- 0.0042 | **0.096 sigma** |
| sin^2 theta_W | 3/13 = 0.2308 | 0.2312 | **0.19%** |
| alpha_s(M_Z) | 7/59 = 0.1186 | 0.1179 +/- 0.0009 | **0.63%** |

### Six Falsification Criteria

1. No fourth generation of fermions with standard gauge couplings
2. Normal neutrino mass hierarchy (not inverted)
3. Proton decay with tau_p ~ 10^35 years
4. Tensor-to-scalar ratio r ~ 0.022
5. No WIMPs, no supersymmetry, no extra dimensions
6. Digit 13 of 1/alpha = 0

---

## 18. Emergence Observations

### Confirmed emergent behaviors

| Behavior | Evidence |
|----------|----------|
| Unlike charges attract | +1/-1 experience force toward each other |
| Like charges repel | +1/+1 experience force apart |
| Force ~ 1/r^2 | Poisson Coulomb exponent -2.25 (CPU), -2.067 (GPU) |
| Isotropic forces | Ratio 1.0 at r=5 |
| Gravity attracts | Both polarities drift toward density |
| Pair production | Flux > K_GENESIS creates +/- pairs |
| Bound states | Opposite charges survive 300+ ticks |
| Wave propagation | Flux pulses at C_WAVE |
| Interference | Two sources create fringes |
| Gauss constraint | div(J) approaches target |
| Self-field buildup | Coupling source builds steady-state EM envelope |
| Causality | No flux beyond C_WAVE * ticks |
| Energy conservation | 0.01% drift (Scale 0), 10^-10% (Scale 1) |

### Open questions

- Spontaneous triad formation without binding code -- not observed
- Stable orbits with radiation damping -- electrons spiral outward (correct physics)
- Sub-ppm alpha precision from higher-order corrections -- not demonstrated in engine

---

## 19. Scientific Status

**Overall grade: C+ for scientific credibility** -- excellent software engineering but insufficient external physics validation.

| Category | Grade | Notes |
|----------|-------|-------|
| Internal consistency | A | Charge exact, energy <1% drift |
| Force laws | B+ | Coulomb -2.07, R^2=0.9999 |
| Constants derivation | B | alpha to 1.26 ppm, integers are inputs |
| Integer uniqueness | A | Only {3,4,7,13} works (315 tested) |
| Negative results | A | 12 falsifiability checks pass |
| Hydrogen quantitative | A- | Virial exact, radius 0.0004% |
| Interference patterns | B+ | 6 fringes, good symmetry |
| External validation | F | Only external test (CERN) failed |

### Path forward

1. External cross-validation against lattice QCD, atomic spectroscopy
2. Statistical Born rule: 10K genesis events chi-squared test
3. Bell ensemble: S-parameter with confidence intervals
4. Blind predictions before looking at data

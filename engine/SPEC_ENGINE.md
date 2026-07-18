# FTD Simulation Engine Reference

**Living document for AI agents and developers.**
**Engine version:** 2.18.0 (single-sourced as `ftd::ENGINE_VERSION` in `include/ftd/constants.h`; mirrored by CMake `project(VERSION)`, `ftd_sim --version`, and the WASM `getEngineVersion()` binding — revision 6.1)
**Golden regression hash:** `0xb604d81a3d79366e` @ L=17 (`test_render_bridge_golden`). The two gauss audit scalars `gauss_violation`/`max_gauss_error` are summed only over vacuum (state==0) sites with the mean-subtracted, coupling-scaled target the SOR projection enforces; per-voxel state/flux/wave_vel/velocity is bit-exact; deterministic (OMP=1 == full pool). Rationale in `test_render_bridge_golden.cpp`.
**Test surface:** C++ tests, Playwright specs, and Python-adjacent verification helpers are registered through CMake and the web test harness. CTest uses the `unit`/`physics`/`golden`/`slow`/`gpu` label scheme; CUDA targets are conditional on `FTD_ENABLE_CUDA`.

## 0. System Narrative: From Field Capacity to Manifested Events

Scale 0 is the engine's discrete substrate. It does not begin with continuous
space plus particles. It begins with a finite cubic lattice, local update loops,
and two coupled voxel layers:

| Layer | Engine fields | Interpretation in the simulation |
|---|---|---|
| Discrete state | `state in {-1, 0, +1}` | Void, negative manifestation, positive manifestation |
| Flux field | `flux`, `wave_vel` | Dispositional vector field and its staggered wave velocity |
| Kinematics | `velocity`, `remainder` | Sub-lattice motion registers for manifested sites |
| Labels | `particle_id`, `pair_id`, `spin`, `color`, `locked` | Identity, pair correlation, internal labels, bound-state locks |
| Optional sectors | `flux_L/R`, `wave_vel_L/R`, `latency`, `tau` | Dual substrate and latency/proper-time extensions |

The continuous-looking physics in the dashboard and diagnostics is an emergent
large-scale behavior of repeated local steps. Each tick stages the work so that
parallel field loops read stable snapshots, while collision and lifecycle events
mutate state only after the relevant local information has been collected.

### 0.1 Runtime Loop Families

The Scale 0 engine is best understood as a composition of loops:

| Loop family | Main implementation | Mutability discipline | Role |
|---|---|---|---|
| Field read | `phase_read.cpp` | Read voxel snapshot, write delta buffers | 18-point Moore Laplacian, state-flux coupling, curl source |
| Field write | `phase_write.cpp` | Parallel voxel mutation | Staggered wave commit, damping/noise, genesis, evaporation |
| Constraint | `poisson_solvers.cpp` | Project flux | Gauss, Coulomb, and latency Poisson solves |
| Force | `phase_forces.cpp` | Update force diagnostics and velocities | Field-mediated force integration with bandwidth-limited velocity |
| Movement | `phase_movement.cpp` | Sequential guarded mutation | Integer moves, bounces, annihilation, self-field transfer |
| Lifecycle extensions | `transmutation_phases.cpp` | Toggle-gated mutation | Pair production, weak transmutation, triad binding, proper time |

This "loop dynamics" language refers to the engine execution loops and local
lattice dynamics. Theory-side perturbative loop-coefficient derivations live in
the theory/proof corpus and are not runtime kernels in Scale 0.

### 0.2 Manifestation Lifecycle

Manifestation is the state transition from high latent flux in a void cell to
an actual ternary site. The main path lives in `phase_write()`:

```
void site
  + flux density above K_GENESIS
  + deterministic stochastic draw below p
  -> state = +1 or -1
  -> spin/color inferred from local field geometry
  -> pending particle_id assigned
  -> deterministic ID resolution in voxel-index order
```

After manifestation, later phases can project the constraint, apply forces,
move the particle, bounce it, annihilate it with an opposite sign, evaporate it
back to void, flip it through weak transmutation, create correlated pairs, or
lock compact triads when the relevant toggles are enabled.

### 0.3 CPU/GPU Parity Model

The CPU path keeps `std::vector<Voxel>` as the authoritative array-of-structures
state. The CUDA path mirrors those fields into structure-of-arrays device
buffers. Host mutations are flushed to the GPU before a device tick; host reads
download the device state lazily. Both paths preserve the same logical phase
order even when solver implementations differ (CPU SOR vs GPU spectral/FFT
machinery where available).

---

## 1. Architecture: Logic-First Engine (v2.0)

The engine was rewritten from ~1382 lines of phenomenological code to a logic-first design. Only behaviors derivable from the axioms {3D lattice, ternary states, flux field, local causality, action principle} remain. Everything else was archived to `archive/engine_v1_phenomenological/`.

**Core rules (default Scale 0 substrate):**

1. **Flux wave equation**: dJ/dt = c^2 nabla^2 J (only possible local linear dynamics for a vector field)
2. **State-flux coupling**: source term g_c * grad(s) + g_c * curl(s*v) (from dS/dJ = 0)
3. **Gauss projection**: enforce div(J) = s each tick (charge conservation -- logical necessity)
4. **Manifestation/Evaporation**: |J| > K_GENESIS -> manifest; stochastic evaporation with per-tick probability p = exp(-E_local/K_MANIFEST^2) * K_EVAP_RATE, where E_local is the 7-site energy (particle + 6 face-neighbors; locked voxels exempt)
5. **Field-mediated forces**: F = -alpha * s * grad(phi_C) + G_N * grad(rho) + alpha * s * (v x B) where B = curl(J) (Poisson Coulomb + Lorentz magnetic + gravity)
6. **Movement + Collision**: remainder accumulation, speed limit C_SPEED = C_WAVE = 1/sqrt(3), annihilation on contact

**What was removed from the default core** (archived in `archive/engine_v1_phenomenological/`):
- Pairwise Coulomb, Yukawa, exchange, Lorentz forces
- QCD running coupling, color Yukawa
- Noetic/reference frame context coupling in the Scale 0 runtime
- Earlier always-on phenomenological latency/bandwidth/proper-time machinery

**Toggle-gated extensions** (default OFF, for pedagogy and exploration):
- Larmor radiation: acceleration-dependent damping (v2.11)
- Color forces, strong force, triad binding, pair production, exchange force
- Latency field and proper-time accumulation when `latency_field` is enabled

A few extension toggles are *promoted to default ON* and run in the default
tick (see the toggle table below and `term_toggles.h`): `dual_substrate`
(J_L + J_R chirality), `selective_damping`, and `weak_transmutation`
(stress-gated polarity flip). Note: `weak_transmutation` is a third J↔s
coupling not named by the two-channel ontology (FTD-0257); whether it should
remain default-on is an open governance question, not a settled rule.

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
  VISUAL_GUIDE.md             # Learner-facing visual guide to sim flow and discreteness
  CALLSTACKS.md               # Feature-by-feature runtime callstack map
  SCENARIO_ARCHITECTURE.md    # Scenario lifecycle, bridge ownership, and seed architecture
  print_ontic.py              # Utility to print ontic chain values
  include/ftd/
    scale_engine.h            # [v2.12] Abstract base class for all scale engines (111L)
    ontic.h                   # Ontic derivation chain (9+ layers), D=3 + varpi -> all constants (1221L)
    constants.h               # Re-exports ontic + engine-specific constants (279L)
    constants_shared.h        # Host+device shared `inline constexpr` constants (renamed from constants_gpu.cuh, revision 2.5; compiles under g++, MSVC, and nvcc; no `__constant__` memory)
    voxel.h                   # Vec3, ForceDiag, Voxel struct (203L)
    lattice.h                 # Lattice class -- 3D cubic grid with periodic boundaries (59L)
    render_bridge.h           # RenderBridge -- main engine API, tick(), diagnostics() (239L)
    lagrangian.h              # 4-term Lagrangian + Rayleigh dissipation (218L)
    term_toggles.h            # Scale 0 runtime toggle registry (33 booleans + typed config fields)
    csv_export.h              # Header-only CSV export (flux field, density slice, timeseries) (385L)
    particle_engine.h         # ParticleEngine : ScaleEngine -- Scale 1 particles (247L)
    atom_engine.h             # AtomEngine -- Scale 2 composite atoms + bonds (327L)
    cosmic_engine.h           # [v2.12] CosmicEngine : ScaleEngine -- Scale 5 N-body+SPH (523L)
    scale.h                   # OnticEntity + scale bridge declarations (83L)
    scenarios.h               # Public dispatch_scenario() -- C++ port of JS scenario library
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
    render_bridge.cpp         # Logic-first engine -- CPU tick ladder and backend handoff
    lagrangian.cpp            # compute_lagrangian_diagnostics() -- 4 active terms (166L)
    main.cpp                  # CLI entry point (scenarios A-K) (937L)
    particle_engine.cpp       # ParticleEngine: Velocity Verlet + analytical forces (394L)
    atom_engine.cpp           # AtomEngine: ionic + vdW + covalent forces (762L)
    cosmic_engine.cpp         # [v2.12] CosmicEngine: Barnes-Hut + SPH + Friedmann (900L)
    scale_bridge.cpp          # Scale 0<->1<->2<->5 coarsen/refine round-trip (283L)
    scenarios.cpp             # 83 scenarios from JS ported to C++ (flux-/light-/quantum-/s0-seed-/s0-field-)
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
      scale4.json             # 10 reference frame context scenarios + 12 figures
      scale5.json             # 4 cosmic scenarios + camera presets
      scale6.json             # Meta scenario + 13 toggle controls
  tests/
    257 test files            # 211 active CMake targets
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
        scenarios.js          # Reference frame context scenario descriptions
      bridge/                 # Simulation bridge layer
        bridge-factory.js     # createBridge() factory (WASM -> MockBridge fallback)
        mock-scale5.js        # CosmicMockBridge (JS-only N-body for dev)
      scales/                 # Per-scale controllers (each owns its own state)
        scale0/controller.js  # Lattice: animateLattice, loadScenario, field viz (702L)
        scale1/controller.js  # Particles: animatePE, cloud rendering, trails (912L)
        scale2/controller.js  # Atoms: animateAE, orbital clouds, force arrows (1056L)
        scale3/controller.js  # Molecules: loadMolecule, reuses Scale 2 animate (217L)
        scale4/controller.js  # Reference frame context: sLoop, Mandelbrot, hologram (443L)
        scale5/controller.js  # Cosmic: N-body, galaxy rendering (193L)
        scale6/controller.js  # Meta: existential unit, geometry toggles (150L)
      viewport.js             # Three.js 3D: particles, bonds, orbitals, fields, camera
      wasm-bridge.js          # WasmBridge + MockBridge (auto-fallback)
      cosmic-renderer.js      # [v2.12] Photorealistic cosmic body rendering
      reference frame context.js        # Reference frame contextEngine (sLoop, measurement cascade)
      reference frame context-pedagogy.js  # Pedagogical visualizations (Canvas 2D)
      reference frame context-figure.js    # Holographic figure (Three.js)
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
cmake --build engine/build --config Release --parallel 24
ctest --test-dir engine/build -j 24 --output-on-failure -C Release
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
python engine/web/serve.py 8080
# Open: http://localhost:8080
```

### CLI simulation
```bash
./engine/build/Release/ftd_sim.exe [scenario] [lattice_size] [num_ticks]
```
Scenarios: `A` (Coulomb electron-proton), `B` (pair production from flux), `D` (locked particle stability), `E` (helium atom), `F` (gravitational cluster), `G` (scale stress test), `H`/`I`/`J` (CSV export variants), `K` (force law profile).

---

## 4. The Tick Cycle

Each call to `RenderBridge::tick()` validates the active `TermToggles`, syncs
the ternary mirrors if needed, then either delegates to the GPU backend or runs
the CPU phase ladder. Not every phase is enabled in every run; the bracketed
toggle tells where a phase enters.

```
RenderBridge::tick()
  0.  validate toggles
  0b. sync_ternary_from_voxels_if_needed()
  1.  phase_read()                 [wave_propagation || coupling]
  2.  phase_write()                [always; damping/genesis/evaporation gated inside]
  2b. pair_production_cpu()        [pair_production]
  3.  gauss_project()              [gauss_projection]
  3b. solve_latency_poisson()      [latency_field]
  4.  phase_forces()               [forces]
  5.  phase_movement()             [movement]
  5b. apply_absorbing_boundary()   [absorbing_boundary]
  6.  weak_transmutation_cpu()     [weak_transmutation]
  7.  triad_binding_cpu()          [triad_binding]
  7b. relax_su2/su3_links_cpu()    [su2_gauge / su3_gauge]  (links only — no substrate writes)
  8.  accumulate_proper_time()     [latency_field]
  9.  physical_time_ += dt_; ++tick_
  10. sync/dirty flags/energy ledger updates
```

### 4.1 CPU phase details

| Phase | Main toggles | What it does |
|-------|--------------|-------------|
| `phase_read` | `wave_propagation`, `coupling` | Parallel read-only voxel loop. Computes `delta_J` from the 18-point Moore Laplacian, `G_C * grad(state)`, and `G_C * curl(state * velocity)`. Dual-substrate mode computes separate L/R deltas and recombines observables after write. |
| `phase_write` | `damping`, `genesis`, `evaporation`, `selective_damping`, `larmor_radiation`, `langevin`, `symplectic_leapfrog` | Parallel commit loop. Advances the staggered wave pair (`wave_vel += delta_J`, `flux += wave_vel`; explicit `dt` factors when `symplectic_leapfrog` is enabled), applies damping/noise, performs genesis and evaporation, snapshots pre-write fields for deterministic labels, and resolves pending manifestation IDs after the parallel section. |
| `pair_production_cpu` | `pair_production` | Creates neighboring correlated `-1/+1` pairs from high-flux voids, consumes local wave/flux energy, assigns shared pair IDs, and conserves charge locally. |
| `gauss_project` | `gauss_projection`, `exact_dual_gauss` | Builds `source = div(J) - coulomb_charge_coupling * state`, solves a warm-started SOR Poisson problem, then subtracts `grad(phi)` from flux. Ordinary mode skips manifested sites during correction; exact dual mode synchronizes the split L/R fields. |
| `solve_latency_poisson` | `latency_field`, `field_energy_gravity` | Builds a mass/field-energy source, solves a latency potential, and stores bounded `latency` values for time dilation and bandwidth accounting. |
| `phase_forces` | `forces`, `poisson_coulomb`, `emergent_forces`, `gravity`, `lorentz_force`, `color_forces`, `strong_force`, `exchange_force`, `cluster_inertia` | Iterates manifested sites. Default EM path solves Coulomb potential and applies `-ALPHA * state * grad(phi_C)`; emergent-force mode uses direct flux gradients instead. Adds gravity, Lorentz, and optional particle-sector forces, writes `ForceDiag`, and integrates velocity using the `gamma_FTD` bandwidth budget. |
| `phase_movement` | `movement`, `symmetric_movement_order` | Sequential guarded mutation. Accumulates sub-lattice remainders, moves into void targets, bounces same-sign collisions, annihilates opposite signs, carries self-field to moved particles, and bursts field energy on annihilation. |
| `apply_absorbing_boundary` | `absorbing_boundary` | Damps the outer shell after movement to reduce periodic wraparound artifacts in selected runs. |
| `weak_transmutation_cpu` | `weak_transmutation` | Stress-threshold stochastic polarity flips. In dual-substrate mode the L/R fluxes are swapped with the flip. |
| `triad_binding_cpu` | `triad_binding` | Detects compact same-sign triples and locks them as bound structures. |
| `relax_su2/su3_links_cpu` | `su2_gauge`, `su3_gauge` | One Jacobi double-buffered Wilson staple sweep per tick over the SU(2)/SU(3) edge links ([IMPOSED] lattice-gauge import; see §8.1). Write-only w.r.t. the substrate — links feed nothing downstream. Buffers lazily allocated on first use. |
| `accumulate_proper_time` | `latency_field` | Updates `tau` for manifested sites using the latency/speed bandwidth factor. |

### 4.2 GPU phase ladder

When a CUDA backend is active, `RenderBridge::tick()` delegates to
`backend_->tick()`. `GpuBackend` flushes host mutations to device buffers, copies
the current toggles into `GpuEngine`, runs the device tick, and marks the host
shadow dirty until diagnostics or voxel access require a download.

`GpuEngine::tick()` preserves the same logical order:

```
gpu_phase_read()
gpu_phase_write()
gpu_pair_production()        [pair_production]
gpu_gauss_project()          [gauss_projection]
gpu_latency_solve()          [latency_field]
gpu_phase_forces()           [forces]
gpu_pairwise_extensions()    [color/strong/exchange/triad-related toggles]
gpu_phase_movement()         [movement]
gpu_weak_transmutation()     [weak_transmutation]
tick_++
```

The main backend difference is numerical plumbing: the CPU path uses
warm-started SOR buffers for Poisson solves, while the GPU path uses device-side
parallel kernels and spectral/FFT solvers where the CUDA implementation supports
them.

### 4.3 Integration-scheme notes

- The field advance is a staggered update. The default unit-tick form is
  `wave_vel += delta_J; flux += wave_vel`. The `symplectic_leapfrog` toggle
  applies the same staggered update with explicit `dt` factors.
- The 18-point Moore Laplacian uses face weight `1/3`, edge weight `1/6`, and
  self weight `-4`, giving the documented isotropy behavior through the
  tested order.
- The velocity update in `phase_forces` uses the `gamma_FTD` bandwidth budget
  rather than a late hard clamp in `phase_movement`.

---

## 5. Constants Hierarchy

The ontic reference chain computes many framework constants from **D = 3**
(spatial dimensions) and **varpi** (lemniscate constant). Active engine kernels
use a smaller subset of those values plus simulation-scale parameters. This
section documents code usage; derivation status and parameter/derivation
distinctions are tracked in the theory ledgers.

### Ontic chain summary (ontic.h)

| Layer | Constants | Source |
|-------|-----------|--------|
| -1 | `EULER_E` | Self-referential seed (e) |
| 0 | `EULER_GAMMA`, `GAMMA_QUARTER` | Transcendental seeds |
| 0b | `NOME_LEMNISCATIC`, `THETA_LEMNISCATIC` | Modular selection |
| 1 | `VARPI`, `GAUSS_CONSTANT_M`, `PI` | Elliptic geometry |
| 2 | `PF`, `G_STAR`, `SQRT_GSTAR` | Universal operator: G* = Gamma(1/4)/Gamma(3/4) ≈ 2.9587 |
| 2b | `K_CRIT`, `X_BORN` | Euler's identity / emergence of i |
| 3 | `COEFFICIENT` (16 G*^2), `X_PLUS` (tree-level 1/alpha), `X_MINUS` (retired as an `N_C` source; mathematical root only) | Master quadratic |
| 3b | `DELTA_SQ`, `DELTA_APPROX` | Dual-substrate splitting: delta^2 = (4G*-1)/(4G*) |
| 4 | `D_SPATIAL`=3, `N_C`=3, `N_GEN`=3, `N_F`=6, `N_BASE`=4, `B_3`=7, `N_EFF`=13 | Framework integers |
| 5 | `ALPHA`, `G_C`, `G_N`=0.01, `SIN2_WEINBERG` | Coupling constants |
| 6 | `K_B`=0.511 (mass anchor), `K_MANIFEST`=0.505462 (:= W_SC, FTD-0388), `K_GENESIS`=1.516386 | Mass scale |
| 7 | Mass ratios, mixing angles, CP violation | Particle physics |
| 8 | Cosmological parameters, reference frame context | Extended hierarchy |
| sim | `C_SPEED`=`C_WAVE`=1/sqrt(3), `DAMPING`=alpha | Simulation parameters |

### Active vs reference constants

**Active (used in engine kernels)**:

| Constant | Value | Used in |
|----------|-------|---------|
| `ALPHA` | 0.00729 (1/X_PLUS, tree-level) | Coulomb force, damping, exchange force |
| `ALPHA_EFT` | `G_C²` (≡ ALPHA by construction) | Same two-vertex force paths; consistency alias |
| `K_B` | 0.511 | Wavepacket amplitude, Larmor scale (mass anchor; the kinetics role moved to K_MANIFEST per FTD-0388) |
| `K_MANIFEST` | 0.5054620197 (:= W_SC [SELECTION — ADOPTED, FTD-0388]) | Boltzmann evaporation scale (p = exp(-E/K_MANIFEST²)·K_EVAP_RATE), genesis probability ramp |
| `G_C` | sqrt(ALPHA) | State-flux coupling (phase_read) |
| `G_N` | 0.01 (lattice toy — see §5 gravity banner) | Gravitational force |
| `C_WAVE` | 1/sqrt(3) | Wave propagation speed (Laplacian coefficient) |
| `C_SPEED` | 1/sqrt(3) | Movement speed limit |
| `K_GENESIS` | 3 * K_MANIFEST = 1.516386 (FTD-0388) | Genesis threshold |
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
| `THETA_C`, `PHI_C` | Reference frame context parameters (theoretical reference) |
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

### Latency and proper-time fields

`latency` and `tau` are active when `latency_field` is enabled. `latency` is a
bounded gravitational-potential proxy solved by the latency Poisson path;
`tau` accumulates proper time for manifested particles. Older noetic/reference
frame context fields such as `drag`, `attention`, and sLoop markers are not
part of the current `Voxel` runtime surface.

### Derived quantities

| Method | Formula |
|--------|---------|
| `density()` | `|flux|` |
| `speed()` | `|velocity|` |
| `bandwidth_used()` | `v^2` when `latency == 0`; otherwise `v^2 / f`, where `f = 1 - latency^2` |
| `gamma_ftd()` | `1/sqrt(1 - v^2)` when `latency == 0`; otherwise `sqrt(f) / sqrt(f^2 - v^2)` |
| `born_infeld_core()` | `-M_REST * sqrt(1 - v^2)` when `latency == 0`; otherwise `-M_REST * sqrt(f^2 - v^2) / sqrt(f)` |

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

The `TermToggles` struct is a table-driven Scale 0 runtime registry. It contains
**33 boolean toggles** in `TOGGLE_SPECS[]` plus **6 typed configuration fields**
that are intentionally kept outside the boolean table.

Adding a new boolean toggle requires a struct field and one registry row; the
helper methods (`validate`, `enable_all`, `disable_all`,
`cpu_runtime_warnings`) consume the table.

### 8.1 Boolean toggle groups

| Group | Toggles | Role |
|---|---|---|
| Core field/state | `wave_propagation`, `coupling`, `damping`, `genesis`, `evaporation`, `gauss_projection` | Wave propagation, state coupling, dissipation, manifestation/evaporation, Gauss projection |
| Forces and motion | `forces`, `gravity`, `poisson_coulomb`, `emergent_forces`, `lorentz_force`, `movement` | Field-mediated force modes and kinematic update |
| Field extensions | `dual_substrate`, `exact_dual_gauss`, `latency_field`, `field_energy_gravity`, `symplectic_leapfrog` | Split substrate, latency/proper-time sector, explicit-`dt` wave integration |
| Damping/noise/boundary | `selective_damping`, `larmor_radiation`, `langevin`, `absorbing_boundary`, `symmetric_movement_order` | Damping modes, stochastic thermostat, boundary sponge, traversal artifact control |
| Particle-sector extensions | `color_forces`, `weak_transmutation`, `strong_force`, `triad_binding`, `pair_production`, `exchange_force`, `cluster_inertia`, `confinement` | Color/strong/exchange explorations, weak flips, pair production, bound clusters, confinement intent flag |
| Gauge/validation flags | `su2_gauge`, `su3_gauge`, `strict_validation` | Per-tick SU(2)/SU(3) link staple relaxation (tick Rule 7b) and strict validation behavior |

**Non-Abelian gauge sector (revision 0.9 option a — wired 2026-07-02).**
`su2_gauge` / `su3_gauge` (default OFF) gate one Jacobi-double-buffered
Wilson-action staple sweep per tick over the SU(2)/SU(3) edge link variables:
CPU `relax_su2/su3_links_cpu` (tick Rule 7b, `transmutation_phases.cpp`), GPU
`kernels_gauge.cu` via `GpuEngine::gpu_gauge_relax()` with `GpuBackend`
marshalling host↔device (upload once on activation, download each gauge tick).
Epistemic status: **[IMPOSED]** — the staple/plaquette relaxation form and its
rate calibrations (`GAUGE_RELAX_DT`, `GAUGE_RELAX_BETA` in `constants.h`) are
imported from standard lattice gauge theory, not derived from the postulates.
The links are **write-only w.r.t. the substrate**: nothing downstream consumes
them (`color_forces` uses per-voxel color labels, not links), so the wired
sector is measurement infrastructure — a live link field on which
plaquette/Wilson-loop observables can later be defined against engine state —
and **no LEDGER claim rides on it** (the Moore-layer gauge-group results are
independent of this code path and gain no evidence from it). Guarantees, all
test-enforced: toggles-OFF runs are bit-identical to every pinned golden;
toggles-ON runs leave the substrate fold unchanged (`test_gauge_links` G1a)
and reproduce the pinned gauge golden profile `GAUGE_GOLDEN_HASH` (G1b,
ADR-0012, bit-identical MSVC↔WSL2-gcc); link buffers are lazily allocated
(528 B/site only when the sector is used, revision 4.1b); CPU/GPU agree to
machine-epsilon scale with bit-exact GPU determinism (`test_gauge_gpu_parity`,
WSL2-canonical).

### 8.2 Defaults and validation

Defaults are specified per row in `TOGGLE_SPECS[]`. Core substrate behavior
defaults on (`wave_propagation`, `coupling`, `damping`, `genesis`,
`gauss_projection`, `forces`, `gravity`, `poisson_coulomb`, `movement`,
`lorentz_force`). Some promoted extension toggles also default on
(`selective_damping`, `dual_substrate`, `weak_transmutation`). Exploration
toggles such as `pair_production`, `latency_field`, `langevin`, color/strong
extensions, and exact dual Gauss are default off.

`validate()` enforces dependencies and conflicts. Important examples:

| Rule | Reason |
|---|---|
| `poisson_coulomb` conflicts with `emergent_forces` | Avoids running two mutually exclusive EM force models |
| `lorentz_force` requires `forces` | Lorentz is part of the force phase |
| `selective_damping` requires `damping` | It refines the damping path rather than replacing the master switch |
| `weak_transmutation` requires `dual_substrate` | Current CPU/GPU implementation uses chirality/split-substrate state |
| `triad_binding` requires `color_forces` | Triad detection depends on color labels/interaction context |
| `field_energy_gravity` requires `latency_field` | Field energy enters through the latency Poisson source |

`enable_all()` applies each table row's default value for bulk-managed toggles;
it does not blindly turn every experimental flag on. `disable_all()` turns
bulk-managed booleans off while preserving direct control of non-bulk/internal
flags as defined by the registry.

### 8.3 Non-boolean configuration fields

| Field | Type | Purpose |
|---|---|---|
| `bcc_stencil` | `BccStencilMode` | Selects the sublattice stencil path for `phase_read`; non-default modes require single-substrate validation. |
| `langevin_site_filter` | `SiteClass` | Selects which parity/site class the Langevin thermostat targets. |
| `langevin_T` | `double` | Target Langevin temperature. |
| `langevin_gamma` | `double` | Langevin damping/noise rate. |
| `langevin_seed` | `unsigned int` | Deterministic stochastic seed. |
| `coulomb_charge_coupling` | `double` | Scalar in the Gauss-law source term. |

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

The lattice engine. Each site is a Voxel with ternary state + continuous flux.
Forces are field-mediated via discrete differential operators. The current tick
ladder is documented in §4: read/write, optional pair production, Gauss,
latency, forces, movement, boundary, weak/triad, proper-time, and ledger sync.

### Scale 1: Particle (ParticleEngine)

Lattice-free engine with continuous positions and analytical forces. All constants from `ontic.h`.

**Force convention** (matches Scale 0 Poisson solver):
```
F_EM   = -alpha * q_i * q_j * r_hat / (4pi * (r^2 + soft^2))
F_grav = +G_N * m_i * m_j * r_hat / (r^2 + soft^2)
```

**Velocity Verlet** (symplectic): half-kick -> drift -> recompute -> half-kick. `dt` and softening are configurable; the C++ default softening is 1.0 and web scenario presets commonly set 0.1 for atomic-scale demos.

Diagnostics report the active Hamiltonian only: Coulomb PE is zero when the Coulomb toggle is off, gravity PE is zero when gravity is off, and `total_pe = coulomb_pe + gravity_pe`. The WASM Scale 1 binding exposes particle positions, velocities, masses, locked flags, effective radii, charges, IDs, extended telemetry, and snapshot force vectors so browser overlays can be backend-true.

Files: `particle_engine.h`, `particle_engine.cpp`, `wasm/bindings_particle.cpp`, and `web/js/scales/scale1/*`.

### Scale 2: Atom (AtomEngine)

Composite atoms with inter-atomic forces and covalent bonding. Three forces:
- **Ionic** (Coulomb): F = -alpha * Q_i * Q_j * r_hat / (4pi * r^2_soft)
- **Van der Waals** (LJ 12-6): 24 eps [2(sigma/r)^12 - (sigma/r)^6] / r
- **Covalent** (harmonic spring): -k * (r - r_eq) * r_hat

Automatic bond formation (r < 1.2 sigma_avg) and breaking (r > 2 r_eq). `compute_atomic_properties(Z, N)` derives all parameters from ontic constants.

**Atomic closure-context vector (diagnostic/readout only).** `compute_atomic_properties(Z, N)` also returns `closure_context`, and `AtomEngine::closure_context_for(id, cfg)` exposes the same shell-context readout for live atoms. The vector records `Z`, `n_shell`, `z_eff`, `r_cloud`, `delta_valence`, `xi_orbital`, `tau_electronic`, and ratios such as `kappa`, `zeta`, `beta`, and `theta`. Its cloud scale follows the shell-context estimate `r_cloud = R_BOHR*n_shell^2/z_eff`: across a period, stronger screened return force contracts the cloud; at a new shell, the scale resets outward. This is a physics-facing scale diagnostic, not a force retuning. `Atom.radius` and `vdw_sigma` remain the legacy simulation/LJ interaction scales used by bonding, CUDA pair-force uploads, and scale bridges.

**JS <-> C++ constant divergence (deliberate, [IMPOSED] both sides).** The C++ AtomEngine derives force prefactors from the ontic chain (Coulomb `ALPHA/(4pi)` in `atom/atom_forces.cpp`; bond spring `ALPHA*K_B/r_eq^2*order` in `atom_engine.cpp`), while the web mock (`web/js/bridge/mock-atom-engine.js`) uses visualization-scale MD tunings from `web/js/constants.js` (`AE_K_COULOMB = 2.0`, `AE_K_BOND = 50.0`, plus a 3.5*r_eq break threshold vs C++'s 2*r_eq). Both parameter sets are calibrations, not derivations; force magnitudes and equilibrium time scales are NOT expected to match across backends. **The JS mock is the production Scale-2/3 backend** — `wasm-bridge.js` `_aeHasWasm` is deliberately disabled (audit P1-2, deferred feature D-11) until a Planck-unit <-> Bohr-unit conversion shim exists, so every browser Scale-2/3 readout comes from the JS engine. Cross-backend numeric comparisons of AE outputs are meaningless until that shim lands.

Files: `atomic_closure_context.h`, `atom_engine.h`, `atom_engine.cpp`, `src/atom/atom_forces.cpp`, `web/js/atomic-props.js`, `web/js/bridge/mock-atom-engine.js` (production web backend).

### Scale Bridge

`coarsen()` extracts particles from lattice voxels. `refine()` calls `inject_wavepacket()` to reconstruct lattice state. Round-trip fidelity: position error = 0, velocity exact, energy error ~7e-13%.

`coarsen_to_atoms()` / `refine_to_particles()` for Scale 1 <-> 2.

Files: `scale.h` (68L), `scale_bridge.cpp` (202L).

---

## 11. Test Catalog

### Summary

Project-level count: **257 C++ test source files**, **211 active CMake
targets**, plus 18 Playwright specs and 25 Python test files. CTest labels
include `unit`, `physics`, `golden`, `slow`, and `gpu`; CUDA targets are
conditional on `FTD_ENABLE_CUDA`.

The category list below is a representative map of the suite rather than a
line-by-line target registry.

### Test categories

**Core infrastructure:**
- `constants` -- Ontic chain values, alpha precision, G* verification
- `lorentz` -- Lorentz factor, bandwidth limit, speed capping
- `lattice` -- Periodic wrapping, neighbor enumeration
- `voxel_properties` -- Voxel derived quantities (density, speed, bandwidth, gamma, Born-Infeld)
- `lattice_operators` -- Lattice topology, corner wrapping, neighbor symmetry, coord round-trip
- `discrete_operators` -- Laplacian, divergence, curl, gradient accuracy and symmetry
- `bridge_dynamics` -- RenderBridge tick cycle integration (vacuum stability, injection, propagation)
- `scale_ratio` -- FC-3 identity criterion: `ScaleRatio` value object (χ = ξ/R, β = δ/R), `is_phenomenon()`, `observe()`; header-only, NO_CORE, α-blind (23 assertions; `engine/include/ftd/scale_ratio.h`)

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
- `atom_engine` (properties, closure context, forces, bonding), `atom_scale_bridge`, `campaign_h2_molecule`

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

**Readout admissibility (scale-context gate):**
- `scale_context` -- read-only, α-blind scale-context gate (`engine/src/scale_context.cpp`):
  per-regime classification (Evaporating / UVLocked / BoundedAdmissible /
  ShellDominated / Percolating), Φ-balance sign, and tracker stationarity. The
  module is external to `tick()` so the golden hash is unchanged. See
  `docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md`.

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

7. **Neighborhood energy evaporation**: 7-site energy (particle + 6 face-neighbors) smooths the leapfrog oscillation; the rule is stochastic (since 2026-04-23) — survival is Boltzmann-weighted, p_evap = exp(-E_local/K_MANIFEST^2) * K_EVAP_RATE per tick.

8. **Gauss exclusion at particle sites**: Gauss projection skips manifested sites -- physically correct since div(J)(i) doesn't involve J(i).

9. **Poisson-based Coulomb**: SOR warm-started solver gives 1/r^2 force (exponent -2.25, isotropy 1.0). Replaces legacy double-gradient (exponent -3.8, isotropy 0.40).

10. **Sequential movement with moved_ guard**: Prevents double-processing after index-order moves.

11. **Lorentz magnetic force**: F = alpha*s*(v x B) does zero work (v*F = 0). Toggle-gated.

12. **E/B field decomposition**: E = -wave_vel, B = curl(J). Poynting vector S = E x B for energy flow diagnostics.

13. **Backward compatibility**: Removed phase functions exist as no-op stubs. Removed toggles exist as deprecated fields. Removed Lagrangian terms return 0.

14. **Double damping is intentional (Rayleigh dissipation)**: Both `flux` and `wave_vel` are damped by `(1-ALPHA)` each tick in `phase_write`. This is deliberate Rayleigh dissipation -- it damps both the position-like degree of freedom (flux) and the velocity-like degree of freedom (wave_vel). Damping only one would leave undamped oscillatory modes. The dual damping ensures monotonic energy decay in the field, which is required for stable self-field buildup and physically correct radiation loss.

15. **Speed limit enforced by γ_FTD momentum integration in phase_forces()** (TRACKER §1.2): the velocity update in `phase_forces` uses `p = γmv` dynamics. Momentum reconstructs from `v + latency`, Newton's law updates `p`, and the new `v` extracts from `p` via `v = p · C · √((1−L²)/(C²+|p|²))`. This respects the FTD bandwidth `v²/C² + L² < 1` by construction — `|v|` asymptotes to `C·√(1−L²)`, never crosses. No clamp needed anywhere downstream; `phase_movement` receives an already-bounded velocity. A non-relativistic clamp would discard energy and be Lorentz-violating; the γ-integration avoids that.

---

## 13. RenderBridge Public API

### Core

| Method | Description |
|--------|-------------|
| `tick()` | Advance one tick through the current toggle-gated phase ladder |
| `diagnostics()` | Returns `Diagnostics` struct (counts, flux totals, charge) |
| `energy_audit()` | Returns `EnergyAudit` (field/wave/KE/PE breakdown, Gauss violation) — one-shot snapshot |
| `energy_ledger()` | Returns `const EnergyLedger&` — per-tick conservation drift (auto-populated on CPU path). Tests assert `abs(.residual) < tol` to refuse energy-drift regressions. GPU: call `update_energy_ledger()` manually after a device→host sync. |
| `update_energy_ledger()` | Populate the ledger (called automatically by `tick()` on CPU path) |
| `inject_particle(x,y,z, state)` | Inject single particle at lattice site |
| `inject_wavepacket(x,y,z, state, sigma, amplitude)` | Inject Gaussian wavepacket |
| `inject_flux(x,y,z, fx,fy,fz)` | Raw flux injection (overwrites site) |
| `inject_flux_add(x,y,z, flux_val)` | Additive flux injection — accumulates instead of overwriting. Required by ported JS scenarios that sum overlapping Gaussians. |
| `inject_wave_vel_add(x,y,z, wv_val)` | Additive wave-velocity injection — same additive semantics, for wave-equation initial conditions. |
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
| `toggles` | Public `TermToggles` struct (33 boolean toggles + typed config fields) |

### Scenario library

`ftd::dispatch_scenario(RenderBridge& rb, const std::string& name)`
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
                                      2b. pair production [optional]
                                      3. gauss / coulomb / latency solves
                                      4. forces + optional particle sectors
                                      5. movement
                                      6. weak/triad/proper-time extensions
```

### FFT Poisson Solver

Replaces CPU's iterative SOR with spectral method via cuFFT:
- **Exact**: Gauss violation = 0.0 (vs CPU SOR ~ 1.14)
- **Single-pass**: No iteration count to tune
- Precomputed Green's function reused every tick

**Numerical parity note:** CPU and GPU
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

### Dashboard Layout

```
+----------------------------------------------------------------+
|  FTD Engine v2.14     [Engine ▼]                     []       |  Toolbar
+----------------------------------------------------------------+
|                                    [Visualization ▾]           |  Overlay (collapsible)
|                                     VOLUME  FIELDS  FORCES     |
|                                     QUANTUM PHENOMENA          |
|                                                                 |
|              Three.js 3D Viewport                               |  ~60%
|         (particles, wireframe, field overlays)                  |
|                                                                 |
|   ┌──────────────────── Scrub Bar ────────────────────┐         |
|   │ [] [▷] [⏵] [↺] │ Speed─●─ │ ⟲ [──timeline──] t  │         |
|   │  global  local            │       Render      │         |
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

### Panels

The three Scale 0 dashboard tabs are built on a shared chart/table primitive set:

- **Charts primitives** (`js/ui/charts/`): vendored uPlot 1.6.30, a theme reader that maps CSS custom properties into uPlot config, and three primitive classes:
  - `UPlotChart` — line/area using bulk `flattenInto()` extraction from SoA MultiRingBuffers for O(1) contiguous typed-array render passes. DPR + ResizeObserver handling, localStorage-persisted series-hidden state.
  - `Sparkline` — axis-free micro chart for table Trend cells.
  - `StackedAreaChart` — custom `paths` renderer that cumulatively sums same-x points across series.
- **Diagnostics panel** (`js/ui/panels/diagnostics-panel/`): descriptor-driven `<table>` sections with `Metric | Value | Unit | Trend` columns, tabular-nums typography, zebra striping, digit-change pulse animation, and inline sparklines per row. The single Scale 0 descriptor declares 5 sections × 27 rows with physics-accurate units (`ct`, `E*`, `|J|`, `nat`, `|S|`, `ℏ`, `E*²`, `|w|²`).
- **Charts panel** (`js/ui/panels/charts-panel/`): horizontally-scrollable chip picker + auto-fit card grid. Chip toggles fully destroy / recreate chart cards — no leaked uPlot instances. Active-chart set persists in localStorage (`ftd.charts.active`).
- **Lagrangian panel** (`js/ui/panels/lagrangian-panel/`): StackedAreaChart with 7 bands · term-row checkboxes that two-way sync with the uPlot legend · `Action & Constraints` + `Ontic Constants` sidecar tables reusing `DiagnosticsTable`.

All three panels read live data from `TelemetryHub` (`js/telemetry-hub.js`), which utilizes `MultiRingBuffer` (Structure-of-Arrays) allocations across all 5 scales. Core buffers (`hub._s0_core`, `hub._s0_aud`, `hub._s1_pe`, etc.) are populated via unified `.push()` objects. The `WasmBridge` bypasses Embind object allocations by extracting native `Float64Array` zero-copy views (`getDiagnosticsView`, `getEnergyAuditView`, `getLagrangianView`) directly from the WASM engine.

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

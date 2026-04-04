# FTD Simulation Engine Reference

**Living document for AI agents and developers.**
**Last updated:** 2026-03-05
**Engine version:** 2.10 (Logic-First + Perfected EM + Multi-Scale Physics + CUDA GPU)
**Test count:** 157 test files, 156 CTests (152 CPU + 4 GPU-conditional). 10-Phase Proof-Out: 125+ checks, all PASS. GPU conditional on `FTD_ENABLE_CUDA`.

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
- Larmor radiation: acceleration-dependent damping (v2.8)
- Dual substrate: J_L + J_R chirality physics
- Color forces, strong force, weak transmutation, triad binding, pair production, exchange force

### Performance

Forces are O(N) field-mediated (single loop over manifested particles) instead of O(N^2) pairwise. Inherently faster for large particle counts.

---

## 2. Directory Layout

```
engine/
  CMakeLists.txt              # Build system -- all targets and test registration
  SPEC_ENGINE.md              # This document
  print_ontic.py              # Utility to print ontic chain values
  include/ftd/
    ontic.h                   # Ontic derivation chain (9+ layers), D=3 + varpi -> all constants (1221L)
    constants.h               # Re-exports ontic + engine-specific constants (276L)
    voxel.h                   # Vec3, ForceDiag, Voxel struct (203L)
    lattice.h                 # Lattice class -- 3D cubic grid with periodic boundaries (59L)
    render_bridge.h           # RenderBridge -- main engine API, tick(), diagnostics() (239L)
    lagrangian.h              # 4-term Lagrangian + Rayleigh dissipation (218L)
    term_toggles.h            # 20 runtime toggles for pedagogy system (62L)
    csv_export.h              # Header-only CSV export (flux field, density slice, timeseries) (385L)
    particle_engine.h         # ParticleEngine -- Scale 1 continuous-position particles (190L)
    atom_engine.h             # AtomEngine -- Scale 2 composite atoms + bonds (306L)
    scale.h                   # OnticEntity + scale bridge declarations (68L)
    correlations.h            # Correlation function analysis (205L)
    ensemble.h                # Statistical ensemble infrastructure (200L)
    spectral.h                # Spectral analysis utilities (195L)
    tracker.h                 # Particle trajectory tracking (173L)
    hilbert.h                 # Hilbert space utilities (209L)
    gpu_engine.h              # GpuEngine -- CUDA GPU drop-in for RenderBridge (115L)
    gpu_buffers.h             # SoA device memory layout (124L)
  src/
    render_bridge.cpp         # Logic-first engine -- 6-phase tick cycle (1420L)
    lattice.cpp               # Lattice implementation (index, coord, wrap, neighbors) (66L)
    lagrangian.cpp            # compute_lagrangian_diagnostics() -- 4 active terms (166L)
    main.cpp                  # CLI entry point (scenarios A-K) (937L)
    particle_engine.cpp       # ParticleEngine: Velocity Verlet + analytical forces (379L)
    atom_engine.cpp           # AtomEngine: ionic + vdW + covalent forces (691L)
    scale_bridge.cpp          # Scale 0<->1<->2 coarsen/refine round-trip (202L)
  cuda/
    gpu_buffers.cu            # SoA device allocation, upload, download (445L)
    gpu_engine.cu             # GpuEngine tick loop, host<->device sync (496L)
    kernels_stencil.cu        # GPU phase_read + phase_write + near_particle + dual-substrate (1172L)
    kernels_poisson.cu        # FFT Poisson solver (cuFFT spectral) (328L)
    kernels_forces.cu         # GPU forces + movement + color/strong/weak/exchange kernels (737L)
    CMakeLists.txt            # CUDA build rules (35L)
  tests/
    157 test files            # All registered as CTests (152 CPU + 4 GPU-conditional)
  wasm/
    ftd_wasm.cpp              # Emscripten Embind bindings -- full engine API (1492L)
    CMakeLists.txt            # WASM build rules (Emscripten-only)
  web/
    index.html                # Browser dashboard (HTML + CSS + tab system)
    js/                       # 27 JS modules (~19K lines total)
      app.js                  # Main controller: WASM loading, frame loop, 3 scale modes
      viewport.js             # Three.js 3D: particles, bonds, orbitals, shells, lobes, force arrows
      wasm-bridge.js          # WasmBridge + MockBridge (auto-fallback, identical API)
      fieldlines.js           # RK4 streamline computation, spatial indexing
      charts.js               # Ring-buffered time-series charts
      diagnostics.js          # Live number displays with sparkline mini-charts
      lagrangian.js           # Stacked area chart (5 terms) + constraint display
      inspector.js            # Click-to-inspect voxel properties + force decomposition
      constants.js            # JS mirror of ontic.h derivation chain
      particle-catalog.js     # Complete SM particle data with FTD mass formulas
      zoo.js                  # Interactive particle zoo table
      fields.js               # Force field visualization (heatmap + arrows)
      elements.js             # Periodic table data (118 elements with CPK colors)
      atomic-energy.js        # Bethe-Weizsacker nuclear binding energies
      spectroscopy.js         # Hydrogen energy levels and spectral series
      cross-sections.js       # Scattering cross-sections from ontic chain
      decay-rates.js          # Particle lifetimes from Fermi theory + FTD constants
      ontic-observatory.js    # Ontic incompleteness theorems
      aggregation-bridge.js   # 4-level aggregation hierarchy + emergence monitoring
      orbitals.js             # Electron orbital cloud + nuclear structure + bonding clouds
      molecules.js            # 25-molecule library for Scale 2
      backgrounds.js          # Environment backgrounds: star field, nebula, quantum foam, etc.
    wasm/
      ftd_core.js             # Emscripten JS loader (generated)
      ftd_core.wasm           # WebAssembly binary (generated)
  thirdparty/glad/            # OpenGL loader (legacy)
  build/                      # CPU build directory
  build_wasm/                 # WASM build directory
  build_cuda/                 # CUDA build directory (when FTD_ENABLE_CUDA=ON)
```

### Source line totals

| Component | Lines |
|-----------|-------|
| Headers (`include/ftd/*.h`) | 4,360 |
| Sources (`src/*.cpp`) | 3,861 |
| CUDA (`cuda/*.cu + CMakeLists`) | 3,218 |
| WASM bindings | 1,492 |
| Web frontend (HTML + JS) | ~18,000 |
| **Total engine C++** | **~12,900** |

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
| `phase_movement` | `movement` | Clears `moved_` flag buffer. Remainder accumulation, integer moves when remainder >= 1. Collisions: void->move, same-sign->bounce, opposite-sign->annihilate. Speed clamped to C_SPEED = 1/sqrt(3). Self-field and particle_id carried to new site |

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
| `ALPHA` | 0.00729 | Coulomb force, damping, exchange force |
| `K_B` | 0.511 | Evaporation threshold, wavepacket amplitude, Larmor scale |
| `G_C` | alpha^2 | State-flux coupling (phase_read) |
| `G_N` | 0.01 | Gravitational force |
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

**Reference-only (computed in ontic.h, not read by engine)**:

| Constant | Purpose |
|----------|---------|
| `MU_RATIO`, `TAU_RATIO`, etc. | Mass ratios (used by ParticleEngine/AtomEngine, not lattice) |
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

### Extension toggles (default OFF)

| Toggle | Description |
|--------|-------------|
| `selective_damping` | Only damp sites near particles; vacuum waves propagate without loss |
| `larmor_radiation` | Acceleration-dependent damping (requires `selective_damping`) |
| `dual_substrate` | Split flux into J_L + J_R substrates with chirality |
| `color_forces` | SU(3)-inspired color-dependent pairwise force |
| `weak_transmutation` | Stress-threshold polarity flip (+1 <-> -1) |
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
| **Total** | **155** | **1000+** |

All 155 tests are registered as CTests (151 CPU + 4 GPU-conditional). GPU tests (4 files) are conditional on `FTD_ENABLE_CUDA`.

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
- `gpu_parity` -- 21 checks: SoA round-trip, vacuum wave parity, energy parity
- `gpu_benchmark` -- Performance timing (363x at 64^3)
- `gpu_physics` -- 26 campaigns, 100+ checks: GP-COULOMB, GP-GAUSS, GP-WAVE-SPEED, GP-ENERGY-LONG, GP-GRAVITY, GP-ANNIHILATION, GP-MAXWELL-AMPERE, GP-EM-ENERGY, GP-CONTINUITY, GP-KCOMP-SHELL, GP-WEAK, GP-COLOR, GP-STRONG, GP-TRIAD, GP-PAIRS, GP-EXCHANGE, GP-BOUNCE, GP-DUAL-SUBSTRATE
- `gpu_experiments` -- Extended GPU experiments (timeout: 1800s)

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

15. **Speed limit enforced in phase_forces(), not phase_movement()**: The velocity clamp `|v| <= C_SPEED` is applied at the end of force accumulation rather than after movement. This prevents transient superluminal velocities from existing between force accumulation and movement -- even briefly. Post-movement clamping would allow one tick of superluminal propagation before correction, which could violate causality guarantees.

---

## 13. RenderBridge Public API

### Core

| Method | Description |
|--------|-------------|
| `tick()` | Advance one tick (all 5 phases) |
| `diagnostics()` | Returns `Diagnostics` struct (counts, flux totals, charge) |
| `energy_audit()` | Returns `EnergyAudit` (field/wave/KE/PE breakdown, Gauss violation) |
| `inject_particle(x,y,z, state)` | Inject single particle at lattice site |
| `inject_wavepacket(x,y,z, state, sigma, amplitude)` | Inject Gaussian wavepacket |
| `inject_flux(x,y,z, fx,fy,fz)` | Raw flux injection |
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

### SoA Memory Layout

~200 bytes/voxel (26+ separate device arrays for coalesced access). At 128^3: ~400 MB.

### Build

```bash
cmake -S engine -B engine/build_cuda -DFTD_ENABLE_CUDA=ON -G Ninja \
      -DCMAKE_CUDA_FLAGS="--allow-unsupported-compiler"
cmake --build engine/build_cuda --config Release
```

Requirements: CUDA 13.0+, compute capability >= 8.9. Target architectures: "89;120" (Ada + Blackwell).

### Benchmarks (RTX 5090)

| Lattice | CPU (ms/tick) | GPU (ms/tick) | Speedup |
|---------|---------------|---------------|---------|
| 16^3 | -- | -- | 18.6x |
| 32^3 | -- | -- | 41x |
| 48^3 | -- | -- | 193x |
| 64^3 | 134 | 0.37 | **363x** |

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
+------------------------------------------------------+
|  FTD Engine v2.8    [Scenario] [Size] [Speed] Play   |  Toolbar
+------------------------------------------------------+
|  [Field toggles: E B Energy div-J Flux Forces ...]   |  Overlay
|                                                       |
|              Three.js 3D Viewport                     |  ~60%
|         (particles, wireframe, field overlays)        |
|                                                       |
|  [Env: Star Field] [Boundary: Cube]                   |
+------+------+------+----------+----------------------+
| Ctrl | Diag | Chart| Lagrangian| Inspector            |  Tabs
+------+------+------+----------+----------------------+
|                Active Tab Panel                       |  ~35%
+------------------------------------------------------+
| Running | Tick: 1,234 | Particles: 12 | 60 fps       |  Status
+------------------------------------------------------+
```

### Scenarios (23+)

**Scale 0 (Lattice):** Flux Pulse, Dipole, Proton+Electron, Genesis Cascade, Damping Demo, 4-Source Interference, Flux Vortex, Particle Collision, Pair Production, Hydrogen Atom, Gravity Cluster, Random Genesis, Rainbow, Lattice Prism, Dipole Radiation, Two-Slit, Photon Race, Dual Substrate, Entangled Pair, Annihilation, Force Law Profile

**Scale 1 (ParticleEngine):** Leptons: Hydrogen, Helium, Positronium, Muonium, True Muonium, Tauonium, Tauonic Hydrogen. Exotic Atoms: Pionic H, Kaonic H, Σ⁺ Atom, Protonium. Hadrons: Pionium, Kaonium, Δ⁺⁺ System, Ω⁻ Scattering. Nuclear: Deuteron, Tritium, Helion. Bosons: W⁺W⁻ Pair. Scattering: p-e, Three-body, π⁺-p, μ⁻-p. Custom. (23 scenarios)

**Scale 2 (AtomEngine):** Individual elements (118), Periodic Table. Noble Gas Clusters: He/Ar/Mix. Ionic Formation: NaCl/MgF₂/Lattice. Covalent Formation: H₂/O₂/CH₄. H-Bonding: Water Dimer/Pentamer. VSEPR Geometry: CO₂/CH₄/H₂O. Thermal Dynamics: Gas/Collision. Metallic Clusters: Fe BCC/Cu FCC. Custom. Phase 3 forces (JS MockBridge): H-bonds, angle strain, dipole-dipole, thermostat, electronegativity. Scale 3 molecules: 25-molecule library + NaCl Crystal

### Field Visualization Overlays (9 toggles, keys 1-9)

E-field streamlines, B-field streamlines, Poynting vectors, Divergence heatmap, Flux streamlines, Force field arrows, Dual substrate volume, Chirality density, Light energy glow.

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

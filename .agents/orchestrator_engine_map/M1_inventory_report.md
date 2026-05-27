# M1 Codebase Inventory & Component Boundary Report

**Date:** 2026-05-26  
**Auditor:** Codebase Inventory Explorer  
**Subject:** Granular Structural Map of the `ftd/engine/` Simulation Suite  
**Target:** `c:\Users\cpaci\Desktop\ftd\.agents\orchestrator_engine_map\M1_inventory_report.md`  

---

## 1. Executive Summary

The **Foundational Ternary Dynamics (FTD)** C++ simulation engine (v2.18.0) is a logic-first computational ontology implementing discrete physics on a 3D cubic lattice. The engine establishes a coupled two-layer ontology:
1. **Discrete State Field ($s \in \{-1, 0, +1\}$)** representing manifested matter/antimatter or void.
2. **Continuous Flux Field ($J \in \mathbb{R}^3$)** representing dispositional energy-momentum density.

Rather than imposing standard phenomenological formulas directly, all physical constants (e.g., fine structure constant $\alpha \approx 1/137.036$, number of colors $N_c = 3$, electron mass amplitude $K_B = 0.511$) cascade from only two first-principles inputs—**dimension $D=3$** and the elliptic **lemniscate constant $\varpi$**—through the **Ontic Derivation Chain** (`ontic.h`).

The simulation operates across **7 discrete scales** of reality, with the C++ engine managing:
* **Scale 0 (Planck-scale voxel lattice)**: field-mediated wave propagation, Gauss SOR projection, and probabilistic genesis.
* **Scale 1 (Continuous-position particles)**: Velocity Verlet symplectic integration and analytical $1/r^2$ forces.
* **Scale 2 (Composite atomic nodes)**: ionic, van der Waals, and covalent bonding.
* **Scale 3 (Molecules)**: topological covalent structures.
* **Scale 5 (Cosmic N-body + SPH)**: Barnes-Hut octree gravity, comoving gas dynamics, stellar lifecycles, and Friedmann expansion.

---

## 2. Directory Layout & Boundaries

The codebase is organized into highly decoupled directories. Component boundaries are strictly demarcated between production-grade simulation logic, GPU kernels, static libraries, Emscripten binding interfaces, tests, and the Three.js web dashboard.

```
engine/
├── CMakeLists.txt                    # Top-level build config; declares all 211 ctest targets
├── SPEC_ENGINE.md                    # Authority on engine internals and tick phases
├── README.md                         # Quickstart guide, architecture table, and limitations
│
├── include/ftd/                      # 28 Public Headers (Declaration Layer)
│   ├── ontic/                        # First-principles constants sub-chain headers
│   └── eft/                          # Effective Field Theory (EFT) continuity headers
│
├── src/                              # C++ Source Files (Implementation Layer)
│   ├── render_bridge_phases/         # Decomposed Scale 0 tick-cycle translation units (TUs)
│   ├── constructors/                 # Scenario & entity constructor helpers
│   ├── scenarios/                    # C++ ports of JS seed scenarios
│   ├── atom/                         # Scale 2 forces, bonding, and thermostats
│   ├── cosmic/                       # Scale 5 Barnes-Hut, SPH, and cosmology TUs
│   └── eft/                          # EFT static library (ftd_eft) implementations
│
├── cuda/                             # GPU Parallel Logic (NVIDIA CUDA 13.0)
├── wasm/                             # Emscripten WebAssembly Binding Layer
├── tests/                            # 211 CTest targets & validation support
└── web/                              # Three.js Browser Dashboard & Pedagogical Engine
    ├── css/                          # Themeable modular UI stylesheets
    ├── js/                           # Three.js Viewport, sub-renderers, and controllers
    └── wasm/                         # Compiled ftd_core.js + ftd_core.wasm binaries
```

---

## 3. Core Structural Structures & Definitions

Across the entire multi-scale system, the following C++ `structs` and `classes` represent the mathematical and physical invariants of the FTD ontology:

### 3.1. `Vec3` (`voxel.h`)
The standard 3D double-precision coordinate/vector container.
* **Methods**: `mag2()` ($r^2$), `mag()` ($r$), `dot()`, and `cross()` (used for Biot-Savart magnetic fields and Lorentz forces).

### 3.2. `ForceDiag` (`voxel.h`)
Per-particle force breakdown allocated as a parallel buffer (`force_diag_`) in `RenderBridge` to preserve $O(N)$ field cache-locality.
* **Members**: `f_coulomb`, `f_strong`, `f_magnetic`, `f_gravity`, `f_exchange`.

### 3.3. `Voxel` (`voxel.h`)
The core state container for each node on the 3D lattice.
* **Fields**: `state` ($s \in \{-1, 0, +1\}$), `flux` ($J$), `wave_vel` ($v_{wave}$), `velocity` ($v_{voxel}$), `remainder`, `particle_id`, `pair_id` (entanglement tracking), `spin`, `color`, `locked`, and `accel_mag`.
* **Dual-Substrate Fields**: `flux_L`, `flux_R` (chiral split fields where $J = J_L + J_R$), `wave_vel_L`, `wave_vel_R`.
* **Methods**: `chirality_density()` ($|J_L|^2 - |J_R|^2$), `gamma_ftd()` (relativistic contraction modified by gravity), and `born_infeld_core()` (Born-Infeld Lagrangian density).

### 3.4. `Lattice` (`lattice.h`)
Periodic cubic lattice index and neighbor mapping. Computes coordinate wrapping on the fly to avoid 176 bytes/site of pointer storage.
* **Neighbor stencils**: `neighbors_6` (flux wave propagation), `neighbors_12` (Moore edge-sharing), `neighbors_8_corner` (BCC sub-stencil), and `neighbors_26` (full Moore neighborhood).

### 3.5. `ScaleEngine` (`scale_engine.h`)
Abstract base class defining the polymorphic runtime interface. Enables seamless switching between Scale 0, 1, 2, and 5 simulations on the web dashboard.
* **API**: `tick()`, `run(N)`, `current_tick()`, `dt()`, `get_toggle(name)`, `set_toggle()`, `entity_count()`, `base_diagnostics()`, `clear()`.

### 3.6. `OnticEntity` (`scale.h`)
The universal ternary triple characterizing every entity at every scale:
$$\{\text{State (identity)}, \text{Energy (mass/coupling)}, \text{Boundary (radius/orbital)}\}$$

### 3.7. `BarnesHutNode` & `BarnesHutTree` (`barnes_hut.h`)
Universal generic $O(N \log N)$ spatial partitioning octree. Integrates mass and charge monopoles. Used by Scales 1, 2, and 5 to accelerate long-range potentials without $O(N^2)$ scaling.

---

## 4. Granular File-by-File Inventory

### 4.1. Core Include Layer (`engine/include/ftd/`)

* **`ontic.h`**: The umbrella header that re-exports the entire 9-layer Ontic Derivation Chain. Consists of highly modular headers under `include/ftd/ontic/`:
  * `lemniscate.h`: Modular seeds, elliptic geometry ($\varpi$, Gauss $M$, $\pi$), universal operators ($G^*$), and the emergence of imaginary unit $i$ at $k_{crit} = 4/G^*$.
  * `master_quadratic.h`: The master quadratic equation $x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$ yielding $x_+ = 1/\alpha \approx 137.036$ and $x_- = N_c \approx 3.024$. Contains dual-substrate allocations.
  * `gauge_couplings.h`: Precision coupling calculations (CODATA 2022 matching via 4-term loop expansions), $G_N = 0.01$ (scaled lattice gravity), and QCD running definitions.
  * `particle_masses.h`: Standard Model mass scale definitions ($K_B = 0.511$ MeV, $K_{genesis} = 3 K_B$) and the Higgs VEV ($V_{Higgs} = 246.09$ GeV, $M_{Higgs} = 124.8$ GeV).
  * `neutrino.h`: Seesaw mechanism equations and PMNS neutrino mixing parameters.
  * `consciousness.h`: Pedagogical noetic coordinates ($y_{real}$, $K_{noetic}$, $\theta_C$) and softplus fixed points.
* **`constants.h`**: Re-exports all ontic constants into the `ftd::` namespace and appends engine tuning parameters (`SOR_ITERATIONS = 6`, `SOR_OMEGA = 1.75`, `EVAP_THRESHOLD = 1e-6`, `BANDWIDTH_FLOOR = 1e-6`). Contains the algebraic identity `ALPHA_EFT = G_C * G_C`.
* **`constants_gpu.cuh`**: CUDA-side `inline constexpr` mirror of `constants.h` compiling cleanly under both standard MSVC/g++ and NVCC.
* **`voxel.h` & `lattice.h`**: Voxel struct and coordinates/neighbor geometry helpers.
* **`render_bridge.h`**: The main Scale 0 execution engine managing the flat voxel grid, 6-phase CPU tick cycle, SOR iterations, self-field injection, and device-host handshakes.
* **`render_bridge_diagnostics.h`**: Holds POD diagnostic structures (`Diagnostics`, `AggregateProfile`, `EnergyAudit`, `EnergyLedger`, `EMFieldDiag`) to prevent compilation fan-out.
* **`term_toggles.h`**: A table-driven, unified registry mapping **27 runtime boolean toggles** (e.g., `wave_propagation`, `poisson_coulomb`, `selective_damping`, `dual_substrate`, `strong_force`) alongside CSV-based dependency and conflict validation logic.
* **`field_operators.h`**: Inline free-function stencils for spatial operators (Laplacians, divergences, curls, and gradients) executing on `std::vector<Voxel>`.
* **`lagrangian.h`**: Defines the 4-term lattice Lagrangian density and Rayleigh dissipation function parameters.
* **`scale_engine.h` & `scale.h`**: Polymorphic scale engine and coarsening/refinement definitions.
* **`particle_engine.h`**: Declares `Particle`, continuous Verlet integration fields, and continuous forces toggles (`ParticleToggles`).
* **`atom_engine.h`**: Declares `Atom`, atomic tables (valence, Pauling electronegativity, covalent valence capacities), and covalent `Bond` structs.
* **`cosmic_engine.h`**: Declares the 9 ternary-mapped `CosmicBodyType` objects, Friedmann scale factors, Bondi accretion rates, and SPH smoothing lengths.
* **`scenarios.h` & `constructors.h`**: C++ interfaces for scenario seeds and aggregate generation.
* **`barnes_hut.h`**: Template implementation of the spatial octree.
* **`correlations.h`, `ensemble.h`, `spectral.h`, `tracker.h`, `hilbert.h`**: Advanced diagnostic, trajectory tracking, Fourier spectral, and Hilbert space mapping libraries.
* **`wilson_dirac.h` & `sublattice.h`**: Standard lattice QCD Wilson loop and sublattice projection definitions.
* **`ws_protocol.h` & `ws_sha1.h`**: WebSocket handshaking headers.

---

### 4.2. Core C++ Implementations (`engine/src/`)

#### 4.2.1. Scale 0 Decomposed Tick Phases (`src/render_bridge_phases/`)
To eliminate structural bloat, the Scale 0 `tick()` pipeline is decomposed into four key translation units:
* **`phase_read.cpp`**: Implements the Wave Equation + state-flux coupling. Computes:
  $$\Delta J = c^2 \nabla^2 J + g_c \nabla s + g_c (\nabla \times (s v))$$
  Includes the dual-substrate split stencils.
* **`phase_write.cpp`**: Implements leapfrog advances ($v_{wave} \mathrel{+}= \Delta J; J \mathrel{+}= v_{wave}$), Langevin stochastic thermalization, Larmor radiation damping, and probabilistic genesis ($|J| > K_{genesis}$). Asserts mass-gap creation and evaporation ($E_{7\text{-site}} < K_B^2 \times 10^{-6}$).
* **`phase_forces.cpp`**: Field-mediated forces pipeline. Evaluates electrostatic $F_{EM} = -\alpha s \nabla \phi_C$ (via warm-started SOR Poisson solver), gravitational $F_{grav} = G_N \nabla \rho$ (using tier-2 stencils), and magnetic Lorentz forces $F_{magnetic} = \alpha s (v \times B)$.
* **`phase_movement.cpp`**: Updates remainder registers, translates particles across voxel bounds, checks periodic wrapping, and executes same-sign bouncing vs opposite-sign annihilation.

#### 4.2.2. Core Engines & Bridges
* **`render_bridge.cpp`**: Allocates the voxel lattice, triggers the 6-phase tick loop, updates `EnergyLedger`, manages WebSocket handshakes, and interfaces with execution backends.
* **`scale_bridge.cpp`**: Handles cross-scale coarsening and refinement:
  * Scale 0 $\leftrightarrow$ Scale 1: Voxel-field clustering to discrete particle states.
  * Scale 1 $\leftrightarrow$ Scale 2: Proximity grouping of electron-proton pairs to shell-screened atoms.
  * Scale 2 $\leftrightarrow$ Scale 5: Grouping heavy atomic clusters into SPH gaseous clouds or stellar cores.
* **`lagrangian.cpp`**: Evaluates the 4 active terms (wave kinetic, wave potential, interaction, mass gap) and Rayleigh dissipation.
* **`ontic_audit.cpp`**: Self-checks the mathematical precision of the ontic cascade and validates CODATA tolerances.
* **`csv_export.cpp`**: Handles data serialization (flux slices, state grids, timeseries logs).
* **`ws_server.cpp` & `ws_protocol.cpp`**: Native high-speed WebSocket broadcaster transmitting binary lattice frames to the web dashboard.
* **`main.cpp`**: Command Line Interface (CLI) entry point providing standalone scenario execution (Scenarios A through K).

#### 4.2.3. Multi-Scale & Macro Engines
* **`particle_engine.cpp`**: Symplectic Velocity Verlet integrator for Scale 1 particles. Handles O(N²) all-to-all forces or delegates to `BarnesHutTree`.
* **`atom_engine.cpp`**: Implements ionic, van der Waals (Lennard-Jones 12-6), and covalent harmonic bond forces.
  * *Bonding sub-module (`src/atom/`)*: `atom_bonding.cpp` (valence tracking and auto-bonding thresholds), `atom_forces.cpp` (VSEPR angular strains, improper torsional planarity, H-bonds), and `atom_thermostat.cpp` (Berendsen velocity rescaling).
* **`cosmic_engine.cpp`**: The Scale 5 macro-simulator. Runscomoving compton-cooling, Friedmann cosmological steps, and SPH gas hydrodynamics.
  * *Cosmic sub-module (`src/cosmic/`)*: `cosmic_barnes_hut.cpp` (comoving octrees), `cosmic_cosmology.cpp` (Friedmann equations), `cosmic_gravitational_waves.cpp` (merger strain emission), `cosmic_scenarios.cpp` (spiral galaxy / quasar builders), and `cosmic_sph.cpp` (Monaghan-Gingold artificial viscosity SPH kernels).

#### 4.2.4. Effective Field Theory (`src/eft/`)
Decoupled static library (`ftd_eft`) solving circular linking problems between standard CPU builds and CUDA binaries.
* **`dual_cell_continuity.cpp`**: Ensures oriented currents and reaction source-drains strictly balance across cells.
* **`blocking.cpp` & `dual_cell_blocking.cpp`**: Implements Kadanoff block-spin renormalization and dual continuity routines.
* **`dual_cell_flow.cpp`**: Renormalization group (RG) flow tracker.
* **`qcd_one_loop_perturbative.cpp`**: Implements standard one-loop running strong coupling $\alpha_s(Q)$ to model asymptotic freedom in particle tests.

---

### 4.3. NVIDIA CUDA Layer (`engine/cuda/`)

GPU acceleration is built as a complete parallel port of `RenderBridge`. All lattice arrays reside entirely in device VRAM; the CPU path handles only diagnostics retrieval.
* **`CMakeLists.txt`**: CUDA compilation rules under MSVC/Ninja.
* **`gpu_buffers.cu`**: Host-device allocation, copy, and SoA (Structure of Arrays) mapping wrappers.
* **`gpu_engine.cu`**: Implements `GpuEngine`. Direct drop-in execution for `RenderBridge`, orchestrating CUDA kernel pipelines and synchronizations.
* **`kernels_stencil_single.cu` & `kernels_stencil_dual.cu`**: GPU-side wave propagation. Launches highly parallel $18$-point stencil loops over the 3D grid.
* **`kernels_poisson.cu`**: Ultra-fast Poisson solver executing spectral 3D Fast Fourier Transforms (cuFFT) to resolve Gauss projections and Coulomb potentials on device.
* **`kernels_forces.cu`**: Launches threads per manifested particle to integrate forces (Poisson Coulomb, magnetic curl, tier-2 gravity) and update positions on device.
* **`kernels_aux.cu`**: Langevin noise generators and Larmor radiation reducers.
* **`kernels_gauge.cu` & `kernels_eft.cu`**: Parallel non-Abelian link updates and dual continuity auditors.
* **`atom_engine_gpu.cu` & `particle_engine_gpu.cu`**: Device-side pair-force kernels (coulomb + vdW) compiled to accelerate continuous space engines.
* **`wilson_dirac_gpu.cu`**: Highly parallel Wilson-Dirac spectral solver.

---

### 4.4. Emscripten WASM Bindings (`engine/wasm/`)

Exposes the C++ simulation backend directly to Three.js through the Emscripten binding library.
* **`ftd_wasm.cpp`**: Declares binding endpoints for `RenderBridge` (`tick`, `voxels`, `toggles`), particle generators, diagnostics payload structures, and scenario dispatchers.
* **`bindings_atom.cpp` & `bindings_particle.cpp`**: Embinds for continuous Scales 1 and 2, enabling continuous particles/atoms to be passed directly to the browser.
* **`bindings_render_bridge.cpp`**: Binds Scale 0 diagnostics, energy audits, and field mapping.

---

### 4.5. Test Layer (`engine/tests/`)

Declares **211 active CMake test targets** validating the entire framework.
* **`support/`**: Contains common testing infrastructures:
  * `bridge_fixtures.h` & `bridge_fixtures.cpp`: Declares standard test environments, particle configurations, and lattice allocations.
  * `test_telemetry.h` & `test_telemetry.cpp`: Handles assertion checks and telemetry validation logs.
* **Core Unit Tests**: `test_a1g_projector.cpp`, `test_annihilation.cpp`, `test_fine_structure_scale1.cpp`, `test_gravity_dynamics.cpp`, etc.
* **Golden Gate Test**: `test_render_bridge_golden.cpp`—performs a deterministic byte-hash verification over 100 ticks to guarantee bit-exact physics across refactor commits (golden hash: `0xcd957b601d47868a`).
* **GPU Parity Tests**: `test_gpu_parity.cpp`, `test_gpu_parity_complete.cpp`—verifies the bit-exact match of all float/double buffers between CPU and GPU runs.

---

### 4.6. Web Dashboard (`engine/web/`)

A high-performance Three.js front-end implementing visualizers for all 7 scales of the FTD hierarchy.

* **`index.html`**: Structured dashboard UI (panel layout, settings modals, scale tabs).
* **`serve.py`**: No-cache dev server ensuring modifications are loaded instantly.
* **`css/`**: Modular layout styling (`tokens.css`, `layout.css`, `scale-visibility.css`, and 5 themes including `abyss` and `midnight`).
* **`js/`**:
  * **`app.js`**: Core dashboard lifecycle coordinator initializing WASM, managing Three.js animation loops, and routing scale transitions.
  * **`constants.js`**: A pure JavaScript mirror of the `ontic.h` derivation chain.
  * **`core/`**: Centralized state manager (`state.js`), pub/sub message router (`event-bus.js`), and scale-agnostic simulation interface (`bridge.js`).
  * **`bridge/`**: Factory routing (`bridge-factory.js`) which binds the WASM compiled runtime (`wasm-bridge.js`) or falls back to a lightweight JS emulator (`mock-bridge.js`). Holds mock engines (`mock-scale5.js` for comoving cosmic simulation).
  * **`viewport/`**: Decoupled Three.js viewport modules:
    * `scene-core.js`: Camera rigging, lights, and grid helpers.
    * `flux-renderer.js`: Heatmaps, vector arrows, and field lines.
    * `particle-renderer.js`: Voxel highlights and particle spheres.
    * `field-renderer.js`: Electrostatic force visualizer.
  * **`scales/`**: Scale-specific controllers (`scale0/controller.js` to `scale6/controller.js`) which manage interaction states and load scenario manifests.
  * **`physics/`**: Binds the `physics-harness.js` layer exposing high-level queries (`sampleEFieldAlongRay`, `getParticleCharge`) symmetrically across WASM and Mock bridges.
  * **Pedagogical Utilities**: `orbitals.js` (hydrogen-like electron probability clouds), `spectroscopy.js` (spectral series lines), `elements.js` (periodic table CPK colors), `molecules.js` (chemical library), and `ontic-observatory.js`.

---

## 5. Component Boundaries & Data Flow

```
   [ Web Dashboard UI ] <======== ( event-bus.js ) ========> [ Three.js Viewport ]
           |                                                        ^
           v                                                        |
   [ bridge-factory.js ]                                  [ viewport/*.js ]
           |                                                        |
           +-----------------+-----------------+                    |
           | (WASM Active)   | (Mock Fallback) |                    |
           v                 v                 v                    |
    [ wasm-bridge.js ]  [ mock-bridge.js ]  [ physics-harness ] ----+
           |                 |
     (embind layer)     (pure JS sim)
           |
           v
  [ ftd_wasm.cpp / bindings ]
           |
           v
   [ ScaleEngine (Base) ]
           |
    +------+------+------+
    |             |      |
    v             v      v
[RenderBridge] [PE] [AtomEngine]  <========== ( scale_bridge.cpp )
  (Scale 0)  (Scale 1) (Scale 2)  (Coarsening/Refinement transitions)
    |             |      |
    v             +------+
 [CpuBackend]            |
    |                    v
    v             [GpuBackend / CUDA]
 [6-Phase Tick]   (d_voxels SoA in VRAM)
```

### 5.1. The Production vs Experimental Boundaries
* **Production Logic**: The core `RenderBridge` (Scale 0), `ParticleEngine` (Scale 1), `AtomEngine` (Scale 2), and `CosmicEngine` (Scale 5). All C++ tests, CTest benchmarks, and WASM dashboard features execute on these production classes.
* **Experimental Logic**: Classes prefixed with `Dag` (`DagEngine`, `DagLattice`, `test_dag_engine.cpp`). While the sparse-voxel DAG structures are functionally complete, the Gauss projection and force equations are currently `[OPEN]` stubs. **Do not use for physics calculations.**

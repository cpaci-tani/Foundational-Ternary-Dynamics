# Foundational Ternary Dynamics (FTD) C++ Engine: Complete Architectural Map & Gap Analysis

**Version:** 5.40 (sync to CLAUDE.md & post-2026-05-08 FTD/FQCR doctrine-ledger)
**Date:** 2026-05-26
**Subject:** Master FTD C++ Engine Architecture & Verification Report
**Target:** `docs/theory/01_reference/MAP_ENGINE_ARCHITECTURE.md`
**Classification:** Canonical Capstone Reference [REFERENCE / SYNTHESIS / ANALYSIS]
**Authoritative References:** `docs/SPEC_FTD.md`, `engine/SPEC_ENGINE.md`, `CLAUDE.md`, `AGENTS.md`

---

## Executive Summary

The **Foundational Ternary Dynamics (FTD)** C++ simulation engine (v2.18.0) is a logic-first computational ontology implementing discrete physics on a 3D cubic lattice. The engine simulates a coupled two-layer ontology:
1. **Discrete State Field ($s \in \{-1, 0, +1\}$)** representing manifested matter, antimatter, or void.
2. **Continuous Flux Field ($J \in \mathbb{R}^3$)** representing dispositional energy-momentum density.

Rather than imposing standard phenomenological formulas directly, all physical constants (e.g., fine structure constant $\alpha \approx 1/137.036$, number of colors $N_c = 3$, electron mass amplitude $K_B = 0.511$) cascade from only two first-principles inputs—**dimension $D=3$** and the elliptic **lemniscate constant $\varpi$**—through the **Ontic Derivation Chain** (`ontic.h`).

This map synthesizes:
* **Section 1 (Inventory & Map):** Decoupled directory boundaries, file-by-file inventory, structural invariants, and production vs. experimental boundaries.
* **Section 2 (Dependencies & Flows):** `#include` inclusion tree, compilation circularity prevention, the 11-step execution pipeline, multi-scale orchestration, AoS-to-SoA data mapping, lazy synchronization, and CUDA kernel targets.
* **Section 3 (Structural Documentation & Gaps):** Exhaustive table-driven documentation of all 29 runtime toggles, explicit mathematical-to-code mapping of the ontic chain, `DagEngine` sparse-voxel stubs, and recommendations for performance scaling.

---

## 1. Directory Layout & boundaries (R1 Map)

The codebase is organized into highly decoupled directories. Component boundaries are strictly demarcated between production-grade simulation logic, GPU kernels, static libraries, Emscripten binding interfaces, tests, and the Three.js web dashboard.

```
engine/
├── CMakeLists.txt                    # Top-level build config; declares all ctest targets
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
├── tests/                            # CTest targets & validation support
└── web/                              # Three.js Browser Dashboard & Pedagogical Engine
```

### 1.1 Granular File-by-File Inventory

#### 1.1.1 Core Include Layer (`engine/include/ftd/`)
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
* **`term_toggles.h`**: A table-driven, unified registry mapping **29 runtime boolean toggles** (e.g., `wave_propagation`, `poisson_coulomb`, `selective_damping`, `dual_substrate`, `strong_force`) alongside CSV-based dependency and conflict validation logic.
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

#### 1.1.2 Scale 0 Decomposed Tick Phases (`src/render_bridge_phases/`)
To eliminate structural bloat, the Scale 0 `tick()` pipeline is decomposed into four key translation units:
* **`phase_read.cpp`**: Implements the Wave Equation + state-flux coupling. Computes:
  $$\Delta J = c^2 \nabla^2 J + g_c \nabla s + g_c (\nabla \times (s v))$$
  Includes the dual-substrate split stencils.
* **`phase_write.cpp`**: Implements leapfrog advances ($v_{wave} \mathrel{+}= \Delta J; J \mathrel{+}= v_{wave}$), Langevin stochastic thermalization, Larmor radiation damping, and probabilistic genesis ($|J| > K_{genesis}$). Asserts mass-gap creation and evaporation ($E_{7\text{-site}} < K_B^2 \times 10^{-6}$).
* **`phase_forces.cpp`**: Field-mediated forces pipeline. Evaluates electrostatic $F_{EM} = -\alpha s \nabla \phi_C$ (via warm-started SOR Poisson solver), gravitational $F_{grav} = G_N \nabla \rho$ (using tier-2 stencils), and magnetic Lorentz forces $F_{magnetic} = \alpha s (v \times B)$.
* **`phase_movement.cpp`**: Updates remainder registers, translates particles across voxel bounds, checks periodic wrapping, and executes same-sign bouncing vs opposite-sign annihilation.

#### 1.1.3 Core Engines & Bridges
* **`render_bridge.cpp`**: Allocates the voxel lattice, triggers the 6-phase tick loop, updates `EnergyLedger`, manages WebSocket handshakes, and interfaces with execution backends.
* **`scale_bridge.cpp`**: Handles cross-scale coarsening and refinement.
* **`lagrangian.cpp`**: Evaluates the 4 active terms (wave kinetic, wave potential, interaction, mass gap) and Rayleigh dissipation.
* **`ontic_audit.cpp`**: Self-checks the mathematical precision of the ontic cascade and validates CODATA tolerances.
* **`csv_export.cpp`**: Handles data serialization (flux slices, state grids, timeseries logs).
* **`ws_server.cpp` & `ws_protocol.cpp`**: Native high-speed WebSocket broadcaster transmitting binary lattice frames to the web dashboard.
* **`main.cpp`**: Command Line Interface (CLI) entry point providing standalone scenario execution (Scenarios A through K).

#### 1.1.4 Multi-Scale & Macro Engines
* **`particle_engine.cpp`**: Symplectic Velocity Verlet integrator for Scale 1 particles. Handles $O(N^2)$ all-to-all forces or delegates to `BarnesHutTree`.
* **`atom_engine.cpp`**: Implements ionic, van der Waals (Lennard-Jones 12-6), and covalent harmonic bond forces.
  * *Bonding sub-module (`src/atom/`)*: `atom_bonding.cpp` (valence tracking and auto-bonding thresholds), `atom_forces.cpp` (VSEPR angular strains, improper torsional planarity, H-bonds), and `atom_thermostat.cpp` (Berendsen velocity rescaling).
* **`cosmic_engine.cpp`**: The Scale 5 macro-simulator. Runs comoving Compton cooling, Friedmann cosmological steps, and SPH gas hydrodynamics.
  * *Cosmic sub-module (`src/cosmic/`)*: `cosmic_barnes_hut.cpp` (comoving octrees), `cosmic_cosmology.cpp` (Friedmann equations), `cosmic_gravitational_waves.cpp` (merger strain emission), `cosmic_scenarios.cpp` (spiral galaxy / quasar builders), and `cosmic_sph.cpp` (Monaghan-Gingold artificial viscosity SPH kernels).

#### 1.1.5 Effective Field Theory (`src/eft/`)
Decoupled static library (`ftd_eft`) solving circular linking problems between standard CPU builds and CUDA binaries.
* **`dual_cell_continuity.cpp`**: Ensures oriented currents and reaction source-drains strictly balance across cells.
* **`blocking.cpp` & `dual_cell_blocking.cpp`**: Implements Kadanoff block-spin renormalization and dual continuity routines.
* **`dual_cell_flow.cpp`**: Renormalization group (RG) flow tracker.
* **`qcd_one_loop_perturbative.cpp`**: Implements standard one-loop running strong coupling $\alpha_s(Q)$ to model asymptotic freedom in particle tests.

#### 1.1.6 NVIDIA CUDA Layer (`engine/cuda/`)
GPU acceleration is built as a complete parallel port of `RenderBridge`. All lattice arrays reside entirely in device VRAM; the CPU path handles only diagnostics retrieval.
* **`CMakeLists.txt`**: CUDA compilation rules under MSVC/Ninja.
* **`gpu_buffers.cu`**: Host-device allocation, copy, and SoA (Structure of Arrays) mapping wrappers.
* **`gpu_engine.cu`**: Implements `GpuEngine`. Direct drop-in execution for `RenderBridge`, orchestrating CUDA kernel pipelines and synchronizations.
* **`kernels_stencil_single.cu` & `kernels_stencil_dual.cu`**: GPU-side wave propagation. Launches highly parallel 18-point stencil loops over the 3D grid.
* **`kernels_poisson.cu`**: Ultra-fast Poisson solver executing spectral 3D Fast Fourier Transforms (cuFFT) to resolve Gauss projections and Coulomb potentials on device.
* **`kernels_forces.cu`**: Launches threads per manifested particle to integrate forces (Poisson Coulomb, magnetic curl, tier-2 gravity) and update positions on device.
* **`kernels_aux.cu`**: Langevin noise generators and Larmor radiation reducers.
* **`kernels_gauge.cu` & `kernels_eft.cu`**: Parallel non-Abelian link updates and dual continuity auditors.
* **`atom_engine_gpu.cu` & `particle_engine_gpu.cu`**: Device-side pair-force kernels (coulomb + vdW) compiled to accelerate continuous space engines.
* **`wilson_dirac_gpu.cu`**: Highly parallel Wilson-Dirac spectral solver.

#### 1.1.7 Emscripten WASM Bindings (`engine/wasm/`)
Exposes the C++ simulation backend directly to Three.js through the Emscripten binding library.
* **`ftd_wasm.cpp`**: Declares binding endpoints for `RenderBridge` (`tick`, `voxels`, `toggles`), particle generators, diagnostics payload structures, and scenario dispatchers.
* **`bindings_atom.cpp` & `bindings_particle.cpp`**: Embinds for continuous Scales 1 and 2, enabling continuous particles/atoms to be passed directly to the browser.
* **`bindings_render_bridge.cpp`**: Binds Scale 0 diagnostics, energy audits, and field mapping.

#### 1.1.8 Test Layer (`engine/tests/`)
Declares active CMake test targets validating the entire framework.
* **`support/`**: Contains common testing infrastructures:
  * `bridge_fixtures.h` & `bridge_fixtures.cpp`: Declares standard test environments, particle configurations, and lattice allocations.
  * `test_telemetry.h` & `test_telemetry.cpp`: Handles assertion checks and telemetry validation logs.
* **Core Unit Tests**: `test_a1g_projector.cpp`, `test_annihilation.cpp`, `test_fine_structure_scale1.cpp`, `test_gravity_dynamics.cpp`, etc.
* **Golden Gate Test**: `test_render_bridge_golden.cpp`—performs a deterministic byte-hash verification over 100 ticks to guarantee bit-exact physics across refactor commits (golden hash: `0xcd957b601d47868a`).
* **GPU Parity Tests**: `test_gpu_parity.cpp`, `test_gpu_parity_complete.cpp`—verifies the bit-exact match of all float/double buffers between CPU and GPU runs.

### 1.2 Production vs. Experimental Boundary
* **Production Logic**: The core `RenderBridge` (Scale 0), `ParticleEngine` (Scale 1), `AtomEngine` (Scale 2), and `CosmicEngine` (Scale 5). All C++ tests, CTest benchmarks, and WASM dashboard features execute on these production classes.
* **Experimental Logic**: Classes prefixed with `Dag` (`DagEngine`, `DagLattice`, `test_dag_engine.cpp`). While the sparse-voxel DAG structures are functionally complete, the Gauss projection and force equations are currently `[OPEN]` stubs. **Do not use`DagEngine` for physics calculations.**

---

## 2. Core Structural Structures & Definitions

Across the entire multi-scale system, the following C++ `structs` and `classes` represent the mathematical and physical invariants of the FTD ontology:

### 2.1 `Vec3` (`voxel.h`)
The standard 3D double-precision coordinate/vector container.
* **Methods**: `mag2()` ($r^2$), `mag()` ($r$), `dot()`, and `cross()` (used for Biot-Savart magnetic fields and Lorentz forces).

### 2.2 `ForceDiag` (`voxel.h`)
Per-particle force breakdown allocated as a parallel buffer (`force_diag_`) in `RenderBridge` to preserve $O(N)$ field cache-locality.
* **Members**: `f_coulomb`, `f_strong`, `f_magnetic`, `f_gravity`, `f_exchange`.

### 2.3 `Voxel` (`voxel.h`)
The core state container for each node on the 3D lattice.
* **Fields**: `state` ($s \in \{-1, 0, +1\}$), `flux` ($J$), `wave_vel` ($v_{wave}$), `velocity` ($v_{voxel}$), `remainder`, `particle_id`, `pair_id` (entanglement tracking), `spin`, `color`, `locked`, and `accel_mag`.
* **Dual-Substrate Fields**: `flux_L`, `flux_R` (chiral split fields where $J = J_L + J_R$), `wave_vel_L`, `wave_vel_R`.
* **Methods**: `chirality_density()` ($|J_L|^2 - |J_R|^2$), `gamma_ftd()` (relativistic contraction modified by gravity), and `born_infeld_core()` (Born-Infeld Lagrangian density).

### 2.4 `Lattice` (`lattice.h`)
Periodic cubic lattice index and neighbor mapping. Computes coordinate wrapping on the fly to avoid 176 bytes/site of pointer storage.
* **Neighbor stencils**: `neighbors_6` (flux wave propagation), `neighbors_12` (Moore edge-sharing), `neighbors_8_corner` (BCC sub-stencil), and `neighbors_26` (full Moore neighborhood).

### 2.5 `ScaleEngine` (`scale_engine.h`)
Abstract base class defining the polymorphic runtime interface. Enables seamless switching between Scale 0, 1, 2, and 5 simulations on the web dashboard.
* **API**: `tick()`, `run(N)`, `current_tick()`, `dt()`, `get_toggle(name)`, `set_toggle()`, `entity_count()`, `base_diagnostics()`, `clear()`.

### 2.6 `OnticEntity` (`scale.h`)
The universal ternary triple characterizing every entity at every scale:
$$\{\text{State (identity)}, \text{Energy (mass/coupling)}, \text{Boundary (radius/orbital)}\}$$

### 2.7 `BarnesHutNode` & `BarnesHutTree` (`barnes_hut.h`)
Universal generic $O(N \log N)$ spatial partitioning octree. Integrates mass and charge monopoles. Used by Scales 1, 2, and 5 to accelerate long-range potentials without $O(N^2)$ scaling.

---

## 3. Compile-Time Dependency Hierarchy & Circular Prevention (R2 Flow)

The FTD C++ engine is engineered around a strict compile-time dependency hierarchy to isolate the foundational mathematical derivations (the **Ontic Chain**) from the runtime implementation details. This ensures maximum optimization, readability, and compilation speed.

```
                  [D=3, varpi Foundations]
                             │
                  [ontic/lemniscate.h] (Layers -1 to 2b)
                             │
               [ontic/master_quadratic.h] (Layers 3 to 4b)
                             │
            ┌────────────────┴────────────────┐
            ▼                                 ▼
   [ontic/gauge_couplings.h]         [ontic/particle_masses.h] (Layers 6 to 6c)
   (Layers 5 to 7)                            │
            │                                 ▼
            │                        [ontic/neutrino.h] (Layer 7b)
            └────────────────┬────────────────┘
                             ▼
                 [ontic/consciousness.h] (Layers 8 to 8b)
                             │
                             ▼
                        [ontic.h] (Umbrella Header)
                             │
               ┌─────────────┴─────────────┐
               ▼                           ▼
         [constants.h]            [constants_gpu.cuh] (CUDA / SoA Friendly)
               │                           │
       ┌───────┴───────┐                   │
       ▼               ▼                   ▼
   [voxel.h]      [lattice.h]     [GpuEngine / Kernels]
               │
               ▼
       [render_bridge.h] ◄─── [render_bridge_diagnostics.h] (Diagnostics split)
               │
               ▼
     [Backend Abstraction] (CpuBackend / GpuBackend)
```

### 3.1 The 9-Layer Ontic Derivation Chain
The mathematical and physical constants of FTD are derived from a 9-layer mathematical hierarchy beginning with spatial dimensions $D=3$ and the lemniscate constant $\varpi$. The headers in `engine/include/ftd/ontic/` reflect this progression:

1. **`ontic/lemniscate.h` (Layers -1 to 2b):** Defines the foundational geometric constants, centered around $\varpi \approx 2.62205755$ (derived as $\Gamma(1/4)/\Gamma(3/4)$). Establishes Layer 0 (lemniscatic constant $G^* \approx 2.95868$, the ratio of the lemniscate perimeter to diameter), Layer 1 (CFL speed limit $c = 1/\sqrt{3}$), and Layer 2b (gauge volume ratios).
2. **`ontic/master_quadratic.h` (Layers 3 to 4b):** Formulates the physics roots from the Master Quadratic equation:
   $$x^2 - 16(G^*)^2 x + 16(G^*)^3 = 0$$
   Solving this quadratic yields the roots $x_+$ and $x_-$.
   * $x_+$ leads to the fine structure constant: $\alpha = 1/x_+ \approx 1/137.036$
   * $x_-$ leads to the QCD color number $N_c = \lfloor x_- \rfloor \approx 3$.
3. **`ontic/gauge_couplings.h` (Layers 5 to 7):** Derives the running gauge couplings, including electroweak $\alpha_w \approx 0.0338$, strong coupling $\alpha_s = 1.0$, and the perturbative loop coefficients $c_1, c_2, c_3$.
4. **`ontic/particle_masses.h` (Layers 6 to 6c):** Computes the mass scales relative to the Planck mass $m_P$, defining the electron mass threshold $m_e = m_P \sqrt{2\pi} (16/3) \alpha^{11} \approx 0.511 \text{ MeV}$ (represented by manifestation threshold $K_B$) and the proton mass $m_p \approx 938.272 \text{ MeV}$.
5. **`ontic/neutrino.h` (Layer 7b):** Implements absolute neutrino mass definitions ($m_{\nu_1}, m_{\nu_2}, m_{\nu_3}$) using topological volume ratios.
6. **`ontic/consciousness.h` (Layers 8 to 8b):** Contains mathematical boundary values for the noetic domain, defining the genesis threshold $K_{genesis}$ and other informational scale bounds.
7. **`ontic.h`:** Serves as the global **Umbrella Header**, including all sub-headers under `ontic/` inside the `ftd::ontic` namespace.

### 3.2 Circular Include Prevention Schemes
Because `RenderBridge` interacts with numerous physical models, potentials, and numerical solvers, the engine employs strict strategies to avoid circular inclusions:
1. **Virtual Backend Abstraction (`backend.h`):** Introduces the abstract `Backend` base class. `RenderBridge` holds a `std::unique_ptr<Backend>` that can point to a `CpuBackend` or a `GpuBackend`. This eliminates compile-time dependencies on the GPU engine (`GpuEngine`) within the core bridge headers.
2. **Decomposition of Stencil Operators (`field_operators.h`):** The 8 core discrete field operators (Laplacian, Divergence, Curl, Gradients) were extracted from the `RenderBridge` class body into inline free helpers inside `field_operators.h`. Headers that perform diagnostic calculations only need to include `field_operators.h`, preventing the inclusion of `render_bridge.h`.
3. **PIMPL Pattern for RNG State (`bridge_rng.h`):** Random number generation for Born-rule stochastic genesis and Langevin thermostats is isolated via the PIMPL (Pointer to Implementation) pattern. The MT19937 engine state is encapsulated inside the `BridgeRng` helper, keeping `<random>` out of `render_bridge.h` to minimize compilation bloat and speed up build times.
4. **Diagnostics Extraction (`render_bridge_diagnostics.h`):** The 5 Plain-Old-Data (POD) diagnostic structures (`Diagnostics`, `AggregateProfile`, `EnergyAudit`, `EnergyLedger`, `EMFieldDiag`) were extracted from `render_bridge.h` into a standalone header. This dramatically reduces the compilation fan-out (Translation Unit rebuild count) from ~30 TUs to ~5 when making diagnostic changes.

---

## 4. Runtime Execution & The Phase Ladder

The execution pipelines of the FTD engine are divided into a **Scale 0 Cellular Tick Cycle** and a **Multi-Scale Orchestration Layer** coordinating physical behaviors across multiple length scales.

### 4.1 The Scale 0 Tick Cycle (11-Step Pipeline)
Every tick of the Scale 0 engine proceeds through a rigorous pipeline, executing physical postulates in chronological order. When a tick is triggered, `RenderBridge::tick()` executes the following sequence:

1. **Toggle Combination Validation:** Inspects the active `TermToggles` combination using `toggles.validate()`. If `strict_validation` is enabled, invalid combinations (e.g., enabling both Poisson Coulomb and Emergent Forces) throw an exception (or abort under WASM). If strict validation is off, warnings are deduplicated and logged once.
2. **Phase 1: Wave Propagation & Coupling (`phase_read()`):** Computes the vector field changes $\Delta J$.
   * Iterates through the lattice, applying an **18-point Moore neighborhood isotropic Laplacian stencil** to evaluate the wave equation $\frac{\partial^2 J}{\partial t^2} = c^2 \nabla^2 J$. The weights (face = 1/3, edge = 1/6, self = -4) maintain fourth-order $O(h^4)$ spatial isotropy.
   * If `toggles.coupling` is active, adds the state-flux coupling term $g_c \nabla s$ (manifested charges acting as sources) and the Biot-Savart term $g_c \nabla \times (s v)$ (moving charges inducing rotational flux).
3. **Phase 2: Commit Flux & Manifestation (`phase_write()`):** Commits the field updates via symplectic Störmer–Verlet leapfrog integration:
   $$\text{wave\_vel} \leftarrow \text{wave\_vel} + \Delta J$$
   $$\text{flux} \leftarrow \text{flux} + \text{wave\_vel}$$
   * Applies damping ($\text{flux} \leftarrow \text{flux} \cdot (1 - \gamma)$ where $\gamma = \alpha$). If Langevin dynamics are active, adds stochastic noise based on target temperature.
   * **Stochastic Genesis:** At void sites ($s=0$), if $|J| > K_{genesis}$, a particle manifests with probability $p = 1 - \exp\left(-\frac{|J| - K_{genesis}}{K_B}\right)$. The newly manifested voxel's state $s \in \{-1, +1\}$ is assigned from the sign of $\nabla \cdot J$, spin is assigned from $\nabla \times J$, and color ($0, 1, 2$) is assigned from the dominant flux axis.
   * **Evaporation:** Manifested voxels evaporate back to void ($s=0$) if $|J| < K_B$.
4. **Phase 2b: Pair Production (`pair_production_cpu()`):** Allows high-flux void sites ($s=0$) to spontaneously split into correlated $+1$ and $-1$ particle pairs, conserving charge locally.
5. **Phase 3: U(1) Gauss Constraint Projection (`gauss_project()`):** Enforces the gauge constraint $\nabla \cdot J = s$ at void sites. Runs a Successive Over-Relaxation (SOR) Poisson solver (or exact FFT spectral solver on GPU) to compute the gauge potential $\phi$ from $\nabla^2 \phi = \nabla \cdot J - s$, then projects the longitudinal modes out of the flux field: $J \leftarrow J - \nabla \phi$.
6. **Phase 3c: Latency Poisson (`solve_latency_poisson()`):** Solves the gravitational Poisson equation $\nabla^2 \phi_L = 4\pi G_N \rho_{mass}$ where $\rho_{mass} = K_B |s|$. The latency field is updated as $L = \sqrt{\text{clamp}(|\phi_L|, 0, 0.998)}$.
7. **Phase 4: Field-Mediated Forces (`phase_forces()`):** Computes field-mediated forces acting on particles.
   * EM Force: $F_{EM} = -\alpha \cdot s \cdot \nabla \phi_C$ (where $\phi_C$ is the Coulomb potential from $\nabla^2 \phi_C = s$, ensuring $1/r^2$ behavior).
   * Gravity Force: $F_{grav} = G_N \nabla \rho$ (using a tier-2 gradient stencil to avoid self-field contamination).
   * Lorentz Force: $F_{Lorentz} = \alpha \cdot s \cdot (v \times B)$ where $B = \nabla \times J$.
   * Integrates momentum using the **$\gamma_{FTD}$ relativistic formalism**:
     $$p \leftarrow \gamma \cdot v + F \cdot dt \implies v_{new} \leftarrow p \cdot c \sqrt{\frac{1 - L^2}{c^2 + |p|^2}}$$
8. **Phase 4b/c: Color & Strong Forces:** If QCD features are enabled, computes short-range exchange forces (spin-Pauli repulsion), Yukawa strong forces, SU(3) color forces (same-color repels, opposite-color attracts), and evaluates compact particle combinations for **Triad Binding** ($3$ same-sign particles locking into a single composite nucleon structure).
9. **Phase 5: Kinematics & Collisions (`phase_movement()`):** Moves particles through the grid using a fractional remainder accumulation: $\text{remainder} \leftarrow \text{remainder} + v \cdot dt$. When a coordinate remainder exceeds $1.0$, the particle executes an integer lattice jump:
   * **Void Target:** Moves successfully, transferring its self-field energy.
   * **Same-Sign Target:** Triggers an elastic bounce, reversing velocity along the collision axis.
   * **Opposite-Sign Target:** Triggers **Annihilation**. Both particles dissolve into void, and their remaining self-field energy is scattered equally to the 6 face neighbors as a high-energy electromagnetic flux burst.
10. **Phase 6: Weak Transmutation (`weak_transmutation_cpu()`):** Evaluates if local electroweak field stress exceeds the weak energy barrier, triggering stochastically-driven flavor/polarity flips.
11. **Phase 8: Proper Time Accumulation (`accumulate_proper_time()`):** Advances proper time based on time dilation: $d\tau = dt \frac{\sqrt{f^2 - v^2}}{\sqrt{f}}$ (where $f = 1 - L^2$), and enforces the bandwidth speed limit ($|v| < f \cdot c$).

---

### 4.2 Multi-Scale Orchestration & Transitions
The engine structures the physical layers of FTD into five distinct simulation scales. At runtime, the application bridges these scales dynamically through the polymorphic `ScaleEngine` interface:
* **Scale 0 (Voxel / `RenderBridge`):** The fundamental discrete lattice simulation. Field variables are stored per voxel, and all dynamics emerge from local Causality Postulates.
* **Scale 1 (Particle / `ParticleEngine`):** Continuous-space representation. Eliminates the spatial lattice, modeling manifested particles as continuous entities ($x, v, a \in \mathbb{R}^3$) evolving under analytical $1/r^2$ Coulomb and gravitational forces. Employs a **Barnes-Hut Octree** ($O(N \log N)$) or pairwise integration ($O(N^2)$) to compute accelerations.
* **Scale 2 (Atom / `AtomEngine`):** Models composite atoms. Properties like mass, Thomas-Fermi radius, covalent bonding capacity, and Pauling electronegativity are derived analytically from atomic number $Z$. Integrates ionic forces, Van der Waals interactions (Lennard-Jones 12-6), and harmonic covalent spring bonds.
* **Scale 5 (Cosmic / `CosmicEngine`):** Macro-scale simulation. Implements comoving cosmological expansion under Friedmann equations. Combines gravity (N-body Barnes-Hut) with **Smoothed Particle Hydrodynamics (SPH)** for cosmic gas clouds. Models dark matter, dark energy, stellar evolution, black hole Bondi accretion, and relativistic jets.

The transitions between scales are implemented in `scale_bridge.cpp`, utilizing two complementary paradigms: **Coarsening** (averaging microscopic states into microscopic parameters) and **Refinement** (decomposing macroscopic entities into structured microscopic components).

* **Scale 0 (Voxel) $\leftrightarrow$ Scale 1 (Particle)**
  * *Coarsening (`coarsen_to_particles`)*: Scans all voxels in `RenderBridge`. For every voxel where `state` $\neq 0$, it extracts the charge ($q = s$), mass ($m = \max(|J|, K_B)$), continuous position (integer voxel coordinate + sub-lattice `remainder`), velocity, spin, color, and entanglement `pair_id` to generate a continuous `Particle` struct.
  * *Refinement (`refine_to_voxels`)*: Takes a continuous `Particle` and maps its continuous coordinates back to integer lattice coordinates. It invokes `inject_wavepacket()`, distributing a Gaussian flux envelope of amplitude $K_B$ and width $\sigma = 3.0$ around the target center. It then restores the sub-lattice remainder, velocity, spin, and color onto the newly generated manifested voxel.
* **Scale 1 (Particle) $\leftrightarrow$ Scale 2 (Atom)**
  * *Coarsening (`coarsen_to_atoms`)*: Evaluates particles in `ParticleEngine`. It clusters locked, positive protons ($q=+1$, `locked=true`) within a spatial clustering radius ($R \approx 5.0$) to form a composite atomic nucleus with atomic number $Z = \text{count}$. It then searches for nearby electrons ($q=-1$) to neutralize the net ionic charge.
  * *Refinement (`refine_to_particles`)*: Decomposes an `Atom` into $Z$ locked protons at the atomic center, and orbits $Z - \text{charge}$ electrons stochastically distributed at the Thomas-Fermi Bohr radius.
* **Scale 2 (Atom) $\leftrightarrow$ Scale 5 (Cosmic)**
  * *Coarsening (`coarsen_to_cosmic`)*: Aggregates atomic clusters in `AtomEngine` into baryonic SPH gas particles. Total mass is computed as the sum of atomic masses, and the comoving position is assigned to the center of mass (centroid).
  * *Refinement (`refine_to_atoms`)*: Decomposes a comoving SPH gas body of mass $M$ into structured hydrogen atoms distributed uniformly inside the SPH smoothing radius.

---

## 5. Host-Device Data Boundaries & Parallel Acceleration

To accelerate massive grid calculations (e.g., $128^3$ grids containing $2,097,152$ voxels), FTD implements a native CUDA execution layer. The host-device data interface is designed to maximize GPU arithmetic throughput while minimizing PCI-Express bus saturation.

### 5.1 Memory Layout: AoS vs. SoA
* **Host Layout (AoS):** The CPU engine organizes voxels as an **Array of Structures** (`std::vector<Voxel>`). This is highly cache-friendly for sequential CPU traversals since all fields of a single site (state, flux, velocity) reside contiguously in memory.
* **Device Layout (SoA):** GPUs require memory coalescence to achieve peak memory bandwidth. In `GpuBuffers`, FTD decomposes the Voxel struct into a **Structure of Arrays** (SoA) layout. Separate, flat pointers (`d_state`, `d_flux_x`, `d_flux_y`, `d_flux_z`, `d_wave_vel_x`, etc.) are allocated in VRAM, allowing CUDA warps to read adjacent spatial fields in single, consolidated DRAM transactions.

### 5.2 The Lazy Synchronization State Machine
Data transfers over the PCI-Express bus are high-latency operations. The engine avoids redundant transfers through a lazy synchronization protocol managed by `GpuBackend` (which implements the virtual `Backend` interface):

1. **`mark_host_dirty()`:** When host code mutates voxels directly (e.g., during particle injection or scenario initialization), it flags `host_mutated_ = true`.
2. **`flush_host_mutations()` / `push_to_device()`:** Prior to executing the next GPU tick, `GpuBackend` intercepts the call, uploads the modified host AoS data into the device SoA buffers, and resets `host_mutated_ = false`.
3. **`mark_gpu_dirty()`:** After the GPU executes a tick on the device, it sets `gpu_dirty_ = true`. The device data is now authoritative, and the host shadow is stale.
4. **`sync_to_host()`:** When host-side code requests access to the voxel array (either via non-const or const `voxels()` accessors, or during diagnostics/energy audits), `GpuBackend` intercepts the call. If `gpu_dirty_` is true, it performs a bulk PCIe download converting device SoA data back to host AoS (`GpuBuffers::download`), updates potentials (`phi`, `phi_coulomb`, `phi_latency`), scatters force components into `force_diag_`, and resets `gpu_dirty_ = false`.

### 5.3 GPU Kernel Acceleration Targets
* **Discrete Stencils & Leapfrog Updates (`kernels_stencil_single.cu` / `kernels_stencil_dual.cu`):** Computes the 18-point Moore Laplacian in parallel. Langevin thermal noise is generated **deterministically on the device per thread** using a fast SplitMix64 generator initialized with `(seed, voxel_index, tick, salt)`.
* **Exact Spectral Poisson Solvers (`kernels_poisson.cu`):** The U(1) Gauss constraint, Coulomb potential, and Latency fields all require solving the Poisson equation $\nabla^2 \phi = \rho$. The GPU engine replaces the CPU's Successive Over-Relaxation (SOR) solver with an **Exact Spectral Solver** using fast Fourier transforms (FFT) via NVIDIA's **cuFFT** library:
  1. *Fourier Transform*: Transforms the charge/mass density into Fourier space: $\hat{\rho}(k) = \text{FFT}(\rho(x))$.
  2. *Spectral Green's Correction*: Computes the exact potential in k-space:
     $$\hat{\phi}(k) = \frac{\hat{\rho}(k)}{G(k)}$$
     where $G(k)$ is the precomputed discrete Laplacian eigenvalues for a periodic cubic lattice:
     $$G(k_x, k_y, k_z) = 2 \left[\cos\left(\frac{2\pi k_x}{L}\right) + \cos\left(\frac{2\pi k_y}{L}\right) + \cos\left(\frac{2\pi k_z}{L}\right) - 3\right]$$
     The DC mode ($G(0,0,0)$) is zeroed out.
  3. *Inverse Fourier Transform*: Transforms back to real space: $\phi(x) = \text{IFFT}(\hat{\phi}(k))$.
* **Forces and Relativistic Integration (`kernels_forces.cu`):** Maps a thread to every manifested voxel. It reads the precomputed potentials ($\phi_C, \phi_L$) and fields, evaluates the EM gradient force, gravitational density gradient force, and Lorentz force. It then performs the full $\gamma_{FTD}$ relativistic integration. Performs a parallel prefix sum (reduction) to build a compact array of active particle indices (`plist_idx`) to avoid $O(N^2)$ calculations.

---

## 6. Runtime Toggles Registry & Physical Interpretations (M3 Analysis)

The C++ engine's behavior is controlled by a unified, table-driven registry in `term_toggles.h`. This table documents all **29 active boolean toggles**, mapping their default configurations, dependencies, conflicts, and physical meanings:

| Toggle Name | Default | Dependencies | Conflicts | Backends | Detailed Physical Meaning / Simulation Role |
| :--- | :---: | :--- | :--- | :---: | :--- |
| **`wave_propagation`** | `true` | None | None | `ANY` | Phase-read: Updates vector field flux via 18-point Moore isotropic Laplacian stencil. Represents core spatial wave equation. |
| **`coupling`** | `true` | None | None | `ANY` | Phase-read: Adds state-flux coupling $g_c \nabla s$ and Biot-Savart term $g_c \nabla \times (s v)$ where charges act as field sources. |
| **`damping`** | `true` | None | None | `ANY` | Phase-write: Exponential flux magnitude decay at rate $\alpha$. Prevents numerical run-away of self-field energy. |
| **`genesis`** | `true` | None | None | `ANY` | Phase-write: Master toggle enabling probabilistic particle creation and evaporation when local flux $|J| > K_{genesis}$. |
| **`evaporation`** | `false` | None | None | `ANY` | Phase-write: Isolates particle evaporation logic ($|J| < K_B \rightarrow s=0$) for test isolation. OR'd with `genesis`. |
| **`gauss_projection`** | `true` | None | None | `ANY` | Enforces $\nabla \cdot J = s$ U(1) gauge constraint at void sites by projecting longitudinal modes out of the flux field. |
| **`forces`** | `true` | None | None | `ANY` | Phase-forces: Master switch activating EM and gravitational forces. |
| **`gravity`** | `true` | None | None | `ANY` | Phase-forces: Evaluates gravitational attraction $F = G_N \nabla \rho$ using tier-2 spatial smoothing stencils. |
| **`poisson_coulomb`** | `true` | None | `emergent_forces` | `ANY` | Solves Poisson equation $\nabla^2 \phi_C = s$ for Coulomb potential to guarantee analytical $1/r^2$ force behavior. |
| **`movement`** | `true` | None | None | `ANY` | Phase-movement: Verlet continuous integration and fractional sub-lattice remainder tracking. |
| **`lorentz_force`** | `true` | `forces` | None | `ANY` | Phase-forces: Rotational magnetic Lorentz force $F = \alpha s (v \times B)$ where $B = \nabla \times J$. |
| **`selective_damping`** | `true` | `damping` | None | `ANY` | Phase-write: Damps only near manifested particles, keeping vacuum propagation lossless (vacuum EM is lossless). |
| **`larmor_radiation`** | `false` | `damping` | `langevin` | `ANY` | Phase-write: Applies radiation reaction damping stochastically proportional to particle acceleration-squared. |
| **`dual_substrate`** | `true` | None | None | `ANY` | Implements chiral splitting $J = J_L + J_R$ to represent matter/antimatter asymmetric substrates. |
| **`color_forces`** | `false` | None | None | `ANY` | Phase-forces: Activates SU(3)-inspired color coupling between quarks based on color charges. |
| **`weak_transmutation`**| `true` | `dual_substrate`| None | `ANY` | Tick: Triggers flavor-changing weak transmutes stochastically when field stress exceeds the weak energy barrier. |
| **`strong_force`** | `false` | None | None | `ANY` | Phase-forces: Yukawa short-range nuclear force (no CPU implementation; GPU only). |
| **`triad_binding`** | `false` | `color_forces` | None | `ANY` | Tick: Detects color-singlet triads, locking them permanently into nucleons (`locked = true`). |
| **`pair_production`** | `false` | None | None | `ANY` | Genesis: Spontaneously splits high-flux void into correlated opposite-charge particle pairs conserving charge. |
| **`exchange_force`** | `false` | `poisson_coulomb`| None | `ANY` | Phase-forces: Pauli exclusion repulsion between same-spin fermions (no CPU implementation; GPU only). |
| **`latency_field`** | `false` | `gravity` | None | `ANY` | Solves Poisson for latency field $\nabla^2 L = 4\pi G_N \rho$, implementing relativistic time dilation. |
| **`exact_dual_gauss`** | `false` | None | None | `ANY` | Gauss-project: High-precision exact dual-cell face-flux projection. |
| **`emergent_forces`** | `false` | None | `poisson_coulomb` | `ANY` | EFT mode: Computes forces from direct flux gradients instead of solving long-range Poisson potentials. |
| **`langevin`** | `false` | None | `larmor_radiation` | `ANY` | Stochastic thermalization: OU process on wave velocity (CPU only at runtime). |
| **`symplectic_leapfrog`**| `false`| `wave_propagation`| None | `ANY` | Scale 0: Symplectic wave integration for high-stability wave propagation. |
| **`su2_gauge`** | `false` | None | None | `ANY` | Scale 0: Reconfigures stencils to incorporate SU(2) non-Abelian link variables. |
| **`su3_gauge`** | `false` | None | None | `ANY` | Scale 0: Reconfigures stencils to incorporate SU(3) non-Abelian link variables. |
| **`confinement`** | `false` | None | None | `ANY` | Intent flag: Aliased to JS scenarios representing linear confinement regime (no C++ branch). |
| **`strict_validation`** | `false` | None | None | `ANY` | When active, throws an exception on the first `TermToggles::validate()` failure. |

### 6.1 Non-Boolean Configuration Fields
These parameterized properties live outside the table-driven registry:
* **`bcc_stencil` (`BccStencilMode`):** Controls sub-lattice stencil modes. Options: `FULL`, `FACE_ONLY`, `CORNER_ONLY`. Validation requires `dual_substrate = false` if non-default.
* **`langevin_site_filter` (`SiteClass`):** Selects which lattice parity sectors the Langevin thermostat targets (e.g., `ALL_SITES`, `EVEN_SITES`, `ODD_SITES`).
* **`langevin_T` & `langevin_gamma`:** Langevin thermodynamic parameters.
* **`coulomb_charge_coupling`:** Explicit scalar modifying the strength of the Gauss projection source.

---

## 7. Theoretical Mathematical Mapping

The first-principles cascade implemented in `engine/include/ftd/ontic/` maps directly to FTD's theoretical postulates. The following table identifies how mathematical definitions translate to actual variables:

| Physical/Mathematical Concept | Theory Formulation (docs/SPEC_FTD.md) | C++ Variable/Symbol (ontic/*.h) |
| :--- | :--- | :--- |
| **Lemniscate Perimeter** | $\varpi = \Gamma(1/4)^2 / (2\sqrt{2\pi}) \approx 2.622$ | `VARPI` (`lemniscate.h`) |
| **Gauss Constant** | $M = 1/\text{AGM}(1, \sqrt{2}) \approx 0.8346$ | `GAUSS_CONSTANT_M` (`lemniscate.h`) |
| **Universal Operator** | $G^* = 2\sqrt{\varpi M} \approx 2.95868$ | `G_STAR` (`lemniscate.h`) |
| **Derived Circle Constant** | $\pi = 4\varpi^2 / G^{*2}$ | `PI` (`lemniscate.h`) |
| **BCC Lattice Self-Energy** | $I_1 = G^{*2}/(2\pi) \approx 1.393$ | `I_1_BCC` / `W_3` (`lemniscate.h`) |
| **Critical Coupling Parameter** | $k_{crit} = 4/G^* \approx 1.352$ | `K_CRIT` (`lemniscate.h`) |
| **Master Quadratic Root ($x_+$)**| $x_+ \approx 137.036$ (leads to $1/\alpha$) | `X_PLUS` (`master_quadratic.h`) |
| **Master Quadratic Root ($x_-$)**| $x_- \approx 3.024$ (leads to $N_c = 3$) | `X_MINUS` (`master_quadratic.h`) |
| **Precision-Corrected Root** | $x_+ - c_1 |\epsilon| + c_2 |\epsilon|^2 - \dots$ | `X_PLUS_PRECISION` (`master_quadratic.h`) |
| **Vieta Product Root ($x_-$)** | $x_- = 16 G^{*3} / x_+^{prec}$ | `X_MINUS_PRECISION` (`master_quadratic.h`) |
| **Chiral Substrate Energies** | $E_{L, R} = S(1 \pm \delta)/2$ | `E_LEFT_APPROX` / `E_RIGHT_APPROX` |
| **Electroweak Coupling** | $\alpha_w \approx 0.0338$ | `ALPHA_WEAK` (`gauge_couplings.h`) |
| **Fermion Exponents Ladder** | Perturbative (4), Higgs (8), Electron (11) | `LADDER_ELECTRON` etc. (`master_quadratic.h`) |
| **Neutrino Seesaw Mass** | topological volume ratios | `SIN2_THETA12` etc. (`neutrino.h`) |

---

## 8. Architectural Gaps, Stubs & Recommendations

A rigorous architectural review identifies the following gaps and open stubs in the C++ engine:

### 8.1 `DagEngine` & Sparse-Voxel DAG Structures
The classes `DagEngine`, `DagLattice`, and the associated unit test `test_dag_engine.cpp` represent an experimental sparse-voxel Directed Acyclic Graph (DAG) execution layer.
* **Current Status:** While the structures compile and are functionally complete for storing coordinates and hierarchical tree pointers, **the Gauss projection and phase forces equations are currently open stubs.**
* **Obstruction:** Solving the Poisson equation (for U(1) Gauss projection) on a sparse, non-uniform DAG requires replacing the fast Fourier transform (cufft) and Successive Over-Relaxation stencils with an algebraic multigrid (AMG) or sparse conjugate gradient (CG) solver. **Do not use `DagEngine` for active physical simulations.**

### 8.2 Performance Scaling Gaps
1. **Device-Side Reduction for the Energy Ledger:** Currently, the per-tick energy ledger computation on the GPU path requires downloading the entire voxel array (~3 MB at $L=64$) over the PCIe bus to run the sum on the CPU.
   * *Recommendation*: Implement a block-reduction CUDA kernel returning three scalars (`E_field`, `E_wave`, `E_kin`). This reduces the PCIe payload to $24$ bytes, eliminating the host-device synchronization bottleneck.
2. **Double-Buffering for Wave Propagation:** The current GPU wave update uses a split kernel execution (`gpu_phase_read` then `gpu_phase_write`) to prevent thread race conditions.
   * *Recommendation*: Implement a double-buffered flux pointer on the device to allow fusing these two kernels into a single launch, improving GPU instruction cache efficiency.
3. **Consolidation of Multi-Scale GPU Acceleration:** The continuous-space engines (`ParticleEngine` and `AtomEngine`) currently upload their coordinates to the device to run custom pair-force kernels, but maintain state on the host between ticks.
   * *Recommendation*: Expand the `GpuBackend` to keep continuous particles/atoms entirely on the GPU between ticks (mirroring `RenderBridge`'s lazy sync design), unlocking a massive performance boost for larger Scale 1 and 2 simulations.

---

*Report compiled by the Architectural Documentation and Verification Expert. Verified against FTD core v2.18.0.*

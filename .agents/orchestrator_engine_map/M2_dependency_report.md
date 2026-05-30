# Foundational Ternary Dynamics (FTD) C++ Engine: Dependency & Data Flow Analysis Report

**Date:** 2026-05-26  
**Analyst:** Dependency and Flow Analyst (Worker M2)  
**Scope:** `engine/` C++ codebase (v2.11)  
**Authoritative References:** `docs/SPEC_FTD.md`, `engine/SPEC_ENGINE.md`, `CLAUDE.md`, `AGENTS.md`

---

## Executive Summary
This report provides a granular dependency and data flow mapping of the **Foundational Ternary Dynamics (FTD)** C++ simulation engine. The FTD engine implements a discrete computational framework simulating physical systems via a cubic lattice where voxels occupy ternary states ($s \in \{-1, 0, +1\}$) and dispositional vector fluxes ($J \in \mathbb{R}^3$) evolve under local update rules. 

Through a deep static and architectural audit of the codebase, this analysis maps:
1. **Compile-Time Dependencies:** Header inclusion chains in `engine/include/ftd/`, focusing on the 9-layer ontic derivation chain, public boundaries, and circular dependency prevention schemes.
2. **Runtime Execution Pipelines:** The step-by-step tick cycle of `RenderBridge` and `GpuEngine`, coordination of multi-scale models (Scale 0 to Scale 5), and scale transitions (coarsening/refinement).
3. **Host-Device Data Boundaries:** The memory translation between Host Array-of-Structures (AoS) and Device Structure-of-Arrays (SoA), the lazy synchronization state machine, and specific GPU kernel acceleration targets.

---

## 1. Compile-Time Header Inclusions (#include Chains)

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
                 [ontic/reference frame context.h] (Layers 8 to 8b)
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
       │               │
       └───────┬───────┘
               ▼
      [render_bridge.h] ◄─── [render_bridge_diagnostics.h] (Diagnostics split)
               │
               ▼
     [Backend Abstraction] (CpuBackend / GpuBackend)
```

### 1.1 The Ontic Derivation Chain
The mathematical and physical constants of FTD are derived from a 9-layer mathematical hierarchy beginning with spatial dimensions $D=3$ and the lemniscate constant $\varpi$. The headers in `engine/include/ftd/ontic/` reflect this progression:

1. **`ontic/lemniscate.h` (Layers -1 to 2b):** Defines the foundational geometric constants, centered around $\varpi \approx 2.62205755$ (derived as $\Gamma(1/4)/\Gamma(3/4)$). Establishes Layer 0 (lemniscatic constant $G^* \approx 2.95868$, the ratio of the lemniscate perimeter to diameter), Layer 1 (CFL speed limit $c = 1/\sqrt{3}$), and Layer 2b (gauge volume ratios).
2. **`ontic/master_quadratic.h` (Layers 3 to 4b):** Formulates the physics roots from the Master Quadratic equation:
   $$x^2 - x \cdot (N_{base} \cdot \pi) + \varpi \cdot N_{base} \cdot D = 0$$
   where $N_{base} = 4$. Solving this quadratic yields the roots $x_+$ and $x_-$.
   * $x_+$ leads to the fine structure constant:
     $$\alpha = \frac{1}{x_+} \approx \frac{1}{137.036}$$
   * $x_-$ leads to the QCD color number $N_c = x_- \approx 3$.
3. **`ontic/gauge_couplings.h` (Layers 5 to 7):** Derives the running gauge couplings, including electroweak $\alpha_w \approx 0.0338$, strong coupling $\alpha_s = 1.0$, and the perturbative loop coefficients $c_1, c_2, c_3$.
4. **`ontic/particle_masses.h` (Layers 6 to 6c):** Computes the mass scales relative to the Planck mass $m_P$, defining the electron mass threshold $m_e = m_P \sqrt{2\pi} (16/3) \alpha^{11} \approx 0.511 \text{ MeV}$ (represented by manifestation threshold $K_B$) and the proton mass $m_p \approx 938.272 \text{ MeV}$.
5. **`ontic/neutrino.h` (Layer 7b):** Implements absolute neutrino mass definitions ($m_{\nu_1}, m_{\nu_2}, m_{\nu_3}$) using topological volume ratios.
6. **`ontic/reference frame context.h` (Layers 8 to 8b):** Contains mathematical boundary values for the noetic domain, defining the genesis threshold $K_{genesis}$ and other informational scale bounds.
7. **`ontic.h`:** Serves as the global **Umbrella Header**, including all sub-headers under `ontic/` inside the `ftd::ontic` namespace.

### 1.2 Translation to Runtime Constants (`constants.h` and `constants_gpu.cuh`)
* **`constants.h`:** Re-exports the mathematical constants from the `ftd::ontic` namespace into the public `ftd::` namespace. This file acts as the compile-time configuration layer for all host C++ modules.
* **`constants_gpu.cuh`:** A header-only, highly optimized CUDA counterpart of `constants.h`. To ensure compatibility with the NVIDIA CUDA Compiler (NVCC) and avoid host-specific standard template library (STL) headers, it translates the constants into `__device__ __constant__` and `__device__ __forceinline__` representations. It is strictly header-only to prevent link-time errors in CUDA compilation units.

### 1.3 Public Interface Headers
* **`voxel.h`:** Defines the fundamental data primitives of Scale 0:
   * `Vec3`: A custom 3D double-precision vector representing local fluxes, velocities, and coordinates.
   * `Voxel`: The primary cellular structure (~154 bytes in AoS layout) containing fields for ternary `state`, `flux`, dual-substrate fluxes (`flux_L`, `flux_R`), `wave_vel`, `velocity`, sub-lattice `remainder`, quantum numbers (`spin`, `color`, `flavor`), `particle_id`, `pair_id`, `locked` flags, and potentials (`phi`, `latency`, `tau`).
* **`lattice.h`:** Implements `Lattice`, managing the 3D Moore neighborhood grid (26-connected voxel coordinates). It provides flat-index calculation ($x \cdot L^2 + y \cdot L + z$) under periodic boundary conditions.
* **`render_bridge.h`:** The primary interface for Scale 0. It declares the `RenderBridge` class, maintaining the simulation grid state on the host and orchestrating CPU/GPU execution.

### 1.4 Circular Include Prevention Schemes
Because `RenderBridge` interacts with numerous physical models, potentials, and numerical solvers, the engine employs strict strategies to avoid circular inclusions:
1. **Virtual Backend Abstraction (`backend.h`):** Introduces the abstract `Backend` base class. `RenderBridge` holds a `std::unique_ptr<Backend>` that can point to a `CpuBackend` or a `GpuBackend`. This eliminates compile-time dependencies on the GPU engine (`GpuEngine`) within the core bridge headers. `GpuEngine` is forward-declared inside `render_bridge.h` and only instantiated in `render_bridge.cpp`.
2. **Decomposition of Stencil Operators (`field_operators.h`):** The 8 core discrete field operators (Laplacian, Divergence, Curl, Gradients) were extracted from the `RenderBridge` class body into inline free helpers inside `field_operators.h`. Headers that perform diagnostic calculations only need to include `field_operators.h`, preventing the inclusion of `render_bridge.h`.
3. **PIMPL Pattern for RNG State (`bridge_rng.h`):** Random number generation for Born-rule stochastic genesis and Langevin thermostats is isolated via the PIMPL (Pointer to Implementation) pattern. The MT19937 engine state is encapsulated inside the `BridgeRng` helper, keeping `<random>` out of `render_bridge.h` to minimize compilation bloat and speed up build times.
4. **Diagnostics Extraction (`render_bridge_diagnostics.h`):** The 5 Plain-Old-Data (POD) diagnostic structures (`Diagnostics`, `AggregateProfile`, `EnergyAudit`, `EnergyLedger`, `EMFieldDiag`) were extracted from `render_bridge.h` into a standalone header. This dramatically reduces the compilation fan-out (Translation Unit rebuild count) from ~30 TUs to ~5 when making diagnostic changes.

---

## 2. Runtime Execution Pipelines

The execution pipelines of the FTD engine are divided into a **Scale 0 Cellular Tick Cycle** and a **Multi-Scale Orchestration Layer** coordinating physical behaviors across multiple length scales.

```
       [TICK INITIATION]
              │
              ▼
   Strict Toggle Validation ──► [Invalid] ──► Abort / Throw
              │ [Valid]
              ▼
    Active Backend Dispatch
         /          \
        /            \
 [CpuBackend]    [GpuBackend]
      │               │
      │               ▼
      │        1. Reset Continuity Ledger
      │        2. Flush Host Mutations (Host ──► Device)
      │        3. Sync Toggles to GPU
      │        4. Execute Device-Side Tick (FFT, CUDA Kernels)
      │        5. Mark GPU Stale (gpu_dirty_ = true)
      │        6. Sync Device ──► Host (voxels_, phi, force_diag)
      │               │
      ▼               ▼
 ┌────────────────────────────────────────────────────────┐
 │                   THE PHASE LADDER                     │
 ├────────────────────────────────────────────────────────┤
 │ Phase 1: Wave Propagation (Moore 18-pt Laplacian)     │
 │ Phase 2: Commit Flux + Damping + Genesis + Evaporation │
 │ Phase 2b: Stochastic Pair Production                   │
 │ Phase 3: U(1) Gauss Constraint Projection (Poisson)    │
 │ Phase 3c: Latency Poisson (Gravitational Potential)    │
 │ Phase 4: Field-Mediated Forces (EM, Gravity, Lorentz)  │
 │ Phase 4b: QCD Color, Strong Yukawa, Spin-Exchange      │
 │ Phase 4c: Triad Binding Detection                      │
 │ Phase 5: Kinematics (Verlet, Collisions, Annihilation) │
 │ Phase 6: Electroweak Stress-Driven Transmutation       │
 │ Phase 8: Proper Time Accumulation + Bandwidth Clamp    │
 └────────────────────────────┬───────────────────────────┘
                              │
                              ▼
                Update Energy Ledger & proper_time
                              │
                              ▼
                       [TICK COMPLETE]
```

### 2.1 The Scale 0 Tick Cycle (11-Step Pipeline)
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
12. **Tick Completion:** Advances physical time and the tick counter, then runs `update_energy_ledger()` to balance total fields and kinetic energy, asserting that energy conservation residuals match the Alpha-damping profile.

---

## 2.2 Multi-Scale Orchestration Layer

The engine structures the physical layers of FTD into five distinct simulation scales. At runtime, the application bridges these scales dynamically through the polymorphic `ScaleEngine` interface.

```
┌────────────────────────────────────────────────────────────────────────┐
│                              ScaleEngine                               │
│                   (Polymorphic Runtime Interface)                      │
└──────────────────────────────────┬─────────────────────────────────────┘
                                   │
         ┌─────────────────────────┼─────────────────────────┐
         ▼                         ▼                         ▼
   Scale 0: Voxels           Scale 1: Particles         Scale 2: Atoms
  ([RenderBridge])          ([ParticleEngine])          ([AtomEngine])
         │                         │                         │
         │ (coarsen / refine)      │ (coarsen / refine)      │ (coarsen / refine)
         └───────► ◄───────────────┴───────► ◄───────────────┴────────► ◄──── ...
                                                                 Scale 5: Cosmic
                                                                ([CosmicEngine])
```

#### The Scale Engines
* **Scale 0 (Voxel / `RenderBridge`):** The fundamental discrete lattice simulation. Field variables are stored per voxel, and all dynamics emerge from local Causality Postulates.
* **Scale 1 (Particle / `ParticleEngine`):** Continuous-space representation. Eliminates the spatial lattice, modeling manifested particles as continuous entities ($x, v, a \in \mathbb{R}^3$) evolving under analytical $1/r^2$ Coulomb and gravitational forces. Employs a **Barnes-Hut Octree** ($O(N \log N)$) or pairwise integration ($O(N^2)$) to compute accelerations.
* **Scale 2 (Atom / `AtomEngine`):** Models composite atoms. Properties like mass, Thomas-Fermi radius, covalent bonding capacity, and Pauling electronegativity are derived analytically from atomic number $Z$. Integrates ionic forces, Van der Waals interactions (Lennard-Jones 12-6), and harmonic covalent spring bonds.
* **Scale 5 (Cosmic / `CosmicEngine`):** Macro-scale simulation. Implements comoving cosmological expansion under Friedmann equations. Combines gravity (N-body Barnes-Hut) with **Smoothed Particle Hydrodynamics (SPH)** for cosmic gas clouds. Models dark matter, dark energy, stellar evolution, black hole Bondi accretion, and relativistic jets.

---

## 2.3 Cross-Scale Mappings (Scale Transitions)
The transitions between scales are implemented in `scale_bridge.cpp`, utilizing two complementary paradigms: **Coarsening** (averaging microscopic states into microscopic parameters) and **Refinement** (decomposing macroscopic entities into structured microscopic components).

```
                      COARSENING (Scale Up)
   Scan Voxels with state != 0
   Extract Charge, Mass, Position, Velocity, Spin, Color
   Assemble continuous Particle structs.
         │
         ▼  [Scale 0 (Voxel)] ───────────────► [Scale 1 (Particle)]
         ▲
         │
   Inject Wavepacket (Gaussian envelope) at Center
   Distribute charge-flux energy amplitude (K_B)
   Restore remainder coordinates and velocity.
                      REFINEMENT (Scale Down)
```

#### 2.3.1 Scale 0 (Voxel) $\leftrightarrow$ Scale 1 (Particle)
* **Coarsening (`coarsen_to_particles`):** Scans all voxels in `RenderBridge`. For every voxel where `state` $\neq 0$, it extracts the charge ($q = s$), mass ($m = \max(|J|, K_B)$), continuous position (integer voxel coordinate + sub-lattice `remainder`), velocity, spin, color, and entanglement `pair_id` to generate a continuous `Particle` struct.
* **Refinement (`refine_to_voxels`):** Takes a continuous `Particle` and maps its continuous coordinates back to integer lattice coordinates. It invokes `inject_wavepacket()`, distributing a Gaussian flux envelope of amplitude $K_B$ and width $\sigma = 3.0$ around the target center. It then restores the sub-lattice remainder, velocity, spin, and color onto the newly generated manifested voxel.

#### 2.3.2 Scale 1 (Particle) $\leftrightarrow$ Scale 2 (Atom)
* **Coarsening (`coarsen_to_atoms`):** Evaluates particles in `ParticleEngine`. It clusters locked, positive protons ($q=+1$, `locked=true`) within a spatial clustering radius ($R \approx 5.0$) to form a composite atomic nucleus with atomic number $Z = \text{count}$. It then searches for nearby electrons ($q=-1$) to neutralize the net ionic charge.
* **Refinement (`refine_to_particles`):** Decomposes an `Atom` into $Z$ locked protons at the atomic center, and orbits $Z - \text{charge}$ electrons stochastically distributed at the Thomas-Fermi Bohr radius.

#### 2.3.3 Scale 2 (Atom) $\leftrightarrow$ Scale 5 (Cosmic)
* **Coarsening (`coarsen_to_cosmic`):** Aggregates atomic clusters in `AtomEngine` into baryonic SPH gas particles. Total mass is computed as the sum of atomic masses, and the comoving position is assigned to the center of mass (centroid).
* **Refinement (`refine_to_atoms`):** Decomposes a comoving SPH gas body of mass $M$ into structured hydrogen atoms distributed uniformly inside the SPH smoothing radius.

---

## 3. Host-Device (CPU/GPU) Data Transfer Boundaries

To accelerate massive grid calculations (e.g., $128^3$ grids containing $2,097,152$ voxels), FTD implements a native CUDA execution layer. The host-device data interface is designed to maximize GPU arithmetic throughput while minimizing PCI-Express bus saturation.

```
       HOST (CPU) MEMORY                        DEVICE (GPU) VRAM
 ┌───────────────────────────┐            ┌───────────────────────────┐
 │ Array-of-Structures (AoS) │            │ Structure-of-Arrays (SoA) │
 ├───────────────────────────┤            ├───────────────────────────┤
 │ std::vector<Voxel>        │            │ d_state       (int8_t*)   │
 │   - state                 │            │ d_flux_x      (double*)   │
 │   - flux.x, flux.y, flux.z│            │ d_flux_y      (double*)   │
 │   - wave_vel.x, ...       │   Upload   │ d_flux_z      (double*)   │
 │   - velocity.x, ...       ├───────────►│ d_wave_vel_x  (double*)   │
 │   - remainder.x, ...      │   (SoA)    │ d_wave_vel_y  (double*)   │
 │   - spin, color, flavor   │            │ d_wave_vel_z  (double*)   │
 │   - particle_id, pair_id  │            │ d_velocity_x  (double*)   │
 └───────────────────────────┘            │ ...                       │
                                          └───────────────────────────┘
                                                       │
                                                       ▼
                                            [CUDA KERNELS EXECUTE]
                                           (All field updates in VRAM)
                                                       │
               Sync to Host                            ▼
     ◄─────────────────────────────────────────────────┘
           - GpuBackend::sync_to_host()
           - GpuBuffers::download()
```

### 3.1 Memory Layout: AoS vs. SoA
* **Host Layout (AoS):** The CPU engine organizes voxels as an **Array of Structures** (`std::vector<Voxel>`). This is highly cache-friendly for sequential CPU traversals since all fields of a single site (state, flux, velocity) reside contiguously in memory.
* **Device Layout (SoA):** GPUs require memory coalescence to achieve peak memory bandwidth. In `GpuBuffers`, FTD decomposes the Voxel struct into a **Structure of Arrays** (SoA) layout. Separate, flat pointers (`d_state`, `d_flux_x`, `d_flux_y`, `d_flux_z`, `d_wave_vel_x`, etc.) are allocated in VRAM, allowing CUDA warps to read adjacent spatial fields in single, consolidated DRAM transactions.

### 3.2 The Lazy Synchronization State Machine
Data transfers over the PCI-Express bus are high-latency operations. The engine avoids redundant transfers through a lazy synchronization protocol managed by `GpuBackend` (which implements the virtual `Backend` interface):

```
                                  [Initial State]
                                 gpu_dirty = false
                                host_mutated = false
                                         │
                                         ▼
                               [Host Writes Voxels]
                           (mark_host_dirty() called)
                                         │
                                         ▼
                                 gpu_dirty = false
                                host_mutated = true
                                         │
                   ┌─────────────────────┴─────────────────────┐
                   ▼ (Before Next Tick)                        ▼ (External Host Read)
          [Upload to Device]                             [No Action Required]
        (push_to_device())                             (Host shadow is fresh)
                   │
                   ▼
                                 gpu_dirty = false
                                host_mutated = false
                                         │
                                         ▼
                               [GPU Kernel Executes]
                                         │
                                         ▼
                                 gpu_dirty = true
                                host_mutated = false
                                         │
                   ┌─────────────────────┴─────────────────────┐
                   ▼ (External Host Read)                      ▼ (Before Next Tick)
         [Download to Host]                             [No Action Required]
         (sync_to_host())                              (Avoid PCIe download)
                   │
                   ▼
                                 gpu_dirty = false
                                host_mutated = false
```

1. **`mark_host_dirty()`:** When host code mutates voxels directly (e.g., during particle injection or scenario initialization), it flags `host_mutated_ = true`.
2. **`flush_host_mutations()` / `push_to_device()`:** Prior to executing the next GPU tick, `GpuBackend` intercepts the call, uploads the modified host AoS data into the device SoA buffers, and resets `host_mutated_ = false`.
3. **`mark_gpu_dirty()`:** After the GPU executes a tick on the device, it sets `gpu_dirty_ = true`. The device data is now authoritative, and the host shadow is stale.
4. **`sync_to_host()`:** When host-side code requests access to the voxel array (either via non-const or const `voxels()` accessors, or during diagnostics/energy audits), `GpuBackend` intercepts the call. If `gpu_dirty_` is true, it performs a bulk PCIe download converting device SoA data back to host AoS (`GpuBuffers::download`), updates potentials (`phi`, `phi_coulomb`, `phi_latency`), scatters force components into `force_diag_`, and resets `gpu_dirty_ = false`.

This lazy synchronization structure guarantees that if the simulation is running continuously without user interaction or visual rendering requests, **zero voxel data is transferred over the PCIe bus**, enabling massive performance scaling.

---

## 4. GPU Kernel Acceleration Targets

The FTD CUDA architecture (`engine/cuda/`) targets the most computationally expensive aspects of the tick cycle for parallel execution.

### 4.1 Discrete Stencils & Leapfrog Updates (`kernels_stencil_single.cu` / `kernels_stencil_dual.cu`)
* **Moore 18-Point Stencil:** Computes the isotropic Laplacian in parallel. Thread blocks are mapped to 3D grid sub-volumes (e.g., $4 \times 8 \times 8$ blocks for $256$ threads). Boundary wrap-arounds are computed using inline register masking.
* **Leapfrog and Langevin Integration:** Fuses the updated $\Delta J$, damping, and Langevin stochastic noise updates into a single thread-per-voxel launch. Rather than generating random numbers on the host or using expensive global GPU generators, the Langevin thermal noise is generated **deterministically on the device per thread** using a fast SplitMix64 generator initialized with `(seed, voxel_index, tick, salt)`. This ensures bit-exact parity between CPU and GPU execution paths.

### 4.2 Exact Spectral Poisson Solvers (`kernels_poisson.cu`)
The U(1) Gauss constraint, Coulomb potential, and Latency fields all require solving the Poisson equation $\nabla^2 \phi = \rho$. The CPU engine uses an iterative Successive Over-Relaxation (SOR) solver. The GPU engine replaces this with an **Exact Spectral Solver** using fast Fourier transforms (FFT) via NVIDIA's **cuFFT** library:
1. **Fourier Transform:** Transforms the charge/mass density into Fourier space: $\hat{\rho}(k) = \text{FFT}(\rho(x))$.
2. **Spectral Green's Correction:** Computes the exact potential in k-space:
   $$\hat{\phi}(k) = \frac{\hat{\rho}(k)}{G(k)}$$
   where $G(k)$ is the precomputed discrete Laplacian eigenvalues for a periodic cubic lattice:
   $$G(k_x, k_y, k_z) = 2 \left[\cos\left(\frac{2\pi k_x}{L}\right) + \cos\left(\frac{2\pi k_y}{L}\right) + \cos\left(\frac{2\pi k_z}{L}\right) - 3\right]$$
   The DC mode ($G(0,0,0)$) is zeroed out to ensure gauge invariance and charge neutrality.
3. **Inverse Fourier Transform:** Transforms back to real space: $\phi(x) = \text{IFFT}(\hat{\phi}(k))$.

* **Precision Pathways:** The engine maintains a double-precision (Z2Z cuFFT) pathway and a single-precision (C2C cuFFT) pathway. By default, it executes the **float-precision C2C pathway**, which is **2× faster** on modern architectures. Float precision (7 decimal digits) is more than sufficient for gradient calculations ($\nabla \phi$).

### 4.3 Forces and Relativistic Integration (`kernels_forces.cu`)
* **`phase_forces_kernel`:** Maps a thread to every manifested voxel. It reads the precomputed potentials ($\phi_C, \phi_L$) and fields, and evaluates the EM gradient force, gravitational density gradient force, and Lorentz force. It then performs the full $\gamma_{FTD}$ relativistic integration and updates the velocity arrays.
* **`build_particle_list_kernel`:** Since forces like color charge, Yukawa, and exchange interactions are pair-wise forces acting only between manifested particles, evaluating them on the entire grid would be an expensive $O(N^2)$ operation where $N = L^3$. This kernel performs a parallel prefix sum (reduction) to build a compact array of active particle indices (`plist_idx`) and writes the active count to `d_num_particles`.

### 4.4 Electroweak & QCD Interactions (`kernels_eft.cu`)
* **`color_force_kernel`:** Iterates over the compact `plist_idx` array. Evaluates color forces according to color charge combinations: same color repels, different color attracts. Applies the three-regime force profile (Coulomb, Transition, Linear confinement) using a device-side running coupling $\alpha_s(Q)$ modeled after the QCD running coupling.
* **`yukawa_force_kernel`:** Computes the short-range nuclear strong force between hadrons:
   $$F_{Yukawa} = \alpha_s \frac{e^{-m_Y \cdot r}}{r^2}(1 + m_Y \cdot r)$$
* **`exchange_force_kernel`:** Evaluates Pauli exclusion exchange forces between identical fermions (same-spin particles):
   $$F_{Exchange} = \alpha_{exchange} \frac{e^{-r^2 / r_{ex}^2}}{r^2}$$

---

## 5. Architectural Recommendations

1. **Device-Side Reduction for the Energy Ledger:** Currently, the per-tick energy ledger computation on the GPU path requires downloading the entire voxel array (~3 MB at $L=64$) over the PCIe bus to run the sum on the CPU. Implementing a block-reduction CUDA kernel returning three scalars (`E_field`, `E_wave`, `E_kin`) would reduce the PCIe payload to $24$ bytes, eliminating a minor bottleneck.
2. **Double-Buffering for Wave Propagation:** The current GPU wave update uses a split kernel execution (`gpu_phase_read` then `gpu_phase_write`) to prevent thread race conditions. Implementing a double-buffered flux pointer on the device would allow fusing these two kernels into a single launch, improving GPU instruction cache efficiency.
3. **Consolidation of Multi-Scale GPU Acceleration:** The continuous-space engines (`ParticleEngine` and `AtomEngine`) currently upload their coordinates to the device to run custom pair-force kernels. Expanding the `GpuBackend` to keep these particles entirely on the GPU between ticks (mirroring `RenderBridge`'s lazy sync design) would unlock a massive performance boost for larger scale-1/2 runs.

---
*Report compiled by Worker M2. Verified against ftd core v2.11.*

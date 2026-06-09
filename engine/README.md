# FTD Simulation Engine

The FTD engine is a discrete simulation system for Foundational Ternary
Dynamics. Its fundamental Scale 0 model is a 3D cubic lattice. Each site has a
discrete ternary state and a continuous flux field:

| Layer | Type | Engine fields | Role |
|---|---|---|---|
| State | `{-1, 0, +1}` | `Voxel::state` | Void, negative manifestation, positive manifestation |
| Flux | `R^3` vector field | `Voxel::flux`, `Voxel::wave_vel` | Dispositional field that propagates, couples, and mediates forces |
| Motion | lattice kinematics | `velocity`, `remainder` | Manifested-particle motion through integer cells |
| Labels | discrete metadata | `particle_id`, `pair_id`, `spin`, `color`, `locked` | Identity, correlations, internal labels, bound-state locks |

Continuity is treated as emergent from repeated local updates, not as the
substrate. The simulation advances by local lattice loops, constraint
projection, field-mediated forces, and discrete manifestation events.

Detailed references:

- [VISUAL_GUIDE.md](VISUAL_GUIDE.md) - visual learner's guide to how the simulation works and why the discrete perspective matters.
- [SPEC_ENGINE.md](SPEC_ENGINE.md) - complete living engine reference.
- [ARCHITECTURE.md](ARCHITECTURE.md) - software call stacks, memory ownership, loop dynamics, manifestation lifecycle.
- [CALLSTACKS.md](CALLSTACKS.md) - feature-by-feature runtime callstacks from entrypoint to phase/kernel.
- [SCENARIO_ARCHITECTURE.md](SCENARIO_ARCHITECTURE.md) - scenario lifecycle, bridge ownership, toggle profiles, and cross-scale seed architecture.
- [../docs/SPEC_FTD.md](../docs/SPEC_FTD.md) - project-level theory specification.
- [../docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md](../docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md) - claim-status and epistemic accounting.

---

## Quick Start

### Build CPU

```bash
cmake -S engine -B engine/build -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build --config Release --parallel 24
```

### Test CPU

```bash
cd engine/build
ctest -j 24 --output-on-failure -C Release
```

### Build CUDA GPU

```bash
# Windows-native CUDA build. Measurement campaigns should use the documented WSL2 path.
cmake -S engine -B engine/build_cuda -DFTD_ENABLE_CUDA=ON -G Ninja \
      -DCMAKE_CUDA_FLAGS="--allow-unsupported-compiler"
cmake --build engine/build_cuda --config Release --parallel 24
```

### Build WASM Dashboard

```bash
emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release
emmake cmake --build engine/build_wasm --target ftd_wasm
cp engine/build_wasm/wasm/ftd_core.{js,wasm} engine/web/wasm/
```

### Run

```bash
./engine/build/Release/ftd_sim.exe [scenario] [lattice_size] [num_ticks]
python -m http.server 8080 -d engine/web
```

Open `http://localhost:8080` for the dashboard.

---

## Production Engine Boundary

| Component | Status | Notes |
|---|---|---|
| `RenderBridge` | Production Scale 0 | Flat voxel lattice, CPU tick ladder, SOR constraint solvers, diagnostics |
| `GpuEngine` / `GpuBackend` | Production when CUDA is enabled | Device SoA mirror, GPU tick ladder, lazy host/device synchronization |
| `ParticleEngine` | Production Scale 1 | Continuous particles and macro-level approximations |
| `AtomEngine` | Production Scale 2/3 | Atoms, molecules, bonds, thermostats |
| `CosmicEngine` | Production Scale 5 | N-body/SPH cosmic layer |
| `DagEngine` | Experimental | Sparse-lattice prototype; do not use for physics claims |

---

## Scale 0 Tick Cycle

`RenderBridge::tick()` runs the CPU ladder below unless a GPU backend is active.
Each major behavior is guarded by `TermToggles`.

```text
tick()
├─ validate toggles
├─ phase_read()                [wave_propagation || coupling]
├─ phase_write()               [always; damping/genesis/evaporation gated inside]
├─ pair_production_cpu()       [pair_production]
├─ gauss_project()             [gauss_projection]
├─ solve_latency_poisson()     [latency_field]
├─ phase_forces()              [forces]
├─ phase_movement()            [movement]
├─ apply_absorbing_boundary()  [absorbing_boundary]
├─ weak_transmutation_cpu()    [weak_transmutation]
├─ triad_binding_cpu()         [triad_binding]
├─ accumulate_proper_time()    [latency_field]
└─ tick/time/field/energy-ledger updates
```

The GPU path preserves the same logical order inside `GpuEngine::tick()`, with
CUDA kernels and device-side solvers replacing CPU loops where available.

---

## Loop Dynamics

The engine's dynamics are staged so local field updates can run in parallel,
while collision mutation remains ordered.

| Loop | Main file | Mutability rule | Purpose |
|---|---|---|---|
| Field read | `phase_read.cpp` | Read voxels, write delta buffers | 18-point Moore Laplacian, state-flux coupling, curl source |
| Field write | `phase_write.cpp` | Mutate flux/state in parallel | Staggered wave update, damping/noise, genesis, evaporation |
| Constraint | `poisson_solvers.cpp` | Project flux | Enforce Gauss relation by solving Poisson and subtracting `grad(phi)` |
| Force | `phase_forces.cpp` | Mutate velocities and diagnostics | Coulomb/emergent forces, gravity, Lorentz, optional color/strong/exchange |
| Movement | `phase_movement.cpp` | Sequential guarded mutation | Integer moves, bounce, annihilation, self-field transfer |
| Transmutation | `transmutation_phases.cpp` | Toggle-gated mutation | Pair production, weak flips, triad locks, proper time |

The wave advance is a staggered update:

```text
wave_vel += delta_J
flux     += wave_vel
```

With `symplectic_leapfrog`, the same update uses explicit `dt` factors. The
default path is the unit-tick staggered form, not a separate first-order Euler
scheme.

---

## Manifestation

Manifestation is the field-to-state transition. In `phase_write()`, a void site
can become a manifested ternary state when local field density exceeds the
genesis threshold.

```text
state == 0 and |flux| > K_GENESIS
  -> probability test
  -> state = +1 or -1
  -> spin/color sampled from local field geometry
  -> particle_id assigned deterministically
```

The lifecycle continues through later phases:

| Event | Mechanism |
|---|---|
| Genesis | Single-site stochastic manifestation from high flux |
| Evaporation | Low local 7-site energy clears unlocked manifested states |
| Pair production | High-flux void creates neighboring `-1/+1` pair |
| Movement | Velocity and remainder move manifested states across cells |
| Bounce | Same-sign collision reverses attempted motion |
| Annihilation | Opposite-sign collision clears both states and bursts field energy |
| Weak transmutation | Stress-threshold polarity flip when enabled |
| Triad binding | Compact same-sign triples become locked when enabled |

---

## Runtime Toggle Surface

The lattice engine uses a table-driven `TermToggles` registry. The current
Scale 0 surface has 33 boolean toggles and 6 typed configuration fields. Core
rules default on; exploratory extensions are toggle-gated.

Representative toggles:

| Area | Toggles |
|---|---|
| Core field/state | `wave_propagation`, `coupling`, `damping`, `genesis`, `evaporation`, `gauss_projection` |
| Forces/motion | `forces`, `gravity`, `poisson_coulomb`, `emergent_forces`, `lorentz_force`, `movement` |
| Field extensions | `dual_substrate`, `exact_dual_gauss`, `latency_field`, `field_energy_gravity`, `symplectic_leapfrog` |
| Stochastic/boundary | `langevin`, `larmor_radiation`, `selective_damping`, `absorbing_boundary`, `symmetric_movement_order` |
| Particle sectors | `color_forces`, `weak_transmutation`, `strong_force`, `triad_binding`, `pair_production`, `exchange_force`, `cluster_inertia`, `confinement` |
| Validation/gauge flags | `su2_gauge`, `su3_gauge`, `strict_validation` |

See `engine/include/ftd/term_toggles.h` and `engine/SPEC_ENGINE.md` for the
full registry, dependencies, and conflicts.

---

## Constants And Claim Status

Engine constants come from a mix of framework constants, simulation parameters,
and theory-side reference values exported through `ontic.h` and `constants.h`.
Do not infer claim status from a constant appearing in code:

- `X_PLUS` is the tree-level master-quadratic root used for `ALPHA` in active
  engine paths.
- `X_MINUS` remains a mathematical root; its old identification with `N_C` is
  retired in the ledgers.
- `N_C = 3` is an explicit framework/topological integer used by color paths.
- `G_N = 0.01` is a lattice-scaled simulation gravity parameter, not the
  physical gravitational coupling.
- Precision Standard Model computations live in scripts and theory docs, not in
  the Scale 0 runtime kernels.

For epistemic tags and derivation/parameter distinctions, use the project
assessment docs rather than this quickstart.

---

## Known Limits

- The lattice defines a preferred discrete frame; continuum symmetries are
  approximate and scale-dependent in the engine.
- Gauss projection is non-variational and can move field energy; conservation
  accounting tracks it separately.
- Short-range QCD-like, weak, pair-production, and latency/proper-time sectors
  are toggle-gated extensions.
- The engine is a computational ontology and simulation instrument, not a claim
  that FTD is experimentally confirmed.

---

## Directory Pointers

```text
engine/
  include/ftd/                  Public C++ headers
  VISUAL_GUIDE.md               Visual guide to the sim and discrete perspective
  CALLSTACKS.md                 Feature-by-feature runtime callstack map
  SCENARIO_ARCHITECTURE.md      Scenario lifecycle and seed architecture map
  src/render_bridge.cpp         CPU tick orchestration and backend handoff
  src/render_bridge_phases/     phase_read, phase_write, phase_forces, phase_movement
  src/poisson_solvers.cpp       Gauss, Coulomb, latency Poisson solvers
  src/transmutation_phases.cpp  pair production, weak transmutation, proper time, triad binding
  cuda/                         CUDA backend and kernels
  wasm/                         Emscripten bindings
  web/                          Browser dashboard
  tests/                        C++ tests and campaigns
```

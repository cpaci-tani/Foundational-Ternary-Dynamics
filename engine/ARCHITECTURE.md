# FTD Software Architecture

This document explains how the FTD engine is wired: execution call stacks,
memory ownership, backend synchronization, loop dynamics, and manifestation
lifecycle. It is an engineering map, not a proof ledger; physics claim status
lives in `docs/SPEC_FTD.md` and the assessment ledgers.

Canonical companions:

- `engine/VISUAL_GUIDE.md` - learner-facing visual guide to simulation flow and the discrete perspective.
- `engine/SPEC_ENGINE.md` - detailed living engine reference.
- `engine/CALLSTACKS.md` - feature-by-feature call graph from entrypoint to phase/kernel.
- `engine/SCENARIO_ARCHITECTURE.md` - scenario lifecycle, bridge ownership, toggle profiles, and cross-scale seed architecture.
- `engine/include/ftd/render_bridge.h` - Scale 0 public API.
- `engine/src/render_bridge.cpp` - CPU tick orchestration and backend handoff.
- `engine/src/render_bridge_phases/` - decomposed CPU phase implementations.
- `engine/cuda/gpu_engine.cu` - GPU tick orchestration.

## 1. System At A Glance

Scale 0 is the fundamental lattice engine. It stores a cubic lattice as a flat
array of `Voxel` records. Each voxel carries:

| Layer | Field | Role |
|---|---|---|
| Discrete state | `state in {-1, 0, +1}` | Void, negative manifestation, positive manifestation |
| Dispositional field | `flux`, `wave_vel` | Vector field and staggered wave velocity |
| Kinematics | `velocity`, `remainder` | Manifested-particle motion through integer lattice cells |
| Identity | `particle_id`, `pair_id`, `spin`, `color`, `locked` | Tracking, pair correlations, internal labels, bound-state locks |
| Optional sectors | `flux_L/R`, `wave_vel_L/R`, `latency`, `tau` | Dual substrate and latency/proper-time extensions |

The CPU path uses an array-of-structures layout (`std::vector<Voxel>`) because
phase code often touches many fields at one site. The CUDA path mirrors the same
state into structure-of-arrays buffers for coalesced device access.

## 2. RenderBridge Tick Call Stack

`RenderBridge::tick()` is the production Scale 0 execution loop. It validates
the active toggle combination, synchronizes any pending ternary-state mirrors,
then either delegates to the GPU backend or runs the CPU phase ladder.

```text
RenderBridge::tick()
├─ validate TermToggles
├─ sync_ternary_from_voxels_if_needed()
├─ if GPU backend:
│  ├─ backend_->tick()
│  ├─ accumulate_proper_time()       [latency_field]
│  └─ update_energy_ledger()
└─ CPU ladder:
   ├─ phase_read()                   [wave_propagation || coupling]
   ├─ phase_write()                  [always; damping/genesis/evaporation gated inside]
   ├─ pair_production_cpu()          [pair_production]
   ├─ gauss_project()                [gauss_projection]
   ├─ solve_latency_poisson()        [latency_field]
   ├─ phase_forces()                 [forces]
   ├─ phase_movement()               [movement]
   ├─ apply_absorbing_boundary()     [absorbing_boundary]
   ├─ weak_transmutation_cpu()       [weak_transmutation]
   ├─ triad_binding_cpu()            [triad_binding]
   ├─ accumulate_proper_time()       [latency_field]
   ├─ physical_time_ += dt_; ++tick_
   ├─ sync_ternary_from_voxels_if_needed()
   ├─ mark_fields_dirty_from_voxels()
   └─ update_energy_ledger()
```

The GPU path preserves the same logical phase order inside `GpuEngine::tick()`.
The main difference is solver implementation: CPU constraint solves use warm
started SOR buffers, while GPU constraint solves use device-side spectral/FFT
machinery where available.

## 3. Loop Dynamics

The engine is organized as a set of loops with different mutability rules.
Those rules are what keep parallel field updates deterministic while still
allowing sequential collision handling where order matters.

### 3.1 Field Read Loop

`phase_read()` is a parallel lattice loop. It reads the previous committed
voxel snapshot and writes only into delta buffers:

```text
delta_J = C_WAVE^2 * laplacian_18(J)
        + G_C * gradient(state)
        + G_C * curl(state * velocity)
```

The 18-point Moore stencil uses face and edge neighbors. It has fast interior
paths and slower periodic-boundary paths. In dual-substrate mode, left and
right flux fields get independent delta buffers and are recombined into the
observable `flux` after writing.

### 3.2 Field Write Loop

`phase_write()` is also a parallel lattice loop, but it mutates voxel fields.
It first snapshots pre-write flux where needed for deterministic genesis labels.
Then it commits the staggered wave update:

```text
wave_vel += delta_J
flux     += wave_vel
```

With `symplectic_leapfrog` enabled, the same staggered update is made explicit
with `dt` scaling:

```text
wave_vel += delta_J * dt
flux     += wave_vel * dt
```

The default unit-tick path is documented as the same staggered update under
`dt = 1`, not as a separate first-order Euler integrator. Damping, selective
damping, Larmor damping, and Langevin noise are all applied inside this loop
when their toggles are active.

### 3.3 Constraint Loop

`gauss_project()` is a projection step, not a variational force. It builds a
source from divergence mismatch,

```text
source = div(J) - coulomb_charge_coupling * state
```

solves a Poisson equation for `phi`, then subtracts `grad(phi)` from the flux
field. In ordinary mode the correction skips manifested sites; with
`exact_dual_gauss`, dual-substrate corrections are synchronized more directly
across the split fields. The conservation ledger treats this as a distinct
non-variational operator because it can change field energy.

### 3.4 Force Loop

`phase_forces()` iterates over manifested sites and writes force diagnostics and
new velocities. The default force path is field-mediated:

- Poisson Coulomb: solve `phi_coulomb`, then apply `-ALPHA * state * grad(phi)`.
- Emergent-force mode: use direct flux-density gradients instead of Poisson
  Coulomb; this conflicts with `poisson_coulomb`.
- Gravity: apply `G_N * grad(density)` with a tier-2 stencil.
- Lorentz: apply `ALPHA * state * (velocity x curl(flux))`.
- Optional color, strong, and exchange sectors add toggle-gated extensions.

Velocity integration uses the `gamma_FTD` bandwidth budget, coupling speed and
latency so the update respects the lattice speed limit.

### 3.5 Movement And Collision Loop

`phase_movement()` is intentionally sequential. It accumulates fractional
remainders, performs integer lattice moves, and resolves collisions:

| Target | Result |
|---|---|
| Void | Particle state, identity, motion, and carried self-field move to target |
| Same sign | Bounce; velocity/remainder reflect along attempted motion |
| Opposite sign | Annihilation; both states clear and stored self-field energy bursts to neighbors |

`symmetric_movement_order` can randomize traversal and axis order to reduce
directional artifacts, but collision mutation itself remains guarded by the
sequential moved-buffer logic.

## 4. Manifestation Lifecycle

Manifestation is the transition from high field intensity in a void cell into
an actual ternary state.

```text
void + high flux
  -> genesis probability test
  -> state = +1 or -1
  -> particle_id assigned in deterministic index order
  -> spin/color labels sampled from local field geometry
  -> Gauss projection / forces / movement in later phases
  -> persistence, evaporation, annihilation, transmutation, or binding
```

The main genesis path lives in `phase_write()`:

- Candidate: `state == 0` and `density() > K_GENESIS`.
- Probability: `p = 1 - exp(-(density - K_GENESIS) / K_MANIFEST)`.
- Polarity: single-substrate mode uses the sign of pre-write divergence;
  dual-substrate mode uses chirality density.
- Spin: extracted from pre-write curl, with deterministic RNG fallback when
  curl is degenerate.
- Color: selected from the dominant flux axis.
- Identity: `particle_id = -2` during the parallel write, then pending IDs are
  resolved deterministically after the loop.

Evaporation is the reverse channel. A manifested, unlocked site can return to
void when local 7-site energy is low enough. Pair production is a separate
toggle-gated channel that creates a neighboring `-1/+1` pair from high field
energy, conserving charge locally. Weak transmutation can flip polarity under
stress, and triad binding can lock compact same-sign triples.

## 5. Backend And Memory Ownership

The engine has a small backend abstraction:

| Backend | Ownership model | Notes |
|---|---|---|
| CPU `RenderBridge` | Host AoS is authoritative | `RenderBridge::tick()` runs phases directly |
| CUDA `GpuBackend` | Device SoA is authoritative between syncs | Host mutations are flushed before tick; host reads download lazily |
| WASM | Host AoS inside Emscripten heap | JS reads typed views and diagnostics through Embind |

Lazy synchronization avoids unnecessary PCIe transfers:

1. Host scenario/injection code marks the host dirty.
2. `GpuBackend::tick()` uploads host AoS into device SoA if needed.
3. CUDA kernels mutate device arrays and mark GPU dirty.
4. Host diagnostics or voxel reads trigger a download and rebuild host mirrors.

## 6. Multi-Scale Shell

`ScaleEngine` provides a common interface (`tick`, `dt`, `set_dt`,
diagnostics, toggle access) for Scale 0 and macro engines. `RenderBridge` is
the production fundamental lattice engine. `ParticleEngine`, `AtomEngine`, and
`CosmicEngine` model coarser analytical layers. `DagEngine` remains an
experimental sparse-lattice prototype and is not used for physics claims.

Cross-scale conversion lives in `scale_bridge.cpp`:

- Coarsening scans manifested voxels into particle/atom/cosmic aggregate data.
- Refinement injects wavepackets or structured components back into lower
  scales.

These bridges are engineering tools for moving between representations; they
do not replace the Scale 0 manifestation lifecycle described above.

## 7. Web And WASM Interface

`engine/wasm/ftd_wasm.cpp` exposes the engine to the browser dashboard through
Emscripten Embind. Large render payloads are surfaced as typed-array views
where possible, avoiding per-frame object serialization. The web dashboard then
renders particles, field overlays, force diagnostics, and scenario controls
without owning the physics state.

## 8. Practical Reading Order

For a full engine audit, read in this order:

1. `engine/VISUAL_GUIDE.md` - build the mental model first.
2. `engine/include/ftd/voxel.h` - data carried by a site.
3. `engine/include/ftd/term_toggles.h` - runtime feature surface.
4. `engine/SCENARIO_ARCHITECTURE.md` - understand how initial conditions enter the engine.
5. `engine/CALLSTACKS.md` - choose the feature path you are tracing.
6. `engine/src/render_bridge.cpp` - tick orchestration.
7. `engine/src/render_bridge_phases/phase_read.cpp` - field read loop.
8. `engine/src/render_bridge_phases/phase_write.cpp` - manifestation and field commit.
9. `engine/src/poisson_solvers.cpp` - Gauss, Coulomb, and latency projections.
10. `engine/src/render_bridge_phases/phase_forces.cpp` - force integration.
11. `engine/src/render_bridge_phases/phase_movement.cpp` - movement and collision.
12. `engine/src/transmutation_phases.cpp` - pair production, weak transmutation, proper time, triad binding.
13. `engine/cuda/gpu_engine.cu` - GPU phase ladder and host/device synchronization.

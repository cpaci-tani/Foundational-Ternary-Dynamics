# FTD Engine Callstacks

This document maps the primary simulation features from public entrypoints down
to the implementation functions that mutate state. It is documentation of the
runtime call graph, not a physics proof ledger.

Companion references:

- `engine/VISUAL_GUIDE.md` - conceptual visual guide for readers new to the simulation.
- `engine/SPEC_ENGINE.md` - detailed phase semantics and constants.
- `engine/ARCHITECTURE.md` - architecture, memory ownership, and loop dynamics.
- `engine/SCENARIO_ARCHITECTURE.md` - scenario lifecycle, bridge ownership, and seed setup.
- `engine/include/ftd/term_toggles.h` - runtime toggle registry.
- `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` - claim status.

## 1. Entrypoints

### 1.1 Browser Scale 0 Frame

```text
engine/web/js/app.js::animate()
  -> engine/web/js/scales/scale0/controller.js::animate(ctx)
     -> runtime/tick.js::advanceSimulation(ctx, state)
        -> ctx.bridge.capabilities.scale0.tickScale0()
           -> engine/web/js/bridge/capabilities/scale0.js
              -> WasmBridge.tick()
                 -> Emscripten RenderBridge::tick()
```

After ticks advance, render data flows back through:

```text
controller.js::animate(ctx)
  -> runtime/frame-sync.js::syncRenderableData(ctx, state, viewport)
     -> getScale0ParticleFrame()
        -> WasmBridge.getParticleData()
           -> bindings_render_bridge.cpp::get_particle_data(RenderBridge&)
     -> optional field samplers
        -> getFluxVolume / getFluxSlice / getEFieldSampled / ...
           -> bindings_render_bridge.cpp sampler helpers
  -> field-overlays.js update paths
  -> viewport adapter upload/render paths
```

When `state.useFluxMock` is true, the Scale 0 JS mock or worker owns the tick
instead. That path is a dashboard fallback/overlay source and is not the
canonical C++ physics path.

### 1.2 WASM Binding Surface

```text
WasmBridge.tick()
  -> this._bridge.tick()
     -> RenderBridge::tick()

WasmBridge.setToggle(name, value)
  -> bindings_render_bridge.cpp::set_toggle(RenderBridge*, name, value)
     -> RenderBridge::toggles field write

WasmBridge.injectParticle(...)
  -> bindings_render_bridge.cpp::inject_particle_simple(...)
     -> RenderBridge::inject_particle(...)

WasmBridge.setupScenario(name)
  -> bindings_render_bridge.cpp::setup_scenario(...)
     -> ftd::dispatch_scenario(RenderBridge&, name)
```

The `RenderBridge` class binding itself lives in `engine/wasm/ftd_wasm.cpp`.
The helper functions for data extraction, toggles, injection, and scenarios
register in `engine/wasm/bindings_render_bridge.cpp`.

### 1.3 Native WebSocket Bridge

```text
engine/src/ws_server.cpp
  -> command parse
     -> "tick" / run loop -> RenderBridge::tick()
     -> "inject_particle" -> RenderBridge::inject_particle()
     -> "inject_wavepacket" -> RenderBridge::inject_wavepacket()
     -> "create_entangled_pair" -> RenderBridge::create_entangled_pair()
     -> "setup_scenario" -> ftd::dispatch_scenario()
     -> "set_toggle" -> TermToggles field write
```

### 1.4 CLI And Tests

```text
engine/src/main.cpp or CTest/benchmark code
  -> RenderBridge rb(L)
  -> scenario setup / direct injection / toggle setup
  -> for N ticks: rb.tick()
  -> diagnostics / energy_audit / energy_ledger / exported samples
```

Scale 1/2/5 tests and demos call `ParticleEngine::tick()`,
`AtomEngine::tick()`, and `CosmicEngine::tick()` directly.

## 2. Scenario And Injection Setup

### 2.1 Scale 0 Scenario Router

```text
WASM setupScenario / C++ ftd::dispatch_scenario(rb, name)
  -> detail::reset_scenario_rng()
  -> setup_flux_scenario(rb, name)
  -> setup_light_scenario(rb, name)
  -> setup_quantum_scenario(rb, name)
  -> setup_vacuum_scenario(rb, name)
  -> setup_s0_seed_scenario(rb, name)
  -> setup_s0_field_scenario(rb, name)
  -> return first matching prefix result
```

Scenario bodies call the same public mutation surface as tests and the CLI:

```text
rb.toggles.<field> = ...
rb.seed_rng(...)
rb.inject_particle(...)
rb.inject_wavepacket(...)
rb.inject_flux(...)
rb.inject_flux_add(...)
rb.inject_wave_vel_add(...)
rb.create_entangled_pair(...)
rb.voxels()[i] / rb.set_state(...) for specialized seeds
```

### 2.2 Particle Injection

```text
RenderBridge::inject_particle(x, y, z, state, flux, spin, color)
  -> injection.cpp::inject_particle_cpu(rb, ...)
     -> if GPU backend active:
        -> rb.backend().flush_host_mutations()
        -> GpuEngine::inject_particle(...)
        -> rb.backend().mark_gpu_dirty()
     -> else CPU:
        -> rb.voxels()[idx]
        -> rb.set_state(idx, state)
        -> Voxel::flux / spin / color / particle_id writes
        -> optional dual-substrate split into flux_L / flux_R
```

### 2.3 Wavepacket Injection

```text
RenderBridge::inject_wavepacket(cx, cy, cz, state, sigma, amplitude)
  -> injection.cpp::inject_wavepacket_cpu(rb, ...)
     -> if GPU backend active:
        -> flush host mutations
        -> GpuEngine::inject_wavepacket(...)
        -> mark GPU dirty
     -> else CPU:
        -> set center state + particle_id
        -> scan cutoff radius
        -> normalize Gaussian shell
        -> add radial flux increments to neighboring voxels
        -> optional dual-substrate split
```

### 2.4 Raw Flux And Entangled Pair Setup

```text
RenderBridge::inject_flux(...)
  -> inject_flux_cpu(...)
     -> GPU: flush -> GpuEngine::inject_flux -> mark dirty
     -> CPU: write Voxel::flux and optional flux_L/R

RenderBridge::inject_flux_add(...)
  -> inject_flux_add_cpu(...)
     -> host read-modify-write through rb.voxels()

RenderBridge::inject_wave_vel_add(...)
  -> inject_wave_vel_add_cpu(...)
     -> host read-modify-write through rb.voxels()

RenderBridge::create_entangled_pair(...)
  -> create_entangled_pair_cpu(...)
     -> assign pair id
     -> set +1 source
     -> find first empty 6-neighbor
     -> set -1 partner with opposite flux
```

## 3. Scale 0 Tick Dispatch

### 3.1 CPU Tick Ladder

```text
RenderBridge::tick()
  -> TermToggles::validate()
     -> strict_validation: throw/abort on invalid combinations
     -> otherwise warn once per unique error
  -> sync_ternary_from_voxels_if_needed()
  -> if GPU backend active: GpuBackend::tick() path, then return
  -> cpu_runtime_warnings() once for CPU no-op/GPU-only toggles
  -> phase_read()                  [wave_propagation || coupling]
  -> phase_write()                 [always]
  -> pair_production_cpu()         [pair_production]
  -> gauss_project()               [gauss_projection]
  -> solve_latency_poisson()       [latency_field]
  -> phase_forces()                [forces]
  -> phase_movement()              [movement]
  -> apply_absorbing_boundary()    [absorbing_boundary]
  -> weak_transmutation_cpu()      [weak_transmutation]
  -> triad_binding_cpu()           [triad_binding]
  -> accumulate_proper_time()      [latency_field]
  -> physical_time_ += dt_; ++tick_
  -> sync_ternary_from_voxels_if_needed()
  -> mark_fields_dirty_from_voxels()
  -> update_energy_ledger()
```

### 3.2 GPU Tick Ladder

```text
RenderBridge::tick()
  -> backend_->tick()
     -> GpuBackend::tick()
        -> flush_host_mutations()
        -> copy RenderBridge toggles into GpuEngine
        -> GpuEngine::tick()
           -> reset continuity ledger
           -> gpu_phase_read()
           -> gpu_phase_write()
           -> optional pair production kernel path
           -> gpu_gauss_project()              [gauss_projection]
           -> gpu_solve_latency_poisson()      [latency_field]
           -> gpu_phase_forces()               [forces]
           -> optional particle-list/pairwise sector kernels
           -> optional triad detection
           -> gpu_phase_movement()             [movement]
           -> optional weak transmutation
           -> ++tick_; host_dirty_ = true
        -> sync_to_host() when host post-processing needs it
        -> optional host cluster-inertia pass after sync
  -> RenderBridge::accumulate_proper_time()     [latency_field]
  -> RenderBridge::update_energy_ledger()
```

The GPU path keeps device SoA buffers authoritative between host reads. Any
host-side scenario mutation must flush before a device mutation and marks the
device dirty afterward.

## 4. Scale 0 Primary Features

### 4.1 Toggle Validation

```text
RenderBridge::tick()
  -> TermToggles::validate(&err)
     -> iterate TOGGLE_SPECS[]
        -> enabled toggle requires_ checks
        -> enabled toggle conflicts checks
     -> hand-coded cross-cutting checks for non-boolean config fields
  -> TermToggles::cpu_runtime_warnings()
     -> warnings for CPU no-op / GPU-only implementation gaps
```

Primary data:

- `engine/include/ftd/term_toggles.h::TOGGLE_SPECS[]`
- `RenderBridge::toggles`

### 4.2 Wave Propagation And State-Flux Coupling

```text
RenderBridge::tick()
  -> RenderBridge::phase_read()
     -> phase_read.cpp::phase_read_main_loop(rb)
        -> if dual_substrate:
           -> parallel lattice loop
           -> 18-point laplacian on Voxel::flux_L / flux_R
           -> add 0.5 * G_C * gradient_state(i)
           -> add 0.5 * G_C * curl_state_velocity(i)
           -> write delta_j_L_[i], delta_j_R_[i]
        -> else single substrate:
           -> parallel lattice loop
           -> if bcc_stencil == FULL:
              -> interior fast 18-point laplacian, boundary wrapped path
           -> else:
              -> laplacian_sublattice<&Voxel::flux>(...)
           -> add G_C * gradient_state(i)
           -> add G_C * curl_state_velocity(i)
           -> write delta_j_[i]
```

Important helpers:

- `RenderBridge::gradient_state(i)`
- `RenderBridge::curl_state_velocity(i)`
- `field_operators.h::laplacian_field`
- `field_operators.h::laplacian_sublattice`

GPU mirror:

```text
GpuEngine::gpu_phase_read()
  -> kernels_stencil_single.cu / kernels_stencil_dual.cu launchers
  -> device 18-point stencil + coupling kernels
```

### 4.3 Field Commit, Damping, Larmor, And Langevin

```text
RenderBridge::tick()
  -> RenderBridge::phase_write()
     -> snapshot_flux_pre_write(rb)
     -> compute_near_particle_mask(rb)
     -> phase_write_main_loop(rb)
        -> parallel lattice loop
        -> dual path:
           -> wave_vel_L/R += delta_j_L/R [* dt if symplectic_leapfrog]
           -> flux_L/R += wave_vel_L/R   [* dt if symplectic_leapfrog]
           -> damping / Larmor damping
           -> observable flux = flux_L + flux_R
        -> single path:
           -> wave_vel += delta_j [* dt if symplectic_leapfrog]
           -> flux += wave_vel   [* dt if symplectic_leapfrog]
           -> Langevin OU update when enabled and site filter matches
           -> otherwise damping / Larmor damping
        -> shared evaporation block
     -> phase_write_assign_pending_ids(rb)
```

Primary data:

- `delta_j_`, `delta_j_L_`, `delta_j_R_`
- `near_particle_`, `near_accel_`
- `flux_pre_write_`
- per-thread RNG state in `BridgeRng`

GPU mirror:

```text
GpuEngine::gpu_phase_write()
  -> kernels_stencil_single.cu / kernels_stencil_dual.cu
  -> shared SplitMix64 voxel RNG for stochastic paths
```

### 4.4 Manifestation And Evaporation

Manifestation is inside `phase_write_main_loop`; it is not a separate public
phase.

```text
RenderBridge::tick()
  -> phase_write()
     -> snapshot_flux_pre_write()
     -> phase_write_main_loop()
        -> if do_genesis && state == 0 && density() > K_GENESIS:
           -> p = 1 - exp(-(density - K_GENESIS) / K_MANIFEST)
           -> voxel_uniform(... GenesisManifest)
           -> dual:
              -> polarity_signal = Voxel::chirality_density()
              -> manifest_at(... dual=true)
           -> single:
              -> drain wave_vel and flux latent heat
              -> polarity_signal = divergence_from_flux_array(flux_pre_write)
              -> manifest_at(... dual=false)
        -> manifest_at(...)
           -> rb.set_state(i, +1/-1)
           -> v.particle_id = -2 pending sentinel
           -> spin from curl_from_flux_array(flux_pre_write)
           -> fallback spin from voxel RNG if curl degenerate
           -> color from dominant live flux axis
        -> if (genesis || evaporation) && state != 0 && !locked:
           -> compute 7-site local field energy
           -> stochastic evaporation check
           -> rb.set_state(i, 0); clear id/spin/color
     -> phase_write_assign_pending_ids()
        -> scan voxel index order
        -> injector_.next_particle_id()
```

### 4.5 Pair Production

```text
RenderBridge::tick()
  -> pair_production_cpu() wrapper
     -> transmutation_phases.cpp::pair_production_cpu(rb)
        -> scan all voxels
        -> require state == 0 and |flux| > K_GENESIS
        -> stochastic voxel_uniform(... PairProduction)
        -> choose major flux axis and adjacent partner site
        -> require partner state == 0
        -> consume wave/flux energy
        -> rb.set_state(upstream, -1)
        -> rb.set_state(downstream, +1)
        -> assign particle ids and shared pair_id
        -> set downstream flux opposite upstream flux
```

GPU mirror:

```text
GpuEngine::tick()
  -> optional pair-production kernel path
```

Pair production is a separate phase from `phase_write` genesis and does not
require the `genesis` toggle.

### 4.6 Gauss Projection

```text
RenderBridge::tick()
  -> RenderBridge::gauss_project()
     -> poisson_solvers.cpp::gauss_project_cpu(
          voxels_, ternary_field(), phi_, sor_source_, lattice_,
          dual_substrate, exact_dual_gauss, coulomb_charge_coupling,
          sor_iterations_)
        -> parallel source build:
           source[i] = div(J)[i] - charge_coupling * state[i]
        -> for iter in sor_iterations:
           -> sor_sweep_18pt(phi, source, lattice, SOR_OMEGA)
              -> sequential red/black 18-point sweep
              -> interior fast path + boundary wrapped path
        -> parallel correction:
           -> skip manifested sites unless exact_dual_gauss
           -> grad_phi from phi
           -> voxels[i].flux -= grad_phi
           -> if dual_substrate: flux_L/R -= 0.5 * grad_phi
```

GPU mirror:

```text
GpuEngine::gpu_gauss_project()
  -> kernels_poisson.cu::launch_gauss_project(...)
  -> device Poisson/projection path
```

### 4.7 Coulomb Potential For Forces

```text
RenderBridge::tick()
  -> phase_forces()
     -> phase_forces_solve_potentials(rb)
        -> if poisson_coulomb && !emergent_forces:
           -> RenderBridge::solve_coulomb_poisson()
              -> poisson_solvers.cpp::solve_coulomb_poisson_cpu(
                   ternary_field(), phi_coulomb_, sor_source_, lattice_,
                   sor_iterations_)
                 -> build neutralized charge source
                 -> sor_sweep_18pt(...) repeated
                 -> subtract mean phi
```

`phase_forces_main_loop` then reads `phi_coulomb_` through
`RenderBridge::gradient_scalar(i, phi_coulomb_)`.

### 4.8 Latency Field And Proper Time

```text
RenderBridge::tick()
  -> solve_latency_poisson()                         [before forces]
     -> solve_latency_poisson_cpu(
          voxels_, ternary_field(), phi_latency_, sor_source_, lattice_,
          sor_iterations_, field_energy_gravity)
        -> build source from M_REST * |state|
        -> optionally add 0.5 * (|flux|^2 + |wave_vel|^2)
        -> subtract mean source
        -> sor_sweep_18pt(...) repeated
        -> subtract mean phi
        -> voxel.latency = sqrt(clamp(abs(phi), LATENCY_HORIZON_CLAMP))
  -> phase_forces()
     -> gamma_FTD momentum update reads voxel.latency
  -> accumulate_proper_time()                       [after movement/triad]
     -> transmutation_phases.cpp::accumulate_proper_time(rb)
        -> active manifested sites
        -> f = 1 - latency^2
        -> tau += sqrt(f^2 - speed^2) / sqrt(f)
```

GPU mirror:

```text
GpuEngine::gpu_solve_latency_poisson()
  -> kernels_poisson.cu latency solver path
RenderBridge::tick()
  -> after backend_->tick(), host accumulate_proper_time()
```

### 4.9 Field-Mediated Forces

```text
RenderBridge::tick()
  -> phase_forces()
     -> phase_forces_solve_potentials()
        -> optional solve_coulomb_poisson()
     -> phase_forces_build_color_cache()
        -> ordered_active_indices()
        -> collect manifested colored sites
     -> phase_forces_main_loop()
        -> parallel active-site loop
        -> EM:
           -> emergent_forces: tier-2 grad |J|
           -> else poisson_coulomb: -ALPHA * state * grad(phi_coulomb_)
           -> else legacy gradient_divergence(i)
        -> gravity:
           -> G_N * tier-2 gradient_density
        -> Lorentz:
           -> ALPHA * state * cross(velocity, curl_flux(i))
        -> color:
           -> loop colored_sites_cache_
           -> alpha_s_lattice(r)
           -> Coulomb / transition / linear profile
        -> write ForceDiag fields
        -> write voxel.accel_mag
        -> if !locked:
           -> reconstruct gamma_FTD momentum
           -> p += f_total * dt
           -> extract bounded velocity
     -> phase_forces_integrate_clusters() [cluster_inertia]
        -> flood-fill locked same-sign 26-connected clusters
        -> sum force_diag forces
        -> integrate COM velocity
        -> write V_COM to all members
```

GPU mirror:

```text
GpuEngine::gpu_phase_forces()
  -> kernels_forces.cu force kernel path
  -> optional particle list + pairwise sector kernels
  -> optional host phase_forces_integrate_clusters after sync
```

### 4.10 Movement, Bounce, And Annihilation

```text
RenderBridge::tick()
  -> phase_movement()
     -> phase_movement.cpp::phase_movement_main_loop(rb)
        -> clear moved_ buffer
        -> choose traversal:
           -> natural voxel order
           -> or shuffled voxel + axis order [symmetric_movement_order]
        -> for each manifested, unlocked, unmoved voxel:
           -> remainder += velocity * dt
           -> convert each axis crossing into dx/dy/dz integer step
           -> target = lattice.index(coord + step)
           -> if target state == 0:
              -> rb.set_state(target, moving_state)
              -> copy velocity/remainder/pair_id/accel/spin/color/particle_id
              -> transfer self-field up to K_B
              -> transfer dual L/R self-field proportionally when enabled
              -> rb.set_state(source, 0)
              -> clear source kinematic/identity labels
              -> moved_[target] = 1
           -> else if target state == source state:
              -> bounce by flipping velocity components along attempted axes
              -> clear remainder
           -> else:
              -> annihilation
              -> save source/target flux and dual flux
              -> rb.set_state(source, 0); rb.set_state(target, 0)
              -> clear motion/identity/spin/color/flux on both
              -> distribute each saved flux to that site's 6-neighbor shell
```

GPU mirror:

```text
GpuEngine::gpu_phase_movement()
  -> kernels_forces.cu / movement kernel path
```

### 4.11 Absorbing Boundary

```text
RenderBridge::tick()
  -> apply_absorbing_boundary(*this)
     -> phase_write.cpp::apply_absorbing_boundary(rb)
        -> scan lattice faces
        -> compute quadratic sponge factor by distance to nearest face
        -> damp flux, wave_vel, flux_L/R, wave_vel_L/R
```

This runs after Gauss/forces/movement so projection does not refill the edge
shell in the same tick.

### 4.12 Weak Transmutation

```text
RenderBridge::tick()
  -> weak_transmutation_cpu() wrapper
     -> transmutation_phases.cpp::weak_transmutation_cpu(rb)
        -> ordered_active_indices()
        -> compute stress:
           -> dual_substrate: compute_stress_left(i)
           -> single: compute_stress(i)
        -> if stress > WEAK_THRESHOLD:
           -> probability p = 1 - exp(-(stress - threshold) / K_MANIFEST)
           -> voxel_uniform(... WeakTransmutation)
           -> rb.set_state(i, -state)
           -> if dual_substrate: swap flux_L/R and wave_vel_L/R
```

### 4.13 Triad Binding

```text
RenderBridge::tick()
  -> triad_binding_cpu() wrapper
     -> transmutation_phases.cpp::triad_binding_cpu(rb)
        -> copy ordered_active_indices()
        -> triple nested scan over active particles
        -> require same sign, unlocked, pairwise distances <= TRIAD_RADIUS
        -> require rmin / rmax >= TRIAD_RATIO_THRESHOLD
        -> set locked = true on all three
```

### 4.14 Energy Ledger And Diagnostics

```text
RenderBridge::tick()
  -> update_energy_ledger()
     -> energy_ledger_compute.cpp::update_energy_ledger_cpu(rb)
        -> compute per-tick conservation drift snapshot

RenderBridge::diagnostics()
  -> diagnostics_compute.cpp helpers
  -> counts, charge, flux, energy-style summaries

RenderBridge::energy_audit()
  -> diagnostics/energy audit helpers
  -> field, wave, kinetic, potential, Gauss residual, EM diagnostics

WASM / Web panels
  -> bindings_render_bridge.cpp functions
     -> getDiagnostics / getEnergyAudit / getEnergyLedger / getLagrangian
     -> sampled field extractors for overlays
```

## 5. GPU Kernel Feature Map

```text
GpuBackend::tick()
  -> GpuEngine::tick()
     -> gpu_phase_read()
        -> kernels_stencil_single.cu or kernels_stencil_dual.cu
     -> gpu_phase_write()
        -> kernels_stencil_single.cu or kernels_stencil_dual.cu
     -> gpu_gauss_project()
        -> kernels_poisson.cu
     -> gpu_solve_latency_poisson()
        -> kernels_poisson.cu
     -> gpu_phase_forces()
        -> kernels_forces.cu
     -> gpu_phase_movement()
        -> kernels_forces.cu movement path
```

Device memory:

```text
RenderBridge host AoS vectors
  -> GpuBackend::flush_host_mutations()
     -> GpuBuffers upload into SoA arrays
  -> GpuEngine kernels mutate device SoA
  -> GpuBackend::sync_to_host()
     -> GpuBuffers download into host AoS
```

## 6. Scale 1 ParticleEngine Features

### 6.1 ParticleEngine Tick

```text
ParticleEngine::run(n)
  -> repeat ParticleEngine::tick()

ParticleEngine::tick()
  -> compute_all_forces()
  -> half_kick()
  -> drift()
  -> compute_all_forces()
  -> half_kick()
  -> store prev_acceleration and acceleration
  -> check_annihilation()
  -> enforce_speed_limit()
  -> apply_damping()
  -> ++tick_
```

### 6.2 Particle Forces

```text
ParticleEngine::compute_all_forces()
  -> resize forces_ and force_diag_
  -> if CUDA enabled, use_gpu_, no advanced toggles, N >= 8:
     -> gpu::ParticleEngineGpu::compute_pair_forces(...)
     -> forces_ and force_diag_ filled by GPU O(N^2) pair kernel
  -> else:
     -> octree_.build(particles_, position/mass/charge lambdas)
     -> for each particle:
        -> tree_force(i, octree_.root)
           -> Barnes-Hut opening-angle traversal
           -> compute_pairwise_force(i, j) for leaf/exact cases
              -> Coulomb
              -> gravity
              -> exchange
              -> strong/confinement profile
              -> magnetic dipole
              -> spin-orbit
              -> Lorentz
              -> optional relativistic correction
  -> per-particle post-pair additions:
     -> radiation reaction
     -> relativistic correction
  -> write forces_[i]
```

### 6.3 Particle Annihilation

```text
ParticleEngine::tick()
  -> check_annihilation()
     -> O(N^2) pair scan
     -> opposite charges within r_eff contact distance
     -> mark both for removal
     -> erase particles and force slots in reverse order
```

## 7. Scale 2/3 AtomEngine Features

### 7.1 AtomEngine Tick

```text
AtomEngine::run(n)
  -> repeat AtomEngine::tick()

AtomEngine::tick()
  -> compute_dipole_moments()
  -> compute_all_forces()
  -> half_kick()
  -> drift()
  -> compute_dipole_moments()
  -> compute_all_forces()
  -> half_kick()
  -> store acceleration
  -> check_bonding()
  -> apply_thermostat()
  -> enforce_speed_limit()
  -> apply_damping()
  -> ++tick_
```

### 7.2 Atom Forces And Bonds

```text
AtomEngine::compute_all_forces()
  -> zero force_diag_ and forces_
  -> if CUDA enabled, use_gpu_, !h_bonds, N >= 8:
     -> gpu::AtomEngineGpu::compute_pair_forces(...)
     -> ionic + vdW pair forces on GPU
  -> octree_.build(atoms_, position/mass/charge lambdas)
  -> if GPU did not handle pair loop:
     -> for each atom: tree_force(i, octree_.root)
        -> pairwise ionic / vdW / H-bond fallback
  -> covalent bond pass:
     -> for each atom and reciprocal bond
     -> harmonic bond force
  -> angle_strain pass:
     -> central atom bond-pair scan
     -> terminal atom reaction forces
  -> dipole_dipole pass
  -> torsional / improper torsional passes
  -> write AtomForceDiag fields
```

Bond lifecycle:

```text
AtomEngine::tick()
  -> check_bonding()
     -> atom_bonding.cpp::AtomEngine::check_bonding()
        -> pair scan
        -> if already bonded and r > 2 * r_eq: remove_bond()
        -> if unbonded, valence available, r < formation radius:
           -> create_bond(id_a, id_b, order)
```

Thermal/dipole support:

```text
compute_dipole_moments()
  -> atom_thermostat.cpp
  -> bond topology + electronegativity

apply_thermostat()
  -> Berendsen velocity rescaling when enabled

apply_damping()
  -> velocity damping when enabled
```

## 8. Scale 5 CosmicEngine Features

### 8.1 Cosmic Tick

```text
CosmicEngine::run(n)
  -> repeat CosmicEngine::tick()

CosmicEngine::tick()
  -> Phase A: forces at current positions
     -> forces_.assign(...)
     -> force_diag_.assign(...)
     -> build_octree()
     -> compute_gravity()
     -> compute_sph_density()
     -> compute_sph_forces()
     -> apply_hubble_expansion()
     -> apply_dark_energy()
     -> compute_magnetic_fields()
     -> compute_radiation_pressure()
  -> Phase B: Velocity Verlet
     -> half_kick()
     -> drift()
     -> clear/reassign forces_
     -> build_octree()
     -> compute_gravity()
     -> if sph_gas: compute_sph_density(); compute_sph_forces()
     -> half_kick()
  -> Phase C: post-integration lifecycle
     -> compute_accretion()
     -> compute_relativistic_jets()
     -> check_star_formation()
     -> check_stellar_evolution()
     -> detect_gw_events()
     -> propagate_gw()
     -> enforce_speed_limit()
     -> ++tick_
```

### 8.2 Cosmic Gravity

```text
CosmicEngine::build_octree()
  -> cosmic_barnes_hut.cpp
  -> octree_.build(bodies_, position/mass/type lambdas)

CosmicEngine::compute_gravity()
  -> if gravity enabled and octree exists
  -> for each body:
     -> tree_force(i, octree_.root)
        -> Barnes-Hut opening-angle traversal
        -> node aggregate or child recursion
  -> accumulate forces_ and force_diag_.f_gravity
```

### 8.3 Cosmic SPH Gas

```text
CosmicEngine::compute_sph_density()
  -> if sph_gas enabled
  -> find_sph_neighbors()
     -> O(N^2) gas/nebula neighbor search
  -> for each SPH body:
     -> self contribution sph_kernel_w(0, h)
     -> neighbor contributions sph_kernel_w(r, h)
     -> pressure = (gamma - 1) * rho * internal_energy
     -> adaptive smoothing_length

CosmicEngine::compute_sph_forces()
  -> for each SPH body:
     -> for each SPH neighbor:
        -> sph_kernel_grad(rij, h_avg)
        -> pressure force
        -> Monaghan-Gingold artificial viscosity
     -> accumulate forces_ and SPH force diagnostics
```

### 8.4 Cosmology, Compact Objects, Stars, And Waves

```text
apply_hubble_expansion()
  -> cosmic_cosmology.cpp::friedmann_step()
  -> update scale factor / Hubble diagnostics

apply_dark_energy()
  -> dark-energy acceleration/source path when enabled

compute_accretion()
  -> black holes / quasars scan nearby SPH bodies
  -> Bondi-Hoyle rate with Eddington cap
  -> transfer mass, update luminosity and latency

compute_relativistic_jets()
  -> black hole/quasar accretion-rate check
  -> jet velocity from latency budget
  -> recoil / luminosity update

check_star_formation()
  -> gas/nebula density and temperature criterion
  -> create new STAR body and reduce gas mass

check_stellar_evolution()
  -> star age/fuel/mass thresholds
  -> transition to white dwarf / neutron star / black hole paths

detect_gw_events()
propagate_gw()
  -> cosmic_gravitational_waves.cpp
```

## 9. Cross-Scale Conversion

```text
scale_bridge.cpp::coarsen_to_particles(RenderBridge&)
  -> scan manifested voxels
  -> extract charge, mass proxy, position, velocity, spin, color, pair_id
  -> ParticleEngine::add_particle(...)

scale_bridge.cpp::refine_to_voxels(ParticleEngine&, RenderBridge&)
  -> for each particle
  -> RenderBridge::inject_wavepacket(...)
  -> restore remainder/velocity/spin/color as needed

coarsen_to_atoms(ParticleEngine&)
  -> cluster locked protons + nearby electrons
  -> AtomEngine::add_atom(...)

refine_to_particles(AtomEngine&)
  -> locked protons + electron shells
  -> ParticleEngine::add_particle(...)

coarsen_to_cosmic(AtomEngine&)
  -> aggregate atom clusters into SPH/cosmic bodies

refine_to_atoms(CosmicEngine&)
  -> decompose gas bodies into atom populations
```

## 10. Debugging Reading Order

For a feature-level trace, start at the public entrypoint and follow only the
branch enabled by toggles:

1. Browser/WASM routing: `engine/web/js/scales/scale0/runtime/tick.js`,
   `engine/web/js/bridge/wasm-bridge.js`, `engine/wasm/bindings_render_bridge.cpp`.
2. Scale 0 orchestration: `engine/src/render_bridge.cpp::RenderBridge::tick`.
3. Scale 0 phase bodies: `engine/src/render_bridge_phases/*.cpp`,
   `engine/src/poisson_solvers.cpp`, `engine/src/transmutation_phases.cpp`.
4. GPU mirror: `engine/src/backend.cpp`, `engine/cuda/gpu_engine.cu`,
   `engine/cuda/kernels_*.cu`.
5. Macro engines: `engine/src/particle_engine.cpp`,
   `engine/src/atom_engine.cpp`, `engine/src/atom/*.cpp`,
   `engine/src/cosmic_engine.cpp`, `engine/src/cosmic/*.cpp`.

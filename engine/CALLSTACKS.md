# FTD Engine Callstacks

This document maps the primary simulation features from public entrypoints down
to the implementation functions that mutate state. It is documentation of the
runtime call graph, not a physics proof ledger.

> Verified against source at commit `e7f17d35`.

Companion references:

- `engine/VISUAL_GUIDE.md` - conceptual visual guide for readers new to the simulation.
- `engine/SPEC_ENGINE.md` - detailed phase semantics and constants.
- `engine/ARCHITECTURE.md` - architecture, memory ownership, and loop dynamics.
- `engine/SCENARIO_ARCHITECTURE.md` - scenario lifecycle, bridge ownership, and seed setup.
- `engine/docs/ENGINE_CODE_MAP.md` - file/subsystem navigation map + per-file manifest.
- `engine/include/ftd/term_toggles.h` - runtime toggle registry.
- `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md` - claim status.

## 1. Entrypoints

### 1.1 Browser Scale 0 Frame

```text
engine/web/js/app.js::animate(now)                       [lattice engineMode]
  -> engine/web/js/scales/scale0/controller.js::animateLattice(ctx)  (alias of animate)
     -> animate(ctx)
        -> runtime/tick.js::advanceSimulation(ctx, state)
           -> runtime/tick.js::runScale0PhysicsTicks(ctx, state, ticksToRun)
              -> ctx.bridge.capabilities.scale0.tickScale0()
                 -> engine/web/js/bridge/capabilities/scale0.js   (tickScale0: () => bridge.tick())
                    -> WasmBridge.tick()
                       -> Emscripten RenderBridge::tick()
```

After ticks advance, render data flows back through:

```text
controller.js::animate(ctx)
  -> runtime/frame-sync.js::syncRenderableData(ctx, state, viewport)
     -> activeScale0.getScale0ParticleFrame()
        -> WasmBridge.getParticleData()
           -> get_particle_data(RenderBridge&)   (defined in ftd_wasm.cpp,
              registered via bindings_render_bridge.cpp)
     -> optional field samplers (gated by visible overlays)
        -> activeScale0.getScale0FluxVolume / getScale0FluxSlice / getScale0FieldSamples
           -> WasmBridge.getFluxVolume / getFluxSlice / getEFieldSampled / ...
              -> sampler helpers (get_flux_slice / get_flux_volume defined in
                 ftd_wasm.cpp, registered in bindings_render_bridge.cpp)
  -> field-overlays.js update paths
  -> viewport adapter upload/render paths
```

When the active Scale-0 backend is the off-thread worker proxy
(`state.fluxMock.isWorker && state.useFluxMock`), the worker owns the tick:
`advanceSimulation` forwards run state (`setRunning` / `setTicksPerFrame`) to the
`WasmBridgeProxy`, and the worker self-ticks on its own loop, posting frames back
via `postFrame()`; the in-thread `tickScale0` path above runs only for the
non-worker case.

```text
runtime/tick.js::advanceSimulation(ctx, state)            [worker path]
  -> state.fluxMock.setRunning(ctx.running) / setTicksPerFrame(...)
     -> WasmBridgeProxy posts to wasm-bridge.worker.js
        -> worker hosts ftd_core_mt RenderBridge, self-ticks bridge.tick()
           -> postFrame() -> main-thread overlay/render refresh on frameCounter
```

The legacy JS MockBridge (`useFluxMock` without a worker) is a dashboard
fallback/overlay source and is not the canonical C++ physics path.

### 1.2 WASM Binding Surface

```text
WasmBridge.tick()
  -> this._bridge.tick()
     -> RenderBridge::tick()

WasmBridge.setToggle(name, value)
  -> bindings_render_bridge.cpp::set_toggle(RenderBridge&, name, value)
     -> RenderBridge::toggles field write

WasmBridge.injectParticle(...)
  -> bindings_render_bridge.cpp::inject_particle_simple(...)
     -> RenderBridge::inject_particle(...)

WasmBridge.setupScenario(name)
  -> bindings_render_bridge.cpp::setup_scenario(...)
     -> ftd::dispatch_scenario(RenderBridge&, name)
```

The `RenderBridge` class binding itself lives in `engine/wasm/ftd_wasm.cpp`
(`EMSCRIPTEN_BINDINGS(ftd_module_core)`, `class_<ftd::RenderBridge>`).
The helper functions for data extraction, toggles, injection, and scenarios
register in `engine/wasm/bindings_render_bridge.cpp`.

### 1.3 Native WebSocket Bridge

```text
engine/src/ws_server.cpp
  -> command parse (cmd)
     -> "tick" -> RenderBridge::tick()
     -> "run" (n) -> RenderBridge::run(n)
     -> "inject_particle" -> RenderBridge::inject_particle()
     -> "inject_wavepacket" -> RenderBridge::inject_wavepacket()
     -> "inject_flux" / "inject_flux_add" -> RenderBridge::inject_flux[_add]()
     -> "inject_wave_vel_add" -> RenderBridge::inject_wave_vel_add()
     -> "create_pair" -> RenderBridge::create_entangled_pair()
     -> "setup_scenario" -> ftd::dispatch_scenario()
     -> "set_toggle" -> find_toggle(rb->toggles, name) field write
     -> "set_param" -> RenderBridge::set_dt() (name == "dt")
     -> "resize" / "reset" -> rebuild RenderBridge
     -> "get_particles" / "get_diagnostics" / "get_energy_audit"
        / "get_flux_slice" / "get_flux_volume" / "info" -> read-back responses
```

### 1.4 CLI And Tests

```text
engine/src/main.cpp
  -> ftd::cli_demos::scenario_X(lattice_size, num_ticks, ...)   (engine/src/cli_demos/)
     -> RenderBridge engine(L)
     -> scenario setup / direct injection / toggle setup
     -> for N ticks: engine.tick()  (or engine.run(N))
     -> diagnostics / energy_audit / energy_ledger / exported samples

CTest/benchmark code
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
  -> if name == "empty": return true (baseline empty lattice, handled inline)
  -> setup_flux_scenario(rb, name)
  -> setup_light_scenario(rb, name)
  -> setup_quantum_scenario(rb, name)
  -> setup_vacuum_scenario(rb, name)
  -> setup_s0_seed_scenario(rb, name)
  -> setup_s0_field_scenario(rb, name)
  -> return first matching prefix result (first true wins)
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
        -> optional dual-substrate split: asymmetric DELTA_APPROX
           major/minor weighting into flux_L / flux_R, keyed on sign(state)
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
        -> optional dual-substrate split (DELTA_APPROX major/minor by sign(state))
```

### 2.4 Raw Flux And Entangled Pair Setup

```text
RenderBridge::inject_flux(...)
  -> inject_flux_cpu(...)
     -> GPU: flush -> GpuEngine::inject_flux -> mark dirty
     -> CPU: write Voxel::flux and optional flux_L/R (plain 0.5/0.5 split)

RenderBridge::inject_flux_add(...)
  -> inject_flux_add_cpu(...)
     -> no GPU path: host read-modify-write through rb.voxels()
        (optional 0.5/0.5 additive flux_L/R split)

RenderBridge::inject_wave_vel_add(...)
  -> inject_wave_vel_add_cpu(...)
     -> no GPU path: host read-modify-write through rb.voxels()
        (optional 0.5/0.5 additive wave_vel_L/R split)

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
       (incl. knot_tracking record + proper-time + ledger; see 3.2)
  -> cpu_runtime_warnings() once per instance for CPU no-op/GPU-only toggles
  -> ew_background_sweep flux drive       [ew_background_sweep]
  -> solve_coulomb_poisson()              [db_clock_coulomb]   (pre-read V(r))
  -> phase_read()                  [wave_propagation || coupling || de_broglie_clock]
  -> phase_write()                 [always]
  -> pair_production_cpu()         [pair_production]
  -> gauss_project()               [gauss_projection]
  -> solve_latency_poisson()       [latency_field]
  -> phase_forces()                [forces]
       -> phase_forces_integrate_clusters()   [cluster_inertia] (inside phase_forces)
  -> phase_movement()              [movement]
  -> apply_absorbing_boundary()    [absorbing_boundary]
  -> apply_reflective_flux_boundary() / apply_dispersal_flux_boundary()
                                   [flux_boundary == Reflective / Dispersal]
  -> weak_transmutation_cpu()      [weak_transmutation]
  -> triad_binding_cpu()           [triad_binding]
  -> relax_su2_links_cpu()         [su2_gauge]   (Rule 7b — links only, no substrate writes)
  -> relax_su3_links_cpu()         [su3_gauge]   (Rule 7b — links only, no substrate writes)
  -> accumulate_proper_time()      [latency_field || de_broglie_clock]
  -> physical_time_ += dt_; ++tick_
  -> knot_tracker_->record(*this)  [knot_tracking]  (observation-only)
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
        -> optional host cluster-inertia pass after sync   [cluster_inertia]
  -> RenderBridge::knot_tracker_->record(*this) [knot_tracking] (observation-only)
  -> RenderBridge::accumulate_proper_time()     [latency_field || de_broglie_clock]
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
     -> Pass 1: iterate TOGGLE_SPECS[]
        -> enabled toggle requires_ checks (comma-separated deps)
        -> enabled toggle conflicts checks (mutex pair, OFF-by-default side declares)
     -> Pass 2: hand-coded cross-cutting checks for non-boolean config fields
        (e.g. bcc_stencil != FULL vs dual_substrate)
  -> TermToggles::cpu_runtime_warnings()
     -> scan TOGGLE_SPECS[] gpu_only_warning for CPU no-op / GPU-only gaps
```

Primary data:

- `engine/include/ftd/term_toggles.h::TOGGLE_SPECS[]`
- `RenderBridge::toggles`

`TOGGLE_SPECS[]` currently holds 38 boolean-toggle rows (each `{name, field,
default, bulk_managed, requires, conflicts, gpu_only_warning, backends,
description}`). Beyond the core physics toggles (`wave_propagation`,
`coupling`, `damping`, `genesis`, `gauss_projection`, `forces`, `gravity`,
`poisson_coulomb`, `movement`, `lorentz_force`, `selective_damping`,
`dual_substrate`, `color_forces`, `weak_transmutation`, `triad_binding`,
`pair_production`, `latency_field`), the table includes:

- `evaporation` - phase_write evaporation alone (OR'd with genesis; test isolation)
- `larmor_radiation` - requires `damping`, conflicts `langevin`
- `strong_force` / `exchange_force` - CPU no-op (`gpu_only_warning` set)
- `exact_dual_gauss` - exact dual-cell face-flux Gauss projection (non-bulk)
- `emergent_forces` - EFT force-from-gradient; conflicts `poisson_coulomb`
- `langevin` - stochastic OU thermostat (CPU only at runtime, non-bulk)
- `symplectic_leapfrog`, `symmetric_movement_order`
- `su2_gauge` / `su3_gauge` - `[IMPOSED]` per-tick Wilson staple relaxation of the SU(2)/SU(3) link variables (CPU tick Rule 7b, GPU Phase 7b; revision 0.9 option a). Links are WRITE-ONLY w.r.t. the substrate (nothing consumes them; `color_forces` uses color labels) — gauge golden profile + write-only guarantee pinned in `test_gauge_links`; CPU/GPU parity in `test_gauge_gpu_parity`. Link buffers lazily allocated (528 B/site, revision 4.1b)
- `absorbing_boundary`, `reflective_boundary`
- `field_energy_gravity` - `[IMPOSED]` latency Poisson sources from 1/2|J|^2
- `cluster_inertia` - `[IMPOSED]` rigid-body cluster a_COM = F_cluster/(N*M_REST); requires `forces` (non-bulk)
- `de_broglie_clock` - `[IMPOSED]` KG mass term -omega0^2*J at manifested voxels (CPU-only backend; FTD-0271)
- `db_clock_coulomb` - `[IMPOSED diagnostic]` live Coulomb clock; requires `wave_propagation,de_broglie_clock,poisson_coulomb`, conflicts `forces` (CPU-only; FTD-0281)
- `confinement` - intent flag (no C++ branch yet)
- `knot_tracking` - `[OBSERVATION-ONLY]` per-knot telemetry at end of tick (golden-neutral)
- `strict_validation` - throw on `validate()` failure vs. stderr warn
- `ew_background_sweep` - sinusoidal +x flux drive before phase_read (EW hysteresis)

Non-bool config fields (`bcc_stencil`, `flux_boundary`, `langevin_site_filter`,
`langevin_seed`, etc.) live OUTSIDE `TOGGLE_SPECS[]` and are validated by the
hand-rolled Pass-2 checks in `validate()`.

### 4.2 Wave Propagation And State-Flux Coupling

```text
RenderBridge::tick()
  -> RenderBridge::phase_read()
     -> sync_ternary_from_voxels_if_needed()
     -> phase_read.cpp::phase_read_main_loop(rb)
        -> if dual_substrate:
           -> parallel lattice loop
           -> [wave_propagation] 18-point laplacian on Voxel::flux_L / flux_R
              (single neighbor sweep; interior fast path, boundary wrapped path)
           -> [coupling] subtract 0.5 * G_C * gradient_state_op(state, lat, ix,iy,iz)   (Term 2 sign, 2026-07-18)
           -> [coupling] add 0.5 * G_C * curl_state_velocity_op(state, voxels, lat, ix,iy,iz)
           -> [db_clock_coulomb] subtract flux_L/R * (omega0^2 - 2*omega0*phi_coulomb_[i])  (all sites)
              else [de_broglie_clock] && state != 0: subtract flux_L/R * omega0^2
           -> write delta_j_L_[i], delta_j_R_[i]
        -> else single substrate:
           -> parallel lattice loop
           -> [wave_propagation]:
              -> if bcc_stencil == FULL:
                 -> interior fast 18-point laplacian, boundary wrapped path (laplacian_flux)
              -> else:
                 -> laplacian_sublattice<&Voxel::flux>(stencil_mode, ...)
           -> [coupling] subtract G_C * gradient_state_op(state, lat, ix,iy,iz)   (Term 2 sign, 2026-07-18)
           -> [coupling] add G_C * curl_state_velocity_op(state, voxels, lat, ix,iy,iz)
           -> [db_clock_coulomb] subtract flux * (omega0^2 - 2*omega0*phi_coulomb_[i])  (all sites)
              else [de_broglie_clock] && state != 0: subtract flux * omega0^2
           -> write delta_j_[i]
```

The de Broglie clock branch (FTD-0271 / FTD-0281) is a Klein-Gordon rest-mass
term `-omega0^2*J` added at manifested (state != 0) voxels; `delta_j` is acceleration,
so the leapfrog integrator gives the KG dispersion `omega^2 = c^2k^2 + omega0^2`. It is
[IMPOSED] (native flux is massless) and strictly additive - with the toggle OFF
it is a dead branch and the golden hash is unaffected.

Important helpers:

- `field_operators.h::gradient_state_op(state, lattice, ix, iy, iz)`
- `field_operators.h::curl_state_velocity_op(state, voxels, lattice, ix, iy, iz)`
- `field_operators.h::laplacian_field`
- `sublattice.h::laplacian_sublattice`

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
     -> [genesis] snapshot_flux_pre_write(rb)
     -> [selective_damping] compute_near_particle_mask(rb)
     -> phase_write_main_loop(rb)
        -> parallel lattice loop
        -> dual path:
           -> wave_vel_L/R += delta_j_L/R [* dt if symplectic_leapfrog]
           -> flux_L/R += wave_vel_L/R   [* dt if symplectic_leapfrog]
           -> [damping] damping / [larmor_radiation] Larmor-modulated damping
           -> observable flux = flux_L + flux_R, wave_vel = wave_vel_L + wave_vel_R
        -> single path:
           -> wave_vel += delta_j [* dt if symplectic_leapfrog]
           -> flux += wave_vel   [* dt if symplectic_leapfrog]
           -> [langevin] OU update on wave_vel when site_matches_filter(langevin_site_filter):
              -> sigma = sqrt(gamma * (2 - gamma) * T)   (FDT-consistent)
              -> wave_vel = (1 - gamma) * wave_vel + sigma * voxel_normal(...)
           -> otherwise [damping] damping / [larmor_radiation] Larmor-modulated damping
        -> shared genesis/evaporation block (Loop 2, sequential)
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

Manifestation is inside `phase_write_main_loop` (Loop 2, a sequential pass for
determinism); it is not a separate public phase.

```text
RenderBridge::tick()
  -> phase_write()
     -> [genesis] snapshot_flux_pre_write()
     -> phase_write_main_loop()
        -> reset genesis_events_this_tick_ / evaporation_events_this_tick_  (FTD-0267 telemetry)
        -> if do_genesis && state == 0 && |flux|^2 > K_GENESIS^2:
           -> p = 1 - exp(-(density - K_GENESIS) / K_MANIFEST)
           -> voxel_uniform(... GenesisManifest) < p
           -> atomic_inc(genesis_events_this_tick_)   (observation only)
           -> dual:
              -> polarity_signal = Voxel::chirality_density()
              -> manifest_at(... dual=true)
           -> single:
              -> drain latent heat: wave_vel *= (1 - kinetic_drain)  [kinetic_drain toggle, default 0.5]
              -> flux *= max(0, 1 - K_GENESIS / |flux|)
              -> polarity_signal = divergence_from_flux_array(flux_pre_write)
              -> manifest_at(... dual=false)
        -> manifest_at(...)
           -> rb.set_state(i, +1/-1)
           -> v.particle_id = -2 pending sentinel
           -> spin from curl_from_flux_array(flux_pre_write)
           -> fallback spin from voxel RNG if curl degenerate
           -> color from dominant live flux axis
        -> if (genesis || evaporation) && state != 0 && !locked:
           -> compute 7-site local field energy (self + neighbors_6)
           -> evap_prob = exp(-local_energy / K_MANIFEST^2)
           -> voxel_uniform(... Evaporation) < evap_prob * K_EVAP_RATE
           -> atomic_inc(evaporation_events_this_tick_)   (observation only)
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
        -> genesis probability p = 1 - exp(-(|flux| - K_GENESIS) / K_MANIFEST)
        -> stochastic voxel_uniform(... PairProduction) < p
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
           source[i] = div(J)[i] - charge_coupling * (state[i] - mean_charge)
        -> for iter in sor_iterations:
           -> sor_sweep_18pt(phi, source, lattice, SOR_OMEGA)
              -> 8-color (2x2x2) parity sweep (NOT 2-color red/black;
                 2-color races on the 18-point stencil's 12 edge diagonals)
              -> interior cells: PARALLEL (race-free, never wraps)
              -> boundary cells: SEQUENTIAL lexicographic per colour
                 (periodic wrap can pair two same-colour boundary cells;
                  bit-exact to a fully-sequential sweep for the golden gate)
        -> sequential phi-mean subtract (golden-gate determinism)
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
        -> optionally add 0.5 * (|flux|^2 + |wave_vel|^2)  [field_energy_gravity]
        -> subtract mean source
        -> sor_sweep_18pt(...) repeated
        -> subtract mean phi
        -> voxel.latency = sqrt(clamp(abs(phi), LATENCY_HORIZON_CLAMP))
  -> phase_forces()
     -> gamma_FTD momentum update reads voxel.latency
  -> accumulate_proper_time()                       [after movement/triad]
     -> transmutation_phases.cpp::accumulate_proper_time(rb)
        -> active manifested sites (ordered_active_indices)
        -> f = 1 - latency^2
        -> delta_tau = sqrt(f^2 - speed^2) / sqrt(f)
        -> tau += delta_tau
        -> if de_broglie_clock: phase += omega0 * delta_tau  [de_broglie_clock]
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
        -> gravity [gravity]:
           -> G_N * tier-2 gradient_density
        -> Lorentz [lorentz_force, speed > EPSILON_MAG]:
           -> ALPHA * state * cross(velocity, curl_flux(i))
        -> color [color_forces]:
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
           -> or shuffled voxel + per-voxel axis order [symmetric_movement_order]
        -> for each manifested, unlocked, unmoved voxel:
           -> remainder += velocity * dt
           -> convert each axis crossing into dx/dy/dz integer step
           -> if no integer step: continue
           -> handle_face_crossing(rb, v, dx, dy, dz, i):
              -> if step would leave the lattice:
                 -> [reflective_boundary] ON: mirror-bounce velocity on crossed axes, clear remainder
                 -> else: remove particle (set_state(i,0), clear velocity/remainder/
                          pair_id/particle_id/spin/color/flux; dual flux_L/R when enabled)
                 -> return Handled (skip rest of this voxel)
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

### 4.11 Absorbing / Flux Boundary

```text
RenderBridge::tick()
  -> apply_absorbing_boundary(*this) [absorbing_boundary]
     -> phase_write.cpp::apply_absorbing_boundary(rb)
        -> scan lattice faces
        -> compute quadratic sponge factor by distance to nearest face
        -> damp flux, wave_vel, flux_L/R, wave_vel_L/R
  -> flux_boundary law (default Periodic = toroidal wrap, no pass):
     -> FluxBoundaryMode::Reflective:
        -> phase_write.cpp::apply_reflective_flux_boundary(rb)
           -> copy first interior layer into the boundary shell (Neumann mirror cavity)
     -> FluxBoundaryMode::Dispersal:
        -> phase_write.cpp::apply_dispersal_flux_boundary(rb)
           -> scale outer shell by (1 - C_SPEED) (single-cell radiating sink)
```

The sponge and the Reflective/Dispersal passes all run after Gauss/forces/movement
(the last flux writers) so projection does not refill the edge shell in the same tick.

### 4.12 Weak Transmutation

```text
RenderBridge::tick()
  -> [weak_transmutation] weak_transmutation_cpu() wrapper
     -> transmutation_phases.cpp::weak_transmutation_cpu(rb)
        -> ordered_active_indices()
        -> compute stress:
           -> dual_substrate: compute_stress_left(i)
           -> single: compute_stress(i)
        -> if stress > WEAK_THRESHOLD:
           -> probability p = 1 - exp(-(stress - threshold) / K_MANIFEST)
           -> if voxel_uniform(... WeakTransmutation) < p:
              -> rb.set_state(i, -state)
              -> if dual_substrate: swap flux_L/R and wave_vel_L/R
```

### 4.13 Triad Binding

```text
RenderBridge::tick()
  -> [triad_binding] triad_binding_cpu() wrapper
     -> transmutation_phases.cpp::triad_binding_cpu(rb)
        -> copy ordered_active_indices()
        -> triple nested scan over active particles
        -> require same sign, unlocked, pairwise distances <= TRIAD_RADIUS
        -> require rmin / rmax >= TRIAD_RATIO_THRESHOLD
        -> set locked = true on all three
```

### 4.13b Non-Abelian Gauge-Link Relaxation (Rule 7b, revision 0.9 option a)

`[IMPOSED]` Wilson-action staple relaxation imported from standard lattice
gauge theory (rates `GAUGE_RELAX_DT`/`GAUGE_RELAX_BETA`, constants.h). The
links are write-only w.r.t. the substrate — nothing downstream consumes them —
so this phase cannot move any pinned golden (enforced by `test_gauge_links`
G1a/G1b; CPU/GPU parity by `test_gauge_gpu_parity`).

```text
RenderBridge::tick()                              (CPU path)
  -> [su2_gauge] relax_su2_links_cpu(rb, GAUGE_RELAX_DT, GAUGE_RELAX_BETA)
     -> ensure_gauge_links()   (lazy 528 B/site buffers, revision 4.1b)
     -> Jacobi sweep: read su2_links_*, staple update, write scratch, swap
  -> [su3_gauge] relax_su3_links_cpu(rb, GAUGE_RELAX_DT, GAUGE_RELAX_BETA)
     -> same double-buffered sweep over su3_links_*

GpuBackend::tick()                                (GPU path)
  -> first gauge-enabled tick: bridge.ensure_gauge_links()
     -> GpuEngine::upload_gauge_links(host arrays)   (lazy device alloc)
  -> GpuEngine::tick() Phase 7b
     -> [su2_gauge] launch_relax_su2_links(src, dst) (kernels_gauge.cu) + swap
     -> [su3_gauge] launch_relax_su3_links(src, dst) + swap
  -> sync_to_host(): GpuEngine::download_gauge_links() -> host arrays
```

### 4.14 Energy Ledger And Diagnostics

```text
RenderBridge::tick()
  -> update_energy_ledger()
     -> energy_ledger_compute.cpp::update_energy_ledger_cpu(rb)
        -> compute per-tick conservation drift snapshot

RenderBridge::diagnostics()
  -> diagnostics_compute.cpp::compute_diagnostics(rb)
  -> counts, charge, flux, energy-style summaries

RenderBridge::energy_audit()
  -> diagnostics_compute.cpp::compute_energy_audit(rb)
  -> field, wave, kinetic, potential, Gauss residual, EM diagnostics

WASM / Web panels
  -> bindings_render_bridge.cpp functions
     -> getDiagnostics / getEnergyAudit / getEnergyLedger / getLagrangian
     -> getDiagnosticsView / getEnergyAuditView / getLagrangianView (struct-of-arrays variants)
     -> getKnotTelemetry / getKnotEvents / getKnotAggregate (observation-only KnotTracker)
     -> setLangevinTemp / getLangevinTemp (FTD-0274 thermal bath temperature)
     -> setOmega0 / getOmega0 (FTD-0271 de Broglie clock frequency)
     -> sampled field extractors for overlays
```

## 5. GPU Kernel Feature Map

```text
GpuBackend::tick()
  -> GpuEngine::tick()
     -> gpu_phase_read()
        -> [dual_substrate] kernels_stencil_dual.cu (launch_phase_read_dual)
        -> [else]           kernels_stencil_single.cu (launch_phase_read)
     -> gpu_phase_write()
        -> [dual_substrate] kernels_stencil_dual.cu (launch_phase_write_dual)
        -> [else]           kernels_stencil_single.cu (launch_phase_write)
        -> [color_forces || strong_force] kernels_stencil_dual.cu (launch_strong_field_stencil)
        -> [weak_field_active]            kernels_stencil_dual.cu (launch_weak_field_stencil)
     -> [pair_production] gpu_pair_production()
        -> kernels_aux.cu (launch_pair_production)
     -> [gauss_projection] gpu_gauss_project()
        -> kernels_poisson.cu (launch_gauss_project, FFT Poisson via fft_poisson_solve_f)
        -> [dual_substrate] kernels_stencil_dual.cu (launch_gauss_sync_dual)
     -> [latency_field] gpu_solve_latency_poisson()
        -> kernels_poisson.cu (launch_solve_latency, FFT Poisson)
     -> [forces] gpu_phase_forces()
        -> [poisson_coulomb && !emergent_forces] gpu_solve_coulomb()
           -> kernels_poisson.cu (launch_solve_coulomb, FFT Poisson)
        -> kernels_forces.cu (launch_phase_forces)
     -> [color_forces || strong_force || exchange_force || triad_binding]
        -> gpu_build_particle_list() -> kernels_forces.cu (launch_build_particle_list)
        -> gpu_particle_forces()
           -> [color_forces]    kernels_forces.cu (launch_color_force)
           -> [strong_force]    kernels_forces.cu (launch_yukawa_force)
           -> [exchange_force]  kernels_forces.cu (launch_exchange_force)
     -> [triad_binding] gpu_triad_detection()
        -> kernels_forces.cu (launch_triad_detection)
     -> [movement] gpu_phase_movement()
        -> kernels_forces.cu movement path (launch_phase_movement)
     -> [weak_transmutation] gpu_weak_transmutation()
        -> kernels_aux.cu (launch_weak_transmutation)
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

ParticleEngine::tick()   (velocity Verlet)
  -> compute_all_forces()
  -> half_kick()                 [relativistic_verlet: momentum push; else v += (dt/2)F/m]
  -> enforce_speed_limit()       (before drift, to avoid teleportation at singularities)
  -> drift()                     (r += dt * v)
  -> compute_all_forces()        (forces at new positions)
  -> half_kick()
  -> store prev_acceleration and acceleration  (per non-locked particle)
  -> check_annihilation()
  -> enforce_speed_limit()       (hard clamp to C_SPEED; breaks symplecticity by design)
  -> apply_damping()             [damping]
  -> evolve_spin_axes()          [magnetic_dipole or lorentz] (spin precession from partner B-fields)
  -> ++tick_
```

### 6.2 Particle Forces

```text
ParticleEngine::compute_all_forces()
  -> resize forces_ and force_diag_
  -> if CUDA enabled, use_gpu_, no advanced toggles
     (none of strong/exchange/lorentz/magnetic_dipole/spin_orbit/radiation/relativistic),
     and N >= 8:
     -> gpu_backend_->engine.compute_pair_forces(...)
        (GpuBackend wraps a gpu::ParticleEngineGpu)
     -> forces_ and force_diag_ filled by GPU O(N^2) Coulomb + gravity pair kernel
        (gpu_pair_handled = true)
  -> else (CPU Barnes-Hut fallback):
     -> octree_.build(particles_, position/mass/charge lambdas)
     -> for each particle:
        -> tree_force(i, octree_.root)
           -> Barnes-Hut opening-angle traversal (THETA_BH = 0.5)
           -> far node (monopole approx): Coulomb [coulomb] + gravity [gravity] only
           -> leaf / near node: compute_pairwise_force(i, j) for each body
              -> Coulomb               [coulomb]
              -> gravity               [gravity]
              -> exchange (Pauli)      [exchange]   (same-spin, same-charge)
              -> strong/confinement    [strong]     (colored particles; 3-regime profile)
              -> magnetic dipole       [magnetic_dipole]
              -> spin-orbit            [spin_orbit]
              -> Lorentz               [lorentz]
  -> per-particle post-pair additions (NOT pairwise):
     -> radiation reaction    [radiation]    (self-interaction)
     -> relativistic correction [relativistic] (MUST be last; isotropic 1/gamma rescale)
  -> write forces_[i]
```

### 6.3 Particle Annihilation

```text
ParticleEngine::tick()
  -> check_annihilation()
     -> O(N^2) pair scan
     -> opposite charges (charge_i * charge_j < 0)
        within contact distance (r_eff_i + r_eff_j)
     -> mark both for removal
     -> erase particles_ and forces_ slots in reverse order
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
     -> sets gpu_pair_handled = true
  -> octree_.build(atoms_, position/mass/charge lambdas)
     (always built - needed for the H-bond fallback + diagnostics)
  -> if !gpu_pair_handled (GPU did not handle pair loop):
     -> for each atom: tree_force(i, octree_.root)
        -> leaf: compute_pairwise_force [ionic] / [van_der_waals] / [h_bonds]
        -> internal node: Barnes-Hut opening test (THETA_BH = 0.5);
           far -> ionic monopole approximation; near -> recurse children
  -> covalent bond pass [covalent_bonds]:
     -> for each atom and reciprocal bond
     -> harmonic bond force
  -> angle_strain pass [angle_strain]:
     -> central atom bond-pair scan (VSEPR theta_eq from steric number)
     -> terminal atom forces + Newton's-3rd-law center reaction
  -> dipole_dipole pass [dipole_dipole]
  -> torsional pass [torsional] (4-body 1-2-3-4 dihedral)
  -> improper torsional pass [improper_torsional] (sp2 planarity)
  -> write AtomForceDiag fields (f_ionic, f_vdw, f_hbond, f_bond,
     f_angle, f_dipole, f_torsion, f_improper)
```

Bond lifecycle:

```text
AtomEngine::tick()
  -> check_bonding()
     -> atom_bonding.cpp::AtomEngine::check_bonding()
        -> [auto_bonding] gate (returns early if off)
        -> O(N^2) pair scan
        -> if already bonded and r > 2 * r_eq: remove_bond(ai.id, aj.id)
        -> if unbonded, both have a free bond slot (bonds.size() < max_bonds),
           and r < formation radius (1.2 * sigma_avg, widened by the
           electronegativity difference under [electronegativity]):
           -> create_bond(ai.id, aj.id, 1)
```

Thermal/dipole support:

```text
compute_dipole_moments()  [atom_thermostat.cpp]
  -> [electronegativity] QEq charge relaxation:
     -> transfer q_frac between bonded atoms toward higher Mulliken chi
  -> dipole moment from bond topology + electronegativity
     + induced polarization (alpha_pol * f_ionic proxy)

apply_thermostat()  [thermostat, target_temperature_ > 0]
  -> Berendsen velocity rescaling (lambda^2 clamped >= 0)

enforce_speed_limit()
  -> clamp free-atom velocity to C_SPEED

apply_damping()  [damping]
  -> velocity *= (1 - DAMPING * dt_)
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

detect_gw_events()  [gravitational_waves]
propagate_gw()      [gravitational_waves]
  -> cosmic_gravitational_waves.cpp
```

## 9. Cross-Scale Conversion

These are free functions in namespace `ftd` (declared in `engine/include/ftd/scale.h`,
defined in `engine/src/scale_bridge.cpp`). Each coarsen/refine function RETURNS a
vector of the target-scale type (or, for `refine_to_voxels`, mutates the
`RenderBridge` in place); the caller is responsible for feeding the returned
objects into the destination engine. They do not call the engines' `add_*`
methods themselves.

```text
scale_bridge.cpp::coarsen_to_particles(const RenderBridge& rb) -> std::vector<Particle>
  -> scan voxels with state != 0
  -> extract charge (= state), mass (= max(density(), K_B)), r_eff (= R_EFF_DEFAULT),
     position (coord + remainder), velocity, spin, color, pair_id, locked, particle_id
  -> push_back Particle into result vector (returned; caller adds to ParticleEngine)

scale_bridge.cpp::refine_to_voxels(const Particle& p, RenderBridge& rb) -> void
  -> floor + wrap p.position to integer lattice site (ix, iy, iz)
  -> rb.inject_wavepacket(ix, iy, iz, p.charge, 3.0, K_B)
  -> restore remainder/velocity/spin/color/pair_id/locked/particle_id on the voxel

coarsen_to_atoms(const ParticleEngine& pe) -> std::vector<Atom>
  -> cluster locked charge=+1 particles within CLUSTER_RADIUS (5.0) into nuclei (Z = count)
  -> count nearby charge=-1 particles as electrons (within 3*CLUSTER_RADIUS)
  -> compute_atomic_properties(Z, Z) -> mass/radius/vdW/max_bonds/valence
  -> push_back Atom into result vector (returned; caller adds to AtomEngine)

refine_to_particles(const Atom& a) -> std::vector<Particle>
  -> Z locked protons at a.position (mass = M_PROTON)
  -> (Z - charge) electrons ringed at a.radius (mass = K_B)
  -> push_back Particles into result vector (returned; caller adds to ParticleEngine)

coarsen_to_cosmic(const AtomEngine& ae) -> std::vector<CosmicBody>
  -> mass-weight atom centroid + sum masses into a single GAS body
  -> push_back CosmicBody into result vector (returned; caller adds to CosmicEngine)

refine_to_atoms(const CosmicBody& cb) -> std::vector<Atom>
  -> decompose cb.mass into hydrogen atoms (m_H = M_PROTON + K_B), capped at 1000
  -> distribute in a sphere of radius cb.radius
  -> push_back Atoms into result vector (returned; caller adds to AtomEngine)
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

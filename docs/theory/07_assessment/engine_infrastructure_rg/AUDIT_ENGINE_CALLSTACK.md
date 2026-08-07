# Engine Callstack Audit

**Scope:** `ftd::RenderBridge` — the production Scale-0 engine — and its GPU counterpart `ftd::gpu::GpuEngine`. Traces every call path from `tick()` to leaf functions; checks CPU/GPU parity; flags dead code, silent no-ops, and inconsistent naming.
**Companion:** [`TRACKER_OPEN_ITEMS.md`](core_ledgers/TRACKER_OPEN_ITEMS.md).

**STATUS:** all 10 findings RESOLVED. Re-verification at HEAD `b4f1dcf` walked every finding against live source; all CALLSTACK fixes confirmed in place. Per-finding RESOLVED annotations in §3 cite the exact `file:line` at HEAD where each fix lives. The verification test referenced in the original status line — `tests/test_callstack_audit_fixes.cpp` — was renamed/folded into `tests/test_audit_regression.cpp` during the engine refactor sweep; `test_callstack_audit_fixes.cpp` no longer exists.

## 1. CPU Tick Callstack (the reference path)

```
RenderBridge::tick()                             [src/render_bridge.cpp:1362]
│
├─ [GPU fork] if (use_gpu_) → gpu_->tick(); gpu_sync_to_host(); update_energy_ledger(); return;
│
├─ toggles.validate(&err)                        → std::cerr on violation
│
├─ phase_read()                                  [gated: wave_propagation || coupling]
│   └─ foreach voxel (#pragma omp parallel):
│       ├─ [interior fast path] 18-point Moore Laplacian via precomputed offsets
│       ├─ [boundary slow path] laplacian_flux(i) / laplacian_impl<&Voxel::flux_L>(i)
│       ├─ if coupling: gradient_state(i) + curl_state_velocity(i) · G_C
│       └─ writes delta_j_[i] (and _L_/_R_ when dual_substrate)
│
├─ phase_write()                                 [always runs; internal toggles gate sub-effects]
│   ├─ [selective_damping] near_particle_ mask + near_accel_ accumulation
│   ├─ stateless per-voxel RNG: voxel_uniform(seed, i, tick, salt) SplitMix64
│   │   (BH-F5/F8/F9 2026-05-05; the per-thread mt19937 this line originally
│   │   documented was removed)
│   └─ foreach voxel:
│       ├─ wave_vel += delta_j  (leapfrog half-step)
│       ├─ flux     += wave_vel (leapfrog drift)
│       ├─ if damping: flux *= (1 − DAMPING)^dt
│       │   └─ [larmor_radiation] modulate damping by accel²
│       ├─ if genesis AND |J|>K_GENESIS AND s==0:
│       │   ├─ state = sign(chi or divJ)
│       │   ├─ spin from curl(J) dominant axis
│       │   └─ color from |J| dominant axis
│       └─ evaporation (stochastic since 15882e98 2026-04-23; this line
│           originally documented the retired deterministic threshold):
│           p = exp(−E_7site/K_MANIFEST²) · K_EVAP_RATE, voxel_uniform
│           draw → s=0 (locked voxels exempt)
│
├─ gauss_project()                               [gated: gauss_projection]
│   └─ SOR_ITERATIONS × sor_sweep_18pt(phi_, sor_source_, lattice_, OMEGA)
│       └─ red-black parity → OMP-parallel interior + modular boundary
│
├─ self_field_injection_ = 0.0                   [DEAD WRITE — see finding F1]
│
├─ solve_latency_poisson()                       [gated: latency_field]
│   ├─ build sor_source_ from |J|² density
│   ├─ SOR_ITERATIONS × sor_sweep_18pt(phi_latency_, sor_source_, lattice_, OMEGA)
│   ├─ subtract mean (gauge fix)
│   └─ foreach voxel: v.latency = sqrt(clamp(|phi_latency|, 0, 0.998))
│
├─ phase_forces()                                [gated: forces]
│   ├─ if poisson_coulomb && !emergent_forces: solve_coulomb_poisson()
│   │   └─ SOR_ITERATIONS × sor_sweep_18pt(phi_coulomb_, sor_source_, lattice_, OMEGA)
│   ├─ if color_forces: scan for coloured manifested particles → colored_sites_cache_
│   └─ foreach manifested voxel:
│       ├─ EM force (3 modes: emergent / poisson / legacy-gradient)
│       ├─ gravity: tier-2 gradient of ρ × G_N
│       ├─ lorentz: α_EFT · s · (v × curl(J))
│       ├─ color: pairwise three-regime (Coulomb / flux-tube / linear)
│       ├─ f_total = f_em + f_grav + f_lorentz + f_color
│       ├─ store force_diag_[i] {f_coulomb, f_strong, f_magnetic, f_gravity, f_exchange}
│       ├─ v.accel_mag = |f_total|
│       └─ [γ_FTD momentum integration, 2026-04-17]
│           ├─ γ_in = 1/√(1 − |v|²/C² − L²)
│           ├─ p = γ_in · v + f_total · dt
│           └─ v = p · C · √((1−L²) / (C² + |p|²))
│
├─ phase_movement()                              [gated: movement]
│   ├─ fill(moved_, 0)
│   └─ foreach unlocked manifested voxel (not yet moved):
│       ├─ remainder += v · dt
│       ├─ if |remainder_axis| ≥ 1: dx/dy/dz = ±1, remainder −= sign
│       ├─ collision resolution:
│       │   ├─ target void → move in (carry self-field + particle_id)
│       │   ├─ target same sign → bounce (velocity → 0 for both)
│       │   └─ target opposite sign → annihilate (state=0, flux burst to 6 neighbours)
│       └─ set moved_[i] = 1
│
├─ weak_transmutation loop (inline)              [gated: weak_transmutation]
│   └─ foreach manifested voxel:
│       ├─ stress = dual ? compute_stress_left(i) : compute_stress(i)
│       └─ if stress > WEAK_THRESHOLD AND rng < 1−exp(−…/K_B):
│           ├─ v.state *= −1
│           └─ [dual] swap flux_L  flux_R, wave_vel_L  wave_vel_R
│
├─ proper-time loop (inline)                     [gated: latency_field]
│   └─ foreach manifested voxel:
│       └─ v.tau += √(f² − |v|²) / √f  where f = 1 − L²
│
├─ physical_time_ += dt_; ++tick_
│
└─ update_energy_ledger()                        [populates EnergyLedger]
    └─ foreach voxel: sum E_field (|J|²), E_wave (|wv|²), E_kin (½|v|² for manifested)
    └─ compute drift_frac, residual, cumulative_injection, cumulative_dissipation
```

## 2. GPU Tick Callstack

```
GpuEngine::tick()                                [cuda/gpu_engine.cu:131]
│
├─ gpu_phase_read()
│   └─ launch_phase_read() or launch_phase_read_dual()
│
├─ gpu_phase_write()
│   └─ launch_phase_write() or launch_phase_write_dual()
│
├─ gpu_pair_production()                         [gated: pair_production]  ★ CPU has NO equivalent
│   └─ launch_pair_production()
│
├─ gpu_gauss_project()                           [gated: gauss_projection]
│   ├─ FFT-based Poisson (NOT the CPU SOR)
│   └─ if dual_substrate: launch_gauss_sync_dual()
│
├─ gpu_solve_latency()                           [gated: latency_field]
│   └─ launch_solve_latency() + mean-subtract
│   ★ No proper-time accumulation here (CPU does it inline)
│
├─ gpu_phase_forces()                            [gated: forces]
│   └─ launch_phase_forces(poisson_coulomb, ...)
│
├─ gpu_build_particle_list()                     [gated: color||strong||exchange||triad]
│   └─ launch_build_particle_list()
│
├─ gpu_particle_forces()                         [same gate]
│   ├─ launch_color_force() when color_forces
│   ├─ launch_strong_force() when strong_force    ★ CPU has NO equivalent
│   └─ launch_exchange_force() when exchange_force ★ CPU has NO equivalent
│
├─ gpu_triad_detection()                         [gated: triad_binding]  ★ CPU has NO equivalent
│   └─ launch_triad_detection()
│
├─ gpu_phase_movement()                          [gated: movement]
│   └─ launch_phase_movement(dt)
│   ★ Does this use γ_FTD momentum? Unverified — audit note below
│
├─ gpu_weak_transmutation()                      [gated: weak_transmutation]
│   └─ launch_weak_transmutation()
│
├─ tick_++; host_dirty_ = true
│
└─ (RenderBridge wrapper: gpu_sync_to_host() + update_energy_ledger())
```

## 3. Findings

### F1 · `self_field_injection_ = 0.0` is a dead write (minor cleanup)

**Location:** `src/render_bridge.cpp:1422`.

The self-field floor was removed in Phase 4 (Energy Conservation). The member `self_field_injection_` is still written (always 0), read by `energy_audit()` to populate `EnergyAudit::self_field_injection`. Nothing else writes a non-zero value, so the reset is a no-op — but the member is API surface.

**Severity:** Low. API stability argues for keeping the read path; remove the tick()-level `= 0.0` reset (it's redundant — never becomes non-zero anywhere else).

**Action:** Remove the assignment; keep the member default-initialised to 0 so `energy_audit()` still returns a consistent struct.

**RESOLVED at `b4f1dcf`:** the per-tick reset is gone. `engine/src/render_bridge.cpp:414-417` documents the removal inline ("Rule 3b: Self-field floor REMOVED (Phase 4 — Energy Conservation)"). The member `self_field_injection_` is still default-initialised to 0 at `engine/include/ftd/render_bridge.h:387` and read by `energy_audit()` at `engine/src/render_bridge.cpp:514`, exactly as the action prescribed. No write path produces a non-zero value anywhere in the engine.

### F2 · Four toggles are silently no-op on CPU (real correctness gap)

**Toggles in `TermToggles` with GPU-only implementation:**

| Toggle | CPU implementation? | GPU implementation |
|---|---|---|
| `pair_production` |  none | `gpu_pair_production()` at phase 2b |
| `strong_force` |  none | `launch_strong_force()` in `gpu_particle_forces` |
| `exchange_force` |  none | `launch_exchange_force()` in `gpu_particle_forces` |
| `triad_binding` |  none | `gpu_triad_detection()` at phase 4c |

If a user flips any of these on in CPU mode, the engine runs as if they were off — silently. No warning, no error. **This is the highest-severity finding in this audit.**

**Severity:** Medium-to-high. Only matters for users who enable these toggles in CPU mode, but they have no way to know the toggle is ignored.

**Action options:**
1. **Runtime warning**: in `toggles.validate()`, emit a diagnostic when a CPU-only run sets any of these. Cheapest fix; zero correctness risk.
2. **Implement on CPU**: port the GPU kernels back. Not all four are equally cheap. `pair_production` and `triad_binding` are small loops; `strong_force` and `exchange_force` have non-trivial kernels.
3. **Remove from `TermToggles`**: make them GPU-only by compile guard. Breaks API for anyone setting them defensively.

**Recommendation:** Option 1 now (5 minutes), Option 2 later per-toggle when a benchmark needs them in CPU mode.

**RESOLVED at `b4f1dcf`:** Option 1 (warning) **AND** Option 2 (CPU port) both landed:

- **CPU ports** for `pair_production` and `triad_binding` shipped: `engine/src/render_bridge.cpp:407-408` gates `pair_production_cpu()` and `:445-446` gates `triad_binding_cpu()`; thin dispatchers at `:471-472` route into `engine/src/render_bridge_phases/transmutation_phases.cpp` implementations.
- **Runtime warnings** for the still-GPU-only toggles `strong_force` and `exchange_force` are emitted by `cpu_runtime_warnings()` (declared in `engine/include/ftd/term_toggles.h:242`) and printed by `engine/src/render_bridge.cpp:386-395` once per RenderBridge instance via the `cpu_warnings_emitted_` flag (`engine/include/ftd/render_bridge.h:367`). Per-toggle warning strings live in the `TOGGLE_SPECS[]` table at `term_toggles.h:135-138`.

The "highest-severity finding in this audit" line in the original entry no longer applies: both pair_production and triad_binding have CPU implementations, and the remaining GPU-only toggles surface a discoverable warning instead of silently no-op-ing.

### F3 · GPU path skips `toggles.validate()` (consistency gap)

**Location:** `src/render_bridge.cpp:1391–1396` — validate block is inside the non-GPU branch.

Adding `validate()` to the GPU branch is a one-line add and would catch the F2 class of misconfiguration in GPU mode too (currently only CPU reports invalid combos).

**Severity:** Low. The GPU forwarded-toggles path assigns `gpu_->toggles = toggles` but never validates.

**Action:** Move `validate()` before the GPU fork.

**RESOLVED at `b4f1dcf`:** `engine/src/render_bridge.cpp:345-373` runs `toggles.validate()` **before** the GPU fork (`:378`). Comment at `:345-346` cites the audit by ID. ARCH-3 hardening on top: violations under `strict_validation` raise `std::logic_error` (or abort under WASM `-fno-exceptions`); otherwise warnings are deduped via `last_validation_warn_` so tests don't spam stderr.

### F4 · Proper-time accumulation runs only on CPU

**Location:** `src/render_bridge.cpp:1486–1502` — `if (toggles.latency_field) { … v.tau += … }`. No GPU counterpart.

If `toggles.latency_field` is on in GPU mode, the latency field `v.latency` IS computed (via `gpu_solve_latency()`) but the per-particle `v.tau` is never accumulated. Benchmarks that read `voxel.tau` post-GPU-tick get zero.

**Severity:** Medium. Affects only GR-sector benchmarks using the GPU path.

**Action:** Add `launch_proper_time_update()` kernel + `gpu_proper_time()` wrapper, call it after `gpu_solve_latency()`. Or, document explicitly that proper-time accumulation is CPU-only (simpler now, kernel later).

**RESOLVED at `b4f1dcf`:** The simpler path was taken: GPU tick syncs voxels back to host, then `accumulate_proper_time()` runs host-side at `engine/src/render_bridge.cpp:380-381` immediately after `backend_->tick()` returns. Implementation at `engine/src/render_bridge_phases/transmutation_phases.cpp:40-55` operates on the synced voxel array, gated on `toggles.latency_field`. No GPU kernel needed; `voxel.tau` is correctly accumulated in both backends. (If a perf-sensitive workload ever needs to skip the sync, the kernel option remains available — but no current benchmark requires it.)

### F5 · CPU inlines weak_transmutation / proper_time — no extraction

**Location:** `src/render_bridge.cpp:1448–1474` (weak), `:1486–1502` (tau).

The GPU path has dedicated methods (`gpu_weak_transmutation`, and no tau at all). The CPU path has raw loops inside `tick()`. This makes tests harder to write (can't call the CPU logic in isolation) and hides the algorithm inside the tick narrative.

**Severity:** Low (maintainability).

**Action:** Extract to `RenderBridge::weak_transmutation()` and `RenderBridge::accumulate_proper_time()` private methods. Code move only, no behaviour change.

**RESOLVED at `b4f1dcf`:** Both loops extracted in the R2 refactor. `engine/src/render_bridge.cpp:438-451` calls `weak_transmutation_cpu()` and `accumulate_proper_time()` as gated helpers; bodies live at `engine/src/render_bridge_phases/transmutation_phases.cpp:15-38` (weak) and `:40-55` (proper-time). Both loops are now independently testable; tests can call them via the bridge's private-method dispatchers at `engine/src/render_bridge.cpp:471-472` (or via the free functions in `transmutation_phases.cpp`). Comments cite the audit by ID at the call sites.

### F6 · Two different Poisson solvers silently coexist

- CPU: **SOR** (iterative, `sor_sweep_18pt`) for gauss_project, solve_coulomb_poisson, solve_latency_poisson.
- GPU: **FFT** (`cufft`) for gauss_project + latency.

This means CPU and GPU produce numerically different constraint-residual profiles at any given tick (SOR converges iteratively to a residual; FFT is exact up to rounding). Benchmarks comparing CPU vs GPU output must account for this ≤10⁻⁴ divergence.

**Severity:** Low — documented implicitly in existing comments but not explicitly in SPEC_ENGINE.md.

**Action:** Add an SPEC_ENGINE.md note under "GPU Acceleration" about the solver difference.

**RESOLVED at `b4f1dcf`:** `engine/SPEC_ENGINE.md:937-946` has the "FFT Poisson Solver" section explicitly noting "Replaces CPU's iterative SOR with spectral method via cuFFT" with the convergence-residual contrast ("Exact: Gauss violation = 0.0 (vs CPU SOR ~ 1.14)"). Lines 940-946 spell out: "solve the SAME Poisson equation but with different numerical methods (SOR iterative vs FFT spectral). CPU output carries a residual ≤ 10⁻⁴". Header file commentary at `engine/include/ftd/poisson_solvers.h:13` provides the same info from the source side. The solver-divergence convention is now first-class documentation, not implicit.

### F7 · Naming inconsistency: `solve_latency_poisson` vs `gpu_solve_latency`

CPU: `solve_latency_poisson()`. GPU: `gpu_solve_latency()`. Elsewhere the pattern is `phase_read`  `gpu_phase_read`, so the GPU side should be `gpu_solve_latency_poisson()` for consistency.

**Severity:** Trivial.

**Action:** Rename when convenient. Non-blocking.

**RESOLVED at `b4f1dcf`:** GPU method is now `gpu_solve_latency_poisson()`; declaration at `engine/include/ftd/gpu_engine.h:93` carries the rationale comment ("Wave 5: GPU latency Poisson. Renamed from gpu_solve_latency (F7 callstack audit 2026-04-17) for parity with CPU solve_latency_poisson"). CPU pair lives at `engine/src/render_bridge.cpp:280` as `RenderBridge::solve_latency_poisson()`. Naming follows the project's `phase_X`  `gpu_phase_X` convention.

### F8 · `ALPHA_EFT` vs `ALPHA` used inconsistently in phase_forces

**Location:** `phase_forces()` has three EM-force modes:
- Emergent (`emergent_forces`): `G_C * v.state * ∇|J|` — uses `G_C`, not `ALPHA_EFT`.
- Poisson (`poisson_coulomb`): `−ALPHA_EFT * v.state * ∇φ` — uses `ALPHA_EFT`.
- Legacy-gradient (fallback): `−ALPHA_EFT * v.state * ∇(∇·J)` — uses `ALPHA_EFT`.

`ALPHA_EFT = G_C² = ALPHA` (all three are numerically equal after the precision rollout). The mixing is cosmetic.

**Severity:** Trivial; semantic clarity only.

**Action:** Pick one constant (prefer `ALPHA`) and use it throughout. The `ALPHA_EFT` alias stays available for pedagogy.

**RESOLVED at `b4f1dcf`:** All EM-force modes in `engine/src/render_bridge_phases/phase_forces.cpp` now use bare `ALPHA` per the action's "prefer ALPHA" guidance: line 102 (poisson_coulomb mode), line 105 (gauss-constraint mode), line 134 (lorentz). The static_assert in `engine/include/ftd/constants.h` guarantees `ALPHA == ALPHA_EFT == G_C²` to 1e-8, so the choice is purely about source-code clarity. The lingering `// ALPHA == ALPHA_EFT (G_C² identity)` reminder comments at those three sites are vestigial; cleanup of those comments will land in a follow-up commit (CS-F8 cosmetic finalisation).

### F9 · `DAMPING` does three jobs

Documented in `ontic.h:771` with `[IMPOSED]` tag, also in the "honest" audit. Not a bug — but callers should know:
1. Physical dissipation.
2. Stability margin for the leapfrog at CFL = 1/√3.
3. Evaporation drag.

Setting `damping = false` disables all three, which mixes concerns. Energy-conservation tests use this; so they measure conservation + stability + evaporation all together.

**Severity:** Low (documented, not broken).

**Action:** None in code. Keep the comment in `ontic.h:771` clear.

**RESOLVED-AS-DOCUMENTED at `b4f1dcf`:** No code change required by the audit. `engine/include/ftd/ontic.h:771` still carries the `[IMPOSED]`-tagged comment explaining DAMPING's three roles (physical dissipation / leapfrog stability margin at CFL = 1/√3 / evaporation drag). Tests that flip `damping = false` (notably the energy-conservation suite) implicitly measure all three concerns together; this is a documented limitation of the conservation-smoke-test methodology, not a bug.

### F10 · EnergyLedger reports drift_frac using L² pseudo-Hamiltonian, not true H

**Location:** `src/render_bridge.cpp:1480+` in `update_energy_ledger()`.

`E_total = ½(Σ|J|² + Σ|v|²) + E_kin` is an L²-indicator, not the true conserved Hamiltonian (which involves `|∇J|²`). This is documented in the leapfrog-audit test comment — the CUMULATIVE balance stays correct (injection ≈ dissipation) but per-tick `residual` looks large during wavefront sloshing.

**Severity:** Low (documented). Tests assert on cumulative balance, not per-tick residual.

**Action:** Leave as-is. Upgrading to the true discrete Hamiltonian would require computing `|∇J|²` over the whole lattice every tick — unnecessary cost for what's essentially a conservation smoke test.

**RESOLVED-AS-DOCUMENTED at `b4f1dcf`:** No code change required by the audit. `update_energy_ledger()` still uses the L²-indicator `E_total = ½(Σ|J|² + Σ|v|²) + E_kin`. `test_leapfrog_integrator_audit` and the broader energy-conservation suite assert on cumulative injection/dissipation balance (which is correct), not per-tick residual (which can spike during wavefront sloshing). Cost-benefit for upgrading to the true discrete Hamiltonian remains unfavourable.

## 4. CPU-only toggles vs TermToggles declarations

Full cross-reference:

| Toggle | CPU | GPU | Tests |
|---|---|---|---|
| `wave_propagation` |  |  | constants, wavepacket, continuity |
| `coupling` |  |  | continuity |
| `damping` |  |  | dissipation |
| `genesis` |  |  | bridge_dynamics |
| `gauss_projection` |  (SOR) |  (FFT) | gauss, em_energy_conservation |
| `forces` |  |  | coulomb, gravity |
| `gravity` |  |  | gravity_attraction |
| `poisson_coulomb` |  |  | coulomb_isotropy |
| `movement` |  |  | wavepacket, gamma_ftd_momentum |
| `lorentz_force` |  |  | campaign_lorentz_measure |
| `selective_damping` |  |  | (no dedicated test — covered by em_energy_conservation) |
| `larmor_radiation` |  |  (?) | campaign_larmor |
| `dual_substrate` |  |  | dual_substrate |
| `weak_transmutation` |  (inline) |  | (no dedicated test) |
| `latency_field` |  |  (no proper_time) | test_einstein_equations |
| `emergent_forces` |  | ? | benchmark_emergent_alpha |
| **`color_forces`** | ** (inline in phase_forces)** | ** (separate particle_forces)** | GP-COLOR |
| **`pair_production`** | ** (F2)** |  | — |
| **`strong_force`** | ** (F2)** |  | GP-STRONG |
| **`exchange_force`** | ** (F2)** |  | GP-EXCHANGE |
| **`triad_binding`** | ** (F2)** |  | — |

## 5. Diagnostics / Data Access Paths

```
Diagnostics   diagnostics()      [host loop over voxels_; GPU-syncs if needed]
EnergyAudit   energy_audit()     [same shape as diagnostics; computes per-field sums + Gauss violation]
EnergyLedger  energy_ledger()    [struct-level accessor; populated by update_energy_ledger()]
ForceDiag     force_diag(idx)    [per-particle slice; written in phase_forces]
EMFieldDiag   em_field_at(idx)   [E = -wave_vel, B = curl(flux); computed on demand]
Vec3          poynting_vector(idx) [E × B]
AggregateProfile aggregate_profile(center, thresh)  [radial profile, CoM, r_eff]
```

**Gaps:**
- `ForceDiag::f_weak` field doesn't exist (weak transmutation isn't a force).
- `ForceDiag::f_exchange` is always zero on CPU (F2 — exchange_force is GPU-only).
- `EMFieldDiag::E` uses `-wave_vel` which is strictly correct as ∂J/∂t only at half-step (leapfrog convention). Users reading E at integer-tick should know this is `v(t−h/2)` averaged with `v(t+h/2)` implicitly.

## 6. Injection API call paths

All are on `RenderBridge`; each performs:

```
inject_flux(x,y,z, Vec3)
  └─ voxels_[idx].flux += val  [and optionally flux_L / flux_R in dual mode]
  └─ (GPU) push_to_device() → marks gpu_dirty_

inject_particle(x,y,z, state, flux, spin, color)
  └─ voxels_[idx].{state,flux,spin,color} set
  └─ particle_id auto-incremented from next_particle_id_

inject_wavepacket(cx,cy,cz, state, sigma, amplitude)
  └─ Gaussian envelope injection over a radius-3σ box
  └─ also injects flux (no wave_vel)

create_entangled_pair(cx,cy,cz, dx,dy,dz)
  └─ calls inject_particle twice with shared pair_id
```

All four correctly push to GPU via `gpu_push_to_device()` / `host_mutated_` flag when `use_gpu_`. No gaps.

## 7. Summary + Action Priority

**Status:** all 10 findings closed. The priority table below reflects the status at HEAD `b4f1dcf`.

| # | Finding | Severity | Effort | Priority | Status @ HEAD |
|---|---|---|---|---|---|
| F2 | 4 toggles silently no-op on CPU | Medium-high | Low (warning) / Medium (impl) | **High** — Option 1 ASAP |  pair_production + triad_binding ported to CPU; strong_force + exchange_force surface a one-shot warning |
| F4 | Proper-time accumulation is CPU-only | Medium | Medium | Mid |  host-side `accumulate_proper_time()` runs after GPU sync |
| F8 | ALPHA/ALPHA_EFT cosmetic mixing | Trivial | Low | Low |  standardised on `ALPHA` in EM-force paths (vestigial comments queued for cosmetic cleanup) |
| F5 | Inline loops in tick() not extracted | Low | Low | Low (cleanup) |  extracted to `transmutation_phases.cpp` |
| F3 | GPU path skips validate() | Low | Trivial | Low |  validate runs before GPU fork |
| F6 | Two Poisson solvers documented implicitly | Low | Low (docs) | Low |  explicit SPEC_ENGINE.md §"FFT Poisson Solver" |
| F7 | Naming inconsistency: `solve_latency_poisson` vs `gpu_solve_latency` | Trivial | Trivial | Low |  GPU renamed to `gpu_solve_latency_poisson` |
| F1 | `self_field_injection_ = 0.0` is a dead write | Trivial | Trivial | Trivial |  assignment removed; member retained for `energy_audit()` API |
| F9 | DAMPING does three jobs (documented) | Documented | — | None |  no-action by design |
| F10 | EnergyLedger uses L² pseudo-H (documented) | Documented | — | None |  no-action by design |

**Open follow-ups (not in this audit):**

The CALLSTACK audit is closed. Remaining engine cleanup is tracked under the **bug-hunt** numbering (commit `f2a721a`): F2 γ_FTD GPU port, F3 accel_mag unification, F5 evaporation Boltzmann RNG, F8/F9 spin/RNG portability, F12 emergent_forces GPU, F15 dead phase_movement_kernel parameters. See [STATUS_2026-05-04_post_bughunt.md](archive/STATUS_2026-05-04_post_bughunt.md) for that tracker. Do **not** confuse the two F-numbering schemes — see that doc's §"Two F-numbering schemes" for disambiguation.

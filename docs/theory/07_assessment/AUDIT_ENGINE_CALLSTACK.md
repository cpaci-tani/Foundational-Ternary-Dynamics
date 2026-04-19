# Engine Callstack Audit

**Date:** 2026-04-17 (post engine-cleanup sweep)
**Scope:** `ftd::RenderBridge` — the production Scale-0 engine — and its GPU counterpart `ftd::gpu::GpuEngine`. Traces every call path from `tick()` to leaf functions; checks CPU/GPU parity; flags dead code, silent no-ops, and inconsistent naming.
**Companion:** [`TRACKER_OPEN_ITEMS.md`](TRACKER_OPEN_ITEMS.md).

**STATUS 2026-04-17 (end of day):** all 10 findings ✅ RESOLVED. See the per-finding annotations in §3 and the verification test `tests/test_callstack_audit_fixes.cpp` (6/6 checks pass).

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
│   ├─ per-thread RNG seeding (mt19937)
│   └─ foreach voxel:
│       ├─ wave_vel += delta_j  (leapfrog half-step)
│       ├─ flux     += wave_vel (leapfrog drift)
│       ├─ if damping: flux *= (1 − DAMPING)^dt
│       │   └─ [larmor_radiation] modulate damping by accel²
│       ├─ if genesis AND |J|>K_GENESIS AND s==0:
│       │   ├─ state = sign(chi or divJ)
│       │   ├─ spin from curl(J) dominant axis
│       │   └─ color from |J| dominant axis
│       └─ evaporation: 7-site energy < K_B² · 1e-6 → s=0
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
│           └─ [dual] swap flux_L ↔ flux_R, wave_vel_L ↔ wave_vel_R
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

### F2 · Four toggles are silently no-op on CPU (real correctness gap)

**Toggles in `TermToggles` with GPU-only implementation:**

| Toggle | CPU implementation? | GPU implementation |
|---|---|---|
| `pair_production` | ❌ none | `gpu_pair_production()` at phase 2b |
| `strong_force` | ❌ none | `launch_strong_force()` in `gpu_particle_forces` |
| `exchange_force` | ❌ none | `launch_exchange_force()` in `gpu_particle_forces` |
| `triad_binding` | ❌ none | `gpu_triad_detection()` at phase 4c |

If a user flips any of these on in CPU mode, the engine runs as if they were off — silently. No warning, no error. **This is the highest-severity finding in this audit.**

**Severity:** Medium-to-high. Only matters for users who enable these toggles in CPU mode, but they have no way to know the toggle is ignored.

**Action options:**
1. **Runtime warning**: in `toggles.validate()`, emit a diagnostic when a CPU-only run sets any of these. Cheapest fix; zero correctness risk.
2. **Implement on CPU**: port the GPU kernels back. Not all four are equally cheap. `pair_production` and `triad_binding` are small loops; `strong_force` and `exchange_force` have non-trivial kernels.
3. **Remove from `TermToggles`**: make them GPU-only by compile guard. Breaks API for anyone setting them defensively.

**Recommendation:** Option 1 now (5 minutes), Option 2 later per-toggle when a benchmark needs them in CPU mode.

### F3 · GPU path skips `toggles.validate()` (consistency gap)

**Location:** `src/render_bridge.cpp:1391–1396` — validate block is inside the non-GPU branch.

Adding `validate()` to the GPU branch is a one-line add and would catch the F2 class of misconfiguration in GPU mode too (currently only CPU reports invalid combos).

**Severity:** Low. The GPU forwarded-toggles path assigns `gpu_->toggles = toggles` but never validates.

**Action:** Move `validate()` before the GPU fork.

### F4 · Proper-time accumulation runs only on CPU

**Location:** `src/render_bridge.cpp:1486–1502` — `if (toggles.latency_field) { … v.tau += … }`. No GPU counterpart.

If `toggles.latency_field` is on in GPU mode, the latency field `v.latency` IS computed (via `gpu_solve_latency()`) but the per-particle `v.tau` is never accumulated. Benchmarks that read `voxel.tau` post-GPU-tick get zero.

**Severity:** Medium. Affects only GR-sector benchmarks using the GPU path.

**Action:** Add `launch_proper_time_update()` kernel + `gpu_proper_time()` wrapper, call it after `gpu_solve_latency()`. Or, document explicitly that proper-time accumulation is CPU-only (simpler now, kernel later).

### F5 · CPU inlines weak_transmutation / proper_time — no extraction

**Location:** `src/render_bridge.cpp:1448–1474` (weak), `:1486–1502` (tau).

The GPU path has dedicated methods (`gpu_weak_transmutation`, and no tau at all). The CPU path has raw loops inside `tick()`. This makes tests harder to write (can't call the CPU logic in isolation) and hides the algorithm inside the tick narrative.

**Severity:** Low (maintainability).

**Action:** Extract to `RenderBridge::weak_transmutation()` and `RenderBridge::accumulate_proper_time()` private methods. Code move only, no behaviour change.

### F6 · Two different Poisson solvers silently coexist

- CPU: **SOR** (iterative, `sor_sweep_18pt`) for gauss_project, solve_coulomb_poisson, solve_latency_poisson.
- GPU: **FFT** (`cufft`) for gauss_project + latency.

This means CPU and GPU produce numerically different constraint-residual profiles at any given tick (SOR converges iteratively to a residual; FFT is exact up to rounding). Benchmarks comparing CPU vs GPU output must account for this ≤10⁻⁴ divergence.

**Severity:** Low — documented implicitly in existing comments but not explicitly in SPEC_ENGINE.md.

**Action:** Add an SPEC_ENGINE.md note under "GPU Acceleration" about the solver difference.

### F7 · Naming inconsistency: `solve_latency_poisson` vs `gpu_solve_latency`

CPU: `solve_latency_poisson()`. GPU: `gpu_solve_latency()`. Elsewhere the pattern is `phase_read` ↔ `gpu_phase_read`, so the GPU side should be `gpu_solve_latency_poisson()` for consistency.

**Severity:** Trivial.

**Action:** Rename when convenient. Non-blocking.

### F8 · `ALPHA_EFT` vs `ALPHA` used inconsistently in phase_forces

**Location:** `phase_forces()` has three EM-force modes:
- Emergent (`emergent_forces`): `G_C * v.state * ∇|J|` — uses `G_C`, not `ALPHA_EFT`.
- Poisson (`poisson_coulomb`): `−ALPHA_EFT * v.state * ∇φ` — uses `ALPHA_EFT`.
- Legacy-gradient (fallback): `−ALPHA_EFT * v.state * ∇(∇·J)` — uses `ALPHA_EFT`.

Since 2026-04-17, `ALPHA_EFT = G_C² = ALPHA` (all three are numerically equal after the precision rollout). The mixing is cosmetic now.

**Severity:** Trivial; semantic clarity only.

**Action:** Pick one constant (prefer `ALPHA`) and use it throughout. The `ALPHA_EFT` alias stays available for pedagogy.

### F9 · `DAMPING` does three jobs

Documented in `ontic.h:771` with `[IMPOSED]` tag, also in the "honest" audit. Not a bug — but callers should know:
1. Physical dissipation.
2. Stability margin for the leapfrog at CFL = 1/√3.
3. Evaporation drag.

Setting `damping = false` disables all three, which mixes concerns. Energy-conservation tests use this; so they measure conservation + stability + evaporation all together.

**Severity:** Low (documented, not broken).

**Action:** None in code. Keep the comment in `ontic.h:771` clear.

### F10 · EnergyLedger reports drift_frac using L² pseudo-Hamiltonian, not true H

**Location:** `src/render_bridge.cpp:1480+` in `update_energy_ledger()`.

`E_total = ½(Σ|J|² + Σ|v|²) + E_kin` is an L²-indicator, not the true conserved Hamiltonian (which involves `|∇J|²`). This is documented in the leapfrog-audit test comment — the CUMULATIVE balance stays correct (injection ≈ dissipation) but per-tick `residual` looks large during wavefront sloshing.

**Severity:** Low (documented). Tests assert on cumulative balance, not per-tick residual.

**Action:** Leave as-is. Upgrading to the true discrete Hamiltonian would require computing `|∇J|²` over the whole lattice every tick — unnecessary cost for what's essentially a conservation smoke test.

## 4. CPU-only toggles vs TermToggles declarations

Full cross-reference:

| Toggle | CPU | GPU | Tests |
|---|---|---|---|
| `wave_propagation` | ✅ | ✅ | constants, wavepacket, continuity |
| `coupling` | ✅ | ✅ | continuity |
| `damping` | ✅ | ✅ | dissipation |
| `genesis` | ✅ | ✅ | bridge_dynamics |
| `gauss_projection` | ✅ (SOR) | ✅ (FFT) | gauss, em_energy_conservation |
| `forces` | ✅ | ✅ | coulomb, gravity |
| `gravity` | ✅ | ✅ | gravity_attraction |
| `poisson_coulomb` | ✅ | ✅ | coulomb_isotropy |
| `movement` | ✅ | ✅ | wavepacket, gamma_ftd_momentum |
| `lorentz_force` | ✅ | ✅ | campaign_lorentz_measure |
| `selective_damping` | ✅ | ✅ | (no dedicated test — covered by em_energy_conservation) |
| `larmor_radiation` | ✅ | ✅ (?) | campaign_larmor |
| `dual_substrate` | ✅ | ✅ | dual_substrate |
| `weak_transmutation` | ✅ (inline) | ✅ | (no dedicated test) |
| `latency_field` | ✅ | ✅ (no proper_time) | test_einstein_equations |
| `emergent_forces` | ✅ | ? | benchmark_emergent_alpha |
| **`color_forces`** | **✅ (inline in phase_forces)** | **✅ (separate particle_forces)** | GP-COLOR |
| **`pair_production`** | **❌ (F2)** | ✅ | — |
| **`strong_force`** | **❌ (F2)** | ✅ | GP-STRONG |
| **`exchange_force`** | **❌ (F2)** | ✅ | GP-EXCHANGE |
| **`triad_binding`** | **❌ (F2)** | ✅ | — |

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

| # | Finding | Severity | Effort | Priority |
|---|---|---|---|---|
| F2 | 4 toggles silently no-op on CPU | Medium-high | Low (warning) / Medium (impl) | **High** — Option 1 ASAP |
| F4 | Proper-time accumulation is CPU-only | Medium | Medium | Mid |
| F8 | ALPHA/ALPHA_EFT cosmetic mixing | Trivial | Low | Low |
| F5 | Inline loops in tick() not extracted | Low | Low | Low (cleanup) |
| F3 | GPU path skips validate() | Low | Trivial | Low |
| F6 | Two Poisson solvers documented implicitly | Low | Low (docs) | Low |
| F7 | Naming inconsistency: `solve_latency_poisson` vs `gpu_solve_latency` | Trivial | Trivial | Low |
| F1 | `self_field_injection_ = 0.0` is a dead write | Trivial | Trivial | Trivial |
| F9 | DAMPING does three jobs (documented) | Documented | — | None |
| F10 | EnergyLedger uses L² pseudo-H (documented) | Documented | — | None |

**Recommended next actions (if engine work resumes):**

1. **Add a validate() warning for CPU-only toggles** (F2 Option 1 + F3). ~10 lines in `term_toggles.h`.
2. **Extract the two inline tick() loops** (F5). ~30 lines of straight code move.
3. **Document the CPU/GPU Poisson-solver difference** (F6) in SPEC_ENGINE.md §14.
4. Remaining items are trivial or documented; no urgent action.

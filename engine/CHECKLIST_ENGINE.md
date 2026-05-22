# FTD Engine Quality Checklist (Living Document)

**Version:** 1.1 (2026-04-26)
**Scope:** Engineering health, constructor-domain formalization, bugs,
refactoring, test coverage, architecture.
**NOT in scope:** theorem promotions (see
[docs/theory/07_assessment/LEDGER.md](../docs/theory/07_assessment/LEDGER.md)).
Physics coverage is tracked in [CHECKLIST_PHYSICS.md](CHECKLIST_PHYSICS.md)
and the native EFT checklist; this file may track the engine infrastructure
needed to make those claims auditable.

---

## How to use

- Each ticket has an ID (`BUG-NNN`, `RF-NNN`, `TEST-NNN`, `ARCH-NNN`).
- Status: ☐ open, ◐ in-progress, ✅ done, ⊗ closed-won't-do (with reason).
- When a ticket closes, leave it in the doc with the date + commit/file refs.
  Don't delete — this is the living history of engine quality work.
- Add new tickets as the agent audits or as users find issues.
- Cross-reference: bug fixes → file:line, refactors → LOC delta, tests → test name.

---

## ROUND 1 — Correctness fixes (CLOSED 2026-04-25)

| ID | Status | Title | Where |
|---|---|---|---|
| BUG-001 | ✅ | `next_particle_id_++` race: `int` → `std::atomic<int>`, removed `omp critical(genesis_id)` | [render_bridge.h:401](include/ftd/render_bridge.h:401), [render_bridge.cpp:436,503](src/render_bridge.cpp:436) |
| BUG-002 | ✅ | GPU annihilation flux race: snapshot `flux[i]/[target]` to registers before scatter loop | [kernels_forces.cu:391-415](cuda/kernels_forces.cu:391) |
| BUG-003 | ✅ | GPU `run(N)` ledger divergence: dropped fast-path; CPU and GPU now identical per-tick ledger | [render_bridge.cpp:1091](src/render_bridge.cpp:1091) |
| BUG-004 | ⊗ | GPU Langevin per-tick re-seed | **FALSE POSITIVE** — guard at [gpu_engine.cu:138](cuda/gpu_engine.cu:138) correctly returns early when seed matches; cuRAND continues sequence as intended. Equipartition test passes (4%). |
| BUG-005 | ⊗ | Proper-time `arg = f² - v²` units mismatch | **FALSE POSITIVE** — `velocity` in engine convention has c=1 (see `gamma_ftd()` at [voxel.h:174](include/ftd/voxel.h:174)). `C_SPEED = 1/√3` is the wave-propagation CFL constant, separate. |
| BUG-006 | ⊗ | CPU/GPU `latency_field` phase ordering disagreement | **FALSE POSITIVE** — both paths run `accumulate_proper_time` → `update_energy_ledger` in same order. |

**Round 1 final:** 3 real bugs fixed (atomic ID, GPU annihilation race, GPU ledger divergence). 3 audit false-positives correctly identified.

### Round 1 — Constants drift (CLOSED 2026-04-25)

| ID | Status | Title | Where |
|---|---|---|---|
| CD-001 | ✅ | Removed stale-G* fallback (G_STAR=2.9586830685, 27 ppm drift) | [scripts/benchmarks/benchmark_engine_vs_theory.py:38](../scripts/benchmarks/benchmark_engine_vs_theory.py:38) |
| CD-002 | ✅ | Same fallback removed | [scripts/benchmarks/analyze_convergence.py:13](../scripts/benchmarks/analyze_convergence.py:13) |
| CD-003 | ✅ | alpha_inv 137.035999177 → 137.035999177 (CODATA-2022) in test fixtures | [scripts/tests/test_verify_manifest_builder.py](../scripts/tests/test_verify_manifest_builder.py) |
| CD-004 | ✅ | G* mistranscription 2.9586788 → 2.958675119188639 | [scripts/exploration/lattice_3x3x3_center.py:102](../scripts/exploration/lattice_3x3x3_center.py:102) |
| CD-005 | ✅ | Same mistranscription fixed | [scripts/exploration/five_minds_round5.py:274](../scripts/exploration/five_minds_round5.py:274) |
| CD-006 | ✅ | Tree-level alpha now imports from canonical | [scripts/proofs/gauss_constrained_green_v2.py:316](../scripts/proofs/gauss_constrained_green_v2.py:316) |
| CD-007 | ✅ | Truncated 5-digit G_STAR literal → canonical import | [scripts/visualization/viz_transfer_matrix.py:290](../scripts/visualization/viz_transfer_matrix.py:290) |
| CD-008 | ✅ | Truncated 4-digit G_STAR → canonical | [scripts/exploration/lattice_partition_L2.py:67](../scripts/exploration/lattice_partition_L2.py:67) |
| CD-009 | ✅ | Added `G_C` and `ALPHA_EFT` to canonical Python (matches C++ static_assert) | [scripts/constants.py](../scripts/constants.py) |

---

## ROUND 2 — Test coverage gaps (CLOSED 2026-04-25)

| ID | Status | Title | Where |
|---|---|---|---|
| TEST-001 | ✅ | Determinism regression (4/4 protocols, single-thread CPU) | [test_determinism.cpp](tests/test_determinism.cpp) |
| TEST-002 | ✅ | Closed-negative regression guards (FTD-0061/73 mode-erasure, FTD-0074 flux separable) | [test_closed_negatives.cpp](tests/test_closed_negatives.cpp) |
| TEST-003 | ✅ | CPU-only warning capture (strong_force, exchange_force) | [test_cpu_warnings.cpp](tests/test_cpu_warnings.cpp) |
| TEST-004 | ✅ | OMP multi-threaded determinism | **CLOSED 2026-04-25.** ARCH-7 fixed particle-ID race; ARCH-7b fixed genesis flux read/write race via pre-write snapshot. Determinism test 4/4 PASS at 32 threads (system default). |
| TEST-005 | ✅ | Toggle pairwise smoke matrix: 78 (i,j) pairs of 13 OFF-default toggles, 5 ticks each, no crash / no NaN / validator rejects cleanly. 78/78 pass. (done 2026-04-25, [test_toggle_matrix.cpp](tests/test_toggle_matrix.cpp)) |
| TEST-006 | ☐ | L-scaling sweeps in core physics tests (Coulomb/Wilson/Born) | Most tests pin a single L; bulk-leakage artifacts invisible. Add slope test across L ∈ {8,16,32,64}. |
| TEST-007 | ✅ | Symplectic-leapfrog conservation contract (corrected framing): no secular drift across two disjoint windows + bounded oscillation amplitude + endpoint drift < 50%. PASS at 5e-5 inter-window drift over 5000 ticks. (Original 1e-12 per-tick framing was wrong for second-order leapfrog — it conserves a shadow Hamiltonian, not strict total energy.) (done 2026-04-25, [test_energy_conservation_tight.cpp](tests/test_energy_conservation_tight.cpp)) |
| TEST-008 | ☐ | GPU/CPU same-seed Langevin trajectory parity | `test_langevin_equipartition` runs single L, single (γ,T), CPU only. Add (γ,T,L) sweep + CPU↔GPU comparison. |
| TEST-009 | ☐ | Phase-H `coulomb_charge_coupling` regression at 3 settings | New numeric knob (1.0 / 0.2141 / 0.3028); no test pins behaviour at non-default values. |

---

## ROUND 3 — Refactoring (PARTIAL 2026-04-25)

| ID | Status | Title | LOC delta | Where |
|---|---|---|---|---|
| RF-1 | ✅ (foundation) | `tests/test_helpers.h` with `check`, `check_close`, voxel inspectors, Counter | +130 (header) | [tests/test_helpers.h](tests/test_helpers.h) |
| RF-1b | ◐ | Mass-migrate 65 canonical-signature tests to test_helpers.h | −2,000 (target) | 1 reference migration done ([test_voxel_properties.cpp](tests/test_voxel_properties.cpp)). 64 to go. |
| RF-2 | ✅ | Toggle presets + `prepare_bridge` helpers in test_helpers.h | n/a | [tests/test_helpers.h](tests/test_helpers.h) |
| RF-3 | ✅ | Pair-force CUDA: extracted `decode_xyz_d`, `periodic_delta_d` | −16 LOC, **eliminates 4-way periodic-wrap bug-fix risk** | [kernels_forces.cu](cuda/kernels_forces.cu) — 4 callsites collapsed (color, yukawa, exchange, triad) |
| RF-4 | ✅ (partial) | Stencil: extracted `effective_damping`, `scale_field_pair` | −12 LOC, **single Larmor formula source of truth** | [kernels_stencil.cu](cuda/kernels_stencil.cu) — phase_write + phase_write_dual |
| RF-4b | ⊗ | Full template-collapse single/dual-substrate stencils | **DEFERRED** — 1434 LOC at risk, kernel signatures propagate; risk-to-LOC tradeoff bad. The Larmor extraction (RF-4) captured highest-value duplication. |
| RF-5 | ☐ | Split `cuda/kernels_stencil.cu` (1434 LOC) into `kernels_phase.cu` / `kernels_genesis.cu` / `kernels_transmutation.cu` | 0 net, faster nvcc incremental builds | After RF-4b decision; mechanical split. |
| RF-6 | ☐ | Split test files >500 LOC: `test_gpu_physics.cpp` (2618), `test_gpu_experiments.cpp` (1676), `campaign_dark_sector.cpp` (1762), `test_constructors.cpp` (1354) | 0 net, faster CTest parallelism | Mechanical split by section. |
| RF-7 | ☐ | Trim `render_bridge.h` public API surface (40+ methods) | −60 LOC header, faster TU rebuild for ~140 includers | Move 8 inline operator delegators to .cpp; PIMPL the RNG/Langevin internals. |
| RF-8 | ✅ | Added `BANDWIDTH_FLOOR` constant in `constants.h`; replaced 3 bare `1e-6` literals at [render_bridge.cpp:822,828](src/render_bridge.cpp). (done 2026-04-25) |
| RF-9 | ✅ | PIMPL'd RNG state via `BridgeRng` (forward-declared in `bridge_rng.h`, full impl in `bridge_rng.cpp`). `<random>` dropped from `render_bridge.h`; replaced 3 RNG members with `std::unique_ptr<BridgeRng>`. Migrated 9 call sites in render_bridge.cpp + 2 in transmutation_phases.cpp to PIMPL'd accessors (`sample_uniform`, `thread_uniform`, `thread_normal`, `reseed_thread_pool`). 1 test (`test_benchmark.cpp`) needed explicit `<random>`. RF-9 regression suite 8/8 PASS (determinism, strict_validation, cpu_warnings, toggle_matrix, closed_negatives, open5, phase_h_regression, eft_phase_h_coupling). CPU langevin parity 100%. (done 2026-04-25) | [bridge_rng.h](include/ftd/bridge_rng.h), [bridge_rng.cpp](src/bridge_rng.cpp), [render_bridge.h:14-29,479-484](include/ftd/render_bridge.h) |

---

## ROUND 4 — Architecture (PARTIAL)

| ID | Status | Title | Effort | Where |
|---|---|---|---|---|
| ARCH-1 | ✅ | Split `RenderBridge` god-class — Injector + RenderBridgeView extracted; delegators deprecated; 17/17 regression pass | done 2026-04-25 | [injector.h](include/ftd/injector.h), [bridge_view.h](include/ftd/bridge_view.h), [render_bridge.h](include/ftd/render_bridge.h) |
| ARCH-2 | ✅ | Backend abstraction — full migration. All ifdef blocks collapsed; `use_gpu_` flag deleted; 6 inject friends dropped; 18/18 regression pass | done 2026-04-25 | [backend.h](include/ftd/backend.h), [backend.cpp](src/backend.cpp), [render_bridge.h](include/ftd/render_bridge.h), [render_bridge.cpp](src/render_bridge.cpp) |
| ARCH-3 | ✅ | Toggle validator strictness: dedup repeated warnings (one per unique error string) + opt-in `strict_validation` toggle that throws std::logic_error. 3 contract tests added (test_strict_validation). | done 2026-04-25 | [render_bridge.cpp:tick()](src/render_bridge.cpp), [term_toggles.h:strict_validation](include/ftd/term_toggles.h) |
| ARCH-4 | ✅ | `seed_rng()` now propagates to: bridge mt19937 + thread_rngs_ (via langevin_seed_initialized_=false) + toggles.langevin_seed (cuRAND picks up next tick) + direct gpu_->set_rng_seed() call. Determinism test PASS. | done 2026-04-25 | [render_bridge.h:seed_rng](include/ftd/render_bridge.h), [render_bridge.cpp:seed_rng](src/render_bridge.cpp) |
| ARCH-5 | ☐ | Device-side energy-ledger reduction (avoid 3 MB PCIe download per tick) | ~80 LOC | [gpu_engine.cu:454](cuda/gpu_engine.cu) — TODO comment already exists |
| ARCH-6 | ☐ | `voxels()` non-const accessor should use `voxels_mut()` rename to avoid spurious GPU sync invalidation | ~40 LOC + audit | [render_bridge.h:154-163](include/ftd/render_bridge.h) |
| ARCH-7 | ✅ | OMP-deterministic genesis: particle-ID assignment moved to sequential post-pass in voxel-index order. Combined with ARCH-7b closes TEST-004 fully. | done 2026-04-25 | [render_bridge.cpp:phase_write post-pass](src/render_bridge.cpp) |
| ARCH-7b | ✅ | Phase-ordering: pre-write flux snapshot (`flux_pre_write_`) populated before parallel-for. Genesis polarity (divergence) and spin (curl) read from the snapshot via new `divergence_from_flux_array` / `curl_from_flux_array` operators. Eliminates cross-thread flux race. **TEST-004 fully closed** — multi-thread determinism 4/4 PASS at 32 threads. | done 2026-04-25 | [render_bridge.cpp:phase_write](src/render_bridge.cpp), [field_operators.h](include/ftd/field_operators.h) |

---

## CPU-only feature gaps (awaiting GPU port)

GPU is the default backend whenever CUDA is available (see `RenderBridge` constructor at [render_bridge.cpp:73-77](src/render_bridge.cpp:73)). The 11 tests that call `force_cpu()` fall into two categories:

**Justified (testing CPU/GPU parity or CPU-only benchmarks):**
- `test_cpu_warnings.cpp` — explicitly tests CPU-only warning emission
- `test_determinism.cpp` — pinned to CPU due to known OMP non-determinism (TEST-004)
- `test_gpu_parity.cpp`, `test_gpu_parity_complete.cpp` — need both backends
- `benchmark_langevin_gpu.cpp` — measures CPU vs GPU performance
- `benchmark_manifestation_flow_cpu.cpp` — name says CPU

**Reveals CPU-only feature gaps:**

| ID | Status | Title | Notes |
|---|---|---|---|
| GPU-PORT-1 | ☐ | Phase H coupling (`coulomb_charge_coupling`) on GPU | [test_phase_h_coupling.cpp:34](tests/test_phase_h_coupling.cpp:34) — "only wired on the CPU path" |
| GPU-PORT-2 | ☐ | Mechanism B (vacuum polarization with Langevin BG) on GPU | [test_mechanism_b.cpp:38,158](tests/test_mechanism_b.cpp:158) — "we only modified the CPU implementation" |
| GPU-PORT-3 | ☐ | Wilson topology measurement on GPU | [test_wilson_topology.cpp:66](tests/test_wilson_topology.cpp:66) |
| GPU-PORT-4 | ☐ | EFT dual-cell adapter on GPU | [test_dual_cell_adapter.cpp:31,45](tests/test_dual_cell_adapter.cpp:31) |
| GPU-PORT-5 | ☐ | Wigner chirality diagnostic on GPU | [campaign_wigner.cpp:115,144](tests/campaign_wigner.cpp:115) — "dual substrate diagnostics need CPU" |

These are tracked here so the GPU-priority directive doesn't lose them. Each is a feature that exists ONLY on CPU; the parity is needed for production-grade GPU campaigns.

---

## Open issues found mid-work (2026-04-25)

| ID | Status | Title | Notes |
|---|---|---|---|
| OPEN-1 | ☐ | Pair-production axis-bias `if (fx >= fy && fx >= fz)` | [transmutation_phases.cpp:73-75](src/transmutation_phases.cpp:73). **Suspected** to contaminate the FTD-0086 plaquette bivector measurement (engine-expert audit). Same pattern in genesis spin/color assignment ([render_bridge.cpp:445-456,519-522](src/render_bridge.cpp:445)). |
| OPEN-2 | ☐ | Pre-existing hydrogen-spectrum test failures (HEM-3/5/6, PH4) | 4 failures unrelated to Round 1-3 work. Coulomb-vs-gravity force-direction tuning issues. |
| OPEN-3 | ☐ | `continuity` test flaky under `ctest -j 4` parallel run | Passes in isolation, fails under GPU resource contention. CTest infrastructure issue, not algorithmic. |
| OPEN-4 | ☐ | Test fixture deduplication for ~275 RenderBridge construction patterns | Original RF-2 scope; deferred until RF-1b mass migration progresses. |
| OPEN-5 | ✅ | Root cause: GpuEngine has its own `toggles` field (default dual_substrate=true), and inject_flux/particle/wavepacket branched on the GPU's local toggles. Tests setting `rb.toggles.dual_substrate=false` BEFORE first tick hit the GPU default. Fix: surgical `gpu->toggles.dual_substrate = rb.toggles.dual_substrate` sync immediately before each GPU inject call ([injection.cpp](src/injection.cpp)). Pinned by [test_open5_legacy_flux_l.cpp](tests/test_open5_legacy_flux_l.cpp) — 3/3 PASS. (done 2026-04-25.) |
| OPEN-6 | ◐ | `action_stationarity` (Particle EL residual RMS / max < 1e-10) + `asymptotic_freedom` (AF-6a: Force nonzero at all separations) — physics-tuning thresholds, not architecture/correctness bugs. **Deferred to physics review.** Both are pre-existing tolerance issues that should be re-examined by someone with the physics calibration context, not mechanically fixed. |
| OPEN-7 | ☐ | Multi-bridge GPU Langevin produces zero wave_vel on subsequent GPU bridges (single-bridge worked when filed). Test-008 deferred GPU portion as a result. See [test_langevin_gpu_cpu_parity.cpp](tests/test_langevin_gpu_cpu_parity.cpp) header comment. |
| OPEN-8 | ☐ | Single-bridge GPU `langevin_equipartition` produces deterministic zero `wave_vel` (regression observed during RF-9 verification, **NOT caused by RF-9** — CPU langevin parity passes; my changes only touched CPU path). **Investigation 2026-04-25:** initial hypothesis (non-const `voxels()` accessor uploading stale host state via `push_to_device` and clobbering cuRAND output) was **falsified**: even with `const auto& vox = ((const RenderBridge&)rb).voxels()` (which routes via const overload that does NOT mark `host_mutated_=true`), the burn-in `rb.run(N_BURN)` still produces zero `wave_vel`. So the bug is in the GPU compute path itself, not the host-shadow round-trip. cuRAND noise gen is correctly gated at [gpu_engine.cu:242-247](cuda/gpu_engine.cu); `phase_write_kernel` correctly uses `langevin_noise[]` at [kernels_stencil.cu:286-293](cuda/kernels_stencil.cu); `gauss_project` only modifies flux (not wave_vel). Suspect a different mechanism — possibly cuRAND state uninitialized between bridge ctor and first phase_write (the `set_rng_seed` early-return path). Defensive const-cast retained in [test_langevin_equipartition.cpp:88-93](tests/test_langevin_equipartition.cpp) for code hygiene; doesn't fix the bug. |

---

## Plan: Next focus (RenderBridge architectural split — ARCH-1)

**Why this next:** RenderBridge is the central type touched by ~140 files. Its god-class status is the biggest design-debt item; splitting it unblocks faster builds (RF-7, RF-9), cleaner tests (RF-1b), and clearer ownership for ARCH-2 (backend abstraction).

**Phased plan:**

1. **Phase A — inventory.** ✅ COMPLETE (2026-04-25). Full catalogue:

   | Category | Count | Methods |
   |---|---|---|
   | A. Lifecycle | 2 | ctor(int), dtor |
   | B. Simulation control | 9 | tick, run, set_dt, dt, current_tick, physical_time, seed_rng, set_sor_iterations, sor_iterations |
   | C. Backend control | 2 (public) | force_cpu, sync_from_gpu |
   | D. Access | 5 | lattice× 2, voxels× 2, voxel_at + public field `toggles` |
   | E. Buffer accessors | 6 | delta_j, prepare_delta_j, phi_coulomb, force_diag (×3 overloads), phi_latency |
   | F. Injection | 6 | inject_flux, inject_flux_add, inject_wave_vel_add, inject_particle, inject_wavepacket, create_entangled_pair |
   | G. Diagnostics (read-only) | 9 | diagnostics, energy_audit, energy_ledger, update_energy_ledger, continuity_step, compute_entropy, aggregate_profile, em_field_at, poynting_vector |
   | H. Discrete operators (inline delegators) | 8 | laplacian_flux, divergence_flux, curl_flux, gradient_state, gradient_density, gradient_divergence, gradient_scalar, curl_state_velocity |
   | I. SM operators (inline) | 3 | compute_stress, compute_stress_left, born_probability |
   | J. Quantum/Hilbert | 1 | hilbert_state |
   | **Total public surface** | **~51 methods + 1 field** | |

   Plus 11 friend declarations (extracted CPU functions in transmutation_phases.cpp / injection.cpp / energy_ledger_compute.cpp).

2. **Phase B — Diagnostician extraction.** Create a `RenderBridgeView` (read-only) and pass it to all diagnostic methods. Should compile clean against existing tests; no behaviour change.

3. **Phase C — Injector extraction.** Move all `inject_*` free functions into an `Injector` class that owns `next_particle_id_` (atomic) and `next_pair_id_`. RenderBridge holds an Injector.

4. **Phase D — discrete-operator delegator removal.** The 8 inline delegators (`divergence_flux`, `curl_flux`, etc.) duplicate `field_operators.h` free functions. Tests can call the free functions directly.

5. **Phase E — verify.** Full regression. Header file size + include-graph trim measurement. RenderBridge public method count drop.

**Risk control:**
- Each phase is one PR-equivalent commit.
- Tests run between phases.
- Backend abstraction (ARCH-2) waits for ARCH-1 to land — too much surface area touched at once otherwise.

**Estimated total LOC delta:** −60 LOC in render_bridge.h public API; +150 LOC across new files (`injector.h`, `bridge_view.h`); net +90 LOC across project but *much* tighter ownership boundaries and faster TU rebuilds.

### ARCH-1 — Closure report (2026-04-25)

**Phase A (inventory):** ✅ Full RenderBridge public-API catalogue: 51 methods + 1 public field across 10 categories. 11 friend declarations on extracted CPU functions.

**Phase B (Diagnostician via RenderBridgeView):** ✅ Created [bridge_view.h](include/ftd/bridge_view.h) — `using RenderBridgeView = const RenderBridge&;`. Existing diagnostic free functions in `diagnostics_compute.cpp` and `energy_ledger_compute.cpp` already take const ref; the type alias formalises the contract for forward-looking PIMPL/value-view migrations.

**Phase C (Injector extraction):** ✅ Created [injector.h](include/ftd/injector.h) (61 LOC). Moved `next_particle_id_` (`std::atomic<int>`) and `next_pair_id_` (`int`) from RenderBridge into a dedicated Injector class. Added `Injector& injector()` accessor on RenderBridge. Migrated 7 call sites:
- `src/render_bridge.cpp` — 2 genesis paths use `injector_.next_particle_id()`
- `src/transmutation_phases.cpp` — 2 pair-production paths
- `src/injection.cpp` — 3 injection paths + 1 entangled-pair path

The Injector encapsulates the BUG-001 atomic semantics behind named methods; future migrations (sharded counter, lock-free queue) won't break call sites. NOTE: 6 of the 11 friend declarations remain because injection functions still access GPU-sync internals (`use_gpu_`, `gpu_dirty_`, `host_mutated_`); collapsing those is ARCH-2 territory.

**Phase D (delegator removal):** ⊘ DEFERRED. The 8 inline discrete-operator delegators are used by **29 test/source files**. Mass-migrating them without per-call-site verification is high-risk. Documented the deprecation intent inline in render_bridge.h; new code should prefer `::ftd::laplacian_flux_op(...)` etc. directly. Mechanical migration tracked as future work.

**Phase E (verify):** ✅ 17/17 regression tests pass: determinism, closed_negatives, cpu_warnings, smallest_particle_emergence, genesis, baryogenesis, annihilation × 2, gpu_parity_complete, master_quadratic × 2, ladder_walk_from_oh, gpu_continuity_ledger, color_binding, voxel_properties, plaquette_bivector, clifford_multigrade.

**LOC summary:**
- `render_bridge.h`: 428 → 445 (+17, due to Injector accessor + Phase D deprecation comment)
- `injector.h`: NEW (+61 LOC)
- `bridge_view.h`: NEW (+36 LOC)
- Removed `omp critical(genesis_id)` blocks from render_bridge.cpp (already done in BUG-001)
- Net: +114 LOC, but ownership boundaries are now explicit. The "next_particle_id_" semantics live in one place; future fixes propagate through one accessor instead of 7 call sites.

**Friend declarations remaining:** 11 (down from would-be 13 — the Injector dependency is collapsed). Further reduction needs ARCH-2 (backend abstraction) so injection helpers don't reach into GPU internals.

### ARCH-2 — Foundation report (2026-04-25)

**Goal:** collapse 14 `#ifdef FTD_ENABLE_CUDA` blocks in render_bridge.cpp + 6 in render_bridge.h into a `Backend` interface with virtual dispatch. Migration is INCREMENTAL — interface added in parallel to existing `use_gpu_` flag, one method migrated at a time with regression at each step.

**Phase A — Backend interface:** ✅ Created [include/ftd/backend.h](include/ftd/backend.h). Abstract `Backend` class with `tick()`, `set_dt(double)`, `sync_to_host()`, `mark_host_dirty()`, `kind()`. `enum class Kind { Cpu, Gpu }` for tests that need to assert which backend is active.

**Phase B — Wiring:** ✅ Created [src/backend.cpp](src/backend.cpp) with `CpuBackend` and `GpuBackend` implementations. Added to engine/CMakeLists.txt. RenderBridge now holds `std::unique_ptr<Backend> backend_` alongside the existing `use_gpu_` flag during the migration. Constructor instantiates `GpuBackend` when CUDA is enabled (matches GPU-default policy), otherwise `CpuBackend`. `force_cpu()` swaps to `CpuBackend`.

**Phase C — `set_dt` migration:** ✅ Replaced `if (use_gpu_ && gpu_) gpu_->set_dt(dt)` with `backend_->set_dt(dt)`. First of 14 ifdef blocks closed.

**Phase D — Public accessors:** ✅ Added `Backend& backend()`, `const Backend& backend() const`, and `Backend::Kind backend_kind() const` to RenderBridge so tests can assert which backend is actually executing.

**Phase E — Verify:** ✅ 14/14 regression tests pass on GPU default (determinism, closed_negatives, cpu_warnings, smallest_particle_emergence, genesis, baryogenesis, annihilation × 2, gpu_parity_complete, master_quadratic_identities/uniqueness, ladder_walk_from_oh, gpu_continuity_ledger, color_binding_and_structure).

**LOC summary:**
- `backend.h`: NEW (+90 LOC)
- `backend.cpp`: NEW (+65 LOC)
- `render_bridge.h`: +12 LOC (Backend member + 3 accessors)
- `render_bridge.cpp`: −5 LOC (one ifdef block collapsed; +instantiation lines)
- Net: +162 LOC, but the foundation for collapsing 13 more ifdef blocks is in place.

**Open follow-on tickets** (added below as ARCH-2-A through ARCH-2-M, one per remaining ifdef migration):

| ID | Status | Title |
|---|---|---|
| ARCH-2-A | ✅ | Migrate `gpu_sync_to_host` ifdef → `backend_->sync_to_host()` (done 2026-04-25) |
| ARCH-2-B | ✅ | Migrate `gpu_push_to_device` → `backend_->push_to_device()` (done 2026-04-25) |
| ARCH-2-C | ✅ | Migrate `gpu_flush_host_mutations` → `backend_->flush_host_mutations()` (done 2026-04-25) |
| ARCH-2-D | ✅ | Migrate `RenderBridge::tick()` GPU branch → `backend_->tick()` (done 2026-04-25) |
| ARCH-2-E | ✅ | `RenderBridge::run()` already on tick() loop (BUG-003 fix); no migration needed |
| ARCH-2-F | ✅ | Migrate `phi_latency()` lazy-fetch → `backend_->mirror_phi_latency()` (done 2026-04-25) |
| ARCH-2-G | ✅ | Migrate `voxels()` non-const accessor → backend dispatch (done 2026-04-25) |
| ARCH-2-H | ✅ | Migrate `voxel_at(x,y,z)` non-const accessor → backend dispatch (done 2026-04-25) |
| ARCH-2-I | ✅ | Migrate `inject_*_cpu` GPU-fast-path branches; **6 of 11 friend declarations dropped** (done 2026-04-25). Added `backend().mark_gpu_dirty()` and `gpu_engine_ptr()` accessor. |
| ARCH-2-J | ✅ | DELETED the 3 private GPU sync delegators (`gpu_sync_to_host`, `gpu_push_to_device`, `gpu_flush_host_mutations`); all callers route through `backend_->...()` directly (done 2026-04-25). |
| ARCH-2-K | ✅ | `continuity_step()` migrated through public `gpu_engine_ptr()` accessor (done 2026-04-25). |
| ARCH-2-L | ◐ | Constructor's GPU init message kept as-is (cosmetic); could move to `GpuBackend` ctor as future cleanup. |
| ARCH-2-M | ✅ | **`use_gpu_` flag DELETED** (2026-04-25). Backend selection is now owned solely by `backend_->kind()`. force_cpu() swaps to CpuBackend; gpu_engine_ptr() checks the kind. |

The migration order matters: ARCH-2-J must come BEFORE ARCH-2-I (the injection migration depends on backend-owned sync methods). Recommended sequence: A → B → C → J → I → D → E → F → G → H → K → L → M.

### ARCH-2 — Migration progress (2026-04-25 update)

**Done in this session:** A, B, C, D, E (auto-satisfied), F, G, H. **8 of 13 ifdef blocks migrated to backend dispatch.** All under one regression sweep — 17/17 tests pass on GPU default.

Key migrations:
- `RenderBridge::tick()` GPU branch (the big one) now reads:
  ```cpp
  if (backend_ && backend_->kind() == Backend::Kind::Gpu) {
      backend_->tick();              // flush + engine->tick + sync
      if (toggles.latency_field) accumulate_proper_time();
      update_energy_ledger();
      return;
  }
  ```
  with the GPU-side flush/tick/sync sequence owned by `GpuBackend::tick()`.
- `voxels()` / `voxel_at()` non-const accessors no longer have `#ifdef FTD_ENABLE_CUDA`; they unconditionally call `backend_->sync_to_host()` and `backend_->mark_host_dirty()` (CPU no-ops, GPU does the work).
- `phi_latency()` const method does the same via `mirror_phi_latency()`.
- `gpu_sync_to_host()`, `gpu_push_to_device()`, `gpu_flush_host_mutations()` are now thin one-line delegators to `backend_->...()`.

**Still TODO** (5 items):
- ARCH-2-I: injection helpers reach into GPU internals — migrate via backend so the 6 inject_*_cpu friends can drop. Largest remaining win.
- ARCH-2-J: move sync logic OUT of RenderBridge entirely into GpuBackend (currently the bridge has thin delegators). Pure cleanup.
- ARCH-2-K: continuity_step() const-method GPU branch. Small, deferred.
- ARCH-2-L: constructor's GPU init message printout.
- ARCH-2-M: delete use_gpu_ flag once everything routes through `backend_->kind()`. Final ticket; lots of grep+replace in src/render_bridge.cpp.

**Friend declarations status:** **5** (down from 11; 6 inject_*_cpu friends DROPPED in ARCH-2-I, 2026-04-25). The 5 remaining (weak_transmutation_cpu, accumulate_proper_time, pair_production_cpu, triad_binding_cpu, update_energy_ledger_cpu) need access to bridge internals (rng_, voxels_, energy_ledger_) regardless of CPU/GPU; their reduction needs further extraction work, not backend dispatch.

**ARCH-2-I details:** added two public accessors: `RenderBridge::backend()` exposes `mark_gpu_dirty()`, and `RenderBridge::gpu_engine_ptr()` returns the GpuEngine pointer (or nullptr). injection.cpp now uses only public API: `voxels()`, `lattice()`, `injector()`, `backend()`, `gpu_engine_ptr()`, `toggles`. Side benefit: `voxels()` accessor's GPU-aware sync makes most inject_*_cpu functions implicitly GPU-aware — no manual `host_mutated_` toggling needed.

### ARCH-2 — FULL CLOSURE (2026-04-25 second pass)

**What landed in the second migration session:**

| Phase | Outcome |
|---|---|
| ARCH-2-I | 6 inject_*_cpu friends DROPPED. injection.cpp uses public API only. |
| ARCH-2-J | 3 private GPU sync delegators DELETED. All callers go directly through `backend_->...()`. |
| ARCH-2-K | continuity_step() migrated to use `gpu_engine_ptr()`. |
| ARCH-2-M | **`bool use_gpu_` field DELETED.** Backend selection is now owned solely by `backend_->kind()`. |

**Final state:**
- 14 of 14 GPU-branching ifdef blocks in `render_bridge.cpp` migrated to backend dispatch.
- 5 of 11 friend declarations remain (only the phase friends; injection friends ALL dropped).
- `use_gpu_` is GONE — no more dual-state-of-truth between flag and backend pointer.
- 18/18 regression tests pass on GPU default (added hydrogen_binding to the sweep — exercises GPU forces + injection heavily).

**Backend interface methods (final):**
- `tick()`, `set_dt()` — execution control
- `sync_to_host()`, `push_to_device()`, `flush_host_mutations()` — bidirectional sync
- `mark_host_dirty()`, `mark_gpu_dirty()` — sync flag setters (CPU no-ops)
- `mirror_phi_latency()` — special-case lazy fetch
- `kind()` — runtime backend identification

**LOC delta cumulative for full ARCH-2:**
- `backend.h`: +118 LOC (interface with 8 virtual methods × 2 implementations)
- `backend.cpp`: +112 LOC
- `render_bridge.h`: −20 net (5 ifdef blocks removed + use_gpu_ field deleted + 3 private method declarations deleted; 6 friend decls dropped, 1 accessor added)
- `render_bridge.cpp`: −44 net (3 delegator methods deleted + multiple ifdef blocks collapsed + use_gpu_ assignment removed)
- `injection.cpp`: ≈ same LOC, but now uses public API only

**Net LOC across project:** +166 (mostly the new backend abstraction). The win is in **eliminated friend declarations (−6), eliminated dual-state-of-truth flag (use_gpu_ deleted), and eliminated all 14 GPU ifdef blocks in the dispatch hot path**.

**GPU-priority verification:**
- Constructor instantiates `GpuBackend` whenever CUDA is enabled. Diagnostic message "[RenderBridge] GPU backend active" prints on every test execution.
- `backend_kind()` accessor lets new tests assert which backend ran.
- All 5 GPU-PORT items (CPU-only features awaiting GPU port) tracked in the dedicated section above.
- 11 force_cpu() callers audited; all justified.

**Pre-existing test failures discovered during sweep** (NOT caused by ARCH-2):
- `dual_substrate` test: "Legacy: flux_L = 0" assertion fails. Verified pre-existing by stash-bisect.
- `action_stationarity` test: failing.
- `asymptotic_freedom` test: failing.

These three were already failing before today's work; they need separate triage. Adding to OPEN issues.

**LOC delta cumulative for ARCH-2:**
- `backend.h`: +103 LOC (interface + 6 methods × 2 implementations)
- `backend.cpp`: +90 LOC
- `render_bridge.h`: −0 net (3 ifdef blocks removed, comments added)
- `render_bridge.cpp`: −20 net (multiple ifdef blocks shrank to delegators)
- Net: +173 LOC, but 8 of 14 #ifdef blocks gone from render_bridge.cpp.

---

## ROUND 5 — Constructor-domain formalization (OPEN 2026-04-26)

Reference contract: [SPEC_ENGINE_CONSTRUCTOR_CONTRACT.md](SPEC_ENGINE_CONSTRUCTOR_CONTRACT.md).

The goal of this round is to make every physics-facing engine test/campaign
declare what constructor domain it exercises, what observable it measures, and
what failure means. This is how the engine becomes a formal proof harness rather
than a pile of successful demos.

| ID | Status | Title | Notes |
|---|---|---|---|
| FORM-001 | ✅ | Draft engine constructor contract | `SPEC_ENGINE_CONSTRUCTOR_CONTRACT.md` maps constructor domains to engine obligations. |
| FORM-002 | ✅ | Add constructor-domain metadata helper | Done 2026-04-26: `ftd::test::ConstructorContract`, `valid_contract`, and `contract()` emit human output or NDJSON `event=contract`. |
| FORM-003 | ✅ | Add native observable registry | Done 2026-04-26: seed descriptor registry for ledgers, blocked operators, field energy, state histogram, flux correlator, and Gauss violation. |
| FORM-004 | ✅ | Add CTest labels for constructor domains | Done 2026-04-26: first constructor-critical tests labeled with `constructor`, `ledger`, `observable`, `blocking`, and GPU/EFT labels where applicable. |
| FORM-005 | ✅ | Define an "EFT quick suite" label | Done 2026-04-26: `eft_quick` first slice covers constructor metadata, observable registry, GPU continuity ledger, transport flow, blocking, Ward, matched-Poisson, and sim observables. |
| FORM-006 | ☐ | Production Gauss representation decision record | Choose collocated, source-core, or dual-cell face flux; update tests and docs. |
| FORM-007 | ☐ | Formal propagation-bound tests | For each state-changing toggle, assert finite support and no nonlocal writes outside declared neighborhood/phase. |
| FORM-008 | ☐ | Closure-domain declarations | Make periodic lattice, blocked cell, and open-boundary campaigns declare their accounting surface. |
| FORM-009 | ☐ | GPU Langevin/stochastic contract | Fix GPU Langevin zero-wave-velocity issue, then declare seed/measure semantics and parity tolerances. |
| FORM-010 | ☐ | Device-side ledger reductions | Long-run continuity/reaction/energy/operator moments without per-tick host downloads. |
| FORM-011 | ☐ | Nonlinear operator mixing matrix | Consume full-tick GPU histories, block them, compute before/after operator vector, assemble measured mixing matrix. |
| FORM-012 | ☐ | Native history measure/action decision | Decide action vs transfer matrix vs deterministic pushforward vs constrained history measure. |
| FORM-013 | ☐ | Continuum-limit scaling protocol | Predeclare L/b scaling observables and acceptance criteria before smooth-field claims. |
| FORM-014 | ☐ | GPU ports for constructor-critical CPU-only diagnostics | Phase-H coupling, Mechanism B/Langevin background, Wilson topology, dual-cell adapter, Wigner chirality. |
| FORM-015 | ☐ | Test/campaign failure taxonomy | Each formalized test should state whether failure means bug, calibration miss, selection falsified, conjecture falsified, or expected open gap. |

Initial audit verdict:

```text
The engine is constructor-serious but not constructor-complete.

Strongest domains:
relation, tick dynamics, GPU continuity ledger, b=2 Gaussian blocking.

Weakest domains:
observable registry, stochastic GPU correctness, production Gauss choice,
nonlinear mixing, native action/measure, continuum scaling protocol.
```

---

*Filed 2026-04-25. Living doc — update on each round closure.*

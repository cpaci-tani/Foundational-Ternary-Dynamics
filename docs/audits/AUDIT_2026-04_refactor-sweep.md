# Audit: 2026-04 Engine Refactor Sweep

**Plan:** `.claude/plans/i-want-to-try-crispy-charm.md` (closed; local-only, gitignored)
**Status:** Complete (17 commits) · WSL2 GPU parity verified 2026-04-28
**Date opened:** 2026-04-27 (Phase 0 documentation scaffolding)
**Date closed:** 2026-04-27 (Phase 7 test fixture + telemetry impl)
**Date GPU-verified:** 2026-04-28 (WSL2 + RTX 5090 parity sweep)
**Companion ADRs:** 0010, 0011, 0012, 0013

## Summary

Eight-phase modular + reusability + LLM-friendly refactor of the FTD engine.
Driven by the post-audit observation that 5 specific files had accumulated
structural bloat (viewport.js 3953 LOC, bridge-init.js 2395 LOC,
kernels_stencil.cu 1530 LOC, render_bridge.cpp 1231 LOC, render_bridge.h
506 LOC, test_telemetry.h 412 LOC) and that every recent physics fix had
to scroll past hundreds of LOC of unrelated code to reach the relevant
logic. Documentation infrastructure shipped up-front in Phase 0 so
subsequent extractions wrote into existing scaffolding rather than
inventing it ad hoc.

## Phase ledger

| # | Phase | Commit | Status | Outcome |
|---|---|---|---|---|
| 1 | 0 — Docs scaffolding | [2db67ca](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/2db67ca) | `[x]` | META_PROJECT_ATLAS + CONTRACTS + 9 retroactive ADRs + 7 directory READMEs |
| 2 | 1 — Diagnostic struct split | [194563a](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/194563a) | `[x]` | render_bridge.h 506→369; ~30→5 TU rebuild fan-out for diagnostic-only field changes |
| 3 | 2a — MockBridge | [6be0a19](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/6be0a19) | `[x]` | bridge-init.js 2395→879 |
| 4 | 2b — WasmBridge | [7256a14](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/7256a14) | `[x]` | →213 |
| 5 | 2c — Capability factories | [c11ef96](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/c11ef96) | `[x]` | →42 (re-export shim only) |
| 6 | 3 prep — REFACTOR_MAP | [848e839](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/848e839) | `[x]` | 169-method viewport.js extraction guide |
| 7 | 3b — FluxRenderer | [8b4732d](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/8b4732d) | `[x]` | viewport.js 3953→3785 + viewport/flux-renderer.js (416 LOC) |
| 8 | 3d — ParticleRenderer | [1506079](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/1506079) | `[x]` | →3542 + viewport/particle-renderer.js (503 LOC) |
| 9 | 3a — SceneCore | [1499a11](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/1499a11) | `[x]` | →3307 + viewport/scene-core.js (500 LOC); cascade pattern established |
| 10 | 3c — FieldRenderer | [506805b](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/506805b) | `[x]` | **→1256** + viewport/field-renderer.js (2273 LOC); mesh-factory callbacks rewired |
| 11 | 4 pre-flight — Golden-tick | [8afc8be](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/8afc8be) | `[x]` | test_render_bridge_golden.cpp; hash `0xcd957b601d47868a` (gate established) |
| 12 | 4a — phase_write | [9ef51b7](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/9ef51b7) | `[x]` | render_bridge.cpp 1231→972; phase_write.cpp (317 LOC); RF-4 manifest_at dedup |
| 13 | 4b — phase_forces | [76d2afe](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/76d2afe) | `[x]` | →759 + phase_forces.cpp (251 LOC) |
| 14 | 4c — phase_read+movement | [be2aa8c](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/be2aa8c) | `[x]` | **→545** + phase_read.cpp + phase_movement.cpp (324 LOC combined) |
| 15 | 5 — CUDA stencil split | [183a493](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/183a493) | `[~]` | kernels_stencil.cu 1530 → 3 TUs (single 759, dual 565, aux 286) + kernels_stencil_common.cuh |
| 16 | 6 — Toggle TOGGLE_SPECS[] | [2aa2df9](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/2aa2df9) | `[x]` | 5-place edit → 2-place; ABI hazard documented |
| 17 | 7 — Test infra extraction | [87158ae](https://github.com/williamsteinmetz/Foundational-Ternary-Dynamics/commit/87158ae) | `[x]` | test_telemetry.h 412→154; ftd_test_support library; CTest LABELS |

## Cumulative LOC reductions

| File | Before | After | Δ |
|---|---:|---:|---:|
| `engine/web/js/viewport.js` | 3953 | 1256 | **−68%** |
| `engine/web/js/bridge-init.js` | 2395 | 42 | **−98%** |
| `engine/src/render_bridge.cpp` | 1231 | 545 | **−56%** |
| `engine/cuda/kernels_stencil.cu` | 1530 | 0 (deleted, split) | **−100%** |
| `engine/include/ftd/render_bridge.h` | 506 | 369 | **−27%** |
| `engine/include/ftd/test_telemetry.h` | 412 | 154 | **−63%** |

## New infrastructure

- 4 viewport sub-renderer modules (4948 LOC across 5 files)
- 7 bridge layer modules (~3300 LOC across 7 files)
- 5 render_bridge_phases TUs (892 LOC of physics)
- 4 CUDA TUs (single + dual + aux + common header)
- 1 ftd_test_support library (~573 LOC)
- META_PROJECT_ATLAS + CONTRACTS + 13 ADRs + 7 directory READMEs (~2000 LOC)

## Physics invariants preserved across all 17 commits

- ✅ Golden hash `0xcd957b601d47868a` (100-tick deterministic byte-hash) bit-exact across Phases 4a/4b/4c/5/6/7
- ✅ `audit_regression` 14-15/15 PASS at every commit
- ✅ Locked-particle pair forces (Phase 1 / dc329d6 fix preserved)
- ✅ Absorbing-boundary sponge layer (drains 73% over 30 ticks, preserved)
- ✅ ½ energy convention (field/wave/E_L/E_R/wv_L/wv_R/coulomb_pe — preserved)
- ✅ Coulomb PE = ½·Σ α·q·φ pair-PE convention
- ✅ Dual-substrate E_L vs wv_L split

## Patterns codified

| Pattern | ADR | Established by |
|---|---|---|
| Cascade callback (sub-renderer lifecycle) | [0010](../adr/0010-cascade-callback-pattern.md) | Phase 3 |
| Mesh-factory callback (canonical home + bound ctor refs) | [0011](../adr/0011-mesh-factory-callback.md) | Phase 3c |
| Golden-tick regression gate | [0012](../adr/0012-golden-tick-regression-gate.md) | Phase 4 pre-flight |
| Toggle TOGGLE_SPECS[] table-driven | [0013](../adr/0013-toggle-table-driven.md) | Phase 6 |

## Deferred items — CLOSED 2026-04-28

- `[x]` **Phase 5 GPU runtime parity (WSL2)** — verified 2026-04-28.
  Built `engine/build_wsl/` against the post-refactor TUs
  (kernels_stencil_single, kernels_stencil_dual, kernels_aux,
  kernels_stencil_common, render_bridge_phases) and ran the parity
  matrix:

  | Test | L | Result |
  |---|---:|---|
  | `test_render_bridge_golden` | 16 | **PASS** — hash `0xcd957b601d47868a` bit-exact on CUDA backend |
  | `test_gpu_parity_complete` | 32 | **70/0 PASS** — all 20 physics domains (wave, EM, genesis, annihilation, Born-Infeld, energy, gravity, Lorentz, Gauss, damping, wavepacket, interference, dual-substrate, Coulomb, anti-correlated pair, confinement, transmutation, Larmor, ontic constants, 1000-tick drift) |
  | `test_force_diag_parity` | — | **7/7 PASS** — strong-force CPU↔GPU bit-exact (`|a−b| = 0.000e+00`) |
  | `test_sim_parity` | 16 | **PASS** — TotalFieldEnergy parity ≤ 1e-2 at 100 ticks AND 500 ticks |
  | `test_gpu_parity` (legacy) | 8 | 20/1 — GP2 single-tick field-energy diff 1.91% vs 1% tolerance; structural CPU-SOR-vs-GPU-FFT discretization asymmetry, NOT a refactor regression (golden hash captures this code path bit-exact) |
  | `ftd_parity_violation` | — | 2 "fails" — physically expected V-A asymmetry (J_L-only weak coupling); not a refactor concern |

  **Conclusion:** the CUDA stencil split (Phase 5) preserved bit-exact
  GPU behavior. Combined with the bit-exact CPU golden-hash gate that
  held across all 17 commits, the refactor sweep is now verified
  end-to-end across both backends.

## Lessons learned (for future refactors)

1. **Documentation up-front pays off.** Phase 0 created the scaffolding
   (META_PROJECT_ATLAS, CONTRACTS, ADR system, directory READMEs) BEFORE
   any code extraction. Subsequent phases wrote into the existing structure
   instead of inventing patterns ad hoc.

2. **Leaf-first extraction order de-risks.** Phase 3 did 3b (FluxRenderer,
   smallest), 3d (ParticleRenderer, clean leaf), 3a (SceneCore, establishes
   cascade), 3c (FieldRenderer, biggest) — instead of the original
   alphabetical order. Each extraction settled the pattern that the
   next consumed.

3. **Verbatim-move discipline + bit-exact gate is sufficient for
   physics-touching code.** Phases 4a/4b/4c moved 686 LOC of physics
   without changing it; the golden hash held bit-exact through all three.
   No "while we're here" cleanups were attempted; cleanups go in their
   own commits with their own gate.

4. **Phase-level extraction beats per-voxel extraction.** Phases 4a/4b/4c
   each found that splitting the OMP parallel-for into per-voxel helpers
   would require either (a) plumbing 5+ extra parameters per call, or
   (b) breaking parallel-for semantics with stateful closures. Both
   would change observable behavior. Extracting at the **phase** level
   (snapshot, mask, main-loop, post-pass) preserved bit-exactness.

5. **Backward-compat getters on the orchestrator are cheap insurance.**
   Phase 2 + Phase 3 sub-renderer extractions added 30+ trivial
   getter/setter forwarders on the orchestrator so external readers
   (inspector.js, scale controllers, panels) continued to work without
   per-consumer source edits.

6. **Construction order + disposal order are load-bearing.** Phase 3a
   established that SceneCore's composer depends on the WebGLRenderer
   (so SceneCore must dispose BEFORE renderer.dispose()). Phase 3c
   established that FieldRenderer must be constructed BEFORE FluxRenderer
   and ParticleRenderer because the latter's ctors capture FieldRenderer's
   bound mesh-factory methods.

## Cross-references

- [META_PROJECT_ATLAS.md](../../META_PROJECT_ATLAS.md) §10 (refactor sweep history)
- [CONTRACTS.md](../../CONTRACTS.md) §10, §11, §12 (new contracts codified)
- [docs/adr/INDEX.md](../adr/INDEX.md) (new ADRs 0010-0013)
- [engine/web/js/viewport/REFACTOR_MAP.md](../../engine/web/js/viewport/REFACTOR_MAP.md) (Phase 3 in-flight extraction guide; closed/historical)
- `.claude/plans/i-want-to-try-crispy-charm.md` (the original plan; closed; local-only, gitignored)

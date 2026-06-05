# SPEC — Scale-0 Lattice Performance: Sparse Tick + Worker Physics

**Status:** `[DESIGN — awaiting approval]`  ·  **Date:** 2026-06-03  ·  **Scope:** `engine/web` Scale-0 (lattice) JS physics path only.

This spec addresses the FPS collapse on the in-browser lattice past L≈65. It is grounded in a
direct per-frame profile (below), not assumptions. Two complementary phases:

- **Phase 1 — Active-region (sparse) wave tick.** Cut the dominant cost by computing only where the
  field is nonzero. Self-contained, bit-verifiable, ships independently.
- **Phase 2 — Physics in a Web Worker.** Move the tick off the main thread so render + UI stay smooth
  regardless of tick cost. Structural; builds on Phase 1.

---

## 1. Problem & evidence (measured 2026-06-03, Chrome/wasm64, default `flux-pulse`, 14 overlays active)

Per-frame component cost vs lattice size, timed via `preview_eval` on the live bridge:

| L | voxels | **physics tick** | render draw | getFluxVolume | overlays (×14) | diagnostics |
|---|--------|------------------|-------------|---------------|----------------|-------------|
| 49 | 118K | 5.5 ms | ~0 | ~0 | ~0 | — |
| 65 | 275K | 12 ms | ~0 | ~0 | 0.1 ms | — |
| 97 | 913K | 38 ms | ~0 | ~0 | ~0 | 11 ms |
| 129 | 2.1M | **89 ms** | **~0** | **~0** | **~0** | 33 ms / 3 frames |

Tick decomposition at L=97 (toggle each pass off, measure the delta):

| pass | ms | note |
|------|----|------|
| **wave_propagation (`_tickFlux`)** | **42.7** | **96% of the 44 ms tick** |
| coupling | 3.7 | only with particles present |
| selective_damping | 3.1 | |
| movement | 3.0 | |
| gauss_projection | 2.3 | *not* a heavy global solve |
| genesis | 2.1 | O(N³) super-threshold scan |
| forces / poisson_coulomb | ~2 each | |

**Findings:**
1. The bottleneck is the **single-threaded JS wave stencil `_tickFlux`** (mock-bridge.js:931) — an O(N³)
   18-point isotropic-Laplacian leapfrog. It is **96% of the tick** and scales with voxel count.
2. The scheduler ([tick.js:16](../js/scales/scale0/runtime/tick.js)) caps physics to **1 tick/frame for
   N>96**, so a *running* sim pays the full tick every frame: ~12 ms @65 (~80 FPS) → 89 ms @129
   (**~10 FPS**). The ~29 FPS "ceiling" measured in a tight loop is the *paused* state (no tick fires).
3. **Render, data extraction (`getFluxVolume`), and overlays are all ~0 ms.** The GPU is idle; the main
   JS thread is the constraint. → *Data compression cannot help FPS; only reducing/relocating compute can.*
4. gauss_projection is only 2.3 ms, so "a global elliptic solve won't sparsify" is a non-issue — the one
   pass that matters (`_tickFlux`) is a **local** stencil and sparsifies cleanly.

## 2. Goals / non-goals

**Goals**
- G1. Restore interactive FPS (>30) for **centered, localized, or damped** flux scenarios at L≥97.
- G2. Keep the UI/render thread smooth (no multi-frame stalls) at any L, even when the field is dense.
- G3. Zero change to physics results within a declared tolerance (bit-exact where claimed; ε-bounded otherwise).
- G4. Zero change to the sampler / overlay / renderer surface (field buffers + layout unchanged).

**Non-goals**
- N1. Speeding up the compiled C++/WASM engine path (`empty`/`light-*`/`quantum-*`) — out of scope; those
  are already SIMD/compiled and are not the reported problem.
- N2. WebGPU compute (separate 6–12 wk effort), the native CUDA `ws_server` path (already exists, untouched),
  and the lattice resize cap (covered by the 2026-06-03 WASM64 work).
- N3. Steady-state speedup for a fully-occupied lattice (a wave that has filled the box) — Phase 1 reverts to
  dense there by design; Phase 2 keeps the UI smooth regardless.

---

## 3. Phase 1 — Active-region (sparse) wave tick

### 3.1 Principle

In a leapfrog wave step, a voxel whose 18-neighborhood of `J` is entirely zero has Laplacian 0, so its `WV`
and `J` do not change — computing it is a provable no-op. The wave front advances at most one voxel per tick
(CFL: `C_SPEED = 1/√3`, so `c·dt ≤ 1`). Therefore the *active region* = {nonzero voxels} ∪ {their 1-voxel
dilation}, and it can only grow by one voxel-shell per tick. Iterate only that region.

### 3.2 Two implementations (start with 3.2a)

**3.2a Bounding-box (block-sparse) — primary.**
Track the axis-aligned extent of the nonzero field: `box = {x0,x1, y0,y1, z0,z1}`. `_tickFlux` iterates only
`[x0-1 .. x1+1] × …` (the +1 margin is the frontier the wave propagates into). This matches the
**centered-phenomena** use case the odd-lattice work was built for: a center injection starts as a point and
the box grows with the spherical wavefront.
- Bookkeeping: 6 integers. Negligible overhead.
- Cost ≈ `dense × (box_volume / N³)`.

**3.2b Per-voxel active list — alternative / upgrade.**
A `Uint8Array activeMask(N³)` + compact `Int32Array activeIdx` + count. Each voxel that becomes nonzero adds
its 18 neighbors to the next-tick set. General (handles scattered or multiple phenomena, and the periodic-wrap
case stays sparse), at the cost of per-voxel mask maintenance. Implement only if 3.2a's box is too loose for
real scenarios (e.g. several separated pulses).

> Decision: ship **3.2a** first (simplest, matches the centered use case, bit-exact). Keep 3.2b as a
> documented upgrade path behind the same `_activeRegion` abstraction so the tick loop doesn't change shape.

### 3.3 Data & mutation hooks

Field buffers (`_fluxJ`, `_fluxWV`, `_fluxMag`; layout `idx = z·N² + y·N + x`, interleaved `×3`) are
**unchanged**. Add to `MockBridge`:
- `_activeBox` (or `_activeMask`/`_activeIdx` for 3.2b), plus `_activeDense` (bool: fell back to dense).

The box must expand on every write that can introduce nonzero flux. All such writes funnel through a small
set of methods:
- `_injectFlux(x,y,z,…)` — mock-bridge.js:898 — **the single flux-write chokepoint** (used by `injectFlux`,
  scenario seeds, the genesis burst at :367). Add `_expandActiveBox(x,y,z)` here.
- `seedRandomFlux()` — :684 — writes `_fluxJ` directly across the whole lattice → set box = full / dense
  (correct: a random fill is genuinely dense; no benefit, no harm).
- `setupScenario(name)` — :1561 (→ `bridge/scenarios/index.js`) — after it runs, do one
  `_recomputeActiveBox()` scan to initialize the box from the seeded field.
- `clearField()` :677 / `reset()` — reset box to empty.

Each tick, after the stencil + J-commit: expand the box by 1 in every direction (wavefront), then **trim**
faces that are entirely sub-ε (the box stays tight as a damped front recedes). A periodic re-scan
(`_recomputeActiveBox`, every K ticks) bounds drift.

### 3.4 Boundary handling (load-bearing)

`_tickFlux` has an interior fast path (no modulo, :1035–1098) and a **periodic-wrap** boundary path
(`% N`, :1103+). When the active box touches any wall, periodic wrap couples it to the opposite wall — the
box would have to span the full axis. Rule: **once the box touches a boundary, fall back to dense** (set
`_activeDense = true`). Also fall back when `box_volume / N³ > 0.4` (bookkeeping no longer pays). The dense
path is the existing code verbatim. (3.2b handles wrap by adding the wrapped neighbor to the set, staying
sparse — another reason it's the general upgrade.)

### 3.5 Correctness & the ε tradeoff

- With **exact-zero** activation the sparse tick is **bit-identical** to dense (skipped voxels are exactly
  no-ops). But damping makes amplitudes decay toward — never exactly reaching — zero, so an exact-zero box
  grows monotonically and never trims. For *sustained* tightness, trim/activate on `|J|,|WV| < ε`
  (default ε small, e.g. 1e-7 of peak). This drops a sub-ε dispersed tail — a tunable, bounded fidelity
  tradeoff, off (ε=0, bit-exact) by default and surfaced as a dev/console setting, not silent.
- Genesis/coupling/other passes stay dense initially (≤3 ms total). Optional later: gate genesis on the box.

### 3.6 Expected payoff (honest)

Tick cost ∝ active-box fraction; the box tracks nonzero support, which the 18-point stencil grows by ≤1
voxel/tick. **Measured (2026-06-03, L=97, dense = 52 ms/tick):** the speedup is bit-exact but strongly
**scenario-dependent**:

| Source | seeded fraction (tick 0) | sparse tick (early) | speedup |
|--------|--------------------------|---------------------|---------|
| point injection (1 voxel) | ~0 | **3.5 ms** | **~15×** |
| **default `flux-pulse`** | **~25% of the lattice** | ~43 ms | **~1.2×** |

The win lands on **localized / point-like** sources, which start with a tiny box and stay sparse for many
ticks. The **default flux-pulse seeds a broad pulse** (~25% of the lattice immediately) and fills to dense
within ~20 ticks, so it sees little benefit — sparsity cannot help a field that already occupies most of the
box. The dense fallback guarantees **no regression** (≤ one extra branch/tick once `_activeDense` latches; the
per-tick box grow/recompute run only while sparse). An optional **ε-prune** could tighten a broad-but-smooth
seed toward its high-amplitude core, but flux-pulse's seed is coherent (not mostly sub-ε tail), so it would
help little. **For the broad / steady-state case (incl. the default scenario), Phase 2 (worker) — not
sparsity — is the fix:** it keeps the UI at 60 FPS regardless of how long each tick takes.

### 3.7 Verification

1. **Equivalence:** new C++-style golden check in JS — hash `_fluxJ` after K=50 ticks from a fixed center
   injection, dense vs sparse; assert equal (ε=0) bit-for-bit. A Playwright spec drives this.
2. **Profile:** re-run the §1 harness; record tick ms vs L for a fresh pulse at tick 10/50/100.
3. **Regression:** `scale0-resize-guard`, `scenario-parity`, `wasm-scenario-coverage`, `flux-slice-axes`,
   `toggle-coverage` stay green; visual spot-check that a centered pulse looks identical.

### 3.8 Files (Phase 1)

- `engine/web/js/bridge/mock-bridge.js` — `_tickFlux` active-region loop; `_activeBox` + helpers
  (`_expandActiveBox`, `_recomputeActiveBox`, trim, dense-fallback); hooks in `_injectFlux`,
  `seedRandomFlux`, `clearField`, `reset`, post-`setupScenario`.
- `engine/web/tests/scale0-sparse-tick.spec.js` — new equivalence + profile spec.
- (No changes to samplers, overlays, renderer, scenario-loader.)

**Effort:** moderate, single-file core. **Risk:** low (dense fallback is the existing code; ε=0 is bit-exact).

---

## 4. Phase 2 — Physics in a Web Worker

### 4.1 Architecture

The MockBridge tick (now sparse) runs in a dedicated Worker. The main thread keeps a **thin proxy** exposing
the API the rest of Scale-0 already calls, backed by a **shadow** of the field buffers updated from the
worker. Only `flux-*`/`s0-*` (MockBridge-owned, per `shouldUseFluxMock`) move to the worker; the WASM engine
path stays on the main thread (out of scope, already fast).

```
main thread                              worker thread
───────────                              ─────────────
MockBridgeProxy  ──commands(postMessage)──▶  MockBridge (authoritative, sparse tick)
  · shadow {_fluxJ,_fluxWV,_fluxMag,        │  runs tickScale0() on its own cadence
     _stateGrid,_particles,latticeSize,     │
     _boundaryMask}                          ▼
  · samplers/getFluxVolume/diagnostics  ◀──field frame (SAB or transferable)
    run on the shadow (already buffer-based)
```

### 4.2 The elegant part — samplers reuse

Every sampler (`mock-lattice-samplers.js`: vorticity, helicity, curlJ, fisher, coherence, latency,
kretschmann, state, gaussResidual, flux slice…) is a **pure function of the field buffers** taking a `state`
object. The shadow carries those buffers, so the **existing sampler code runs unchanged on the main thread**
against the shadow. `getFluxVolume`/`getStateGrid` likewise read the shadow. No per-frame round-trip to the
worker for reads. (This is why §1 measured reads at ~0 ms — they stay ~0.)

### 4.3 Message protocol

- **Commands** (main→worker, infrequent — user actions): `create(N)`/`resize`, `setupScenario(id)`,
  `injectFlux(...)`, `seedRandomFlux`, `clearField`, `reset`, `setToggle(k,v)`, `setParams`, `setDt`,
  `setBoundaryShape`, `setReflectiveBoundary`, `pause/resume`.
- **Frames** (worker→main, per tick): the field buffers + `{tick, physicalTime, energy scalars, particles,
  fieldDataVersion}`. Diagnostics (energy sums) computed **in the worker** and shipped as scalars (removes
  the §1 33 ms/3-frame diagnostics cost from the main thread too).

### 4.4 Transfer mechanism

- **Preferred: `SharedArrayBuffer`** — zero-copy; the shadow views the same memory the worker writes
  (double-buffer + a seqlock/Atomics flag to avoid tearing). **Requires COOP/COEP headers**
  (`Cross-Origin-Opener-Policy: same-origin`, `Cross-Origin-Embedder-Policy: require-corp`) on `serve.py`
  **and** the deployed host — must verify nothing else on the page breaks under cross-origin isolation.
- **Fallback: transferable `ArrayBuffer`** double-buffering (worker posts buffer, gets the prior one back) —
  no header requirement, one copy/transfer per frame (cheap; buffers are ~tens of MB and transfer is
  zero-copy move). Ship this if COOP/COEP is problematic.

### 4.5 Hard parts (call out explicitly)

- **~~Timeline / scrubbing.~~** ❌ **No longer applicable (2026-06-05).** The scrub-back / timeline
  feature was removed — the simulation is forward-only (single time source). There is no memory
  recorder and no "display-frozen vs live" state to reconcile, which *removes* what was the riskiest
  worker seam. The only freeze state the proxy forwards is plain pause (`setRunning(false)`). See
  `SPEC_SCALE0_RUNTIME_PIPELINE.md` §8.
- **Synchronous-call audit.** Build-step 1 of Phase 2 is enumerating every synchronous `mock.*` / `fluxMock.*`
  call site (scenario-loader, controller runtime, diagnostics, samplers via `capabilities/scale0`)
  and classifying each as read-from-shadow vs command-to-worker. Reads must tolerate "shadow is one tick
  behind the worker" (it always is — that's the point).
- **Lifecycle.** Worker created on first Scale-0 mount; terminated on `destroy`; re-created on re-mount. The
  resize path (`resizeScale0Lattice`) sends `create(N)` to the worker instead of `new MockBridge(N)`.

### 4.6 Verification

- Field-frame parity: worker-driven field hash matches a main-thread reference run after K ticks.
- All Phase-1 specs green; add a worker-lifecycle spec (mount→tick→resize→destroy, no leaks via
  `__ftdRAF.size()` proxy).
- Manual: confirm UI stays at 60 FPS (drag camera, toggle overlays) while a heavy L=129 tick runs.

### 4.7 Files (Phase 2)

- `engine/web/js/bridge/mock-bridge.worker.js` — worker entry hosting MockBridge + message loop.
- `engine/web/js/bridge/mock-bridge-proxy.js` — main-thread proxy + shadow + sampler delegation.
- `engine/web/js/scales/scale0/runtime/scenario-loader.js` — construct the proxy instead of `MockBridge`.
- `engine/web/serve.py` (+ deploy host) — COOP/COEP headers *if* SAB path is chosen.
- diagnostics wiring; a worker-lifecycle Playwright spec.

**Effort:** larger (multi-file, threading, header decision). **Risk:** medium — gated behind a
feature flag (see §6) so it can ship dark and be A/B'd against the main-thread path.

---

## 5. Build sequence

1. **Phase 1a** bounding-box sparse `_tickFlux` + hooks + dense fallback (ε=0, bit-exact).
2. Verify: equivalence hash, re-profile, regression suite. **Ship.**
3. (If needed) **Phase 1b** per-voxel active list behind the same abstraction.
4. **Phase 2** worker, behind a feature flag: (a) synchronous-call audit, (b) proxy+shadow+samplers,
   (c) command/frame protocol with transferable fallback, (d) SAB+headers if viable.
5. Verify: parity, lifecycle, manual smoothness. Flip the flag on by default for MockBridge scenarios.

## 6. Risks, flags, rollback

- **Feature flags:** `FTD_SPARSE_TICK` (default on after §5.2; falls back to dense loop when off) and
  `FTD_PHYSICS_WORKER` (default off until §5.5). Both let us revert instantly without a code change.
- **ε-prune** defaults to 0 (bit-exact); any nonzero ε is opt-in and logged.
- **COOP/COEP** may interfere with third-party embeds / the WASM load — verify before adopting SAB; the
  transferable fallback needs no headers.
- **Forward-only** (2026-06-05): scrub/timeline removed, so the formerly highest-risk Phase-2 seam
  (display-frozen-vs-live snapshot reconciliation) no longer exists.
- Worst case for Phase 1 (fully dense lattice): ~one extra bounds check per tick vs today — negligible.

## 7. Open questions

- O1. Do any shipped flux scenarios seed **multiple separated** phenomena (favoring 3.2b over 3.2a)? Audit
  `bridge/scenarios/*` during Phase 1a; if common, prioritize 3.2b.
- O2. Is the deploy host able to send COOP/COEP? If not, Phase 2 uses the transferable path (no functional loss,
  one cheap copy/frame).
- O3. Should diagnostics move into the worker in Phase 2 (removes the 11 ms/frame main-thread cost) or stay
  main-thread on the shadow? Lean: compute in worker, ship scalars.

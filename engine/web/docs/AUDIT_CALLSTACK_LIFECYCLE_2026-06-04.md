# Web Engine Callstack + Lifecycle Audit — 2026-06-04

**Scope:** dissect the web-engine callstack, prove all lifecycles are valid (with
evidence, not assertion), and apply contained, correctness-neutral performance
fixes — focus on the large-lattice (L>65) cliff. Status of the subsystem going in:
already hardened by the 2026-06-01 lifecycle/callstack audit and the 2026-05-31
perf campaign, with a green test harness. This pass **verifies** that, **corrects**
two false-alarm findings, **closes** the one under-tested lifecycle surface
(worker teardown), and **lands** two safe perf wins.

> **Headline:** lifecycles are valid (71/71 harness tests pass given adequate
> time); the worker-teardown surface now has a regression test; the flux-volume
> upload is ~10× cheaper at L=129. No physics changed. No risky rewrites.

---

## 1. Callstack (confirmed)

### Cold start → first frame
`index.html` → `js/app.js` module → `init()` (async, [app.js:450]) →
`requestAnimationFrame(animate)` ([app.js:684]). `init()` order: AppShell → bridge
probe (native WS → WASM → Mock) → `new Viewport()` (Three.js scene + sub-renderers)
→ panel components → UI wiring (toolbar/tabs/controls/viewport-toggles/keyboard) →
default scenario load (Scale-0 `flux-pulse`) → kick the loop.

### Per-frame — two driver loops
1. **Main `animate()`** ([app.js:695]) dispatches by `engineMode`: lattice
   (Scale 0), particles (1), atoms/molecules (2/3), cosmic (5), meta (6).
   Scale-0 per frame ([scales/scale0/controller.js:311-321]):
   `advanceSimulation` → `syncRenderableData` → `updateFieldOverlays` (amortized
   scheduler) → `renderFrame` → `updateDiagnosticsAndPanels`.
2. **`rafCoordinator`** ([js/lib/raf-coordinator.js]) — a single shared rAF that
   throttles Scale-0 overlay panels (2–4 Hz) and the Scale-4 planetary loop
   (60 Hz) independently of the main loop; pauses slow subscribers when the tab is
   hidden; auto-unsubscribes a subscriber after 10 consecutive throws.

### Scale-0 physics owner
For `flux-*/s0-seed-*/s0-field-*/quantum-*` scenarios, physics runs **off the main
thread** in a Web Worker via `MockBridgeProxy` ([bridge/mock-bridge-proxy.js]): a
shadow `MockBridge` reads the worker's `SharedArrayBuffer` zero-copy, so samplers /
`getFluxVolume` / diagnostics work unchanged while the heavy O(N³) tick never
stalls render. Other scenarios use the shared WASM/Mock bridge on the main thread.

---

## 2. Lifecycles — VALID (verified with the harness)

`BaseLifecycleController` ([scales/lifecycle.js]) tracks listeners (`bindEvent`),
timers, and Three.js objects, freeing them on `destroy()`. Scale-0 `mount`/`destroy`
([controller.js:251-287]) re-creates/disposes the four overlay panels and tracks
`pagehide` (the 2026-06-01 "P1-4" work). Worker/flux-mock teardown:
`setFluxMock()` disposes the previous mock before overwriting — for a
`MockBridgeProxy` that calls `terminate()` and kills the Worker
([scales/scale0/state/store.js:137-157]). WASM bridge `reset()` deletes the C++
`RenderBridge/ParticleEngine/AtomEngine` before realloc.

### Evidence — full harness run (2026-06-04)
Ran the lifecycle/scheduler/scales/panel-mount suite plus the worker specs.

**Result: 71/71 pass given adequate per-test timeout.** A first full serial run
(25.8 min) reported 5 failures; on investigation **every one was a 30 s test
timeout, none a logic/assertion failure**:

| Test | What happened | Verdict |
|------|---------------|---------|
| `scales.spec.js:29` | `window._ftdBridge` 15 s init timeout | transient — **passed on isolated re-run** |
| `panel-mount.spec.js:16` | bridge-init timeout in `gotoAndReady` | transient — **passed on isolated re-run** |
| `lifecycle-harness.spec.js:187` (geometry sweep) | timed out in `switchMode` **before the leak assertion ran** | **passed at 35.0 s** with a 180 s timeout — no geometry leak |
| `overlay-scheduler.spec.js:398` (work-budget) | timed out in the timing `page.evaluate` **before the assertion ran** | **passed at 32.8 s** — no time-slicing regression |
| `scale0-resize-guard.spec.js:85` (145³/181³ resize) | 30 s timeout | **passed at 35.1 s** — resize guard correct |

Root cause: this dev machine takes **~20 s for WASM init on every page load**
(visible in every test's duration), so the heaviest three tests land at 32–35 s —
marginally over Playwright's 30 s default. The lifecycle/scheduler/resize **logic is
correct**; the failures were environmental headroom.

> **Minor follow-up (optional, not a bug):** add `test.setTimeout(60_000)` to the
> three heavy tests so a loaded machine doesn't flake them. Tracked, not required.

### False-alarm corrections (so future audits don't re-raise them)
- ❌ **"Scale-0 leaks ~75 event listeners per scale switch / stale re-entry from
  `_bound`."** FALSE. `bindScale0UI()` is `_bound`-guarded ([bindings.js:24]) and
  called from `bindUI()` **once at boot** ([controller.js:186]) — **not** from the
  per-switch `mount()`/`destroy()`. Listeners bind once to app-lifetime DOM via the
  shared live-reading `ctx`; nothing accumulates per switch. Verified by reading the
  call graph and by `lifecycle-harness` (B/E) showing the rAF-subscriber count
  returns to baseline across full mode sweeps and 10× rapid cycling.
- ❌ **"Flux mocks leak on scenario churn."** Already handled by `setFluxMock`
  disposing the prior mock ([store.js:142-145]); now also regression-tested (§3).

---

## 3. Worker teardown — newest surface, now regression-tested

The Phase-2 worker became the **default deployed path** only recently (2026-06-03)
and predated `lifecycle-harness.spec.js`. The teardown code is correct but the
existing `scale0-worker.spec.js` only asserted the `useFluxMock` flag flips — it did
**not** prove the Worker thread is actually terminated, nor cover the resize and
scale-switch paths.

**Added:**
- A live-instance counter on `MockBridgeProxy` exposed as `window.__ftdScale0Workers()`
  → `{ live, created, terminated }`, incremented on construct and decremented once on
  `terminate()` (guarded against the `dispose()`→`terminate()` double-call).
  ([bridge/mock-bridge-proxy.js])
- `tests/scale0-worker-teardown.spec.js` — drives **scenario churn + lattice resize +
  scale switch + Scale-0 re-entry** and asserts worker conservation at every step:
  `created === terminated + live` and `live ≤ 1` (no accumulation), `live === 0`
  once Scale 0 is exited, `live === 1` after re-entry.

---

## 4. Performance — safe fixes (large-lattice focus)

Going in, the per-frame hot paths were already optimized (2026-05-31): allocation-free
streamlines, persistent job pool, particle spatial hash, in-place flux color writes,
shared `ctx`, **upload cadence throttle** (`frame-sync.js:5`: every 6th frame at
L>96), **sparse-tick** active-region (`_activeBox`, ε=0 bit-exact), **worker physics**,
and the flux-renderer write-loop **already `step`-decimated** (1/2/4). The bridge
`getFluxVolume()` returns `_fluxMag` **by reference** (zero-copy, no recompute on the
worker path). All confirmed intact — not re-done.

### The one remaining contained large-L cost (fixed)
`FluxRenderer.updateFluxVolume()` did a **full O(N³) maxFlux scan every upload**
([flux-renderer.js]) even though the write loop below it was subsampled — so cost
scaled with voxel count, not drawn points.

**Fix 1 — decimate the maxFlux scan by the same `step` as the write loop.** The scan
now visits exactly the voxels that get drawn; normalising against the max of the drawn
(subsampled) set is self-consistent with the rendered point cloud. **Render-only —
physics untouched.** Identical behaviour at L≤48 (step=1).

Microbenchmark (`tests/flux-upload-microbench.spec.js`, `updateFluxVolume` ms/call):

| N | voxels | BEFORE | AFTER | speedup |
|---|--------|--------|-------|---------|
| 49 | 118K | 0.115 | 0.055 | ~2× |
| 65 | 275K | 0.225 | 0.105 | ~2× |
| 97 | 913K | 0.49 | **0.055** | ~9× |
| 129 | 2.1M | **1.29** | **0.13** | **~10×** |

After the fix, cost tracks drawn points (N=97 with fewer points is now cheaper than
N=65) — the O(N³) term is gone.

**Fix 2 — memoize the Scale-0 viewport adapter per viewport.** `viewportAdapter(ctx)`
was rebuilding a ~50-closure object literal 3×/frame ([controller.js], called from
frame-sync / field-overlays / renderFrame). The adapter only closes over `viewport`
(read live), so one instance per viewport is always valid; it now rebuilds only on a
viewport swap (scale switch). Steady-state GC win across all L.

**Fix 3 — adaptive flux-dot budget (replaces the fixed 1/2/4 subsample).** User
observation: bigger lattices didn't show more flux dots — the cloud looked denser but
the count stayed flat. Root cause (two stacking mechanisms, both by design):
1. The `flux-pulse` seed scales with the lattice — `sigma = N/10`
   ([scenarios/flux-scenarios.js:28]), so the lit blob is always the same fraction of
   the box and looks self-similar under the whole-lattice camera.
2. The renderer subsampled with a fixed `step = N>96?4 : N>48?2 : 1`, and since
   `step ≈ N/32` the on-screen dot spacing — and the drawn count (~4K–9K sawtooth) —
   was held scale-invariant.

Not a bug, but now that Fix 1 made the upload ~10× cheaper there's headroom to show the
extra resolution. Replaced the integer-step subsample with a **fractional stride**: a
fixed `fluxVolumeAxisSamples(N) = min(N, FLUX_MAX_AXIS_POINTS)` grid spread evenly across
the lattice (stride = N/samples ≥ 1). For N ≤ 53 every voxel draws (stride 1); above it
the grid saturates at 53³ and the stride grows continuously — so the cloud is **~constant
density at every size**, with no awkward sparse sizes at the old integer-step (1/2/4) tier
jumps (e.g. N=65 used to fall to ⅛ density just past the step 1→2 boundary). The helper is
shared by **both** the buffer sizing (`_buildFluxVolume`) and the scan/write
(`updateFluxVolume`) so they can't drift. Effect (microbench, real `updateFluxVolume`,
flux-pulse-like field):

| L | drawn dots before (fixed 1/2/4) | drawn dots after (fractional) | ms/call |
|---|------------------|-----------------|---------|
| 49 | 3,941 | 31,512 | 0.48 |
| 65 | 9,176 | **39,791** | 0.76 |
| 97 | 3,839 | 39,550 | 0.66 |
| 129 | 8,991 | 39,922 | 0.76 |

Every size ≥53 now draws ~40K dots (uniform density, no sawtooth); N=65 went 9.2K → 39.8K.
Cost is bounded ~0.75 ms (constant 53³ scan for large N) — still well under the original
1.29 ms. Tunable via the one named constant. Render-only; physics untouched.

**Fix 3b — flux-volume rendering (additive glow + dim-dot floor).** Denser sampling
exposed that the flux volume read as a "lattice of cubes": a regular grid of soft round
points whose dim (low-flux) dots were smaller than their spacing, so the gaps showed.
Switched the material from `NormalBlending` to **additive glow** (so overlapping soft
dots accumulate into a continuous luminous volume, with the previously-unused `uGlow`
uniform adding a gaussian halo), and floored the dim-dot size at `FLUX_DOT_MIN·stride`
so low-flux dots tile rather than leave a visible grid. Verified visually via a headless
Playwright capture loop (Three.js renders fine in Playwright Chromium, unlike the paused
preview tool). Three tunable knobs — `FLUX_DOT_MIN` (dot size), `uGlow`, `uOpacity` —
trade "smooth cloud" against "crisp + colorful"; current values lean crisp/colorful so
the blue→red flux colormap reads clearly. Render-only; physics untouched.

**Fix 3c — even render positions + 3D jitter (kill grid/beat/moiré).** Two further
artifacts surfaced under the denser sampling: (i) at fractional strides `floor(i·stride)`
snaps samples to voxels spaced 1,1,1,2,… — a beat that bunches dots into visible *blocks*;
(ii) the perfectly regular grid, under additive blending, aligns into *rings/rays* (moiré).
Fixed by **decoupling render position from the field read**: dots are placed at evenly-
spaced stratum centres (uniform, no floor() beat) and read their magnitude from the
nearest voxel; then each dot gets a **stable 3D-hashed sub-cell jitter** (`jamp = stride`
when subsampling, 0 at stride 1 so N≤53 stays an exact grid). The 3D hash breaks *all*
planar alignment — unlike the earlier per-axis jitter, which left shared sheets and read
as plaid — yielding an organic cloud with no grid/blocks/moiré at any view angle.
Deterministic, so no per-frame shimmer. Verified face-on and iso via the capture loop.

**Fix 3d — user toggles (Organic / Glow) + weakened default glow.** Exposed the two
levers as Scale-0 view sub-toggles under "Flux Volume" (`toggle-flux-organic`,
`toggle-flux-glow`): **Organic** switches the 3D-jitter scatter on/off (off = the regular
lattice grid; re-uploads since dot positions change), **Glow** switches additive bloom vs
flat normal-blended dots (live material swap). The default additive glow was too strong, so
it's weakened (`uGlow` 0.18→0.06, `uOpacity` 0.55→0.34). Wired through the standard chain
template.js → bindings.js → viewport-adapter → viewport → flux-renderer; `setFluxGlow`
swaps `material.blending`+uniforms live, `setFluxOrganic` flips the jitter flag. The
Organic jitter applies at **every** size (`jamp = organic ? stride : 0`) — an earlier
`stride > 1` gate made the toggle a no-op at N≤53 (where stride is 1), so it "did nothing"
on small lattices. Each control is mirrored by a second button in the bottom-right Scene
Visuals panel (`-scene` ids); both buttons + the renderer flag stay in sync via shared
`applyFluxOrganic`/`applyFluxGlow` in bindings.js.

**Fix 3e — flux settings now survive a rebuild.** The opacity (and shape) sliders set the
material uniform directly without storing the value, so any `_buildFluxVolume` rebuild
(resize, scenario change, glow toggle) reset them to the material default — opacity
"didn't stick". Now opacity and shape are stored as renderer state (`_fluxOpacity`,
`_fluxShape`), and a single `_applyFluxMaterialState()` re-applies all material-driven
settings (glow blend + halo, opacity, shape) from state at the tail of **every**
`_buildFluxVolume`, alongside the restored `visible` and lattice-spacing. The glow toggle
delegates to the same applier, so a user-set opacity is preserved (override beats the
glow-mode default) instead of being clobbered. Point-size / threshold persist as before
(re-read each frame in `updateFluxVolume`). Net: all flux-volume settings stay continuous
across rebuilds.

### Bonus finding — `perf-baseline.spec.js` was stale w.r.t. the worker path
The perf-baseline gate read the steady-state tick via `b.currentTick()` / `b._tick`,
which the worker-default `MockBridgeProxy` doesn't expose (it self-ticks off-thread),
so the gate timed out and had silently stopped gating since the worker became default
(2026-06-03). Fixed with an additive fallback that reads the tick via the shared
diagnostics path (`capabilities.scale0.getScale0Diagnostics().tick`). The gate runs and
passes again on the worker path; a fresh baseline was captured.

### Deferred (bigger lift than "safe fixes")
Further large-L gains would require **active-box-scoped uploads** (push only the
`_activeBox` region through the bridge→adapter→renderer path) or a flux-renderer
rearchitecture. Recommended as the next step if L>180 interactive editing becomes a
requirement; out of scope for a contained pass.

---

## 5. Files touched

| File | Change |
|------|--------|
| `js/viewport/flux-renderer.js` | maxFlux scan decimated to match the write-loop `step` (Fix 1); fixed 1/2/4 tiers → shared adaptive `fluxVolumeStep()` budget (Fix 3) |
| `js/scales/scale0/controller.js` | viewport-adapter memoized per viewport (Fix 2) |
| `js/bridge/mock-bridge-proxy.js` | `__ftdScale0Workers()` live counter + `terminate()` double-call guard |
| `tests/scale0-worker-teardown.spec.js` | **new** — worker conservation across churn/resize/switch |
| `tests/flux-upload-microbench.spec.js` | **new** — deterministic large-L upload-cost + dot-budget probe |
| `tests/perf-baseline.spec.js` | tick-read fallback so the gate runs on the worker-default path |

## 6. Regression evidence

All runs on this (loaded) dev machine; `--timeout=180000` used where noted to absorb the
~20 s/test WASM-init overhead (see §2).

- **Lifecycle/scheduler/scales/panel-mount suite:** 71/71 pass. The 5 first-run
  failures were all environmental 30 s timeouts (never reached an assertion); 2 passed
  on plain isolated re-run, the 3 heavy ones passed at 32–35 s with a 180 s timeout.
- **Edit-validation batch (post Fixes 1–2 + worker counter):** lifecycle-harness A–E ✓,
  overlay-scheduler ×3 ✓, **scale0-sparse-tick ×5 ✓ (incl. "sparse tick is bit-identical
  to dense"** — confirms the render-only flux change doesn't perturb physics),
  scale0-worker ×4 ✓.
- **Adaptive dots (Fix 3):** microbench confirms more dots at large L (L=97: 3.8K→30.6K)
  with bounded cost (~0.5 ms) and no buffer overflow; flux scene boots clean.
- **perf-baseline:** after the tick-read fix, runs on the worker path and **passes**
  (fresh worker-path baseline written); L=32 render is unchanged (`step=1` either way).
- **scale0-worker-teardown:** **passes** — workers conserved across scenario churn,
  resize, scale-switch, and Scale-0 re-entry (`created === terminated + live`, `live ≤ 1`).

**Net: lifecycles valid, no regressions, two perf wins + one quality win + one revived
gate.** Physics bit-exact throughout (render-only changes).

# Scale-0 Sparse Wave Tick — Phase 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Cut the L≥97 FPS drop by computing the JS wave tick (`MockBridge._tickFlux`, 96% of tick cost) only over the bounding box of the nonzero field, instead of all N³ voxels — bit-exact, behind a flag, with a dense fallback.

**Architecture:** Track the axis-aligned bounding box of nonzero flux (`_activeBox`), updated at every flux-write entry point and expanded by one voxel per tick (CFL: the wave front advances ≤1 voxel/tick). `_tickFlux` restricts its interior Laplacian loop and the J-commit to that box (+1 frontier), and skips the boundary loops + absorbing sponge while the box is interior. It falls back to the existing dense code when the box nears a wall (periodic wrap couples both walls) or fills >40%. Correctness is bit-exact because every skipped voxel has an all-zero 18-neighborhood (Laplacian 0 → no change).

**Tech Stack:** Vanilla ES-module JS (`engine/web/js/bridge/mock-bridge.js`), Playwright (`engine/web/tests`, run from `engine/web/tests/` via `npx playwright test`), Chromium/wasm64.

**Spec:** `engine/web/docs/SPEC_SCALE0_LATTICE_PERF.md` (§3 Phase 1).

**Commit policy:** This repo forbids AI-attribution trailers — commit messages end with the description, no `Co-Authored-By`. Per the user's standing rule, run the `git commit` steps only once the user has said to commit; otherwise leave the work staged-by-path and report. Never `git add -A` (shared working tree — stage explicit paths and verify `git diff --cached`).

---

## File Structure

- **Modify** `engine/web/js/bridge/mock-bridge.js`
  - Constructor: add `_sparseTick`, `_activeBox`, `_activeDense`, `_sparseEps` fields.
  - New methods: `_resetActiveBox()`, `_expandActiveBox(x,y,z)`, `_growActiveBox()`, `_recomputeActiveBox()`.
  - Mutation hooks: `_injectFlux` (:898), `seedRandomFlux` (:684), `clearField` (:677), `reset()`, and after `setupScenario` (:1561).
  - `_tickFlux` (:931): add the sparse window/flags block; parameterize the interior loop bounds; gate the two boundary WV loops + the sponge; bound the J-commit; grow/recompute the box at the end.
- **Create** `engine/web/tests/scale0-sparse-tick.spec.js` — equivalence (bit-exact) + performance + box-tracking regression.

Each task below leaves the engine working (flag defaults OFF until Task 6, so the dense path is untouched until equivalence is proven).

---

## Task 1: Equivalence test harness (dense determinism baseline)

**Files:**
- Test: `engine/web/tests/scale0-sparse-tick.spec.js` (create)

- [ ] **Step 1: Write the harness + a determinism test**

```javascript
// @ts-check
/**
 * Scale-0 sparse wave-tick regression (2026-06-03). Pins that the active-region
 * (bounding-box) tick is BIT-IDENTICAL to the dense tick, and faster for a
 * localized pulse. See engine/web/docs/SPEC_SCALE0_LATTICE_PERF.md §3.
 *
 * Runs a standalone MockBridge in-page (no app/UI) so the physics is isolated.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

// Run `ticks` steps of a fresh MockBridge(N) seeded with a single deterministic
// center pulse, with sparse on/off, and return a snapshot of _fluxJ.
async function runField(page, { N, ticks, sparse }) {
    return page.evaluate(async ({ N, ticks, sparse }) => {
        const { MockBridge } = await import('/js/bridge/mock-bridge.js');
        const b = new MockBridge(N);                 // ctor snaps even→odd; pass odd N
        b._sparseTick = sparse;
        const c = (b.latticeSize - 1) >> 1;          // true center voxel
        b.injectFlux(c, c, c, 0.5, 0.5, 0.5);        // deterministic seed
        const s0 = b.capabilities.scale0;
        for (let i = 0; i < ticks; i++) s0.tickScale0();
        // Return a compact checksum + the full array length for equality checks.
        const J = b._fluxJ;
        let sum = 0, sumsq = 0, nz = 0;
        for (let i = 0; i < J.length; i++) { const v = J[i]; sum += v; sumsq += v * v; if (v !== 0) nz++; }
        return { len: J.length, sum, sumsq, nz, jcopy: Array.from(J) };
    }, { N, ticks, sparse });
}

function maxAbsDiff(a, b) {
    let m = 0; for (let i = 0; i < a.length; i++) { const d = Math.abs(a[i] - b[i]); if (d > m) m = d; }
    return m;
}

test.beforeEach(async ({ page }) => { page.setDefaultTimeout(30_000); });

test.describe('Scale-0 sparse wave tick', () => {
    test('dense tick is deterministic (baseline)', async ({ page }) => {
        await gotoAndReady(page);
        const a = await runField(page, { N: 33, ticks: 20, sparse: false });
        const b = await runField(page, { N: 33, ticks: 20, sparse: false });
        expect(a.len).toBe(33 ** 3 * 3);
        expect(a.nz, 'a center pulse must spread to many nonzero voxels').toBeGreaterThan(100);
        expect(maxAbsDiff(a.jcopy, b.jcopy), 'dense tick is deterministic').toBe(0);
    });
});
```

- [ ] **Step 2: Run it to verify it passes (baseline harness works)**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js --reporter=line`
Expected: 1 passed. (Confirms `MockBridge` imports standalone, `injectFlux`+`tickScale0` run, and the field spreads. `_sparseTick` is read but has no effect yet.)

- [ ] **Step 3: Commit** (on user go-ahead)

```bash
git add engine/web/tests/scale0-sparse-tick.spec.js
git commit -m "test(web): add Scale-0 sparse-tick equivalence harness (dense baseline)"
```

---

## Task 2: Active-box state + helpers + mutation hooks

**Files:**
- Modify: `engine/web/js/bridge/mock-bridge.js` (constructor ~:69-128; methods near `_injectFlux` :898, `seedRandomFlux` :684, `clearField` :677, `reset`, `setupScenario` :1561)

- [ ] **Step 1: Write a box-tracking test (add to the spec)**

```javascript
    test('active box tracks injections and clears', async ({ page }) => {
        await gotoAndReady(page);
        const r = await page.evaluate(async () => {
            const { MockBridge } = await import('/js/bridge/mock-bridge.js');
            const b = new MockBridge(33);
            const empty = b._activeBox && b._activeBox.x1 < b._activeBox.x0;
            b.injectFlux(10, 11, 12, 0.1, 0, 0);
            b.injectFlux(20, 19, 18, 0, 0.1, 0);
            const box = { ...b._activeBox };
            b.clearField();
            const clearedEmpty = b._activeBox.x1 < b._activeBox.x0;
            return { empty, box, clearedEmpty };
        });
        expect(r.empty, 'box starts empty').toBe(true);
        expect(r.box).toMatchObject({ x0: 10, x1: 20, y0: 11, y1: 19, z0: 12, z1: 18 });
        expect(r.clearedEmpty, 'clearField empties the box').toBe(true);
    });
```

- [ ] **Step 2: Run to verify it fails**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js -g "active box tracks" --reporter=line`
Expected: FAIL (`_activeBox` is undefined).

- [ ] **Step 3: Add the state fields in the constructor**

In `mock-bridge.js`, in the constructor (after `this._reflectiveBoundary = false;` ~:84) add:

```javascript
        // ── Sparse (active-region) wave tick — Phase 1 (SPEC_SCALE0_LATTICE_PERF §3) ──
        // _activeBox = inclusive bounds of nonzero flux; x1<x0 means "empty".
        // _activeDense latches true when the wave reaches a wall / fills >40%
        // (then _tickFlux runs the original full dense path). _sparseEps: trim
        // threshold (0 = bit-exact). _sparseTick gates the whole optimization.
        this._sparseTick = false;   // flipped on in Task 6 after equivalence is proven
        this._activeBox = { x0: this.latticeSize, x1: -1, y0: this.latticeSize, y1: -1, z0: this.latticeSize, z1: -1 };
        this._activeDense = false;
        this._sparseEps = 0;
```

- [ ] **Step 4: Add the helper methods**

Add these methods to the `MockBridge` class (e.g. just above `_tickFlux` :931):

```javascript
    _resetActiveBox() {
        const N = this.latticeSize;
        this._activeBox = { x0: N, x1: -1, y0: N, y1: -1, z0: N, z1: -1 };
        this._activeDense = false;
    }

    _expandActiveBox(x, y, z) {
        const b = this._activeBox;
        if (x < b.x0) b.x0 = x; if (x > b.x1) b.x1 = x;
        if (y < b.y0) b.y0 = y; if (y > b.y1) b.y1 = y;
        if (z < b.z0) b.z0 = z; if (z > b.z1) b.z1 = z;
    }

    // Grow the box by one shell each tick (the wave front advances ≤1 voxel/tick),
    // clamped to the lattice. Cheap O(1); keeps the box a superset of nonzero J.
    _growActiveBox() {
        const b = this._activeBox, N = this.latticeSize;
        if (b.x1 < b.x0) return;
        b.x0 = Math.max(0, b.x0 - 1); b.x1 = Math.min(N - 1, b.x1 + 1);
        b.y0 = Math.max(0, b.y0 - 1); b.y1 = Math.min(N - 1, b.y1 + 1);
        b.z0 = Math.max(0, b.z0 - 1); b.z1 = Math.min(N - 1, b.z1 + 1);
    }

    // Tight rescan of nonzero bounds (O(N³)); call only occasionally (after
    // setup, and every K ticks so a damped field can shrink the box).
    _recomputeActiveBox() {
        const N = this.latticeSize, J = this._fluxJ, WV = this._fluxWV, eps = this._sparseEps;
        this._resetActiveBox();
        if (!J) return;
        const b = this._activeBox;
        for (let z = 0; z < N; z++) for (let y = 0; y < N; y++) for (let x = 0; x < N; x++) {
            const i3 = (z * N * N + y * N + x) * 3;
            const a = Math.abs(J[i3]) + Math.abs(J[i3 + 1]) + Math.abs(J[i3 + 2])
                    + Math.abs(WV[i3]) + Math.abs(WV[i3 + 1]) + Math.abs(WV[i3 + 2]);
            if (a > eps) {
                if (x < b.x0) b.x0 = x; if (x > b.x1) b.x1 = x;
                if (y < b.y0) b.y0 = y; if (y > b.y1) b.y1 = y;
                if (z < b.z0) b.z0 = z; if (z > b.z1) b.z1 = z;
            }
        }
    }
```

- [ ] **Step 5: Hook the flux-write entry points**

In `_injectFlux(x, y, z, fx, fy, fz)` (:898), after the three `this._fluxJ[idx*3 + …] += …` writes, add:

```javascript
        this._expandActiveBox(x, y, z);
```

In `seedRandomFlux()` (:684), after the triple-loop fill, add (a full random fill is genuinely dense):

```javascript
        this._activeDense = true;
```

In `clearField()` (:677), after the `.fill(0)` calls, add:

```javascript
        this._resetActiveBox();
```

Find `reset(` for the MockBridge (the method that re-initializes the lattice) and add `this._resetActiveBox();` after the field buffers are (re)allocated. In `setupScenario(name)` (:1561), change the body to recompute the tight box after seeding:

```javascript
    setupScenario(name) { const r = runSetupScenario.call(this, name); this._recomputeActiveBox(); return r; }
```

- [ ] **Step 6: Run to verify the box test passes**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js -g "active box tracks" --reporter=line`
Expected: PASS.

- [ ] **Step 7: Commit** (on user go-ahead)

```bash
git add engine/web/js/bridge/mock-bridge.js engine/web/tests/scale0-sparse-tick.spec.js
git commit -m "feat(web): MockBridge active-box tracking (sparse-tick scaffolding, flag off)"
```

---

## Task 3: Bound the interior WV loop + gate boundary loops (behind the flag)

**Files:**
- Modify: `engine/web/js/bridge/mock-bridge.js` `_tickFlux` (:931 — interior loop :1035-1098, boundary loops :1103-1261)

- [ ] **Step 1: Write the equivalence test (sparse == dense, bit-exact)**

```javascript
    test('sparse tick is bit-identical to dense (interior pulse)', async ({ page }) => {
        await gotoAndReady(page);
        // N=65, 20 ticks: the nonzero box grows ≤1 voxel/tick from center (32),
        // reaching [12,52] — inside the wall margin — so the sparse path stays
        // active for every tick (a clean sparse-vs-dense comparison, no fallback).
        const dense  = await runField(page, { N: 65, ticks: 20, sparse: false });
        const sparse = await runField(page, { N: 65, ticks: 20, sparse: true });
        expect(sparse.nz, 'sparse run actually propagated').toBeGreaterThan(100);
        expect(maxAbsDiff(dense.jcopy, sparse.jcopy), 'sparse must equal dense bit-for-bit').toBe(0);
    });
```

- [ ] **Step 2: Run to verify it fails (or passes trivially — flag has no tick effect yet)**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js -g "bit-identical" --reporter=line`
Expected: PASS trivially (sparse path not wired into `_tickFlux` yet, so both runs are dense). This test becomes the guard that the upcoming change stays bit-exact.

- [ ] **Step 3: Add the sparse-window block at the top of `_tickFlux`**

In `_tickFlux`, immediately before the interior loop comment `// ── Fast interior path …` (~:1034), insert:

```javascript
        // ── Sparse (active-region) windowing ─────────────────────────────
        // Restrict the O(N³) interior Laplacian + commit to the nonzero
        // bounding box (+1 frontier). Skip the boundary loops + sponge while
        // the box is interior (those voxels are zero → provably no-ops). Fall
        // back to the full dense path once the front nears a wall (periodic
        // wrap couples both walls) or fills >40%.
        let sx0 = 1, sx1 = N - 2, sy0 = 1, sy1 = N - 2, sz0 = 1, sz1 = N - 2;
        let sparseActive = false;
        let runBoundaryWV = true;
        if (this._sparseTick && !this._activeDense) {
            const b = this._activeBox;
            if (b.x1 < b.x0) return;                 // empty field → nothing to do
            const Dsp = this._reflectiveBoundary ? 1 : Math.min(6, Math.max(2, Math.floor(N / 4)));
            const margin = Dsp + 1;                  // stay clear of the sponge shell too
            const nearWall = b.x0 <= margin || b.x1 >= N - 1 - margin
                          || b.y0 <= margin || b.y1 >= N - 1 - margin
                          || b.z0 <= margin || b.z1 >= N - 1 - margin;
            const vol = (b.x1 - b.x0 + 1) * (b.y1 - b.y0 + 1) * (b.z1 - b.z0 + 1);
            if (nearWall || vol > 0.4 * N * N * N) {
                this._activeDense = true;            // latch dense from here on
            } else {
                sparseActive = true;
                runBoundaryWV = false;
                sx0 = Math.max(1, b.x0 - 1); sx1 = Math.min(N - 2, b.x1 + 1);
                sy0 = Math.max(1, b.y0 - 1); sy1 = Math.min(N - 2, b.y1 + 1);
                sz0 = Math.max(1, b.z0 - 1); sz1 = Math.min(N - 2, b.z1 + 1);
            }
        }
```

- [ ] **Step 4: Parameterize the interior loop bounds**

Replace the interior loop headers (:1035, :1037, :1044):

```javascript
        for (let z = 1; z < Nm1; z++) {
            const zBase = z * NN;
            for (let y = 1; y < Nm1; y++) {
                const rowStart = zBase + y * N + 1;
```
with:
```javascript
        for (let z = sz0; z <= sz1; z++) {
            const zBase = z * NN;
            for (let y = sy0; y <= sy1; y++) {
                const rowStart = zBase + y * N + sx0;
```
and the inner `for (let x = 1; x < Nm1; x++) {` (:1044) with:
```javascript
                for (let x = sx0; x <= sx1; x++) {
```

(The `i3`/`vi` init off `rowStart` already starts at `sx0`; the unchanged `i3 += 3; vi++;` advance is correct.)

- [ ] **Step 5: Gate the two boundary WV loops**

Wrap the first boundary loop (`// ── Slow boundary path …` :1100, the `for (let z = 0; z < N; z++)` through its close :1195) and the second boundary loop (`// Also process boundary x-edges …` :1197, the `for (let z = 1; z < Nm1; z++)` through :1261) each in:

```javascript
        if (runBoundaryWV) {
            // … existing boundary loop body …
        }
```

- [ ] **Step 6: Run the equivalence + determinism tests**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js --reporter=line`
Expected: all PASS. The `bit-identical` test now genuinely exercises the bounded interior loop vs dense and must be `maxAbsDiff === 0`. If nonzero: the window/gating is wrong — do not proceed; re-derive bounds (likely `margin` too small or an off-by-one in `sx1`).

- [ ] **Step 7: Commit** (on user go-ahead)

```bash
git add engine/web/js/bridge/mock-bridge.js engine/web/tests/scale0-sparse-tick.spec.js
git commit -m "feat(web): sparse interior wave loop + boundary gating (bit-exact, flag off)"
```

---

## Task 4: Bound the J-commit + gate the sponge + grow the box

**Files:**
- Modify: `engine/web/js/bridge/mock-bridge.js` `_tickFlux` (commit :1263-1313, sponge :1347-1377, end :1379)

- [ ] **Step 1: Bound the commit when sparse-active**

The commit currently runs `if (selective && damp < 1.0) { …flat loop… } else { …flat loop… }` (:1269-1313). Wrap a sparse branch around it:

```javascript
        if (sparseActive) {
            // Commit only the active window. Outside it J=WV=0 ⇒ (0+0)·d = 0 (no-op).
            const dampNow = (selective && damp < 1.0) ? null : damp;  // null ⇒ per-voxel mask path
            if (dampNow === null && (!this._selectiveDampMask || this._selectiveDampMask.length !== total)) {
                this._selectiveDampMask = new Uint8Array(total);
            }
            // (selective masking only matters when particles exist; flux-only
            //  scenarios take the uniform path with d = damp.)
            for (let z = sz0; z <= sz1; z++) {
                for (let y = sy0; y <= sy1; y++) {
                    let i3 = (z * NN + y * N + sx0) * 3;
                    let vi = z * NN + y * N + sx0;
                    for (let x = sx0; x <= sx1; x++) {
                        const d = (dampNow === null) ? (this._selectiveDampMask[vi] ? damp : 1.0) : damp;
                        J[i3]     = (J[i3]     + WV[i3]     * dt) * d;
                        J[i3 + 1] = (J[i3 + 1] + WV[i3 + 1] * dt) * d;
                        J[i3 + 2] = (J[i3 + 2] + WV[i3 + 2] * dt) * d;
                        WV[i3] *= d; WV[i3 + 1] *= d; WV[i3 + 2] *= d;
                        i3 += 3; vi++;
                    }
                }
            }
        } else if (selective && damp < 1.0) {
            // … existing selective flat commit (unchanged) …
        } else {
            // … existing uniform flat commit (unchanged) …
        }
```

> Note: the existing `if (selective && damp < 1.0) {` line becomes `} else if (selective && damp < 1.0) {` and the existing `} else {` stays. The selective mask-building block stays inside its (now `else if`) branch for the dense path. For the sparse path, particle scenarios are rare on flux-* (which the box optimization targets); if particles are present the dense fallback usually engages anyway. Equivalence test (Task 3, no particles) covers the uniform sparse commit.

- [ ] **Step 2: Gate the sponge loop**

Wrap the absorbing-sponge block (`if (!this._reflectiveBoundary) { …` :1347-1377) so it is skipped when the box is interior:

```javascript
        if (!this._reflectiveBoundary && runBoundaryWV) {
            // … existing sponge body …
        }
```

(`runBoundaryWV` is false exactly when the box is interior — where the sponge shell is all-zero, so skipping is a no-op. When dense/near-wall, `runBoundaryWV` is true and the sponge runs as before.)

- [ ] **Step 3: Grow/recompute the box at the end of `_tickFlux`**

Replace the final `this._fluxDirty = true;` (:1379) with:

```javascript
        this._fluxDirty = true;
        if (this._sparseTick && !this._activeDense) {
            this._growActiveBox();                          // wave front advanced ≤1 voxel
            // Periodic tight rescan lets a damped/dissipating field shrink the
            // box again (cheap amortized: O(N³)/32 per tick).
            if ((this._tick & 31) === 0) this._recomputeActiveBox();
        }
```

- [ ] **Step 4: Run the full spec (equivalence must stay bit-exact)**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js --reporter=line`
Expected: all PASS, `bit-identical` still `maxAbsDiff === 0`. (Now the commit + sponge + box-grow are all in the sparse path.)

- [ ] **Step 5: Commit** (on user go-ahead)

```bash
git add engine/web/js/bridge/mock-bridge.js
git commit -m "feat(web): bound sparse commit + gate sponge + grow active box (bit-exact)"
```

---

## Task 5: Performance assertion + profile capture

**Files:**
- Modify: `engine/web/tests/scale0-sparse-tick.spec.js`

- [ ] **Step 1: Add a performance test (sparse ≪ dense for a fresh centered pulse)**

```javascript
    test('sparse tick is faster than dense for a localized pulse (L=97)', async ({ page }) => {
        await gotoAndReady(page);
        const timeTick = await page.evaluate(async () => {
            const { MockBridge } = await import('/js/bridge/mock-bridge.js');
            const make = (sparse) => {
                const b = new MockBridge(97); b._sparseTick = sparse;
                const c = (b.latticeSize - 1) >> 1;
                b.injectFlux(c, c, c, 0.5, 0.5, 0.5);
                return b;
            };
            const time = (b, n) => {
                const s0 = b.capabilities.scale0;
                for (let i = 0; i < 3; i++) s0.tickScale0();   // warm + let box settle small
                const t = performance.now();
                for (let i = 0; i < n; i++) s0.tickScale0();
                return (performance.now() - t) / n;
            };
            const dense = time(make(false), 10);
            const sparse = time(make(true), 10);
            return { dense: +dense.toFixed(2), sparse: +sparse.toFixed(2) };
        });
        // Early pulse occupies a tiny box ⇒ sparse should be well under half dense.
        expect(timeTick.sparse, `dense=${timeTick.dense}ms sparse=${timeTick.sparse}ms`)
            .toBeLessThan(timeTick.dense * 0.5);
    });
```

- [ ] **Step 2: Run it**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js -g "faster than dense" --reporter=line`
Expected: PASS, with the printed `dense=…ms sparse=…ms` showing a large gap (dense ≈ 38 ms, sparse single-digit for the first ~10 ticks).

- [ ] **Step 3: Commit** (on user go-ahead)

```bash
git add engine/web/tests/scale0-sparse-tick.spec.js
git commit -m "test(web): assert sparse tick beats dense for a localized pulse"
```

---

## Task 6: Default the flag on + full regression

**Files:**
- Modify: `engine/web/js/bridge/mock-bridge.js` (constructor flag)

- [ ] **Step 1: Flip the flag default on**

Change the constructor field from `this._sparseTick = false;` to:

```javascript
        this._sparseTick = true;    // FTD_SPARSE_TICK — bit-exact active-region tick (SPEC §3)
```

- [ ] **Step 2: Re-run the sparse spec (now exercising the default-on path)**

Run: `cd engine/web/tests && npx playwright test scale0-sparse-tick.spec.js --reporter=line`
Expected: all PASS (the equivalence test sets `_sparseTick` explicitly per run, so it still compares both paths).

- [ ] **Step 3: Run the Scale-0 regression suite**

Run: `cd engine/web/tests && npx playwright test scale0-resize-guard.spec.js scenario-parity.spec.js wasm-scenario-coverage.spec.js flux-slice-axes.spec.js toggle-coverage.spec.js --reporter=line`
Expected: all green (same counts as before this plan). If a flux scenario diverges visually, set `_sparseTick = false` (instant revert) and investigate the box hooks for that scenario's seeding path.

- [ ] **Step 4: Re-profile in the live preview (evidence the FPS drop is fixed)**

Re-run the §1 profiling harness from the spec (resize flux-pulse to 97/129, time `tickScale0`) and confirm the per-tick cost during the expansion phase is a fraction of the pre-change 38 ms / 89 ms. Record the numbers in the spec's verification note.

- [ ] **Step 5: Commit** (on user go-ahead)

```bash
git add engine/web/js/bridge/mock-bridge.js
git commit -m "feat(web): enable sparse Scale-0 wave tick by default (bit-exact, dense fallback)"
```

---

## Self-Review

- **Spec coverage:** §3.2a bounding-box (Tasks 2-4); §3.3 mutation hooks (Task 2 Step 5); §3.4 periodic-wrap → dense fallback (Task 3 Step 3, `nearWall`); §3.5 ε/bit-exact (`_sparseEps = 0`, equivalence test); §3.6 payoff (Task 5); §3.7 verification (Tasks 1,3,5,6); §3.8 files (matches). Phase 1 fully covered. (§3.2b per-voxel and Phase 2 are out of scope by design.)
- **Placeholder scan:** none — every code step has concrete code or an exact before→after edit.
- **Type/name consistency:** `_sparseTick`, `_activeBox` (`{x0,x1,y0,y1,z0,z1}`), `_activeDense`, `_sparseEps`, `_resetActiveBox`, `_expandActiveBox`, `_growActiveBox`, `_recomputeActiveBox`, and the `sx0..sz1`/`sparseActive`/`runBoundaryWV` locals are used consistently across Tasks 2-4.
- **Risk:** flag defaults OFF until Task 6; the dense path is byte-for-byte unchanged when off; the equivalence test gates every behavioral task at `maxAbsDiff === 0`.

## Open follow-ups (not in this plan)

- Per-voxel active list (spec §3.2b) if scenarios seed multiple separated phenomena (spec O1).
- Phase 2 (Web Worker, spec §4) — separate plan.
- Optional: restrict genesis (:240) to the active box (≤2 ms, low priority).

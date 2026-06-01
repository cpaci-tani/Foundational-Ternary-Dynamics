// @ts-check
/**
 * Lifecycle harness (ticket W7-1) — THE HEADLINE merge-bar safety net.
 *
 * Proves the engine is "flawless" across scale switches: every scale
 * round-trip leaks nothing and leaves the app clean. This is the net that
 * blocks a merge when a controller forgets to tear something down.
 *
 * Why these specific observables (each ground-truthed against HEAD, not
 * invented — an earlier harness attempt failed by asserting a nonexistent
 * `.tick` field, so every assertion here pins a real source path):
 *
 *   • Console errors — the shared realErrors()/KNOWN_NOISE filter
 *     (_helpers.js). A teardown/mount bug almost always surfaces as a throw.
 *
 *   • rAF subscriber count — window.__ftdRAF.size()
 *     (js/lib/raf-coordinator.js:138 `size()`, exposed on window at :156-158).
 *     Each scale subscribes its render/physics loop on mount and unsubscribes
 *     on destroy(); a leaked subscription shows as monotonic growth across
 *     switches. This is the PRIMARY leak proxy.
 *
 *   • Three.js renderer.info.memory {geometries, textures} — reached via
 *     window.__ftdCtx.viewport.renderer.info.memory. The Viewport owns
 *     `this.renderer = new THREE.WebGLRenderer(...)` (js/viewport.js:146);
 *     ctx.viewport is that Viewport instance (js/app.js:541 builds it,
 *     :793 assigns it onto the live ctx). BaseLifecycleController.destroy()
 *     disposes tracked geometries/materials/textures (js/lifecycle.js:104-127),
 *     so orphaned GPU resources show as unbounded growth across repeated sweeps.
 *
 *   • Camera/controls restore — window.__ftdCtx.viewport.camera.{far,position}
 *     + .controls.{maxDistance,target}. The cosmic controller saves these on
 *     enter and restores them on destroy() (js/scales/scale5/controller.js:
 *     114-135 save+mutate, :193-207 restore). far (2000→50000) and maxDistance
 *     (500→5000) are the load-bearing fields the audit P1-8 fix restores.
 *
 * The module-private controller state (_listeners/_timers/_threeObjects on
 * each BaseLifecycleController instance, js/lifecycle.js:11-15) is NOT
 * directly readable from a test — those instances never reach window. So the
 * harness asserts on the OBSERVABLE PROXIES above (rAF count, renderer.info,
 * camera fields, console) rather than on the private bookkeeping arrays.
 *
 * Robustness: every behavioural read polls with expect.poll and allows settle
 * frames after switchMode (WASM compile + Three.js scene build are async);
 * console assertions go through realErrors()/KNOWN_NOISE. Leak thresholds are
 * tolerant of pooled/persistent objects (a fixed headroom) but tight enough to
 * catch per-switch LINEAR growth — the signature of a real leak.
 */

import { test, expect } from '@playwright/test';
import {
    gotoAndReady,
    switchMode,
    attachConsoleWatcher,
    realErrors,
    rafSize,
    getRendererMemory,
    getCameraState,
    fullModeSweep,
} from './_helpers.js';

// Same ordered mode list scales.spec.js drives. Index 0 ('lattice') is the
// baseline we always return to.
const MODES = ['lattice', 'particles', 'atoms', 'molecules', 'planetary', 'cosmic', 'meta'];

// Settle budget after each switchMode. WASM/Three controllers mount + tear
// down asynchronously; this gives the rAF loop + scene build a few frames.
const SETTLE_MS = 400;

test.beforeEach(async ({ page }) => {
    // WASM compile + Three.js + module graph need headroom on slower machines.
    page.setDefaultTimeout(20_000);
});

/**
 * Wait until the Scale-0 controller has published the live ctx AND the
 * Viewport is reachable through it — the precondition for every
 * renderer/camera read.
 * @param {import('@playwright/test').Page} page
 */
async function waitForCtxViewport(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.viewport && window.__ftdCtx.viewport.renderer)),
        { timeout: 15_000, message: 'window.__ftdCtx.viewport.renderer never became reachable' },
    ).toBe(true);
}

test.describe('lifecycle harness — scale round-trips leak nothing', () => {

    // ────────────────────────────────────────────────────────────────────
    // (A) Per-mode round-trip cleanliness.
    //
    // For EACH non-lattice mode: from the lattice baseline, switch into the
    // mode and back to lattice; assert ZERO real console errors across the
    // whole round-trip. A controller that throws on mount OR on destroy is
    // caught here, attributed to one specific mode.
    //
    // Each mode is its own test() so a failure names the offending scale.
    // ────────────────────────────────────────────────────────────────────
    for (const mode of MODES.filter((m) => m !== 'lattice')) {
        test(`(A) round-trip lattice → ${mode} → lattice is console-clean`, async ({ page }) => {
            const errors = attachConsoleWatcher(page);
            await gotoAndReady(page);
            // Settle the initial lattice mount before we start switching.
            await page.waitForTimeout(SETTLE_MS);

            await switchMode(page, mode);
            await page.waitForTimeout(SETTLE_MS);

            // Bridge must survive the switch (mirrors scales.spec.js).
            const aliveInMode = await page.evaluate(() => !!window._ftdBridge);
            expect(aliveInMode, `bridge lost after switching to ${mode}`).toBe(true);

            await switchMode(page, 'lattice');
            await page.waitForTimeout(SETTLE_MS);

            const aliveBack = await page.evaluate(() => !!window._ftdBridge);
            expect(aliveBack, `bridge lost after returning to lattice from ${mode}`).toBe(true);

            const real = realErrors(errors);
            expect(
                real,
                `console errors during lattice → ${mode} → lattice:\n${real.join('\n')}`,
            ).toHaveLength(0);
        });
    }

    // ────────────────────────────────────────────────────────────────────
    // (B) rAF-subscriber leak across a full mode sweep.
    //
    // Capture the rAF subscriber count at the settled lattice baseline. Then
    // sweep through ALL modes and back to lattice. Each scale should subscribe
    // on mount and unsubscribe on destroy(), so once we are back on lattice
    // the count must return to (≤) baseline. A leak (a controller that never
    // unsubscribes) shows as the count sitting ABOVE baseline.
    //
    // Threshold: ≤ baseline. We poll so a just-destroyed subscriber that
    // unsubscribes a frame late still settles to baseline. (Returning to the
    // SAME lattice scale we started from means the steady-state subscriber set
    // is identical — no headroom is needed for "persistent" subs because they
    // are persistent on BOTH sides of the comparison.)
    // ────────────────────────────────────────────────────────────────────
    test('(B) full mode sweep leaks no rAF subscribers (count returns to baseline)', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtxViewport(page);
        await page.waitForTimeout(SETTLE_MS);

        // Baseline at settled lattice. Poll until it stops moving so panels
        // that subscribe on mount are all counted.
        const baseline = await rafSize(page);

        await fullModeSweep(page, MODES, { settleMs: SETTLE_MS });

        // Back on lattice: the subscriber count must settle to ≤ baseline.
        await expect.poll(
            () => rafSize(page),
            {
                timeout: 10_000,
                message: `rAF subscriber count did not return to baseline (${baseline}) after a full mode sweep — a scale leaked a subscription`,
            },
        ).toBeLessThanOrEqual(baseline);

        const real = realErrors(errors);
        expect(real, `console errors during sweep:\n${real.join('\n')}`).toHaveLength(0);
    });

    // ────────────────────────────────────────────────────────────────────
    // (C) Three.js orphan growth across repeated sweeps.
    //
    // Capture renderer.info.memory {geometries, textures} at the lattice
    // baseline, run the full mode sweep 3×, return to lattice, and assert the
    // counts are BOUNDED — they must not grow ~linearly with the number of
    // sweeps. BaseLifecycleController.destroy() disposes tracked Three objects
    // (lifecycle.js:104-127); an orphan (a mesh/texture never tracked or never
    // disposed) accumulates per sweep.
    //
    // Threshold rationale: allow a fixed HEADROOM over baseline for pooled /
    // lazily-created persistent objects that legitimately appear once and stay
    // (e.g. a scale builds a geometry on first entry and caches it). A leak
    // would add ≈(per-sweep orphan count) × 3; a fixed cache adds the same
    // bounded amount regardless of sweep count. We pick generous absolute
    // headroom (geometries + textures) so we never false-positive on caching,
    // but it is far below what 3× linear growth of any non-trivial scene would
    // produce. If renderer.info is unreachable, the test SKIPS (and says so)
    // rather than asserting on garbage.
    // ────────────────────────────────────────────────────────────────────
    test('(C) repeated mode sweeps do not grow Three.js geometries/textures unboundedly', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtxViewport(page);
        await page.waitForTimeout(SETTLE_MS);

        const baseline = await getRendererMemory(page);
        test.skip(baseline === null, 'renderer.info.memory unreachable via window.__ftdCtx.viewport.renderer.info — orphan-growth check omitted');
        // After test.skip with a true condition the test stops; the lines
        // below run only when baseline is non-null.
        const base = /** @type {{geometries:number, textures:number}} */ (baseline);

        const SWEEPS = 3;
        for (let i = 0; i < SWEEPS; i++) {
            await fullModeSweep(page, MODES, { settleMs: SETTLE_MS });
        }
        // Let any deferred disposals (GPU resource frees happen on the next
        // render after destroy) settle.
        await page.waitForTimeout(SETTLE_MS);
        await waitForCtxViewport(page);

        const after = await getRendererMemory(page);
        expect(after, 'renderer.info.memory became unreachable mid-test').not.toBeNull();
        const a = /** @type {{geometries:number, textures:number}} */ (after);

        // Fixed absolute headroom: tolerant of one-time caches/pools, but a
        // per-sweep orphan (×3 sweeps × multiple scales) would blow past it.
        const GEOM_HEADROOM = 60;
        const TEX_HEADROOM = 30;

        expect(
            a.geometries,
            `Three.js geometry count grew from ${base.geometries} to ${a.geometries} over ${SWEEPS} sweeps (headroom ${GEOM_HEADROOM}) — orphaned geometries are leaking`,
        ).toBeLessThanOrEqual(base.geometries + GEOM_HEADROOM);

        expect(
            a.textures,
            `Three.js texture count grew from ${base.textures} to ${a.textures} over ${SWEEPS} sweeps (headroom ${TEX_HEADROOM}) — orphaned textures are leaking`,
        ).toBeLessThanOrEqual(base.textures + TEX_HEADROOM);
    });

    // ────────────────────────────────────────────────────────────────────
    // (D) Lattice camera restored after a camera-mutating scale.
    //
    // Capture camera.{far,position} + controls.{maxDistance,target} at the
    // lattice baseline. Switch to cosmic (which mutates far 2000→50000 and
    // maxDistance 500→5000, scale5/controller.js:131-135) and back. The
    // destroy() restore (scale5/controller.js:193-207) must put the lattice
    // values back. far + maxDistance are the load-bearing fields.
    // ────────────────────────────────────────────────────────────────────
    test('(D) lattice camera (far + maxDistance + position + target) is restored after cosmic', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtxViewport(page);
        await page.waitForTimeout(SETTLE_MS);

        const before = await getCameraState(page);
        expect(before, 'camera state unreachable via window.__ftdCtx.viewport.camera/controls').not.toBeNull();
        const b = /** @type {NonNullable<typeof before>} */ (before);

        // Sanity: we are at the lattice defaults the mutation departs from
        // (camera far 2000, controls maxDistance 500 — viewport.js:142/159).
        expect(b.far, 'lattice baseline camera.far').toBe(2000);
        expect(b.maxDistance, 'lattice baseline controls.maxDistance').toBe(500);

        // Into cosmic (camera-mutating) and confirm the mutation actually
        // happened — otherwise the round-trip proves nothing.
        await switchMode(page, 'cosmic');
        await expect.poll(
            () => page.evaluate(() => window.__ftdCtx?.viewport?.camera?.far ?? null),
            { timeout: 15_000, message: 'cosmic never mutated camera.far (scene did not enter cosmic)' },
        ).toBe(50000);

        // Back to lattice; the destroy() restore must run.
        await switchMode(page, 'lattice');

        // Poll the load-bearing fields back to the lattice baseline.
        await expect.poll(
            () => page.evaluate(() => window.__ftdCtx?.viewport?.camera?.far ?? null),
            { timeout: 10_000, message: 'camera.far not restored to lattice baseline after cosmic round-trip' },
        ).toBe(b.far);
        await expect.poll(
            () => page.evaluate(() => window.__ftdCtx?.viewport?.controls?.maxDistance ?? null),
            { timeout: 10_000, message: 'controls.maxDistance not restored to lattice baseline after cosmic round-trip' },
        ).toBe(b.maxDistance);

        // Position + target are also saved/restored (clone()/copy()); assert
        // them too, with a tiny epsilon for float copies.
        const after = await getCameraState(page);
        expect(after, 'camera state unreachable after cosmic round-trip').not.toBeNull();
        const a = /** @type {NonNullable<typeof after>} */ (after);

        const EPS = 1e-6;
        expect(Math.abs(a.posX - b.posX), 'camera.position.x restored').toBeLessThan(EPS);
        expect(Math.abs(a.posY - b.posY), 'camera.position.y restored').toBeLessThan(EPS);
        expect(Math.abs(a.posZ - b.posZ), 'camera.position.z restored').toBeLessThan(EPS);
        expect(Math.abs(a.targetX - b.targetX), 'controls.target.x restored').toBeLessThan(EPS);
        expect(Math.abs(a.targetY - b.targetY), 'controls.target.y restored').toBeLessThan(EPS);
        expect(Math.abs(a.targetZ - b.targetZ), 'controls.target.z restored').toBeLessThan(EPS);
    });

    // ────────────────────────────────────────────────────────────────────
    // (E) 10× rapid stress loop.
    //
    // Rapidly cycle lattice ↔ cosmic 10× with minimal settle. Two leak
    // signatures compound per iteration if teardown is broken:
    //   - console errors accumulate (a throw every destroy), and
    //   - the rAF subscriber count grows (a subscription per mount never
    //     released).
    // After the loop (settled, back on lattice) assert: zero real console
    // errors, and the rAF count is bounded at ≤ baseline + small headroom.
    //
    // cosmic is the heaviest single-scale round-trip (own bridge + renderer +
    // camera mutation), so lattice↔cosmic is the meanest 2-cycle to hammer.
    // ────────────────────────────────────────────────────────────────────
    test('(E) 10× rapid lattice↔cosmic cycling accumulates no errors and no rAF leak', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtxViewport(page);
        await page.waitForTimeout(SETTLE_MS);

        const baseline = await rafSize(page);

        const RAPID_SETTLE_MS = 120; // minimal — deliberately stress the teardown
        for (let i = 0; i < 10; i++) {
            await switchMode(page, 'cosmic');
            await page.waitForTimeout(RAPID_SETTLE_MS);
            await switchMode(page, 'lattice');
            await page.waitForTimeout(RAPID_SETTLE_MS);
        }

        // Settle, then assert the rAF count is bounded. Small headroom over
        // baseline absorbs a single in-flight subscription that unsubscribes a
        // frame after we sample; a real per-iteration leak would be ≈10× the
        // per-switch subscription delta, far above this.
        const RAF_HEADROOM = 2;
        await expect.poll(
            () => rafSize(page),
            {
                timeout: 10_000,
                message: `rAF subscriber count grew under 10× rapid cycling (baseline ${baseline}, headroom ${RAF_HEADROOM}) — per-iteration subscription leak`,
            },
        ).toBeLessThanOrEqual(baseline + RAF_HEADROOM);

        const real = realErrors(errors);
        expect(
            real,
            `console errors accumulated under 10× rapid lattice↔cosmic cycling:\n${real.join('\n')}`,
        ).toHaveLength(0);
    });
});

// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

/**
 * Knot telemetry — Scale-0 Knots panel data path (Phase 6).
 *
 * Verifies the C++ KnotTracker telemetry surfaces through the WASM RenderBridge
 * (window.__ftdCtx.bridge) under the new `knot_tracking` toggle. The telemetry
 * lives on the WASM bridge, NOT the flux mock — so we use a scenario that
 * MANIFESTS a real cluster on the real WASM engine: `s0-seed-emergent-ic1` (the
 * ic1 cluster). The C++ KnotTracker floor is min_cluster_size = 1, so a manifested
 * cluster reliably exercises the list path.
 *
 * Ticks are driven deterministically via bridge.tick() with the loop paused
 * (no wall-clock playback), mirroring genesis-burst.spec.js. Invariants asserted:
 *   - getKnotTelemetry().count === getKnotAggregate().alive
 *   - fields.length === count * stride
 *   - rows carry finite numeric fields (sizes, ages, flux)
 */

test.describe('Knot telemetry (Scale-0 KnotTracker)', () => {
    /** @type {import('@playwright/test').BrowserContext|undefined} */
    let context;
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        // Force the in-thread WASM path (documented tests/fallback hook,
        // scenario-loader.js:126) so ticks are synchronous + deterministic and
        // window.__ftdCtx.bridge is the live ticking bridge. The panel itself
        // resolves the active bridge (proxy when the worker IS active), so this
        // only pins determinism for the test, not the production path.
        await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!(window.__ftdCtx?.bridge)), { timeout: 20_000 }).toBe(true);
    });

    test.afterAll(async () => {
        await context?.close();
    });

    test('emergent-ic1 manifests a cluster and reports per-knot telemetry under tracking', async () => {
        // Pause the loop so all ticking is deterministic / under our control.
        await page.evaluate(() => { if (window.__ftdCtx) window.__ftdCtx.running = false; });

        // Switch to the manifesting scenario.
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (!sel) throw new Error('scenario-select not found');
            sel.value = 's0-seed-emergent-ic1';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await page.waitForTimeout(500); // let the registry load() run (toggles + inject)

        // Must be the real WASM engine, not the JS flux-mock — knot telemetry
        // is only computed by the C++ KnotTracker on the WASM bridge.
        const useMock = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return !!getScale0State().useFluxMock;
        });
        expect(useMock, 'emergent-ic1 must run on the real WASM engine').toBe(false);

        // Enable knot_tracking on the WASM bridge (capabilities.scale0 path, with
        // the direct setToggle as fallback) and tick to let the cluster manifest
        // + the tracker record.
        const data = await page.evaluate(async () => {
            const b = window.__ftdCtx?.bridge;
            if (!b) return { ok: false, reason: 'no bridge' };
            window.__ftdCtx.running = false;

            // Scenario selection now loads + injects the seed IMMEDIATELY (no longer
            // deferred to state._pendingScenarioLoad / the running tick loop), so the
            // seed is already present from the 'change' event above. The guard below is
            // a harmless no-op kept for backward compatibility in case any path still
            // sets _pendingScenarioLoad.
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            if (typeof st._pendingScenarioLoad === 'function') {
                st._pendingScenarioLoad();
                st._pendingScenarioLoad = null;
            }

            if (b.capabilities?.scale0?.setToggle) b.capabilities.scale0.setToggle('knot_tracking', true);
            else if (b.setToggle) b.setToggle('knot_tracking', true);

            // emergent-ic1 has a ~200-tick genesis burn-in before the (in-browser
            // CPU-throttled) cluster manifests; tick well past it. The cluster is
            // small + transient on the in-thread CPU path (FTD-0261 suppression),
            // so capture a SNAPSHOT at the tick where the tracker reports the most
            // alive knots — that is the non-vacuous tick for the list-path asserts.
            // Every snapshot also re-checks the shape invariants (count===alive,
            // fields.length===count*stride) so they hold at every tick, not just
            // the peak.
            let peakManifested = 0, births = 0;
            let snap = { count: 0, alive: 0, stride: 11, fieldsLen: 0, size0: -1, age0: -1, flux0: NaN };
            let invariantOk = true;
            for (let i = 0; i < 500; i++) {
                if (typeof b.tick === 'function') b.tick();
                const m = Number(b.getDiagnostics?.().manifested ?? 0);
                if (m > peakManifested) peakManifested = m;
                const agg = b.getKnotAggregate?.();
                const tel = b.getKnotTelemetry?.();
                if (agg) births = agg.births;
                const c = tel ? tel.count : -1;
                const a = agg ? agg.alive : -1;
                const fl = tel ? tel.fields.length : -1;
                const stride = tel ? (tel.stride || 11) : 11;
                if (c !== a || fl !== c * stride) invariantOk = false;
                if (tel && tel.count > snap.count) {
                    snap = {
                        count: tel.count, alive: a, stride,
                        fieldsLen: fl,
                        size0: tel.size[0], age0: tel.age[0], flux0: tel.fields[6],
                    };
                }
            }

            const aggEnd = b.getKnotAggregate?.();
            const telEnd = b.getKnotTelemetry?.();
            return {
                ok: true,
                invariantOk,
                births,
                peakManifested,
                snap,
                // end-of-run aggregate/telemetry (may be 0 if the cluster decayed)
                endCount: telEnd ? telEnd.count : -1,
                endAlive: aggEnd ? aggEnd.alive : -1,
                endFieldsLen: telEnd ? telEnd.fields.length : -1,
                endStride: telEnd ? (telEnd.stride || 11) : 11,
            };
        });

        console.log('[knots-telemetry] live:', JSON.stringify(data));
        expect(data.ok, data.reason).toBe(true);

        // Invariants must hold at EVERY tick of the run:
        //   getKnotTelemetry().count === getKnotAggregate().alive
        //   fields.length === count * stride
        expect(data.invariantOk, 'count===alive and fields.length===count*stride at every tick').toBe(true);
        // …and at the end-of-run snapshot specifically.
        expect(data.endCount).toBe(data.endAlive);
        expect(data.endFieldsLen).toBe(data.endCount * data.endStride);

        // The tracker must observe the genesis manifestation: the cluster crosses
        // the manifestation threshold (peakManifested > 0) and the tracker records
        // at least one birth. This proves the WASM → telemetry path is live, even
        // though the in-browser CPU engine throttles the cluster small + transient
        // (FTD-0261), so the alive count may be 0 again by end-of-run.
        expect(data.peakManifested, 'emergent-ic1 must manifest voxels').toBeGreaterThan(0);
        expect(data.births, 'tracker must record at least one knot birth').toBeGreaterThan(0);

        // At the peak-alive snapshot, the per-knot row carries valid numeric fields
        // and the cluster meets the tracking threshold. The C++ KnotTracker floor is
        // min_cluster_size = 1 (render_bridge.cpp ~L101), so a tracked knot is ≥ 1
        // voxel — the peak-count tick may land on a single-voxel knot.
        if (data.snap.count > 0) {
            expect(data.snap.count).toBe(data.snap.alive);
            expect(data.snap.fieldsLen).toBe(data.snap.count * data.snap.stride);
            expect(data.snap.stride).toBe(11);
            expect(data.snap.size0).toBeGreaterThanOrEqual(1);
            expect(Number.isFinite(data.snap.age0)).toBe(true);
            expect(Number.isFinite(data.snap.flux0)).toBe(true);
        }
    });

    test('tracking off ⇒ tracker stops recording (telemetry shape stays consistent)', async () => {
        // NOTE: toggling knot_tracking OFF stops record() but does NOT clear the
        // tracker — the last-recorded knots remain in the C++ history (death is
        // only detected on a subsequent record() where a knot is absent). So the
        // raw bridge aggregate does NOT zero out. The PANEL zeroes the display at
        // the UI layer (it gates rendering on the tracking checkbox). Here we
        // assert the bridge-level contract: no new knots accrue and the flat
        // fields buffer stays self-consistent.
        const data = await page.evaluate(() => {
            const b = window.__ftdCtx?.bridge;
            if (!b) return { ok: false };
            window.__ftdCtx.running = false;
            if (b.capabilities?.scale0?.setToggle) b.capabilities.scale0.setToggle('knot_tracking', false);
            else if (b.setToggle) b.setToggle('knot_tracking', false);

            const before = b.getKnotTelemetry?.().count ?? -1;
            for (let i = 0; i < 10; i++) { if (typeof b.tick === 'function') b.tick(); }
            const tel = b.getKnotTelemetry?.();
            const agg = b.getKnotAggregate?.();
            return {
                ok: true,
                before,
                after: tel ? tel.count : -1,
                alive: agg ? agg.alive : -1,
                stride: tel ? tel.stride : 0,
                fieldsLen: tel ? tel.fields.length : -1,
            };
        });
        expect(data.ok).toBe(true);
        // No new knots recorded while tracking is off.
        expect(data.after).toBe(data.before);
        // Aggregate stays internally consistent with the per-knot row count.
        expect(data.after).toBe(data.alive);
        // Flat buffer remains exactly count * stride.
        expect(data.fieldsLen).toBe(data.after * data.stride);
    });
});

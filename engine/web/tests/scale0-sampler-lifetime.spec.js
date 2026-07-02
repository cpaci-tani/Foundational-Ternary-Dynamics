// @ts-check
/**
 * WASM sampler static-cache lifetime contract (revision 0.10).
 *
 * engine/wasm/ftd_wasm.cpp's bulk samplers (getFluxVolume, getFluxSlice,
 * get*FieldSampled) return zero-copy typed_memory_view's over static
 * std::vector caches. The documented contract (ftd_wasm.cpp:402-404):
 * "the returned view is valid until the next call to this function — JS
 * callers must consume (or copy) before the next call."
 *
 * That contract was enforced by comments only; this spec pins it at the real
 * boundary: a held view from call N is silently REWRITTEN by call N+1
 * (aliasing), so any consumer that stores a view across frames is corrupt.
 * If a future change makes samplers return fresh copies instead, this test
 * fails — update the contract docs (ftd_wasm.cpp + CONTRACTS.md §2) and this
 * pin together in that commit.
 *
 * Runs only when the active bridge is the direct WasmBridge (the worker
 * proxy path serves copies out of a SharedArrayBuffer and has different,
 * safe-by-construction semantics).
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.beforeEach(async ({ page }) => { page.setDefaultTimeout(25_000); });

test('held getFluxVolume view is aliased (rewritten) by the next call', async ({ page }) => {
    await gotoAndReady(page);
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
        { timeout: 15_000, message: 'window.__ftdCtx.bridge never became available' },
    ).toBe(true);

    const r = await page.evaluate(() => {
        const bridge = window.__ftdCtx?.bridge;
        if (!bridge?.getFluxVolume || !bridge.injectFlux || !bridge.tick) {
            return { skip: 'bridge lacks direct sampler surface (mock/worker path)' };
        }
        const N = bridge.latticeSize;
        if (!N) return { skip: 'no latticeSize' };

        // Everything below is one synchronous block — no animation frame can
        // interleave between the two sampler calls.
        const viewA = bridge.getFluxVolume();
        if (!viewA || viewA.length !== N * N * N) {
            return { skip: `unexpected volume length ${viewA?.length} (proxy path?)` };
        }
        const mid = Math.floor(N / 2);
        // JS-side layout is Z-slowest (ftd_wasm.cpp transpose comment).
        const jsIdx = mid * N * N + mid * N + mid;
        const centerBefore = viewA[jsIdx];

        // Mutate engine state, then take a second view.
        bridge.injectFlux(mid, mid, mid, 3.0, 0.0, 0.0);
        bridge.tick();
        const viewB = bridge.getFluxVolume();

        return {
            skip: null,
            centerBefore,
            centerAfterViaB: viewB[jsIdx],
            // The held view A after call B — if aliased, it shows B's data.
            centerAfterViaHeldA: viewA[jsIdx],
            sameLength: viewA.length === viewB.length,
        };
    });

    test.skip(!!r.skip, r.skip || '');

    // Sanity: the mutation was visible through the fresh view.
    expect(r.centerAfterViaB, 'injectFlux+tick must change the center voxel density')
        .not.toBe(r.centerBefore);
    expect(r.sameLength).toBe(true);

    // THE CONTRACT: the held view now shows the NEW data — it aliases the
    // static cache and was silently rewritten by the second call. Consumers
    // must copy before the next sampler call.
    expect(r.centerAfterViaHeldA,
        'held view was NOT rewritten by the next getFluxVolume call — the ' +
        'zero-copy static-cache contract has changed; update ftd_wasm.cpp ' +
        'lifetime comments, CONTRACTS.md §2, and this pin together')
        .toBe(r.centerAfterViaB);
});

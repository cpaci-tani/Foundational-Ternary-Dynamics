// @ts-check
/**
 * Scale-0 sparse wave-tick regression (2026-06-03). Pins that the active-region
 * (bounding-box) tick is BIT-IDENTICAL to the dense tick, and faster for a
 * localized pulse. See engine/web/docs/SPEC_SCALE0_LATTICE_PERF.md §3 and
 * engine/web/docs/PLAN_SCALE0_SPARSE_TICK.md.
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
                for (let i = 0; i < 3; i++) s0.tickScale0();   // warm + keep the box small
                const t = performance.now();
                for (let i = 0; i < n; i++) s0.tickScale0();
                return (performance.now() - t) / n;
            };
            const dense = time(make(false), 10);
            const sparse = time(make(true), 10);
            return { dense: +dense.toFixed(2), sparse: +sparse.toFixed(2) };
        });
        // An early pulse occupies a tiny box ⇒ sparse should be well under half dense.
        expect(timeTick.sparse, `dense=${timeTick.dense}ms sparse=${timeTick.sparse}ms`)
            .toBeLessThan(timeTick.dense * 0.5);
    });

    test('SAB-backed buffers tick identically to plain buffers', async ({ page }) => {
        await gotoAndReady(page);
        const coi = await page.evaluate(() =>
            globalThis.crossOriginIsolated === true && typeof SharedArrayBuffer !== 'undefined');
        test.skip(!coi, 'requires cross-origin isolation (serve.py COOP/COEP)');
        const r = await page.evaluate(async () => {
            const { MockBridge } = await import('/js/bridge/mock-bridge.js');
            const run = (useSAB) => {
                const b = new MockBridge(33); b._useSAB = useSAB; b._sparseTick = false;
                const c = 16; b.injectFlux(c, c, c, 0.5, 0.5, 0.5);
                const s0 = b.capabilities.scale0;
                for (let i = 0; i < 20; i++) s0.tickScale0();
                return Array.from(b._fluxJ);
            };
            const plain = run(false), sab = run(true);
            let m = 0; for (let i = 0; i < plain.length; i++) m = Math.max(m, Math.abs(plain[i] - sab[i]));
            return { maxDiff: m, sabLen: sab.length };
        });
        expect(r.sabLen, 'SAB run produced a full field').toBe(33 ** 3 * 3);
        expect(r.maxDiff, 'SAB-backed tick is bit-identical to plain buffers').toBe(0);
    });
});

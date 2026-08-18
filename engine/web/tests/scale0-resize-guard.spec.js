// @ts-check
/**
 * Scale-0 lattice-resize heap-guard regression.
 *
 * The guard in scenario-loader.js::resizeScale0Lattice estimates memory usage
 * as 1300 bytes/voxel (C++ WASM engine) and caps at (isWasm64 ? 8 : 2) GB.
 * Requests that would exceed the cap are refused (dropdown reverts).
 *
 * These tests drive the REAL dropdown-change → controller.resize path and read
 * the live bridge state. Chromium supports Memory64, so this runs on the wasm64
 * module (isWasm64 === true).
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

test.beforeEach(async ({ page }) => { page.setDefaultTimeout(25_000); });

async function waitForCtx(page) {
    await expect.poll(
        () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
        { timeout: 15_000, message: 'window.__ftdCtx.bridge never became available' },
    ).toBe(true);
}

/** Read the live Scale-0 owner + sizes after a resize settles. */
async function readState(page) {
    return page.evaluate(async () => {
        const ctx = window.__ftdCtx;
        const store = await import('/js/scales/scale0/state/store.js');
        const st = store.getScale0State?.();
        const mock = st?.fluxMock || null;
        let mockFluxLen = null, mockHasFlux = false;
        if (mock?.getFluxVolume) {
            try {
                const fv = mock.getFluxVolume();
                mockFluxLen = fv?.length ?? null;
                for (let i = 0; i < fv.length; i++) { if (fv[i] !== 0) { mockHasFlux = true; break; } }
            } catch { /* ignore */ }
        }
        return {
            dropdown: document.getElementById('lattice-size')?.value,
            bridgeLattice: ctx?.bridge?.latticeSize,
            isWasm64: !!ctx?.bridge?.isWasm64,
            scenario: st?.currentScenarioId,
            useFluxMock: !!st?.useFluxMock,
            mockLattice: mock?.latticeSize ?? null,
            mockFluxLen,
            mockHasFlux,
        };
    });
}

/** Pick a size in the dropdown (optionally injecting an out-of-list option) and fire change. */
async function selectSize(page, val, ensureOption = false) {
    await page.evaluate(({ val, ensureOption }) => {
        const sel = document.getElementById('lattice-size');
        if (ensureOption && ![...sel.options].some(o => o.value === String(val))) {
            sel.add(new Option(String(val), String(val)));
        }
        sel.value = String(val);
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, { val, ensureOption });
}

const sizeHolds = (page, n) => expect.poll(
    () => page.evaluate(() => Number(document.getElementById('lattice-size')?.value)),
    { timeout: 20_000, message: `dropdown should hold ${n} (a refusal reverts it)` },
).toBe(n);

test.describe('Scale-0 lattice resize heap guard', () => {

    test('WASM-worker-owned scenario (empty) refuses an oversized lattice (C++ heap cap)', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtx(page);

        await page.evaluate(() => {
            const s = document.getElementById('scenario-select');
            s.value = 'empty';
            s.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(async () => {
            const st = (await import('/js/scales/scale0/state/store.js')).getScale0State?.();
            return st?.currentScenarioId;
        }), { timeout: 15_000, message: 'scenario should switch to empty' }).toBe('empty');

        const st = await readState(page);
        // The legacy fluxMock slot now holds the off-thread WASM proxy.  It is
        // not a JavaScript physics mock; useFluxMock=true means the C++ WASM
        // worker is the active Scale-0 owner.
        expect(st.useFluxMock, 'empty is owned by the C++ WASM worker').toBe(true);
        expect(st.mockLattice, 'worker reports its live lattice size').toBe(st.bridgeLattice);

        // 257 on the WASM owner: 257³ × 1300 B = 20.6 GB > 8 GB (wasm64) ⇒ refused.
        // The refusal short-circuits BEFORE any C++ allocation, so this is cheap.
        const before = st.bridgeLattice;
        const workerBefore = st.mockLattice;
        await selectSize(page, 257, /* ensureOption */ true);
        await page.waitForTimeout(500);
        const after = await readState(page);
        expect(after.dropdown, '257 reverts on the WASM owner (20.6 GB > 8 GB cap)').toBe(String(before));
        expect(after.bridgeLattice, '257 does not advance the WASM bridge').toBe(before);
        expect(after.mockLattice, '257 does not advance the WASM worker').toBe(workerBefore);
    });
});

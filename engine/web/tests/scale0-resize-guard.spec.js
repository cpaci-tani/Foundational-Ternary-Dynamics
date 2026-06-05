// @ts-check
/**
 * Scale-0 lattice-resize heap-guard regression (2026-06-03).
 *
 * Pins the fix for the recurring
 *   "L=NNN would need ~X GB of heap (max 2 GB ...). Refusing to resize."
 * error on the DEFAULT flux-pulse scenario.
 *
 * Root cause (scenario-loader.js::resizeScale0Lattice): the guard estimated
 * EVERY scenario at the C++ engine's ~1300 bytes/voxel and capped the flux- and
 * s0- scenario families at 2 GB — but those run on the JS MockBridge
 * (~150 B/voxel) and the
 * resize path never reallocates the C++ grid (it only sets bridge.latticeSize +
 * builds a fresh MockBridge(newSize)). So big *centered* flux-* lattices were
 * refused over memory that is never allocated, even on the wasm64 (8 GB) build.
 * The guard now estimates per-owner: MockBridge → 150 B/voxel + 2 GB JS cap;
 * C++ engine → 1300 B/voxel + (isWasm64 ? 8 : 2) GB WASM cap.
 *
 * These tests drive the REAL dropdown-change → controller.resize path and read
 * the live bridge / fluxMock state. The guard + MockBridge build run
 * synchronously on the change handler, so no rAF is needed for the data layer.
 * Chromium supports Memory64, so this runs on the wasm64 module
 * (isWasm64 === true) — the same environment the user hit the bug in.
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

test.describe('Scale-0 lattice resize heap guard (per-owner)', () => {

    test('flux-pulse (MockBridge-owned) resizes to a big centered lattice without refusal', async ({ page }) => {
        // Heavy: resize to a large lattice = big heap alloc + re-seed + GPU
        // re-upload. Completes in ~30-35s, which tips over the 30s default test
        // timeout under machine load. 2× headroom avoids the flake.
        test.setTimeout(60_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await waitForCtx(page);

        const before = await readState(page);
        expect(before.scenario, 'default scenario is flux-pulse').toBe('flux-pulse');
        expect(before.useFluxMock, 'flux-pulse is MockBridge-owned').toBe(true);
        expect(before.isWasm64, 'Chromium loads the wasm64 module').toBe(true);

        // 145 — the exact size from the bug report; pre-fix this refused at "max 2 GB".
        await selectSize(page, 145);
        await sizeHolds(page, 145);
        const at145 = await readState(page);
        expect(at145.bridgeLattice, 'bridge advanced to 145').toBe(145);
        expect(at145.mockLattice, 'MockBridge reallocated at 145').toBe(145);
        expect(at145.mockFluxLen, 'flux field allocated at 145³').toBe(145 ** 3);
        expect(at145.mockHasFlux, 'flux-pulse populated the field at 145').toBe(true);

        // 181 — the dropdown maximum; must also pass on the mock owner.
        await selectSize(page, 181);
        await sizeHolds(page, 181);
        const at181 = await readState(page);
        expect(at181.mockLattice, 'MockBridge reallocated at 181').toBe(181);
        expect(at181.mockFluxLen, 'flux field allocated at 181³').toBe(181 ** 3);
        expect(at181.mockHasFlux, 'flux-pulse populated the field at 181').toBe(true);

        const real = realErrors(errors);
        expect(real, `console errors:\n  ${real.join('\n  ')}`).toHaveLength(0);
    });

    test('mock guard still refuses a truly oversized lattice (guard not disabled)', async ({ page }) => {
        await gotoAndReady(page);
        await waitForCtx(page);

        // 401³ × 150 B = 8.07 GB > the 2 GB JS-heap cap ⇒ must be refused (revert).
        const before = await readState(page);
        await selectSize(page, 401, /* ensureOption */ true);
        await page.waitForTimeout(500);
        const after = await readState(page);
        expect(after.dropdown, 'oversized 401 reverts the dropdown').toBe(String(before.bridgeLattice));
        expect(after.bridgeLattice, 'oversized 401 does not advance the bridge').toBe(before.bridgeLattice);
    });

    test('WASM-owned scenario (empty) gates on the C++ heap cap, not the mock cap', async ({ page }) => {
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
        expect(st.useFluxMock, 'empty is WASM-owned (no fluxMock)').toBe(false);

        // 257 on the WASM owner: 257³ × 1300 B = 20.6 GB > 8 GB (wasm64) ⇒ refused.
        // The refusal short-circuits BEFORE any C++ allocation, so this is cheap.
        const before = st.bridgeLattice;
        await selectSize(page, 257, /* ensureOption */ true);
        await page.waitForTimeout(500);
        const after = await readState(page);
        expect(after.dropdown, '257 reverts on the WASM owner (20.6 GB > 8 GB cap)').toBe(String(before));
        expect(after.bridgeLattice, '257 does not advance the WASM bridge').toBe(before);
    });
});

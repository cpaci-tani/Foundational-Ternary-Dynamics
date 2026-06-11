// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

/**
 * Genesis-burst N(A) cluster-size law scenario (FTD-0269).
 *
 * Verifies the new Scale-0 scenario family runs on the REAL WASM engine and
 * exhibits the two-regime structure: the three fixed-A "answer-key" variants
 * (subknee A=12 / knee A=16 / superknee A=40) must manifest clusters in
 * increasing size (geometry-limited → energy-limited), and the interactive
 * panel must record an (A,N) point on its live plot.
 *
 * Ticks are driven deterministically via bridge.tick() in page.evaluate (the
 * page is paused), so no wall-clock playback. Tolerances are generous: the
 * in-browser CPU engine's genesis-drain suppresses N vs the GPU FTD-0261 table,
 * so we assert the regime ORDERING, never the absolute campaign numbers.
 */

const SETTLE = 200;

async function loadAndSettle(page, id, ticks) {
    await page.evaluate((sid) => {
        const sel = document.getElementById('scenario-select');
        if (!sel) throw new Error('scenario-select not found');
        sel.value = sid;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await page.waitForTimeout(500); // let the registry load() run (toggles + inject)
    return page.evaluate((n) => {
        const b = window.__ftdCtx?.bridge;
        if (!b) return -1;
        window.__ftdCtx.running = false;
        for (let i = 0; i < n; i++) { if (typeof b.tick === 'function') b.tick(); }
        return Number(b.getDiagnostics?.().manifested ?? 0);
    }, ticks);
}

test.describe('Genesis-burst N(A) law (FTD-0269)', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!(window.__ftdCtx?.bridge)), { timeout: 20_000 }).toBe(true);
    });

    test('three regimes: N(subknee) < N(knee) < N(superknee)', async ({ page }) => {
        const sub = await loadAndSettle(page, 's0-seed-cluster-law-subknee', SETTLE);
        const knee = await loadAndSettle(page, 's0-seed-cluster-law-knee', SETTLE);
        const sup = await loadAndSettle(page, 's0-seed-cluster-law-superknee', SETTLE);

        expect(sub, 'sub-knee should manifest a cluster').toBeGreaterThan(0);
        expect(knee, 'knee cluster > sub-knee (broken-power growth)').toBeGreaterThan(sub);
        expect(sup, 'super-knee cluster > knee (bulk volume regime)').toBeGreaterThan(knee);
    });

    test('canonical scenario runs on the real WASM engine and the panel records a point', async ({ page }) => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-cluster-law';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        // panel mounts on load
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);

        // real engine, not the JS flux-mock
        const useMock = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return !!getScale0State().useFluxMock;
        });
        expect(useMock, 'genesis-burst must run on the real WASM engine').toBe(false);

        // drive the panel's fire() and confirm a live (A,N) point is recorded
        const pts = await page.evaluate(async () => {
            await window.__ftdGenesisBurstPanel.fire(16);
            return window.__ftdGenesisBurstPanel.getPoints();
        });
        expect(pts.length).toBe(1);
        expect(pts[0].A).toBe(16);
        expect(Number.isFinite(pts[0].N)).toBe(true);
    });

    test('panel is disposed when switching away from the scenario', async ({ page }) => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-cluster-law';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 's0-seed-emergent-ic1';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        // the 500ms disposal guard removes the panel
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 5_000 }).toBe(false);
        expect(await page.evaluate(() => !!document.getElementById('genesis-burst-panel'))).toBe(false);
    });
});

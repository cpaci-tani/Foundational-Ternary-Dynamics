// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

/**
 * Selected genesis amplitude-response scenarios (FTD-0269 provenance).
 *
 * Verifies the new Scale-0 scenario family runs on the REAL WASM engine and
 * exhibits a reproducible finite-box ordering: the three fixed-A variants
 * A=12 / A=16 / A=40 must manifest clusters in increasing size, and the interactive
 * panel must record an (A,N) point on its live plot.
 *
 * Ticks are driven deterministically via bridge.tick() in page.evaluate (the
 * page is paused), so no wall-clock playback. Tolerances are generous: the
 * in-browser CPU engine's genesis-drain suppresses N vs the GPU FTD-0261 table,
 * so we assert the regime ORDERING, never the absolute campaign numbers.
 */

const SETTLE = 200;

async function loadAndSettle(page, id, ticks) {
    // 1. Pause the loop first so no background ticking can happen during load
    await page.evaluate(() => {
        if (window.__ftdCtx) {
            window.__ftdCtx.running = false;
        }
    });
    // 2. Load the hidden research scenario through the production UI path.
    await selectScale0Scenario(page, id);
    // 4. Tick exactly `n` times from the initial scenario state
    const result = await page.evaluate(async (n) => {
        const { getScale0State, resolveActiveScale0BridgeFromWindow } =
            await import('/js/scales/scale0/state/store.js');
        const { runScale0PhysicsTicks } = await import('/js/scales/scale0/runtime/tick.js');
        const state = getScale0State();
        const b = resolveActiveScale0BridgeFromWindow();
        if (!b) return { manifested: -1, peak: -1, history: [] };
        window.__ftdCtx.running = false;
        let peak = 0;
        const history = [];
        for (let i = 0; i < n; i++) {
            runScale0PhysicsTicks(window.__ftdCtx, state, 1);
            const sc = b.capabilities?.scale0;
            const current = Number(sc?.getScale0Diagnostics?.()?.manifested
                ?? sc?.getScale0EnergyAudit?.()?.manifested ?? 0);
            if (current > peak) peak = current;
            if (i % 20 === 0 || i === n - 1) {
                history.push(`t=${i}:${current}`);
            }
        }
        return {
            manifested: Number(b.capabilities?.scale0?.getScale0Diagnostics?.()?.manifested
                ?? b.capabilities?.scale0?.getScale0EnergyAudit?.()?.manifested ?? 0),
            peak,
            history
        };
    }, ticks);
    console.log(`History for ${id}:`, result.history.join(', '), `(peak: ${result.peak})`);
    return result.peak;
}

test.describe('Selected genesis amplitude response (FTD-0269 provenance)', () => {
    /** @type {import('@playwright/test').BrowserContext|undefined} */
    let context;
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!(window.__ftdCtx?.bridge)), { timeout: 20_000 }).toBe(true);
    });

    test.afterAll(async () => {
        await context?.close();
    });

    test('fixed amplitudes give N(12) < N(16) < N(40)', async () => {
        const sub = await loadAndSettle(page, 's0-seed-cluster-law-subknee', SETTLE);
        const knee = await loadAndSettle(page, 's0-seed-cluster-law-knee', SETTLE);
        const sup = await loadAndSettle(page, 's0-seed-cluster-law-superknee', SETTLE);

        expect(sub, 'sub-knee should manifest a cluster').toBeGreaterThan(0);
        expect(knee, 'knee cluster > sub-knee (broken-power growth)').toBeGreaterThan(sub);
        expect(sup, 'super-knee cluster > knee (bulk volume regime)').toBeGreaterThan(knee);
    });

    test('canonical scenario runs on the real WASM engine and the panel records a point', async () => {
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        // panel mounts on load
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);

        // The active owner must be the real C++ WASM engine. In a COI browser
        // it is normally hosted by WasmBridgeProxy; `useFluxMock` names the
        // historical owner slot and does not mean a JS MockBridge.
        const owner = await page.evaluate(async () => {
            const { resolveActiveScale0BridgeFromWindow } = await import('/js/scales/scale0/state/store.js');
            const b = resolveActiveScale0BridgeFromWindow();
            return { isWasm: !!b?.isWasm, isWorker: !!b?.isWorker, name: b?.constructor?.name ?? '' };
        });
        expect(owner.isWasm || owner.isWorker,
            `genesis-burst must run on a real WASM owner, got ${owner.name}`).toBe(true);

        // drive the panel's fire() and confirm a live (A,N) point is recorded
        const pts = await page.evaluate(async () => {
            await window.__ftdGenesisBurstPanel.fire(16);
            return window.__ftdGenesisBurstPanel.getPoints();
        });
        expect(pts.length).toBe(1);
        expect(pts[0].A).toBe(16);
        expect(Number.isFinite(pts[0].N)).toBe(true);
        expect(pts[0].N, 'interactive A=16 firing should produce a nonzero native response').toBeGreaterThan(0);
    });

    test('panel is disposed when switching away from the scenario', async () => {
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);
        await selectScale0Scenario(page, 's0-seed-emergent-ic1', { settleMs: 0 });
        // the 500ms disposal guard removes the panel
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 5_000 }).toBe(false);
        expect(await page.evaluate(() => !!document.getElementById('genesis-burst-panel'))).toBe(false);
    });
});

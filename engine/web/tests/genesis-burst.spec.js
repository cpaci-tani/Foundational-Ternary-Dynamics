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
        window.__ftdCtx?.pauseSimulation?.();
    });
    // 2. Load the hidden research scenario through the production UI path.
    await selectScale0Scenario(page, id);
    await expect.poll(() => page.evaluate(async () => {
        const { getScale0QualificationState } = await import('/js/scales/scale0/state/store.js');
        return getScale0QualificationState().status;
    }), { timeout: 30_000 }).toBe('within-contract');
    // 3. Tick exactly `n` times, then await the worker's authoritative tick.
    const result = await page.evaluate(async (n) => {
        const { getScale0State, resolveActiveScale0BridgeFromWindow } =
            await import('/js/scales/scale0/state/store.js');
        const { runScale0PhysicsTicks } = await import('/js/scales/scale0/runtime/tick.js');
        const state = getScale0State();
        const b = resolveActiveScale0BridgeFromWindow();
        if (!b) return { manifested: -1, startTick: -1, finalTick: -1 };
        window.__ftdCtx.pauseSimulation?.();
        const sc = b.capabilities?.scale0;
        const startTick = Number(sc?.getScale0Diagnostics?.()?.tick ?? 0);
        runScale0PhysicsTicks(window.__ftdCtx, state, n);
        if (b.isWorker) {
            const targetTick = startTick + n;
            const deadline = performance.now() + 45_000;
            while (Number(sc?.getScale0Diagnostics?.()?.tick ?? -1) < targetTick) {
                if (performance.now() > deadline) {
                    throw new Error(`worker stopped before deterministic target ${targetTick}`);
                }
                await new Promise(resolve => setTimeout(resolve, 10));
            }
        }
        return {
            manifested: Number(sc?.getScale0Diagnostics?.()?.manifested
                ?? sc?.getScale0EnergyAudit?.()?.manifested ?? 0),
            startTick,
            finalTick: Number(sc?.getScale0Diagnostics?.()?.tick ?? -1),
        };
    }, ticks);
    console.log(`Settled ${id}: N=${result.manifested}, tick ${result.startTick}→${result.finalTick}`);
    return result.manifested;
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
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
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
        const result = await page.evaluate(async () => {
            const { getScale0QualificationState } = await import('/js/scales/scale0/state/store.js');
            const before = getScale0QualificationState().mutationEpoch;
            await window.__ftdGenesisBurstPanel.fire(16);
            const qualification = getScale0QualificationState();
            return {
                pts: window.__ftdGenesisBurstPanel.getPoints(),
                before,
                after: qualification.mutationEpoch,
                mutation: qualification.lastMutation,
            };
        });
        const pts = result.pts;
        expect(pts.length).toBe(1);
        expect(pts[0].A).toBe(16);
        expect(Number.isFinite(pts[0].N)).toBe(true);
        expect(pts[0].N, 'interactive A=16 firing should produce a nonzero native response').toBeGreaterThan(0);
        expect(result.after - result.before, 'one Fire intent must create exactly one mutation epoch').toBe(1);
        expect(result.mutation?.reason).toBe('genesis-experiment');
        expect(result.mutation?.source).toBe('panel.genesis-burst');
    });

    test('Fire pauses active playback at the transport before deterministic stepping', async () => {
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);
        await page.evaluate(() => {
            if (!window.__ftdCtx.running) document.getElementById('btn-play')?.click();
        });
        await expect.poll(() => page.evaluate(() => !!window.__ftdCtx?.running)).toBe(true);

        const result = await page.evaluate(async () => {
            const { resolveActiveScale0BridgeFromWindow } = await import('/js/scales/scale0/state/store.js');
            const ctx = window.__ftdCtx;
            const owner = resolveActiveScale0BridgeFromWindow();
            const originalPause = ctx.pauseSimulation;
            const originalSetRunning = owner.setRunning?.bind(owner);
            let pauseCalls = 0;
            const transitions = [];
            ctx.pauseSimulation = () => {
                pauseCalls += 1;
                return originalPause();
            };
            if (originalSetRunning) {
                owner.setRunning = (value) => {
                    transitions.push(!!value);
                    return originalSetRunning(value);
                };
            }
            try {
                const value = await window.__ftdGenesisBurstPanel.fire(12);
                return {
                    value,
                    pauseCalls,
                    transitions,
                    restoredRunning: ctx.running,
                    points: window.__ftdGenesisBurstPanel.getPoints(),
                };
            } finally {
                ctx.pauseSimulation = originalPause;
                if (originalSetRunning) owner.setRunning = originalSetRunning;
                originalPause();
            }
        });

        expect(result.pauseCalls).toBe(1);
        expect(result.transitions[0]).toBe(false);
        expect(result.transitions.at(-1)).toBe(true);
        expect(result.restoredRunning).toBe(true);
        expect(result.points).toHaveLength(1);
        expect(Number.isFinite(result.value)).toBe(true);
    });

    test('generation turnover aborts an in-flight fire without restoring stale running state', async () => {
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);

        const result = await page.evaluate(async () => {
            const { getScale0QualificationState } = await import('/js/scales/scale0/state/store.js');
            const api = window.__ftdGenesisBurstPanel;
            const ctx = window.__ftdCtx;
            const before = getScale0QualificationState().mutationEpoch;
            ctx.running = false;
            const pending = api.fire(90);
            // fire() yields after its first tick. Turn the dashboard generation
            // over before that continuation can post the remaining 219 ticks.
            ctx._loadGeneration += 1;
            ctx.running = true;
            const value = await pending;
            const qualification = getScale0QualificationState();
            return {
                value,
                running: ctx.running,
                points: api.getPoints(),
                epochDelta: qualification.mutationEpoch - before,
            };
        });

        expect(result.value).toBeNull();
        expect(result.points).toHaveLength(0);
        expect(result.running, 'a stale experiment must not restore its captured paused state').toBe(true);
        expect(result.epochDelta, 'the accepted Fire intent is recorded once even when later aborted').toBe(1);

        // Repair the deliberately advanced test generation through the normal
        // authoritative loader before the next case.
        await selectScale0Scenario(page, 's0-seed-cluster-law');
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

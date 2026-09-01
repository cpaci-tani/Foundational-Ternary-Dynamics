// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

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

    test('ten remount cycles conserve the scenario guard interval and singleton DOM', async () => {
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const [{ getPhysicsHarness }, { mountGenesisBurstPanel }, store] = await Promise.all([
                import('/js/physics/index.js'),
                import('/js/scales/scale0/ui/overlays/genesis-burst-panel.js?v=2'),
                import('/js/scales/scale0/state/store.js'),
            ]);
            window.__ftdGenesisBurstPanel?.dispose?.();
            const owner = store.resolveActiveScale0BridgeFromWindow();
            const harness = getPhysicsHarness(owner);
            const originalSetInterval = window.setInterval;
            const originalClearInterval = window.clearInterval;
            const liveIntervals = new Set();
            window.setInterval = function (callback, delay, ...args) {
                const id = originalSetInterval(callback, delay, ...args);
                liveIntervals.add(id);
                return id;
            };
            window.clearInterval = function (id) {
                liveIntervals.delete(id);
                return originalClearInterval(id);
            };
            const cycles = [];
            try {
                for (let i = 0; i < 10; i++) {
                    const api = mountGenesisBurstPanel(harness);
                    const mounted = {
                        intervals: liveIntervals.size,
                        panels: document.querySelectorAll('#genesis-burst-panel').length,
                        singleton: window.__ftdGenesisBurstPanel === api,
                    };
                    api.dispose();
                    cycles.push({
                        mounted,
                        disposedIntervals: liveIntervals.size,
                        disposedPanels: document.querySelectorAll('#genesis-burst-panel').length,
                        singletonNull: window.__ftdGenesisBurstPanel === null,
                    });
                }
            } finally {
                window.setInterval = originalSetInterval;
                window.clearInterval = originalClearInterval;
            }
            const final = mountGenesisBurstPanel(harness);
            return {
                cycles,
                final: {
                    panels: document.querySelectorAll('#genesis-burst-panel').length,
                    singleton: window.__ftdGenesisBurstPanel === final,
                },
            };
        });

        expect(result.cycles).toHaveLength(10);
        for (const cycle of result.cycles) {
            expect(cycle).toEqual({
                mounted: { intervals: 1, panels: 1, singleton: true },
                disposedIntervals: 0,
                disposedPanels: 0,
                singletonNull: true,
            });
        }
        expect(result.final).toEqual({ panels: 1, singleton: true });
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('live Fire work sustains the formal hardware frame budget', async ({}, testInfo) => {
        testInfo.setTimeout(120_000);
        await selectScale0Scenario(page, 's0-seed-cluster-law');
        await expect.poll(() => page.evaluate(() => !!window.__ftdGenesisBurstPanel), { timeout: 10_000 }).toBe(true);
        const consoleErrors = attachConsoleWatcher(page);
        await page.waitForTimeout(3_000);
        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const gl = window.__ftdCtx?.viewport?.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            const webglRenderer = rendererInfo
                ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                : '';
            probe.startScale0UiAuditProbe({ rootSelector: '#genesis-burst-panel' });
            const paintMs = await probe.measureScale0UiActionToPaint(
                'fire A=16',
                () => { void window.__ftdGenesisBurstPanel.fire(16); },
            );
            await new Promise((resolve) => setTimeout(resolve, 12_000));
            return { ...await probe.stopScale0UiAuditProbe(), webglRenderer, paintMs };
        });
        await testInfo.attach('scale0-genesis-burst-performance-report.json', {
            body: Buffer.from(JSON.stringify(report, null, 2)),
            contentType: 'application/json',
        });
        console.log('scale0 genesis burst performance', JSON.stringify(report));

        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(report.webglRenderer, 'release gate exposes a WebGL renderer').not.toBe('');
            expect(report.webglRenderer, 'release gate does not certify SwiftShader/software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        expect(report.frames.count).toBeGreaterThanOrEqual(600);
        expect(report.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
        expect(report.frames.p95Ms).toBeLessThanOrEqual(17);
        expect(report.frames.p99Ms).toBeLessThanOrEqual(20);
        expect(report.frames.intervalsOver33_4ms).toBe(0);
        expect(report.longTasks).toEqual([]);
        expect(report.paintMs).toBeLessThanOrEqual(50);
        expect(report.resourceDelta.rafSubscribers).toBe(0);
        expect(report.resourceDelta.domNodes).toBe(0);
        expect(report.resourceDelta.canvases).toBe(0);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});

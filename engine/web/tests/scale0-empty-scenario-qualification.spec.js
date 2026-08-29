// @ts-check
/**
 * Browser/UI qualification for Scale-0 Scenario 1: `empty`.
 *
 * Epistemic scope: this is the dashboard qualification of an exact null
 * control.  It does not promote an absent/unsupported browser read to a
 * measured zero, and it does not claim native-GPU parity.  The native C++
 * behavioral qualification remains engine/tests/test_scenario_behavior.cpp.
 */
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    getRendererMemory,
    gotoAndReady,
    rafSize,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

const EMPTY = 'empty';
const WASM_PATH = '/?engine=wasm';

test.describe.configure({ mode: 'serial' });
test.beforeEach(async ({ page }) => {
    page.setDefaultTimeout(30_000);
    await page.addInitScript(() => { window.__ftdTelemetryOnDemand = true; });
});

/** @param {import('@playwright/test').Page} page */
async function waitForEmptyReady(page, latticeSize = null) {
    await expect.poll(
        () => page.evaluate(async ({ scenarioId, expectedSize }) => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            const owner = state.useFluxMock && state.fluxMock
                ? state.fluxMock
                : window.__ftdCtx?.bridge;
            const diag = owner?.capabilities?.scale0?.getScale0Diagnostics?.();
            const sizeOK = expectedSize == null || Number(owner?.latticeSize) === expectedSize;
            const ownerScenarioOK = owner?._scenarioId === undefined
                || owner._scenarioId === scenarioId;
            return state.currentScenarioId === scenarioId
                && ownerScenarioOK
                && (!owner?.isWorker || owner.ready === true)
                && sizeOK
                && Number.isFinite(Number(diag?.tick));
        }, { scenarioId: EMPTY, expectedSize: latticeSize }),
        { timeout: 90_000, message: `empty did not become ready${latticeSize ? ` at L=${latticeSize}` : ''}` },
    ).toBe(true);
}

/**
 * Read the scientific null channels from the active owner. Optional channels
 * carry an explicit status; null/absent is never converted to numeric zero.
 * @param {import('@playwright/test').Page} page
 */
async function readEmptySnapshot(page) {
    return page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const owner = state.useFluxMock && state.fluxMock
            ? state.fluxMock
            : window.__ftdCtx?.bridge;
        const caps = owner?.capabilities?.scale0 || {};
        const diag = caps.getScale0Diagnostics?.() ?? null;
        const audit = caps.getScale0EnergyAudit?.() ?? null;
        const lagrangian = caps.getScale0Lagrangian?.() ?? null;
        const particles = caps.getScale0ParticleFrame?.() ?? null;
        const flux = caps.getScale0FluxVolume?.() ?? null;
        const latticeSize = Number(owner?.latticeSize ?? 0);
        const expectedFluxLength = latticeSize > 0 ? latticeSize ** 3 : null;
        const fluxStatus = typeof caps.getScale0FluxVolume !== 'function'
            ? 'unsupported'
            : flux == null || flux.length === 0
                ? 'not-published'
                : flux.length === expectedFluxLength
                    ? 'supported'
                    : 'contract-error';

        const peak = (values) => {
            let out = 0;
            for (let i = 0; values && i < values.length; i += 1) {
                out = Math.max(out, Math.abs(Number(values[i])));
            }
            return out;
        };
        const optional = (methodName, value, note = '') => {
            if (typeof caps[methodName] !== 'function') {
                return { status: 'unsupported', value: null, note };
            }
            if (value == null) return { status: 'not-published', value: null, note };
            return { status: 'supported', value, note };
        };

        // Full latency volume is intentionally unsupported by WasmBridgeProxy.
        // Classify it from the backend contract rather than misreporting the
        // proxy's empty typed array as a physically measured zero-volume field.
        const latency = owner?.isWorker
            ? { status: 'unsupported-worker-contract', length: null }
            : optional(
                'getScale0LatencyVolume',
                caps.getScale0LatencyVolume?.(),
                'supported only when the active bridge publishes a full latency volume',
            );

        return {
            selected: document.getElementById('scenario-select')?.value ?? null,
            current: state.currentScenarioId,
            generation: Number(window.__ftdCtx?._loadGeneration ?? -1),
            owner: owner?.isNativeGPU ? 'native-gpu' : (owner?.isWorker ? 'wasm-worker' : 'wasm-main'),
            ownerScenario: owner?._scenarioId ?? null,
            ready: !owner?.isWorker || owner.ready === true,
            latticeSize,
            flux: {
                status: fluxStatus,
                length: flux?.length ?? null,
                maxAbs: flux ? peak(flux) : null,
            },
            particles: optional('getScale0ParticleFrame', particles),
            diagnostics: optional('getScale0Diagnostics', diag),
            audit: optional('getScale0EnergyAudit', audit),
            lagrangian: optional('getScale0Lagrangian', lagrangian),
            latency,
        };
    });
}

function expectFiniteExactZeros(record, keys, label) {
    expect(record, `${label} object must be published`).not.toBeNull();
    for (const key of keys) {
        expect(Number.isFinite(Number(record[key])), `${label}.${key} must be finite`).toBe(true);
        expect(Number(record[key]), `${label}.${key} must be exact null-control zero`).toBe(0);
    }
}

function expectExactNull(snapshot) {
    expect(snapshot.selected).toBe(EMPTY);
    expect(snapshot.current).toBe(EMPTY);
    expect(snapshot.ready).toBe(true);
    if (snapshot.owner === 'wasm-worker') expect(snapshot.ownerScenario).toBe(EMPTY);

    expect(snapshot.flux.status).toBe('supported');
    expect(snapshot.flux.length).toBe(snapshot.latticeSize ** 3);
    expect(snapshot.flux.maxAbs, 'dense |J| volume must be bit-exact zero').toBe(0);
    expect(snapshot.particles.status).toBe('supported');
    expect(snapshot.particles.value?.count, 'manifested particle frame').toBe(0);

    expect(snapshot.diagnostics.status).toBe('supported');
    expectFiniteExactZeros(snapshot.diagnostics.value, [
        'manifested', 'positive', 'negative', 'totalFlux', 'totalEnergy',
    ], 'diagnostics');

    if (snapshot.audit.status === 'supported') {
        expectFiniteExactZeros(snapshot.audit.value, [
            'fieldEnergy', 'waveEnergy', 'particleKE', 'dynamicEnergy',
            'gaussViolation', 'chargeTotal', 'manifested',
        ], 'energy audit');
    } else {
        expect(snapshot.audit.status, 'an absent audit must be classified, never coerced to zero')
            .toBe('not-published');
    }
}

/** @param {import('@playwright/test').Page} page */
async function readWorkerCounters(page) {
    return page.evaluate(() => (typeof window.__ftdWasmWorkers === 'function'
        ? window.__ftdWasmWorkers()
        : null));
}

test('is an exact null after load, reset/reload, supported resize, and rapid generation changes', async ({ page }, testInfo) => {
    test.setTimeout(180_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: WASM_PATH, timeout: 90_000 });
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await waitForEmptyReady(page);

    const initial = await readEmptySnapshot(page);
    testInfo.annotations.push({
        type: 'backend-coverage',
        description: `${initial.owner} qualified live; native-GPU parity is not asserted without a connected native backend`,
    });
    expectExactNull(initial);

    await page.locator('#btn-reset').click();
    await waitForEmptyReady(page, initial.latticeSize);
    const reloaded = await readEmptySnapshot(page);
    expect(reloaded.generation).toBeGreaterThan(initial.generation);
    expectExactNull(reloaded);

    const resizeTarget = initial.latticeSize === 49 ? 33 : 49;
    const resizeSupported = await page.evaluate((size) => {
        const option = [...document.querySelectorAll('#lattice-size option')]
            .find((candidate) => Number(candidate.value) === size);
        return !!option && !option.disabled;
    }, resizeTarget);
    if (resizeSupported) {
        await page.selectOption('#lattice-size', String(resizeTarget));
        await waitForEmptyReady(page, resizeTarget);
        expectExactNull(await readEmptySnapshot(page));
    } else {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: `L=${resizeTarget} resize is disabled for the active backend`,
        });
    }

    const beforeRapid = await page.evaluate(() => Number(window.__ftdCtx?._loadGeneration ?? 0));
    await page.evaluate(() => {
        const select = document.getElementById('scenario-select');
        for (const id of ['empty', 'flux-pulse', 'empty']) {
            select.value = id;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await waitForEmptyReady(page, resizeSupported ? resizeTarget : initial.latticeSize);
    const final = await readEmptySnapshot(page);
    expect(final.generation, 'each rapid UI request receives its own monotonic generation')
        .toBe(beforeRapid + 3);
    expectExactNull(final);
    expect(realErrors(errors)).toEqual([]);
});

test('keeps null telemetry finite/zero while distinguishing unavailable data and collapsed demand', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: WASM_PATH, timeout: 90_000 });
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await waitForEmptyReady(page);

    // Request the deepest published channels through the real panel visibility
    // path. This distinguishes "not demanded" from a scientific zero.
    await page.locator('#tab-bar .tab[data-panel="lagrangian"]').click();
    await page.evaluate(() => {
        // The worker publishes a new demand-masked Lagrangian snapshot only on
        // a completed frame. Let the null control tick; exact zero is invariant.
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await expect.poll(async () => {
        const snapshot = await readEmptySnapshot(page);
        return snapshot.audit.status === 'supported' && snapshot.lagrangian.status === 'supported';
    }, { timeout: 15_000, message: 'demanded audit/Lagrangian snapshots were not published' }).toBe(true);

    const published = await readEmptySnapshot(page);
    expectExactNull(published);
    expectFiniteExactZeros(published.lagrangian.value, [
        'fieldKinetic', 'fieldGradient', 'coupling', 'velocity', 'gauss',
        'dissipation', 'gaussViolation', 'maxGaussError', 'totalFluxMag',
        'totalWaveEnergy', 'manifested', 'locked',
    ], 'Lagrangian');
    for (const key of ['bornInfeld', 'total', 'hamiltonian', 'totalAction']) {
        expect(Number.isFinite(Number(published.lagrangian.value[key])),
            `Lagrangian.${key} vacuum-offset channel remains finite`).toBe(true);
    }
    testInfo.annotations.push({
        type: 'telemetry-semantics',
        description: 'Born-Infeld/total/action include the lattice vacuum offset; exact-null excitation is asserted through diagnostics.dynamicEnergy and the zero field/interaction channels',
    });
    if (published.latency.status === 'unsupported-worker-contract') {
        expect(published.latency.length, 'unsupported latency volume is not presented as a measured empty field')
            .toBeNull();
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: 'WasmBridgeProxy does not publish a full latency volume; no parity assertion fabricated',
        });
    }

    const ui = await page.evaluate(async () => {
        const { getScale0TelemetryDemand } = await import('/js/telemetry/demand.js');
        const dock = window.__ftdCtx?.appShell?.panelDock;
        if (!dock) throw new Error('panel dock unavailable');
        dock.setCollapsed(true);
        await new Promise((resolve) => requestAnimationFrame(resolve));
        const demand = getScale0TelemetryDemand(window.__ftdCtx);
        const body = document.querySelector('.s0-overlay-body');
        const columns = [...document.querySelectorAll('.s0-overlay-col')];
        return {
            collapsed: document.getElementById('app')?.classList.contains('panels-collapsed'),
            wantAudit: demand.wantAudit,
            wantLag: demand.wantLag,
            wantGravity: demand.wantGravity,
            diagnostics: demand.diagnostics,
            applicabilityEmpty: body?.classList.contains('is-applicability-empty'),
            allOverlayColumnsHidden: columns.length > 0
                && columns.every((column) => getComputedStyle(column).display === 'none'),
            overlayDomains: document.getElementById('viewport-overlay')?.dataset.overlayDomains ?? null,
        };
    });
    expect(ui).toEqual({
        collapsed: true,
        wantAudit: false,
        wantLag: false,
        wantGravity: false,
        diagnostics: true,
        applicabilityEmpty: true,
        allOverlayColumnsHidden: true,
        overlayDomains: '',
    });
    expect(realErrors(errors)).toEqual([]);
});

test('matches the exact-null browser contract on the main-thread WASM runtime', async ({ page }, testInfo) => {
    test.setTimeout(150_000);
    const errors = attachConsoleWatcher(page);
    await page.addInitScript(() => { window.__ftdWasmWorker = false; });
    await gotoAndReady(page, { path: WASM_PATH, timeout: 90_000 });
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await waitForEmptyReady(page);

    const initial = await readEmptySnapshot(page);
    expect(initial.owner, 'the opt-out must exercise WasmBridge in the browser main thread')
        .toBe('wasm-main');
    expectExactNull(initial);

    const tickResult = await page.evaluate(() => {
        const caps = window.__ftdCtx?.bridge?.capabilities?.scale0;
        const before = Number(caps?.getScale0Diagnostics?.()?.tick ?? NaN);
        for (let i = 0; i < 16; i += 1) caps?.tickScale0?.();
        const after = Number(caps?.getScale0Diagnostics?.()?.tick ?? NaN);
        return { before, after };
    });
    expect(Number.isFinite(tickResult.before)).toBe(true);
    expect(tickResult.after, 'main-thread ticks advance the real WASM engine synchronously')
        .toBe(tickResult.before + 16);
    expectExactNull(await readEmptySnapshot(page));

    const beforeRapid = await page.evaluate(() => Number(window.__ftdCtx?._loadGeneration ?? 0));
    await page.evaluate(() => {
        const select = document.getElementById('scenario-select');
        for (const id of ['empty', 'flux-pulse', 'empty']) {
            select.value = id;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await waitForEmptyReady(page, initial.latticeSize);
    const final = await readEmptySnapshot(page);
    expect(final.owner).toBe('wasm-main');
    expect(final.generation).toBe(beforeRapid + 3);
    expectExactNull(final);
    testInfo.annotations.push({
        type: 'backend-coverage',
        description: 'exact-null contract qualified on main-thread WasmBridge; the serial worker tests qualify WasmBridgeProxy separately',
    });
    testInfo.annotations.push({
        type: 'unsupported-path',
        description: 'native GPU and WebSocket backends are not connected by this browser harness, so no parity claim is made for them',
    });
    expect(realErrors(errors)).toEqual([]);
});

test('recovers from a real hidden tab without stale generation, worker churn, or a resume burst', async ({ page }, testInfo) => {
    test.setTimeout(180_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: WASM_PATH, timeout: 90_000 });
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await waitForEmptyReady(page);

    const initial = await readEmptySnapshot(page);
    if (initial.owner !== 'wasm-worker') {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: `hidden-tab worker recovery requires WasmBridgeProxy; active runtime is ${initial.owner}`,
        });
        test.skip(true, 'WasmBridgeProxy is unavailable on this server/browser');
    }
    const workersBefore = await readWorkerCounters(page);
    const generationBefore = initial.generation;
    await page.evaluate(() => {
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await expect.poll(async () => Number((await readEmptySnapshot(page)).diagnostics.value?.tick ?? 0),
        { timeout: 10_000, message: 'worker did not begin ticking before tab backgrounding' })
        .toBeGreaterThan(0);

    const other = await page.context().newPage();
    try {
        await other.goto('about:blank');
        await other.bringToFront();
        const hiddenSupported = await page.waitForFunction(
            () => document.visibilityState === 'hidden',
            undefined,
            { timeout: 5_000 },
        ).then(() => true).catch(() => false);
        if (!hiddenSupported) {
            testInfo.annotations.push({
                type: 'unsupported-path',
                description: 'this headless Chromium session did not expose a real hidden-tab visibility transition; document.hidden was not fabricated',
            });
            test.skip(true, 'real hidden-tab lifecycle is not observable in this browser session');
        }

        const hiddenStart = await readEmptySnapshot(page);
        await page.waitForTimeout(1_200);
        const hiddenEnd = await readEmptySnapshot(page);
        expect(hiddenEnd.generation, 'backgrounding cannot schedule a scenario reload')
            .toBe(generationBefore);
        expect(hiddenEnd.diagnostics.value.tick, 'worker ticks remain monotonic while the page is hidden')
            .toBeGreaterThanOrEqual(hiddenStart.diagnostics.value.tick);

        await page.bringToFront();
        await page.waitForFunction(() => document.visibilityState === 'visible');
        // Start after one foreground rAF so the hidden interval itself is not
        // mislabeled as a dropped foreground frame.
        await page.evaluate(() => new Promise((resolve) => requestAnimationFrame(resolve)));
        const recovery = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const frames = [];
            const ticks = [];
            let previous = await new Promise((resolve) => requestAnimationFrame(resolve));
            for (let i = 0; i < 90; i += 1) {
                const now = await new Promise((resolve) => requestAnimationFrame(resolve));
                frames.push(now - previous);
                previous = now;
                const owner = getScale0State().fluxMock;
                ticks.push(Number(owner?.capabilities?.scale0?.getScale0Diagnostics?.()?.tick ?? NaN));
            }
            const tickDeltas = ticks.slice(1).map((tick, index) => tick - ticks[index]);
            const orderedFrames = [...frames].sort((a, b) => a - b);
            return {
                frameP95: orderedFrames[Math.floor(orderedFrames.length * 0.95)],
                maxTickDelta: Math.max(0, ...tickDeltas),
                minTickDelta: Math.min(0, ...tickDeltas),
                finiteTicks: ticks.every(Number.isFinite),
            };
        });
        const recovered = await readEmptySnapshot(page);
        const workersAfter = await readWorkerCounters(page);
        expect(recovered.generation).toBe(generationBefore);
        expectExactNull(recovered);
        expect(workersAfter, 'worker counters remain available').not.toBeNull();
        expect(workersAfter).toEqual(workersBefore);
        expect(recovery.finiteTicks).toBe(true);
        expect(recovery.minTickDelta, 'resume never moves the scientific clock backwards').toBe(0);
        expect(recovery.maxTickDelta, 'resume has no multi-tick catch-up burst between frames')
            .toBeLessThanOrEqual(4);
        expect(recovery.frameP95, 'foreground frame pacing recovers after a real hidden interval')
            .toBeLessThanOrEqual(25);
        await testInfo.attach('empty-hidden-tab-recovery', {
            body: JSON.stringify({
                backend: recovered.owner,
                hiddenTicks: hiddenEnd.diagnostics.value.tick - hiddenStart.diagnostics.value.tick,
                ...recovery,
            }, null, 2),
            contentType: 'application/json',
        });
        testInfo.annotations.push({
            type: 'lifecycle-scope',
            description: 'real Chromium visibility transition qualified for WasmBridgeProxy; no synthetic document.hidden override and no native GPU/WebSocket claim',
        });
    } finally {
        await other.close();
    }
    expect(realErrors(errors)).toEqual([]);
});

test('keeps reload listeners, workers, rAF subscriptions, and renderer allocations bounded', async ({ page }, testInfo) => {
    test.setTimeout(180_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: WASM_PATH, timeout: 90_000 });
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await waitForEmptyReady(page);
    await page.evaluate(() => new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve))));

    const initial = await readEmptySnapshot(page);
    const baseline = {
        raf: await rafSize(page),
        renderer: await getRendererMemory(page),
        workers: await readWorkerCounters(page),
    };
    const generations = [];
    const workerSamples = [];
    for (let i = 0; i < 5; i += 1) {
        const before = await page.evaluate(() => Number(window.__ftdCtx?._loadGeneration ?? 0));
        await page.locator('#btn-reset').click();
        await waitForEmptyReady(page, initial.latticeSize);
        const after = await page.evaluate(() => Number(window.__ftdCtx?._loadGeneration ?? 0));
        expect(after, `reset ${i + 1} is handled exactly once`).toBe(before + 1);
        generations.push(after);
        const counters = await readWorkerCounters(page);
        if (counters) {
            expect(counters.created, `reset ${i + 1}: every created worker is accounted for`)
                .toBe(counters.terminated + counters.live);
            expect(counters.live, `reset ${i + 1}: worker proxies do not accumulate`)
                .toBeLessThanOrEqual(1);
            workerSamples.push(counters);
        }
        expectExactNull(await readEmptySnapshot(page));
    }

    // A single native change event must produce exactly one load generation.
    // This is a behavioral proxy for duplicate scenario-select listeners.
    const beforeChange = await page.evaluate(() => Number(window.__ftdCtx?._loadGeneration ?? 0));
    await page.evaluate(() => {
        const select = document.getElementById('scenario-select');
        select.value = 'empty';
        select.dispatchEvent(new Event('change', { bubbles: true }));
    });
    await waitForEmptyReady(page, initial.latticeSize);
    const afterChange = await page.evaluate(() => Number(window.__ftdCtx?._loadGeneration ?? 0));
    expect(afterChange, 'one scenario-select event has exactly one registered load handler')
        .toBe(beforeChange + 1);

    await page.evaluate(() => new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve))));
    const final = {
        raf: await rafSize(page),
        renderer: await getRendererMemory(page),
        workers: await readWorkerCounters(page),
        singletonCounts: await page.evaluate(() => ({
            scenarioSelect: document.querySelectorAll('#scenario-select').length,
            latticeSize: document.querySelectorAll('#lattice-size').length,
            playButton: document.querySelectorAll('#btn-play').length,
        })),
    };
    expect(final.singletonCounts).toEqual({ scenarioSelect: 1, latticeSize: 1, playButton: 1 });
    expect(final.raf, 'scenario reloads do not accumulate rAF coordinator subscribers')
        .toBeLessThanOrEqual(baseline.raf);
    if (baseline.renderer && final.renderer) {
        expect(final.renderer.geometries, 'renderer geometry allocations stay bounded across reloads')
            .toBeLessThanOrEqual(baseline.renderer.geometries + 4);
        expect(final.renderer.textures, 'renderer texture allocations stay bounded across reloads')
            .toBeLessThanOrEqual(baseline.renderer.textures + 2);
    } else {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: 'Three.js renderer.info.memory was unavailable; no allocation value was fabricated',
        });
    }
    if (initial.owner === 'wasm-worker') {
        expect(baseline.workers).not.toBeNull();
        expect(final.workers.created).toBe(final.workers.terminated + final.workers.live);
        expect(final.workers.live).toBe(1);
    } else {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: `worker conservation is inapplicable on ${initial.owner}; main-thread lifecycle remains covered by generation/rAF/allocation proxies`,
        });
    }
    await testInfo.attach('empty-reload-lifecycle-proxies', {
        body: JSON.stringify({ baseline, final, generations, workerSamples }, null, 2),
        contentType: 'application/json',
    });
    testInfo.annotations.push({
        type: 'lifecycle-proxy',
        description: 'rAF subscriber count, renderer.info.memory, worker conservation, singleton DOM nodes, and one-event/one-generation are bounded proxies; they are not a heap proof',
    });
    expectExactNull(await readEmptySnapshot(page));
    expect(realErrors(errors)).toEqual([]);
});

test('records 60 FPS frame pacing at the largest lattice enabled for the active backend', async ({ page }, testInfo) => {
    test.setTimeout(240_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: WASM_PATH, timeout: 90_000 });
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await waitForEmptyReady(page);

    const target = await page.evaluate(() => Math.max(...[...document.querySelectorAll('#lattice-size option')]
        .filter((option) => !option.disabled)
        .map((option) => Number(option.value))
        .filter(Number.isFinite)));
    await page.selectOption('#lattice-size', String(target));
    await waitForEmptyReady(page, target);
    expectExactNull(await readEmptySnapshot(page));

    await page.evaluate(() => {
        window.__ftdCtx?.appShell?.panelDock?.setCollapsed(true);
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await page.waitForTimeout(1_200);

    const report = await page.evaluate(async () => {
        const deltas = [];
        let previous = await new Promise((resolve) => requestAnimationFrame(resolve));
        const start = previous;
        while (deltas.length < 240) {
            const now = await new Promise((resolve) => requestAnimationFrame(resolve));
            deltas.push(now - previous);
            previous = now;
        }
        const ordered = [...deltas].sort((a, b) => a - b);
        const percentile = (p) => ordered[Math.min(ordered.length - 1, Math.floor(ordered.length * p))];
        const durationMs = previous - start;
        return {
            frames: deltas.length,
            durationMs,
            effectiveFps: deltas.length * 1000 / durationMs,
            medianMs: percentile(0.5),
            p95Ms: percentile(0.95),
            p99Ms: percentile(0.99),
            maxMs: ordered[ordered.length - 1],
        };
    });
    const backend = (await readEmptySnapshot(page)).owner;
    console.log(`[empty qualification perf] backend=${backend} L=${target} `
        + `fps=${report.effectiveFps.toFixed(2)} median=${report.medianMs.toFixed(2)}ms `
        + `p95=${report.p95Ms.toFixed(2)}ms p99=${report.p99Ms.toFixed(2)}ms max=${report.maxMs.toFixed(2)}ms`);
    await testInfo.attach('empty-largest-lattice-frame-pacing', {
        body: JSON.stringify({ scenario: EMPTY, backend, latticeSize: target, ...report }, null, 2),
        contentType: 'application/json',
    });
    testInfo.annotations.push({
        type: 'performance-scope',
        description: `foreground Chromium rAF reference, ${backend}, empty L=${target}; not native-GPU parity`,
    });

    expect(report.frames).toBe(240);
    expect(report.effectiveFps, 'largest enabled lattice sustains the 60 Hz presentation cadence')
        .toBeGreaterThanOrEqual(59);
    expect(report.p95Ms, 'p95 frame time stays inside a 60 FPS budget').toBeLessThanOrEqual(16.9);
    expect(report.p99Ms, 'rare frame-pacing tail remains bounded').toBeLessThanOrEqual(25);
    expect(realErrors(errors)).toEqual([]);
});

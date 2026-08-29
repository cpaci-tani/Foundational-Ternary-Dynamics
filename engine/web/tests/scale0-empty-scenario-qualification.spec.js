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
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

const EMPTY = 'empty';

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
            latticeSize: Number(owner?.latticeSize ?? 0),
            flux: {
                status: flux ? 'supported' : 'not-published',
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

test('is an exact null after load, reset/reload, supported resize, and rapid generation changes', async ({ page }, testInfo) => {
    test.setTimeout(180_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { timeout: 90_000 });
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
    await gotoAndReady(page, { timeout: 90_000 });
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

test('records 60 FPS frame pacing at the largest lattice enabled for the active backend', async ({ page }, testInfo) => {
    test.setTimeout(240_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, { timeout: 90_000 });
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

// @ts-check
/**
 * Proper-time / lapse / de Broglie phase overlay and telemetry contract.
 *
 * The active Scale-0 owner may be a worker proxy. Tests therefore use the
 * public capability surface and canonical telemetry collector instead of an
 * inactive bridge's private `_module` field.
 */

import { test, expect } from '@playwright/test';
import {
    gotoAndReady,
    selectScale0Scenario,
    openDockPanel,
    attachConsoleWatcher,
    realErrors,
} from './_helpers.js';
import { SCALE0_SAMPLER_METHODS } from '../js/bridge/bridge-contract.js';

test.describe('proper-time/lapse/dbPhase sampler-kind registry', () => {
    test('registry maps all proper-time channels to native sampler methods', () => {
        expect(SCALE0_SAMPLER_METHODS.tau).toBe('getTauSampled');
        expect(SCALE0_SAMPLER_METHODS.dbPhase).toBe('getPhaseSampled');
        expect(SCALE0_SAMPLER_METHODS.lapse).toBe('getLapseSampled');
    });
});

test.describe('Scale-0 proper-time overlay wiring', () => {
    test('field flags, rendered controls and telemetry-grid channels are registered', async ({ page }) => {
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const [{ getScale0State }, dom, grid] = await Promise.all([
                import('/js/scales/scale0/state/store.js'),
                import('/js/scales/scale0/ui/dom.js'),
                import('/js/telemetry/registry/scale0-grid-channels.js'),
            ]);
            const keys = Object.keys(getScale0State().fieldFlags);
            const bindings = new Map(dom.FIELD_TOGGLE_BINDINGS);
            const targets = ['showProperTime', 'showLapse', 'showDBPhase'];
            return {
                controls: targets.map((key) => {
                    const buttonId = [...bindings.entries()].find(([, value]) => value === key)?.[0] || null;
                    return { key, inKeys: keys.includes(key), buttonId,
                        buttonExists: buttonId ? !!document.getElementById(buttonId) : false };
                }),
                channels: grid.SCALE0_GRID_CHANNELS
                    .filter(channel => channel.buffer.startsWith('ptime.'))
                    .map(channel => channel.key),
            };
        });
        for (const entry of result.controls) {
            expect(entry.inKeys, `${entry.key} should be a fieldFlags key`).toBe(true);
            expect(entry.buttonId, `${entry.key} should have a bound control`).toBeTruthy();
            expect(entry.buttonExists, `${entry.key} control should be rendered`).toBe(true);
        }
        expect(result.channels).toEqual(expect.arrayContaining([
            'properTimeMean', 'properTimeSpread', 'lapseMean', 'dbPhaseMean', 'dbPhaseCircVar',
        ]));
    });

    test('active owner exposes bounded proper-time sample records through its public capability', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            const owner = state.useFluxMock ? state.fluxMock : window.__ftdCtx?.bridge;
            const caps = owner?.capabilities?.scale0;
            const rows = ['tau', 'lapse', 'dbPhase'].map((kind) => {
                const sample = caps?.getScale0ProperTimeSamples?.({ kind, stride: 2 });
                return { kind, count: sample?.count, valuesLength: sample?.values?.length,
                    sampleTick: sample?.sampleTick ?? null, sourceEpoch: sample?.sourceEpoch ?? null };
            });
            return { ownerPresent: !!owner,
                cacheSurface: typeof caps?.getScale0ProperTimeSamples === 'function', rows };
        });
        expect(result.ownerPresent).toBe(true);
        expect(result.cacheSurface).toBe(true);
        for (const row of result.rows) {
            expect(Number.isSafeInteger(row.count), `${row.kind} count`).toBe(true);
            expect(row.count, `${row.kind} count is nonnegative`).toBeGreaterThanOrEqual(0);
            expect(row.valuesLength, `${row.kind} values array`).toBeGreaterThanOrEqual(row.count);
        }
        expect(realErrors(errors)).toEqual([]);
    });

    test('telemetry hub exposes one passive ptime group and source metadata slot', async ({ page }) => {
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const { telemetryHub } = await import('/js/telemetry-hub.js');
            return {
                channels: ['properTimeMean', 'properTimeSpread', 'lapseMean', 'dbPhaseMean', 'dbPhaseCircVar']
                    .map(key => ({ key, present: !!telemetryHub.ptime?.[key] })),
                meta: telemetryHub.getScale0TelemetryMeta('properTime'),
            };
        });
        expect(result.channels.every(channel => channel.present)).toBe(true);
        if (result.meta) {
            expect(['available', 'unavailable']).toContain(result.meta.status);
            expect(typeof result.meta.stale).toBe('boolean');
        }
    });
});

test('active WASM owner publishes one coherent proper-time observation consumed by Time', async ({ page }) => {
    test.setTimeout(90_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page);
    await selectScale0Scenario(page, 's0-seed-hydrogen', { settleMs: 1200 });
    await openDockPanel(page, 'time');

    // Exercise Card F's real one-click affordance. de_broglie_clock is owned
    // by a clocked scenario; the latency path enables gravity before its
    // dependent latency_field through the production checkbox handlers.
    await page.evaluate(() => {
        const input = /** @type {HTMLInputElement|null} */ (document.getElementById('t-latency-field'));
        if (input?.checked) {
            input.checked = false;
            input.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    const enable = page.locator('#time-panel-enable-ptime');
    await expect(enable).toBeVisible();
    await expect(enable).toHaveText('Enable gravity + latency_field');
    await enable.click();
    await expect.poll(() => page.evaluate(() =>
        /** @type {HTMLInputElement|null} */ (document.getElementById('t-gravity'))?.checked,
    )).toBe(true);
    await expect.poll(() => page.evaluate(() =>
        /** @type {HTMLInputElement|null} */ (document.getElementById('t-latency-field'))?.checked,
    )).toBe(true);

    const play = page.locator('#btn-play');
    if (await play.getAttribute('data-paused') === 'true') await play.click();

    await expect.poll(async () => page.evaluate(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const meta = telemetryHub.getScale0TelemetryMeta('properTime');
        const metrics = telemetryHub.s0.properTime;
        return !!meta && meta.stale !== true && Number.isSafeInteger(meta.tick)
            && metrics?.hasField === true && metrics.tauCount > 0
            && metrics.lapseCount > 0 && metrics.phaseCount > 0
            && metrics.sampleTick === meta.tick
            && telemetryHub.ptime.properTimeMean.count > 0;
    }), { timeout: 30_000,
        message: 'canonical proper-time collector should publish a coherent manifested-field observation' }).toBe(true);

    if (await play.getAttribute('data-paused') === 'false') await play.click();
    const result = await page.evaluate(async () => {
        const [{ telemetryHub }, { getScale0State }] = await Promise.all([
            import('/js/telemetry-hub.js'), import('/js/scales/scale0/state/store.js'),
        ]);
        const state = getScale0State();
        const owner = state.useFluxMock ? state.fluxMock : window.__ftdCtx?.bridge;
        const meta = telemetryHub.getScale0TelemetryMeta('properTime');
        const metrics = telemetryHub.s0.properTime;
        const buffer = telemetryHub.ptime.properTimeMean;
        const card = document.querySelector('#time-panel-card-f');
        return { activeOwner: !!owner, coherent: metrics?.coherent,
            metricTick: metrics?.sampleTick, metaTick: meta?.tick, sourceEpoch: meta?.sourceEpoch,
            bufferTick: buffer.getTick(buffer.count - 1), mean: buffer.last(),
            lapseMean: telemetryHub.ptime.lapseMean.last(), cardText: card?.textContent || '' };
    });
    expect(result.activeOwner).toBe(true);
    expect(result.coherent).toBe(true);
    expect(result.metricTick).toBe(result.metaTick);
    expect(result.bufferTick).toBe(result.metaTick);
    expect(Number.isFinite(result.sourceEpoch)).toBe(true);
    expect(result.mean).toBeGreaterThanOrEqual(0);
    expect(Number.isFinite(result.lapseMean)).toBe(true);
    expect(result.cardText).toContain('τ mean / min / max');
    expect(realErrors(errors)).toEqual([]);
});

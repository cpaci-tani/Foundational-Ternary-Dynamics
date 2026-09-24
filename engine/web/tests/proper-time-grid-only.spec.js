// @ts-check
import { test, expect } from '@playwright/test';
import { bootDashboard, openDockPanel, selectScale0Scenario } from './_helpers.js';

test('Grid alone owns fresh canonical proper-time telemetry and releases it when collapsed', async ({ page }) => {
    test.setTimeout(90_000);
    await bootDashboard(page, { engine: 'wasm', requiredCapabilities: ['scale0'] });
    await selectScale0Scenario(page, 's0-seed-hydrogen', { settleMs: 1200 });
    await page.evaluate(() => {
        // latency_field is dependency-gated by gravity; exercise the real
        // handlers in prerequisite order so this fixture remains valid.
        for (const id of ['t-gravity', 't-latency-field', 't-de-broglie']) {
            const input = /** @type {HTMLInputElement|null} */ (document.getElementById(id));
            if (input && !input.checked) {
                input.checked = true;
                input.dispatchEvent(new Event('change', { bubbles: true }));
            }
        }
    });
    await openDockPanel(page, 'telemetry-grid');

    await page.waitForFunction(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const meta = telemetryHub.getScale0TelemetryMeta('properTime');
        const ptime = telemetryHub.s0.properTime;
        return meta?.stale === false && Number.isSafeInteger(meta?.sampleTick)
            && Number.isSafeInteger(meta?.sourceEpoch) && ptime?.hasField === true;
    }, undefined, { timeout: 45_000 });

    const fresh = await page.evaluate(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const { getScale0TelemetryDemand } = await import('/js/telemetry/demand.js');
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const ctx = window.__ftdCtx;
        const state = getScale0State();
        const owner = state.useFluxMock ? state.fluxMock : ctx?.bridge;
        const meta = telemetryHub.getScale0TelemetryMeta('properTime');
        return {
            timeActive: document.getElementById('panel-time')?.classList.contains('active'),
            demand: getScale0TelemetryDemand(ctx, state),
            meta: { tick: meta?.sampleTick, sourceEpoch: meta?.sourceEpoch, stale: meta?.stale },
            sample: telemetryHub.s0.properTime && {
                tick: telemetryHub.s0.properTime.sampleTick,
                sourceEpoch: telemetryHub.s0.properTime.sourceEpoch,
                hasField: telemetryHub.s0.properTime.hasField,
            },
            historyTick: telemetryHub.ptime.properTimeMean.getTick(telemetryHub.ptime.properTimeMean.count - 1),
            wants: owner?._samplerWants ? [...owner._samplerWants.wanted()] : null,
        };
    });
    expect(fresh.timeActive).toBe(false);
    expect(fresh.demand.wantProperTime).toBe(true);
    expect(fresh.demand.wantGravity).toBe(false);
    expect(fresh.sample).toEqual(expect.objectContaining({
        tick: fresh.meta.tick, sourceEpoch: fresh.meta.sourceEpoch, hasField: true,
    }));
    expect(fresh.historyTick).toBe(fresh.meta.tick);
    if (fresh.wants) {
        expect(fresh.wants).toEqual(expect.arrayContaining([
            `tau@${fresh.demand.properTimeStride}`,
            `lapse@${fresh.demand.properTimeStride}`,
            `dbPhase@${fresh.demand.properTimeStride}`,
        ]));
    }

    await page.locator('#btn-panel-toggle').click();
    await page.waitForFunction(async () => {
        const { getScale0TelemetryDemand } = await import('/js/telemetry/demand.js');
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const ctx = window.__ftdCtx;
        const state = getScale0State();
        const owner = state.useFluxMock ? state.fluxMock : ctx?.bridge;
        return document.getElementById('app')?.classList.contains('panels-collapsed')
            && getScale0TelemetryDemand(ctx, state).wantProperTime === false
            && (!owner?._samplerWants || ![...owner._samplerWants.wanted()]
                .some((key) => /^(tau|lapse|dbPhase)@/.test(key)));
    }, undefined, { timeout: 15_000 });
});

// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    bootDashboard,
    openDockPanel,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

const DASH = '\u2014';

async function activeClockToggles(page) {
    return page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const owner = state.useFluxMock ? state.fluxMock : window.__ftdCtx?.bridge;
        return {
            gravity: owner?.getToggle?.('gravity'),
            latency: owner?.getToggle?.('latency_field'),
            clock: owner?.getToggle?.('de_broglie_clock'),
        };
    });
}

test('flux packet Grid explains zero sampled clock support without changing physics toggles', async ({ page }) => {
    test.setTimeout(90_000);
    const errors = attachConsoleWatcher(page);
    await bootDashboard(page, { engine: 'wasm', requiredCapabilities: ['scale0'] });
    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 1200 });
    const togglesBefore = await activeClockToggles(page);

    await openDockPanel(page, 'telemetry-grid');
    const card = page.locator('.telemetry-card[data-channel-key="properTimeMean"]');
    await card.scrollIntoViewIfNeeded();

    await expect.poll(() => page.evaluate(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const meta = telemetryHub.getScale0TelemetryMeta('properTime');
        return meta && {
            status: meta.status,
            reason: meta.unavailableReason,
            detail: meta.unavailableDetail,
            stale: meta.stale,
        };
    }), {
        timeout: 30_000,
        message: 'zero-particle flux packet should publish an explicit proper-time availability reason',
    }).toEqual({
        status: 'unavailable',
        reason: 'proper-time-disabled',
        detail: 'no-manifested-voxel-support',
        stale: true,
    });

    await expect(card.locator('.telemetry-card-value')).toHaveText(DASH);
    await expect(card.locator('.telemetry-card-status')).toBeVisible();
    await expect(card.locator('.telemetry-card-status')).toContainText('Clock disabled');
    await expect(card.locator('.telemetry-card-status')).toContainText('No manifested clock samples');
    expect(await activeClockToggles(page)).toEqual(togglesBefore);
    expect(realErrors(errors)).toEqual([]);
});

test('Grid alone publishes a coherent clocked-scenario observation', async ({ page }) => {
    test.setTimeout(90_000);
    const errors = attachConsoleWatcher(page);
    await bootDashboard(page, { engine: 'wasm', requiredCapabilities: ['scale0'] });
    await selectScale0Scenario(page, 's0-seed-de-broglie-clock', { settleMs: 1200 });
    const togglesBefore = await activeClockToggles(page);

    await openDockPanel(page, 'telemetry-grid');
    const play = page.locator('#btn-play');
    if (await play.getAttribute('data-paused') === 'true') await play.click();

    const card = page.locator('.telemetry-card[data-channel-key="properTimeMean"]');
    await card.scrollIntoViewIfNeeded();
    await expect.poll(() => page.evaluate(async () => {
        const [{ telemetryHub }, { getScale0TelemetryDemand }, { getScale0State }] = await Promise.all([
            import('/js/telemetry-hub.js'),
            import('/js/telemetry/demand.js'),
            import('/js/scales/scale0/state/store.js'),
        ]);
        const meta = telemetryHub.getScale0TelemetryMeta('properTime');
        const metrics = telemetryHub.s0.properTime;
        return {
            timeActive: document.getElementById('panel-time')?.classList.contains('active') === true,
            demanded: getScale0TelemetryDemand(window.__ftdCtx, getScale0State()).wantProperTime,
            available: meta?.status === 'available' && meta.stale === false,
            coherent: metrics?.coherent === true,
            sameTick: Number.isSafeInteger(meta?.sampleTick)
                && metrics?.sampleTick === meta.sampleTick,
            supported: metrics?.tauCount > 0 && metrics?.lapseCount > 0
                && metrics?.phaseCount > 0,
        };
    }), {
        timeout: 30_000,
        message: 'the Grid demand should drive one canonical clock observation without the Time panel',
    }).toEqual({
        timeActive: false,
        demanded: true,
        available: true,
        coherent: true,
        sameTick: true,
        supported: true,
    });

    await expect(card.locator('.telemetry-card-value')).not.toHaveText(DASH);
    await expect(card.locator('.telemetry-card-status')).toBeHidden();
    expect(await activeClockToggles(page)).toEqual(togglesBefore);
    expect(realErrors(errors)).toEqual([]);
});

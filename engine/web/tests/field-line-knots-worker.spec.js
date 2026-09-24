import { test, expect } from '@playwright/test';
import { bootDashboard, openDockPanel, selectScale0Scenario,
    attachConsoleWatcher, realErrors } from './_helpers.js';

test('real WASM knot workers publish advancing source-tagged results across scenario changes', async ({ page }) => {
    const errors = attachConsoleWatcher(page);
    await bootDashboard(page, { engine: 'wasm' });
    await selectScale0Scenario(page, 'flux-pulse');
    await openDockPanel(page, 'knots');
    await page.evaluate(async () => {
        const { FieldLineKnotTracker } = await import('/js/scales/scale0/runtime/field-line-knots.js');
        window.__mainThreadKnotRecords = 0;
        FieldLineKnotTracker.prototype.record = function () {
            window.__mainThreadKnotRecords++;
            throw new Error('Dense knot analysis unexpectedly ran on the UI thread');
        };
    });
    await page.locator('#kp-toggle-tracking').check();
    if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') {
        await page.locator('#btn-play').click();
    }
    const observations = () => page.evaluate(async () => {
        const { getFieldLineKnotTracker } = await import('/js/scales/scale0/runtime/field-line-knots.js');
        return Object.fromEntries(['e', 'b', 'flux'].map(field => {
            const tracker = getFieldLineKnotTracker(field);
            const telemetry = tracker.getTelemetry();
            return [field, { provenance: tracker.getAdoptedWorkerProvenance(),
                tick: telemetry.sampleTick, status: telemetry.status ?? null }];
        }));
    });
    const current = value => ['e', 'b', 'flux'].every(field => {
        const row = value[field];
        return row.status === null && row.provenance?.field === field
            && Number.isSafeInteger(row.tick) && row.tick >= 0
            && row.provenance.sampleTick === row.tick;
    });
    await expect.poll(async () => current(await observations()), { timeout: 30_000 }).toBe(true);
    const first = await observations();
    await expect.poll(async () => {
        const next = await observations();
        return current(next) && ['e', 'b', 'flux'].every(field => next[field].tick > first[field].tick);
    }, { timeout: 30_000 }).toBe(true);

    await selectScale0Scenario(page, 'flux-thermalization');
    if (await page.locator('#btn-play').getAttribute('data-paused') === 'true') {
        await page.locator('#btn-play').click();
    }
    await expect.poll(async () => {
        const next = await observations();
        return current(next) && ['e', 'b', 'flux'].every(field =>
            next[field].provenance.loadGeneration !== first[field].provenance.loadGeneration);
    }, { timeout: 30_000 }).toBe(true);
    const changed = await observations();
    await expect.poll(async () => {
        const next = await observations();
        return current(next) && ['e', 'b', 'flux'].every(field => next[field].tick > changed[field].tick);
    }, { timeout: 30_000 }).toBe(true);
    expect(await page.evaluate(() => window.__mainThreadKnotRecords)).toBe(0);
    expect(realErrors(errors)).toEqual([]);
});

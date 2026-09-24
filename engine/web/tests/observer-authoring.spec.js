import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function openPausedObserver(page) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        workspace.setSetting('renderScale', 0.25);
        workspace.setSetting('autoQuality', false);
        await workspace.command({ type: 'pause' });
    });
}

test('the storage cap control changes the real store budget and never evicts an existing named save', async ({ page }) => {
    await openPausedObserver(page);
    await page.getByRole('button', { name: 'Saves', exact: true }).click();
    const limit = page.getByLabel('Storage limit · MiB', { exact: true });
    await limit.fill('2');
    await limit.press('Tab');
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').storage.storageCapBytes)).toBe(2 * 1048576);
    await page.getByLabel('World name', { exact: true }).fill('Keep this named frame');
    await page.getByRole('button', { name: 'Save world', exact: true }).click();
    await expect(page.locator('.observer-save')).toContainText('Keep this named frame');
    const saved = await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        return (await workspace.storage.list()).find(record => record.name === 'Keep this named frame');
    });
    expect(saved.kind).toBe('named');
    await limit.fill('1');
    await limit.press('Tab');
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').storage.storageCapBytes)).toBe(1048576);
    expect(await page.evaluate(async id => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { exists: (await workspace.storage.list()).some(record => record.id === id),
            cap: workspace.settings.storageCapMiB, autosave: workspace.storage.autosave };
    }, saved.id)).toEqual({ exists: true, cap: 1, autosave: false });
    await limit.fill('0');
    await limit.press('Tab');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').storage.storageCapBytes)).toBe(1048576);
});

test('spectrum and point-force controls preview without writing, commit together, and drive the selected clock', async ({ page }) => {
    await openPausedObserver(page);
    const target = await page.evaluate(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const clock = workspace.snapshot.entities.find(entity => entity.alive && entity.shape === 'clock');
        const extended = workspace.snapshot.entities.find(entity => entity.alive && entity.shape === 'box');
        return { clock: { id: clock.id, revision: clock.revision, spectral: clock.spectral,
            properAcceleration: clock.properAcceleration, position: clock.position }, extendedId: extended.id };
    });
    await page.getByRole('button', { name: 'Objects', exact: true }).click();
    await page.getByLabel('Selected object', { exact: true }).selectOption(target.clock.id);
    await page.getByLabel('Emission spectrum', { exact: true }).selectOption('red-line');
    await page.locator('[data-observer-entity-field="properAcceleration.0"]').fill('0.2');
    const preview = await page.evaluate(id => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const entity = workspace.snapshot.entities.find(item => item.id === id);
        return { revision: entity.revision, spectral: entity.spectral, force: entity.properAcceleration,
            previewSpectral: workspace.preview.spectral, previewForce: workspace.preview.properAcceleration };
    }, target.clock.id);
    expect(preview).toEqual({ revision: target.clock.revision, spectral: target.clock.spectral,
        force: target.clock.properAcceleration, previewSpectral: 'red-line', previewForce: [0.2, 0, 0] });
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    await page.waitForFunction(id => {
        const entity = window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(item => item.id === id);
        return entity.spectral === 'red-line' && entity.properAcceleration[0] === 0.2;
    }, target.clock.id);
    const committed = await page.evaluate(id => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const entity = workspace.snapshot.entities.find(item => item.id === id);
        return { revision: entity.revision, oldSpectrumRetained: workspace.snapshot.segments.some(segment => segment.entityId === id && segment.spectral === 'white' && segment.end !== null) };
    }, target.clock.id);
    expect(committed.revision).toBe(target.clock.revision + 1);
    expect(committed.oldSpectrumRetained).toBe(true);
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await page.getByRole('button', { name: 'Play', exact: true }).click();
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(item => item.id === id).velocity[0] > 0.005, target.clock.id);
    await page.getByRole('button', { name: 'Pause', exact: true }).click();
    const velocity = await page.evaluate(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(item => item.id === id).velocity, target.clock.id);
    expect(velocity[0]).toBeGreaterThan(0);
    expect(Math.hypot(...velocity)).toBeLessThan(1);
    await page.getByRole('button', { name: 'Objects', exact: true }).click();
    await page.getByLabel('Selected object', { exact: true }).selectOption(target.extendedId);
    await expect(page.locator('[data-observer-entity-field^="properAcceleration."]')).toHaveCount(0);
});

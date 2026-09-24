/* global window, document */
import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function openPaused(page) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale', 0.25); w.setSetting('autoQuality', false);
        await w.command({ type: 'pause' });
        await w.command({ type: 'observer', patch: { position: [0, 1, 8], yaw: 0, pitch: 0 } });
        w.input.setPose({ yaw: 0, pitch: 0 });
    });
    await page.waitForFunction(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.hit?.entityId === w.snapshot.entities[0].id;
    });
}

async function capture(page) {
    await page.locator('.observer-canvas').click({ position: { x: 300, y: 260 } });
    await expect.poll(() => page.evaluate(() => document.pointerLockElement === window.__FTD_DEV__.registry.get('observerWorkspace').renderer.canvas)).toBe(true);
}

test('entry click captures, E selects the crosshair target for editing, and an empty aim never reselects a stale object', async ({ page }) => {
    await openPaused(page);
    const id = await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities[0].id);
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').hit?.entityId === id, id);
    await capture(page);
    await expect(page.locator('[data-observer-panel]')).toBeHidden();
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').selectedId)).toBeNull();
    await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-panel="objects"]')).toBeVisible();
    await expect(page.getByLabel('Selected object', { exact: true })).toHaveValue(id);
    expect(await page.evaluate(() => document.pointerLockElement)).toBeNull();
    await page.getByLabel('Name', { exact: true }).fill('Crosshair-selected sphere');
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    await expect(page.locator('[data-observer-selection]')).toContainText('Selected: Crosshair-selected sphere');
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.input.setPose({ yaw: 0, pitch: 1.4 });
    });
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').hit === null);
    await expect(page.locator('[data-observer-target] strong')).toHaveText('Infinite potential');
    await expect(page.locator('[data-observer-selection]')).toContainText('Selected: Crosshair-selected sphere');
    await page.locator('.observer-canvas').focus(); await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-status]')).toContainText('No object under the crosshair');
    await expect(page.locator('[data-observer-panel]')).toBeHidden();
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').selectedId)).toBeNull();
});

test('crosshair inspection preserves the mirrored historical source and supports restoring it', async ({ page }) => {
    await openPaused(page);
    const deleted = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        const entity = w.snapshot.entities[0];
        await w.command({ type: 'delete', id: entity.id });
        await w.command({ type: 'observer', patch: { position: [0, -1, 8] } });
        return { id: entity.id, revision: entity.revision };
    });
    await page.waitForFunction(id => {
        const hit = window.__FTD_DEV__.registry.get('observerWorkspace').hit;
        return hit?.entityId === id && hit.mirrored && hit.historical;
    }, deleted.id);
    await capture(page); await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-panel="objects"]')).toContainText('Historical image');
    await expect(page.locator('[data-observer-panel="objects"]')).toContainText('Mirrored image across the plane');
    expect(await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { id: w.selectedId, revision: w.selectedHit.revision, mirrored: w.selectedHit.mirrored, historical: w.selectedHit.historical };
    })).toEqual({ ...deleted, mirrored: true, historical: true });
    await page.getByRole('button', { name: 'Restore', exact: true }).click();
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(e => e.id === id)?.alive, deleted.id);
    expect(await page.evaluate(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.filter(e => e.id === id).length, deleted.id)).toBe(1);
});

test('impulses expose profile limits, change paused velocity, reject zero, and can explicitly resume motion', async ({ page }) => {
    await openPaused(page);
    await page.locator('.observer-canvas').focus(); await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-panel="objects"]')).toBeVisible();
    await expect(page.getByRole('button', { name: 'Apply impulse', exact: true })).toBeDisabled();
    await page.getByRole('button', { name: 'Use Playground physics', exact: true }).click();
    await expect(page.getByRole('button', { name: 'Apply impulse', exact: true })).toBeEnabled();
    await page.getByLabel('Rest mass', { exact: true }).fill('2');
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    await expect(page.locator('[data-observer-impulse="impulse.1"]')).toHaveValue('6');
    const before = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { time: w.snapshot.time, tick: w.snapshot.tick, entity: w.snapshot.entities.find(e => e.id === w.selectedId) };
    });
    await page.getByRole('button', { name: 'Apply impulse', exact: true }).click();
    await expect(page.locator('[data-observer-status]')).toContainText('Paused: press Play');
    const after = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { time: w.snapshot.time, tick: w.snapshot.tick, playing: w.snapshot.playing, entity: w.snapshot.entities.find(e => e.id === w.selectedId) };
    });
    expect(after.time).toBe(before.time); expect(after.tick).toBe(before.tick); expect(after.playing).toBe(false);
    expect(after.entity.velocity[1] - before.entity.velocity[1]).toBeCloseTo(3, 5);
    expect(after.entity.position).toEqual(before.entity.position);
    await page.locator('[data-observer-impulse="impulse.1"]').fill('0');
    await page.getByRole('button', { name: 'Apply impulse', exact: true }).click();
    await expect(page.locator('[data-observer-status]')).toContainText('nonzero impulse');
    await page.locator('[data-observer-impulse="impulse.1"]').fill('6');
    await page.getByRole('button', { name: 'Apply impulse & play', exact: true }).click();
    await expect(page.locator('[data-observer-panel]')).toBeHidden();
    await page.waitForFunction(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.snapshot.playing && w.snapshot.entities.find(e => e.id === id).position[1] > 1.05;
    }, before.entity.id);
});

test('fixed and prescribed bodies visibly disable impulses until a dynamic edit is committed', async ({ page }) => {
    await openPaused(page);
    await page.locator('.observer-canvas').focus(); await page.keyboard.press('KeyE');
    await page.getByRole('button', { name: 'Use Playground physics', exact: true }).click();
    await expect(page.getByLabel('Body behavior', { exact: true })).toBeVisible();
    for (const type of ['fixed', 'kinematic']) {
        await page.getByLabel('Body behavior', { exact: true }).selectOption(type);
        await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
        await expect(page.getByRole('button', { name: 'Apply impulse', exact: true })).toBeDisabled();
        await expect(page.locator('[data-observer-panel="objects"]')).toContainText('Choose Dynamic body and apply changes');
    }
    await page.getByLabel('Body behavior', { exact: true }).selectOption('dynamic');
    await expect(page.getByRole('button', { name: 'Apply impulse', exact: true })).toBeDisabled();
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    await expect(page.getByRole('button', { name: 'Apply impulse', exact: true })).toBeEnabled();
});

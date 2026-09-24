/* global window */
import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function openPaused(page) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'pause' });
        w.setSetting('feedbackEnabled', false); w.setSetting('autoQuality', false); w.setSetting('renderScale', 0.3);
        window.__planeLatticeOwner = window.__FTD_DEV__.registry.get('observerHost').deps.readLattice().owner;
        window.__planeLatticeTick = window.__planeLatticeOwner.currentTick();
    });
}

test('World controls commit gravity and collision settings together and preserve the retained lattice', async ({ page }, testInfo) => {
    await openPaused(page);
    await page.getByRole('button', { name: 'World', exact: true }).click();
    await expect(page.locator('[data-observer-panel="world"]')).toContainText('World gravity and rigid-body collisions are available in Playground');
    await page.getByRole('button', { name: 'Use Playground physics', exact: true }).click();
    await expect(page.getByLabel('Gravity model', { exact: true })).toHaveValue('plane');
    await expect(page.getByLabel('Gravity strength', { exact: true })).toHaveValue('9.81');
    await page.screenshot({ path: testInfo.outputPath('world-gravity-controls.png') });
    await page.getByLabel('Gravity strength', { exact: true }).fill('4.5');
    await page.getByLabel('Object-to-object collisions', { exact: true }).uncheck();
    await page.getByLabel('Collisions with the central plane', { exact: true }).uncheck();
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.gravityStrength)).toBe(9.81);
    await page.getByRole('button', { name: 'Apply world physics', exact: true }).click();
    await expect(page.locator('[data-observer-status]')).toContainText('World physics applied');
    expect(await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { mode: w.snapshot.gravityMode, strength: w.snapshot.gravityStrength,
            objects: w.snapshot.objectCollisions, plane: w.snapshot.planeCollision, playing: w.snapshot.playing };
    })).toEqual({ mode: 'plane', strength: 4.5, objects: false, plane: false, playing: false });
    await page.getByRole('button', { name: 'Undo edit', exact: true }).click();
    await expect(page.getByLabel('Gravity strength', { exact: true })).toHaveValue('9.81');
    await expect(page.getByLabel('Object-to-object collisions', { exact: true })).toBeChecked();
    await page.getByLabel('Gravity model', { exact: true }).selectOption('uniform');
    await page.locator('[data-observer-global-gravity="gravity.1"]').fill('-2');
    await page.getByRole('button', { name: 'Apply world physics', exact: true }).click();
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.gravityMode === 'uniform');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.gravity)).toEqual([0, -2, 0]);
    expect(await page.evaluate(() => {
        const owner = window.__FTD_DEV__.registry.get('observerHost').deps.readLattice().owner;
        return owner === window.__planeLatticeOwner && owner.currentTick() === window.__planeLatticeTick;
    })).toBe(true);
});

test('a below-plane crosshair selection edits and impulses the linked image in its displayed coordinates', async ({ page }) => {
    await openPaused(page);
    const id = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'profile', profile: 'playground' });
        await w.command({ type: 'world-physics', patch: { gravityStrength: 0 } });
        const id = w.snapshot.entities[0].id;
        await w.command({ type: 'update', id, patch: { position: [0, 2, -6], velocity: [0, 0, 0], mass: 1 } });
        await w.command({ type: 'observer', patch: { position: [0, -2, 8], yaw: 0, pitch: 0 } });
        w.input.setPose({ yaw: 0, pitch: 0 });
        return id;
    });
    await page.waitForFunction(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.hit?.entityId === id && w.hit.mirrored;
    }, id);
    await page.locator('.observer-canvas').focus(); await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-panel="objects"]')).toContainText('Mirrored image · world coordinates');
    await expect(page.locator('[data-observer-entity-field="position.1"]')).toHaveValue('-2');
    await page.getByRole('button', { name: 'Apply impulse', exact: true }).click();
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(e => e.id === id).velocity[1] < -2.9, id);
    await page.getByLabel('Collisions on object', { exact: true }).uncheck();
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(e => e.id === id).collisions === false, id);
    await page.locator('[data-observer-entity-field="position.1"]').fill('-4');
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    expect(await page.evaluate(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { count: w.snapshot.entities.filter(e => e.id === id).length, y: w.snapshot.entities.find(e => e.id === id).position[1] };
    }, id)).toEqual({ count: 1, y: 4 });
});

test('opposite-side objects collide through linked mirrors and gravity pulls both toward the plane', async ({ page }) => {
    await openPaused(page);
    const result = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.active = false;
        try {
            await w.command({ type: 'profile', profile: 'playground' });
            for (const entity of [...w.snapshot.entities]) await w.command({ type: 'delete', id: entity.id });
            await w.command({ type: 'world-physics', patch: { gravityStrength: 0, objectCollisions: true, planeCollision: true } });
            for (const [name, position, velocity] of [['Upper', [-2, 3, -6], [2, 0, 0]], ['Lower', [2, -3, -6], [-2, 0, 0]]]) {
                await w.command({ type: 'create', entity: { name, shape: 'sphere', position, velocity, size: [1, 1, 1], restitution: 1, friction: 0, damping: 0 } });
            }
            await w.command({ type: 'play' });
            for (let i = 0; i < 5; i++) await w.client.send({ type: 'advance', dt: 0.25, input: {}, sessionId: w.snapshot.sessionId, epoch: w.snapshot.epoch });
            await w.command({ type: 'pause' });
            const collision = w.snapshot.entities.filter(e => e.alive).map(e => ({ name: e.name, vx: e.velocity[0], y: e.position[1] }));
            await w.command({ type: 'world-physics', patch: { gravityStrength: 4 } });
            for (const e of w.snapshot.entities.filter(e => e.alive)) await w.command({ type: 'update', id: e.id, patch: { velocity: [0, 0, 0] } });
            await w.command({ type: 'play' });
            await w.client.send({ type: 'advance', dt: 0.25, input: {}, sessionId: w.snapshot.sessionId, epoch: w.snapshot.epoch });
            await w.command({ type: 'pause' });
            return { collision, falling: w.snapshot.entities.filter(e => e.alive).map(e => ({ name: e.name, vy: e.velocity[1], y: e.position[1] })) };
        } finally { w.active = true; }
    });
    expect(result.collision[0].vx).toBeLessThan(-1.5);
    expect(result.collision[1].vx).toBeGreaterThan(1.5);
    expect(result.falling[0].vy).toBeLessThan(-0.8);
    expect(result.falling[1].vy).toBeGreaterThan(0.8);
    expect(result.falling[0].y).toBeCloseTo(-result.falling[1].y, 4);
});

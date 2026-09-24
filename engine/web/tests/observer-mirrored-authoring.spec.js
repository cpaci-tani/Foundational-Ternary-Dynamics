/* global window */
import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

test('mirrored inspector numbers, impulse and gizmo drag act in the selected image frame', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    const id = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale', 0.25); w.setSetting('autoQuality', false);
        await w.command({ type: 'pause' });
        await w.command({ type: 'profile', profile: 'playground' });
        const id = w.snapshot.entities[0].id;
        await w.command({ type: 'update', id, patch: { position: [0, 2, -2], rotation: [.2, .3, .4],
            velocity: [0, 0, 0], angularVelocity: [.5, .6, .7], mass: 2, gravity: false } });
        await w.command({ type: 'observer', patch: { position: [0, -2, 8], yaw: 0, pitch: 0 } });
        w.input.setPose({ yaw: 0, pitch: 0 });
        return id;
    });
    await page.waitForFunction(id => {
        const hit = window.__FTD_DEV__.registry.get('observerWorkspace').hit;
        return hit?.entityId === id && hit.mirrored;
    }, id);
    await page.locator('.observer-canvas').focus(); await page.keyboard.press('KeyE');
    const field = name => page.locator(`[data-observer-entity-field="${name}"]`);
    await expect(field('position.1')).toHaveValue('-2');
    expect(Number(await field('rotation.0').inputValue())).toBeCloseTo(-.2 * 180 / Math.PI, 5);
    await field('position.1').fill('-3');
    await field('velocity.1').fill('2');
    await field('rotation.0').fill('30');
    await field('angularVelocity.2').fill('1');
    expect(await page.evaluate(() => {
        const p = window.__FTD_DEV__.registry.get('observerWorkspace').preview;
        return { y: p.position[1], vy: p.velocity[1], rx: p.rotation[0], wz: p.angularVelocity[2] };
    })).toEqual({ y: 3, vy: -2, rx: -Math.PI / 6, wz: -1 });
    await page.getByRole('button', { name: 'Apply changes', exact: true }).click();
    await expect.poll(() => page.evaluate(id => {
        const e = window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(e => e.id === id);
        return { y: e.position[1], vy: e.velocity[1] };
    }, id)).toEqual({ y: 3, vy: -2 });
    await page.locator('[data-observer-impulse="impulse.1"]').fill('6');
    await page.getByRole('button', { name: 'Apply impulse', exact: true }).click();
    await expect.poll(() => page.evaluate(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(e => e.id === id).velocity[1], id)).toBeCloseTo(-5, 5);
    await expect(page.locator('[data-observer-status]')).toContainText('mirrored image velocity');
    const handle = page.getByRole('button', { name: 'Drag Y axis', exact: true });
    const bounds = await handle.boundingBox();
    await page.mouse.move(bounds.x + bounds.width / 2, bounds.y + bounds.height / 2);
    await page.mouse.down();
    await page.mouse.move(bounds.x + bounds.width / 2, bounds.y + bounds.height / 2 - 50, { steps: 5 });
    await page.mouse.up();
    await expect.poll(() => page.evaluate(id => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.entities.find(e => e.id === id).position[1], id)).toBeCloseTo(2, 5);
    expect(await page.evaluate(async id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'update', id, patch: { velocity: [0, 7, 0] } });
        return w.snapshot.entities.find(e => e.id === id).velocity[1];
    }, id)).toBe(7);
});

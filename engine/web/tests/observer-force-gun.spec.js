/* global window, document, Event */
import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function openGunWorld(page, mirrored = false) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    const id = await page.evaluate(async mirrored => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale', .35); w.setSetting('autoQuality', false); w.setSetting('feedbackEnabled', false);
        await w.command({ type: 'pause' }); await w.command({ type: 'profile', profile: 'playground' });
        await w.command({ type: 'world-physics', patch: { gravityStrength: 0 } });
        const id = w.snapshot.entities[0].id;
        for (const entity of w.snapshot.entities.slice(1)) await w.command({ type: 'delete', id: entity.id });
        await w.command({ type: 'update', id, patch: { position: [0, 2, -4], size: [2, 2, 2], velocity: [0, 0, 0], angularVelocity: [0, 0, 0], damping: 0, mass: 2, bodyType: 'dynamic' } });
        await w.command({ type: 'observer', patch: { position: [0, mirrored ? -2 : 2, 8], yaw: 0, pitch: 0 } });
        w.input.setPose({ yaw: 0, pitch: 0 });
        const owner = window.__FTD_DEV__.registry.get('observerHost').deps.readLattice().owner;
        window.__gunLatticeOwner = owner; window.__gunLatticeTick = owner.currentTick();
        return id;
    }, mirrored);
    await page.waitForFunction(({ id, mirrored }) => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.hit?.entityId === id && w.hit.mirrored === mirrored;
    }, { id, mirrored });
    return id;
}
async function capture(page) {
    await page.locator('.observer-canvas').click({ position: { x: 250, y: 250 } });
    await expect.poll(() => page.evaluate(() => document.pointerLockElement === window.__FTD_DEV__.registry.get('observerWorkspace').renderer.canvas)).toBe(true);
}
async function held(page, mode) {
    await page.waitForFunction(mode => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.forceGun.held && !w.forceGun.held.starting && w.client.latestForceGun?.mode === mode;
    }, mode);
}
async function released(page) {
    await page.waitForFunction(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.forceGun.held === null && w.client.latestForceGun === null;
    });
    await expect(page.locator('.observer-force-tether')).toBeHidden();
}

test('captured left hold elastically pulls, wheel changes depth, release throws, and E still inspects', async ({ page }, testInfo) => {
    const id = await openGunWorld(page); await capture(page);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').forceGun.held)).toBeNull();
    const before = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { camera: w.snapshot.observer.position, depth: w.hit.distance };
    });
    await page.mouse.down(); await held(page, 'pull');
    await page.mouse.wheel(0, -350);
    await page.waitForFunction(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.snapshot.entities.find(e => e.id === id).position[2] > -3.95 && w.client.latestForceGun.forceMagnitude > 0;
    }, id);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position)).toEqual(before.camera);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').forceGun.state.depth)).toBeLessThan(before.depth);
    await expect(page.locator('.observer-force-tether')).toBeVisible();
    await expect(page.locator('[data-observer-force-gun]')).toContainText(/pull/i);
    await page.screenshot({ path: testInfo.outputPath('force-gun-tether.png') });
    await page.mouse.up(); await released(page);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing)).toBe(true);
    await page.mouse.wheel(0, -100);
    await page.waitForFunction(z => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2] < z, before.camera[2]);
    await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-panel="objects"]')).toBeVisible();
    expect(await page.evaluate(() => {
        const owner = window.__FTD_DEV__.registry.get('observerHost').deps.readLattice().owner;
        return owner === window.__gunLatticeOwner && owner.currentTick() === window.__gunLatticeTick;
    })).toBe(true);
});

test('right hold pushes with a hidden crosshair, applies configured strength, and the off switch disables interaction', async ({ page }) => {
    const id = await openGunWorld(page);
    await page.locator('[data-observer-panel-tab="forcegun"]').click();
    await page.locator('[data-observer-setting="forceGunSensitivity"]').selectOption('strong');
    await page.locator('[data-observer-setting="forceGunMultiplier"]').fill('2');
    await page.locator('[data-observer-setting="forceGunMultiplier"]').dispatchEvent('change');
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await capture(page); await page.keyboard.press('KeyH');
    await expect(page.locator('.observer-reticle')).toBeHidden();
    await page.mouse.down({ button: 'right' }); await held(page, 'push');
    await page.waitForFunction(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.snapshot.entities.find(e => e.id === id).velocity[2] < -.1;
    }, id);
    expect(await page.evaluate(() => {
        const t = window.__FTD_DEV__.registry.get('observerWorkspace').client.latestForceGun;
        return { sensitivity: t.sensitivity, multiplier: t.multiplier, mass: t.mass };
    })).toEqual({ sensitivity: 'strong', multiplier: 2, mass: 2 });
    await expect(page.locator('.observer-force-tether')).toBeVisible();
    await page.mouse.up({ button: 'right' }); await released(page);
    await page.keyboard.press('Escape');
    await page.locator('[data-observer-panel-tab="forcegun"]').click();
    await page.locator('[data-observer-setting="forceGunEnabled"]').uncheck();
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await capture(page); await page.mouse.down();
    await expect(page.locator('[data-observer-status]')).toContainText('Force gun is disabled');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').forceGun.held)).toBeNull();
    await page.mouse.up();
});

test('tether color follows measured effort, warns at excess demand and recovers when strength increases', async ({ page }, testInfo) => {
    const id = await openGunWorld(page);
    await page.evaluate(async id => {
        await window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'update', id, patch: { mass: 1000 } });
    }, id);
    await page.waitForFunction(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.hit?.entityId === id && w.hit.revision === w.snapshot.entities.find(e => e.id === id).revision;
    }, id);
    await capture(page); await page.mouse.down(); await held(page, 'pull');
    const tether = page.locator('.observer-force-tether');
    const line = tether.locator('path');
    await expect(tether).toHaveAttribute('data-effort', '0');
    await expect(line).toHaveAttribute('stroke', '#6de4ff');
    await expect(page.locator('[data-observer-force-gun]')).toContainText('effort 0%');
    await page.mouse.wheel(0, -100);
    await page.waitForFunction(() => {
        const effort = window.__FTD_DEV__.registry.get('observerWorkspace').forceGun.state?.effort;
        return effort && effort.percent > 5 && effort.percent < 90 && !effort.overloaded;
    });
    await expect(line).not.toHaveAttribute('stroke', '#6de4ff');
    await page.mouse.wheel(0, -1500);
    await expect(tether).toHaveAttribute('data-overloaded', 'true');
    await expect(tether).toHaveAttribute('data-effort', '100');
    await expect(line).toHaveAttribute('stroke', '#f178ff');
    await expect(line).toHaveAttribute('stroke-dasharray', '3 4');
    await expect(page.locator('[data-observer-force-gun]')).toContainText('LIMIT EXCEEDED');
    await page.screenshot({ path: testInfo.outputPath('force-gun-overload.png') });
    // Raising strength increases available capacity without breaking the held tether.
    await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('forceGunSensitivity', 'strong'); w.setSetting('forceGunMultiplier', 10);
    });
    await expect(tether).toHaveAttribute('data-overloaded', 'false');
    await expect(line).not.toHaveAttribute('stroke', '#f178ff');
    await expect(line).toHaveAttribute('stroke-dasharray', 'none');
    await expect(page.locator('[data-observer-force-gun]')).not.toContainText('LIMIT EXCEEDED');
    await page.mouse.up(); await released(page);
});

test('mirrored dragging follows the displayed direction and blur releases the linked source', async ({ page }) => {
    const id = await openGunWorld(page, true); await capture(page);
    await page.mouse.down(); await held(page, 'pull');
    await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.input.setPose({ yaw: 0, pitch: .15 });
    });
    await page.waitForFunction(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return w.snapshot.entities.find(e => e.id === id).velocity[1] < -.1;
    }, id);
    const state = await page.evaluate(id => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { count: w.snapshot.entities.filter(e => e.id === id).length,
            targetY: w.client.latestForceGun.target[1], displayedY: w.forceGun.anchor[1] };
    }, id);
    expect(state.count).toBe(1); expect(state.targetY).toBeLessThan(2); expect(state.displayedY).toBeGreaterThan(-2);
    await page.evaluate(() => window.dispatchEvent(new Event('blur'))); await released(page);
    await page.mouse.up();
});

test('late begin replies after release cannot revive the force or release a newer hold', async ({ page }) => {
    await openGunWorld(page); await capture(page);
    await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        const original = w.client.request.bind(w.client); let delay = true;
        w.client.request = async command => {
            const result = await original(command);
            if (command.type === 'gun-begin' && delay) {
                delay = false;
                await new Promise(resolve => { window.__releaseGunBegin = resolve; });
            }
            return result;
        };
    });
    await page.mouse.down();
    await page.waitForFunction(() => typeof window.__releaseGunBegin === 'function');
    const oldToken = await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').forceGun.held.token);
    await page.mouse.up(); await released(page);
    await page.mouse.down(); await held(page, 'pull');
    const newToken = await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').forceGun.held.token);
    expect(newToken).not.toBe(oldToken);
    await page.evaluate(() => window.__releaseGunBegin());
    await page.waitForFunction(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace'); return w.client.pending.size <= 1;
    });
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').client.latestForceGun.token)).toBe(newToken);
    await page.mouse.up(); await released(page);
});

test('pause, author panels, deletion, epoch change and workspace exit cancel held forces', async ({ page }) => {
    const id = await openGunWorld(page); await capture(page);
    await page.mouse.down(); await held(page, 'pull');
    await page.evaluate(async () => { await window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'pause' }); });
    await released(page); await page.mouse.up();
    await page.mouse.down(); await held(page, 'pull'); await page.keyboard.press('KeyE');
    await expect(page.locator('[data-observer-panel="objects"]')).toBeVisible(); await released(page); await page.mouse.up();
    await page.getByRole('button', { name: 'Close controls', exact: true }).click(); await capture(page);
    await page.mouse.down(); await held(page, 'pull');
    await page.evaluate(async id => { await window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'delete', id }); }, id);
    await released(page); await page.mouse.up();
    await page.evaluate(async id => { await window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'restore', id }); }, id);
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').hit?.entityId === id, id);
    await page.mouse.down(); await held(page, 'pull');
    await page.evaluate(async () => { await window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'profile', profile: 'sr' }); });
    await released(page); await page.mouse.up();
    await page.mouse.down();
    await expect(page.locator('[data-observer-status]')).toContainText('Force gun requires Playground');
    await page.mouse.up();
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.profile)).toBe('sr');
    await page.evaluate(async () => { await window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'profile', profile: 'playground' }); });
    await page.waitForFunction(id => window.__FTD_DEV__.registry.get('observerWorkspace').hit?.entityId === id, id);
    await page.mouse.down(); await held(page, 'pull');
    await page.evaluate(async () => { await window.__FTD_DEV__.registry.get('observerHost').exit(); });
    await released(page); await page.mouse.up();
});

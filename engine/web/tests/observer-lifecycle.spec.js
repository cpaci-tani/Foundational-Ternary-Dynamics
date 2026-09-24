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

test('pointer-lock denial reports the failure and leaves author controls reachable', async ({ page }) => {
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await openPausedObserver(page);
    await page.evaluate(() => {
        const canvas = window.__FTD_DEV__.registry.get('observerWorkspace').renderer.canvas;
        window.__observerDeniedCaptures = 0;
        // Fault injection at the browser API boundary exercises both real application attempts.
        Object.defineProperty(canvas, 'requestPointerLock', {
            configurable: true,
            value: () => {
                window.__observerDeniedCaptures++;
                return Promise.reject(new DOMException('Test: mouse capture denied', 'NotAllowedError'));
            },
        });
    });
    await page.locator('.observer-canvas').click({ position: { x: 300, y: 260 } });
    await expect.poll(() => page.evaluate(() => window.__observerDeniedCaptures)).toBe(2);
    await expect(page.locator('[data-observer-status]')).toContainText('mouse capture denied');
    expect(await page.evaluate(() => document.pointerLockElement === null)).toBe(true);
    await page.getByRole('button', { name: 'Camera', exact: true }).click();
    await expect(page.locator('[data-observer-panel="camera"]')).toBeVisible();
    await page.getByLabel('Field of view · degrees', { exact: true }).fill('84');
    await page.getByLabel('Field of view · degrees', { exact: true }).press('Tab');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').settings.fov)).toBe(84);
    expect(errors).toEqual([]);
});

test('actual pointer-lock loss and a delivered window blur clear held movement keys', async ({ page }) => {
    await openPausedObserver(page);
    await page.locator('.observer-canvas').click({ position: { x: 300, y: 260 } });
    await expect.poll(() => page.evaluate(() => document.pointerLockElement === window.__FTD_DEV__.registry.get('observerWorkspace').renderer.canvas)).toBe(true);
    await page.keyboard.down('KeyW');
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').input.sample().move[2])).toBe(1);
    await page.evaluate(() => document.exitPointerLock());
    await expect.poll(() => page.evaluate(() => {
        const input = window.__FTD_DEV__.registry.get('observerWorkspace').input;
        return { locked: !!document.pointerLockElement, keys: input.keys.size, move: input.sample().move };
    })).toEqual({ locked: false, keys: 0, move: [0, 0, 0] });
    await page.keyboard.up('KeyW');
    await page.locator('.observer-canvas').focus();
    await page.keyboard.down('KeyD');
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').input.keys.size)).toBe(1);
    // Deliver the same browser event that an OS focus loss emits, without switching test windows.
    await page.evaluate(() => window.dispatchEvent(new Event('blur')));
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').input.sample().move)).toEqual([0, 0, 0]);
    await page.keyboard.up('KeyD');
});

test('editable fields suppress navigation and rebound keys replace the old binding', async ({ page }) => {
    await openPausedObserver(page);
    await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        await workspace.action('select', { id: workspace.snapshot.entities.find(entity => entity.alive).id });
        workspace.updateUI();
        workspace.ui.openPanel('objects');
        window.__observerReticleBeforeTyping = workspace.settings.reticle;
    });
    await page.getByLabel('Name', { exact: true }).fill('');
    await page.getByLabel('Name', { exact: true }).pressSequentially('hwasd');
    await expect(page.getByLabel('Name', { exact: true })).toHaveValue('hwasd');
    expect(await page.evaluate(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { sameReticle: workspace.settings.reticle === window.__observerReticleBeforeTyping,
            keys: workspace.input.keys.size, move: workspace.input.sample().move };
    })).toEqual({ sameReticle: true, keys: 0, move: [0, 0, 0] });
    await page.getByRole('button', { name: 'Controls', exact: true }).click();
    await page.locator('[data-observer-binding="forward"]').press('KeyI');
    await expect(page.locator('[data-observer-binding="forward"]')).toHaveValue('KeyI');
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await page.locator('.observer-canvas').focus();
    await page.keyboard.down('KeyW');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').input.sample().move)).toEqual([0, 0, 0]);
    await page.keyboard.up('KeyW');
    await page.keyboard.down('KeyI');
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').input.sample().move[2])).toBe(1);
    await page.keyboard.up('KeyI');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').input.sample().move)).toEqual([0, 0, 0]);
});

test('axis locks constrain the authoritative observer position while other movement remains available', async ({ page }) => {
    await openPausedObserver(page);
    await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        await workspace.command({ type: 'observer', patch: { position: [0, 1.6, 8], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 } });
        workspace.input.setPose({ yaw: 0, pitch: 0, roll: 0 });
    });
    await page.getByRole('button', { name: 'Camera', exact: true }).click();
    await page.getByLabel('Lock X movement', { exact: true }).check();
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await page.locator('.observer-canvas').focus();
    await page.keyboard.down('KeyD');
    await page.keyboard.down('KeyW');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2] < 7.98);
    await page.keyboard.up('KeyD');
    await page.keyboard.up('KeyW');
    const state = await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const ack = await workspace.command({ type: 'pause' });
        return { position: ack.snapshot.observer.position, locks: workspace.settings.axisLocks,
            playing: ack.snapshot.playing, keys: workspace.input.keys.size };
    });
    expect(state.position[0]).toBeCloseTo(0, 12);
    expect(state.position[2]).toBeLessThan(7.98);
    expect(state.locks).toEqual([true, false, false]);
    expect(state.playing).toBe(false);
    expect(state.keys).toBe(0);
});

test('real WebGL context loss retains the world and restores a working rendering context', async ({ page }) => {
    test.setTimeout(90_000);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await openPausedObserver(page);
    const supported = await page.evaluate(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        window.__observerLossExtension = workspace.renderer.renderer.getContext().getExtension('WEBGL_lose_context');
        window.__observerStateBeforeLoss = { sessionId: workspace.snapshot.sessionId, epoch: workspace.snapshot.epoch,
            tick: workspace.snapshot.tick, time: workspace.snapshot.time,
            entities: structuredClone(workspace.snapshot.entities), segments: structuredClone(workspace.snapshot.segments) };
        return !!window.__observerLossExtension;
    });
    expect(supported, 'The Chromium acceptance target must support controlled WebGL context loss.').toBe(true);
    await page.evaluate(() => window.__observerLossExtension.loseContext());
    await page.waitForFunction(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        return workspace.contextLost && workspace.renderer.contextLost && !workspace.snapshot.playing;
    });
    expect(await page.evaluate(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const current = { sessionId: workspace.snapshot.sessionId, epoch: workspace.snapshot.epoch,
            tick: workspace.snapshot.tick, time: workspace.snapshot.time,
            entities: workspace.snapshot.entities, segments: workspace.snapshot.segments };
        return JSON.stringify(current) === JSON.stringify(window.__observerStateBeforeLoss);
    })).toBe(true);
    await page.evaluate(() => window.__observerLossExtension.restoreContext());
    await page.waitForFunction(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        return !workspace.contextLost && !workspace.renderer.contextLost && !workspace.renderer.renderer.getContext().isContextLost();
    });
    await expect(page.locator('[data-observer-status]')).toContainText('Graphics restored');
    const rendered = await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        await new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)));
        const hit = workspace.renderer.readPixelHit(workspace.snapshot, workspace.settings, 0, 0);
        const gl = workspace.renderer.renderer.getContext();
        return { error: gl.getError(), noError: gl.NO_ERROR, retained: workspace.snapshot.sessionId === window.__observerStateBeforeLoss.sessionId,
            paused: !workspace.snapshot.playing, hitFinite: hit === null || Number.isFinite(hit.emissionTime) };
    });
    expect(rendered.error).toBe(rendered.noError);
    expect(rendered.retained).toBe(true);
    expect(rendered.paused).toBe(true);
    expect(rendered.hitFinite).toBe(true);
    await page.getByRole('button', { name: 'Play', exact: true }).click();
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.tick > window.__observerStateBeforeLoss.tick);
    expect(errors).toEqual([]);
});

test('disposing an active workspace rejects in-flight work and releases input and presentation ownership', async ({ page }) => {
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await openPausedObserver(page);
    await page.locator('.observer-canvas').focus();
    await page.keyboard.down('KeyW');
    const result = await page.evaluate(async () => {
        const registry = window.__FTD_DEV__.registry;
        const host = registry.get('observerHost');
        const workspace = registry.get('observerWorkspace');
        const nativeOwner = workspace.lattice.read().owner;
        const pending = workspace.client.send({ type: 'advance', dt: 0.1, input: {},
            sessionId: workspace.snapshot.sessionId, epoch: workspace.snapshot.epoch })
            .then(() => ({ resolved: true, message: '' }), error => ({ resolved: false, message: error.message }));
        host.dispose();
        const settlement = await pending;
        await new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)));
        return { settlement, registered: !!registry.get('observerWorkspace'),
            clientDisposed: workspace.client.disposed, pendingCount: workspace.client.pending.size,
            latestCleared: workspace.client.latest === null, inputEnabled: workspace.input.enabled,
            keys: workspace.input.keys.size, rendererDisposed: workspace.renderer.disposed,
            controlsRestored: window.__FTD_DEV__.viewport.controls.enabled,
            appInert: document.querySelector('#app').inert,
            sameNativeOwner: workspace.lattice.read().owner === nativeOwner,
            workspaceAttached: workspace.element.isConnected };
    });
    await page.keyboard.up('KeyW');
    expect(result.settlement.resolved).toBe(false);
    expect(result.settlement.message).toMatch(/disposed/);
    expect(result).toMatchObject({ registered: false, clientDisposed: true, pendingCount: 0, latestCleared: true,
        inputEnabled: false, keys: 0, rendererDisposed: true, controlsRestored: true,
        appInert: false, sameNativeOwner: true, workspaceAttached: false });
    expect(errors).toEqual([]);
});

test('hidden-page handling pauses the retained session and resumes without charging hidden wall time', async ({ page, context }, testInfo) => {
    await openPausedObserver(page);
    await page.getByRole('button', { name: 'Play', exact: true }).click();
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing);
    const cover = await context.newPage();
    await cover.goto('about:blank');
    await cover.bringToFront();
    const realVisibility = await page.evaluate(() => document.hidden);
    testInfo.annotations.push({ type: 'visibility-path', description: realVisibility
        ? 'A second foreground page produced actual document.hidden.'
        : 'Headless Chromium retained visible pages; the test controls document.hidden and delivers visibilitychange to the real listeners.' });
    if (!realVisibility) {
        await page.evaluate(() => {
            window.__observerVisibilityOverride = true;
            Object.defineProperty(document, 'hidden', { configurable: true, get: () => window.__observerVisibilityOverride });
            document.dispatchEvent(new Event('visibilitychange'));
        });
    }
    try {
        await page.waitForFunction(() => !window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing);
        const frozen = await page.evaluate(() => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return { time: workspace.snapshot.time, tick: workspace.snapshot.tick, epoch: workspace.snapshot.epoch,
                properTime: workspace.snapshot.observer.properTime, keys: workspace.input.keys.size };
        });
        // A real elapsed interval is necessary to distinguish a paused owner from a slow renderer.
        await page.waitForTimeout(650);
        expect(await page.evaluate(() => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return { time: workspace.snapshot.time, tick: workspace.snapshot.tick, epoch: workspace.snapshot.epoch,
                properTime: workspace.snapshot.observer.properTime, keys: workspace.input.keys.size };
        })).toEqual(frozen);
        expect(frozen.keys).toBe(0);
        if (!realVisibility) {
            await page.evaluate(() => {
                window.__observerVisibilityOverride = false;
                delete document.hidden;
                document.dispatchEvent(new Event('visibilitychange'));
            });
        }
        await page.bringToFront();
        await cover.close();
        await page.waitForFunction(before => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return workspace.snapshot.playing && workspace.snapshot.time > before;
        }, frozen.time, { polling: 'raf' });
        const resumed = await page.evaluate(() => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return { time: workspace.snapshot.time, epoch: workspace.snapshot.epoch, queued: workspace.client.queuedSeconds };
        });
        expect(resumed.epoch).toBe(frozen.epoch);
        expect(resumed.time - frozen.time).toBeLessThan(0.3);
        expect(resumed.queued).toBeLessThan(0.3);
    } finally {
        if (!realVisibility) await page.evaluate(() => { delete document.hidden; document.dispatchEvent(new Event('visibilitychange')); });
        if (!cover.isClosed()) await cover.close();
    }
});

test('an aborted panorama reports a fallback and a superseded download cannot replace the chosen environment', async ({ page }) => {
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    let releaseDownload;
    let requestStarted = false;
    const released = new Promise(resolve => { releaseDownload = resolve; });
    await page.route('**/assets/observer/environments/*.hdr', async route => {
        if (route.request().url().includes('machine_shop_02_1k.hdr')) {
            requestStarted = true;
            await released;
            // Deliver malformed late bytes if the browser still accepts them. A cancelled request
            // may already be closed; either transport outcome must leave the new generation alone.
            await route.fulfill({ status: 200, contentType: 'application/octet-stream', body: 'late superseded panorama bytes' }).catch(() => {});
        } else await route.abort('connectionfailed');
    });
    await openPausedObserver(page);
    const before = await page.evaluate(() => {
        const snapshot = window.__FTD_DEV__.registry.get('observerWorkspace').snapshot;
        return { sessionId: snapshot.sessionId, epoch: snapshot.epoch, entities: snapshot.entities };
    });
    await page.getByRole('button', { name: 'World', exact: true }).click();
    await page.locator('#observer-workspace').getByLabel('Environment', { exact: true }).selectOption('studio');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').renderer.diagnostics.environmentStatus === 'fallback');
    await expect(page.locator('[data-observer-status]')).toContainText('Panorama unavailable');
    try {
        await page.locator('#observer-workspace').getByLabel('Environment', { exact: true }).selectOption('workshop');
        await expect.poll(() => requestStarted, { timeout: 10000,
            message: 'The local workshop HDR request must reach the network route before testing cancellation.' }).toBe(true);
        await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').renderer.environment.status === 'loading');
        await page.locator('#observer-workspace').getByLabel('Environment', { exact: true }).selectOption('stars');
        await page.waitForFunction(() => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return workspace.snapshot.environment.preset === 'stars' && workspace.renderer.environment.preset === 'stars'
                && workspace.renderer.environment.status === 'ready';
        });
        releaseDownload();
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
        const after = await page.evaluate(() => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return { sessionId: workspace.snapshot.sessionId, epoch: workspace.snapshot.epoch,
                entities: workspace.snapshot.entities, preset: workspace.renderer.environment.preset,
                status: workspace.renderer.environment.status, texture: !!workspace.renderer.environment.texture,
                pendingAborted: workspace.renderer.environment.abort?.signal.aborted };
        });
        expect(after).toMatchObject({ sessionId: before.sessionId, epoch: before.epoch,
            entities: before.entities, preset: 'stars', status: 'ready', texture: false, pendingAborted: true });
        expect(errors).toEqual([]);
    } finally { releaseDownload(); }
});

test('all six bundled panoramas decode into real textures under the normal static-server CSP', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    const errors = [];
    const requests = [];
    page.on('pageerror', error => errors.push(error.message));
    page.on('request', request => { if (/\.hdr(?:\?|$)/.test(request.url())) requests.push(request.url()); });
    await openPausedObserver(page);
    await page.getByRole('button', { name: 'World', exact: true }).click();
    const evidence = [];
    for (const preset of ['studio', 'workshop', 'sunset', 'night', 'forest', 'urban']) {
        await page.locator('#observer-workspace').getByLabel('Environment', { exact: true }).selectOption(preset);
        await page.waitForFunction(expected => {
            const environment = window.__FTD_DEV__.registry.get('observerWorkspace').renderer.environment;
            return environment.preset === expected && environment.status === 'ready'
                && environment.texture?.image.width > 0 && environment.texture?.image.height > 0;
        }, preset, { timeout: 15000 });
        evidence.push(await page.evaluate(() => {
            const environment = window.__FTD_DEV__.registry.get('observerWorkspace').renderer.environment;
            return { preset: environment.preset, width: environment.texture.image.width,
                height: environment.texture.image.height, status: environment.status,
                pixelValues: environment.texture.image.data.length };
        }));
    }
    expect(evidence).toHaveLength(6);
    for (const result of evidence) {
        expect(result.width, result.preset).toBeGreaterThan(0);
        expect(result.height, result.preset).toBeGreaterThan(0);
        expect(result.pixelValues, result.preset).toBe(result.width * result.height * 4);
        expect(result.status, result.preset).toBe('ready');
    }
    expect(requests.length).toBeGreaterThanOrEqual(6);
    for (const url of requests) {
        expect(new URL(url).origin).toBe(new URL(page.url()).origin);
        expect(new URL(url).pathname).toContain('/assets/observer/environments/');
    }
    expect(errors).toEqual([]);
    await testInfo.attach('bundled-panorama-decode-evidence.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
});

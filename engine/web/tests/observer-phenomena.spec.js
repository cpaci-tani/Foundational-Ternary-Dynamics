/* global window, document, structuredClone, Buffer */
import { test, expect } from '@playwright/test';
import { writeFile } from 'node:fs/promises';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

const LAYER_LABELS = [
    ['interference', 'Two-source interference'], ['standingWaves', 'Standing waves'],
    ['polarization', 'Electromagnetic polarization'], ['lightCones', 'Spacetime light cones'],
    ['simultaneity', 'Relativity of simultaneity'], ['lightPaths', 'Received-light events'],
    ['aberration', 'Aberration sky compass'], ['ghosts', 'Simultaneous ghosts'],
];

async function openObserver(page) {
    await page.setViewportSize({ width: 1920, height: 1080 });
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        workspace.setSetting('renderScale', 0.25);
        workspace.setSetting('autoQuality', false);
        workspace.setSetting('feedbackEnabled', false);
        await workspace.command({ type: 'pause' });
    });
}

test('received-event readouts wait for a matching rendered view after same-time projection and camera edits', async ({ page }) => {
    await openObserver(page);
    await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        const snapshot = structuredClone(w.snapshot);
        snapshot.observer = { ...snapshot.observer, position: [0, 2, 6], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 };
        const entity = { ...snapshot.entities[0], id: 'received-test', name: 'Received test', position: [0, 2, -6], size: [2, 2, 2], velocity: [0, 0, 0], shape: 'sphere', originTime: 0 };
        snapshot.entities = [entity]; snapshot.segments = [{ ...entity, entityId: entity.id, start: -100, end: null }];
        await w.command({ type: 'load', snapshot });
        w.setSetting('mirrorWorld', false); w.setSetting('layers', { lightPaths: true });
    });
    const received = page.locator('[data-observer-received-event]');
    await expect(received).toContainText('received-test · revision');
    const afterFoV = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('fov', 75);
        return document.querySelector('[data-observer-received-event]').textContent;
    });
    expect(afterFoV).toContain('Aim the crosshair');
    await expect(received).toContainText('received-test · revision');
    const afterLook = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.input.yaw = 0.2; w.updateUI();
        return document.querySelector('[data-observer-received-event]').textContent;
    });
    expect(afterLook).toContain('Aim the crosshair');
});

test('Phenomena controls expose instruments without editing the paused session or retained lattice', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    await openObserver(page);
    await page.getByRole('button', { name: 'Phenomena', exact: true }).click();
    // Wait for the panel's default pause command before capturing the baseline.
    await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'pause' }));
    const panel = page.locator('[data-observer-panel]');
    await expect(panel).toContainText('Coordinate-frame analytic reference illustrations');
    await expect(panel.getByLabel('Wavelength · world units', { exact: true })).toHaveValue('4');
    await expect(panel.getByLabel('Polarization mode', { exact: true })).toHaveValue('circular');
    await page.evaluate(async () => {
        const { getActiveScale0Bridge } = await import('/js/scales/scale0/state/store.js');
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const lattice = getActiveScale0Bridge(window.__ftdCtx);
        window.__observerPhenomenaOwners = { lattice, latticeWorker: lattice._worker,
            client: workspace.client, sessionWorker: workspace.client.worker, getActiveScale0Bridge };
    });
    await page.waitForFunction(() => {
        const lattice = window.__observerPhenomenaOwners.lattice;
        return window.__ftdCtx.running === false && lattice.runningStateSettled && lattice._runningAck === false;
    });
    const before = await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const lattice = window.__observerPhenomenaOwners.lattice;
        return { snapshot: JSON.stringify(workspace.snapshot), digest: await lattice.captureDynamicalStateDigest(),
            sourceEpoch: lattice.getScale0TelemetryGroupMeta('diagnostics')?.sourceEpoch, tick: lattice.currentTick() };
    });
    expect(before.digest, 'require a fresh microscopic-state digest').not.toBeNull();
    for (const [, label] of LAYER_LABELS) await panel.getByLabel(label, { exact: true }).check();
    for (const [, label] of LAYER_LABELS) await expect(panel.getByLabel(label, { exact: true })).toBeChecked();
    const instrument = page.locator('[data-observer-phenomena]');
    await expect(instrument).toBeVisible();
    await expect(instrument).toContainText('Adopted SR and analytic wave illustrations');
    await expect(page.locator('[data-observer-spacetime]')).toBeVisible();
    await expect(page.locator('[data-observer-aberration]')).toBeVisible();
    await expect(page.locator('.observer-geometry-overlay')).toBeVisible();
    await page.screenshot({ path: testInfo.outputPath('phenomena-controls-1920.png') });
    const after = await page.evaluate(async () => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const original = window.__observerPhenomenaOwners;
        const lattice = original.getActiveScale0Bridge(window.__ftdCtx);
        return { snapshot: JSON.stringify(workspace.snapshot), digest: await lattice.captureDynamicalStateDigest(),
            sourceEpoch: lattice.getScale0TelemetryGroupMeta('diagnostics')?.sourceEpoch, tick: lattice.currentTick(),
            sameLattice: lattice === original.lattice, sameLatticeWorker: lattice._worker === original.latticeWorker,
            sameClient: workspace.client === original.client, sameSessionWorker: workspace.client.worker === original.sessionWorker };
    });
    expect(after.snapshot).toBe(before.snapshot);
    expect(after.digest).toEqual(before.digest);
    expect(after.sourceEpoch).toBe(before.sourceEpoch);
    expect(after.tick).toBe(before.tick);
    for (const key of ['sameLattice', 'sameLatticeWorker', 'sameClient', 'sameSessionWorker']) expect(after[key], key).toBe(true);
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await expect(panel).toBeHidden();
    await page.screenshot({ path: testInfo.outputPath('phenomena-world-1920.png') });
});

test('Playground explicitly disables SR instruments while keeping analytic waves available', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    await openObserver(page);
    await page.getByRole('button', { name: 'Phenomena', exact: true }).click();
    const panel = page.locator('[data-observer-panel]');
    for (const label of ['Spacetime light cones', 'Aberration sky compass', 'Two-source interference']) await panel.getByLabel(label, { exact: true }).check();
    await page.getByRole('button', { name: 'World', exact: true }).click();
    await panel.getByLabel('Physics profile', { exact: true }).selectOption('playground');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.profile === 'playground');
    await page.getByRole('button', { name: 'Phenomena', exact: true }).click();
    await expect(panel).toContainText('Relativistic instruments are unavailable in Playground');
    for (const label of ['Spacetime light cones', 'Relativity of simultaneity', 'Received-light events', 'Aberration sky compass', 'Simultaneous ghosts']) {
        await expect(panel.getByLabel(label, { exact: true })).toBeDisabled();
    }
    for (const label of ['Two-source interference', 'Standing waves', 'Electromagnetic polarization', 'Spherical pulse fronts']) {
        await expect(panel.getByLabel(label, { exact: true })).toBeEnabled();
    }
    await expect(page.locator('[data-observer-frame-readout]')).toContainText('unavailable in Playground');
    await expect(page.locator('[data-observer-spacetime]')).toBeHidden();
    await expect(page.locator('[data-observer-aberration]')).toBeHidden();
    await page.screenshot({ path: testInfo.outputPath('phenomena-playground-availability.png') });
});

test('wave pixels evolve deterministically, pulse fronts require recorded emissions, and layers preserve shader targeting', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    page.on('console', message => { if (message.type() === 'error' && /shader|WebGL|Observer/i.test(message.text())) errors.push(message.text()); });
    await openObserver(page);
    const evidence = await page.evaluate(async layerKeys => {
        const { ObserverRenderer } = await import('/js/observer/renderer.js');
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const client = workspace.client, worker = client.worker;
        const authoritativeBefore = JSON.stringify(workspace.snapshot);
        const container = document.createElement('div');
        container.style.cssText = 'position:fixed;left:-1000px;top:0;width:480px;height:300px';
        document.body.append(container);
        const renderer = new ObserverRenderer({ container });
        const snapshot = structuredClone(workspace.snapshot);
        snapshot.time = 12; snapshot.historyStart = -60; snapshot.profile = 'sr';
        snapshot.entities = []; snapshot.segments = []; snapshot.pulses = []; snapshot.environmentHistory = undefined;
        snapshot.environment = { ...snapshot.environment, preset: 'void', animationRate: 0 };
        snapshot.observer = { ...snapshot.observer, position: [0, 3.5, 12], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 };
        const settings = { ...structuredClone(workspace.settings), layers: {}, selectedId: null, autoQuality: false,
            renderScale: 1, mirrorWorld: false, feedbackEnabled: false, cameraOverride: snapshot.observer };
        const readOverlay = () => {
            renderer.drawOverlays(snapshot, settings);
            const pixels = renderer.overlayContext.getImageData(0, 0, renderer.width, renderer.height).data;
            let hash = 2166136261, colored = 0;
            for (let i = 0; i < pixels.length; i++) {
                hash = Math.imul(hash ^ pixels[i], 16777619);
                if (i % 4 === 3 && pixels[i] > 0) colored++;
            }
            return { hash: hash >>> 0, colored };
        };
        const image = () => {
            const canvas = document.createElement('canvas'); canvas.width = renderer.width; canvas.height = renderer.height;
            const context = canvas.getContext('2d'); context.fillStyle = '#07111d'; context.fillRect(0, 0, canvas.width, canvas.height);
            context.drawImage(renderer.overlayCanvas, 0, 0);
            return canvas.toDataURL('image/png').split(',')[1];
        };
        try {
            renderer.resize(480, 300);
            const empty = readOverlay(), waves = [];
            for (const layer of ['interference', 'standingWaves', 'polarization']) {
                settings.layers = { [layer]: true }; snapshot.time = 12;
                // Chromium may migrate a Canvas2D context from GPU to CPU after
                // repeated readbacks. Warm that raster path before exact pixel
                // comparisons; do not relax deterministic paused-frame equality.
                const warmup = [readOverlay(), readOverlay(), readOverlay()];
                const first = readOverlay(), repeated = readOverlay();
                snapshot.time = 12.375; const later = readOverlay();
                waves.push({ layer, warmup, first, repeated, later, image: image() });
            }
            snapshot.time = 12;
            const beacon = { ...workspace.snapshot.entities[0], id: 'recorded-emission-source', revision: 7,
                shape: 'beacon', position: [0, 3.5, -6], velocity: [0, 0, 0], originTime: 0,
                size: [2, 2, 2], rotation: [0, 0, 0], alive: true };
            snapshot.entities = [beacon]; snapshot.segments = [{ ...beacon, entityId: beacon.id, start: -100, end: null }];
            settings.layers = { pulses: true };
            const noRecordedPulse = readOverlay();
            snapshot.pulses = [{ origin: [...beacon.position], start: 8, color: [1, 0.8, 0.4] }];
            const recordedPulse = readOverlay(), pulseImage = image();
            snapshot.pulses = []; settings.layers = {};
            const syntheticBefore = JSON.stringify(snapshot);
            renderer.render(snapshot, settings, 0);
            const before = { cpu: renderer.pick(snapshot, settings, 0, 0), gpu: renderer.readPixelHit(snapshot, settings, 0, 0) };
            settings.layers = Object.fromEntries(layerKeys.map(key => [key, true]));
            renderer.render(snapshot, settings, 0);
            const after = { cpu: renderer.pick(snapshot, settings, 0, 0), gpu: renderer.readPixelHit(snapshot, settings, 0, 0) };
            return { empty, waves, noRecordedPulse, recordedPulse, pulseImage, before, after,
                layerCount: renderer.diagnostics.layerCount, gpu: renderer.diagnostics.gpu,
                glError: renderer.renderer.getContext().getError(), syntheticUnchanged: JSON.stringify(snapshot) === syntheticBefore,
                authoritativeUnchanged: JSON.stringify(workspace.snapshot) === authoritativeBefore,
                clientUnchanged: workspace.client === client && client.worker === worker };
        } finally { renderer.dispose(); container.remove(); }
    }, LAYER_LABELS.map(([id]) => id));
    for (const entry of [...evidence.waves, { layer: 'recorded-pulse', image: evidence.pulseImage }]) {
        const path = testInfo.outputPath(`${entry.layer}-overlay.png`);
        await writeFile(path, Buffer.from(entry.image, 'base64'));
        await testInfo.attach(entry.layer, { path, contentType: 'image/png' });
        delete entry.image;
    }
    delete evidence.pulseImage;
    await testInfo.attach('phenomena-functional-evidence.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
    expect(evidence.empty.colored).toBe(0);
    for (const wave of evidence.waves) {
        expect(wave.first.colored, wave.layer).toBeGreaterThan(30);
        expect(wave.repeated, `${wave.layer}: paused`).toEqual(wave.first);
        expect(wave.later.hash, `${wave.layer}: evolution`).not.toBe(wave.first.hash);
    }
    expect(evidence.noRecordedPulse).toEqual(evidence.empty);
    expect(evidence.recordedPulse.colored).toBeGreaterThan(30);
    expect(evidence.before.cpu?.entityId).toBe('recorded-emission-source');
    expect(evidence.before.gpu?.entityId).toBe(evidence.before.cpu.entityId);
    expect(evidence.before.gpu.emissionTime).toBeCloseTo(evidence.before.cpu.emissionTime, 3);
    expect(evidence.after).toEqual(evidence.before);
    expect(evidence.layerCount).toBe(8);
    expect(evidence.syntheticUnchanged).toBe(true);
    expect(evidence.authoritativeUnchanged).toBe(true);
    expect(evidence.clientUnchanged).toBe(true);
    expect(evidence.glError).toBe(0);
    expect(errors).toEqual([]);
});

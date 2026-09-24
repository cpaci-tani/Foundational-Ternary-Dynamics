import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function openObserver(page) {
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

test('default W flight follows upward and downward look, crosses the plane, and optional ground lock fixes height', async ({ page }) => {
    await openObserver(page);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').settings.grounded)).toBe(false);

    async function prepareFlight(pitch, height) {
        await page.evaluate(async ({ pitch, height }) => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            await workspace.command({ type: 'pause' });
            await workspace.command({ type: 'observer', patch: { position: [0, height, 8], velocity: [0, 0, 0], yaw: 0, pitch, roll: 0 } });
            workspace.input.setPose({ yaw: 0, pitch, roll: 0 });
            await workspace.command({ type: 'play' });
        }, { pitch, height });
        await page.locator('.observer-canvas').focus();
    }
    async function stopFlight() {
        await page.keyboard.up('KeyW');
        return page.evaluate(async () => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            const result = await workspace.command({ type: 'pause' });
            return { position: result.snapshot.observer.position, velocity: result.snapshot.observer.velocity,
                properTime: result.snapshot.observer.properTime, keys: workspace.input.keys.size };
        });
    }

    await prepareFlight(Math.PI / 4, 0.04);
    await page.keyboard.down('KeyW');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[1] > 0.10);
    const up = await stopFlight();
    expect(up.position[1]).toBeGreaterThan(0.10);
    expect(up.position[2]).toBeLessThan(8);
    expect(up.properTime).toBeGreaterThan(0);
    expect(Math.hypot(...up.velocity)).toBeLessThanOrEqual(0.99);

    await prepareFlight(-Math.PI / 4, 0.04);
    await page.keyboard.down('KeyW');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[1] < -0.02);
    const down = await stopFlight();
    expect(down.position[1]).toBeLessThan(-0.02);
    expect(down.position[2]).toBeLessThan(8);

    await page.getByRole('button', { name: 'Camera', exact: true }).click();
    await page.getByLabel('Lock movement to ground plane', { exact: true }).check();
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    await prepareFlight(Math.PI / 3, -0.2);
    await page.keyboard.down('KeyW');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2] < 7.94);
    const grounded = await stopFlight();
    expect(grounded.position[1]).toBeCloseTo(-0.2, 12);
    expect(grounded.velocity[1]).toBeCloseTo(0, 12);
    expect(grounded.keys).toBe(0);
});

test('all evolving fractal environments render real, deterministic pixels from simulation time', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    page.on('console', message => {
        if (message.type() === 'error' && /shader|WebGL|Observer/i.test(message.text())) errors.push(message.text());
    });
    await openObserver(page);
    await page.setViewportSize({ width: 1280, height: 800 });
    await page.evaluate(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        workspace.setSetting('mirrorWorld', true);
        workspace.setSetting('feedbackEnabled', true);
        workspace.setSetting('feedbackLayers', 3);
        workspace.setSetting('feedbackDepth', 5);
        workspace.input.setPose({ yaw: 0, pitch: -0.12, roll: 0 });
    });
    const evidence = [];
    for (const preset of ['fractal-julia', 'fractal-menger', 'fractal-kaleidoscope']) {
        await page.getByRole('button', { name: 'World', exact: true }).click();
        await page.locator('#observer-workspace').getByLabel('Environment', { exact: true }).selectOption(preset);
        await page.waitForFunction(expected => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.environment.preset === expected, preset);
        const result = await page.evaluate(() => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            const snapshot = structuredClone(workspace.snapshot);
            snapshot.entities = []; snapshot.segments = []; snapshot.pulses = [];
            snapshot.environmentHistory = undefined;
            snapshot.observer = { ...snapshot.observer, position: [0, 2, 8], velocity: [0, 0, 0], yaw: 0, pitch: 0.55, roll: 0 };
            snapshot.environment.animationRate = 0.35;
            const settings = { ...workspace.settings, autoQuality: false, renderScale: 0.25, layers: {},
                mirrorWorld: false, feedbackEnabled: false, doppler: false, beaming: false,
                cameraOverride: snapshot.observer };
            const renderer = workspace.renderer;
            const read = () => {
                renderer.render(snapshot, settings, 0);
                const gl = renderer.renderer.getContext();
                const pixels = new Uint8Array(gl.drawingBufferWidth * gl.drawingBufferHeight * 4);
                gl.readPixels(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
                let hash = 2166136261, minimum = 255, maximum = 0;
                const colors = new Set();
                for (let i = 0; i < pixels.length; i += 4) {
                    colors.add((pixels[i] << 16) | (pixels[i + 1] << 8) | pixels[i + 2]);
                    for (let component = 0; component < 3; component++) {
                        hash = Math.imul(hash ^ pixels[i + component], 16777619);
                        minimum = Math.min(minimum, pixels[i + component]);
                        maximum = Math.max(maximum, pixels[i + component]);
                    }
                }
                return { hash: hash >>> 0, colors: colors.size, minimum, maximum, error: gl.getError() };
            };
            snapshot.time = 12; const first = read(); const repeated = read();
            snapshot.time = 29; const evolved = read();
            snapshot.environment.animationRate = 0;
            snapshot.time = 12; const frozen = read();
            snapshot.time = 29; const frozenLater = read();
            snapshot.environment.seed += 101; const differentSeed = read();
            return { preset: snapshot.environment.preset, first, repeated, evolved, frozen, frozenLater, differentSeed,
                tracedInstances: renderer.diagnostics.tracedInstances, selectedRate: workspace.snapshot.environment.animationRate };
        });
        expect(result.selectedRate, `${preset} should begin evolving when selected`).toBeGreaterThan(0);
        expect(result.first.colors, preset).toBeGreaterThan(100);
        expect(result.first.maximum - result.first.minimum, preset).toBeGreaterThan(20);
        expect(result.repeated).toEqual(result.first);
        expect(result.evolved.hash, `${preset} must evolve with simulation time`).not.toBe(result.first.hash);
        expect(result.frozenLater).toEqual(result.frozen);
        expect(result.differentSeed.hash, `${preset} must respond deterministically to the seed`).not.toBe(result.frozen.hash);
        expect(result.tracedInstances).toBe(0);
        for (const sample of [result.first, result.repeated, result.evolved, result.frozen, result.frozenLater, result.differentSeed]) expect(sample.error).toBe(0);
        evidence.push(result);
        await page.getByRole('button', { name: 'Close controls', exact: true }).click();
        // Capture visual acceptance at full resolution; pixel comparisons above
        // use a smaller buffer to keep functional tests inexpensive.
        await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').setSetting('renderScale', 1));
        await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
        await page.screenshot({ path: testInfo.outputPath(`minds-eye-${preset}-mirrored-feedback.png`) });
    }
    expect(errors).toEqual([]);
    await testInfo.attach('fractal-pixel-evidence.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
});

test('the world below the plane reflects asymmetric geometry while CPU and GPU retain the source identity', async ({ page }, testInfo) => {
    await openObserver(page);
    const evidence = await page.evaluate(() => {
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const snapshot = structuredClone(workspace.snapshot);
        snapshot.time = 10; snapshot.historyStart = -50; snapshot.environment.preset = 'void'; snapshot.environmentHistory = undefined;
        snapshot.observer = { ...snapshot.observer, position: [0, 2, 6], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 };
        const entity = { ...snapshot.entities[0], id: 'asymmetric-mirror', shape: 'wedge', revision: 7,
            position: [0, 2, 0], velocity: [0.08, 0.04, 0], originTime: 10, size: [2.6, 2.8, 2], rotation: [0.3, 0.2, 0.5], alive: true };
        snapshot.entities = [entity]; snapshot.segments = [{ ...entity, entityId: entity.id, start: -50, end: null }];
        const originalState = JSON.stringify(snapshot);
        const settings = { ...workspace.settings, layers: {}, feedbackEnabled: false, autoQuality: false, renderScale: 0.25,
            mirrorWorld: false, cameraOverride: snapshot.observer };
        const renderer = workspace.renderer, failures = [];
        let mirroredHits = 0, missedRays = 0;
        for (const x of [-0.25, -0.15, 0, 0.15, 0.25]) for (const y of [-0.3, -0.15, 0, 0.15, 0.3]) {
            const sourceSettings = { ...settings, mirrorWorld: false, cameraOverride: snapshot.observer };
            renderer.render(snapshot, sourceSettings);
            const source = renderer.pick(snapshot, sourceSettings, x, y);
            const mirrorSettings = { ...settings, mirrorWorld: true,
                cameraOverride: { ...snapshot.observer, position: [0, -2, 6] } };
            renderer.render(snapshot, mirrorSettings);
            const mirror = renderer.pick(snapshot, mirrorSettings, x, -y);
            const gpu = renderer.readPixelHit(snapshot, mirrorSettings, x, -y);
            if (!source) { missedRays++; if (mirror || gpu) failures.push({ reason: 'Reflection filled an empty asymmetric silhouette', x, y, mirror, gpu }); continue; }
            mirroredHits++;
            if (!mirror || !gpu || !mirror.mirrored || !gpu.mirrored || source.mirrored
                || mirror.entityId !== source.entityId || gpu.entityId !== source.entityId
                || mirror.revision !== 7 || gpu.revision !== 7
                || Math.abs(mirror.distance - source.distance) > 1e-7
                || Math.abs(mirror.emissionTime - gpu.emissionTime) > 0.003
                || Math.abs(mirror.distance - gpu.distance) > 0.003
                || mirror.restPosition.some((value, i) => Math.abs(value - source.restPosition[i]) > 1e-7)
                || Math.abs(mirror.position[1] + source.position[1]) > 1e-7
                || Math.abs(mirror.normal[1] + source.normal[1]) > 1e-7) failures.push({ reason: 'Mirror surface or identity differs', x, y, source, mirror, gpu });
        }
        return { mirroredHits, missedRays, failures: failures.slice(0, 5), unchanged: JSON.stringify(snapshot) === originalState };
    });
    expect(evidence.mirroredHits).toBeGreaterThan(3);
    expect(evidence.missedRays).toBeGreaterThan(3);
    expect(evidence.failures, JSON.stringify(evidence.failures)).toEqual([]);
    expect(evidence.unchanged).toBe(true);
    await testInfo.attach('mirrored-geometry-parity.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
});

test('camera billboards bound recursion, preserve targeting, reuse two targets, and dispose them exactly once', async ({ page }, testInfo) => {
    test.setTimeout(120_000);
    await openObserver(page);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    page.on('console', message => { if (message.type() === 'error' && /shader|WebGL/i.test(message.text())) errors.push(message.text()); });
    const evidence = await page.evaluate(async () => {
        const { ObserverRenderer } = await import('/js/observer/renderer.js');
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const container = document.createElement('div'); container.style.cssText = 'position:fixed;left:-1000px;top:0;width:256px;height:160px';
        document.body.append(container);
        const renderer = new ObserverRenderer({ container });
        const snapshot = structuredClone(workspace.snapshot);
        snapshot.time = 10; snapshot.historyStart = -50; snapshot.environment.preset = 'fractal-kaleidoscope'; snapshot.environmentHistory = undefined;
        snapshot.environment.animationRate = 0.35;
        snapshot.observer = { ...snapshot.observer, position: [0, 2, 6], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 };
        const entity = { ...snapshot.entities[0], id: 'primary-target', shape: 'box', position: [0, 2, 0],
            velocity: [0, 0, 0], originTime: 10, size: [1, 1, 1], rotation: [0, 0, 0], alive: true };
        snapshot.entities = [entity]; snapshot.segments = [{ ...entity, entityId: entity.id, start: -50, end: null }];
        const stateBefore = JSON.stringify(snapshot);
        const settings = { ...workspace.settings, autoQuality: false, renderScale: 1, layers: {}, mirrorWorld: false,
            feedbackEnabled: false, cameraOverride: snapshot.observer };
        const pixelHash = () => {
            const gl = renderer.renderer.getContext(), pixels = new Uint8Array(gl.drawingBufferWidth * gl.drawingBufferHeight * 4);
            gl.readPixels(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
            let hash = 2166136261; for (const value of pixels) hash = Math.imul(hash ^ value, 16777619);
            return hash >>> 0;
        };
        const result = { combinations: [], disposed: [], unchanged: false, detached: false };
        try {
            renderer.render(snapshot, settings);
            const baseline = { hash: pixelHash(), hit: renderer.hit };
            const targets = [...renderer.feedbackTargets];
            for (const configuration of [
                { feedbackLayers: 1, feedbackDepth: 3 }, { feedbackLayers: 2, feedbackDepth: 4 }, { feedbackLayers: 3, feedbackDepth: 5 },
                { feedbackLayers: 999, feedbackDepth: 999, feedbackScale: 999 },
                { feedbackLayers: -99, feedbackDepth: -99, feedbackScale: -99 },
                { feedbackLayers: NaN, feedbackDepth: Infinity, feedbackScale: Infinity },
            ]) {
                const configured = { ...settings, feedbackEnabled: true, feedbackStrength: 0.45, feedbackScale: 0.6, ...configuration };
                renderer.render(snapshot, configured);
                const first = pixelHash(), firstHit = renderer.hit;
                renderer.render(snapshot, configured);
                const repeated = pixelHash();
                const gpu = renderer.readPixelHit(snapshot, configured, 0, 0);
                result.combinations.push({
                    first, repeated, baselineHash: baseline.hash, hit: firstHit?.entityId, baselineHit: baseline.hit?.entityId,
                    gpuHit: gpu?.entityId, emission: firstHit?.emissionTime, baselineEmission: baseline.hit?.emissionTime,
                    layers: renderer.diagnostics.feedbackLayers, depth: renderer.diagnostics.feedbackDepth,
                    passes: renderer.diagnostics.feedbackPasses, resolution: [...renderer.diagnostics.feedbackResolution],
                    sameTargets: renderer.feedbackTargets.length === 2 && renderer.feedbackTargets.every((target, i) => target === targets[i]),
                });
            }
            result.unchanged = JSON.stringify(snapshot) === stateBefore;
            // Listen after resizing: final owner disposal should dispatch once per target.
            const disposals = [0, 0];
            targets.forEach((target, i) => target.addEventListener('dispose', () => { disposals[i]++; }));
            renderer.dispose(); renderer.dispose();
            result.disposed = disposals;
            result.detached = !renderer.canvas.isConnected && !renderer.overlayCanvas.isConnected;
        } finally { renderer.dispose(); container.remove(); }
        return result;
    });
    expect(evidence.combinations).toHaveLength(6);
    for (const [index, sample] of evidence.combinations.entries()) {
        expect(sample.layers).toBeGreaterThanOrEqual(1); expect(sample.layers).toBeLessThanOrEqual(3);
        expect(sample.depth).toBeGreaterThanOrEqual(3); expect(sample.depth).toBeLessThanOrEqual(5);
        expect(sample.passes).toBe(sample.depth);
        expect(sample.resolution[0]).toBeLessThanOrEqual(256); expect(sample.resolution[1]).toBeLessThanOrEqual(160);
        expect(sample.sameTargets).toBe(true);
        expect(sample.first).toBe(sample.repeated);
        expect(sample.first, `Billboard configuration ${index} should produce a visible image change`).not.toBe(sample.baselineHash);
        expect(sample.hit).toBe('primary-target'); expect(sample.gpuHit).toBe(sample.baselineHit);
        expect(sample.emission).toBeCloseTo(sample.baselineEmission, 8);
    }
    expect(evidence.combinations.map(sample => [sample.layers, sample.depth])).toEqual([[1, 3], [2, 4], [3, 5], [3, 5], [1, 3], [2, 3]]);
    expect(evidence.disposed).toEqual([1, 1]); expect(evidence.detached).toBe(true); expect(evidence.unchanged).toBe(true);
    expect(errors).toEqual([]);
    await testInfo.attach('bounded-camera-feedback.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
});

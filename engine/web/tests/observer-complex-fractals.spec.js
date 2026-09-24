/* global window, document, structuredClone, Buffer */
import { test, expect } from '@playwright/test';
import { writeFile } from 'node:fs/promises';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

const COMPLEX_PRESETS = [
    'fractal-mandelbulb', 'fractal-mandelbox', 'fractal-sierpinski',
    'fractal-apollonian', 'fractal-julia-quaternion', 'fractal-kleinian',
];

async function openObserver(page) {
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

test('complex fractals are available in World and detail changes presentation without editing the session', async ({ page }) => {
    test.setTimeout(120_000);
    await openObserver(page);
    await page.getByRole('button', { name: 'World', exact: true }).click();
    const world = page.locator('#observer-workspace');
    const detail = world.getByLabel('Fractal detail · 0 fast / 1 intricate', { exact: true });
    await expect(detail).toHaveValue('0.65');
    for (const preset of COMPLEX_PRESETS) {
        await world.getByLabel('Environment', { exact: true }).selectOption(preset);
        await page.waitForFunction(expected => {
            const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
            return workspace.snapshot.environment.preset === expected && workspace.renderer.diagnostics.fractalStyle >= 3;
        }, preset);
        expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.environment.animationRate)).toBeGreaterThan(0);
    }
    const before = await page.evaluate(() => JSON.stringify(window.__FTD_DEV__.registry.get('observerWorkspace').snapshot));
    await detail.fill('0.2');
    await detail.dispatchEvent('change');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').renderer.diagnostics.fractalDetail === 0.2);
    const after = await page.evaluate(() => JSON.stringify(window.__FTD_DEV__.registry.get('observerWorkspace').snapshot));
    expect(after).toBe(before);
});

test('reduced motion keeps a newly selected fractal still while allowing an explicit animation rate', async ({ page }) => {
    await page.emulateMedia({ reducedMotion: 'reduce' });
    await openObserver(page);
    await page.getByRole('button', { name: 'World', exact: true }).click();
    const world = page.locator('#observer-workspace');
    await world.getByLabel('Environment', { exact: true }).selectOption('fractal-mandelbulb');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.environment.preset === 'fractal-mandelbulb');
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.environment.animationRate)).toBe(0);
    const rate = world.getByLabel('Animation rate', { exact: true });
    await expect(rate).toHaveValue('0');
    await rate.fill('0.35');
    await rate.dispatchEvent('change');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.environment.animationRate === 0.35);
    await expect(rate).toHaveValue('0.35');
});

test('six complex skies evolve deterministically, cover all directions, and preserve optical target identity', async ({ page }, testInfo) => {
    test.setTimeout(180_000);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    page.on('console', message => {
        if (message.type() === 'error' && /shader|WebGL|Observer/i.test(message.text())) errors.push(message.text());
    });
    await openObserver(page);
    const evidence = await page.evaluate(async presets => {
        const { ObserverRenderer } = await import('/js/observer/renderer.js');
        const workspace = window.__FTD_DEV__.registry.get('observerWorkspace');
        const owner = workspace.client;
        const worker = owner.worker;
        const authoritativeBefore = JSON.stringify(workspace.snapshot);
        const container = document.createElement('div');
        container.style.cssText = 'position:fixed;left:-1000px;top:0;width:160px;height:100px';
        document.body.append(container);
        // This renderer has no frame loop, worker, or physics owner. Small buffers
        // keep functional coverage independent of the hardware performance gate.
        const renderer = new ObserverRenderer({ container });
        const snapshot = structuredClone(workspace.snapshot);
        snapshot.entities = []; snapshot.segments = []; snapshot.pulses = [];
        snapshot.environmentHistory = undefined;
        snapshot.time = 12; snapshot.historyStart = -50;
        // At y=0 the analytic ground cannot mask any of the six sky directions.
        snapshot.observer = { ...snapshot.observer, position: [0, 0, 0], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 };
        snapshot.environment = { ...snapshot.environment, seed: 17, density: 1, opacity: 1, animationRate: 0.35 };
        const settings = { ...workspace.settings, autoQuality: false, renderScale: 1, fractalDetail: 0.65,
            layers: {}, mirrorWorld: false, feedbackEnabled: false, doppler: false, beaming: false,
            cameraOverride: snapshot.observer };
        const read = () => {
            renderer.render(snapshot, settings, 0);
            const gl = renderer.renderer.getContext();
            const pixels = new Uint8Array(gl.drawingBufferWidth * gl.drawingBufferHeight * 4);
            gl.readPixels(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
            let hash = 2166136261, minimum = 255, maximum = 0, opaque = true;
            const colors = new Set();
            for (let i = 0; i < pixels.length; i += 4) {
                colors.add((pixels[i] << 16) | (pixels[i + 1] << 8) | pixels[i + 2]);
                opaque &&= pixels[i + 3] === 255;
                for (let component = 0; component < 3; component++) {
                    hash = Math.imul(hash ^ pixels[i + component], 16777619);
                    minimum = Math.min(minimum, pixels[i + component]);
                    maximum = Math.max(maximum, pixels[i + component]);
                }
            }
            return { hash: hash >>> 0, colors: colors.size, minimum, maximum, opaque, error: gl.getError() };
        };
        const results = [];
        try {
            for (const preset of presets) {
                snapshot.environment.preset = preset; snapshot.environment.seed = 17;
                snapshot.environment.animationRate = 0.35; snapshot.time = 12;
                snapshot.observer.yaw = 0; snapshot.observer.pitch = 0;
                settings.fractalDetail = 0.65; renderer.resize(160, 100);
                const first = read(), repeated = read();
                const diagnostic = { style: renderer.diagnostics.fractalStyle, detail: renderer.diagnostics.fractalDetail,
                    instances: renderer.diagnostics.tracedInstances, environmentInstances: renderer.environment.segments.length };
                snapshot.time = 29; const evolved = read();
                snapshot.environment.animationRate = 0;
                snapshot.time = 12; const frozen = read();
                snapshot.time = 29; const frozenLater = read();
                snapshot.environment.seed = 118; const differentSeed = read();
                snapshot.environment.seed = 17;
                const directions = [];
                for (const [name, yaw, pitch] of [
                    ['forward', 0, 0], ['backward', Math.PI, 0],
                    ['left', Math.PI / 2, 0], ['right', -Math.PI / 2, 0],
                    ['north pole', 0, Math.PI / 2], ['south pole', 0, -Math.PI / 2],
                ]) {
                    snapshot.observer.yaw = yaw; snapshot.observer.pitch = pitch;
                    directions.push({ name, ...read() });
                }
                snapshot.observer.yaw = 0; snapshot.observer.pitch = 0;
                settings.fractalDetail = 0; const fast = read();
                settings.fractalDetail = 1; const intricate = read();
                // Save independent visual evidence directly from the just-rendered
                // buffer; preserveDrawingBuffer=false must not defer this read.
                settings.fractalDetail = 0.8; renderer.resize(480, 300); read();
                const image = renderer.canvas.toDataURL('image/png').split(',')[1];
                renderer.resize(160, 100);
                const entity = { ...workspace.snapshot.entities[0], id: 'fractal-reference-target', revision: 7,
                    shape: 'box', position: [0, 0, -6], velocity: [0, 0, 0], originTime: 12,
                    size: [1.2, 1.2, 1.2], rotation: [0, 0, 0], alive: true };
                snapshot.entities = [entity]; snapshot.segments = [{ ...entity, entityId: entity.id, start: -50, end: null }];
                const originalState = JSON.stringify(snapshot);
                const targets = [];
                for (const detail of [0, 1]) {
                    settings.fractalDetail = detail; read();
                    const cpu = renderer.pick(snapshot, settings, 0, 0);
                    const gpu = renderer.readPixelHit(snapshot, settings, 0, 0);
                    targets.push({ detail, cpu, gpu, error: renderer.renderer.getContext().getError() });
                }
                results.push({ preset, first, repeated, evolved, frozen, frozenLater, differentSeed, directions,
                    fast, intricate, diagnostic, targets, snapshotUnchanged: JSON.stringify(snapshot) === originalState, image });
                snapshot.entities = []; snapshot.segments = [];
            }
            snapshot.environment.preset = 'void'; read();
            return { results, noFractalStyle: renderer.diagnostics.fractalStyle,
                authoritativeUnchanged: JSON.stringify(workspace.snapshot) === authoritativeBefore,
                ownerUnchanged: workspace.client === owner && owner.worker === worker, gpu: renderer.diagnostics.gpu };
        } finally { renderer.dispose(); container.remove(); }
    }, COMPLEX_PRESETS);

    // Keep the images and measurements even when a later assertion fails.
    for (const result of evidence.results) {
        const imagePath = testInfo.outputPath(`${result.preset}.png`);
        await writeFile(imagePath, Buffer.from(result.image, 'base64'));
        await testInfo.attach(result.preset, { path: imagePath, contentType: 'image/png' });
        delete result.image;
    }
    await testInfo.attach('complex-fractal-pixel-evidence.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
    for (const result of evidence.results) {
        expect(result.diagnostic.style, result.preset).toBeGreaterThanOrEqual(3);
        expect(result.diagnostic.style, result.preset).toBeLessThanOrEqual(8);
        expect(result.diagnostic.detail).toBe(0.65);
        expect(result.diagnostic.instances).toBe(0);
        expect(result.diagnostic.environmentInstances).toBe(0);
        expect(result.first.colors, result.preset).toBeGreaterThan(100);
        expect(result.first.maximum - result.first.minimum, result.preset).toBeGreaterThan(20);
        expect(result.repeated, result.preset).toEqual(result.first);
        expect(result.evolved.hash, `${result.preset} evolution`).not.toBe(result.first.hash);
        expect(result.frozenLater, `${result.preset} frozen`).toEqual(result.frozen);
        expect(result.differentSeed.hash, `${result.preset} seed`).not.toBe(result.frozen.hash);
        expect(result.fast.hash, `${result.preset} detail`).not.toBe(result.intricate.hash);
        for (const sample of [result.first, result.repeated, result.evolved, result.frozen, result.frozenLater,
            result.differentSeed, result.fast, result.intricate, ...result.directions]) {
            expect(sample.error, result.preset).toBe(0);
            expect(sample.opaque, result.preset).toBe(true);
            expect(sample.maximum, `${result.preset} nonblank view`).toBeGreaterThan(0);
        }
        for (const target of result.targets) {
            expect(target.error).toBe(0);
            expect(target.cpu?.entityId).toBe('fractal-reference-target');
            expect(target.gpu?.entityId).toBe(target.cpu?.entityId);
            expect(target.cpu?.revision).toBe(7);
            expect(target.gpu?.revision).toBe(7);
            expect(target.gpu.emissionTime).toBeCloseTo(target.cpu.emissionTime, 3);
            expect(target.gpu.distance).toBeCloseTo(target.cpu.distance, 3);
        }
        expect(result.targets[1].cpu).toEqual(result.targets[0].cpu);
        expect(result.targets[1].gpu).toEqual(result.targets[0].gpu);
        expect(result.snapshotUnchanged).toBe(true);
    }
    expect(new Set(evidence.results.map(result => result.first.hash)).size).toBe(COMPLEX_PRESETS.length);
    expect(new Set(evidence.results.map(result => result.diagnostic.style)).size).toBe(COMPLEX_PRESETS.length);
    expect(evidence.noFractalStyle).toBe(-1);
    expect(evidence.authoritativeUnchanged).toBe(true);
    expect(evidence.ownerUnchanged).toBe(true);
    expect(errors).toEqual([]);
});

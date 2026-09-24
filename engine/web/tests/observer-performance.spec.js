import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';
import { writeFile } from 'node:fs/promises';

for (const configuration of [
    { name: 'default immersion', preset: 'void', layers: 2, depth: 3 },
    { name: 'maximum camera echoes in evolving Julia', preset: 'fractal-julia', layers: 3, depth: 5 },
    { name: 'intricate Mandelbulb with maximum camera echoes', preset: 'fractal-mandelbulb', layers: 3, depth: 5, detail: 1 },
    { name: 'wave and spacetime instruments', preset: 'fractal-mandelbulb', layers: 2, depth: 3, phenomena: true },
]) test(`Observer declared workload sustains 60Hz with ${configuration.name}`, async ({ page }, testInfo) => {
    test.skip(process.env.FTD_HARDWARE_WEBGL !== '1', 'Performance certification requires the explicit hardware renderer run.');
    test.setTimeout(180_000);
    await page.setViewportSize({ width: 1920, height: 1080 });
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    const provenance = await page.evaluate(async configuration => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'pause' });
        const snapshot = structuredClone(w.snapshot);
        snapshot.entities = []; snapshot.segments = [];
        for (let i = 0; i < 64; i++) {
            const entity = { ...w.snapshot.entities[0], id: `load-${i}`, name: `Moving ${i}`,
                shape: ['sphere', 'box', 'tetrahedron', 'torus'][i % 4],
                position: [((i % 8) - 3.5) * 1.4, 0.8 + Math.floor(i / 8), -10 - Math.floor(i / 8) * 0.05],
                velocity: [i % 2 ? 0.08 : -0.08, 0, 0], size: [0.7, 0.7, 0.7], originTime: 0, overlay: true, alive: true };
            snapshot.entities.push(entity);
            snapshot.segments.push({ ...entity, entityId: entity.id, start: -1000, end: null });
        }
        await w.command({ type: 'load', snapshot });
        await w.command({ type: 'environment', patch: { preset: configuration.preset, animationRate: 0.35 } });
        w.setSetting('mirrorWorld', true);
        w.setSetting('feedbackEnabled', true);
        w.setSetting('feedbackLayers', configuration.layers);
        w.setSetting('feedbackDepth', configuration.depth);
        w.setSetting('fractalDetail', configuration.detail ?? 0.65);
        if (configuration.phenomena) { w.setSetting('waveWavelength', 0.5); w.setSetting('waveAmplitude', 2); }
        w.renderer.setBenchmarkScene({ staticInstances: 2000, uniqueTriangles: 100000 });
        w.setSetting('overlayFilter', 'all');
        w.setSetting('layers', configuration.phenomena
            ? { grid: true, interference: true, standingWaves: true, polarization: true, lightCones: true, simultaneity: true, lightPaths: true, aberration: true }
            : { grid: true, axes: true, sites: true, bonds: true, wireframe: true, vectors: true, clocks: true, rings: true });
        w.setSetting('renderScale', Number(new URLSearchParams(location.search).get('observerQuality') || 1));
        await w.command({ type: 'play' });
        return w.renderer.diagnostics.gpu;
    }, configuration);
    expect(JSON.stringify(provenance)).not.toMatch(/SwiftShader|llvmpipe|software/i);
    await page.waitForTimeout(4000);
    // Warm shader caches and the adaptive controller before measuring. A fixed
    // delay alone can include resolution reallocations in the warmed sample.
    // This has a declared bound: failure to settle fails the gate.
    const warmup = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        const started = performance.now();
        let stableFrames = 0, previous = '';
        while (stableFrames < 120 && performance.now() - started < 20000) {
            await new Promise(resolve => requestAnimationFrame(resolve));
            const key = JSON.stringify([w.renderer.diagnostics.internalResolution, w.renderer.diagnostics.feedbackResolution]);
            stableFrames = key === previous ? stableFrames + 1 : 0;
            previous = key;
        }
        return { stableFrames, additionalWarmupMs: performance.now() - started, initialWarmupMs: 4000 };
    });
    expect(warmup.stableFrames, 'Adaptive quality must settle within the bounded warmup.').toBeGreaterThanOrEqual(120);
    const evidence = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.frameIntervals.length = 0;
        const resolutionChanges = [];
        let previousResolution = '';
        while (w.frameIntervals.length < 600) {
            await new Promise(resolve => requestAnimationFrame(resolve));
            const diagnostics = w.renderer.diagnostics;
            const key = JSON.stringify([diagnostics.internalResolution, diagnostics.feedbackResolution]);
            if (key !== previousResolution) {
                resolutionChanges.push({ frame: w.frameIntervals.length, image: [...diagnostics.internalResolution], echoes: [...diagnostics.feedbackResolution] });
                previousResolution = key;
            }
        }
        const frames = w.frameIntervals.slice(-600).sort((a, b) => a - b);
        const total = frames.reduce((sum, value) => sum + value, 0);
        return { ...w.renderer.diagnostics, resolutionChanges, frames: frames.length, fps: 600000 / total,
            p95: frames[Math.floor(frames.length * 0.95)], p99: frames[Math.floor(frames.length * 0.99)],
            max: frames.at(-1), labels: w.labels.labels.filter(label => !label.hidden).length,
            waveWavelength: w.settings.waveWavelength, waveAmplitude: w.settings.waveAmplitude };
    });
    const evidencePath = testInfo.outputPath('hardware-performance.json');
    await writeFile(evidencePath, JSON.stringify({ ...evidence, warmup }, null, 2));
    await testInfo.attach('hardware-performance.json', { path: evidencePath, contentType: 'application/json' });
    expect(evidence.movingInstances).toBeGreaterThanOrEqual(64);
    expect(evidence.staticInstances).toBeGreaterThanOrEqual(2000);
    expect(evidence.requestedTriangleBudget).toBe(100000);
    expect(evidence.uniqueTriangles).toBeGreaterThanOrEqual(99990);
    expect(evidence.uniqueTriangles).toBeLessThanOrEqual(100000);
    expect(evidence.meshBvhNodes).toBeGreaterThan(25000);
    expect(evidence.layerCount).toBe(8);
    expect(evidence.labels).toBe(32);
    expect(evidence.mirrorWorld).toBe(true);
    expect(evidence.feedbackLayers).toBe(configuration.layers);
    expect(evidence.feedbackPasses).toBe(configuration.depth);
    expect(evidence.feedbackTargetCount).toBe(2);
    expect(evidence.fps).toBeGreaterThanOrEqual(59.5);
    expect(evidence.p95).toBeLessThanOrEqual(17);
    expect(evidence.p99).toBeLessThanOrEqual(20);
});

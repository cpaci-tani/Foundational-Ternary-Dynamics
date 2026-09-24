import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';
import { writeFile } from 'node:fs/promises';

for (const configuration of [
    { name: 'AI loaded and idle', preset: 'void', layers: 2, depth: 3, decoding: false },
    { name: 'AI actively decoding', preset: 'void', layers: 2, depth: 3, decoding: true },
]) test(`Observer declared workload sustains 60Hz with ${configuration.name}`, async ({ page }, testInfo) => {
    test.skip(process.env.FTD_HARDWARE_WEBGL !== '1', 'Performance certification requires the explicit hardware renderer run.');
    test.setTimeout(180_000);
    await page.setViewportSize({ width: 1920, height: 1080 });
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('assistant'));
    const loadMs=await page.evaluate(async()=>{const t=performance.now();await window.__FTD_DEV__.registry.get('assistant').model.load();return performance.now()-t;});
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
        w.renderer.setBenchmarkScene({ staticInstances: 2000, uniqueTriangles: 100000 });
        w.setSetting('overlayFilter', 'all');
        w.setSetting('layers', { grid: true, axes: true, sites: true, bonds: true, wireframe: true, vectors: true, clocks: true, rings: true });
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
    const inferenceWarmup=await page.evaluate(async configuration => {
        const ai = window.__FTD_DEV__.registry.get('assistant');
        // Record first-generation compilation separately, then measure warmed
        // generation exactly as the declared 600-warmed-frame gate requires.
        const started=performance.now();
        if(configuration.decoding)await ai.model.generate([{role:'user',content:'Explain the difference between received light and simultaneous geometry.'}],new AbortController().signal);
        const firstGenerationMs=performance.now()-started;
        window.__aiPerf = { running: configuration.decoding, generations: 0, chunks: 0, characters: 0, errors: [], ms: 0 };
        if (configuration.decoding) void (async () => {
            while (window.__aiPerf.running) {
                const start=performance.now();
                try {
                    await ai.model.generate([{role:'user',content:'Explain geometric grids, clocks, and the difference between received light and simultaneous measurements in 300 words.'}], new AbortController().signal, undefined, text => {
                        if (text) { window.__aiPerf.chunks++; window.__aiPerf.characters += text.length; }
                    });
                    window.__aiPerf.generations++;
                }
                catch(error){window.__aiPerf.errors.push(error.message);break;}
                window.__aiPerf.ms+=performance.now()-start;
            }
        })();
        // Inference changes the automatic pixel budget. Let that transition
        // settle with decoding running continuously before the measured sample.
        const w=window.__FTD_DEV__.registry.get('observerWorkspace');
        let stable=0,last='';const warm=performance.now();
        while(stable<120 && performance.now()-warm<20000){
            await new Promise(resolve=>requestAnimationFrame(resolve));
            const next=JSON.stringify([w.renderer.diagnostics.internalResolution,w.renderer.diagnostics.feedbackResolution]);
            stable=next===last?stable+1:0;last=next;
        }
        return{firstGenerationMs,stableFrames:stable,decodingWarmupMs:performance.now()-warm};
    }, configuration);
    expect(inferenceWarmup.stableFrames).toBeGreaterThanOrEqual(120);
    const evidence = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        const initialTick=w.snapshot.tick; w.frameIntervals.length = 0;
        const initialChunks=window.__aiPerf.chunks, initialCharacters=window.__aiPerf.characters;
        let decodingFrames=0, sampledFrames=0;
        let lastTick=initialTick,freshSnapshots=0;const measuredAt=performance.now();
        const resolutionChanges = [];
        let previousResolution = '';
        while (w.frameIntervals.length < 600) {
            await new Promise(resolve => requestAnimationFrame(resolve));
            sampledFrames++;
            if(window.__FTD_DEV__.registry.get('assistant').model.busy)decodingFrames++;
            if(w.snapshot.tick!==lastTick){freshSnapshots++;lastTick=w.snapshot.tick;}
            const diagnostics = w.renderer.diagnostics;
            const key = JSON.stringify([diagnostics.internalResolution, diagnostics.feedbackResolution]);
            if (key !== previousResolution) {
                resolutionChanges.push({ frame: w.frameIntervals.length, image: [...diagnostics.internalResolution], echoes: [...diagnostics.feedbackResolution] });
                previousResolution = key;
            }
        }
        const frames = w.frameIntervals.slice(-600).sort((a, b) => a - b);
        const total = frames.reduce((sum, value) => sum + value, 0);
        window.__aiPerf.running=false; window.__FTD_DEV__.registry.get('assistant').model.cancel();
        return { ai:{...window.__aiPerf,measuredChunks:window.__aiPerf.chunks-initialChunks,measuredCharacters:window.__aiPerf.characters-initialCharacters,decodingFrames,sampledFrames},initialTick,finalTick:w.snapshot.tick,freshSnapshots,freshSnapshotsPerSecond:freshSnapshots*1000/(performance.now()-measuredAt),jsHeapBytes:performance.memory?.usedJSHeapSize??null,gpuMemory:'No browser GPU allocation counter; model manifest estimate only',...w.renderer.diagnostics, resolutionChanges, frames: frames.length, fps: 600000 / total,
            p95: frames[Math.floor(frames.length * 0.95)], p99: frames[Math.floor(frames.length * 0.99)],
            max: frames.at(-1), labels: w.labels.labels.filter(label => !label.hidden).length };
    });
    const evidencePath = testInfo.outputPath('hardware-performance.json');
    await writeFile(evidencePath, JSON.stringify({ ...evidence, warmup,inferenceWarmup,modelLoadMs:loadMs }, null, 2));
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
    expect(evidence.ai.errors).toEqual([]);
    if(configuration.decoding){
        // A complete response can exceed the sample duration. Require observed
        // streaming progress during this exact window, not response completion.
        expect(evidence.ai.measuredChunks).toBeGreaterThan(0);
        expect(evidence.ai.measuredCharacters).toBeGreaterThan(0);
        expect(evidence.ai.decodingFrames/evidence.ai.sampledFrames).toBeGreaterThanOrEqual(0.99);
    }
    expect(evidence.finalTick).toBeGreaterThan(evidence.initialTick);
    expect(evidence.fps).toBeGreaterThanOrEqual(59.5);
    expect(evidence.p95).toBeLessThanOrEqual(17);
    expect(evidence.p99).toBeLessThanOrEqual(20);
});

import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

test('GPU historical rays agree with the Float64 oracle across all rest shapes and moving frames', async ({ page }) => {
    test.setTimeout(120_000);
    const errors = [];
    page.on('console', message => { if (message.type() === 'error' && /shader|WebGL|Observer/i.test(message.text())) errors.push(message.text()); });
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    const evidence = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'pause' });
        const { SHAPES } = await import('/js/observer/catalog.js');
        const snapshot = structuredClone(w.snapshot);
        snapshot.time = 10; snapshot.historyStart = -50; snapshot.environment.preset = 'void';
        snapshot.observer = { ...snapshot.observer, position: [0, 0, 6], velocity: [0, 0, 0], yaw: 0, pitch: 0, roll: 0 };
        const failures = []; let hits = 0, rays = 0;
        for (const shape of SHAPES.map(s => s.id)) {
            for (const beta of [0, 0.6]) {
                snapshot.observer.velocity = [beta, 0, 0];
                const entity = { ...w.snapshot.entities[0], id: 'parity', shape, position: [0, 0, 0],
                    velocity: [0.2, 0, 0], originTime: 10, size: [3, 3, 3], rotation: [0.1, 0.2, 0.05], alive: true };
                snapshot.entities = [entity]; snapshot.segments = [{ ...entity, entityId: entity.id, start: -1000, end: null }];
                const settings = { ...w.settings, layers: {}, cameraOverride: snapshot.observer, renderScale: 0.5 };
                w.renderer.render(snapshot, settings);
                for (const x of [-0.8, -0.4, 0, 0.4, 0.8]) for (const y of [-0.15, 0, 0.15]) {
                    const cpu = w.renderer.pick(snapshot, settings, x, y);
                    const gpu = w.renderer.readPixelHit(snapshot, settings, x, y);
                    rays++;
                    if (cpu) hits++;
                    if ((cpu?.entityId || null) !== (gpu?.entityId || null)
                        || (cpu && Math.abs(cpu.emissionTime - gpu.emissionTime) > 0.003)) {
                        failures.push({ shape, beta, x, y, cpu, gpu });
                    }
                }
            }
        }
        return { failures: failures.slice(0, 8), rays, hits, gpu: w.renderer.diagnostics.gpu };
    });
    expect(evidence.hits).toBeGreaterThan(30);
    expect(evidence.failures, JSON.stringify(evidence.failures)).toEqual([]);
    expect(errors).toEqual([]);
});

test('rendered presets and inspector have usable desktop and narrow layouts', async ({ page }, testInfo) => {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.screenshot({ path: testInfo.outputPath('minds-eye-desktop.png') });
    await page.getByRole('button', { name: 'World', exact: true }).click();
    await expect(page.locator('[data-observer-panel]')).toBeVisible();
    expect(await page.locator('#observer-workspace').evaluate(element => element.scrollWidth <= element.clientWidth)).toBe(true);
    await page.screenshot({ path: testInfo.outputPath('minds-eye-world-controls.png') });
    await page.setViewportSize({ width: 800, height: 700 });
    expect(await page.locator('#observer-workspace').evaluate(element => element.scrollWidth <= element.clientWidth)).toBe(true);
    await page.screenshot({ path: testInfo.outputPath('minds-eye-narrow.png') });
});

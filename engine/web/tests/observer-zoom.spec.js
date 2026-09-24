import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function openObserver(page) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale', 0.25);
        w.setSetting('autoQuality', false);
        await w.command({ type: 'pause' });
    });
    await page.locator('.observer-canvas').hover({ position: { x: 600, y: 320 } });
}

test('wheel zoom travels past objects without changing FoV, time, or authored geometry and reverses', async ({ page }) => {
    await openObserver(page);
    const before = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { position: w.snapshot.observer.position, time: w.snapshot.time, entities: w.snapshot.entities,
            segments: w.snapshot.segments, fov: w.settings.fov };
    });
    for (let step = 0; step < 8; step++) await page.mouse.wheel(0, -1000);
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2])).toBeCloseTo(before.position[2] - 80, 8);
    const after = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        return { time: w.snapshot.time, entities: w.snapshot.entities, segments: w.snapshot.segments,
            fov: w.settings.fov, velocity: w.snapshot.observer.velocity, properTime: w.snapshot.observer.properTime };
    });
    expect(after).toEqual({ time: before.time, entities: before.entities, segments: before.segments,
        fov: before.fov, velocity: [0, 0, 0], properTime: 0 });
    for (let step = 0; step < 8; step++) await page.mouse.wheel(0, 1000);
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2])).toBeCloseTo(before.position[2], 8);
});

test('scroll follows the complete look direction, supports wheel units, and respects adjustable sensitivity', async ({ page }) => {
    await openObserver(page);
    await page.getByRole('button', { name: 'Camera', exact: true }).click();
    await page.getByLabel('Scroll zoom speed · units per notch').fill('2');
    await page.getByLabel('Scroll zoom speed · units per notch').press('Tab');
    await page.getByRole('button', { name: 'Close controls', exact: true }).click();
    const before = await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.input.setPose({ yaw: -Math.PI / 2, pitch: Math.PI / 6 });
        const position = [...w.snapshot.observer.position];
        // Line-mode and page-mode events exercise Firefox/mouse and trackpad units.
        w.renderer.canvas.dispatchEvent(new WheelEvent('wheel', { deltaY: -2, deltaMode: 1, bubbles: true, cancelable: true }));
        w.renderer.canvas.dispatchEvent(new WheelEvent('wheel', { deltaY: -1, deltaMode: 2, bubbles: true, cancelable: true }));
        return { position, distance: (32 + w.renderer.canvas.clientHeight) * 2 / 100 };
    });
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[0])).toBeCloseTo(before.position[0] + Math.cos(Math.PI / 6) * before.distance, 7);
    const position = await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position);
    expect(position[1]).toBeCloseTo(before.position[1] + before.distance / 2, 7);
    expect(position[2]).toBeCloseTo(before.position[2], 7);
});

test('panels, browser zoom and suspended workspaces do not consume scene scroll', async ({ page }) => {
    await openObserver(page);
    const before = await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position);
    await page.getByRole('button', { name: 'Camera', exact: true }).click();
    await page.locator('.observer-panel-body').hover();
    await page.mouse.wheel(0, 500);
    await expect.poll(() => page.locator('.observer-panel-body').evaluate(element => element.scrollTop)).toBeGreaterThan(0);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position)).toEqual(before);
    const result = await page.evaluate(() => {
        const registry = window.__FTD_DEV__.registry, w = registry.get('observerWorkspace');
        w.ui.closePanel();
        const browserZoom = new WheelEvent('wheel', { deltaY: -100, ctrlKey: true, cancelable: true });
        w.renderer.canvas.dispatchEvent(browserZoom);
        registry.get('observerHost').exit();
        const hiddenZoom = new WheelEvent('wheel', { deltaY: -100, cancelable: true });
        w.renderer.canvas.dispatchEvent(hiddenZoom);
        return { browserConsumed: browserZoom.defaultPrevented, hiddenConsumed: hiddenZoom.defaultPrevented, pending: w.input.consumeZoom() };
    });
    expect(result).toEqual({ browserConsumed: false, hiddenConsumed: false, pending: [0, 0, 0] });
});

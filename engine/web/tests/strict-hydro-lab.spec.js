import { test, expect } from '@playwright/test';
import { mkdir, writeFile } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

// Actual isolated HydroState/WASM; bounded UI controls, not a recovery campaign.
test.skip(process.env.FTD_STRICT_HYDRO_LAB !== '1', 'Requires isolated compiled hydro artifacts and the hydro config');
const PREPARATIONS = ['hydro-shear-wave-t2', 'hydro-shear-wave-e', 'hydro-sound-wave', 'hydro-taylor-green', 'hydro-shear-layer', 'hydro-vortex-pair'];
const SCREENSHOT_PATH = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../test-results/strict-hydro-lab/fluid-panel.png');
const errors = new WeakMap();
test.beforeEach(async ({ page }) => {
    const messages = []; errors.set(page, messages);
    page.on('pageerror', error => messages.push(error.message));
});
test.afterEach(async ({ page }) => { expect(errors.get(page)).toEqual([]); });
const snapshot = page => page.evaluate(() => window.__hydroLabSnapshot);
const lineage = s => ({ ownerId: s.ownerId, generation: s.generation, microtick: s.microtick, historyLength: s.historyLength });
async function ready(page) {
    await expect(page.locator('#step')).toBeEnabled();
    await expect.poll(async () => Boolean(await snapshot(page))).toBe(true);
}
async function load(page, preparation = PREPARATIONS[0]) {
    await page.goto('/strict/web/hydro/'); await ready(page);
    if (preparation !== PREPARATIONS[0]) {
        await page.locator('#preparation').selectOption(preparation);
        await expect.poll(async () => (await snapshot(page))?.preparation).toBe(preparation);
        await ready(page);
    }
}
async function cycle(page) {
    const tick = BigInt((await snapshot(page)).microtick);
    await page.locator('#step').click();
    await expect.poll(async () => (await snapshot(page))?.microtick).toBe(String(tick + 4n));
    await ready(page);
}
async function conserved(page, initial) {
    const final = await snapshot(page);
    expect(final.mass).toBe(initial.mass); expect(final.momentum).toEqual(initial.momentum);
    expect(final.work).toBe(initial.work); expect(final.viscosityAvailable).toBe(false);
    await expect(page.locator('#viscosity')).toHaveText('Unavailable');
    return final;
}
for (const preparation of PREPARATIONS) {
    test(preparation + ': two actual L16 cycles preserve N/P and leave viscosity open', async ({ page }) => {
        await load(page, preparation);
        const initial = await snapshot(page);
        expect(initial.L).toBe(16); expect(initial.microtick).toBe('0');
        await cycle(page); await cycle(page);
        const final = await conserved(page, initial);
        expect(final.microtick).toBe('8'); expect(final.generation).toBe('2'); expect(final.historyLength).toBe(3);
        expect(final.source.status).toBe('available'); expect(final.source.transferred_tokens).toBe('0');
        await expect(page.locator('#caption')).toContainText('do not establish autonomous fluid evolution');
        if (preparation === PREPARATIONS[0]) {
            await page.setViewportSize({ width: 1440, height: 1100 });
            await page.evaluate(() => new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve))));
            await mkdir(path.dirname(SCREENSHOT_PATH), { recursive: true });
            await page.screenshot({ path: SCREENSHOT_PATH, fullPage: true });
        }
    });
}
for (const preparation of [PREPARATIONS[0], PREPARATIONS[1], 'hydro-taylor-green']) {
    test(preparation + ': eight samples respect the empirical shear-only fit gate', async ({ page }) => {
        await load(page, preparation);
        const initial = await snapshot(page);
        for (let i = 0; i < 7; i++) await cycle(page);
        const final = await conserved(page, initial); expect(final.historyLength).toBe(8);
        const provenance = JSON.parse(await page.locator('#provenance').textContent());
        if (preparation === 'hydro-taylor-green') {
            expect(provenance.fit.available).toBe(false);
            await expect(page.locator('#gammaFit')).toHaveText('—');
        } else {
            expect(provenance.fit.available).toBe(true);
            expect(Number.isFinite(provenance.fit.gamma)).toBe(true);
            expect(Number.isFinite(provenance.fit.diffusivity)).toBe(true);
            expect(provenance.fit.uncertainty).toBe('not certified');
            await expect(page.locator('#gammaFit')).not.toHaveText('—');
        }
    });
}
test('resolution, axes, overlays and polarity preserve clock, owner, generation and history', async ({ page }) => {
    await page.addInitScript(() => {
        window.__hydroUiRequests = [];
        const send = Worker.prototype.postMessage;
        Worker.prototype.postMessage = function(message, ...args) {
            window.__hydroUiRequests.push({ op: message.op, observable: message.observable });
            return send.call(this, message, ...args);
        };
    });
    await load(page); await cycle(page);
    const initial = await snapshot(page);
    await page.evaluate(() => { window.__hydroUiRequests.length = 0; });
    await page.locator('#width').selectOption('4');
    await expect.poll(async () => (await snapshot(page))?.width).toBe(4); await ready(page);
    await page.locator('#plane').selectOption('XZ'); await page.locator('#plane').selectOption('YZ');
    await page.locator('#slice').fill('2'); await page.locator('#arrows').uncheck();
    await page.locator('#stress-mode').selectOption('xy'); await page.locator('#flow-mode').selectOption('divergence');
    await page.locator('#polarity').selectOption('both');
    expect(lineage(await snapshot(page))).toEqual(lineage(initial));
    await page.locator('#polarity').selectOption('1'); expect((await snapshot(page)).mass).toBe('0');
    await page.locator('#polarity').selectOption('0'); expect((await snapshot(page)).mass).toBe(initial.mass);
    expect(lineage(await snapshot(page))).toEqual(lineage(initial));
    expect(await page.evaluate(() => window.__hydroUiRequests)).toEqual([{ op: 'observe', observable: 'fluid' }]);
});
test('reset during a real pending advance suppresses the old owner publication', async ({ page }) => {
    await load(page); const old = await snapshot(page);
    const pending = await page.evaluate(() => {
        document.querySelector('#step').click();
        const during = window.__hydroLabPerformance();
        document.querySelector('#reset').click();
        return { during, cleared: window.__hydroLabSnapshot === null };
    });
    expect(pending.during.pendingRequests).toBe(1); expect(pending.during.busy).toBe(true); expect(pending.cleared).toBe(true);
    await ready(page); const fresh = await snapshot(page);
    expect(fresh.ownerId).not.toBe(old.ownerId); expect(fresh.generation).toBe('0');
    expect(fresh.microtick).toBe('0'); expect(fresh.historyLength).toBe(1);
    await cycle(page); expect((await snapshot(page)).ownerId).toBe(fresh.ownerId);
});
test('pagehide disposes observation UI and persisted pageshow creates one fresh owner', async ({ page }) => {
    await load(page); await cycle(page); const old = await snapshot(page);
    await page.evaluate(() => window.dispatchEvent(new PageTransitionEvent('pagehide', { persisted: true })));
    expect(await snapshot(page)).toBeNull();
    await expect(page.locator('#hydro-validity-status')).toHaveCount(0);
    await expect(page.locator('#hydro-validity-status-details')).toHaveCount(0);
    expect(await page.evaluate(() => window.__hydroLabPerformance().pendingRequests)).toBe(0);
    await page.evaluate(() => window.dispatchEvent(new PageTransitionEvent('pageshow', { persisted: true })));
    await ready(page); const fresh = await snapshot(page);
    expect(fresh.ownerId).not.toBe(old.ownerId); expect(fresh.microtick).toBe('0'); expect(fresh.historyLength).toBe(1);
    await expect(page.locator('#hydro-validity-status')).toHaveCount(1);
    await expect(page.locator('#hydro-validity-status-details')).toHaveCount(1);
});
test('vacuum publishes zero counts with undefined velocity derivatives and no fit', async ({ page }) => {
    await load(page); await page.locator('#occupation').selectOption('0'); await ready(page);
    const initial = await snapshot(page);
    expect(initial.mass).toBe('0'); expect(initial.derivativeBlocks).toBe(0);
    await cycle(page); await conserved(page, initial);
    await expect(page.locator('#speed')).toHaveText('—'); await expect(page.locator('#gammaFit')).toHaveText('—');
    await page.locator('#density').click();
    await expect(page.locator('#inspector')).toContainText('undefined (vacuum)');
    await expect(page.locator('#flow-legend')).toContainText('64 undefined cells');
});
test('interactive block side is capped at16 while L16 width1 remains available', async ({ page }) => {
    await load(page);
    await expect(page.locator('#width option').filter({ hasText: /^1$/ })).toHaveJSProperty('disabled', false);
    await page.locator('#width').selectOption('1');
    await expect.poll(async () => (await snapshot(page))?.width).toBe(1); await ready(page);
    await page.locator('#size').selectOption('32');
    await expect.poll(async () => (await snapshot(page))?.L).toBe(32); await ready(page);
    await expect(page.locator('#width option').filter({ hasText: /^1$/ })).toHaveJSProperty('disabled', true);
    const capped = await snapshot(page);
    expect(capped.width).toBe(2); expect(capped.microtick).toBe('0'); expect(capped.generation).toBe('0');
    await page.locator('#size').selectOption('16');
    await expect.poll(async () => (await snapshot(page))?.L).toBe(16); await ready(page);
    await expect(page.locator('#width option').filter({ hasText: /^1$/ })).toHaveJSProperty('disabled', false);
});
test('bounded L32 width2 live rendering records 600 frames and at least ten seconds of actual work', async ({ page }, testInfo) => {
    const width = 2;
    await load(page); await page.locator('#size').selectOption('32');
    await expect.poll(async () => (await snapshot(page))?.L).toBe(32); await ready(page);
    await page.locator('#width').selectOption(String(width));
    await expect.poll(async () => (await snapshot(page))?.width).toBe(width); await ready(page);
    const initial = await snapshot(page);
    await page.locator('#run').click();
    const measurement = await page.evaluate(async width => {
        const canvas = document.createElement('canvas'), gl = canvas.getContext('webgl2') || canvas.getContext('webgl');
        const extension = gl?.getExtension('WEBGL_debug_renderer_info');
        const renderer = gl ? { vendor: gl.getParameter(gl.VENDOR), renderer: gl.getParameter(gl.RENDERER), version: gl.getParameter(gl.VERSION),
            unmaskedVendor: extension ? gl.getParameter(extension.UNMASKED_VENDOR_WEBGL) : null,
            unmaskedRenderer: extension ? gl.getParameter(extension.UNMASKED_RENDERER_WEBGL) : null } : { unavailable: true };
        gl?.getExtension('WEBGL_lose_context')?.loseContext();
        const start = performance.now(), startSnapshot = structuredClone(window.__hydroLabSnapshot);
        const paintStart = window.__hydroLabPerformance().paintMs.length, intervals = [];
        let previous = null;
        await new Promise((resolve, reject) => {
            const timeout = setTimeout(() => reject(new Error('600-frame observation exceeded 35 seconds')), 35000);
            function sample(time) {
                if (previous !== null) intervals.push(time - previous); previous = time;
                if (intervals.length >= 600 && performance.now() - start >= 10000) { clearTimeout(timeout); resolve(); }
                else requestAnimationFrame(sample);
            }
            requestAnimationFrame(sample);
        });
        const elapsedMs = performance.now() - start, performanceReadout = window.__hydroLabPerformance();
        const paintMs = performanceReadout.paintMs.slice(paintStart);
        const p95 = values => values.length ? [...values].sort((a,b)=>a-b)[Math.ceil(values.length * .95)-1] : null;
        const endSnapshot = structuredClone(window.__hydroLabSnapshot);
        return { scope: 'One bounded interactive compiled-WASM case; no fluid-recovery or universal FPS claim',
            renderer, userAgent: navigator.userAgent, devicePixelRatio, viewport: [innerWidth,innerHeight],
            L: 32, width, elapsedMs, intervalsMs: intervals, frameCount: intervals.length,
            meanFps: 1000 * intervals.length / intervals.reduce((s,v)=>s+v,0), frameP95Ms: p95(intervals),
            paintMs, paintCount: paintMs.length, paintP95Ms: p95(paintMs), startSnapshot, endSnapshot,
            publishedCycles: endSnapshot.stage-startSnapshot.stage, performance: performanceReadout };
    }, width);
    await page.locator('#run').click(); await ready(page);
    const final = await conserved(page, initial); measurement.finalSnapshot = final;
    const artifact = testInfo.outputPath('fluid-l32-width' + width + '-rendering.json');
    await writeFile(artifact, JSON.stringify(measurement, null, 2));
    await testInfo.attach('fluid-l32-rendering.json', { path: artifact, contentType: 'application/json' });
    expect(measurement.elapsedMs).toBeGreaterThanOrEqual(10000); expect(measurement.frameCount).toBeGreaterThanOrEqual(600);
    expect(measurement.paintCount).toBeGreaterThan(0); expect(measurement.publishedCycles).toBeGreaterThan(0);
    expect(measurement.intervalsMs.every(v => Number.isFinite(v) && v > 0)).toBe(true);
    expect(final.ownerId).toBe(initial.ownerId); expect(final.stage).toBeGreaterThan(0);
});

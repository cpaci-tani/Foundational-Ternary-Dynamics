import { test, expect } from '@playwright/test';
import { createHash } from 'node:crypto';
import { mkdir, writeFile } from 'node:fs/promises';

test.skip(process.env.FTD_STRICT_LAB !== '1', 'Requires isolated compiled artifacts and strict config');
const preparations = ['relation', 'sparse'];
const sizes = [3, 4, 7, 9];
const divisors = size => Array.from({ length: size }, (_, i) => i + 1).filter(n => size % n === 0);
const snapshot = page => page.evaluate(() => window.__strictLabSnapshot);

async function ready(page) {
    await expect(page.locator('#step')).toBeEnabled();
    await expect.poll(async () => Boolean(await snapshot(page))).toBe(true);
}

async function load(page, preparation = 'relation', size = 4) {
    await page.goto('/strict/web/');
    await ready(page);
    if (preparation !== 'relation') {
        await page.locator('#preparation').selectOption(preparation);
        await ready(page);
    }
    if (size !== 4) {
        await page.locator('#size').selectOption(String(size));
        await ready(page);
    }
}

async function instrumentWorkers(page) {
    await page.addInitScript(() => {
        const NativeWorker = window.Worker;
        window.__strictWorkers = [];
        window.__strictHeld = [];
        window.__strictHoldOp = null;
        window.__strictSent = [];
        window.Worker = class extends NativeWorker {
            constructor(...args) { super(...args); window.__strictWorkers.push(this); }
            postMessage(message, ...args) {
                window.__strictSent.push({ worker: window.__strictWorkers.indexOf(this), message });
                if (window.__strictHoldOp === message.op) {
                    window.__strictHeld.push(() => super.postMessage(message, ...args));
                } else super.postMessage(message, ...args);
            }
        };
        window.__strictRelease = () => {
            window.__strictHoldOp = null;
            for (const send of window.__strictHeld.splice(0)) send();
        };
    });
}

function summed(values) { return values.reduce((sum, value) => sum + BigInt(value), 0n); }

for (const preparation of preparations) for (const size of sizes) {
    test(`real WASM ${preparation} L=${size}: all divisor views share one immutable state`, async ({ page }) => {
        const errors = [];
        page.on('pageerror', error => errors.push(error.message));
        await load(page, preparation, size);
        const initial = await snapshot(page);
        const owner = initial.ownerId;
        for (const width of divisors(size)) {
            await page.locator('#width').selectOption(String(width));
            await ready(page);
            const observed = await snapshot(page);
            expect(observed.ownerId).toBe(owner);
            expect(observed.tick).toBe(initial.tick);
            expect(observed.generation).toBe(initial.generation);
            expect(observed.width).toBe(String(width));
            expect(observed.blocks.width).toBe(String(width));
            expect(observed.blocks.law_id).toBe('phi-v2-staged-candidate-1');
            expect(observed.blocks.collision_hash).toMatch(/^[A-F0-9]{64}$/);
            expect(observed.blocks.tick_start).toBe(observed.tick);
            expect(observed.blocks.tick_end).toBe(observed.tick);
            expect(observed.blocks.spatial_support).toBe('aligned_periodic_blocks');
            expect(observed.blocks.length_unit).toBe('microscopic_node');
            expect(observed.blocks.time_unit).toBe('staged_microtick');
            expect(observed.blocks.observer_kind).toBe('external_diagnostic');
            expect(observed.blocks.physical_calibration).toBe('unidentified');
            expect(observed.micro).toEqual(initial.micro);
            expect(summed(observed.blocks.field_tokens)).toBe(summed(observed.micro.field_tokens));
            expect(summed(observed.blocks.relation_tokens)).toBe(summed(observed.micro.relation_tokens));
            expect(summed(observed.blocks.incidence)).toBe(0n);
            expect(observed.blocks.field_tokens).toHaveLength((size / width) ** 3);
        }
        await page.locator('#step').click();
        await ready(page);
        const next = await snapshot(page);
        expect(next.ownerId).toBe(owner);
        expect(BigInt(next.tick)).toBe(BigInt(initial.tick) + 1n);
        expect(BigInt(next.generation)).toBe(BigInt(initial.generation) + 1n);
        expect(next.tokens).toBe(initial.tokens);
        expect(errors).toEqual([]);
    });
}

test('busy observation width is disabled, pause preserves a coherent publication', async ({ page }) => {
    await instrumentWorkers(page);
    await load(page, 'sparse', 4);
    await page.evaluate(() => { window.__strictHoldOp = 'advance'; });
    await page.locator('#step').click();
    await expect(page.locator('#width')).toBeDisabled();
    expect(await page.locator('#width').inputValue()).toBe('2');
    await page.evaluate(() => window.__strictRelease());
    await ready(page);
    const observed = await snapshot(page);
    expect(observed.width).toBe(observed.blocks.width);
    await page.locator('#run').click();
    await expect.poll(async () => BigInt((await snapshot(page)).tick) > BigInt(observed.tick)).toBe(true);
    await expect(page.locator('#run')).toHaveText('Pause');
    await page.locator('#run').click();
    await ready(page);
    const paused = await snapshot(page);
    await page.waitForTimeout(150);
    expect(await snapshot(page)).toEqual(paused);
});

test('reset ignores old worker error while the fresh owner is initializing', async ({ page }) => {
    await instrumentWorkers(page);
    await load(page);
    const old = await snapshot(page);
    await page.evaluate(() => { window.__strictHoldOp = 'init'; });
    await page.locator('#reset').click();
    await expect.poll(() => page.evaluate(() => window.__strictHeld.length)).toBe(1);
    expect(await snapshot(page)).toBeNull();
    await expect(page.locator('#run')).toBeDisabled();
    await page.evaluate(() => {
        window.__strictWorkers[0].onerror({ message: 'late replaced-owner error' });
        window.__strictRelease();
    });
    await ready(page);
    const fresh = await snapshot(page);
    expect(fresh.ownerId).not.toBe(old.ownerId);
    expect(fresh.tick).toBe('0');
    await expect(page.locator('#status')).not.toContainText('late replaced-owner');
});

test('artifact errors clear old data and controls; reset recovers', async ({ page }) => {
    await load(page);
    await page.route('**/build_strict/lab/relation_4.bin', route => route.fulfill({ status: 404, body: '' }));
    await page.locator('#reset').click();
    await expect(page.locator('#status')).toContainText('preparations are missing');
    expect(await snapshot(page)).toBeNull();
    await expect(page.locator('#step')).toBeDisabled();
    await expect(page.locator('#run')).toBeDisabled();
    await expect(page.locator('#width')).toBeDisabled();
    await expect(page.locator('#tick')).toHaveText('—');
    expect(await page.evaluate(() => ['micro', 'blocks'].every(id => {
        const canvas = document.getElementById(id);
        return !canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height).data.some(Boolean);
    }))).toBe(true);
    await page.unroute('**/build_strict/lab/relation_4.bin');
    await page.locator('#reset').click();
    await ready(page);
});

test('invalid response generation fails closed and cannot publish', async ({ page }) => {
    await instrumentWorkers(page);
    await load(page);
    await page.evaluate(() => { window.__strictHoldOp = 'advance'; });
    await page.locator('#step').click();
    await page.evaluate(() => {
        const { worker, message } = window.__strictSent.at(-1);
        window.__strictWorkers[worker].onmessage({ data: {
            ok: true, requestId: message.requestId, ownerId: message.ownerId,
            generation: message.generation, payload: {},
        } });
    });
    await expect(page.locator('#status')).toContainText('Invalid response lineage or generation');
    expect(await snapshot(page)).toBeNull();
    await expect(page.locator('#step')).toBeDisabled();
});

test('pagehide/pageshow stays unloaded until explicit reset', async ({ page }) => {
    await load(page);
    const old = await snapshot(page);
    await page.evaluate(() => {
        window.dispatchEvent(new PageTransitionEvent('pagehide', { persisted: true }));
        window.dispatchEvent(new PageTransitionEvent('pageshow', { persisted: true }));
    });
    expect(await snapshot(page)).toBeNull();
    await expect(page.locator('#status')).toContainText('Runtime unloaded');
    await expect(page.locator('#run')).toBeDisabled();
    await page.locator('#reset').click();
    await ready(page);
    expect((await snapshot(page)).ownerId).not.toBe(old.ownerId);
});

test('hardware browser matrix: 20 cases, 600 frames and 12 seconds each', async ({ page }, testInfo) => {
    test.skip(process.env.FTD_HARDWARE_WEBGL !== '1', 'Explicit hardware provenance required');
    testInfo.setTimeout(900_000);
    const reports = [];
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    const artifactHashes = async () => {
        const hashes = {};
        for (const path of ['/build_strict_wasm/ftd_strict_wasm.mjs', '/build_strict_wasm/ftd_strict_wasm.wasm',
            '/strict/web/laboratory.js', '/web/js/strict/strict-worker.js', '/web/js/strict/worker-protocol.js']) {
            const response = await page.request.get(path);
            expect(response.ok(), path).toBe(true);
            const bytes = await response.body();
            hashes[path] = { bytes: bytes.length, sha256: createHash('sha256').update(bytes).digest('hex') };
        }
        return hashes;
    };
    const artifactsBefore = await artifactHashes();
    await mkdir(testInfo.outputDir, { recursive: true });
    const reportPath = testInfo.outputPath('strict-lab-hardware-matrix.json');
    await load(page);
    const renderer = await page.evaluate(() => {
        const canvas = document.createElement('canvas');
        const gl = canvas.getContext('webgl2') || canvas.getContext('webgl');
        const info = gl?.getExtension('WEBGL_debug_renderer_info');
        const name = info ? String(gl.getParameter(info.UNMASKED_RENDERER_WEBGL)) : '';
        gl?.getExtension('WEBGL_lose_context')?.loseContext();
        return name;
    });
    expect(renderer).not.toBe('');
    expect(renderer).not.toMatch(/swiftshader|software|llvmpipe/i);
    for (const preparation of preparations) for (const size of sizes) {
        await load(page, preparation, size);
        for (const width of divisors(size)) {
            await page.locator('#width').selectOption(String(width));
            await ready(page);
            await page.locator('#run').click();
            await expect.poll(async () => BigInt((await snapshot(page)).tick) >= 8n).toBe(true);
            await page.waitForTimeout(500);
            const frames = await page.evaluate(async () => {
                const intervals = [], longTasks = [];
                const observer = new PerformanceObserver(list => longTasks.push(...list.getEntries().map(e => e.duration)));
                observer.observe({ entryTypes: ['longtask'] });
                const tickStart = window.__strictLabSnapshot.tick;
                let first, previous;
                await new Promise(resolve => {
                    function frame(now) {
                        if (first === undefined) first = now;
                        if (previous !== undefined) intervals.push(now - previous);
                        previous = now;
                        if (intervals.length >= 600 && now - first >= 12_000) resolve();
                        else requestAnimationFrame(frame);
                    }
                    requestAnimationFrame(frame);
                });
                observer.disconnect();
                const sorted = [...intervals].sort((a, b) => a - b);
                const durationMs = intervals.reduce((a, b) => a + b, 0);
                return { count: intervals.length, durationMs, meanFps: 1000 * intervals.length / durationMs,
                    p95Ms: sorted[Math.ceil(sorted.length * .95) - 1],
                    p99Ms: sorted[Math.ceil(sorted.length * .99) - 1], maxMs: sorted.at(-1),
                    intervalsOver33_4ms: intervals.filter(n => n > 33.4).length, longTasks,
                    tickStart, tickEnd: window.__strictLabSnapshot.tick,
                    visibility: document.visibilityState };
            });
            await page.locator('#run').click();
            await ready(page);
            reports.push({ preparation, size, width, renderer, rendering: 'Canvas2D; hardware WebGL probe only', frames });
            await writeFile(reportPath, JSON.stringify({ complete: false, artifactsBefore, reports, errors }, null, 2));
        }
    }
    const artifactsAfter = await artifactHashes();
    await writeFile(reportPath, JSON.stringify({ complete: true, artifactsBefore, artifactsAfter, reports, errors }, null, 2));
    await testInfo.attach('strict-lab-hardware-matrix.json', {
        path: reportPath, contentType: 'application/json',
    });
    console.log(JSON.stringify({ reportPath, renderer, cases: reports.length,
        minimumFps: Math.min(...reports.map(r => r.frames.meanFps)),
        worstP99Ms: Math.max(...reports.map(r => r.frames.p99Ms)),
        longestIntervalMs: Math.max(...reports.map(r => r.frames.maxMs)),
    }));
    expect(artifactsAfter, 'same frozen artifact throughout the matrix').toEqual(artifactsBefore);
    expect(reports).toHaveLength(20);
    for (const report of reports) {
        const label = `${report.preparation} L=${report.size} width=${report.width}`;
        const { frames } = report;
        expect(frames.count, label).toBeGreaterThanOrEqual(600);
        expect(frames.durationMs, label).toBeGreaterThanOrEqual(12_000);
        expect(frames.visibility, label).toBe('visible');
        expect(BigInt(frames.tickEnd), label).toBeGreaterThan(BigInt(frames.tickStart));
        expect(frames.meanFps, label).toBeGreaterThanOrEqual(59.5);
        expect(frames.p95Ms, label).toBeLessThanOrEqual(17);
        expect(frames.p99Ms, label).toBeLessThanOrEqual(20);
        expect(frames.intervalsOver33_4ms, label).toBe(0);
        expect(frames.longTasks, label).toEqual([]);
    }
    expect(errors).toEqual([]);
});

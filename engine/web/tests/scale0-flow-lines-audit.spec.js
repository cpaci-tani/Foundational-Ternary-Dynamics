// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

const LATTICE_SIZES = [9, 17, 25, 33, 49, 65, 97, 113, 145, 181];

test.describe('Scale 0 Flux/E/B flow-lines audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(30_000);
        await page.addInitScript(() => {
            if (sessionStorage.getItem('ftd.flowLinesTestInitialized') === '1') return;
            localStorage.removeItem('ftd.scale0.flowLines');
            sessionStorage.setItem('ftd.flowLinesTestInitialized', '1');
        });
        await gotoAndReady(page);
        await page.waitForFunction(() => window.__ftdCtx?.fluxMock?.ready === true);
    });

    test('all supported sizes stay inside deterministic work and seed bounds', async ({ page }) => {
        const result = await page.evaluate(async (sizes) => {
            const { computeStreamlineParams } = await import(
                '/js/scales/scale0/runtime/streamline-integrator.js'
            );
            const { generateGridSeeds } = await import('/js/fieldlines.js');
            return sizes.map((N) => {
                const full = computeStreamlineParams(N);
                const reduced = computeStreamlineParams(N, { density: 0.25, length: 0.4 });
                const fallback = computeStreamlineParams(N, { inThreadWasm: true });
                const seeds = generateGridSeeds(N, full.seedSpacing, 1000);
                const outside = seeds.filter(([x, y, z]) => (
                    x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N
                )).length;
                return {
                    N,
                    full,
                    reduced,
                    fallback,
                    outside,
                    bWork: full.maxSeeds * Math.ceil(full.maxSteps * 1.5),
                };
            });
        }, LATTICE_SIZES);

        expect(result.map(({ full }) => full.maxSeeds))
            .toEqual([60, 60, 40, 36, 24, 24, 24, 24, 24, 24]);
        for (const row of result) {
            expect(row.outside, `L=${row.N} generated an out-of-bounds seed`).toBe(0);
            expect(row.full.maxLines).toBe(row.full.maxSeeds);
            expect(row.fallback.maxSeeds).toBeLessThanOrEqual(16);
            expect(row.fallback.maxLines).toBe(row.fallback.maxSeeds);
            expect(row.reduced.maxSeeds).toBeLessThan(row.full.maxSeeds);
            expect(row.reduced.maxSteps).toBeLessThan(row.full.maxSteps);
            expect(row.bWork, `L=${row.N} exceeded the audited B-line work envelope`)
                .toBeLessThanOrEqual(5364);
        }
    });

    test('incremental RK4 output is byte-identical to the synchronous API', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const {
                advanceStreamlineTask,
                beginStreamlineTask,
                computeStreamlines,
                createStreamlineTaskState,
                generateGridSeeds,
            } = await import('/js/fieldlines.js');
            const N = 17;
            const positions = [];
            const vectors = [];
            for (let z = 0.5; z < N; z += 2) {
                for (let y = 0.5; y < N; y += 2) {
                    for (let x = 0.5; x < N; x += 2) {
                        positions.push(x, y, z);
                        vectors.push(-(y - 8), x - 8, 0.1);
                    }
                }
            }
            const field = {
                positions: new Float32Array(positions),
                vectors: new Float32Array(vectors),
                count: positions.length / 3,
            };
            const seeds = generateGridSeeds(N, 4, 20);
            const opts = {
                N, stride: 2, maxSteps: 51, stepSize: 0.5,
                maxLines: 20, bidirectional: true,
            };
            const immediate = computeStreamlines(field, seeds, opts);
            const expected = {
                count: immediate.count,
                offsets: Array.from(immediate.offsets.slice(0, immediate.count)),
                lengths: Array.from(immediate.lengths.slice(0, immediate.count)),
                buffer: Array.from(immediate.buffer.slice(
                    0,
                    immediate.count
                        ? immediate.offsets[immediate.count - 1] + immediate.lengths[immediate.count - 1]
                        : 0,
                )),
            };
            const task = createStreamlineTaskState();
            beginStreamlineTask(task, field, seeds, opts);
            let slices = 0;
            while (!advanceStreamlineTask(task, { maxSeeds: 2 })) slices++;
            slices++;
            const actual = task.result;
            const liveFloats = actual.count
                ? actual.offsets[actual.count - 1] + actual.lengths[actual.count - 1]
                : 0;
            return {
                slices,
                expected,
                actual: {
                    count: actual.count,
                    offsets: Array.from(actual.offsets.slice(0, actual.count)),
                    lengths: Array.from(actual.lengths.slice(0, actual.count)),
                    buffer: Array.from(actual.buffer.slice(0, liveFloats)),
                },
            };
        });
        expect(result.slices).toBeGreaterThan(1);
        expect(result.actual).toEqual(result.expected);
    });

    test('shared controls persist and separate geometry work from material-only opacity', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const inventory = await page.evaluate(() => {
            const ids = [
                'flow-line-density', 'flow-line-density-val',
                'flow-line-length', 'flow-line-length-val',
                'flow-line-opacity', 'flow-line-opacity-val',
                'flow-line-budget', 'flow-line-reset',
            ];
            const card = document.getElementById('flow-lines-card');
            return {
                missing: ids.filter((id) => !document.getElementById(id)),
                duplicates: ids.filter((id) => document.querySelectorAll(`#${id}`).length !== 1),
                title: card?.querySelector('.card-title')?.textContent,
                values: ids.slice(0, 6).filter((_, i) => i % 2 === 0)
                    .map((id) => Number(document.getElementById(id)?.value)),
                labels: [...card.querySelectorAll('input[type="range"]')]
                    .map((input) => input.getAttribute('aria-label')),
                budget: document.getElementById('flow-line-budget')?.textContent,
            };
        });
        expect(inventory.missing).toEqual([]);
        expect(inventory.duplicates).toEqual([]);
        expect(inventory.title).toBe('Flow Lines');
        expect(inventory.values).toEqual([1, 1, 0.7]);
        expect(inventory.labels.every(Boolean)).toBe(true);
        expect(inventory.budget).toMatch(/^L=\d+ · \d+ lines · \d+\/\d+ steps$/);

        const transaction = await page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const state = store.getScale0State();
            const sentinel = { retained: true };

            state.streamlineSeedCache = sentinel;
            state.fieldNeedsUpdate = false;
            const opacity = document.getElementById('flow-line-opacity');
            opacity.value = '0.4';
            opacity.dispatchEvent(new Event('change', { bubbles: true }));
            const opacityOnly = {
                dirty: state.fieldNeedsUpdate,
                cacheRetained: state.streamlineSeedCache === sentinel,
                fieldOpacity: window.__ftdCtx.viewport._fieldRenderer._flowLineOpacity,
                fluxOpacity: window.__ftdCtx.viewport._fluxRenderer._flowLineOpacity,
            };

            state.streamlineSeedCache = sentinel;
            state.fieldNeedsUpdate = false;
            const density = document.getElementById('flow-line-density');
            density.value = '0.5';
            density.dispatchEvent(new Event('change', { bubbles: true }));
            const geometry = {
                dirty: state.fieldNeedsUpdate,
                cacheCleared: state.streamlineSeedCache === null,
            };
            return {
                opacityOnly,
                geometry,
                settings: { ...store.getFlowLineSettings() },
                stored: JSON.parse(localStorage.getItem('ftd.scale0.flowLines')),
            };
        });
        expect(transaction.opacityOnly).toEqual({
            dirty: false,
            cacheRetained: true,
            fieldOpacity: 0.4,
            fluxOpacity: 0.4,
        });
        expect(transaction.geometry).toEqual({ dirty: true, cacheCleared: true });
        expect(transaction.settings).toMatchObject({ density: 0.5, length: 1, opacity: 0.4 });
        expect(transaction.stored).toMatchObject({ density: 0.5, length: 1, opacity: 0.4 });

        await page.reload({ waitUntil: 'domcontentloaded' });
        await page.waitForFunction(() => !!window._ftdBridge);
        await expect(page.locator('#flow-line-density')).toHaveValue('0.5');
        await expect(page.locator('#flow-line-opacity')).toHaveValue('0.4');
        await page.locator('#flow-line-reset').click();
        await expect(page.locator('#flow-line-density')).toHaveValue('1');
        await expect(page.locator('#flow-line-length')).toHaveValue('1');
        await expect(page.locator('#flow-line-opacity')).toHaveValue('0.7');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('line meshes use bounded buffers and never submit empty requested layers', async ({ page }) => {
        const result = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            viewport.toggleEFieldLines(true);
            viewport.toggleBFieldLines(true);
            viewport.toggleFluxStreamlines(true);
            const field = viewport._fieldRenderer;
            const flux = viewport._fluxRenderer;
            return {
                e: {
                    capacity: field._eFieldLines.geometry.getAttribute('position').count,
                    visible: field._eFieldLines.visible,
                    draw: field._eFieldLines.geometry.drawRange.count,
                },
                b: {
                    capacity: field._bFieldLines.geometry.getAttribute('position').count,
                    visible: field._bFieldLines.visible,
                    draw: field._bFieldLines.geometry.drawRange.count,
                },
                flux: {
                    capacity: flux._fluxStreamlines.geometry.getAttribute('position').count,
                    visible: flux._fluxStreamlines.visible,
                    draw: flux._fluxStreamlines.geometry.drawRange.count,
                },
            };
        });
        expect(result).toEqual({
            e: { capacity: 16000, visible: false, draw: 0 },
            b: { capacity: 24000, visible: false, draw: 0 },
            flux: { capacity: 16000, visible: false, draw: 0 },
        });
    });

    test('Flux/E/B worker results paint atomically and the worker is disposed on scale exit', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await page.evaluate(() => {
            for (const id of ['toggle-flux-lines', 'toggle-e-field', 'toggle-b-field']) {
                const button = document.getElementById(id);
                if (button && !button.classList.contains('active')) button.click();
            }
            if (!window.__ftdCtx?.running) document.getElementById('btn-play')?.click();
        });
        await expect.poll(() => page.evaluate(() => {
            const viewport = window.__ftdCtx?.viewport;
            return [
                viewport?._fieldRenderer?._eFieldLines?.geometry?.drawRange?.count || 0,
                viewport?._fieldRenderer?._bFieldLines?.geometry?.drawRange?.count || 0,
                viewport?._fluxRenderer?._fluxStreamlines?.geometry?.drawRange?.count || 0,
            ].every((count) => count > 0);
        }), { timeout: 15_000 }).toBe(true);

        const beforeExit = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const sched = getScale0State().overlaySched;
            return {
                hasClient: !!sched?.streamlineWorkerClient,
                jobKinds: sched?.jobs?.slice(0, sched.jobCount).map((job) => job.kind),
            };
        });
        expect(beforeExit.hasClient).toBe(true);
        expect(beforeExit.jobKinds).toEqual([0, 1, 2]);

        await switchMode(page, 'particles');
        const afterExit = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const sched = getScale0State().overlaySched;
            return { active: sched?.active, hasClient: !!sched?.streamlineWorkerClient };
        });
        expect(afterExit).toEqual({ active: false, hasClient: false });
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});

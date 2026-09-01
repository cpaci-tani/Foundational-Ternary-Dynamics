// @ts-check
/**
 * Flux-volume activation/upload microbenchmark.
 *
 * NOT a gate — a deterministic measurement tool. Isolates the cost of
 * `Viewport.updateFluxVolume(volume, N)` for the synchronous dense browser
 * lattices and largest compact native support, independent of FPS noise /
 * worker setTimeout throttling. Dense supports above this matrix use the
 * cooperative release gate below instead of one blocking timed call.
 *
 * Why this path: at large L physics is off-thread (worker) and the GPU
 * upload is cadence-throttled (frame-sync.js: every 6th frame at L>96), so
 * the remaining per-upload main-thread cost is updateFluxVolume. The
 * threshold no longer changes spatial density. This benchmark uses threshold
 * zero so every received coordinate is written and catches regressions in the
 * complete-source scan/write budget.
 *
 * Run:   npx playwright test flux-upload-microbench.spec.js --reporter=list
 * Output: per-N ms/call table to stdout + attached JSON.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

// WebGL trace screencasts add compositor readback cost to the quantity this
// microbenchmark isolates. Keep this measurement untraced.
test.use({ trace: 'off' });

const CASES = [
    { label: 'dense-L17', latticeSize: 17, axisCount: 17, stride: 1, compact: false },
    { label: 'dense-L25', latticeSize: 25, axisCount: 25, stride: 1, compact: false },
    { label: 'dense-L33', latticeSize: 33, axisCount: 33, stride: 1, compact: false },
    { label: 'compact-L181-A53', latticeSize: 181, axisCount: 53, stride: 3, compact: true },
];
const ITERS = 120;        // timed calls per case (after a warmup call)

test.describe('flux-volume upload microbench', () => {
    test('updateFluxVolume cost vs lattice size', async ({ page }) => {
        test.setTimeout(120_000);
        await gotoAndReady(page);

        // The default mode is lattice; the Viewport is published on window.__ftdCtx.
        const ok = await page.evaluate(() => !!(window.__ftdCtx?.viewport?.updateFluxVolume));
        expect(ok, 'viewport.updateFluxVolume is reachable via window.__ftdCtx').toBe(true);

        const results = await page.evaluate(async ({ CASES, ITERS }) => {
            const vp = window.__ftdCtx.viewport;
            // Make the flux volume "shown" so the build path is exercised like prod.
            try { vp.toggleFluxVolume?.(true); } catch (_e) { /* ignore */ }

            // Build a faithful sparse field: a central Gaussian blob (like flux-pulse)
            // plus a small off-center lobe, so a realistic fraction of voxels exceed
            // FLUX_THRESHOLD and the write loop does real work.
            const buildVolume = (axisCount) => {
                const v = new Float64Array(axisCount ** 3);
                const c = axisCount / 2;
                const s2 = (axisCount * 0.12) ** 2;
                for (let z = 0; z < axisCount; z++) {
                    for (let y = 0; y < axisCount; y++) {
                        const base = z * axisCount * axisCount + y * axisCount;
                        for (let x = 0; x < axisCount; x++) {
                            const r2 = (x - c) ** 2 + (y - c) ** 2 + (z - c) ** 2;
                            const r2b = (x - c * 1.4) ** 2 + (y - c * 0.7) ** 2 + (z - c) ** 2;
                            v[base + x] = Math.exp(-r2 / (2 * s2)) + 0.4 * Math.exp(-r2b / (2 * s2 * 0.5));
                        }
                    }
                }
                return v;
            };

            const median = (arr) => {
                const a = arr.slice().sort((p, q) => p - q);
                return a[Math.floor(a.length / 2)];
            };

            const out = [];
            vp.setFluxThreshold(0);
            for (const config of CASES) {
                const v = buildVolume(config.axisCount);
                const frame = config.compact ? {
                    data: v,
                    latticeSize: config.latticeSize,
                    stride: config.stride,
                    origin: 0,
                    axisCount: config.axisCount,
                } : v;
                // Warmup: builds geometry for this complete source grid.
                vp.updateFluxVolume(frame, config.latticeSize);
                // Time ITERS calls.
                const samples = [];
                for (let i = 0; i < ITERS; i++) {
                    const t0 = performance.now();
                    vp.updateFluxVolume(frame, config.latticeSize);
                    samples.push(performance.now() - t0);
                }
                // Count drawn points for context (drawRange end).
                let drawn = null;
                try {
                    const g = vp._fluxRenderer?._fluxVolume?.geometry;
                    drawn = g?.drawRange?.count ?? null;
                } catch (_e) { /* ignore */ }
                out.push({
                    label: config.label,
                    latticeSize: config.latticeSize,
                    sourceVoxels: config.axisCount ** 3,
                    msMedian: +median(samples).toFixed(4),
                    msMean: +(samples.reduce((a, b) => a + b, 0) / samples.length).toFixed(4),
                    drawnPoints: drawn,
                });
            }
            return out;
        }, { CASES, ITERS });

        // Human-readable table to stdout.
        console.log('\n[flux-upload-microbench] updateFluxVolume ms/call:');
        console.log('  case                 source      ms(median)  ms(mean)   drawnPts');
        for (const r of results) {
            console.log(
                `  ${String(r.label).padEnd(20)} ${String(r.sourceVoxels).padEnd(11)} `
                + `${String(r.msMedian).padEnd(11)} ${String(r.msMean).padEnd(10)} ${r.drawnPoints}`,
            );
        }

        await test.info().attach('flux-upload-microbench', {
            body: JSON.stringify({ timestamp: new Date().toISOString(), iters: ITERS, results }, null, 2),
            contentType: 'application/json',
        });

        // Sanity only — this is a measurement, not a gate.
        expect(results.length).toBe(CASES.length);
        for (const r of results) expect(r.msMedian).toBeGreaterThan(0);
    });

    test('threshold zero sustains the complete L97 worker frame budget', async ({ page }, testInfo) => {
        test.skip(
            process.env.FTD_FLUX_VOLUME_RELEASE_GATE !== '1',
            'Set FTD_FLUX_VOLUME_RELEASE_GATE=1 for the sustained hardware gate',
        );
        testInfo.setTimeout(120_000);
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });

        await page.selectOption('#lattice-size', '97');
        await expect.poll(async () => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            return state.useFluxMock === true
                && state.fluxMock?.isWorker === true
                && state.fluxMock?.ready === true
                && Number(state.fluxMock?.latticeSize) === 97;
        }), { timeout: 90_000 }).toBe(true);
        await page.evaluate(() => {
            const threshold = document.getElementById('flux-threshold');
            threshold.value = '0';
            threshold.dispatchEvent(new Event('input', { bubbles: true }));
            const play = document.getElementById('btn-play');
            if (play?.getAttribute('data-paused') === 'true') play.click();
        });
        await expect.poll(() => page.evaluate(() => (
            window.__ftdCtx?.viewport?._fluxRenderer?._fluxVolume?.geometry?.drawRange?.count ?? 0
        )), { timeout: 30_000 }).toBe(97 ** 3);
        await expect.poll(() => page.evaluate(() => (
            window.__ftdCtx?.viewport?._fluxRenderer?._fluxVisibleCount ?? 0
        )), { timeout: 30_000 }).toBe(97 ** 3);
        await page.waitForTimeout(3_000);

        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const viewport = window.__ftdCtx.viewport;
            const gl = viewport.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            const webglRenderer = rendererInfo
                ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                : '';
            probe.startScale0UiAuditProbe();
            await new Promise((resolve) => setTimeout(resolve, 12_000));
            const sampled = await probe.stopScale0UiAuditProbe();
            const geometry = viewport._fluxRenderer._fluxVolume.geometry;
            return {
                ...sampled,
                webglRenderer,
                drawCount: geometry.drawRange.count,
                capacity: geometry.getAttribute('position').count,
            };
        });

        await testInfo.attach('flux-volume-L97-zero-threshold-release.json', {
            body: Buffer.from(JSON.stringify(report, null, 2)),
            contentType: 'application/json',
        });
        console.log('flux-volume L97 zero-threshold release', JSON.stringify(report));

        expect(report.drawCount).toBe(97 ** 3);
        expect(report.capacity).toBe(97 ** 3);
        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(report.webglRenderer, 'release gate exposes a WebGL renderer').not.toBe('');
            expect(report.webglRenderer, 'release gate does not certify software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        expect(report.frames.count).toBeGreaterThanOrEqual(600);
        expect(report.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
        expect(report.frames.p95Ms).toBeLessThanOrEqual(17);
        expect(report.frames.p99Ms).toBeLessThanOrEqual(20);
        expect(report.frames.intervalsOver33_4ms).toBe(0);
        expect(report.longTasks).toEqual([]);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});

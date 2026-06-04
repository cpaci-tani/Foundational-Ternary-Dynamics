// @ts-check
/**
 * Large-lattice flux-volume upload microbenchmark (Scale-0 L>65 cliff probe).
 *
 * NOT a gate — a deterministic measurement tool. Isolates the cost of
 * `Viewport.updateFluxVolume(volume, N)` (the per-upload main-thread work:
 * the maxFlux scan + the subsampled write loop) across a range of lattice
 * sizes, independent of FPS noise / worker setTimeout throttling.
 *
 * Why this path: at large L physics is off-thread (worker) and the GPU
 * upload is cadence-throttled (frame-sync.js: every 6th frame at L>96), so
 * the remaining per-upload main-thread cost is updateFluxVolume. Its render
 * write-loop is already `step`-decimated (1/2/4), but the maxFlux scan
 * (flux-renderer.js) iterates the full N^3 array every call. This bench
 * quantifies that before/after a fix.
 *
 * Run:   npx playwright test flux-upload-microbench.spec.js --reporter=list
 * Output: per-N ms/call table to stdout + attached JSON.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const SIZES = [49, 65, 97, 129];
const ITERS = 120;        // timed calls per size (after a warmup call)

test.describe('flux-volume upload microbench', () => {
    test('updateFluxVolume cost vs lattice size', async ({ page }) => {
        test.setTimeout(120_000);
        await gotoAndReady(page);

        // The default mode is lattice; the Viewport is published on window.__ftdCtx.
        const ok = await page.evaluate(() => !!(window.__ftdCtx?.viewport?.updateFluxVolume));
        expect(ok, 'viewport.updateFluxVolume is reachable via window.__ftdCtx').toBe(true);

        const results = await page.evaluate(async ({ SIZES, ITERS }) => {
            const vp = window.__ftdCtx.viewport;
            // Make the flux volume "shown" so the build path is exercised like prod.
            try { vp.toggleFluxVolume?.(true); } catch (_e) { /* ignore */ }

            // Build a faithful sparse field: a central Gaussian blob (like flux-pulse)
            // plus a small off-center lobe, so a realistic fraction of voxels exceed
            // FLUX_THRESHOLD and the write loop does real work.
            const buildVolume = (N) => {
                const v = new Float64Array(N * N * N);
                const c = N / 2;
                const s2 = (N * 0.12) * (N * 0.12);
                for (let z = 0; z < N; z++) {
                    for (let y = 0; y < N; y++) {
                        const base = z * N * N + y * N;
                        for (let x = 0; x < N; x++) {
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
            for (const N of SIZES) {
                const v = buildVolume(N);
                // Warmup: builds geometry for this N (rebuild happens on size change).
                vp.updateFluxVolume(v, N);
                // Time ITERS calls.
                const samples = [];
                for (let i = 0; i < ITERS; i++) {
                    const t0 = performance.now();
                    vp.updateFluxVolume(v, N);
                    samples.push(performance.now() - t0);
                }
                // Count drawn points for context (drawRange end).
                let drawn = null;
                try {
                    const g = vp._fluxRenderer?._fluxVolume?.geometry;
                    drawn = g?.drawRange?.count ?? null;
                } catch (_e) { /* ignore */ }
                out.push({
                    N,
                    voxels: N * N * N,
                    msMedian: +median(samples).toFixed(4),
                    msMean: +(samples.reduce((a, b) => a + b, 0) / samples.length).toFixed(4),
                    drawnPoints: drawn,
                });
            }
            return out;
        }, { SIZES, ITERS });

        // Human-readable table to stdout.
        console.log('\n[flux-upload-microbench] updateFluxVolume ms/call:');
        console.log('  N    voxels      ms(median)  ms(mean)   drawnPts');
        for (const r of results) {
            console.log(
                `  ${String(r.N).padEnd(4)} ${String(r.voxels).padEnd(11)} `
                + `${String(r.msMedian).padEnd(11)} ${String(r.msMean).padEnd(10)} ${r.drawnPoints}`,
            );
        }

        await test.info().attach('flux-upload-microbench', {
            body: JSON.stringify({ timestamp: new Date().toISOString(), iters: ITERS, results }, null, 2),
            contentType: 'application/json',
        });

        // Sanity only — this is a measurement, not a gate.
        expect(results.length).toBe(SIZES.length);
        for (const r of results) expect(r.msMedian).toBeGreaterThan(0);
    });
});

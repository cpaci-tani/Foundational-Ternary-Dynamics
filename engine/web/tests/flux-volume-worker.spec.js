// @ts-check
/**
 * Flux-volume off-thread pipeline against the REAL module worker.
 *
 * tests/flux-volume-worker.node.test.mjs pins the kernel to the synchronous
 * renderer path and exercises the glue with an in-process stand-in. This spec
 * closes the remaining gap: the browser's own Worker, module loading inside
 * it, and structured-clone transfer. Two renderers receive the same fields,
 * one through the worker and one forced onto the cooperative main-thread
 * path, and every output array must match bit for bit.
 */
import { test, expect } from '@playwright/test';

async function installImportMap(page) {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    await page.setContent(`
        <script type="importmap">
        {"imports":{"three":"/js/vendor/three/build/three.module.js","three/addons/":"/js/vendor/three/examples/jsm/"}}
        </script>
    `);
}

// Shared memory is the normal mode; transferred buffers serve pages that are
// not cross-origin isolated. Both must match the cooperative path.
for (const shared of [true, false]) test(`worker (${shared ? 'shared memory' : 'transferred buffers'}) and cooperative paths produce identical flux-volume buffers`, async ({ page }) => {
    const pageErrors = [];
    page.on('pageerror', (error) => pageErrors.push(String(error)));
    await installImportMap(page);

    const result = await page.evaluate(async (shared) => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import('/js/viewport/flux-renderer.js?flux-volume-worker-test=1');
        const N = 59; // 205,379 sources: above the off-thread threshold.
        const inSphere = (x, y, z) => x * x + y * y + z * z <= 1;
        const make = (useWorker) => {
            // (shared is read from the enclosing evaluate argument)
            const renderer = new ViewportFluxRenderer({
                scene: new THREE.Scene(),
                latticeSize: N,
                halfN: N / 2,
                boundaryShape: 'sphere',
                insideBoundary: inSphere,
                applyScenarioScale: () => {},
                buildStreamlineMesh: () => null,
                writeStreamlinesIntoMesh: () => {},
            });
            renderer.setFluxVolumeWorkerEnabled(useWorker);
            renderer.setFluxVolumeSharedMemory(shared);
            renderer.setFluxOrganic(true);
            renderer.setFluxThreshold(0.05);
            return renderer;
        };
        const field = (seed, scale) => {
            const out = new Float32Array(N ** 3);
            let state = (seed * 2654435761) >>> 0;
            const c = N / 2;
            const s2 = (N * 0.14) ** 2;
            let i = 0;
            for (let z = 0; z < N; z++) {
                for (let y = 0; y < N; y++) {
                    for (let x = 0; x < N; x++, i++) {
                        state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
                        const r2 = (x - c) ** 2 + (y - c) ** 2 + (z - c) ** 2;
                        out[i] = scale * (Math.exp(-r2 / (2 * s2)) + 0.01 * (state / 4294967296));
                    }
                }
            }
            // Strongest energy outside the inscribed sphere.
            for (let k = 0; k < 8; k++) out[((k >> 2) * N + ((k >> 1) & 1)) * N + (k & 1)] = 4 * scale;
            return out;
        };
        const settle = (renderer) => new Promise((resolve, reject) => {
            const started = performance.now();
            const poll = () => {
                if (!renderer._fluxAsyncJob && !renderer._fluxPendingFrame) resolve(undefined);
                else if (performance.now() - started > 15_000) reject(new Error('flux update did not settle'));
                else requestAnimationFrame(poll);
            };
            poll();
        });
        const sameBits = (a, b) => {
            if (a.length !== b.length || a.constructor !== b.constructor) return false;
            const x = new Uint8Array(a.buffer, a.byteOffset, a.byteLength);
            const y = new Uint8Array(b.buffer, b.byteOffset, b.byteLength);
            for (let i = 0; i < x.length; i++) if (x[i] !== y[i]) return false;
            return true;
        };
        const compare = (a, b) => {
            const ga = a._fluxVolume.geometry;
            const gb = b._fluxVolume.geometry;
            const attribute = (name) => sameBits(ga.getAttribute(name).array, gb.getAttribute(name).array);
            return {
                colour: attribute('particleColor'),
                size: attribute('size'),
                visibility: attribute('particleVisibility'),
                position: attribute('position'),
                sourcePosition: attribute('sourcePosition'),
                activation: sameBits(a._fluxActivation, b._fluxActivation),
                visibleCount: a._fluxVisibleCount === b._fluxVisibleCount,
                normaliser: a._fluxMaxDecay === b._fluxMaxDecay,
                drawRange: ga.drawRange.count === gb.drawRange.count && ga.drawRange.count === N ** 3,
            };
        };

        const viaWorker = make(true);
        const cooperative = make(false);
        const particles = {
            positions: new Float32Array([29.5, 29.5, 29.5, 20.2, 31.9, 27.1]),
            colors: new Float32Array([0.9, 0.1, 0.1, 0.1, 0.9, 0.1]),
            locked: new Uint8Array([1, 0]),
            count: 2,
        };
        const frames = [];
        for (const [seed, scale] of [[1, 1], [2, 0.3], [3, 2]]) {
            const density = field(seed, scale);
            viaWorker.updateFluxVolume(density, N, particles);
            cooperative.updateFluxVolume(density, N, particles);
            await Promise.all([settle(viaWorker), settle(cooperative)]);
            frames.push(compare(viaWorker, cooperative));
        }

        // Burst: the worker computes the first and the last frame only. The
        // cooperative reference is fed exactly those two.
        const burst = [field(4, 1.5), field(5, 0.2), field(6, 0.8)];
        for (const density of burst) viaWorker.updateFluxVolume(density, N, particles);
        cooperative.updateFluxVolume(burst[0], N, particles);
        await settle(cooperative);
        cooperative.updateFluxVolume(burst[2], N, particles);
        await Promise.all([settle(viaWorker), settle(cooperative)]);
        frames.push(compare(viaWorker, cooperative));

        const visible = viaWorker._fluxVisibleCount;
        const jittered = !sameBits(
            viaWorker._fluxVolume.geometry.getAttribute('position').array,
            viaWorker._fluxVolume.geometry.getAttribute('sourcePosition').array,
        );
        const summary = {
            frames,
            visible,
            jittered,
            workerAvailable: viaWorker._fluxWorkerClient?.available === true,
            workerComputeMs: viaWorker._fluxWorkerComputeMs,
            cooperativeHasNoWorker: cooperative._fluxWorkerClient === null,
            cornerHidden: viaWorker._fluxVolume.geometry.getAttribute('particleVisibility').array[0] === 0,
            sharedArrays: Object.prototype.toString.call(
                viaWorker._fluxVolume.geometry.getAttribute('particleColor').array.buffer,
            ) === '[object SharedArrayBuffer]',
            isolated: self.crossOriginIsolated === true,
        };
        viaWorker.dispose();
        cooperative.dispose();
        return summary;
    }, shared);

    expect(pageErrors).toEqual([]);
    expect(result.workerAvailable, 'the module worker loaded and answered').toBe(true);
    expect(result.isolated, 'the test page is cross-origin isolated, so shared memory is really exercised').toBe(true);
    expect(result.sharedArrays, 'the arrays on screen are of the requested kind').toBe(shared);
    expect(result.workerComputeMs).toBeGreaterThan(0);
    expect(result.cooperativeHasNoWorker).toBe(true);
    expect(result.jittered, 'Organic positions were written on the worker path').toBe(true);
    expect(result.cornerHidden, 'shaped boundary clips the hot corner').toBe(true);
    expect(result.visible).toBeGreaterThan(0);
    for (const [index, frame] of result.frames.entries()) {
        for (const [name, same] of Object.entries(frame)) {
            expect(same, `frame ${index}: ${name}`).toBe(true);
        }
    }
});

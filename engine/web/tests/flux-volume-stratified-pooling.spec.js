// @ts-check
import { test, expect } from '@playwright/test';

async function installImportMap(page) {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    await page.setContent(`
        <script type="importmap">
        {"imports":{"three":"/js/vendor/three/build/three.module.js","three/addons/":"/js/vendor/three/examples/jsm/"}}
        </script>
    `);
}

test('threshold keeps every source coordinate available without spatial pooling', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import(
            '/js/viewport/flux-renderer.js?stratified-pool-test=1'
        );
        const makeRenderer = (N, boundaryShape = 'cube', insideBoundary = () => true) => {
            const renderer = new ViewportFluxRenderer({
                scene: new THREE.Scene(),
                latticeSize: N,
                halfN: N / 2,
                boundaryShape,
                insideBoundary,
                applyScenarioScale: () => {},
                buildStreamlineMesh: () => null,
                writeStreamlinesIntoMesh: () => {},
            });
            renderer._fluxOrganic = false;
            return renderer;
        };
        const points = (renderer) => {
            const geometry = renderer._fluxVolume.geometry;
            const drawCount = geometry.drawRange.count;
            const position = geometry.getAttribute('position').array;
            const source = geometry.getAttribute('sourcePosition').array;
            const visibility = geometry.getAttribute('particleVisibility').array;
            const triples = (array) => {
                const out = [];
                for (let i = 0; i < drawCount; i++) {
                    if (visibility[i] < 0.5) continue;
                    out.push(Array.from(array.slice(i * 3, i * 3 + 3)));
                }
                return out.sort((a, b) => a.join(',').localeCompare(b.join(',')));
            };
            return {
                count: renderer._fluxVisibleCount,
                drawCount,
                capacity: geometry.getAttribute('position').count,
                position: triples(position),
                source: triples(source),
            };
        };

        const denseN = 49;
        const dense = new Float32Array(denseN ** 3);
        const denseAt = (x, y, z) => z * denseN * denseN + y * denseN + x;
        dense[denseAt(24, 24, 24)] = 1;
        dense[denseAt(37, 8, 21)] = 0.8;
        const denseRenderer = makeRenderer(denseN);
        denseRenderer.setFluxThreshold(0.5);
        denseRenderer.updateFluxVolume(dense, denseN);

        // Both values occupy the same x-stratum. The larger one is outside
        // this shaped boundary and must not suppress the drawable value.
        const shaped = new Float32Array(denseN ** 3);
        shaped[denseAt(0, 24, 24)] = 10;
        shaped[denseAt(3, 24, 24)] = 1;
        const shapedRenderer = makeRenderer(
            denseN,
            'custom-test-shape',
            (nx) => nx > -0.9,
        );
        shapedRenderer.setFluxThreshold(0.5);
        shapedRenderer.updateFluxVolume(shaped, denseN);

        const compactN = 105;
        const axisCount = 53;
        const compact = new Float32Array(axisCount ** 3);
        const compactAt = (x, y, z) => z * axisCount * axisCount + y * axisCount + x;
        compact[compactAt(26, 26, 26)] = 1;
        compact[compactAt(17, 31, 44)] = 0.8;
        const compactRenderer = makeRenderer(compactN);
        compactRenderer.setFluxThreshold(0.5);
        compactRenderer.updateFluxVolume({
            data: compact,
            latticeSize: compactN,
            stride: 2,
            axisCount,
            origin: 0,
        }, compactN);

        return {
            dense: points(denseRenderer),
            shaped: points(shapedRenderer),
            compact: points(compactRenderer),
        };
    });

    expect(result.dense.count).toBe(2);
    expect(result.dense.drawCount).toBe(49 ** 3);
    expect(result.dense.capacity).toBe(49 ** 3);
    expect(result.dense.position).toEqual(result.dense.source);
    expect(result.dense.source).toEqual([
        [24.5, 24.5, 24.5],
        [37.5, 8.5, 21.5],
    ].sort((a, b) => a.join(',').localeCompare(b.join(','))));

    expect(result.shaped.count).toBe(1);
    expect(result.shaped.drawCount).toBe(49 ** 3);
    expect(result.shaped.source).toEqual([[3.5, 24.5, 24.5]]);

    expect(result.compact.count).toBe(2);
    expect(result.compact.drawCount).toBe(53 ** 3);
    expect(result.compact.capacity).toBe(53 ** 3);
    expect(result.compact.position).toEqual(result.compact.source);
    expect(result.compact.source).toEqual([
        [52.5, 52.5, 52.5],
        [34.5, 62.5, 88.5],
    ].sort((a, b) => a.join(',').localeCompare(b.join(','))));
    expect(result.dense.capacity).toBe(49 ** 3);
    expect(result.compact.capacity).toBe(53 ** 3);
});

test('large dense support applies the same threshold semantics cooperatively', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import(
            '/js/viewport/flux-renderer.js?large-cooperative-threshold-test=1'
        );
        const N = 59; // 205,379 voxels: exercises the cooperative path.
        const at = (x, y, z) => (z * N + y) * N + x;
        const renderer = new ViewportFluxRenderer({
            scene: new THREE.Scene(),
            latticeSize: N,
            halfN: N / 2,
            boundaryShape: 'cube',
            insideBoundary: () => true,
            applyScenarioScale: () => {},
            buildStreamlineMesh: () => null,
            writeStreamlinesIntoMesh: () => {},
        });
        renderer.setFluxOrganic(false);
        renderer.setFluxThreshold(0.5);
        const density = new Float32Array(N ** 3);
        density[at(29, 29, 29)] = 1;
        density[at(42, 8, 21)] = 0.8;
        renderer.updateFluxVolume(density, N);

        await new Promise((resolve, reject) => {
            const started = performance.now();
            const poll = () => {
                if (!renderer._fluxAsyncJob && !renderer._fluxPendingFrame) {
                    resolve();
                } else if (performance.now() - started > 5_000) {
                    reject(new Error('cooperative flux update did not settle'));
                } else {
                    requestAnimationFrame(poll);
                }
            };
            poll();
        });

        const geometry = renderer._fluxVolume.geometry;
        const visibility = geometry.getAttribute('particleVisibility').array;
        const source = geometry.getAttribute('sourcePosition').array;
        const visibleSources = [];
        for (let i = 0; i < geometry.drawRange.count; i++) {
            if (visibility[i] < 0.5) continue;
            visibleSources.push(Array.from(source.slice(i * 3, i * 3 + 3)));
        }
        return {
            drawCount: geometry.drawRange.count,
            capacity: geometry.getAttribute('position').count,
            visibleCount: renderer._fluxVisibleCount,
            visibleSources,
        };
    });

    expect(result.drawCount).toBe(59 ** 3);
    expect(result.capacity).toBe(59 ** 3);
    expect(result.visibleCount).toBe(2);
    expect(result.visibleSources).toEqual([
        [42.5, 8.5, 21.5],
        [29.5, 29.5, 29.5],
    ]);
});

test('zero threshold reveals every available lattice sample at every point size', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import(
            '/js/viewport/flux-renderer.js?full-lattice-inspection-test=1'
        );
        const makeRenderer = (N, pointScale = 1) => {
            const renderer = new ViewportFluxRenderer({
                scene: new THREE.Scene(),
                latticeSize: N,
                halfN: N / 2,
                boundaryShape: 'cube',
                insideBoundary: () => true,
                applyScenarioScale: () => {},
                buildStreamlineMesh: () => null,
                writeStreamlinesIntoMesh: () => {},
            });
            renderer.setFluxOrganic(false);
            renderer.setFluxPointScale(pointScale);
            renderer.setFluxThreshold(0);
            return renderer;
        };
        const snapshot = (renderer) => {
            const geometry = renderer._fluxVolume.geometry;
            const drawCount = geometry.drawRange.count;
            const source = geometry.getAttribute('sourcePosition').array;
            const colors = geometry.getAttribute('particleColor').array;
            const sizes = geometry.getAttribute('size').array;
            const visibilities = geometry.getAttribute('particleVisibility').array;
            return {
                count: renderer._fluxVisibleCount,
                drawCount,
                capacity: geometry.getAttribute('position').count,
                firstSource: Array.from(source.slice(0, 3)),
                lastSource: Array.from(source.slice((drawCount - 1) * 3, drawCount * 3)),
                firstColor: Array.from(colors.slice(0, 3)),
                firstSize: sizes[0],
                lastSize: sizes[drawCount - 1],
                minSize: Math.min(...sizes.slice(0, drawCount)),
                maxSize: Math.max(...sizes.slice(0, drawCount)),
                minVisibility: Math.min(...visibilities.slice(0, drawCount)),
                maxVisibility: Math.max(...visibilities.slice(0, drawCount)),
            };
        };

        const denseN = 33;
        const dense = new Float32Array(denseN ** 3);
        dense[16 * denseN * denseN + 16 * denseN + 16] = 1;
        const denseInspections = {};
        for (const pointScale of [0.1, 1, 3]) {
            const renderer = makeRenderer(denseN, pointScale);
            renderer.updateFluxVolume(dense, denseN);
            denseInspections[String(pointScale)] = snapshot(renderer);
        }

        const nearZeroRamp = {};
        for (const threshold of [0, 0.00025, 0.0005, 0.00075, 0.001]) {
            const renderer = makeRenderer(denseN, 1);
            renderer.setFluxThreshold(threshold);
            renderer.updateFluxVolume(dense, denseN);
            nearZeroRamp[String(threshold)] = snapshot(renderer);
        }

        // Above zero, threshold is relative to the current peak and the normal
        // bounded production path resumes. Values exactly at the cutoff stay.
        const relativeRenderer = makeRenderer(denseN, 1);
        relativeRenderer.setFluxThreshold(0.5);
        const relative = new Float32Array(denseN ** 3);
        relative[0] = Math.sqrt(0.49);
        relative[16 * denseN * denseN + 16 * denseN + 16] = 0.72;
        relative[relative.length - 1] = 1;
        relativeRenderer.updateFluxVolume(relative, denseN);
        const relativeGeometry = relativeRenderer._fluxVolume.geometry;
        const relativeVisibility = relativeGeometry.getAttribute('particleVisibility').array;
        const relativeSources = relativeGeometry.getAttribute('sourcePosition').array;
        const visibleSources = [];
        for (let i = 0; i < relativeGeometry.drawRange.count; i++) {
            if (relativeVisibility[i] < 0.5) continue;
            visibleSources.push(...relativeSources.slice(i * 3, i * 3 + 3));
        }
        const relativeSnapshot = {
            count: relativeRenderer._fluxVisibleCount,
            drawCount: relativeGeometry.drawRange.count,
            capacity: relativeGeometry.getAttribute('position').count,
            sources: visibleSources,
        };

        const compactN = 97;
        const axisCount = 5;
        const compactRenderer = makeRenderer(compactN);
        compactRenderer.updateFluxVolume({
            data: new Float32Array(axisCount ** 3),
            latticeSize: compactN,
            stride: 24,
            axisCount,
            origin: 0,
        }, compactN);

        return {
            denseInspections,
            relativeSnapshot,
            compactInspection: snapshot(compactRenderer),
            nearZeroRamp,
        };
    });

    for (const pointScale of ['0.1', '1', '3']) {
        const inspection = result.denseInspections[pointScale];
        expect(inspection.count).toBe(33 ** 3);
        expect(inspection.drawCount).toBe(33 ** 3);
        expect(inspection.capacity).toBe(33 ** 3);
        expect(inspection.firstSource).toEqual([0.5, 0.5, 0.5]);
        expect(inspection.lastSource).toEqual([32.5, 32.5, 32.5]);
        expect(inspection.firstColor[0]).toBeGreaterThanOrEqual(0.159);
        expect(inspection.firstColor[1]).toBeGreaterThanOrEqual(0.349);
        expect(inspection.firstColor[2]).toBeGreaterThanOrEqual(0.549);
        expect(inspection.firstSize).toBe(1);
        expect(inspection.lastSize).toBe(1);
        expect(inspection.minSize).toBe(1);
        expect(inspection.minVisibility).toBe(1);
        expect(inspection.maxVisibility).toBe(1);
    }
    expect(result.denseInspections['0.1'].maxSize).toBeCloseTo(1.9, 5);
    expect(result.denseInspections['1'].maxSize).toBe(10);
    expect(result.denseInspections['3'].maxSize).toBe(28);

    expect(result.relativeSnapshot.count).toBe(2);
    expect(result.relativeSnapshot.drawCount).toBe(33 ** 3);
    expect(result.relativeSnapshot.capacity).toBe(33 ** 3);
    expect(result.relativeSnapshot.sources).toEqual([
        16.5, 16.5, 16.5,
        32.5, 32.5, 32.5,
    ]);

    expect(result.compactInspection.count).toBe(5 ** 3);
    expect(result.compactInspection.drawCount).toBe(5 ** 3);
    expect(result.compactInspection.capacity).toBe(5 ** 3);
    expect(result.compactInspection.firstSource).toEqual([0.5, 0.5, 0.5]);
    expect(result.compactInspection.lastSource).toEqual([96.5, 96.5, 96.5]);
    expect(result.compactInspection.firstSize).toBe(1);
    expect(result.compactInspection.lastSize).toBe(1);

    const ramp = [0, 0.00025, 0.0005, 0.00075, 0.001]
        .map((threshold) => result.nearZeroRamp[String(threshold)]);
    expect(ramp.map((entry) => entry.capacity)).toEqual(Array(5).fill(33 ** 3));
    expect(ramp.map((entry) => entry.count)).toEqual([
        33 ** 3,
        27,
        27,
        27,
        27,
    ]);
    expect(ramp.map((entry) => entry.minVisibility)).toEqual([1, 0, 0, 0, 0]);
    expect(ramp.every((entry) => entry.maxVisibility === 1)).toBe(true);
});

test('activation uses manifested state plus surrounding energy for threshold, size, and colour', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import(
            '/js/viewport/flux-renderer.js?activation-semantics-test=1'
        );
        const N = 7;
        const at = (x, y, z) => (z * N + y) * N + x;
        const makeRenderer = (threshold) => {
            const renderer = new ViewportFluxRenderer({
                scene: new THREE.Scene(),
                latticeSize: N,
                halfN: N / 2,
                boundaryShape: 'cube',
                insideBoundary: () => true,
                applyScenarioScale: () => {},
                buildStreamlineMesh: () => null,
                writeStreamlinesIntoMesh: () => {},
            });
            renderer.setFluxOrganic(false);
            renderer.setFluxThreshold(threshold);
            return renderer;
        };
        const snapshot = (renderer) => {
            const geometry = renderer._fluxVolume.geometry;
            const drawCount = geometry.drawRange.count;
            const sources = geometry.getAttribute('sourcePosition').array;
            const sizes = geometry.getAttribute('size').array;
            const colors = geometry.getAttribute('particleColor').array;
            const visibility = geometry.getAttribute('particleVisibility').array;
            const rows = [];
            for (let i = 0; i < drawCount; i++) {
                if (visibility[i] < 0.5) continue;
                rows.push({
                    source: Array.from(sources.slice(i * 3, i * 3 + 3)),
                    size: sizes[i],
                    color: Array.from(colors.slice(i * 3, i * 3 + 3)),
                });
            }
            return {
                count: renderer._fluxVisibleCount,
                drawCount,
                capacity: geometry.getAttribute('position').count,
                rows,
            };
        };

        const energy = new Float32Array(N ** 3);
        energy[at(3, 3, 3)] = 1;
        const haloRenderer = makeRenderer(0.02);
        haloRenderer.updateFluxVolume(energy, N);
        const halo = snapshot(haloRenderer);

        const peakOnlyRenderer = makeRenderer(0.05);
        peakOnlyRenderer.updateFluxVolume(energy, N);
        const peakOnly = snapshot(peakOnlyRenderer);

        const stateRenderer = makeRenderer(0.5);
        stateRenderer.updateFluxVolume(
            new Float32Array(N ** 3),
            N,
            { positions: new Float32Array([3.5, 3.5, 3.5]), count: 1 },
        );
        const manifested = snapshot(stateRenderer);

        return { halo, peakOnly, manifested };
    });

    expect(result.halo.capacity).toBe(7 ** 3);
    expect(result.halo.drawCount).toBe(7 ** 3);
    expect(result.halo.count).toBe(27);
    const center = result.halo.rows.find((row) => row.source.join(',') === '3.5,3.5,3.5');
    const neighbour = result.halo.rows.find((row) => row.source.join(',') === '2.5,3.5,3.5');
    expect(center).toBeTruthy();
    expect(neighbour).toBeTruthy();
    expect(center.size).toBeGreaterThan(neighbour.size);
    expect(center.color[0]).toBeGreaterThan(neighbour.color[0]);

    expect(result.peakOnly.count).toBe(1);
    expect(result.peakOnly.rows[0].source).toEqual([3.5, 3.5, 3.5]);
    expect(result.manifested.count).toBe(1);
    expect(result.manifested.rows[0].source).toEqual([3.5, 3.5, 3.5]);
});

test('frame sync reuses the exact manifested-state frame for flux activation', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const { syncRenderableData } = await import(
            '/js/scales/scale0/runtime/frame-sync.js?flux-state-frame-test=1'
        );
        const particleData = {
            positions: new Float32Array([1.5, 1.5, 1.5]),
            count: 1,
        };
        const volume = new Float32Array(27);
        const scale0 = {
            getScale0ParticleFrame: () => particleData,
            getScale0FluxVolume: () => volume,
        };
        const bridge = { latticeSize: 3, capabilities: { scale0 } };
        const state = {
            latticeNeedsUpload: true,
            useFluxMock: false,
            fieldFlags: { showConfinement: false },
        };
        let appliedParticle = null;
        let appliedFlux = null;
        const adapter = {
            raw: null,
            applyParticleFrame: (frame) => { appliedParticle = frame; },
            isFluxVolumeVisible: () => true,
            applyFluxVolume: (data, latticeSize, frame) => {
                appliedFlux = { data, latticeSize, frame };
            },
            isFluxSliceVisible: () => false,
        };
        const latticeSize = syncRenderableData({ bridge, frameCount: 0 }, state, adapter);
        return {
            latticeSize,
            sameRenderableFrame: appliedFlux?.frame === appliedParticle,
            statePositionsReused: appliedFlux?.frame?.positions === particleData.positions,
            volumeForwarded: appliedFlux?.data === volume,
            uploadCleared: state.latticeNeedsUpload === false,
        };
    });

    expect(result).toEqual({
        latticeSize: 3,
        sameRenderableFrame: true,
        statePositionsReused: true,
        volumeForwarded: true,
        uploadCleared: true,
    });
});

test('zero frames retain peak history while an authoritative visual reset clears it', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import(
            '/js/viewport/flux-renderer.js?flux-reset-test=1'
        );
        const { createScale0ViewportAdapter } = await import(
            '/js/scales/scale0/viewport-adapter.js?flux-reset-test=1'
        );
        const N = 25;
        const renderer = new ViewportFluxRenderer({
            scene: new THREE.Scene(),
            latticeSize: N,
            halfN: N / 2,
            boundaryShape: 'cube',
            insideBoundary: () => true,
            applyScenarioScale: () => {},
            buildStreamlineMesh: () => null,
            writeStreamlinesIntoMesh: () => {},
        });
        renderer._fluxOrganic = false;
        const at = (x, y, z) => z * N * N + y * N + x;
        const high = new Float32Array(N ** 3);
        const zero = new Float32Array(N ** 3);
        const low = new Float32Array(N ** 3);
        high[at(12, 12, 12)] = 100;
        low[at(12, 12, 12)] = 1;

        renderer.updateFluxVolume(high, N);
        const afterHigh = renderer._fluxMaxDecay;
        renderer.updateFluxVolume(zero, N);
        const afterZero = renderer._fluxMaxDecay;
        const zeroVisibleCount = renderer._fluxVisibleCount;
        renderer.updateFluxVolume(low, N);
        const lowBeforeReset = renderer._fluxMaxDecay;

        const adapter = createScale0ViewportAdapter({
            resetFluxNormalization: () => renderer.resetFluxNormalization(),
        });
        adapter.clearScaleVisuals();
        const afterReset = renderer._fluxMaxDecay;
        renderer.updateFluxVolume(low, N);
        const lowAfterReset = renderer._fluxMaxDecay;
        const lowVisibleCount = renderer._fluxVisibleCount;

        renderer.updateFluxVolume(high, N);
        renderer.onLatticeSizeChanged(27, 13.5);
        const afterResize = renderer._fluxMaxDecay;
        return {
            afterHigh,
            afterZero,
            zeroVisibleCount,
            lowBeforeReset,
            afterReset,
            lowAfterReset,
            lowVisibleCount,
            afterResize,
        };
    });

    expect(result.afterHigh).toBe(100);
    expect(result.afterZero).toBe(100);
    expect(result.zeroVisibleCount).toBe(0);
    expect(result.lowBeforeReset).toBeCloseTo(98.5, 8);
    expect(result.afterReset).toBe(0);
    expect(result.lowAfterReset).toBe(1);
    expect(result.lowVisibleCount).toBe(27);
    expect(result.afterResize).toBe(0);
});

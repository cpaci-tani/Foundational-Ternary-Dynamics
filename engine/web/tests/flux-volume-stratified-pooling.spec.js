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

test('uniform max pooling retains dense and compact off-stride source coordinates', async ({ page }) => {
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
            const count = geometry.drawRange.count;
            const position = geometry.getAttribute('position').array;
            const source = geometry.getAttribute('sourcePosition').array;
            const triples = (array) => {
                const out = [];
                for (let i = 0; i < count; i++) {
                    out.push(Array.from(array.slice(i * 3, i * 3 + 3)));
                }
                return out.sort((a, b) => a.join(',').localeCompare(b.join(',')));
            };
            return {
                count,
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
        shapedRenderer.updateFluxVolume(shaped, denseN);

        const compactN = 105;
        const axisCount = 53;
        const compact = new Float32Array(axisCount ** 3);
        const compactAt = (x, y, z) => z * axisCount * axisCount + y * axisCount + x;
        compact[compactAt(26, 26, 26)] = 1;
        compact[compactAt(17, 31, 44)] = 0.8;
        const compactRenderer = makeRenderer(compactN);
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
    expect(result.dense.capacity).toBe(1728);
    expect(result.dense.position).toEqual(result.dense.source);
    expect(result.dense.source).toEqual([
        [24.5, 24.5, 24.5],
        [37.5, 8.5, 21.5],
    ].sort((a, b) => a.join(',').localeCompare(b.join(','))));

    expect(result.shaped.count).toBe(1);
    expect(result.shaped.source).toEqual([[3.5, 24.5, 24.5]]);

    expect(result.compact.count).toBe(2);
    expect(result.compact.capacity).toBe(1728);
    expect(result.compact.position).toEqual(result.compact.source);
    expect(result.compact.source).toEqual([
        [52.5, 52.5, 52.5],
        [34.5, 62.5, 88.5],
    ].sort((a, b) => a.join(',').localeCompare(b.join(','))));
    expect(result.dense.count).toBeLessThanOrEqual(1728);
    expect(result.compact.count).toBeLessThanOrEqual(1728);
});

test('minimum point size plus zero threshold reveals every available lattice sample', async ({ page }) => {
    await installImportMap(page);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import(
            '/js/viewport/flux-renderer.js?full-lattice-inspection-test=1'
        );
        const makeRenderer = (N) => {
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
            renderer.setFluxPointScale(0.1);
            renderer.setFluxThreshold(0);
            return renderer;
        };
        const snapshot = (renderer) => {
            const geometry = renderer._fluxVolume.geometry;
            const count = geometry.drawRange.count;
            const source = geometry.getAttribute('sourcePosition').array;
            const colors = geometry.getAttribute('particleColor').array;
            const sizes = geometry.getAttribute('size').array;
            return {
                count,
                capacity: geometry.getAttribute('position').count,
                firstSource: Array.from(source.slice(0, 3)),
                lastSource: Array.from(source.slice((count - 1) * 3, count * 3)),
                firstColor: Array.from(colors.slice(0, 3)),
                firstSize: sizes[0],
                lastSize: sizes[count - 1],
            };
        };

        const denseN = 33;
        const denseRenderer = makeRenderer(denseN);
        denseRenderer.updateFluxVolume(new Float32Array(denseN ** 3), denseN);
        const denseInspection = snapshot(denseRenderer);

        // Moving either slider away from its minimum restores the bounded
        // production path and does not make zero-flux samples visible.
        denseRenderer.setFluxThreshold(0.0001);
        denseRenderer.updateFluxVolume(new Float32Array(denseN ** 3), denseN);
        const normal = {
            count: denseRenderer._fluxVolume.geometry.drawRange.count,
            capacity: denseRenderer._fluxVolume.geometry.getAttribute('position').count,
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
            denseInspection,
            normal,
            compactInspection: snapshot(compactRenderer),
        };
    });

    expect(result.denseInspection.count).toBe(33 ** 3);
    expect(result.denseInspection.capacity).toBe(33 ** 3);
    expect(result.denseInspection.firstSource).toEqual([0.5, 0.5, 0.5]);
    expect(result.denseInspection.lastSource).toEqual([32.5, 32.5, 32.5]);
    expect(result.denseInspection.firstColor[0]).toBeGreaterThanOrEqual(0.159);
    expect(result.denseInspection.firstColor[1]).toBeGreaterThanOrEqual(0.349);
    expect(result.denseInspection.firstColor[2]).toBeGreaterThanOrEqual(0.549);
    expect(result.denseInspection.firstSize).toBe(1);
    expect(result.denseInspection.lastSize).toBe(1);

    expect(result.normal.count).toBe(0);
    expect(result.normal.capacity).toBe(12 ** 3);

    expect(result.compactInspection.count).toBe(5 ** 3);
    expect(result.compactInspection.capacity).toBe(5 ** 3);
    expect(result.compactInspection.firstSource).toEqual([0.5, 0.5, 0.5]);
    expect(result.compactInspection.lastSource).toEqual([96.5, 96.5, 96.5]);
    expect(result.compactInspection.firstSize).toBe(1);
    expect(result.compactInspection.lastSize).toBe(1);
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
        const zeroDrawCount = renderer._fluxVolume.geometry.drawRange.count;
        renderer.updateFluxVolume(low, N);
        const lowBeforeReset = renderer._fluxMaxDecay;

        const adapter = createScale0ViewportAdapter({
            resetFluxNormalization: () => renderer.resetFluxNormalization(),
        });
        adapter.clearScaleVisuals();
        const afterReset = renderer._fluxMaxDecay;
        renderer.updateFluxVolume(low, N);
        const lowAfterReset = renderer._fluxMaxDecay;
        const lowDrawCount = renderer._fluxVolume.geometry.drawRange.count;

        renderer.updateFluxVolume(high, N);
        renderer.onLatticeSizeChanged(27, 13.5);
        const afterResize = renderer._fluxMaxDecay;
        return {
            afterHigh,
            afterZero,
            zeroDrawCount,
            lowBeforeReset,
            afterReset,
            lowAfterReset,
            lowDrawCount,
            afterResize,
        };
    });

    expect(result.afterHigh).toBe(100);
    expect(result.afterZero).toBe(100);
    expect(result.zeroDrawCount).toBe(0);
    expect(result.lowBeforeReset).toBeCloseTo(98.5, 8);
    expect(result.afterReset).toBe(0);
    expect(result.lowAfterReset).toBe(1);
    expect(result.lowDrawCount).toBe(1);
    expect(result.afterResize).toBe(0);
});

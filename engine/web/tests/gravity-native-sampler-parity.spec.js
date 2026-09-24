// @ts-check
import { test, expect } from '@playwright/test';
import { bootDashboard } from './_helpers.js';

// Runs synchronously inside either the direct Emscripten module or the
// dashboard's classic worker. The oracle owns the exported flux volume before
// any other Embind call and derives every expected sample independently in JS.
function checkGravityNativeSamplers({ threaded }) {
    const wasm = threaded ? mod : window._ftdBridge._module;
    const constants = wasm.getConstants();
    const HORIZON_CLAMP = constants.LATENCY_HORIZON_CLAMP;
    const G_N = constants.G_N;
    // Canonical engine/include/ftd/constants.h::GRAD_TIER2_SCALE. It is kept
    // explicit because getConstants deliberately does not export stencil
    // implementation coefficients.
    const GRAD_TIER2_SCALE = 0.25;
    if (HORIZON_CLAMP !== 0.998 || G_N !== 0.01) {
        throw new Error(`Unexpected native constants: clamp=${HORIZON_CLAMP}, G_N=${G_N}`);
    }

    const f32 = Math.fround;
    const floatBits = array => new Uint32Array(new Float32Array(array).buffer);
    const sameFloatBits = (actual, expected, label) => {
        const a = floatBits(actual), e = floatBits(expected);
        if (a.length !== e.length) throw new Error(`${label}: lengths ${a.length} != ${e.length}`);
        for (let index = 0; index < a.length; index++) {
            if (a[index] !== e[index]) {
                throw new Error(`${label}[${index}]: 0x${a[index].toString(16)} != 0x${e[index].toString(16)}`);
            }
        }
    };
    const effectiveGrid = (N, requestedStride, interior) => {
        let stride = Math.max(1, Math.min(Math.max(1, N), Math.trunc(requestedStride)));
        const extent = Math.max(0, N - (interior ? 2 : 0));
        const sampleCount = value => Math.ceil(extent / value) ** 3;
        while (sampleCount(stride) > 262144) stride++;
        const lo = interior ? 1 : 0;
        const hi = interior ? N - 2 : N - 1;
        if (hi < lo) return { stride, origin: 0, count: 0 };
        const center = Math.floor((N - 1) / 2);
        const origin = center - Math.floor((center - lo) / stride) * stride;
        return { stride, origin, count: Math.floor((hi - origin) / stride) + 1 };
    };
    const sequentialFloatSum = values => {
        let sum = values[0];
        for (let index = 1; index < values.length; index++) sum = f32(sum + values[index]);
        return sum;
    };
    const seedFlux = (bridge, N) => {
        const center = Math.floor((N - 1) / 2);
        const step = N <= 17 ? 3 : 10;
        const axis = new Set([0, 1, center, N - 2, N - 1]);
        for (let value = 0; value < N; value += step) axis.add(value);
        const coordinates = [...axis].sort((a, b) => a - b);
        const sites = new Map();
        const add = (x, y, z) => sites.set(`${x},${y},${z}`, [x, y, z]);
        add(0, 0, 0);
        add(N - 1, N - 1, N - 1);
        add(center, center, center);
        // Center is present on every center-anchored sample grid. Offset seeds
        // guarantee nonzero radius-2 gravity and curvature at sampled sites
        // for every requested stride, independent of the sparse hash below.
        add(Math.min(N - 1, center + 2), center, center);
        add(center, Math.min(N - 1, center + 1), Math.max(0, center - 1));
        add(1, N - 2, Math.min(N - 1, 2));
        add(N - 2, Math.min(N - 1, 3), Math.max(0, N - 4));
        for (const z of coordinates) for (const y of coordinates) for (const x of coordinates) {
            if (((x * 11 + y * 7 + z * 5) & 3) === 0) add(x, y, z);
        }
        let index = 0;
        for (const [x, y, z] of sites.values()) {
            const exponent = ((x * 3 + y * 5 + z * 7 + index) % 12) - 8;
            const sign = ((x + 2 * y + 3 * z + index) & 1) ? -1 : 1;
            wasm.injectFlux(bridge, x, y, z, sign * (2 ** exponent), 0, 0);
            index++;
        }
        return sites.size;
    };
    const ownScalar = raw => ({
        positions: new Float32Array(raw.positions),
        values: new Float32Array(raw.values),
        count: raw.count,
        effectiveStride: raw.effectiveStride,
        origin: raw.origin,
    });
    const ownVector = raw => ({
        positions: new Float32Array(raw.positions),
        vectors: new Float32Array(raw.vectors),
        count: raw.count,
        effectiveStride: raw.effectiveStride,
        origin: raw.origin,
    });

    const rows = [];
    for (const { N, strides } of [
        { N: 17, strides: [1, 2, 3] },
        { N: 65, strides: [3] },
    ]) {
        const bridge = new wasm.RenderBridge(N);
        try {
            const seededSites = seedFlux(bridge, N);
            // Embind views are borrowed. This copy must precede every other
            // call so the independent oracle sees the exact prepared field.
            const volume = new Float64Array(wasm.getFluxVolume(bridge));
            if (volume.length !== N ** 3) throw new Error(`L=${N}: bad flux volume length`);
            const tickBefore = bridge.currentTick();
            const indexOf = (x, y, z) => ((z * N + y) * N + x);
            const wrap = value => (value % N + N) % N;
            const densityAt = (x, y, z) => volume[indexOf(wrap(x), wrap(y), wrap(z))];
            let maxRho = 0;
            for (const magnitude of volume) maxRho = Math.max(maxRho, magnitude * magnitude);
            const inverseRho = maxRho < 1e-30 ? 0 : 1 / maxRho;
            const latencyDoubleAt = (x, y, z) => Math.sqrt(Math.min(
                densityAt(x, y, z) ** 2 * inverseRho,
                HORIZON_CLAMP,
            ));
            const latencyFloatAt = (x, y, z) => f32(latencyDoubleAt(x, y, z));

            for (const requestedStride of strides) {
                const before = bridge.currentTick();
                const latency = ownScalar(wasm.getLatencySampled(bridge, requestedStride));
                const kretschmann = ownScalar(wasm.getKretschmannSampled(bridge, requestedStride));
                const gravity = ownVector(wasm.getGravityFieldSampled(bridge, requestedStride));
                if (bridge.currentTick() !== before || before !== tickBefore) {
                    throw new Error(`L=${N} h=${requestedStride}: observation advanced the engine tick`);
                }

                const expectedLatencyPositions = [], expectedLatencyValues = [];
                const latencyGrid = effectiveGrid(N, requestedStride, false);
                for (let z = latencyGrid.origin; z < latencyGrid.origin + latencyGrid.count * latencyGrid.stride; z += latencyGrid.stride)
                    for (let y = latencyGrid.origin; y < latencyGrid.origin + latencyGrid.count * latencyGrid.stride; y += latencyGrid.stride)
                        for (let x = latencyGrid.origin; x < latencyGrid.origin + latencyGrid.count * latencyGrid.stride; x += latencyGrid.stride) {
                            const value = latencyDoubleAt(x, y, z);
                            if (value < 1e-6) continue;
                            expectedLatencyPositions.push(x + 0.5, y + 0.5, z + 0.5);
                            expectedLatencyValues.push(f32(value));
                        }

                const expectedKPositions = [], expectedKValues = [];
                const kGrid = effectiveGrid(N, requestedStride, true);
                for (let z = kGrid.origin; z < kGrid.origin + kGrid.count * kGrid.stride; z += kGrid.stride)
                    for (let y = kGrid.origin; y < kGrid.origin + kGrid.count * kGrid.stride; y += kGrid.stride)
                        for (let x = kGrid.origin; x < kGrid.origin + kGrid.count * kGrid.stride; x += kGrid.stride) {
                            const self = latencyFloatAt(x, y, z);
                            const faceSum = sequentialFloatSum([
                                latencyFloatAt(x + 1, y, z), latencyFloatAt(x - 1, y, z),
                                latencyFloatAt(x, y + 1, z), latencyFloatAt(x, y - 1, z),
                                latencyFloatAt(x, y, z + 1), latencyFloatAt(x, y, z - 1),
                            ]);
                            const edgeSum = sequentialFloatSum([
                                latencyFloatAt(x + 1, y + 1, z), latencyFloatAt(x + 1, y - 1, z),
                                latencyFloatAt(x - 1, y + 1, z), latencyFloatAt(x - 1, y - 1, z),
                                latencyFloatAt(x + 1, y, z + 1), latencyFloatAt(x + 1, y, z - 1),
                                latencyFloatAt(x - 1, y, z + 1), latencyFloatAt(x - 1, y, z - 1),
                                latencyFloatAt(x, y + 1, z + 1), latencyFloatAt(x, y + 1, z - 1),
                                latencyFloatAt(x, y - 1, z + 1), latencyFloatAt(x, y - 1, z - 1),
                            ]);
                            const laplacian = (1 / 3) * faceSum + (1 / 6) * edgeSum - 4 * self;
                            const value = laplacian * laplacian;
                            if (value < 1e-18) continue;
                            expectedKPositions.push(x + 0.5, y + 0.5, z + 0.5);
                            expectedKValues.push(f32(value));
                        }

                const expectedGravityPositions = [], expectedGravityVectors = [];
                const gravityGrid = effectiveGrid(N, requestedStride, false);
                for (let z = gravityGrid.origin; z < gravityGrid.origin + gravityGrid.count * gravityGrid.stride; z += gravityGrid.stride)
                    for (let y = gravityGrid.origin; y < gravityGrid.origin + gravityGrid.count * gravityGrid.stride; y += gravityGrid.stride)
                        for (let x = gravityGrid.origin; x < gravityGrid.origin + gravityGrid.count * gravityGrid.stride; x += gravityGrid.stride) {
                            const scale = G_N * GRAD_TIER2_SCALE;
                            const fx = scale * (densityAt(x + 2, y, z) - densityAt(x - 2, y, z));
                            const fy = scale * (densityAt(x, y + 2, z) - densityAt(x, y - 2, z));
                            const fz = scale * (densityAt(x, y, z + 2) - densityAt(x, y, z - 2));
                            if (Math.hypot(fx, fy, fz) < 1e-15) continue;
                            expectedGravityPositions.push(x + 0.5, y + 0.5, z + 0.5);
                            expectedGravityVectors.push(f32(fx), f32(fy), f32(fz));
                        }

                const verifyMetadata = (actual, expectedGrid, label) => {
                    if (actual.effectiveStride !== expectedGrid.stride || actual.origin !== expectedGrid.origin) {
                        throw new Error(`${label}: grid h/o ${actual.effectiveStride}/${actual.origin} != ${expectedGrid.stride}/${expectedGrid.origin}`);
                    }
                };
                verifyMetadata(latency, latencyGrid, 'latency');
                verifyMetadata(kretschmann, kGrid, 'kretschmann');
                verifyMetadata(gravity, gravityGrid, 'gravity');
                sameFloatBits(latency.positions, expectedLatencyPositions, `L=${N} h=${requestedStride} latency positions`);
                sameFloatBits(latency.values, expectedLatencyValues, `L=${N} h=${requestedStride} latency values`);
                sameFloatBits(kretschmann.positions, expectedKPositions, `L=${N} h=${requestedStride} K positions`);
                sameFloatBits(kretschmann.values, expectedKValues, `L=${N} h=${requestedStride} K values`);
                sameFloatBits(gravity.positions, expectedGravityPositions, `L=${N} h=${requestedStride} gravity positions`);
                sameFloatBits(gravity.vectors, expectedGravityVectors, `L=${N} h=${requestedStride} gravity vectors`);
                if (latency.count !== expectedLatencyValues.length
                    || kretschmann.count !== expectedKValues.length
                    || gravity.count !== expectedGravityVectors.length / 3) {
                    throw new Error(`L=${N} h=${requestedStride}: compacted count mismatch`);
                }
                rows.push({
                    N, requestedStride, seededSites,
                    latency: { count: latency.count, stride: latency.effectiveStride, origin: latency.origin },
                    kretschmann: { count: kretschmann.count, stride: kretschmann.effectiveStride, origin: kretschmann.origin },
                    gravity: { count: gravity.count, stride: gravity.effectiveStride, origin: gravity.origin },
                    tick: bridge.currentTick(),
                });
            }
        } finally {
            bridge.delete();
        }
    }
    return { threaded, rows, horizonClamp: HORIZON_CLAMP, gravitationalCoupling: G_N };
}

for (const threaded of [false, true]) {
    test(`${threaded ? 'threaded' : 'direct'} native Gravity samplers preserve the exact pre-optimization field`, async ({ page }, testInfo) => {
        test.setTimeout(180_000);
        if (!threaded) await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        await bootDashboard(page, { engine: 'wasm', timeout: 90_000 });
        let result;
        if (threaded) {
            const worker = page.workers().find(candidate => candidate.url().includes('wasm-bridge.worker'));
            expect(worker).toBeDefined();
            await expect.poll(() => worker.evaluate(() => (
                typeof mod !== 'undefined' && !!mod
                && typeof bridge !== 'undefined' && !!bridge
            )), {
                timeout: 90_000,
                message: 'active WASM worker module/RenderBridge did not finish initialization',
            }).toBe(true);
            result = await worker.evaluate(checkGravityNativeSamplers, { threaded });
        } else {
            result = await page.evaluate(checkGravityNativeSamplers, { threaded });
        }
        expect(result.rows).toHaveLength(4);
        expect(result.rows.every(row => row.tick === 0)).toBe(true);
        expect(result.rows.every(row => row.seededSites >= 100)).toBe(true);
        expect(result.rows.every(row => row.latency.count > 0)).toBe(true);
        expect(result.rows.every(row => row.kretschmann.count > 0)).toBe(true);
        expect(result.rows.every(row => row.gravity.count > 0)).toBe(true);
        await testInfo.attach(`gravity-native-sampler-parity-${threaded ? 'threaded' : 'direct'}.json`, {
            body: JSON.stringify(result, null, 2),
            contentType: 'application/json',
        });
    });
}

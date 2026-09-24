import { test, expect } from '@playwright/test';
import { bootDashboard } from './_helpers.js';

// Evaluate synchronously inside the actual WASM owner. This isolated bridge
// exercises the native export without changing the dashboard's live state.
function checkFluxVolume({ threaded }) {
    const wasm = threaded ? mod : window._ftdBridge._module;
    const size = 17, probe = new wasm.RenderBridge(size);
    const samples = [[1, 3, 9, 3, 4, 12], [14, 7, 2, 8, 15, 0], [0, 16, 11, 1, 2, 2]];
    try {
        const expected = new Float64Array(size ** 3);
        for (const [x, y, z, fx, fy, fz] of samples) {
            wasm.injectFlux(probe, x, y, z, fx, fy, fz);
            expected[z * size * size + y * size + x] = Math.sqrt(fx * fx + fy * fy + fz * fz);
        }
        const seeded = new Float64Array(wasm.getFluxVolume(probe));
        const seedMatches = seeded.every((value, index) => value === expected[index]);
        const observations = [];
        for (let step = 0; step < 3; step++) {
            probe.tick();
            const before = probe.currentTick();
            const volume = new Float64Array(wasm.getFluxVolume(probe));
            // The unchanged slice exporter uses FieldSoA, independently
            // checking both post-tick freshness and X/Z transpose in AoS export.
            let maxDifference = 0;
            for (let z = 0; z < size; z++) {
                const slice = wasm.getFluxSlice(probe, 2, z);
                for (let x = 0; x < size; x++) for (let y = 0; y < size; y++) {
                    maxDifference = Math.max(maxDifference,
                        Math.abs(volume[z * size * size + y * size + x] - slice[x * size + y]));
                }
            }
            observations.push({ before, after: probe.currentTick(), maxDifference,
                changed: volume.some((value, index) => value !== seeded[index]) });
        }
        return { seedMatches, length: seeded.length, observations };
    } finally { probe.delete(); }
}

for (const threaded of [false, true]) {
    test(`${threaded ? 'threaded' : 'direct'} WASM volume preserves exact density, transpose and post-tick freshness`, async ({ page }) => {
        if (!threaded) await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        await bootDashboard(page, { engine: 'wasm', timeout: 90000 });
        let result;
        if (threaded) {
            const worker = page.workers().find(candidate => candidate.url().includes('wasm-bridge.worker'));
            expect(worker).toBeDefined();
            result = await worker.evaluate(checkFluxVolume, { threaded });
        } else result = await page.evaluate(checkFluxVolume, { threaded });
        expect(result.seedMatches).toBe(true);
        expect(result.length).toBe(17 ** 3);
        for (const observation of result.observations) {
            expect(observation.maxDifference).toBe(0);
            expect(observation.after).toBe(observation.before);
            expect(observation.changed).toBe(true);
        }
    });
}

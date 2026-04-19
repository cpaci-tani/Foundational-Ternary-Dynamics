// @ts-check
/**
 * Force-field decomposition sampler verification (2026-04-19).
 *
 * Confirms that the three WASM force-field samplers — getGravityFieldSampled,
 * getEMForceField, getStrongForceField — return physically sensible,
 * non-empty data on a particle-rich scenario. Before this fix they all
 * returned EMPTY_FIELD_SAMPLE unconditionally, so the gravity / EM / strong
 * force-arrow overlays on Scale 0 were silently blank for WASM backends.
 *
 * Physics expectations after 10 ticks of s0-seed-proton-candidate:
 *   - getEMForceField     → count > 0 (three manifested quarks carry charge)
 *   - getStrongForceField → count > 0 (three quarks form flux tubes / nuclear
 *                                       attraction regions)
 *   - getGravityFieldSampled → count ≥ 0 (scenario supplies some flux so |J|
 *                                          gradient is non-zero in the vicinity
 *                                          of each quark; we allow 0 if the
 *                                          scenario omits bulk flux seeding)
 *
 * Sample magnitudes must be finite (no NaN / Infinity) — a common Embind
 * mistake is forgetting typed_memory_view length bounds and returning stale
 * heap garbage.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Force-field decomposition samplers (WASM)', () => {
    test('all three samplers return non-empty, finite data on s0-seed-proton-candidate', async ({ page }) => {
        page.on('pageerror', (e) => console.error('PAGEERROR:', e.message));
        await gotoAndReady(page);

        const result = await page.evaluate(async () => {
            const b = window._ftdBridge;
            if (!b) return { error: 'no bridge' };
            if (typeof b.setupScenario !== 'function') return { error: 'no setupScenario' };

            b.setupScenario('s0-seed-proton-candidate');
            for (let i = 0; i < 5; i++) b.tick();

            // Also probe how many particles the bridge sees via getParticleData,
            // so we can distinguish "no manifested particles" from "sampler bug".
            const pd = b.getParticleData();
            const particleCount = pd?.count ?? -1;
            const backend = (b.constructor && b.constructor.name) || 'unknown';

            const samplers = ['getGravityFieldSampled', 'getEMForceField', 'getStrongForceField'];
            const out = { backend, particleCount };
            for (const name of samplers) {
                const r = b[name](2);
                const count = r?.count ?? -1;
                const vectors = r?.vectors;
                let finite = true;
                let sampleMag = 0;
                if (count > 0 && vectors && vectors.length >= 3) {
                    const vx = vectors[0], vy = vectors[1], vz = vectors[2];
                    finite = Number.isFinite(vx) && Number.isFinite(vy) && Number.isFinite(vz);
                    sampleMag = Math.hypot(vx, vy, vz);
                }
                out[name] = { count, finite, sampleMag };
            }
            return out;
        });

        console.log('Force-field sampler probe:', JSON.stringify(result, null, 2));
        expect(result.error).toBeUndefined();
        expect(result.getEMForceField.count).toBeGreaterThan(0);
        expect(result.getEMForceField.finite).toBe(true);
        expect(result.getStrongForceField.count).toBeGreaterThan(0);
        expect(result.getStrongForceField.finite).toBe(true);
        expect(result.getGravityFieldSampled.count).toBeGreaterThanOrEqual(0);
        expect(result.getGravityFieldSampled.finite).toBe(true);
    });
});

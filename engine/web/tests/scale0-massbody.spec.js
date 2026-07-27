// @ts-check
/**
 * Massive body — real-mass gravity (unified-mass Phase 1).
 *
 * The s0-seed-massive-body scenario seeds a dense ball of LOCKED rest mass.
 * Its gravity must come from REAL manifested mass (rho = M_REST·|state|) via the
 * latency-Poisson solver (latency_field on), NOT the |J|² field-energy proxy
 * (field_energy_gravity off). Genesis is disabled so the body's self-field can't
 * trigger runaway manifestation, and the well must be sub-horizon (a clean
 * gravity well, not a saturated black hole).
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test.describe('Scale-0 massive body (real-mass gravity)', () => {
    test('seeds a stable mass that sources a real sub-horizon latency well', async ({ page }) => {
        test.setTimeout(50_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdGravityPanel), { timeout: 20_000 }).toBe(true);

        await selectScale0Scenario(page, 's0-seed-massive-body');
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
            document.querySelector('#tab-bar .tab[data-panel="gravity"]')?.click();
        });

        // Real-mass source: latency_field ON, field_energy_gravity OFF.
        const toggles = await page.evaluate(async () => {
            const { resolveActiveScale0BridgeFromWindow } = await import('/js/scales/scale0/state/store.js');
            const b = resolveActiveScale0BridgeFromWindow();
            return { lf: b?.getToggle?.('latency_field'), fe: b?.getToggle?.('field_energy_gravity') };
        });
        expect(toggles.lf, 'latency_field on').toBe(true);
        expect(toggles.fe, 'field_energy_gravity off (real mass, not |J|² proxy)').toBe(false);

        // Stable mass — no genesis runaway (manifested count constant + small).
        const m1 = await page.evaluate(async () => {
            const { resolveActiveScale0BridgeFromWindow } = await import('/js/scales/scale0/state/store.js');
            return resolveActiveScale0BridgeFromWindow()?.capabilities?.scale0?.getScale0EnergyAudit?.()?.manifested ?? -1;
        });
        await page.waitForTimeout(4000);
        const m2 = await page.evaluate(async () => {
            const { resolveActiveScale0BridgeFromWindow } = await import('/js/scales/scale0/state/store.js');
            return resolveActiveScale0BridgeFromWindow()?.capabilities?.scale0?.getScale0EnergyAudit?.()?.manifested ?? -1;
        });
        expect(m1, 'mass manifested').toBeGreaterThan(0);
        expect(m1, 'mass is a compact body, not a runaway').toBeLessThan(200);
        expect(m2, 'mass is stable (no runaway manifestation)').toBe(m1);

        // Real C++ latency well: active, sub-horizon, and spatially localized.
        // The worker bridge intentionally does not mirror the full latency
        // volume; radial monotonicity is covered directly by the native C++
        // scenario_behavior test against RenderBridge::voxel_at().
        const r = await page.evaluate(async () => {
            const { resolveActiveScale0BridgeFromWindow } = await import('/js/scales/scale0/state/store.js');
            const sc = resolveActiveScale0BridgeFromWindow()?.capabilities?.scale0;
            const agg = sc?.getScale0GravityMetricAgg?.();
            return {
                active: agg?.active,
                latencyMax: agg?.latencyMax,
                latencyMean: agg?.latencyMean,
            };
        });
        expect(r.active, 'real C++ latency active').toBe(true);
        expect(r.latencyMax, 'a real well exists').toBeGreaterThan(0.05);
        expect(r.latencyMax, 'sub-horizon (not a saturated black hole)').toBeLessThan(0.9);
        expect(r.latencyMax, 'well is localized rather than spatially uniform').toBeGreaterThan(r.latencyMean);
    });
});

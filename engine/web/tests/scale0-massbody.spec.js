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
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 massive body (real-mass gravity)', () => {
    test('seeds a stable mass that sources a real sub-horizon latency well', async ({ page }) => {
        test.setTimeout(50_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdGravityPanel), { timeout: 20_000 }).toBe(true);

        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) { sel.value = 's0-seed-massive-body'; sel.dispatchEvent(new Event('change', { bubbles: true })); }
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
            document.querySelector('#tab-bar .tab[data-panel="gravity"]')?.click();
        });

        // Real-mass source: latency_field ON, field_energy_gravity OFF.
        const toggles = await page.evaluate(() => {
            const b = window.__ftdCtx?.bridge;
            return { lf: b?.getToggle?.('latency_field'), fe: b?.getToggle?.('field_energy_gravity') };
        });
        expect(toggles.lf, 'latency_field on').toBe(true);
        expect(toggles.fe, 'field_energy_gravity off (real mass, not |J|² proxy)').toBe(false);

        // Stable mass — no genesis runaway (manifested count constant + small).
        const m1 = await page.evaluate(() => window.__ftdCtx?.bridge?.capabilities?.scale0?.getScale0EnergyAudit?.()?.manifested ?? -1);
        await page.waitForTimeout(4000);
        const m2 = await page.evaluate(() => window.__ftdCtx?.bridge?.capabilities?.scale0?.getScale0EnergyAudit?.()?.manifested ?? -1);
        expect(m1, 'mass manifested').toBeGreaterThan(0);
        expect(m1, 'mass is a compact body, not a runaway').toBeLessThan(200);
        expect(m2, 'mass is stable (no runaway manifestation)').toBe(m1);

        // Real C++ latency well: active, sub-horizon, peaks at the mass, falls off.
        const r = await page.evaluate(() => {
            const sc = window.__ftdCtx?.bridge?.capabilities?.scale0;
            const agg = sc?.getScale0GravityMetricAgg?.();
            const vol = sc?.getScale0LatencyVolume?.();
            const N = sc?.latticeSize | 0;
            const vidx = (x, y, z) => (z * N + y) * N + x;
            const c = N >> 1;
            return {
                active: agg?.active, latencyMax: agg?.latencyMax,
                centerLat: vol[vidx(c, c, c)], outerLat: vol[vidx(N - 1, c, c)],
            };
        });
        expect(r.active, 'real C++ latency active').toBe(true);
        expect(r.latencyMax, 'a real well exists').toBeGreaterThan(0.05);
        expect(r.latencyMax, 'sub-horizon (not a saturated black hole)').toBeLessThan(0.9);
        expect(r.centerLat, 'latency peaks at the mass, falls with distance').toBeGreaterThan(r.outerLat);
    });
});

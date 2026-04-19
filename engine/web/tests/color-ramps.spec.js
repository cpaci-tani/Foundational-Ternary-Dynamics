// @ts-check
import { test, expect } from '@playwright/test';

/**
 * Unit-style tests for viewport/color-ramps.js — extracted from viewport.js
 * as Wave 1 ticket 1 of the large-file refactor. These are "pure math"
 * tests: load the module in the browser context (where Three.js + import
 * maps are already set up) and assert the exported ramp functions produce
 * the documented color outputs.
 *
 * These tests protect against accidental rewrites of the ramp tables
 * during future cleanup passes. Each ramp's endpoint values are
 * hand-verified against viewport.js @ the commit before extraction.
 */

const EPS = 0.001;

function nearly(actual, expected, eps = EPS) {
    return Math.abs(actual - expected) < eps;
}

test.describe('color-ramps module', () => {
    test.beforeEach(async ({ page }) => {
        // Boot the dashboard so the importmap for `three` is available —
        // color-ramps.js has no Three imports itself, but it's loaded
        // through the same graph that does, and running it inside the
        // page context is the simplest verification path.
        await page.goto('/index.html');
        await expect.poll(() => page.evaluate(() => !!window._ftdBridge), {
            timeout: 20_000,
        }).toBeTruthy();
    });

    test('module exports all 15 named ramp functions + FORCE_PALETTES + lerpPalette + RAMP_BY_NAME', async ({ page }) => {
        const exports = await page.evaluate(async () => {
            const mod = await import('/js/viewport/color-ramps.js');
            return {
                ramps: [
                    typeof mod.rampViridis,
                    typeof mod.rampCyclicHSL,
                    typeof mod.rampDivergingRdBu,
                    typeof mod.rampGrayscale,
                    typeof mod.rampGravWell,
                    typeof mod.rampEmEnergy,
                    typeof mod.rampCharge,
                    typeof mod.rampVorticity,
                    typeof mod.rampHelicity,
                    typeof mod.rampKretschmann,
                    typeof mod.rampEPressure,
                    typeof mod.rampBPressure,
                    typeof mod.rampKineticEnergy,
                    typeof mod.rampFisher,
                    typeof mod.rampCoherence,
                ],
                lerpPalette: typeof mod.lerpPalette,
                forcePalettes: typeof mod.FORCE_PALETTES,
                rampByName: typeof mod.RAMP_BY_NAME,
                rampByNameKeys: Object.keys(mod.RAMP_BY_NAME || {}).length,
            };
        });
        expect(exports.ramps).toEqual(new Array(15).fill('function'));
        expect(exports.lerpPalette).toBe('function');
        expect(exports.forcePalettes).toBe('object');
        expect(exports.rampByName).toBe('object');
        expect(exports.rampByNameKeys).toBe(15);
    });

    test('rampViridis endpoints match known purple/yellow values', async ({ page }) => {
        const samples = await page.evaluate(async () => {
            const { rampViridis } = await import('/js/viewport/color-ramps.js');
            const a = new Float32Array(3);
            const b = new Float32Array(3);
            rampViridis(0.0, a, 0);  // purple
            rampViridis(1.0, b, 0);  // yellow
            return { low: [...a], high: [...b] };
        });
        // Low endpoint: (0.267, 0.004, 0.329) — purple
        expect(nearly(samples.low[0], 0.267)).toBeTruthy();
        expect(nearly(samples.low[1], 0.004)).toBeTruthy();
        expect(nearly(samples.low[2], 0.329)).toBeTruthy();
        // High endpoint: (0.993, 0.906, 0.144) — yellow
        expect(nearly(samples.high[0], 0.993)).toBeTruthy();
        expect(nearly(samples.high[1], 0.906)).toBeTruthy();
        expect(nearly(samples.high[2], 0.144)).toBeTruthy();
    });

    test('rampGrayscale is t → (t,t,t) with clamping', async ({ page }) => {
        const r = await page.evaluate(async () => {
            const { rampGrayscale } = await import('/js/viewport/color-ramps.js');
            const out = new Float32Array(9);
            rampGrayscale(-0.5, out, 0);   // clamped to 0
            rampGrayscale(0.5, out, 3);    // mid
            rampGrayscale(2.0, out, 6);    // clamped to 1
            return [...out];
        });
        expect(r[0]).toBe(0); expect(r[1]).toBe(0); expect(r[2]).toBe(0);
        expect(r[3]).toBe(0.5); expect(r[4]).toBe(0.5); expect(r[5]).toBe(0.5);
        expect(r[6]).toBe(1); expect(r[7]).toBe(1); expect(r[8]).toBe(1);
    });

    test('rampDivergingRdBu is signed: negative=blue, zero=white, positive=red', async ({ page }) => {
        const r = await page.evaluate(async () => {
            const { rampDivergingRdBu } = await import('/js/viewport/color-ramps.js');
            const out = new Float32Array(9);
            rampDivergingRdBu(-1, out, 0);   // blue endpoint
            rampDivergingRdBu(0, out, 3);    // white mid
            rampDivergingRdBu(1, out, 6);    // red endpoint
            return [...out];
        });
        // Blue: (0.129, 0.400, 0.675)
        expect(nearly(r[0], 0.129)).toBeTruthy();
        expect(nearly(r[1], 0.400)).toBeTruthy();
        expect(nearly(r[2], 0.675)).toBeTruthy();
        // White mid: (0.969, 0.969, 0.969)
        expect(nearly(r[3], 0.969)).toBeTruthy();
        expect(nearly(r[4], 0.969)).toBeTruthy();
        expect(nearly(r[5], 0.969)).toBeTruthy();
        // Red: (0.698, 0.094, 0.169)
        expect(nearly(r[6], 0.698)).toBeTruthy();
        expect(nearly(r[7], 0.094)).toBeTruthy();
        expect(nearly(r[8], 0.169)).toBeTruthy();
    });

    test('rampCoherence: diverging orange ↔ violet', async ({ page }) => {
        const r = await page.evaluate(async () => {
            const { rampCoherence } = await import('/js/viewport/color-ramps.js');
            const out = new Float32Array(6);
            rampCoherence(-1, out, 0);   // violet (negative)
            rampCoherence(1, out, 3);    // orange (positive)
            return [...out];
        });
        // Violet endpoint: (0.45, 0.15, 0.85)
        expect(nearly(r[0], 0.45)).toBeTruthy();
        expect(nearly(r[1], 0.15)).toBeTruthy();
        expect(nearly(r[2], 0.85)).toBeTruthy();
        // Orange endpoint: (1.00, 0.55, 0.10)
        expect(nearly(r[3], 1.00)).toBeTruthy();
        expect(nearly(r[4], 0.55)).toBeTruthy();
        expect(nearly(r[5], 0.10)).toBeTruthy();
    });

    test('FORCE_PALETTES has em/gravity/strong/weak with low/mid/high tuples', async ({ page }) => {
        const pals = await page.evaluate(async () => {
            const { FORCE_PALETTES } = await import('/js/viewport/color-ramps.js');
            const result = {};
            for (const [name, pal] of Object.entries(FORCE_PALETTES)) {
                result[name] = {
                    lowLen: pal.low?.length,
                    midLen: pal.mid?.length,
                    highLen: pal.high?.length,
                };
            }
            return result;
        });
        for (const force of ['em', 'gravity', 'strong', 'weak']) {
            expect(pals[force]).toBeDefined();
            expect(pals[force].lowLen).toBe(3);
            expect(pals[force].midLen).toBe(3);
            expect(pals[force].highLen).toBe(3);
        }
    });

    test('lerpPalette: t=0 → low, t=0.5 → mid, t=1 → high', async ({ page }) => {
        const r = await page.evaluate(async () => {
            const { lerpPalette, FORCE_PALETTES } = await import('/js/viewport/color-ramps.js');
            const em = FORCE_PALETTES.em;
            return {
                t0: lerpPalette(em, 0),
                tMid: lerpPalette(em, 0.5),
                t1: lerpPalette(em, 1),
            };
        });
        expect(r.t0).toEqual([0.0, 0.2, 0.4]);        // em.low
        expect(r.tMid).toEqual([0.0, 0.9, 1.0]);      // em.mid
        expect(r.t1).toEqual([0.7, 1.0, 1.0]);        // em.high
    });

    test('RAMP_BY_NAME exposes every ramp by string key', async ({ page }) => {
        const keys = await page.evaluate(async () => {
            const mod = await import('/js/viewport/color-ramps.js');
            return Object.keys(mod.RAMP_BY_NAME).sort();
        });
        expect(keys).toEqual([
            'rampBPressure', 'rampCharge', 'rampCoherence', 'rampCyclicHSL',
            'rampDivergingRdBu', 'rampEPressure', 'rampEmEnergy', 'rampFisher',
            'rampGravWell', 'rampGrayscale', 'rampHelicity', 'rampKineticEnergy',
            'rampKretschmann', 'rampViridis', 'rampVorticity',
        ]);
    });
});

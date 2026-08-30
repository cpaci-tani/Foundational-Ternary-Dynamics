// @ts-check
/**
 * Gravity Observatory panel — integration test.
 *
 * The exact transverse-wave control must remain proxy-only: it renders the
 * field-derived readouts and slices, but it must not fabricate an engine
 * latency map. The dedicated massive-body source then exercises the engine's
 * Poisson-derived [IMPOSED] mapping. Together these checks cover getGravitySlice
 * → transpose → paint, proxy field sampling, and the mapped aggregate
 * without calling the closed-negative wave a gravitational field.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test.describe('Scale-0 Gravity Observatory', () => {
    test('keeps the transverse-wave control proxy-only and surfaces mapped latency for a mass source', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdGravityPanel), { timeout: 20_000 }).toBe(true);

        // This exact transverse n=4 wave has no metric, mass source, or
        // gravity-specific operator. It is valid coverage for the visual
        // proxy, but it must not activate the engine latency mapping.
        await selectScale0Scenario(page, 's0-seed-gravitational-wave');
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
            document.querySelector('#tab-bar .tab[data-panel="gravity"]')?.click();
        });

        // The field-derived proxy does respond to the wave.
        await expect.poll(async () => page.evaluate(() => {
            const m = window.__ftdGravityPanel?.lastMetrics;
            return m ? m.L.max : 0;
        }), { timeout: 12_000, message: 'proxy latency L should populate on the transverse-wave control' }).toBeGreaterThan(0);

        const tel = await page.evaluate(() => {
            const m = window.__ftdGravityPanel.lastMetrics;
            return {
                Lmax: m.L.max,
                Kmax: m.K.max,
                Fmean: m.F.mean,
                dil: m.dilationPct,
                hasGravityPE: Object.prototype.hasOwnProperty.call(m, 'gravPE'),
                gnG: m.gnG,
            };
        });
        expect(tel.Lmax, 'latency max').toBeGreaterThan(0);
        expect(tel.Kmax, 'curvature-proxy max ≥ 0').toBeGreaterThanOrEqual(0);
        expect(tel.Fmean, 'force mean ≥ 0').toBeGreaterThanOrEqual(0);
        expect(tel.dil, 'dilation % ≥ 0').toBeGreaterThanOrEqual(0);
        expect(tel.hasGravityPE, 'unsupported pairwise gravity PE stays absent').toBe(false);
        expect(tel.gnG, 'G_N = 0.01').toBeCloseTo(0.01, 5);

        const waveCpp = await page.evaluate(() => window.__ftdGravityPanel?.lastAgg);
        expect(waveCpp?.active, 'closed-negative wave must not fabricate mapped engine latency').toBe(false);
        expect(waveCpp?.voxelCount, 'closed-negative wave has no mapped latency cells').toBe(0);

        // Slices: at least one of the 3 axis tiles painted non-background pixels.
        const anyPainted = await page.evaluate(() => {
            let bright = 0;
            for (const axis of [0, 1, 2]) {
                const cv = document.getElementById(`gravity-panel-tile-${axis}`);
                if (!cv) continue;
                const d = cv.getContext('2d').getImageData(0, 0, cv.width, cv.height).data;
                for (let i = 0; i < d.length; i += 4) {
                    if (d[i] > 40 || d[i + 1] > 40 || d[i + 2] > 60) { bright++; break; }
                }
            }
            return bright;
        });
        expect(anyPainted, 'at least one gravity slice tile painted structure').toBeGreaterThan(0);

        // Cycle the quantity selector — each kind repaints without error.
        for (const kind of ['kretschmann', 'force', 'dilation', 'latency']) {
            await page.evaluate((k) => window.__ftdGravityPanel.setKind(k), kind);
            const active = await page.evaluate(() => window.__ftdGravityPanel.activeKind);
            expect(active).toBe(kind);
        }

        // Δ-trace: sparklines exist and the history grows as the field advances.
        const h0 = await page.evaluate(() => window.__ftdGravityPanel.historyLength);
        const sparks = await page.evaluate(() => document.querySelectorAll('#gravity-panel-delta .grav-spark').length);
        expect(sparks, '4 metric sparklines').toBe(4);
        await expect.poll(async () => page.evaluate(() => window.__ftdGravityPanel.historyLength),
            { timeout: 8_000, message: 'Δ-trace history grows as fieldDataVersion bumps' }).toBeGreaterThan(h0);

        // The massive-body setup is the dedicated mapped-latency source: a
        // locked rest-mass ball with gravity + latency_field enabled. Its proxy
        // |J|² value can correctly remain zero, so only the engine aggregate is
        // expected to become active here.
        await selectScale0Scenario(page, 's0-seed-massive-body');
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });
        await expect.poll(async () => page.evaluate(() => window.__ftdGravityPanel?.lastAgg?.active),
            { timeout: 12_000, message: 'mapped engine latency should activate on the massive-body source' }).toBe(true);
        const cpp = await page.evaluate(() => ({
            latencyMax: window.__ftdGravityPanel.lastAgg.latencyMax,
            voxelCount: window.__ftdGravityPanel.lastAgg.voxelCount,
            txt: document.getElementById('panel-gravity')?.textContent || '',
        }));
        expect(cpp.latencyMax, 'mapped latency max > 0').toBeGreaterThan(0);
        expect(cpp.voxelCount, 'mapped latency cells populated').toBeGreaterThan(0);
        expect(cpp.txt.includes('Engine latency map (Poisson-derived; [IMPOSED])'),
            '[IMPOSED] engine block rendered').toBe(true);
        expect(cpp.txt.includes('[ENGINE]'), '[ENGINE] implementation tag present').toBe(true);
    });
});

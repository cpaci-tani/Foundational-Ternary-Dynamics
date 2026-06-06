// @ts-check
/**
 * Gravity Observatory panel — integration test.
 *
 * On the gravitational-wave scenario the panel must: render the per-axis
 * gravity slices (a real painted plane, not blank), populate the proxy
 * telemetry (latency/force/dilation), and grow the live Δ-trace as the field
 * advances. Verifies the whole chain: getGravitySlice → transpose → paint, and
 * getScale0FieldSamples('latency'/'kretschmann') + force field → aggregateMetrics.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 Gravity Observatory', () => {
    test('slices render, telemetry populates, and the Δ-trace responds to field advance', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdGravityPanel), { timeout: 20_000 }).toBe(true);

        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) { sel.value = 's0-seed-gravitational-wave'; sel.dispatchEvent(new Event('change', { bubbles: true })); }
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
            document.querySelector('#tab-bar .tab[data-panel="gravity"]')?.click();
        });

        // Telemetry: the latency proxy populates from the seeded flux field.
        await expect.poll(async () => page.evaluate(() => {
            const m = window.__ftdGravityPanel?.lastMetrics;
            return m ? m.L.max : 0;
        }), { timeout: 12_000, message: 'latency L should populate on the GW scenario' }).toBeGreaterThan(0);

        const tel = await page.evaluate(() => {
            const m = window.__ftdGravityPanel.lastMetrics;
            return { Lmax: m.L.max, Fmean: m.F.mean, dil: m.dilationPct, gravPE: m.gravPE, gnG: m.gnG };
        });
        expect(tel.Lmax, 'latency max').toBeGreaterThan(0);
        expect(tel.Fmean, 'force mean ≥ 0').toBeGreaterThanOrEqual(0);
        expect(tel.dil, 'dilation % ≥ 0').toBeGreaterThanOrEqual(0);
        expect(Number.isFinite(tel.gravPE), 'gravity PE finite').toBe(true);
        expect(tel.gnG, 'G_N = 0.01').toBeCloseTo(0.01, 5);

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
    });
});

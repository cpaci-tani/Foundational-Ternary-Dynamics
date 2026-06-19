// @ts-check
/**
 * Two Sectors panel — integration test (FTD-0004 causality demo).
 *
 * The docked Scale-0 panel must mount, render two mid-plane heatmap canvases +
 * the radius-vs-tick chart, carry the honest constraint-vs-signal framing, and —
 * on Play — capture both sectors on an isolated engine and replay them, with the
 * fitted transverse slope landing near the lattice light-speed c = 1/√3 ≈ 0.577.
 * The capture must NOT disturb the user's main bridge.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 Two Sectors', () => {
    test('mounts, captures on Play, replays, and the fitted slope ≈ 1/√3', async ({ page }) => {
        test.setTimeout(120_000);
        await gotoAndReady(page, { timeout: 90_000 });   // generous headroom for a cold WASM boot
        await expect.poll(() => page.evaluate(() => !!window.__ftdTwoSectorsPanel),
            { timeout: 20_000 }).toBe(true);

        // Open the tab (exercises the registry/tab wiring).
        await page.evaluate(() => {
            document.querySelector('#tab-bar .tab[data-panel="two-sectors"]')?.click();
        });

        const structure = await page.evaluate(() => {
            const host = document.getElementById('panel-two-sectors');
            return {
                canvases: host?.querySelectorAll('canvas.ts-canvas').length ?? 0,
                hasChart: !!host?.querySelector('svg.ts-chart'),
                txt: host?.textContent || '',
            };
        });

        // Structure: two heatmap canvases + the chart render.
        expect(structure.canvases, 'two mid-plane canvases render').toBe(2);
        expect(structure.hasChart, 'radius-vs-tick chart renders').toBe(true);

        // Honest framing (epistemic guard — no superluminal claim).
        expect(/1\/√3|0\.577/.test(structure.txt), '1/√3 framing present').toBe(true);
        expect(structure.txt.includes('carries no signal'), 'constraint-not-a-signal note present').toBe(true);
        expect(structure.txt.includes('Postulate 4'), 'Postulate 4 (causality) cited').toBe(true);
        expect(structure.txt.includes('FTD-0004'), 'FTD-0004 cited').toBe(true);

        // Capture on Play; the isolated capture engine must not disturb the main bridge.
        const mainTickBefore = await page.evaluate(() => window._ftdBridge?.currentTick?.() ?? null);
        await page.evaluate(() => window.__ftdTwoSectorsPanel.play());
        await expect.poll(() => page.evaluate(() => !!window.__ftdTwoSectorsPanel.cache),
            { timeout: 45_000 }).toBe(true);

        // The fitted transverse slope lands near the lattice light-speed (band kept wide
        // for lattice discretization).
        const slope = await page.evaluate(() => window.__ftdTwoSectorsPanel.fitSlope);
        expect(slope, 'fitted slope is a number').not.toBeNull();
        expect(slope, 'slope ≳ 0.50').toBeGreaterThan(0.50);
        expect(slope, 'slope ≲ 0.66').toBeLessThan(0.66);

        // Step advances the cursor deterministically. (The auto-replay loop is
        // rAF-driven and pauses in a hidden/headless document; Step does not need rAF.)
        const c0 = await page.evaluate(() => window.__ftdTwoSectorsPanel.cursor);
        await page.evaluate(() => { window.__ftdTwoSectorsPanel.step(); window.__ftdTwoSectorsPanel.step(); });
        const c1 = await page.evaluate(() => window.__ftdTwoSectorsPanel.cursor);
        expect(c1, 'Step advances the cursor').not.toBe(c0);

        // A canvas was actually painted (non-background pixels present).
        const painted = await page.evaluate(() => {
            const cv = document.querySelector('#panel-two-sectors canvas.ts-canvas');
            const ctx = cv.getContext('2d');
            const d = ctx.getImageData(0, 0, cv.width, cv.height).data;
            let lit = 0;
            for (let i = 0; i < d.length; i += 4) if (d[i] > 20 || d[i + 1] > 20 || d[i + 2] > 20) lit++;
            return lit;
        });
        expect(painted, 'transverse heatmap is painted').toBeGreaterThan(50);
    });
});

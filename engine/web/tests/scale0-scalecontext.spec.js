// @ts-check
/**
 * Scale Context panel — integration test (FTD-0306).
 *
 * The docked Scale-0 panel must mount, render the log-scale length ruler (SVG)
 * with a live "you are here" lattice bracket + the CERN/LHC reach marker, and
 * the length/energy readout rows — all carrying the honest tags ([CALIBRATION],
 * IDENT-NULL) and the live lattice size. Numbers mirror the canonical
 * shared browser constants (K_GENESIS = 3·K_MANIFEST, 2 m_e = 2·M_E_PHYS).
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 Scale Context', () => {
    test('mounts, renders the ruler + readouts, shows live L and the honest tags', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdScaleContextPanel),
            { timeout: 20_000 }).toBe(true);

        // Open the tab (panel content is rendered at mount regardless, but this
        // exercises the registry tab wiring too).
        await page.evaluate(() => {
            document.querySelector('#tab-bar .tab[data-panel="scale-context"]')?.click();
        });

        const r = await page.evaluate(async () => {
            const { K_GENESIS, M_E_PHYS } = await import('/js/constants.js');
            const host = document.getElementById('panel-scale-context');
            const svg = host?.querySelector('svg.sc-ruler');
            return {
                hasRuler: !!svg,
                rows: host?.querySelectorAll('.sc-row').length ?? 0,
                txt: host?.textContent || '',
                svgTxt: svg?.textContent || '',
                expectedGenesis: K_GENESIS.toFixed(3),
                expectedPair: (2 * M_E_PHYS).toFixed(3),
            };
        });

        // Structure: the ruler SVG + length/energy readout rows render.
        expect(r.hasRuler, 'scale ruler SVG renders').toBe(true);
        expect(r.rows, 'length + energy readout rows render').toBeGreaterThanOrEqual(10);

        // Honest framing + tags present (epistemic guard).
        expect(r.txt.includes('[CALIBRATION]'), '[CALIBRATION] tag present').toBe(true);
        expect(r.txt.includes('IDENT-NULL'), 'IDENT-NULL caveat present').toBe(true);
        expect(/Planck/i.test(r.txt), 'Planck framing present').toBe(true);
        expect(/K_GENESIS|manifestation/i.test(r.txt), 'manifestation threshold present').toBe(true);

        // The live lattice size is shown (default scenario L=33).
        expect(/L=\d+/.test(r.txt), 'live lattice size rendered').toBe(true);

        // The CERN/LHC reach marker is drawn (ruler text or readout).
        expect(/CERN|LHC/.test(r.svgTxt + r.txt), 'CERN/LHC reach marker present').toBe(true);

        // Key numbers match the canonical script (no overclaim, no drift).
        expect(r.txt.includes(r.expectedGenesis),
            `K_GENESIS = ${r.expectedGenesis} MeV`).toBe(true);
        expect(r.txt.includes(r.expectedPair),
            `QED pair threshold 2 m_e = ${r.expectedPair} MeV`).toBe(true);
    });
});

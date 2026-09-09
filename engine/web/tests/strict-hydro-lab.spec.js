import { test, expect } from '@playwright/test';
import { mkdir } from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

// Requires the isolated hydro-candidate artifacts and config (mirrors
// strict-candidate-lab.spec.js's skip pattern for the Φ-v2 candidate).
test.skip(process.env.FTD_STRICT_HYDRO_LAB !== '1', 'Requires isolated compiled hydro artifacts and the hydro config');

const PREPARATIONS = [
    'hydro-shear-wave-t2', 'hydro-shear-wave-e', 'hydro-sound-wave',
    'hydro-taylor-green', 'hydro-shear-layer', 'hydro-vortex-pair',
];

// Byte-for-byte the owner-ruling caption fixed in
// .superpowers/sdd/2026-09-08-fchc-successor-part-a/task-13-brief.md (item 6) and
// rendered verbatim in engine/strict/web/hydro/index.html's #caption element.
const CAPTION_TEXT = 'Discrete lattice-gas fluid on `phi-hydro-staged-candidate-1`, a priced successor to the selected Φ-v2 (adopted, not derived). Its fluid limit — sound speed, Galilean factor and two cubic shear viscosities at the registered density — is derived by exact Chapman–Enskog (H1′) and measured in the registered campaign (H2′). It is anisotropic momentum hydrodynamics, not Navier–Stokes: the adopted symmetry group forces a cubic viscosity tensor. One realization: statistical noise scales as 1/√N per block.';

const TESTS_DIR = path.dirname(fileURLToPath(import.meta.url)); // engine/web/tests
// Ignored build directory (not the wave-scoped .superpowers/sdd/... plan folder,
// which outlives this wave and should not accumulate test artifacts).
const SCREENSHOT_PATH = path.resolve(
    TESTS_DIR, '..', '..', 'build_strict_hydro', 'lab', 'screenshots',
    'task-13-shear-wave-t2-after-20-stages.png',
);

const snapshot = (page) => page.evaluate(() => window.__hydroLabSnapshot);

async function ready(page) {
    await expect(page.locator('#step')).toBeEnabled();
    await expect.poll(async () => Boolean(await snapshot(page))).toBe(true);
}

async function load(page, preparation) {
    await page.goto('/strict/web/hydro/');
    await ready(page);
    if (preparation !== 'hydro-shear-wave-t2') {
        await page.locator('#preparation').selectOption(preparation);
        await ready(page);
    }
}

for (const preparation of PREPARATIONS) {
    test(`${preparation} L=16: 20 stages advance the fitted readouts and preserve mass/momentum`, async ({ page }) => {
        const errors = [];
        page.on('pageerror', (error) => errors.push(error.message));
        await load(page, preparation);

        const initial = await snapshot(page);
        expect(initial.preparation).toBe(preparation);
        expect(initial.L).toBe(16);
        expect(initial.stage).toBe(0);

        // The headless pane pauses requestAnimationFrame, so Run never advances here --
        // every stage is driven explicitly through the Step button.
        for (let stage = 1; stage <= 20; stage++) {
            await page.locator('#step').click();
            await ready(page);
            await expect(page.locator('#stage')).toHaveText(String(stage));
        }

        const final = await snapshot(page);
        expect(final.stage).toBe(20);
        expect(final.mass).toBe(initial.mass);
        expect(final.momentum).toEqual(initial.momentum);

        // The fitted decay-rate readouts are populated once at least two stages exist.
        await expect(page.locator('#gammaFit')).not.toHaveText('—');
        await expect(page.locator('#nuFit')).not.toHaveText('—');
        await expect(page.locator('#nuT2')).not.toHaveText('—');
        await expect(page.locator('#nuE')).not.toHaveText('—');
        await expect(page.locator('#csq')).not.toHaveText('—');
        await expect(page.locator('#gconst')).not.toHaveText('—');

        const captionText = await page.locator('#caption').textContent();
        expect(captionText).toBe(CAPTION_TEXT);

        expect(errors).toEqual([]);

        if (preparation === 'hydro-shear-wave-t2') {
            await mkdir(path.dirname(SCREENSHOT_PATH), { recursive: true });
            await page.screenshot({ path: SCREENSHOT_PATH, fullPage: true });
        }
    });
}

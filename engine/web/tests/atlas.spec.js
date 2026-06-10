// Smoke + acceptance test for the standalone FTD Ontology Atlas page.
// The atlas is NOT the dashboard — it has no _ftdBridge — so we navigate
// directly and poll its own test handle, window.__ftdAtlas.
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, realErrors } from './_helpers.js';

test.describe('FTD Ontology Atlas', () => {
  test('boots and exposes the test handle', async ({ page }) => {
    test.setTimeout(60_000);
    const errors = attachConsoleWatcher(page);
    await page.goto('/fields-atlas.html', { waitUntil: 'domcontentloaded' });
    await expect.poll(() => page.evaluate(() => window.__ftdAtlas?.ready), { timeout: 30_000 }).toBe(true);
    await expect(page.locator('#atlas-canvas')).toBeVisible();
    expect(await page.evaluate(() => window.__ftdAtlas?.stageCount)).toBe(14);
    const real = realErrors(errors);
    expect(real, real.join('\n')).toEqual([]);
  });
});

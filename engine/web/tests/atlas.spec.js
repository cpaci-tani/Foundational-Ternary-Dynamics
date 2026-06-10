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

  test('chain steps, layers toggle, ghosts sit outside, tags show', async ({ page }) => {
    test.setTimeout(60_000);
    await page.goto('/fields-atlas.html', { waitUntil: 'domcontentloaded' });
    await page.waitForFunction(() => window.__ftdAtlas?.ready, { timeout: 30_000 });
    await page.evaluate(() => window.__ftdAtlas.setStage(11)); // Ψ stage
    await expect(page.locator('#detail-panel')).toContainText('[');
    await expect(page.locator('#detail-panel')).toContainText(/SELECTION|OPEN|bookkeeping/i);
    expect(await page.evaluate(() => window.__ftdAtlas.layerBoundsCenterX('psiWave'))).toBeGreaterThan(1);
    await expect(page.locator('#layer-panel')).toContainText('Declined');
    const flipped = await page.evaluate(() => window.__ftdAtlas.toggleLayer('latency'));
    expect(typeof flipped).toBe('boolean');
  });
});

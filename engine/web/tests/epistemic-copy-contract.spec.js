import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';

const engineRoot = path.resolve(process.cwd(), '..', '..');
const read = (relativePath) => fs.readFileSync(path.join(engineRoot, relativePath), 'utf8');

test('audited web claims retain their canonical epistemic status', () => {
  const knowledge = read('web/js/ui/components/knowledge-base/data/constants.js');
  const faq = read('web/js/ui/components/faq/data.js');
  const catalog = read('web/js/particle-catalog.js');
  const cosmic = read('web/js/cosmic-renderer.js');
  const cosmicPanel = read('web/js/ui/components/panel-resources/template.js');

  expect(knowledge).toContain('electron-mass anchor [IMPOSED]');
  expect(knowledge).not.toContain('derived from the model’s mass story');
  expect(faq).toContain('CODATA-calibrated');
  expect(faq).not.toContain('FTD-derived');
  expect(catalog).toMatch(/id: 'higgs'[\s\S]*?ftd_status: 'parametric'/);
  expect(catalog).toContain('excluded at PDG-2024 precision');
  expect(cosmic).toContain('Schwarzschild-inspired lattice proxy');
  expect(cosmic).toContain('/ (C_SPEED * C_SPEED)');
  expect(cosmic).not.toContain('Genuine Schwarzschild horizon');
  expect(cosmicPanel).toContain('BH radius proxy [IMPOSED]');
  expect(cosmicPanel).toContain('2 G<sub>N</sub> M / c²');
});

test('black-hole render radius remains bounded and mass-linear before clamping', async ({ page }) => {
  await page.goto('/');
  const result = await page.evaluate(async () => {
    const { schwarzschildRenderRadius } = await import('/js/cosmic-renderer.js');
    return {
      r100: schwarzschildRenderRadius(100),
      r200: schwarzschildRenderRadius(200),
    };
  });

  expect(result.r100).toBeCloseTo(0.7, 12);
  expect(result.r200 / result.r100).toBeCloseTo(2, 12);
});

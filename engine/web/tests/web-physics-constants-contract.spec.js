import { test, expect } from '@playwright/test';

test('Higgs display mass follows the precision constant chain', async ({ page }) => {
  await page.goto('/js/constants.js');

  const result = await page.evaluate(async () => {
    const constants = await import('/js/constants.js');
    const catalog = await import('/js/particle-catalog.js');
    return {
      displayed: constants.M_HIGGS,
      fromChain: constants.N_EFF * constants.M_E
        / (constants.ALPHA * constants.ALPHA) / 1000,
      catalogFormula: catalog.getById('higgs').ftd_formula,
    };
  });

  expect(result.displayed).toBeCloseTo(result.fromChain, 12);
  expect(result.displayed).toBeCloseTo(124.75, 2);
  expect(result.catalogFormula).toContain(result.displayed.toFixed(2));
});

test('Scale 4 telemetry preserves an intentional zero gravity coupling', async ({ page }) => {
  await page.goto('/js/constants.js');

  const result = await page.evaluate(async () => {
    const [{ TelemetryHub }, { G_N }] = await Promise.all([
      import('/js/telemetry-hub.js?scale4-zero-gravity-contract=1'),
      import('/js/constants.js'),
    ]);
    const bodies = [
      { mass: 2, x: 0, y: 0, z: 0, vx: 0, vy: 0, vz: 0 },
      { mass: 3, x: 1, y: 0, z: 0, vx: 0, vy: 0, vz: 0 },
    ];
    const diagnostics = (tick) => ({ tick });

    const zeroHub = new TelemetryHub();
    zeroHub.collectScale4({ G: 0, _bodies: bodies, getDiagnostics: () => diagnostics(1) });

    const fallbackHub = new TelemetryHub();
    fallbackHub.collectScale4({ _bodies: bodies, getDiagnostics: () => diagnostics(1) });

    return {
      zeroPotential: zeroHub.plPE.last(),
      fallbackPotential: fallbackHub.plPE.last(),
      expectedFallback: -G_N * 2 * 3 / Math.sqrt(1 + 1e-6),
    };
  });

  expect(result.zeroPotential).toBe(0);
  // Telemetry ring buffers store chart channels as Float32.
  expect(result.fallbackPotential).toBeCloseTo(result.expectedFallback, 7);
});

test('Scale 4 toolbar displays the canonical decorative gravity coupling', async ({ page }) => {
  await page.goto('/js/constants.js');

  const result = await page.evaluate(async () => {
    const [{ G_N }, { getScale4ScenarioToolbarTemplate }] = await Promise.all([
      import('/js/constants.js'),
      import('/js/scales/scale4/ui/toolbar/template.js?gravity-contract=1'),
    ]);
    return {
      gravity: G_N,
      markup: getScale4ScenarioToolbarTemplate(),
    };
  });

  expect(result.markup).toContain(`G=${result.gravity}, lattice-natural`);
});

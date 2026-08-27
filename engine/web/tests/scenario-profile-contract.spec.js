import { test, expect } from '@playwright/test';

test('prepared Coulomb candidates share one substrate-carried term profile', async ({ page }) => {
  await page.goto('/');

  const result = await page.evaluate(async () => {
    const config = await import('/js/config/toggles.js');
    const ids = [
      's0-seed-hydrogen',
      's0-seed-helium',
      's0-seed-h2-bond-formation',
    ];
    return ids.map((id) => ({
      id,
      enabled: config.getScale0ScenarioToggleProfile(id)
        .filter(([, value]) => value)
        .map(([name]) => name),
    }));
  });

  const expected = [
    'wave_propagation',
    'coupling',
    'damping',
    'gauss_projection',
    'forces',
    'movement',
  ];
  for (const profile of result) expect(profile.enabled, profile.id).toEqual(expected);
});

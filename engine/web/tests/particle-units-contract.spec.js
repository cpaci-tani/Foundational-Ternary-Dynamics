import { test, expect } from '@playwright/test';

test('particle masses preserve neutrino units across constants and catalog formatting', async ({ page }) => {
  await page.goto('/');

  const result = await page.evaluate(async () => {
    const constants = await import('/js/constants.js');
    const catalog = await import('/js/particle-catalog.js');
    const units = await import('/js/units.js');
    const spectroscopy = await import('/js/spectroscopy.js');
    const { particleDataToList } = await import('/js/bridge/bridge-contract.js');
    const mapped = particleDataToList({
      positions: new Float32Array([1.5, 2.5, 3.5, 4.5, 5.5, 6.5]),
      colors: new Float32Array([0.29, 0.87, 0.5, 0.97, 0.44, 0.44]),
      spin: new Float32Array([1, -1]),
      colorCharge: new Float32Array([2, 3]),
      locked: new Uint8Array([1, 0]),
      count: 2,
    });
    return {
      lightestMeV: constants.M_NU_E_PHYS,
      lightestCatalogMeV: catalog.getById('nu_e').mass_mev,
      lightestLabel: catalog.formatMass(constants.M_NU_E_PHYS),
      middleLabel: units.formatEnergy(constants.M_NU_MU_PHYS, 1).text,
      electronLabel: catalog.formatMass(constants.M_E),
      mappedLocked: mapped.map((particle) => particle.locked),
      energyRatioHeH: spectroscopy.hydrogenEnergyLevel(2, 2)
        / spectroscopy.hydrogenEnergyLevel(2, 1),
      fineRatioHeH: spectroscopy.fineStructureCorrection(2, 1.5, 2)
        / spectroscopy.fineStructureCorrection(2, 1.5, 1),
      wavelengthRatioHeH: spectroscopy.spectralSeries(2, 2)[0].wavelength_nm
        / spectroscopy.spectralSeries(1, 2)[0].wavelength_nm,
    };
  });

  expect(result.lightestMeV).toBe(4.1e-15);
  expect(result.lightestCatalogMeV).toBe(result.lightestMeV);
  expect(result.lightestLabel).toContain('neV');
  expect(result.middleLabel).toContain('meV');
  expect(result.electronLabel).toContain('keV');
  expect(result.mappedLocked).toEqual([true, false]);
  expect(result.energyRatioHeH).toBeCloseTo(4, 12);
  expect(result.fineRatioHeH).toBeCloseTo(16, 12);
  expect(result.wavelengthRatioHeH).toBeCloseTo(0.25, 12);
});

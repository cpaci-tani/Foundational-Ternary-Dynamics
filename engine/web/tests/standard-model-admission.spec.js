import { test, expect } from '@playwright/test';
import { gotoAndReady, attachConsoleWatcher, realErrors } from './_helpers.js';

test('catalog injection preserves supported native charges and refuses substitutions', async ({ page }) => {
    test.setTimeout(120_000);
    const errors = attachConsoleWatcher(page);
    await gotoAndReady(page, {path: '/?engine=wasm', timeout: 90_000});
    await page.selectOption('#engine-mode', 'particles');
    await page.selectOption('#pe-scenario-select', 's1-empty-zoo');
    const result = await page.evaluate(async () => {
        const bridge = window.__ftdCtx.bridge;
        const {getById} = await import('/js/particle-catalog.js');
        const card = id => document.querySelector(`.zoo-inject-btn[data-particle="${id}"]`);
        const disabled = ['up','down','nu_e','photon'].map(id => card(id).disabled);
        card('delta_pp').click();
        const types = bridge.peGetParticleTypes();
        const id = [...types].find(([,name]) => name === 'delta_pp')?.[0];
        const record = bridge.peInspectParticle(id);
        const before = bridge.peParticleCount();
        const rejected = [
            bridge.peAddParticle('up', 2/3, 0,0,0, 0,0,0, getById('up').mass_mev, 0.1),
            bridge.peAddParticle('up', 1, 0,0,0, 0,0,0, getById('up').mass_mev, 0.1),
            bridge.peAddParticle('electron', 1, 0,0,0, 0,0,0, getById('electron').mass_mev, 0.1),
            bridge.peAddParticle('nu_e', 0, 0,0,0, 0,0,0, 1, 0.1),
        ];
        return {disabled, id, charge: record?.charge, mass: record?.mass,
            expectedMass: getById('delta_pp').mass_mev,
            flavorMass: card('nu_e').closest('.zoo-card').querySelector('.zoo-mass').textContent,
            rejected, before, after: bridge.peParticleCount()};
    });
    expect(result.disabled).toEqual([true,true,true,true]);
    expect(result.id).toBeGreaterThanOrEqual(0);
    expect(result.charge).toBe(2);
    expect(result.mass).toBe(result.expectedMass);
    expect(result.flavorMass).toBe('unavailable');
    expect(result.rejected).toEqual([-1,-1,-1,-1]);
    expect(result.after).toBe(result.before);
    await page.selectOption('#pe-scenario-select', 's1-quantum-color-triplet');
    const probes = await page.evaluate(() => {
        const bridge = window.__ftdCtx.bridge;
        return [...bridge.peGetParticleTypes()].map(([id, catalog]) => {
            const p = bridge.peInspectParticle(id);
            return {catalog, charge: p.charge, color: p.colorId};
        });
    });
    expect(probes).toEqual([1,2,3].map(color => ({catalog: null, charge: 0, color})));
    expect(realErrors(errors)).toEqual([]);
});

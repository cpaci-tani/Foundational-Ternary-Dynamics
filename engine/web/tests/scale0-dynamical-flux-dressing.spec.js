// @ts-check
/**
 * Browser/WASM contract for the FTD-0476 dynamical flux-dressing scenario.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test('native source builds the visualized field from zero initial flux', async ({ page }) => {
    test.setTimeout(90_000);
    await page.addInitScript(() => {
        window.__ftdWasmWorker = false;
        window.__ftdPrimeTickOnLoad = false;
    });
    await gotoAndReady(page);
    await selectScale0Scenario(page, 's0-seed-dynamical-flux-dressing');

    const result = await page.evaluate(async () => {
        const { resolveActiveScale0BridgeFromWindow, getScale0State } =
            await import('/js/scales/scale0/state/store.js');
        const bridge = resolveActiveScale0BridgeFromWindow();
        const caps = bridge?.capabilities?.scale0;
        const before = caps?.getScale0EnergyAudit?.();
        for (let i = 0; i < 12; i++) caps?.tickScale0?.();
        const after = caps?.getScale0EnergyAudit?.();
        const diagnostics = caps?.getScale0Diagnostics?.();
        const state = getScale0State();
        const toggleIds = [
            't-wave', 't-coupling', 't-damping', 't-gauss', 't-genesis',
            't-movement', 't-forces', 't-poisson', 't-gravity', 't-lorentz',
            't-confinement', 't-color-forces', 't-strong-force', 't-exchange',
            't-weak', 't-selective', 't-larmor', 't-dual',
        ];
        const overlay = (id) => {
            const button = document.getElementById(id);
            return {
                active: !!button?.classList.contains('active'),
                applicable: !!button && !button.classList.contains('is-inapplicable'),
            };
        };
        return {
            beforeField: Number(before?.fieldEnergy ?? 0),
            beforeWave: Number(before?.waveEnergy ?? 0),
            afterField: Number(after?.fieldEnergy ?? 0),
            afterWave: Number(after?.waveEnergy ?? 0),
            manifested: Number(diagnostics?.manifested ?? -1),
            wave: !!bridge?.getToggle?.('wave_propagation'),
            coupling: !!bridge?.getToggle?.('coupling'),
            gauss: !!bridge?.getToggle?.('gauss_projection'),
            movement: !!bridge?.getToggle?.('movement'),
            fluxLines: overlay('toggle-flux-lines'),
            stateField: overlay('toggle-state-field'),
            divergence: overlay('toggle-div-field'),
            stateFlags: {
                fluxLines: !!state.fieldFlags.showFluxLines,
                stateField: !!state.fieldFlags.showStateField,
                divergence: !!state.fieldFlags.showDivField,
            },
            domProfile: Object.fromEntries(toggleIds.map((id) => [
                id, !!document.getElementById(id)?.checked,
            ])),
            profileWarningHidden: !!document.getElementById('physics-profile-warning')?.hidden,
        };
    });

    expect(result.beforeField + result.beforeWave, 'initializer is exactly field-free').toBe(0);
    expect(result.afterField + result.afterWave, 'native source creates visible field activity').toBeGreaterThan(1e-8);
    expect(result.manifested, 'one locked source remains').toBe(1);
    expect(result).toMatchObject({ wave: true, coupling: true, gauss: false, movement: false });
    expect(result.domProfile).toEqual({
        't-wave': true,
        't-coupling': true,
        't-damping': false,
        't-gauss': false,
        't-genesis': false,
        't-movement': false,
        't-forces': false,
        't-poisson': false,
        't-gravity': false,
        't-lorentz': false,
        't-confinement': false,
        't-color-forces': false,
        't-strong-force': false,
        't-exchange': false,
        't-weak': false,
        't-selective': false,
        't-larmor': false,
        't-dual': false,
    });
    expect(result.profileWarningHidden, 'registered profile starts qualified').toBe(true);
    for (const key of ['fluxLines', 'stateField', 'divergence']) {
        expect(result[key].applicable, `${key} overlay applies`).toBe(true);
        expect(result[key].active, `${key} overlay is enabled by the visual profile`).toBe(true);
        expect(result.stateFlags[key], `${key} runtime flag is enabled`).toBe(true);
    }

    const profileGuard = await page.evaluate(() => {
        const gauss = /** @type {HTMLInputElement|null} */ (document.getElementById('t-gauss'));
        const restore = /** @type {HTMLButtonElement|null} */ (
            document.getElementById('btn-reset-physics-toggles'));
        gauss?.click();
        const modified = {
            gauss: !!gauss?.checked,
            warningVisible: !document.getElementById('physics-profile-warning')?.hidden,
            metadata: document.getElementById('lat-scenario-desc-text')?.textContent ?? '',
        };
        restore?.click();
        return {
            modified,
            restored: {
                wave: !!document.getElementById('t-wave')?.checked,
                coupling: !!document.getElementById('t-coupling')?.checked,
                gauss: !!gauss?.checked,
                warningHidden: !!document.getElementById('physics-profile-warning')?.hidden,
                metadata: document.getElementById('lat-scenario-desc-text')?.textContent ?? '',
            },
        };
    });
    expect(profileGuard.modified.gauss).toBe(true);
    expect(profileGuard.modified.warningVisible).toBe(true);
    expect(profileGuard.modified.metadata).toContain('QUALIFICATION SUSPENDED');
    expect(profileGuard.restored).toMatchObject({
        wave: true,
        coupling: true,
        gauss: false,
        warningHidden: true,
    });
    expect(profileGuard.restored.metadata).not.toContain('QUALIFICATION SUSPENDED');
});

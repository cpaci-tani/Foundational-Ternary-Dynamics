// @ts-check
/* global window, document */
/**
 * Browser/WASM contract for the FTD-0477 qualified-negative discriminator.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test('driven polarity exposes the selected sub-voxel response without promoting it to a wake', async ({ page }) => {
    test.setTimeout(90_000);
    await page.addInitScript(() => {
        window.__ftdWasmWorker = false;
        window.__ftdPrimeTickOnLoad = false;
    });
    await gotoAndReady(page);
    await selectScale0Scenario(page, 's0-seed-moving-source-reciprocity');

    const result = await page.evaluate(() => {
        const bridge = window._ftdBridge;
        const caps = bridge?.capabilities?.scale0;
        const position = () => {
            const data = bridge?.getParticleData?.();
            if (!data || data.count !== 1) return null;
            return [
                Number(data.positions[0]),
                Number(data.positions[1]),
                Number(data.positions[2]),
            ];
        };
        const before = position();
        for (let tick = 0; tick < 72; tick++) caps?.tickScale0?.();
        const after = position();
        const overlay = (id) => {
            const button = document.getElementById(id);
            return {
                active: !!button?.classList.contains('active'),
                applicable: !!button && !button.classList.contains('is-inapplicable'),
            };
        };
        const visibleToggles = [
            't-wave', 't-coupling', 't-damping', 't-gauss', 't-genesis',
            't-movement', 't-forces', 't-poisson', 't-gravity', 't-lorentz',
            't-confinement', 't-color-forces', 't-strong-force', 't-exchange',
            't-weak', 't-selective', 't-larmor', 't-dual',
        ];
        return {
            before,
            after,
            manifested: Number(caps?.getScale0Diagnostics?.()?.manifested ?? -1),
            selectedForce: !!bridge?.getToggle?.('emergent_forces'),
            strict: !!bridge?.getToggle?.('strict_validation'),
            profile: Object.fromEntries(visibleToggles.map((id) => [
                id, !!document.getElementById(id)?.checked,
            ])),
            overlays: {
                fluxLines: overlay('toggle-flux-lines'),
                state: overlay('toggle-state-field'),
                fieldChange: overlay('toggle-e-field'),
                poynting: overlay('toggle-poynting'),
            },
            scenarioLabel: document.querySelector('#scenario-select option:checked')?.textContent ?? '',
            description: document.getElementById('lat-scenario-desc-text')?.textContent ?? '',
            warningHidden: !!document.getElementById('physics-profile-warning')?.hidden,
        };
    });

    expect(result.before).not.toBeNull();
    expect(result.after).not.toBeNull();
    const displacement = Math.hypot(
        result.after[0] - result.before[0],
        result.after[1] - result.before[1],
        result.after[2] - result.before[2],
    );
    expect(displacement, 'WASM renders the production sub-voxel remainder').toBeGreaterThan(1e-4);
    expect(result.manifested).toBe(1);
    expect(result.selectedForce).toBe(true);
    expect(result.strict).toBe(true);
    expect(result.profile).toEqual({
        't-wave': true,
        't-coupling': true,
        't-damping': false,
        't-gauss': false,
        't-genesis': false,
        't-movement': true,
        't-forces': true,
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
    expect(result.warningHidden).toBe(true);
    expect(result.scenarioLabel).toContain('A Nudged Charge Responds');
    expect(result.description).toContain('no integer hop');
    for (const [name, state] of Object.entries(result.overlays)) {
        expect(state.applicable, `${name} overlay applies`).toBe(true);
        expect(state.active, `${name} overlay is enabled`).toBe(true);
    }
});

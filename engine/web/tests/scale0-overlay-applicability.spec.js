// @ts-check
/**
 * Scale-0 scenario-aware overlay availability.
 *
 * These tests pin the distinction between a valid zero-valued diagnostic and
 * a diagnostic whose source quantity does not exist in the selected scenario.
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test.setTimeout(120_000);

async function waitForOverlayScenario(page, scenarioId) {
    await page.waitForFunction(
        (id) => document.getElementById('viewport-overlay')?.dataset.scenarioId === id,
        scenarioId,
    );
}

async function overlayState(page, ids) {
    return page.evaluate((buttonIds) => Object.fromEntries(buttonIds.map((id) => {
        const btn = document.getElementById(id);
        return [id, {
            present: !!btn,
            applicable: !!btn && !btn.classList.contains('is-inapplicable'),
            active: !!btn?.classList.contains('active'),
            display: btn ? getComputedStyle(btn).display : null,
        }];
    })), ids);
}

test.beforeEach(async ({ page }) => {
    page.setDefaultTimeout(60_000);
    await gotoAndReady(page);
    await waitForOverlayScenario(page, 'flux-pulse');
});

test('pure-wave scenario exposes field diagnostics but hides absent matter features', async ({ page }) => {
    const states = await overlayState(page, [
        'toggle-flux-volume', 'toggle-e-field', 'toggle-vorticity',
        'toggle-state-field', 'toggle-force-em', 'toggle-force-gravity',
        'toggle-phase', 'toggle-genesis-iso', 'toggle-damping-zones',
        'toggle-color-charge', 'toggle-confinement', 'toggle-latency', 'toggle-horizon',
    ]);

    for (const id of ['toggle-flux-volume', 'toggle-e-field', 'toggle-vorticity']) {
        expect(states[id].applicable, `${id} should apply to a pure wave`).toBe(true);
    }
    for (const id of [
        'toggle-state-field', 'toggle-force-em', 'toggle-force-gravity',
        'toggle-phase', 'toggle-genesis-iso', 'toggle-damping-zones',
        'toggle-color-charge', 'toggle-confinement', 'toggle-latency', 'toggle-horizon',
    ]) {
        expect(states[id].applicable, `${id} should not apply to a pure wave`).toBe(false);
        expect(states[id].display, `${id} should be hidden`).toBe('none');
    }
});

test('canonical mixed seeds retain J and state channels independently of live terms or particles', async ({ page }) => {
    const scenarioIds = [
        'flux-annihilation',
        'flux-meson',
        'flux-string-breaking',
        'flux-baryon',
        'quantum-entangle',
        's0-seed-ee-annihilation',
        's0-seed-hydrogen',
        's0-seed-helium',
        's0-seed-h2-bond-formation',
        's0-seed-sloop',
        's0-vacuum-proton',
        's0-vacuum-neutron',
        's0-vacuum-pion-charged',
        's0-vacuum-pion-neutral',
        's0-vacuum-kaon-charged',
    ];
    const result = await page.evaluate(async (ids) => {
        const { getScale0OverlayApplicability } = await import(
            '/js/scales/scale0/ui/overlays/applicability.js?mixed-seed-domains-test=1'
        );
        return Object.fromEntries(ids.map((id) => {
            // An all-off engine readback models inert/tick-0 profiles and the
            // post-annihilation state. Seed-domain metadata must still win.
            const profile = getScale0OverlayApplicability(id, {
                wave_propagation: false,
                coupling: false,
                gauss_projection: false,
                genesis: false,
                forces: false,
                color_forces: false,
                strong_force: false,
                confinement: false,
                weak_transmutation: false,
            });
            return [id, {
                flux: profile.domains.flux,
                state: profile.domains.state,
                volume: profile.applicable.has('toggle-flux-volume'),
                stateField: profile.applicable.has('toggle-state-field'),
            }];
        }));
    }, scenarioIds);

    for (const id of scenarioIds) {
        expect(result[id], `${id} must preserve both canonical seed domains`).toEqual({
            flux: true,
            state: true,
            volume: true,
            stateField: true,
        });
    }
});

test('Wilson loop is a flux-only seed with no blank state channel', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const { getScale0OverlayApplicability } = await import(
            '/js/scales/scale0/ui/overlays/applicability.js?wilson-flux-domain-test=1'
        );
        const profile = getScale0OverlayApplicability('s0-seed-wilson-loop', {
            wave_propagation: false,
            coupling: false,
            gauss_projection: false,
            genesis: false,
            forces: false,
        });
        return {
            flux: profile.domains.flux,
            state: profile.domains.state,
            volume: profile.applicable.has('toggle-flux-volume'),
            fluxLines: profile.applicable.has('toggle-flux-lines'),
            stateField: profile.applicable.has('toggle-state-field'),
        };
    });

    expect(result).toEqual({
        flux: true,
        state: false,
        volume: true,
        fluxLines: true,
        stateField: false,
    });
});

test('scenario switch suspends an incompatible overlay and restores its preference later', async ({ page }) => {
    await page.evaluate(() => {
        const btn = document.getElementById('toggle-e-field');
        if (btn && !btn.classList.contains('active')) btn.click();
    });

    await selectScale0Scenario(page, 's0-seed-massive-body');
    await waitForOverlayScenario(page, 's0-seed-massive-body');

    const stateOnly = await page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const e = document.getElementById('toggle-e-field');
        const s = document.getElementById('toggle-state-field');
        return {
            eApplicable: !e?.classList.contains('is-inapplicable'),
            ePreferenceRetained: !!e?.classList.contains('active'),
            eRuntimeEnabled: getScale0State().fieldFlags.showEField,
            stateApplicable: !s?.classList.contains('is-inapplicable'),
            activeStrip: document.getElementById('s0-overlay-active')?.textContent || '',
        };
    });
    expect(stateOnly).toMatchObject({
        eApplicable: false,
        ePreferenceRetained: true,
        eRuntimeEnabled: false,
        stateApplicable: true,
    });
    // Overlay label renamed "E Field" → "Radiative E (−∂J/∂t)" (radiative-only clarity, 2026-08-21).
    // The active-strip chip is the button's textContent (panel-shell.js makeChip), so key the
    // absence check on the current label — otherwise it would pass trivially and stop catching a
    // wrongly-active E-field overlay.
    expect(stateOnly.activeStrip).not.toContain('Radiative E');

    await selectScale0Scenario(page, 'flux-pulse');
    await waitForOverlayScenario(page, 'flux-pulse');
    const restored = await page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const e = document.getElementById('toggle-e-field');
        return {
            applicable: !e?.classList.contains('is-inapplicable'),
            active: !!e?.classList.contains('active'),
            runtimeEnabled: getScale0State().fieldFlags.showEField,
        };
    });
    expect(restored).toEqual({ applicable: true, active: true, runtimeEnabled: true });
});

test('matter, strong, and gravity scenarios expose only their native channels', async ({ page }) => {
    await selectScale0Scenario(page, 's0-vacuum-proton');
    await waitForOverlayScenario(page, 's0-vacuum-proton');
    const proton = await overlayState(page, [
        'toggle-state-field', 'toggle-force-em', 'toggle-force-strong',
        'toggle-color-charge', 'toggle-confinement', 'toggle-flux-volume',
        'toggle-force-gravity', 'toggle-latency',
    ]);
    for (const id of [
        'toggle-state-field', 'toggle-force-em', 'toggle-force-strong',
        'toggle-color-charge', 'toggle-confinement', 'toggle-flux-volume',
    ]) expect(proton[id].applicable, `${id} should apply to the proton cohort`).toBe(true);
    for (const id of ['toggle-force-gravity', 'toggle-latency']) {
        expect(proton[id].applicable, `${id} should be hidden for the proton cohort`).toBe(false);
    }

    await selectScale0Scenario(page, 's0-seed-massive-body');
    await waitForOverlayScenario(page, 's0-seed-massive-body');
    const massive = await overlayState(page, [
        'toggle-state-field', 'toggle-flux-volume', 'toggle-force-em',
        'toggle-force-gravity', 'toggle-grav-potential', 'toggle-latency', 'toggle-horizon',
        'toggle-force-strong', 'toggle-color-charge', 'toggle-confinement',
    ]);
    // The engine profile for this scenario is `only_terms_enabled(rb, {"gravity",
    // "latency_field"})` (engine/tests/test_scenario_behavior.cpp). Its seed is
    // IP + LOCK only — 33 locked manifested voxels, no flux write — so the state
    // field is populated while every flux, EM and strong channel is genuinely
    // absent. These expectations follow the engine, not the JS toggle model.
    for (const id of [
        'toggle-state-field',
        'toggle-force-gravity', 'toggle-grav-potential', 'toggle-latency', 'toggle-horizon',
    ]) expect(massive[id].applicable, `${id} should apply to the massive body`).toBe(true);
    for (const id of [
        'toggle-flux-volume', 'toggle-force-em',
        'toggle-force-strong', 'toggle-color-charge', 'toggle-confinement',
    ]) {
        expect(massive[id].applicable, `${id} should be hidden for the massive body`).toBe(false);
    }
});

test('null control hides every category and reports why the panel is empty', async ({ page }) => {
    await selectScale0Scenario(page, 'empty');
    await waitForOverlayScenario(page, 'empty');

    const result = await page.evaluate(() => {
        const body = document.querySelector('.s0-overlay-body');
        const cols = [...document.querySelectorAll('.s0-overlay-col')];
        return {
            empty: body?.classList.contains('is-applicability-empty'),
            allColumnsHidden: cols.length > 0 && cols.every((col) => getComputedStyle(col).display === 'none'),
            activeStripHidden: !!document.getElementById('s0-overlay-active')?.hidden,
            domains: document.getElementById('viewport-overlay')?.dataset.overlayDomains,
        };
    });
    expect(result).toEqual({
        empty: true,
        allColumnsHidden: true,
        activeStripHidden: true,
        domains: '',
    });
});

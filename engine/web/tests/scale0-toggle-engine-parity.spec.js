// @ts-check
/**
 * The physics-toggles card must report engine state, not the JS model of it.
 *
 * `WasmBridge.setupScenario` calls `reset()`, which reconstructs the RenderBridge
 * at C++ defaults; each `configure_*_terms` helper then zeroes every TOGGLE_SPECS
 * entry and sets its own profile. The C++ body is therefore the only truth about
 * which terms are live. The dashboard, however, paints its checkboxes from
 * `SCALE0_TOGGLES` + `SCALE0_SCENARIO_OVERRIDES` and never reads the engine back.
 *
 * Expected values here are pinned to the committed C++ evidence tests, not to
 * either JS table, so this cannot be satisfied by making the two JS models agree:
 *
 *   s0-seed-massive-body  engine/tests/test_scenario_behavior.cpp
 *                         `only_terms_enabled(rb, {"gravity", "latency_field"})`
 *   s0-seed-octahedron    engine/tests/test_scenario_behavior.cpp
 *                         `only_terms_enabled(rb, {})`
 *
 * (`latency_field` has no dashboard checkbox and is not in SCALE0_TOGGLES.)
 */

import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

/** key -> checkbox id, mirroring SCALE0_TOGGLES in js/config/toggles.js. */
const TOGGLE_ELEMENTS = {
    wave_propagation: 't-wave',
    coupling: 't-coupling',
    damping: 't-damping',
    genesis: 't-genesis',
    evaporation: 't-evaporation',
    gauss_projection: 't-gauss',
    forces: 't-forces',
    gravity: 't-gravity',
    movement: 't-movement',
    poisson_coulomb: 't-poisson',
    lorentz_force: 't-lorentz',
    selective_damping: 't-selective',
    larmor_radiation: 't-larmor',
    dual_substrate: 't-dual',
    confinement: 't-confinement',
    color_forces: 't-color-forces',
    strong_force: 't-strong-force',
    exchange_force: 't-exchange',
    weak_transmutation: 't-weak',
};

/** Scenario -> the engine terms the committed C++ test says are live. */
const ENGINE_TRUTH = {
    's0-seed-massive-body': ['gravity'],
    's0-seed-octahedron': [],
};

async function readToggleCheckboxes(page, elementMap) {
    return page.evaluate((map) => {
        const out = {};
        for (const [key, elId] of Object.entries(map)) {
            const el = document.getElementById(elId);
            if (el instanceof HTMLInputElement) out[key] = el.checked;
        }
        return out;
    }, elementMap);
}

test.beforeEach(async ({ page }, testInfo) => {
    testInfo.setTimeout(180_000);
    page.setDefaultTimeout(60_000);
    await gotoAndReady(page);
});

for (const [scenarioId, liveTerms] of Object.entries(ENGINE_TRUTH)) {
    test(`physics-toggles card matches the engine profile for ${scenarioId}`, async ({ page }) => {
        await selectScale0Scenario(page, scenarioId);

        await expect.poll(
            async () => {
                const checked = await readToggleCheckboxes(page, TOGGLE_ELEMENTS);
                return Object.entries(checked)
                    .filter(([, on]) => on)
                    .map(([key]) => key)
                    .sort();
            },
            {
                message: `checkbox state for ${scenarioId} must equal the engine's live terms `
                    + `(${liveTerms.length ? liveTerms.join(', ') : 'none'})`,
                timeout: 30_000,
            },
        ).toEqual([...liveTerms].sort());
    });
}

test('overlay applicability for a gravity-only scenario excludes flux and EM channels', async ({ page }) => {
    await selectScale0Scenario(page, 's0-seed-massive-body');
    await page.waitForFunction(
        () => document.getElementById('viewport-overlay')?.dataset.scenarioId === 's0-seed-massive-body',
    );

    await expect.poll(async () => page.evaluate(() => {
        const state = (id) => {
            const btn = document.getElementById(id);
            return !!btn && !btn.classList.contains('is-inapplicable');
        };
        return {
            gravity: state('toggle-force-gravity'),
            latency: state('toggle-latency'),
            fluxVolume: state('toggle-flux-volume'),
            forceEm: state('toggle-force-em'),
            stateField: state('toggle-state-field'),
        };
    }), { timeout: 30_000 }).toEqual({
        // gravity + latency_field are the only live terms per the C++ evidence test
        gravity: true,
        latency: true,
        // wave_propagation/coupling/gauss are all off and the seed body writes no
        // flux (IP + LOCK only — s0_seed.cpp: "a locked-rest-mass body has zero
        // flux"), so there is no flux channel to inspect
        fluxVolume: false,
        // forces/poisson_coulomb/lorentz_force are all off, so no EM force exists
        forceEm: false,
        // Applicable, and deliberately so: state applicability tracks manifested
        // CONTENT, not state-producing terms. genesis is off, but the seed
        // manifests 33 locked voxels (test_scenario_behavior.cpp: "locked mass
        // probe seeds the exact compact 33-site ball"), so the state field is
        // populated and must stay inspectable.
        stateField: true,
    });
});

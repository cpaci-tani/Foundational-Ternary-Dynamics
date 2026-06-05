// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

/**
 * WASM scenario coverage test.
 *
 * Before the scenarios.cpp port, only 15 of the 84 UI-exposed Scale-0
 * scenarios were implemented in the C++ WASM engine; the other 69
 * silently no-op'd (users with WASM backend picked s0-seed-hydrogen
 * and got an empty lattice with no console error).
 *
 * After the port (engine/src/scenarios.cpp, April 2026), all 83 JS
 * scenarios have C++ implementations. This test verifies that the
 * WASM backend actually executes each one by loading it, ticking the
 * engine a few times, and asserting that at least one voxel now has
 * non-zero flux OR at least one particle was manifested.
 *
 * Coverage sample: we pick 3-5 representative scenarios from each
 * newly-ported group (flux-/light-/quantum-/s0-seed-/s0-field-) so
 * the test runs in < 30 s on a modern laptop yet still catches any
 * compile-time coverage regression.
 */

const NEWLY_PORTED_SCENARIOS = [
    // flux-* — previously WASM-supported subset (baseline)
    ['flux-pulse',            'baseline (pre-port)',  'flux'],
    // flux-* — newly ported
    ['flux-meson',            'newly ported',  'flux'],
    ['flux-baryon',           'newly ported',  'flux'],
    ['flux-string-breaking',  'newly ported',  'flux'],
    ['flux-cyclotron',        'newly ported',  'flux'],
    ['flux-triad',            'newly ported',  'flux'],
    ['flux-vacuum-foam',      'newly ported',  'flux'],
    ['flux-zero-point',       'newly added',   'flux'],
    ['flux-annihilation',     'newly ported',  'flux'],
    ['flux-nested-standing',  'newly ported',  'flux'],
    // light-* — mostly baseline (already supported) but light-prism was dropped
    ['light-rainbow',         'baseline',      'light'],
    ['light-two-slit',        'baseline',      'light'],
    // quantum-* — ALL newly ported (no WASM support before)
    ['quantum-born-rule',     'newly ported',  'quantum'],
    ['quantum-double-slit',   'newly ported',  'quantum'],
    ['quantum-tunnel',        'newly ported',  'quantum'],
    ['quantum-well',          'newly ported',  'quantum'],
    ['quantum-entangle',      'newly ported',  'quantum'],
    ['quantum-casimir',       'newly ported',  'quantum'],
    // s0-field-* — ALL newly ported
    ['s0-field-plane-wave',   'newly ported',  's0-field'],
    ['s0-field-uniform-e',    'newly ported',  's0-field'],
    ['s0-field-uniform-b',    'newly ported',  's0-field'],
    ['s0-field-electric-dipole', 'newly ported', 's0-field'],
    ['s0-field-vortex-line',  'newly ported',  's0-field'],
    // s0-vacuum-* + s0-seed-* — sample spans all sub-categories.
    // (Audit-3 + Audit-4 2026-04-28: many seed mirrors removed; replaced
    //  with their s0-vacuum-* canonical counterparts.)
    ['s0-vacuum-electron',    'canonical',     'lepton'],
    ['s0-vacuum-muon',        'canonical',     'lepton'],
    ['s0-vacuum-photon',      'canonical',     'gauge boson'],
    ['s0-vacuum-proton',      'canonical',     'baryon'],
    ['s0-seed-octahedron',    'newly ported',  's0-seed Moore shell'],
    ['s0-seed-cuboctahedron', 'newly ported',  's0-seed Moore shell'],
    ['s0-seed-stella-octangula', 'newly ported', 's0-seed Moore shell'],
    ['s0-seed-moore-cell',    'newly ported',  's0-seed Moore cell'],
    ['s0-seed-moore-decomposition', 'newly ported', 's0-seed Moore decomp'],
    ['s0-seed-hydrogen',      'newly ported',  's0-seed atom'],
    ['s0-seed-helium',        'newly ported',  's0-seed atom'],
    ['s0-vacuum-higgs',       'canonical',     'gauge boson'],
    ['s0-vacuum-w-boson',     'canonical',     'gauge boson'],
    ['s0-vacuum-z-boson',     'canonical',     'gauge boson'],
    ['s0-seed-gluon',         'newly ported',  's0-seed SM'],
    ['s0-seed-up-quark',      'newly ported',  's0-seed quark'],
    ['s0-seed-top-quark',     'newly ported',  's0-seed quark'],
    ['s0-seed-wilson-loop',   'newly ported',  's0-seed gauge'],
    ['s0-seed-flux-tube',     'newly ported',  's0-seed gauge'],
    ['s0-seed-schwarzschild', 'newly ported',  's0-seed gravity'],
    ['s0-seed-sloop',         'newly ported',  's0-seed reference frame context'],
    ['s0-seed-observer-cell', 'newly ported',  's0-seed reference frame context'],
];

test.describe('WASM Scale-0 scenario coverage', () => {
    test.beforeEach(async ({ page }) => {
        page.on('pageerror', (e) => console.error('PAGEERROR:', e.message));
        await gotoAndReady(page);
        // Confirm we're on the WASM bridge, not MockBridge fallback.
        const isWasm = await page.evaluate(() => !!window._ftdBridge?.isWasm);
        if (!isWasm) {
            test.skip(true, 'WASM bridge not active — coverage test requires real WASM backend.');
        }
    });

    for (const [name, tag, group] of NEWLY_PORTED_SCENARIOS) {
        test(`${name.padEnd(32)} [${tag}, group: ${group}]`, async ({ page }) => {
            const result = await page.evaluate((scenarioName) => {
                const b = window._ftdBridge;
                b.setupScenario(scenarioName);
                // Advance 3 ticks so flux integrates / particles register.
                for (let i = 0; i < 3; i++) b.tick();
                const diag = b.getDiagnostics();
                // Measure scenario footprint: total flux magnitude + particle count.
                return {
                    totalEnergy: diag.totalEnergy || 0,
                    totalFlux:   diag.totalFlux   || 0,
                    manifested:  diag.manifested  || 0,
                    tick:        diag.tick        || 0,
                };
            }, name);

            // The scenario must have DONE SOMETHING — either injected flux
            // (non-zero totalFlux/totalEnergy) or manifested at least one
            // particle. An empty result means WASM silently ignored the name.
            const didSomething = (result.totalFlux > 0.01) ||
                                 (result.totalEnergy > 0.01) ||
                                 (result.manifested > 0);
            expect(didSomething,
                `Scenario '${name}' produced flux=${result.totalFlux} energy=${result.totalEnergy} particles=${result.manifested} — WASM setup_scenario appears to have silently no-op'd.`)
                .toBe(true);
        });
    }
});

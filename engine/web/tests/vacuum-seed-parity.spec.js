// @ts-check
/**
 * Vacuum-vs-seed body drift guard.
 *
 * The 10 wrapper scenarios in vacuum-scenarios.js are intentional verbatim
 * mirrors of their s0-seed-* counterparts (per SPEC_VACUUM_PARTICLE_SCENARIOS.md).
 * This guard runs both via the live WasmBridge, captures totalFlux + manifested
 * at tick=30, and asserts they match. If someone edits one body without
 * updating the other, the test fails — preventing silent drift.
 *
 * Pairs are tested at L=32 (default lattice) with WasmBridge active.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const PAIRS = [
    ['s0-vacuum-electron',       's0-seed-electron'],
    ['s0-vacuum-muon',           's0-seed-muon'],
    ['s0-vacuum-tau',            's0-seed-tau'],
    ['s0-vacuum-photon',         's0-seed-photon'],
    ['s0-vacuum-w-boson',        's0-seed-w-boson'],
    ['s0-vacuum-z-boson',        's0-seed-z-boson'],
    ['s0-vacuum-higgs',          's0-seed-higgs-boson'],
    ['s0-vacuum-proton',         's0-seed-proton-l4'],
    ['s0-vacuum-neutron',        's0-seed-neutron'],
    ['s0-vacuum-pion-charged',   's0-seed-pion'],
];

const FLUX_REL_TOL = 1e-3;  // matches MockBridge JS↔WASM rounding

test.describe('vacuum-seed body parity', () => {
    test.beforeEach(async ({ page }) => {
        page.on('pageerror', (e) => console.error('PAGEERROR:', e.message));
        await gotoAndReady(page);
    });

    for (const [vacName, seedName] of PAIRS) {
        test(`${vacName} ≡ ${seedName} (body parity)`, async ({ page }) => {
            const result = await page.evaluate(async ([vac, seed]) => {
                const b = window._ftdBridge;
                const measure = (name) => {
                    b.setupScenario(name);
                    for (let t = 0; t < 30; t++) b.tick();
                    const d = b.getDiagnostics();
                    return {
                        totalEnergy: +d.totalEnergy.toFixed(6),
                        totalFlux: +d.totalFlux.toFixed(6),
                        manifested: d.manifested | 0,
                    };
                };
                return { vac: measure(vac), seed: measure(seed) };
            }, [vacName, seedName]);

            // Particle count must match exactly.
            expect(result.vac.manifested,
                `${vacName} manifested=${result.vac.manifested} vs ${seedName} ${result.seed.manifested}`)
                .toBe(result.seed.manifested);

            // totalFlux must match within tight relative tolerance.
            // Allowing 0 on either side as a special case (some scenarios are
            // pure manifested-state with no envelope).
            const denom = Math.max(Math.abs(result.seed.totalFlux), 1e-6);
            const relDiff = Math.abs(result.vac.totalFlux - result.seed.totalFlux) / denom;
            expect(relDiff,
                `${vacName} flux=${result.vac.totalFlux} vs ${seedName} ${result.seed.totalFlux} (relDiff=${relDiff.toExponential(3)})`)
                .toBeLessThan(FLUX_REL_TOL);
        });
    }
});

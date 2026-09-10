// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

// Browser-level coverage for the four Scale 3 liquid-transport laboratories
// (Task 2 of the hydro-scenarios-part-b brief): mol-liquid-shear-layer,
// mol-liquid-channel-decay, mol-liquid-droplet-diffusion,
// mol-liquid-spinning-droplet. The Node-level suite
// (scale3-liquid-labs.node.test.mjs) already exercises the estimators,
// protocol wiring, and telemetry hookup against a bare createAtomEngine()
// stub; this spec drives the same code paths through the real production
// DOM (mol-scenario-select change handler -> scale3/controller.js
// loadMoleculeScenario -> telemetryHub.attachLiquidTracker) inside an actual
// Chromium page, using window._ftdBridge exactly as a user's browser would.
const LIQUID_IDS = [
    'mol-liquid-shear-layer',
    'mol-liquid-channel-decay',
    'mol-liquid-droplet-diffusion',
    'mol-liquid-spinning-droplet',
];

test.describe('Scale 3 liquid-transport laboratories', () => {
    test('all four labs load their seed topology, run the molecular-liquid-transport protocol, and publish a measuring estimate', async ({ page }) => {
        test.setTimeout(240_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'molecules');

        for (const id of LIQUID_IDS) {
            const started = Date.now();
            // eslint-disable-next-line no-await-in-loop
            const result = await page.evaluate(async (scenarioId) => {
                const registry = await import('/js/scales/scale3/scenario-registry.js');
                // Same module instance the app's own animateAE loop uses
                // (controller.js:53 imports the identical URL), so the
                // `active` experiment state set by the real scenario-select
                // change handler is shared with this in-page tick loop.
                const { advanceAEExperiment } = await import('/js/scales/scale2/experiment-runtime.js');
                const { telemetryHub } = await import('/js/telemetry-hub.js');
                const scenario = registry.SCALE3_SCENARIOS.find((s) => s.id === scenarioId);
                const select = /** @type {HTMLSelectElement} */ (document.getElementById('mol-scenario-select'));
                const bridge = window._ftdBridge;

                select.value = scenarioId;
                select.dispatchEvent(new Event('change', { bubbles: true }));

                const initial = bridge.aeGetDiagnostics();
                const molecule = bridge.aeGetMoleculeDiagnostics();
                const failures = [];
                if (initial.atomCount !== scenario.expected.atomCount) {
                    failures.push(`atomCount ${initial.atomCount} != ${scenario.expected.atomCount}`);
                }
                if (initial.bondCount !== scenario.expected.bondCount) {
                    failures.push(`bondCount ${initial.bondCount} != ${scenario.expected.bondCount}`);
                }
                if (molecule.componentCount !== scenario.expected.componentCount) {
                    failures.push(`componentCount ${molecule.componentCount} != ${scenario.expected.componentCount}`);
                }

                // Drive the exact per-tick hook the production animateAE loop
                // runs (controller.js: bridge.aeTick(); advanceAEExperiment(bridge);),
                // and mirror the app's telemetry cadence (collectScale2 every
                // 3rd frame) so the window's sample count matches production.
                const targetTicks = scenario.liquid.window.start + 200;
                for (let tick = 1; tick <= targetTicks; tick++) {
                    bridge.aeTick();
                    advanceAEExperiment(bridge);
                    if (tick % 3 === 0) telemetryHub.collectScale2(bridge);
                }

                return {
                    failures,
                    targetTicks,
                    lastError: bridge.aeGetDiagnostics().lastError,
                    liquid: telemetryHub.s2.liquid,
                };
            }, id);
            const wallMs = Date.now() - started;

            expect(result.failures, `${id}: seed/topology mismatch: ${result.failures.join('; ')}`).toEqual([]);
            expect(result.lastError, `${id}: engine reported an error after ${result.targetTicks} ticks`).toBe('ok');
            expect(result.liquid, `${id}: no liquid telemetry published`).toBeTruthy();
            expect(result.liquid?.status, `${id}: liquid status ${JSON.stringify(result.liquid)}`).toBe('measuring');
            expect(Number.isFinite(result.liquid?.value), `${id}: non-finite value ${result.liquid?.value}`).toBe(true);
            expect(result.liquid?.se, `${id}: negative standard error`).toBeGreaterThanOrEqual(0);
            expect(result.liquid?.samples, `${id}: fewer than 4 samples`).toBeGreaterThanOrEqual(4);

            // Surfaced for the task report, not asserted on (no SLA in the brief).
            console.log(`[liquid-transport] ${id}: ${wallMs}ms, ${JSON.stringify(result.liquid)}`);
        }

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('mol-liquid-droplet-diffusion reproduces bit-identical dynamics across repeated loads (seed 0x030103)', async ({ page }) => {
        await gotoAndReady(page);
        await switchMode(page, 'molecules');

        const runOnce = () => page.evaluate(async () => {
            const { advanceAEExperiment } = await import('/js/scales/scale2/experiment-runtime.js');
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('mol-scenario-select'));
            const bridge = window._ftdBridge;

            select.value = 'mol-liquid-droplet-diffusion';
            select.dispatchEvent(new Event('change', { bubbles: true }));

            for (let tick = 1; tick <= 100; tick++) {
                bridge.aeTick();
                advanceAEExperiment(bridge);
            }

            const diag = bridge.aeGetDiagnostics();
            const atoms = bridge.aeGetAtomData();
            return {
                totalEnergy: diag.totalEnergy,
                lastError: diag.lastError,
                firstAtom: [atoms.positions[0], atoms.positions[1], atoms.positions[2]],
            };
        });

        const first = await runOnce();
        const second = await runOnce();

        expect(first.lastError).toBe('ok');
        expect(second.lastError).toBe('ok');
        // Bit-identical, not tolerance-matched: the seeded LCG (0x030103) plus
        // a deterministic engine must reproduce the exact same trajectory on
        // reload. A divergence here is a real defect, not noise to paper over.
        expect(second.totalEnergy).toBe(first.totalEnergy);
        expect(second.firstAtom).toEqual(first.firstAtom);
    });
});

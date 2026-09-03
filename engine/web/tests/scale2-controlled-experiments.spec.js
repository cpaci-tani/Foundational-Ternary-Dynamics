// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function loadExperiment(page, id, expectedAtoms) {
    await page.evaluate((scenarioId) => {
        const select = /** @type {HTMLSelectElement | null} */ (
            document.getElementById('ae-scenario-select'));
        if (!select) throw new Error('ae-scenario-select not found');
        select.value = scenarioId;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await expect.poll(
        () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
        { timeout: 10_000, message: `${id} did not seed its expected atoms` },
    ).toBe(expectedAtoms);
}

test.describe('Scale 2 controlled experiment laboratories', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
    });

    test('registry publishes four explicit experiment contracts and telemetry phases', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const registry = await import('./js/scales/scale2/scenario-registry.js');
            const ids = [
                'ae-bond-rupture-cycle',
                'ae-argon-thermal-cycle',
                'ae-crystal-impulse-vacancy',
                'ae-u235-criticality-controls',
            ];
            return {
                contracts: ids.map(id => {
                    const item = registry.getAEScenarioMeta(id);
                    return {
                        id: item?.id,
                        status: item?.epistemicStatus,
                        protocol: item?.experiment?.protocol,
                        phases: item?.experiment?.phases,
                        evidence: item?.evidence,
                    };
                }),
                validation: registry.validateAEScenarioRegistry(),
            };
        });

        expect(result.validation.ok, result.validation.errors.join('\n')).toBe(true);
        expect(result.validation.count).toBe(33);
        expect(result.contracts).toHaveLength(4);
        for (const contract of result.contracts) {
            expect(contract.id).toBeTruthy();
            expect(['imposed', 'parametric']).toContain(contract.status);
            expect(contract.protocol).toBeTruthy();
            expect(contract.phases?.length).toBeGreaterThan(0);
            expect(contract.evidence).toMatch(/^\[(?:IMPOSED|PARAMETRIC)\]/);
        }
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('bond protocol traverses one-bond, broken, and recaptured states', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadExperiment(page, 'ae-bond-rupture-cycle', 2);
        const result = await page.evaluate(async () => {
            const { advanceAEExperiment } = await import('./js/scales/scale2/experiment-runtime.js');
            const bridge = window._ftdBridge;
            const changes = [];
            let prior = bridge.aeGetDiagnostics().bondCount;
            for (let i = 0; i < 1200; i++) {
                bridge.aeTick();
                advanceAEExperiment(bridge);
                const next = bridge.aeGetDiagnostics().bondCount;
                if (next !== prior) {
                    changes.push({ tick: i + 1, from: prior, to: next });
                    prior = next;
                }
            }
            return {
                changes,
                diag: bridge.aeGetDiagnostics(),
                runtime: bridge.aeGetRuntimeState(),
                finite: Array.from(bridge.aeGetAtomData().positions).every(Number.isFinite),
            };
        });

        expect(result.changes).toEqual([
            expect.objectContaining({ from: 1, to: 0 }),
            expect.objectContaining({ from: 0, to: 1 }),
        ]);
        expect(result.diag.bondCount).toBe(1);
        expect(result.runtime.experiment.phase).toBe('Recapture settling');
        expect(result.runtime.experiment.transitionCount).toBe(3);
        expect(result.runtime.toggles.damping).toBe(true);
        expect(result.finite).toBe(true);
        expect(result.diag.lastError).toBe('ok');
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('argon protocol heats, quenches, then removes its thermostat', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadExperiment(page, 'ae-argon-thermal-cycle', 27);
        const result = await page.evaluate(async () => {
            const { advanceAEExperiment } = await import('./js/scales/scale2/experiment-runtime.js');
            const bridge = window._ftdBridge;
            const tick = (count) => {
                for (let i = 0; i < count; i++) {
                    bridge.aeTick();
                    advanceAEExperiment(bridge);
                }
            };
            tick(499);
            const hot = { diag: bridge.aeGetDiagnostics(), runtime: bridge.aeGetRuntimeState() };
            tick(1);
            const quench = { diag: bridge.aeGetDiagnostics(), runtime: bridge.aeGetRuntimeState() };
            tick(1000);
            const released = { diag: bridge.aeGetDiagnostics(), runtime: bridge.aeGetRuntimeState() };
            return {
                hotTemperature: hot.diag.temperature,
                hotPhase: hot.runtime.experiment.phase,
                quenchTarget: quench.runtime.thermostatTemp,
                quenchPhase: quench.runtime.experiment.phase,
                releasedTemperature: released.diag.temperature,
                releasedPhase: released.runtime.experiment.phase,
                releasedThermostat: released.runtime.toggles.thermostat,
                thermostatSlider: Number(document.getElementById('ae-thermostat-slider')?.value),
                thermostatCheckbox: document.getElementById('ae-thermostat')?.checked,
                transitionCount: released.runtime.experiment.transitionCount,
                finite: [hot.diag.totalEnergy, quench.diag.totalEnergy, released.diag.totalEnergy]
                    .every(Number.isFinite),
            };
        });

        expect(result.hotPhase).toBe('Heat at T*=1.80');
        expect(result.hotTemperature).toBeGreaterThan(1);
        expect(result.quenchPhase).toBe('Quench at T*=0.08');
        expect(result.quenchTarget).toBeCloseTo(0.08, 12);
        expect(result.releasedPhase).toBe('Thermostat released');
        expect(result.releasedTemperature).toBeLessThan(result.hotTemperature);
        expect(result.releasedThermostat).toBe(false);
        expect(result.thermostatSlider).toBeCloseTo(0.08, 12);
        expect(result.thermostatCheckbox).toBe(false);
        expect(result.transitionCount).toBe(3);
        expect(result.finite).toBe(true);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('vacancy blocks the matched harmonic impulse while the complete chain transmits it', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await loadExperiment(page, 'ae-crystal-impulse-vacancy', 17);
        const result = await page.evaluate(async () => {
            const { advanceAEExperiment } = await import('./js/scales/scale2/experiment-runtime.js');
            const bridge = window._ftdBridge;
            const e0 = bridge.aeGetDiagnostics().totalEnergy;
            for (let i = 0; i < 1500; i++) {
                bridge.aeTick();
                advanceAEExperiment(bridge);
            }
            const diag = bridge.aeGetDiagnostics();
            const velocities = Array.from(bridge.aeGetVelocities().velocities);
            return {
                diag,
                completeFarSpeed: Math.abs(velocities[8 * 3]),
                vacancyFarSpeed: Math.abs(velocities[16 * 3]),
                drift: Math.abs(diag.totalEnergy - e0) / Math.max(1, Math.abs(e0)),
                finite: velocities.every(Number.isFinite),
            };
        });

        expect(result.diag.bondCount).toBe(14);
        expect(result.completeFarSpeed).toBeGreaterThan(1e-3);
        expect(result.vacancyFarSpeed).toBeLessThan(1e-10);
        expect(result.completeFarSpeed).toBeGreaterThan(result.vacancyFarSpeed * 1e6);
        expect(result.drift).toBeLessThan(1e-3);
        expect(result.finite).toBe(true);
        expect(result.diag.lastError).toBe('ok');
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('criticality controls change finite neutron outcomes without imposing k-effective', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        const run = async (absorberStrength) => {
            await loadExperiment(page, 'ae-u235-criticality-controls', 28);
            return page.evaluate(async (absorber) => {
                const { advanceAEExperiment } = await import('./js/scales/scale2/experiment-runtime.js');
                const bridge = window._ftdBridge;
                bridge.aeSetNuclearEnvironment({ absorberStrength: absorber });
                for (let i = 0; i < 200; i++) {
                    bridge.aeTick();
                    advanceAEExperiment(bridge);
                }
                return bridge.aeGetNuclearDiagnostics();
            }, absorberStrength);
        };

        const baseline = await run(0);
        const absorbed = await run(1);
        expect(baseline.mode).toBe('sandbox');
        expect(baseline.sourceEnabled).toBe(true);
        expect(baseline.eventCount).toBeGreaterThan(0);
        expect(baseline.fissionNeutronBirths).toBe(3 * baseline.eventCount);
        expect(baseline.kEffective).toBeGreaterThan(0);
        expect(absorbed.absorbedNeutrons).toBeGreaterThan(0);
        expect(absorbed.eventCount).toBeLessThan(baseline.eventCount);
        expect(absorbed.kEffective).toBeLessThanOrEqual(baseline.kEffective);
        for (const diag of [baseline, absorbed]) {
            expect(Math.abs(diag.protonResidual)).toBeLessThan(1e-9);
            expect(Math.abs(diag.neutronResidual)).toBeLessThan(1e-9);
            expect(Math.abs(diag.transportResidualFraction)).toBeLessThan(1e-12);
        }
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});

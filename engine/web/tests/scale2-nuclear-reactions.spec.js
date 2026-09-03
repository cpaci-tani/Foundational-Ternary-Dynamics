// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function loadNuclearScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const select = /** @type {HTMLSelectElement | null} */ (
            document.getElementById('ae-scenario-select'));
        if (!select) throw new Error('ae-scenario-select not found');
        select.value = scenarioId;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
    await expect.poll(
        () => page.evaluate(() => window._ftdBridge?.aeGetNuclearDiagnostics?.()?.channel || ''),
        { timeout: 10_000, message: `${id} did not arm its nuclear channel` },
    ).not.toBe('');
}

test.describe('Scale 2 parametric dynamic nuclear laboratory', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
    });

    for (const fixture of [
        {
            id: 'ae-dt-fusion',
            channel: 'dt_fusion',
            initial: ['1:1', '1:2'],
            products: ['0:1', '2:2'],
            labels: ['n', '⁴He'],
            qMeV: 17.58929688978,
            recoverableMeV: 17.58929688978,
            incidentEnergyMeV: 0.020,
            partition: { charged: 3.52, neutron: 17.58929688978 - 3.52, gamma: 0, delayed: 0 },
            finalCount: 2,
        },
        {
            id: 'ae-u235-fission',
            channel: 'u235_fission',
            initial: ['0:1', '92:143'],
            products: ['0:1', '0:1', '0:1', '36:56', '56:85'],
            labels: ['n', 'n', 'n', '⁹²Kr', '¹⁴¹Ba'],
            qMeV: 173.2801360445,
            recoverableMeV: 200,
            incidentEnergyMeV: 2.53e-8,
            partition: { charged: 168, neutron: 5, gamma: 13, delayed: 14 },
            finalCount: 5,
        },
    ]) {
        test(`${fixture.id} closes composition, momentum, and Q ledgers`, async ({ page }) => {
            const errors = attachConsoleWatcher(page);
            await loadNuclearScenario(page, fixture.id);

            const audit = await page.evaluate(async ({ expectedChannel }) => {
                const bridge = window._ftdBridge;
                const composition = () => {
                    const data = bridge.aeGetAtomData();
                    return Array.from(data.atomicNums, (Z, i) => `${Z}:${data.neutronCounts[i]}`).sort();
                };

                const initial = composition();
                let accepted = true;
                for (let tick = 0; tick < 400; tick++) {
                    if (bridge.aeTick() === false) { accepted = false; break; }
                    if (bridge.aeGetNuclearDiagnostics()?.eventCount === 1) break;
                }

                const diag = bridge.aeGetDiagnostics();
                const nuclear = bridge.aeGetNuclearDiagnostics();
                const products = composition();

                // A channel is single-shot. Extra ticks must not consume its
                // own products or book Q twice.
                for (let tick = 0; tick < 25; tick++) accepted = bridge.aeTick() !== false && accepted;
                const afterExtraTicks = bridge.aeGetNuclearDiagnostics();
                const finalData = bridge.aeGetAtomData();
                const inspections = Array.from(finalData.ids, (id) => bridge.aeInspectAtom(id));

                const { telemetryHub } = await import('./js/telemetry-hub.js');
                const { M_P_PHYS } = await import('./js/constants.js');
                telemetryHub.collectScale2(bridge);
                return {
                    expectedChannel,
                    accepted,
                    initial,
                    products,
                    count: finalData.count,
                    inspectableProducts: inspections.every((item) => item &&
                        Number.isInteger(item.Z) && Number.isInteger(item.N) &&
                        Number.isFinite(item.mass) && Number.isFinite(item.ke)),
                    tick: diag.tick,
                    lastError: diag.lastError,
                    nuclear,
                    kineticBeforeMeV: nuclear?.kineticBeforeSim * M_P_PHYS,
                    eventCountAfterExtraTicks: afterExtraTicks?.eventCount,
                    releasedTrend: telemetryHub.aeNuclearReleased.last(),
                    eventTrend: telemetryHub.aeReactionEvents.last(),
                };
            }, { expectedChannel: fixture.channel });

            expect(audit.accepted).toBe(true);
            expect(audit.initial).toEqual(fixture.initial);
            expect(audit.products).toEqual(fixture.products);
            expect(audit.count).toBe(fixture.finalCount);
            expect(audit.inspectableProducts).toBe(true);
            expect(audit.tick).toBeGreaterThan(0);
            expect(audit.lastError).toBe('ok');
            expect(audit.nuclear?.channel).toBe(fixture.channel);
            expect(audit.nuclear?.phase).toBe('complete');
            expect(audit.nuclear?.eventCount).toBe(1);
            expect(audit.eventCountAfterExtraTicks).toBe(1);
            expect(audit.nuclear?.qMeV).toBeCloseTo(fixture.qMeV, 10);
            expect(audit.nuclear?.incidentEnergyMeV).toBeCloseTo(fixture.incidentEnergyMeV, 12);
            expect(audit.kineticBeforeMeV).toBeCloseTo(fixture.incidentEnergyMeV, 10);
            expect(audit.nuclear?.releasedMeV).toBeCloseTo(fixture.recoverableMeV, 10);
            expect(audit.nuclear?.releasedJoule).toBeCloseTo(fixture.recoverableMeV * 1.602176634e-13, 12);
            expect(audit.nuclear?.chargedMeV).toBeCloseTo(fixture.partition.charged, 10);
            expect(audit.nuclear?.neutronMeV).toBeCloseTo(fixture.partition.neutron, 10);
            expect(audit.nuclear?.promptGammaMeV).toBeCloseTo(fixture.partition.gamma, 10);
            expect(audit.nuclear?.delayedHeatMeV).toBeCloseTo(fixture.partition.delayed, 10);
            expect(audit.nuclear?.depositedMeV + audit.nuclear?.inTransitMeV + audit.nuclear?.escapedMeV)
                .toBeCloseTo(audit.nuclear?.releasedMeV, 10);
            expect(audit.nuclear?.protonResidual).toBe(0);
            expect(audit.nuclear?.chargeResidual).toBe(0);
            expect(audit.nuclear?.neutronResidual).toBe(0);
            expect(Math.abs(audit.nuclear?.momentumResidual || 0)).toBeLessThan(1e-12);
            expect(Math.abs(audit.nuclear?.energyResidualMeV || 0)).toBeLessThan(1e-10);
            expect(Math.abs(audit.nuclear?.totalLedgerResidualMeV || 0)).toBeLessThan(1e-10);
            expect(Math.abs(audit.nuclear?.transportResidualMeV || 0)).toBeLessThan(1e-9);
            // Telemetry ring buffers intentionally store Float32 samples;
            // the event ledger above remains Float64 and is the exact gate.
            expect(audit.releasedTrend).toBeCloseTo(fixture.recoverableMeV, 4);
            expect(audit.eventTrend).toBe(1);
            await expect.poll(() => page.evaluate(() => (
                window.__ftdCtx?.viewport?._molRenderer?._labelPool || []
            ).filter((sprite) => sprite.visible).map((sprite) => sprite._symbol).sort()))
                .toEqual([...fixture.labels].sort());
            await expect.poll(() => page.locator('#ae-legend').textContent())
                .toContain('Nuclear reaction');
            await expect(page.locator('#ae-legend')).toContainText('complete');
            expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
        });
    }

    test('scenario registry publishes all four reaction presets as parametric', async ({ page }) => {
        const registry = await page.evaluate(async () => {
            const module = await import('./js/scales/scale2/scenario-registry.js');
            const nuclear = await import('./js/scales/scale2/nuclear-reactions.js');
            return {
                count: module.AE_CURATED_SCENARIOS.length,
                scenarios: ['ae-dt-fusion', 'ae-u235-fission', 'ae-dt-fusion-burn', 'ae-u235-chain-reaction'].map((id) => {
                    const item = module.getAEScenarioMeta(id);
                    return {
                        id: item?.id,
                        reaction: item?.reaction,
                        mode: item?.nuclear?.mode,
                        status: item?.epistemicStatus,
                        owner: item?.owner,
                        evidence: item?.evidence,
                    };
                }),
                validation: module.validateAEScenarioRegistry(),
                hazards: {
                    disabled: nuclear.nuclearReactionProbability('dt_fusion', 0.064, 1, 0),
                    dtPeak: nuclear.nuclearReactionProbability('dt_fusion', 0.064, 1, 1),
                    dtCold: nuclear.nuclearReactionProbability('dt_fusion', 1e-6, 1, 1),
                    fissionThermal: nuclear.nuclearReactionProbability('u235_fission', 2.53e-8, 1, 1),
                    fissionFast: nuclear.nuclearReactionProbability('u235_fission', 1, 1, 1),
                },
            };
        });

        expect(registry.count).toBe(29);
        expect(registry.validation.ok, registry.validation.errors.join('\n')).toBe(true);
        expect(registry.scenarios).toEqual([
            expect.objectContaining({ id: 'ae-dt-fusion', reaction: 'dt_fusion', status: 'parametric', owner: 'js_effective_atom_engine' }),
            expect.objectContaining({ id: 'ae-u235-fission', reaction: 'u235_fission', status: 'parametric', owner: 'js_effective_atom_engine' }),
            expect.objectContaining({ id: 'ae-dt-fusion-burn', reaction: 'dt_fusion', mode: 'batch', status: 'parametric', owner: 'js_effective_atom_engine' }),
            expect.objectContaining({ id: 'ae-u235-chain-reaction', reaction: 'u235_fission', mode: 'chain', status: 'parametric', owner: 'js_effective_atom_engine' }),
        ]);
        for (const scenario of registry.scenarios) {
            expect(scenario.evidence).toContain('[PARAMETRIC]');
        }
        expect(registry.hazards.disabled).toBe(0);
        expect(registry.hazards.dtPeak).toBeGreaterThan(registry.hazards.dtCold);
        expect(registry.hazards.fissionThermal).toBeGreaterThan(registry.hazards.fissionFast);
    });

    test('finite D-T burn scales microscopic events into an explicit ensemble ledger', async ({ page }) => {
        await loadNuclearScenario(page, 'ae-dt-fusion-burn');
        const audit = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            for (let i = 0; i < 80; i++) bridge.aeTick();
            return {
                nuclear: bridge.aeGetNuclearDiagnostics(),
                atomCount: bridge.aeGetAtomData().count,
                visuals: bridge.aeGetNuclearVisuals(),
            };
        });
        expect(audit.nuclear.eventCount).toBe(12);
        expect(audit.nuclear.representedEventCount).toBe(1.2e19);
        expect(audit.nuclear.microscopicReleasedMeV).toBeCloseTo(12 * 17.58929688978, 8);
        expect(audit.nuclear.releasedJoule).toBeGreaterThan(3.3e7);
        expect(audit.nuclear.releasedJoule).toBeLessThan(3.5e7);
        expect(Math.abs(audit.nuclear.transportResidualFraction)).toBeLessThan(1e-12);
        expect(audit.atomCount).toBe(24);
        expect(audit.visuals.effects.length).toBe(12);
        expect(audit.visuals.effects.every((event) =>
            event.depositedFraction > 0 && event.depositedFraction <= 1)).toBe(true);
        expect(audit.visuals.effects.every((event) =>
            event.neutronDirections.length === 1)).toBe(true);
        await expect(page.locator('#toggle-ae-nuclear-events')).toHaveClass(/active/);
        await expect(page.locator('#toggle-ae-radiation')).toHaveClass(/active/);
        await expect(page.locator('#toggle-ae-heat')).toHaveClass(/active/);
        await expect(page.locator('#toggle-ae-nuclear-boundary')).toHaveClass(/active/);
        await expect.poll(() => page.evaluate(() => ({
            heat: window.__ftdCtx?.viewport?._molRenderer?._nuclearHeat?.count || 0,
            flashes: window.__ftdCtx?.viewport?._molRenderer?._nuclearFlashes?.count || 0,
            rays: window.__ftdCtx?.viewport?._molRenderer?._nuclearRadiation?.geometry?.drawRange?.count || 0,
            fronts: window.__ftdCtx?.viewport?._molRenderer?._nuclearWavefronts?.count || 0,
            packets: window.__ftdCtx?.viewport?._molRenderer?._nuclearPackets?.count || 0,
        }))).toEqual(expect.objectContaining({ heat: 12 }));
        const rendered = await page.evaluate(() => ({
            heat: window.__ftdCtx.viewport._molRenderer._nuclearHeat.count,
            flashes: window.__ftdCtx.viewport._molRenderer._nuclearFlashes.count,
            rays: window.__ftdCtx.viewport._molRenderer._nuclearRadiation.geometry.drawRange.count,
            fronts: window.__ftdCtx.viewport._molRenderer._nuclearWavefronts.count,
            packets: window.__ftdCtx.viewport._molRenderer._nuclearPackets.count,
        }));
        expect(rendered.rays).toBeGreaterThan(0);
        expect(rendered.flashes).toBeGreaterThan(0);
        expect(rendered.fronts).toBeGreaterThan(0);
        expect(rendered.packets).toBeGreaterThan(0);
        for (const id of [
            'toggle-ae-nuclear-events', 'toggle-ae-radiation',
            'toggle-ae-heat', 'toggle-ae-nuclear-boundary',
        ]) {
            await page.locator(`#${id}`).click();
        }
        const hidden = await page.evaluate(() => {
            const renderer = window.__ftdCtx.viewport._molRenderer;
            return {
                flashes: renderer._nuclearFlashes.visible,
                rings: renderer._nuclearShockRings.visible,
                rays: renderer._nuclearRadiation.visible,
                fronts: renderer._nuclearWavefronts.visible,
                packets: renderer._nuclearPackets.visible,
                heat: renderer._nuclearHeat.visible,
                boundary: renderer._nuclearTransportBoundary.visible,
                presentationRecords: renderer._nuclearPresentationEvents.size,
                effectRecords: window._ftdBridge.aeGetNuclearVisuals().effects.length,
            };
        });
        expect(hidden).toMatchObject({
            flashes: false, rings: false, rays: false, fronts: false,
            packets: false, heat: false, boundary: false,
        });
        expect(hidden.presentationRecords).toBeLessThanOrEqual(hidden.effectRecords);
        await expect.poll(() => page.locator('#status-energy').textContent()).toContain('MJ');
    });

    test('finite U-235 chain remains bounded and conserves the transport ledger', async ({ page }) => {
        await loadNuclearScenario(page, 'ae-u235-chain-reaction');
        const audit = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            let accepted = true;
            for (let i = 0; i < 800; i++) accepted = bridge.aeTick() !== false && accepted;
            return {
                accepted,
                nuclear: bridge.aeGetNuclearDiagnostics(),
                finite: Array.from(bridge.aeGetAtomData().positions).every(Number.isFinite),
                visuals: bridge.aeGetNuclearVisuals(),
            };
        });
        expect(audit.accepted).toBe(true);
        expect(audit.finite).toBe(true);
        expect(audit.nuclear.eventCount).toBeGreaterThan(1);
        expect(audit.nuclear.eventCount).toBeLessThanOrEqual(27);
        expect(audit.nuclear.generation).toBeGreaterThan(0);
        expect(audit.nuclear.fuelRemaining).toBe(27 - audit.nuclear.eventCount);
        expect(audit.nuclear.releasedJoule).toBeCloseTo(audit.nuclear.eventCount * 200 * 1e18 * 1.602176634e-13, 4);
        expect(Math.abs(audit.nuclear.transportResidualFraction)).toBeLessThan(1e-12);
        expect(audit.nuclear.liveNeutrons).toBeGreaterThan(0);
        expect(audit.nuclear.kEffective).toBeGreaterThan(0);
        expect(audit.visuals.flights).toEqual([]);
        expect(audit.visuals.effects.length).toBeLessThanOrEqual(27);
        expect(audit.visuals.effects.map((event) => event.ordinal))
            .toEqual(Array.from({ length: audit.nuclear.eventCount }, (_, index) => index + 1));
    });

    test('live reactivity control gates collisions without reloading the scenario', async ({ page }) => {
        await loadNuclearScenario(page, 'ae-u235-chain-reaction');
        const audit = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            bridge.aeSetNuclearEnvironment({ reactivityScale: 0 });
            for (let i = 0; i < 100; i++) bridge.aeTick();
            const disabled = bridge.aeGetNuclearDiagnostics();
            bridge.aeSetNuclearEnvironment({ reactivityScale: 20 });
            for (let i = 0; i < 400 && bridge.aeGetNuclearDiagnostics().eventCount === 0; i++) bridge.aeTick();
            return { disabled, enabled: bridge.aeGetNuclearDiagnostics() };
        });
        expect(audit.disabled.eventCount).toBe(0);
        expect(audit.disabled.fuelRemaining).toBe(27);
        expect(audit.enabled.eventCount).toBeGreaterThan(0);
        expect(audit.enabled.reactivityScale).toBe(20);
    });

    test('seed replay is exact and absorber intervention suppresses the same chain', async ({ page }) => {
        const run = async (absorberStrength) => {
            await loadNuclearScenario(page, 'ae-u235-chain-reaction');
            return page.evaluate((absorber) => {
                const bridge = window._ftdBridge;
                bridge.aeSetNuclearEnvironment({ absorberStrength: absorber });
                for (let i = 0; i < 500; i++) bridge.aeTick();
                const diag = bridge.aeGetNuclearDiagnostics();
                return {
                    eventCount: diag.eventCount,
                    generation: diag.generation,
                    fuelRemaining: diag.fuelRemaining,
                    liveNeutrons: diag.liveNeutrons,
                    leakedNeutrons: diag.leakedNeutrons,
                    absorbedNeutrons: diag.absorbedNeutrons,
                    scatteredNeutrons: diag.scatteredNeutrons,
                    kEffective: diag.kEffective,
                    positions: Array.from(bridge.aeGetAtomData().positions, value => Number(value.toFixed(6))),
                };
            }, absorberStrength);
        };
        const baseline = await run(0);
        const replay = await run(0);
        const absorbed = await run(1);
        expect(replay).toEqual(baseline);
        expect(baseline.eventCount).toBeGreaterThan(1);
        expect(absorbed.eventCount).toBeLessThan(baseline.eventCount);
        expect(absorbed.absorbedNeutrons).toBeGreaterThan(0);
    });

    test('nuclear laboratory injectors and source mutate the live custom sandbox', async ({ page }) => {
        await expect(page.locator('#panel-controls-grid .card.scale-ae')).toHaveCount(4);
        await expect(page.locator('#ae-nuclear-channel')).toBeVisible();
        await page.evaluate(() => {
            const select = document.getElementById('ae-scenario-select');
            select.value = 'ae-custom';
            select.dispatchEvent(new Event('change', { bubbles: true }));
            const channel = document.getElementById('ae-nuclear-channel');
            channel.value = 'u235_fission';
            channel.dispatchEvent(new Event('change', { bubbles: true }));
            document.getElementById('btn-ae-inject-u235').click();
            document.getElementById('btn-ae-inject-neutron').click();
            const rate = document.getElementById('ae-nuclear-source-rate');
            rate.value = '1';
            rate.dispatchEvent(new Event('input', { bubbles: true }));
            const enabled = document.getElementById('ae-nuclear-source-enabled');
            enabled.checked = true;
            enabled.dispatchEvent(new Event('change', { bubbles: true }));
            for (let i = 0; i < 3; i++) window._ftdBridge.aeTick();
        });
        const audit = await page.evaluate(() => ({
            diag: window._ftdBridge.aeGetNuclearDiagnostics(),
            atoms: window._ftdBridge.aeGetAtomData().count,
        }));
        expect(audit.diag.mode).toBe('sandbox');
        expect(audit.diag.sourceEnabled).toBe(true);
        expect(audit.diag.sourceRate).toBe(1);
        expect(audit.diag.sourceNeutrons).toBeGreaterThanOrEqual(4);
        expect(audit.atoms).toBeGreaterThanOrEqual(5);
    });
});

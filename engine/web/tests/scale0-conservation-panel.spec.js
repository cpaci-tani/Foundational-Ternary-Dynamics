// @ts-check
/**
 * Conservation panel ownership regression.
 *
 * flux-* scenarios run on the WasmBridgeProxy worker, not the main-thread WASM
 * bridge. The always-on conservation panel must sample the active owner or it
 * sits at t=0 and its energy deltas look frozen while the visible flux pulse
 * evolves.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test.describe('Conservation panel and WASM diagnostics', () => {
    /** @type {import('@playwright/test').BrowserContext|undefined} */
    let context;
    /** @type {import('@playwright/test').Page} */
    let page;

    test.beforeAll(async ({ browser, baseURL }) => {
        context = await browser.newContext({ baseURL });
        page = await context.newPage();
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
    });

    test.afterAll(async () => {
        await context?.close();
    });

    test('conservation panel follows worker-owned flux-pulse ticks', async () => {
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            if (sel) {
                sel.value = 'flux-pulse';
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        await expect.poll(
            () => page.locator('#conservation-micropanel-status').textContent(),
            { timeout: 10_000, message: 'conservation panel never advanced on flux-pulse' },
        ).toMatch(/^state t=([1-9]\d*) · E (?:t=\d+|waiting) · p (?:t=\d+|waiting)$/);

        // Default Controls tab does not request the energy-audit stream, so
        // momentum (Poynting) is honestly blank rather than a fake Δp = 0.
        await expect.poll(
            () => page.locator('[data-cons-val="p"]').textContent(),
            { timeout: 5_000, message: 'conservation Δp never rendered' },
        ).toMatch(/—/);
    });

    test('staggered audit provenance is deduplicated independently and unavailable audit stays unavailable', async () => {
        const result = await page.evaluate(async () => {
            const [panelModule, physicsModule] = await Promise.all([
                import('/js/scales/scale0/ui/overlays/conservation-micropanel.js'),
                import('/js/physics/index.js'),
            ]);
            window.__ftdConservationPanel?.dispose();

            const host = document.createElement('div');
            document.body.appendChild(host);
            const bridge = {};
            let diagMeta = {
                source: 'fixture', sourceEpoch: 2, sampleTick: 100, tick: 100,
                stateVersion: 100, snapshotVersion: 100, stale: false,
            };
            let auditMeta = {
                source: 'fixture', sourceEpoch: 2, sampleTick: 96, tick: 96,
                stateVersion: 12, snapshotVersion: 12, stale: false,
            };
            const hub = {
                s0: {
                    meta: { expectedSource: 'fixture', expectedSourceEpoch: 2 },
                    diag: {
                        tick: 100, dynamicEnergy: 900, totalEnergy: 9_000,
                        angMomX: 1, angMomY: 0, angMomZ: 0, chargeBalance: 1,
                    },
                    audit: {
                        dynamicEnergy: 10,
                        totalPoynting: { x: 1, y: 0, z: 0 },
                    },
                },
                getResetVersion: () => 0,
                getScale0TelemetryMeta: (group) => (
                    group === 'diagnostics' ? diagMeta : (group === 'audit' ? auditMeta : null)
                ),
            };
            const harness = physicsModule.getPhysicsHarness(bridge);
            harness.telemetry = hub;
            const api = panelModule.mountConservationMicropanel(host, () => bridge, hub);
            const readRows = () => ({
                energy: api.element.querySelector('[data-cons-val="E"]')?.textContent.trim(),
                momentum: api.element.querySelector('[data-cons-val="p"]')?.textContent.trim(),
                status: api.element.querySelector('#conservation-micropanel-status')?.textContent,
            });

            const firstTotals = harness.getConservationTotals();
            api.update();
            const firstLengths = {
                diagnostics: api.diagnosticHistoryLength,
                energy: api.energyHistoryLength,
                momentum: api.momentumHistoryLength,
            };

            // A genuinely new audit observation 100 audit ticks later produces
            // one new E/p sample and a finite headline delta.
            diagMeta = { ...diagMeta, sampleTick: 201, tick: 201, stateVersion: 201 };
            hub.s0.diag = {
                ...hub.s0.diag, tick: 201, dynamicEnergy: 901, totalEnergy: 9_001,
                angMomX: 2, chargeBalance: 2,
            };
            auditMeta = { ...auditMeta, sampleTick: 196, tick: 196, stateVersion: 13 };
            hub.s0.audit = {
                dynamicEnergy: 12,
                totalPoynting: { x: 3, y: 0, z: 0 },
            };
            api.update();
            const afterNewAudit = {
                diagnostics: api.diagnosticHistoryLength,
                energy: api.energyHistoryLength,
                momentum: api.momentumHistoryLength,
                rows: readRows(),
            };

            // Diagnostics advances, but the audit stateVersion/sampleTick is
            // deliberately reused. E/p histories must not grow.
            diagMeta = { ...diagMeta, sampleTick: 202, tick: 202, stateVersion: 202 };
            hub.s0.diag = { ...hub.s0.diag, tick: 202, totalEnergy: 99_999 };
            api.update();
            const afterReusedAudit = {
                diagnostics: api.diagnosticHistoryLength,
                energy: api.energyHistoryLength,
                momentum: api.momentumHistoryLength,
                rows: readRows(),
            };

            // Retain tempting diagnostics energy values while making the audit
            // unavailable. Neither dynamic nor raw totalEnergy may fill ΔE.
            diagMeta = { ...diagMeta, sampleTick: 203, tick: 203, stateVersion: 203 };
            hub.s0.diag = {
                ...hub.s0.diag, tick: 203, dynamicEnergy: 777, totalEnergy: 88_888,
            };
            auditMeta = { ...auditMeta, stale: true };
            const unavailableTotals = harness.getConservationTotals();
            api.update();
            const unavailable = {
                diagnostics: api.diagnosticHistoryLength,
                energy: api.energyHistoryLength,
                momentum: api.momentumHistoryLength,
                rows: readRows(),
                energyAvailable: unavailableTotals.EAvailable,
                momentumAvailable: unavailableTotals.pAvailable,
                energyIsNaN: Number.isNaN(unavailableTotals.E),
            };

            // Null provenance is unavailable, not numeric tick/version zero.
            // Keep finite retained values present to prove metadata—not value
            // shape—is what closes the scientific observation contract.
            auditMeta = {
                ...auditMeta,
                sampleTick: null,
                tick: null,
                stateVersion: null,
                snapshotVersion: null,
                stale: false,
            };
            hub.s0.audit = {
                dynamicEnergy: 44,
                totalPoynting: { x: 4, y: 0, z: 0 },
            };
            const nullProvenanceTotals = harness.getConservationTotals();
            api.update();
            const nullProvenance = {
                energy: api.energyHistoryLength,
                momentum: api.momentumHistoryLength,
                energyAvailable: nullProvenanceTotals.EAvailable,
                momentumAvailable: nullProvenanceTotals.pAvailable,
                energyIsNaN: Number.isNaN(nullProvenanceTotals.E),
                rows: readRows(),
            };

            const provenance = {
                energy: {
                    sampleTick: firstTotals.energyObservation.sampleTick,
                    stateVersion: firstTotals.energyObservation.stateVersion,
                    source: firstTotals.energyObservation.source,
                    sourceEpoch: firstTotals.energyObservation.sourceEpoch,
                },
                momentum: {
                    sampleTick: firstTotals.momentumObservation.sampleTick,
                    stateVersion: firstTotals.momentumObservation.stateVersion,
                    source: firstTotals.momentumObservation.source,
                    sourceEpoch: firstTotals.momentumObservation.sourceEpoch,
                },
            };

            api.dispose();
            host.remove();
            panelModule.initConservationMicropanel();
            return {
                provenance, firstLengths, afterNewAudit, afterReusedAudit,
                unavailable, nullProvenance,
            };
        });

        expect(result.provenance.energy).toEqual({
            sampleTick: 96, stateVersion: 12, source: 'fixture', sourceEpoch: 2,
        });
        expect(result.provenance.momentum).toEqual(result.provenance.energy);
        expect(result.firstLengths).toEqual({ diagnostics: 1, energy: 1, momentum: 1 });
        expect(result.afterNewAudit.diagnostics).toBe(2);
        expect(result.afterNewAudit.energy).toBe(2);
        expect(result.afterNewAudit.momentum).toBe(2);
        expect(result.afterNewAudit.rows.energy).not.toContain('—');
        expect(result.afterNewAudit.rows.momentum).not.toContain('—');
        expect(result.afterReusedAudit.diagnostics).toBe(3);
        expect(result.afterReusedAudit.energy).toBe(2);
        expect(result.afterReusedAudit.momentum).toBe(2);
        expect(result.afterReusedAudit.rows.status).toBe('state t=202 · E t=196 · p t=196');
        expect(result.unavailable.diagnostics).toBe(4);
        expect(result.unavailable.energy).toBe(2);
        expect(result.unavailable.momentum).toBe(2);
        expect(result.unavailable.energyAvailable).toBe(false);
        expect(result.unavailable.momentumAvailable).toBe(false);
        expect(result.unavailable.energyIsNaN).toBe(true);
        expect(result.unavailable.rows.energy).toContain('—');
        expect(result.unavailable.rows.momentum).toContain('—');
        expect(result.unavailable.rows.status).toBe('state t=203 · E waiting · p waiting');
        expect(result.nullProvenance.energy).toBe(2);
        expect(result.nullProvenance.momentum).toBe(2);
        expect(result.nullProvenance.energyAvailable).toBe(false);
        expect(result.nullProvenance.momentumAvailable).toBe(false);
        expect(result.nullProvenance.energyIsNaN).toBe(true);
        expect(result.nullProvenance.rows.energy).toContain('—');
        expect(result.nullProvenance.rows.momentum).toContain('—');
        expect(result.nullProvenance.rows.status).toBe('state t=203 · E waiting · p waiting');
    });

    test('WASM vacuum diagnostics expose moving physical energy', async () => {
        await selectScale0Scenario(page, 's0-vacuum-electron', { settleMs: 0 });
        await expect.poll(async () => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const state = getScale0State();
            const owner = state.useFluxMock && state.fluxMock
                ? state.fluxMock : window.__ftdCtx?.bridge;
            return state.currentScenarioId === 's0-vacuum-electron'
                && owner?.ready === true;
        }), {
            timeout: 60_000,
            message: 's0-vacuum-electron owner did not become authoritative/ready',
        }).toBe(true);

        const snap = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const st = getScale0State();
            const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
            const caps = bridge.capabilities.scale0;

            const d0 = caps.getScale0Diagnostics();
            const a0 = caps.getScale0EnergyAudit();
            for (let i = 0; i < 20; i++) caps.tickScale0();
            const d20 = caps.getScale0Diagnostics();
            const a20 = caps.getScale0EnergyAudit();

            return {
                owner: !st.useFluxMock && bridge?.isWasm === true ? 'wasm' : 'other',
                backendName: bridge?.constructor?.name ?? null,
                e0: d0?.totalEnergy,
                e20: d20?.totalEnergy,
                audit0: a0?.totalEnergy,
                audit20: a20?.totalEnergy,
                cellVolume: a20?.cellVolume,
                fieldEnergy: a20?.fieldEnergy,
                fieldEnergyDensitySum: a20?.fieldEnergyDensitySum,
                waveEnergy: a20?.waveEnergy,
                waveEnergyDensitySum: a20?.waveEnergyDensitySum,
                baseline0: d0?.vacuumBaselineEnergy ?? null,
                baseline20: d20?.vacuumBaselineEnergy ?? null,
                hasAudit: !!a0 && !!a20,
            };
        });

        test.skip(snap.owner !== 'wasm',
            `s0-vacuum-electron owner is ${snap.backendName}, not main-thread WASM`);
        expect(snap.hasAudit, 'energy audit object present').toBe(true);
        expect(Math.abs(snap.e0 - snap.audit0)).toBeLessThan(1e-9);
        expect(Math.abs(snap.e20 - snap.audit20)).toBeLessThan(1e-9);
        expect(snap.cellVolume).toBe(1);
        expect(Number.isFinite(snap.fieldEnergyDensitySum)).toBe(true);
        expect(Number.isFinite(snap.waveEnergyDensitySum)).toBe(true);
        expect(Math.abs(snap.fieldEnergy - snap.fieldEnergyDensitySum)).toBeLessThan(1e-12);
        expect(Math.abs(snap.waveEnergy - snap.waveEnergyDensitySum)).toBeLessThan(1e-12);
        expect(snap.e0).not.toBe(snap.e20);
        expect(snap.baseline0).toBeGreaterThan(1000);
        expect(snap.baseline20).toBe(snap.baseline0);
    });
});

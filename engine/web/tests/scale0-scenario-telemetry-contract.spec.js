// @ts-check
/**
 * Broad Scale-0 telemetry contract.
 *
 * Loads every registry scenario, ticks the active owner directly, and verifies
 * diagnostics, energy audit, and Lagrangian telemetry stay finite and live.
 * This catches stale-owner regressions where a panel reads the idle WASM bridge
 * while the scenario is actually owned by the JS MockBridge, and stale-baseline
 * regressions where WASM diagnostics report the Born-Infeld vacuum baseline
 * instead of the moving physical energy channel.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

const EPS = 1e-6;

function isFiniteNumber(v) {
    return Number.isFinite(Number(v));
}

test.describe('Scale-0 scenario telemetry contract', () => {
    test.beforeEach(async ({ page }) => {
        await page.addInitScript(() => {
            window.__ftdTelemetryOnDemand = false;
            window.__ftdPhysicsWorker = false;
        });
    });

    test('every scenario exposes finite active-owner diagnostics, audit, and lagrangian telemetry', async ({ page }) => {
        test.setTimeout(240_000);
        await gotoAndReady(page);

        const rows = await page.evaluate(async () => {
            const { SCALE0_SCENARIOS } = await import('/js/scales/scale0/scenario-registry.js');
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const { telemetryHub } = await import('/js/telemetry-hub.js');

            const finite = (v) => Number.isFinite(Number(v));
            const readActive = () => {
                const st = getScale0State();
                const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
                const caps = bridge?.capabilities?.scale0 || {};
                const diag = caps.getScale0Diagnostics?.() || null;
                const audit = caps.getScale0EnergyAudit?.() || null;
                const lag = caps.getScale0Lagrangian?.() || null;
                return { st, bridge, caps, diag, audit, lag };
            };

            const rows = [];
            for (const scenario of SCALE0_SCENARIOS) {
                const sel = document.getElementById('scenario-select');
                if (![...sel.options].some((o) => o.value === scenario.id)) {
                    sel.add(new Option(scenario.id, scenario.id));
                }
                sel.value = scenario.id;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
                await new Promise((resolve) => setTimeout(resolve, 60));

                let active = readActive();
                const before = {
                    tick: active.diag?.tick ?? active.bridge?.currentTick?.() ?? null,
                    diagE: active.diag?.totalEnergy ?? null,
                    auditE: active.audit?.totalEnergy ?? null,
                    lagH: active.lag?.hamiltonian ?? active.lag?.total ?? null,
                };

                for (let i = 0; i < 12; i++) active.caps.tickScale0?.();
                active = readActive();
                telemetryHub.collectScale0(window.__ftdCtx.bridge, active.st.fluxMock, active.st.useFluxMock);
                telemetryHub.collectScale0Audit(window.__ftdCtx.bridge, active.st.fluxMock, active.st.useFluxMock);
                telemetryHub.collectScale0Lagrangian(window.__ftdCtx.bridge, active.st.fluxMock, active.st.useFluxMock);

                const after = {
                    tick: active.diag?.tick ?? active.bridge?.currentTick?.() ?? null,
                    diagE: active.diag?.totalEnergy ?? null,
                    baselineE: active.diag?.vacuumBaselineEnergy ?? null,
                    auditE: active.audit?.totalEnergy ?? null,
                    fieldE: active.audit?.fieldEnergy ?? null,
                    waveE: active.audit?.waveEnergy ?? null,
                    particleKE: active.audit?.particleKE ?? null,
                    lagH: active.lag?.hamiltonian ?? active.lag?.total ?? null,
                    totalFlux: active.diag?.totalFlux ?? null,
                    manifested: active.diag?.manifested ?? null,
                };
                const live = Number(after.totalFlux) > 0.01 ||
                    Number(after.manifested) > 0 ||
                    Number(after.fieldE) > 0 ||
                    Number(after.waveE) > 0 ||
                    Number(after.particleKE) > 0;

                rows.push({
                    id: scenario.id,
                    owner: active.st.useFluxMock ? 'mock' : 'wasm',
                    live,
                    before,
                    after,
                    hubDiagE: telemetryHub.s0.diag?.totalEnergy ?? null,
                    hubAuditE: telemetryHub.s0.audit?.totalEnergy ?? null,
                    hubLagH: telemetryHub.s0.lagrangian?.hamiltonian ?? telemetryHub.s0.lagrangian?.total ?? null,
                    hasObjects: { diag: !!active.diag, audit: !!active.audit, lag: !!active.lag },
                    finite: {
                        tick: finite(after.tick),
                        diagE: finite(after.diagE),
                        auditE: finite(after.auditE),
                        lagH: finite(after.lagH),
                        hubDiagE: finite(telemetryHub.s0.diag?.totalEnergy),
                        hubAuditE: finite(telemetryHub.s0.audit?.totalEnergy),
                        hubLagH: finite(telemetryHub.s0.lagrangian?.hamiltonian ?? telemetryHub.s0.lagrangian?.total),
                    },
                });
            }
            return rows;
        });

        const missing = rows.filter((r) => !r.hasObjects.diag || !r.hasObjects.audit || !r.hasObjects.lag);
        const nonFinite = rows.filter((r) => Object.values(r.finite).some((ok) => !ok));
        const deadLiveTicks = rows.filter((r) => r.live && !(Number(r.after.tick) > Number(r.before.tick)));
        const frozenLiveEnergy = rows.filter((r) => r.live &&
            Math.abs(Number(r.after.diagE) - Number(r.before.diagE)) <= 1e-9 &&
            Math.abs(Number(r.after.auditE) - Number(r.before.auditE)) <= 1e-9 &&
            Math.abs(Number(r.after.lagH) - Number(r.before.lagH)) <= 1e-9);
        const staleWasmEnergy = rows.filter((r) => r.owner === 'wasm' &&
            Math.abs(Number(r.after.diagE) - Number(r.after.auditE)) > EPS);
        const staleHubOwner = rows.filter((r) =>
            Math.abs(Number(r.hubDiagE) - Number(r.after.diagE)) > EPS ||
            Math.abs(Number(r.hubAuditE) - Number(r.after.auditE)) > EPS ||
            Math.abs(Number(r.hubLagH) - Number(r.after.lagH)) > EPS);

        expect(missing, 'scenarios missing telemetry objects').toEqual([]);
        expect(nonFinite, 'scenarios with non-finite telemetry scalars').toEqual([]);
        expect(deadLiveTicks, 'live scenarios whose active owner did not tick').toEqual([]);
        expect(frozenLiveEnergy, 'live scenarios whose energy/lag telemetry stayed frozen').toEqual([]);
        expect(staleWasmEnergy, 'WASM diagnostics.totalEnergy must mirror audit.totalEnergy').toEqual([]);
        expect(staleHubOwner, 'telemetryHub must collect from the active owner').toEqual([]);
    });

    test('all vacuum scenarios report moving physical energy, not the fixed vacuum baseline', async ({ page }) => {
        test.setTimeout(120_000);
        await gotoAndReady(page);

        const rows = await page.evaluate(async () => {
            const { SCALE0_SCENARIOS } = await import('/js/scales/scale0/scenario-registry.js');
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const ids = SCALE0_SCENARIOS.map((s) => s.id).filter((id) => id.startsWith('s0-vacuum-'));
            const rows = [];
            for (const id of ids) {
                const sel = document.getElementById('scenario-select');
                sel.value = id;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
                await new Promise((resolve) => setTimeout(resolve, 60));

                const st = getScale0State();
                const bridge = (st.useFluxMock && st.fluxMock) ? st.fluxMock : window.__ftdCtx.bridge;
                const caps = bridge.capabilities.scale0;
                const d0 = caps.getScale0Diagnostics();
                const a0 = caps.getScale0EnergyAudit();
                for (let i = 0; i < 12; i++) caps.tickScale0();
                const d1 = caps.getScale0Diagnostics();
                const a1 = caps.getScale0EnergyAudit();
                rows.push({
                    id,
                    owner: st.useFluxMock ? 'mock' : 'wasm',
                    before: { diagE: d0.totalEnergy, auditE: a0.totalEnergy, tick: d0.tick },
                    after: {
                        diagE: d1.totalEnergy,
                        auditE: a1.totalEnergy,
                        baselineE: d1.vacuumBaselineEnergy ?? null,
                        tick: d1.tick,
                    },
                });
            }
            return rows;
        });

        expect(rows.length, 'vacuum scenario coverage').toBeGreaterThan(0);
        expect(rows.filter((r) => r.owner !== 'wasm'), 'vacuum scenarios should be WASM-owned').toEqual([]);
        expect(rows.filter((r) => !isFiniteNumber(r.after.diagE) || !isFiniteNumber(r.after.auditE)),
            'vacuum scenarios must expose finite physical energy').toEqual([]);
        expect(rows.filter((r) => Math.abs(Number(r.after.diagE) - Number(r.after.auditE)) > EPS),
            'vacuum diagnostics energy should mirror audit energy').toEqual([]);
        expect(rows.filter((r) => !(Number(r.after.tick) > Number(r.before.tick))),
            'vacuum scenarios should advance ticks under manual active-owner ticks').toEqual([]);
        expect(rows.filter((r) => Math.abs(Number(r.after.diagE) - Number(r.before.diagE)) <= 1e-9 &&
            Math.abs(Number(r.after.auditE) - Number(r.before.auditE)) <= 1e-9),
            'vacuum physical energy should move over ticks').toEqual([]);
        expect(rows.filter((r) => isFiniteNumber(r.after.baselineE) &&
            Math.abs(Number(r.after.baselineE) - Number(r.after.auditE)) > EPS &&
            Math.abs(Number(r.after.diagE) - Number(r.after.baselineE)) <= EPS),
            'diagnostics.totalEnergy should not be the stale vacuum baseline').toEqual([]);
    });
});

// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.beforeEach(async ({ page }) => {
    test.setTimeout(120_000);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
});

test('status and Thermo never reinterpret diagnostics.totalEnergy as dynamic energy', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const { updateDiagnosticsAndPanels } = await import(
            '/js/scales/scale0/runtime/diagnostics.js'
        );
        const { mountThermoPanel } = await import(
            '/js/scales/scale0/ui/overlays/thermo-panel.js'
        );

        const original = {
            collectScale0: telemetryHub.collectScale0,
            collectScale0Audit: telemetryHub.collectScale0Audit,
            collectScale0Lagrangian: telemetryHub.collectScale0Lagrangian,
            getScale0TelemetryMeta: telemetryHub.getScale0TelemetryMeta,
            diag: telemetryHub.s0.diag,
            audit: telemetryHub.s0.audit,
            prevWantAudit: telemetryHub._prevWantAudit,
            prevWantLag: telemetryHub._prevWantLag,
            lastAuditVersion: telemetryHub._lastAuditVersion,
        };

        const currentMeta = (tick) => ({
            source: 'fixture', sourceEpoch: 1, stateVersion: tick,
            snapshotVersion: tick, tick, sampleTick: tick,
            stale: false, status: 'available', receivedAt: performance.now(),
        });
        const failedMeta = (tick) => ({
            ...currentMeta(tick), stale: true, status: 'nonfinite',
        });

        async function runFixture({ diag, audit, diagMeta, auditMeta }) {
            telemetryHub.s0.diag = diag;
            telemetryHub.s0.audit = audit;
            telemetryHub.collectScale0 = () => diag;
            telemetryHub.collectScale0Audit = () => audit;
            telemetryHub.collectScale0Lagrangian = () => null;
            telemetryHub.getScale0TelemetryMeta = (group) => ({
                diagnostics: diagMeta,
                audit: auditMeta,
            }[group] ?? null);

            const makeEl = (tag = 'span') => document.createElement(tag);
            const statusEnergy = makeEl();
            updateDiagnosticsAndPanels({
                frameCount: 3,
                bridge: { latticeSize: 3 },
                activeTab: 'none',
                isPanelVisible: () => false,
                running: false,
                dom: {
                    statusPtime: makeEl(),
                    statusParticles: makeEl(),
                    statusEnergy,
                    statusDot: makeEl('div'),
                    statusState: makeEl(),
                },
            }, {
                useFluxMock: false,
                fluxMock: null,
                fieldDataVersion: diagMeta?.stateVersion ?? 0,
            });

            const host = document.createElement('div');
            host.classList.add('active');
            document.body.appendChild(host);
            const bridge = {
                latticeSize: 3,
                getLangevinTemp: () => 0,
            };
            const api = mountThermoPanel(host, () => bridge);
            api.update();
            const totalRow = [...api.element.querySelectorAll('.tp-row')]
                .find(row => row.querySelector('span')?.textContent === 'E total');
            const thermoTotal = totalRow?.querySelector('span:last-child')?.textContent ?? null;
            api.dispose();
            host.remove();
            return { status: statusEnergy.textContent, thermo: thermoTotal };
        }

        try {
            const diagBase = { tick: 10, manifested: 0, totalWaveEnergy: 0, totalEnergy: 321 };
            return {
                baselineOnly: await runFixture({
                    diag: diagBase,
                    audit: null,
                    diagMeta: currentMeta(10),
                    auditMeta: failedMeta(10),
                }),
                nonfiniteDynamic: await runFixture({
                    diag: { ...diagBase, dynamicEnergy: Number.NaN },
                    audit: null,
                    diagMeta: currentMeta(10),
                    auditMeta: failedMeta(10),
                }),
                olderAudit: await runFixture({
                    diag: diagBase,
                    audit: { dynamicEnergy: 654, waveEnergy: 1, fieldEnergy: 2 },
                    diagMeta: currentMeta(10),
                    auditMeta: currentMeta(9),
                }),
                exactDiagZero: await runFixture({
                    diag: { ...diagBase, dynamicEnergy: 0 },
                    audit: null,
                    diagMeta: currentMeta(10),
                    auditMeta: failedMeta(10),
                }),
                exactAuditZero: await runFixture({
                    diag: diagBase,
                    audit: { dynamicEnergy: 0, waveEnergy: 0, fieldEnergy: 0 },
                    diagMeta: currentMeta(10),
                    auditMeta: currentMeta(10),
                }),
            };
        } finally {
            telemetryHub.collectScale0 = original.collectScale0;
            telemetryHub.collectScale0Audit = original.collectScale0Audit;
            telemetryHub.collectScale0Lagrangian = original.collectScale0Lagrangian;
            telemetryHub.getScale0TelemetryMeta = original.getScale0TelemetryMeta;
            telemetryHub.s0.diag = original.diag;
            telemetryHub.s0.audit = original.audit;
            telemetryHub._prevWantAudit = original.prevWantAudit;
            telemetryHub._prevWantLag = original.prevWantLag;
            telemetryHub._lastAuditVersion = original.lastAuditVersion;
        }
    });

    for (const unavailable of [
        result.baselineOnly,
        result.nonfiniteDynamic,
        result.olderAudit,
    ]) {
        expect(unavailable.status).toBe('—');
        expect(unavailable.thermo).toBe('—');
        expect(unavailable.status).not.toContain('321');
        expect(unavailable.status).not.toContain('654');
        expect(unavailable.thermo).not.toContain('321');
        expect(unavailable.thermo).not.toContain('654');
    }
    expect(result.exactDiagZero.status).toContain('0 sim');
    expect(result.exactDiagZero.thermo).toBe('0.000');
    expect(result.exactAuditZero.status).toContain('0 sim');
    expect(result.exactAuditZero.thermo).toBe('0.000');
});

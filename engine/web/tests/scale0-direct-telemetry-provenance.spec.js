// @ts-check
/** Direct-WASM telemetry provenance at paused/same-tick mutation boundaries. */
import { test, expect } from '@playwright/test';

test('same-tick direct mutations advance history, nulls go stale, and source boundaries reset drift', async ({ page }) => {
    await page.goto('/js/telemetry-hub.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const [{ TelemetryHub }, { WasmBridge }] = await Promise.all([
            import('/js/telemetry-hub.js?direct-provenance-contract=1'),
            import('/js/bridge/wasm-bridge.js?direct-provenance-contract=1'),
        ]);

        const owner = Object.create(WasmBridge.prototype);
        owner._module = {};
        owner._bridge = { currentTick: () => 7 };
        owner.ready = true;
        owner._lastScale0Audit = null;
        owner._lastScale0AuditTick = -1;
        owner._scale0TelemetrySourceEpoch = 1;
        owner._scale0TelemetryStateVersion = 1;
        owner._scale0TelemetryGroupMeta = new Map();

        let audit = { dynamicEnergy: 10, totalEnergy: 10, gaussViolation: 0 };
        let diagnostics = { tick: 7, totalFlux: 0 };
        owner.capabilities = {
            scale0: {
                getScale0Diagnostics: () => diagnostics,
                getScale0EnergyAudit: () => audit,
            },
        };

        const hub = new TelemetryHub();
        hub.collectScale0(owner, null, false);
        hub.collectScale0Audit(owner, null, false);
        const initial = {
            rows: hub.aud.dynamicEnergy.total,
            value: hub.aud.dynamicEnergy.last(),
            drift: hub.s0.audit.energyDrift,
            meta: hub.getScale0TelemetryMeta('audit'),
        };

        // A paused write changes the scientific record at the same engine tick.
        owner._markScale0StateChanged();
        audit = { dynamicEnergy: 12, totalEnergy: 12, gaussViolation: 0 };
        hub.collectScale0Audit(owner, null, false);
        const sameTick = {
            rows: hub.aud.dynamicEnergy.total,
            value: hub.aud.dynamicEnergy.last(),
            drift: hub.s0.audit.energyDrift,
            meta: hub.getScale0TelemetryMeta('audit'),
        };

        // A missing getter result hides the retained sample without appending a
        // fabricated zero. Recovery at the same state identity is accepted but
        // remains deduplicated.
        audit = null;
        hub.collectScale0Audit(owner, null, false);
        const unavailable = {
            rows: hub.aud.dynamicEnergy.total,
            value: hub.aud.dynamicEnergy.last(),
            meta: hub.getScale0TelemetryMeta('audit'),
        };
        audit = { dynamicEnergy: 12, totalEnergy: 12, gaussViolation: 0 };
        hub.collectScale0Audit(owner, null, false);
        const recovered = {
            rows: hub.aud.dynamicEnergy.total,
            meta: hub.getScale0TelemetryMeta('audit'),
        };

        // A configuration/source epoch is an intervention boundary. Its first
        // audit establishes a new conservation reference instead of reporting
        // drift against the previous experiment.
        owner._markScale0StateChanged(true);
        audit = { dynamicEnergy: 20, totalEnergy: 20, gaussViolation: 0 };
        hub.collectScale0Audit(owner, null, false);
        const boundary = {
            rows: hub.aud.dynamicEnergy.total,
            value: hub.aud.dynamicEnergy.last(),
            drift: hub.s0.audit.energyDrift,
            meta: hub.getScale0TelemetryMeta('audit'),
        };

        diagnostics = null;
        owner.ready = false;
        owner._markScale0StateChanged(true);
        hub.collectScale0(owner, null, false);
        const unreadyDiagnostics = hub.getScale0TelemetryMeta('diagnostics');

        return { initial, sameTick, unavailable, recovered, boundary, unreadyDiagnostics };
    });

    expect(result.initial).toMatchObject({ rows: 1, value: 10, drift: 0 });
    expect(result.initial.meta).toMatchObject({ tick: 7, stateVersion: 1, stale: false });

    expect(result.sameTick).toMatchObject({ rows: 2, value: 12, drift: 20 });
    expect(result.sameTick.meta).toMatchObject({ tick: 7, stateVersion: 2, stale: false });

    expect(result.unavailable).toMatchObject({ rows: 2, value: 12 });
    expect(result.unavailable.meta).toMatchObject({ stale: true, status: 'unavailable' });
    expect(result.recovered.rows).toBe(2);
    expect(result.recovered.meta).toMatchObject({ stateVersion: 2, stale: false });

    expect(result.boundary).toMatchObject({ rows: 3, value: 20, drift: 0 });
    expect(result.boundary.meta).toMatchObject({ sourceEpoch: 2, stateVersion: 3, stale: false });
    expect(result.unreadyDiagnostics).toMatchObject({ tick: null, stale: true, status: 'unavailable' });
});

import test from 'node:test';
import assert from 'node:assert/strict';
import { WasmBridge } from '../js/bridge/wasm-bridge.js';
import { RingBuffer, MultiRingBuffer, TelemetryHub } from '../js/telemetry-hub.js';

function directWasmFixture() {
    let tick = 7;
    const diagnostics = new Float64Array(23);
    diagnostics[0] = tick;
    diagnostics[7] = 41;
    const audit = new Float64Array(31);
    audit[0] = 2;
    audit[1] = 3;
    audit[2] = 5;
    audit[3] = 13;
    audit[19] = 8;
    audit[24] = 13;
    let auditCalls = 0;
    const bridge = Object.create(WasmBridge.prototype);
    Object.assign(bridge, {
        _bridge: {},
        _lastScale0Audit: null,
        _lastScale0AuditTick: -1,
        currentTick: () => tick,
        _module: {
            getDiagnosticsView: () => diagnostics,
            getEnergyLedger: () => ({ ECurr: 13 }),
            getEnergyAuditView: () => { auditCalls++; return audit; },
        },
    });
    return {
        bridge,
        calls: () => auditCalls,
        setTick(next) { tick = next; diagnostics[0] = next; },
    };
}

test('direct diagnostics stays on the ledger until the demanded audit has completed', () => {
    const f = directWasmFixture();
    const first = f.bridge.getDiagnostics();
    assert.equal(f.calls(), 0, 'ordinary diagnostics must not initiate an O(N^3) audit');
    assert.equal(first.dynamicEnergy, 13);
    assert.equal(first.energySampleSource, 'per-tick-ledger');
    assert.equal(Object.hasOwn(first, 'fieldEnergy'), false);

    const audit = f.bridge.getEnergyAudit();
    assert.equal(f.calls(), 1, 'the demand-owned audit getter initiates one reduction');
    assert.equal(audit.dynamicEnergy, 13);
    const enriched = f.bridge.getDiagnostics();
    assert.equal(f.calls(), 1, 'same-tick diagnostics joins the completed cache only');
    assert.equal(enriched.fieldEnergy, 2);
    assert.equal(enriched.vacuumBaselineEnergy, 41, 'ledger replacement does not erase the diagnostics baseline');

    f.setTick(8);
    const later = f.bridge.getDiagnostics();
    assert.equal(f.calls(), 1, 'a prior-tick audit is never recomputed or relabelled current');
    assert.equal(Object.hasOwn(later, 'fieldEnergy'), false);
});

test('history reset generations distinguish equal-length refills for cache invalidation', () => {
    const one = new RingBuffer(2);
    assert.equal(one.generation, 0);
    one.push(1, 1);
    one.clear();
    assert.equal(one.generation, 1);
    one.push(2, 2);
    one.clear();
    assert.equal(one.generation, 2);

    const multi = new MultiRingBuffer(2, ['a', 'b']);
    const view = multi.views.a;
    assert.equal(view.generation, 0);
    multi.push({ a: 1, b: 2 }, 1);
    multi.clear();
    assert.equal(multi.generation, 1);
    assert.equal(view.generation, 1, 'channel views expose their parent reset identity');
});

const metaAt = (tick, stateVersion = tick) => ({
    source: 'fixture', sourceEpoch: 1, stateVersion, tick, sampleTick: tick,
    stale: false, status: 'available', sampledAt: tick,
});

test('core energy history never retimestamps a retained audit onto a newer diagnostics tick', () => {
    const held = new TelemetryHub();
    held._publishScale0Audit({ dynamicEnergy: 99 }, metaAt(4));
    held._publishScale0Diagnostics({ totalFlux: 0, manifested: 0 }, metaAt(5));
    assert.equal(Number.isNaN(held.energy.last()), true,
        'an absent ledger value stays unavailable when the only audit belongs to an older tick');

    const joined = new TelemetryHub();
    joined._publishScale0Audit({ dynamicEnergy: 77 }, metaAt(6));
    joined._publishScale0Diagnostics({ totalFlux: 0, manifested: 0 }, metaAt(6));
    assert.equal(joined.energy.last(), 77,
        'an exact-tick audit remains a valid compatibility refinement');
    assert.equal(joined.s0.diag.dynamicEnergy, 77);
    assert.equal(joined.s0.diag.energySampleSource, 'same-tick-audit');

    const refined = new TelemetryHub();
    refined._publishScale0Diagnostics({ totalFlux: 0, manifested: 0, dynamicEnergy: 13 }, metaAt(7));
    refined._publishScale0Audit({ dynamicEnergy: 17 }, metaAt(7));
    assert.equal(refined.energy.last(), 17, 'same-tick audit refines the matching core-history row');
    assert.equal(refined.s0.diag.dynamicEnergy, 17, 'Diagnostics row remains coherent with that refinement');
    assert.equal(refined.s0.diag.energySampleSource, 'same-tick-audit');
});

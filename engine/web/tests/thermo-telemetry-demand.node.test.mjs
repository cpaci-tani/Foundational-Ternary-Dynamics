import test from 'node:test';
import assert from 'node:assert/strict';
import { getScale0TelemetryDemand } from '../js/telemetry/demand.js';

test('Thermo owns the audit it renders without requesting Lagrangian telemetry', () => {
    const visible = new Set(['thermo']);
    const demand = getScale0TelemetryDemand({
        bridge: { latticeSize: 97 },
        isPanelVisible: id => visible.has(id),
    }, null);
    assert.equal(demand.wantAudit, true);
    assert.equal(demand.wantLag, false);

    visible.clear();
    const hidden = getScale0TelemetryDemand({
        bridge: { latticeSize: 97 },
        isPanelVisible: id => visible.has(id),
    }, null);
    assert.equal(hidden.wantAudit, false);
});

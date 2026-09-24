import test from 'node:test';
import assert from 'node:assert/strict';
import { forceGunEffort } from '../js/observer/force-gun-effort.js';

test('effort colors describe the measured fraction of each force limit', () => {
    for (const cap of [2, 20, 120, 6000]) {
        for (const [fraction, color] of [[0, '#6de4ff'], [.5, '#ffe36d'], [.75, '#ff9d55'], [1, '#ff526b']]) {
            const effort = forceGunEffort({ forceMagnitude: fraction * cap, maxForce: cap, requestedForceMagnitude: fraction * cap });
            assert.equal(effort.color, color);
            assert.equal(effort.percent, fraction * 100);
            assert.equal(effort.overloaded, false, 'full effort alone does not mean the cap was exceeded');
        }
    }
    const partial = forceGunEffort({ forceMagnitude: 30, maxForce: 120 });
    assert.equal(partial.percent, 25);
    assert.notEqual(partial.color, '#6de4ff'); assert.notEqual(partial.color, '#ffe36d');
});

test('overload uses uncapped demand, tolerates roundoff, and recovers without sticky state', () => {
    const full = { forceMagnitude: 120, maxForce: 120, requestedForceMagnitude: 120 };
    assert.equal(forceGunEffort({ ...full, requestedForceMagnitude: 120 + 1e-8 }).overloaded, false);
    const overloaded = forceGunEffort({ ...full, requestedForceMagnitude: 180 });
    assert.equal(overloaded.percent, 100); assert.equal(overloaded.overloaded, true);
    assert.equal(overloaded.color, '#f178ff');
    assert.equal(forceGunEffort(full).overloaded, false);
    assert.equal(forceGunEffort({ forceMagnitude: 120, maxForce: 120 }).overloaded, false);
});

test('unavailable and invalid force measurements remain unknown', () => {
    for (const telemetry of [null, { forceMagnitude: 1, maxForce: 0 }, { forceMagnitude: NaN, maxForce: 20 },
        { forceMagnitude: -1, maxForce: 20 }, { forceMagnitude: 1, maxForce: Infinity }]) assert.equal(forceGunEffort(telemetry), null);
});

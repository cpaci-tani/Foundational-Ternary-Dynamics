// Node unit test for the pure orbit-period estimator.
// Run: node engine/web/tests/scale1-orbit-period.node.test.mjs
import assert from 'node:assert/strict';
import { estimateOrbitPeriod } from '../js/scales/scale1/telemetry/orbit-period.js';

// A clean circular 2-body orbit: separation returns to ~start every 10 ticks.
const history = [];
for (let tick = 0; tick <= 25; tick++) {
    const sep = 10 + Math.sin((tick / 10) * 2 * Math.PI) * 0.01; // returns near 10 every 10 ticks
    history.push({ tick, separation: sep });
}
const est = estimateOrbitPeriod(history);
assert.ok(est !== null, 'expected a period estimate for a returning orbit');
assert.ok(Math.abs(est - 10) <= 1, `expected ~10 ticks, got ${est}`);

// Never returns within tolerance (e.g. an escaping/unbound trajectory) -> null.
const escaping = [];
for (let tick = 0; tick <= 25; tick++) escaping.push({ tick, separation: 10 + tick });
assert.equal(estimateOrbitPeriod(escaping), null);

// Fewer than 2 samples -> null (nothing to compare against).
assert.equal(estimateOrbitPeriod([]), null);
assert.equal(estimateOrbitPeriod([{ tick: 0, separation: 5 }]), null);

console.log('scale1-orbit-period OK');

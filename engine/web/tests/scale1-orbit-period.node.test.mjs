// Node unit test for the pure orbit-period estimator.
// Run: node engine/web/tests/scale1-orbit-period.node.test.mjs
import assert from 'node:assert/strict';
import { estimateOrbitPeriod } from '../js/scales/scale1/telemetry/orbit-period.js';

// A clean bound orbit as a period-10 triangle wave (separation rises
// linearly from 10 to 20 over the first 5 ticks of each period, then falls
// back to 10 over the next 5): this departs tolerance immediately (avoiding
// a false match near tick 1, which a smooth sinusoid starting at an
// extremum would give — its curvature is ~0 there) and returns to within
// tolerance of the start value ONLY at the true period (a sinusoid
// starting at its extremum OR at its zero-crossing both spuriously
// re-match at the HALF period too, since either point recurs twice per
// cycle) — a triangle wave anchored at its minimum has no such ambiguity.
const history = [];
for (let tick = 0; tick <= 25; tick++) {
    const phase = tick % 10;
    const sep = phase <= 5 ? 10 + phase * 2 : 10 + (10 - phase) * 2;
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

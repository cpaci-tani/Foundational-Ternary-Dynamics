// Node unit test for the pure time-dilation math.
// Run: node engine/web/tests/time-analysis.node.test.mjs
import assert from 'node:assert/strict';
import * as T from '../js/scales/scale0/analysis/time-analysis.js';

// lapse, clock rate, slowdown
assert.equal(T.lapse(0), 1);                          // L=0 -> f=1 (flat)
assert.ok(Math.abs(T.lapse(0.5) - 0.75) < 1e-12);     // f = 1 - 0.25
assert.ok(Math.abs(T.clockRate(0.5) - Math.sqrt(0.75)) < 1e-12);
assert.ok(Math.abs(T.slowdownPct(0) - 0) < 1e-12);
assert.ok(T.slowdownPct(0.5) > 0);

// SR dilation + FTD generalized gamma
assert.ok(Math.abs(T.srDilation(0.6) - 0.8) < 1e-12);  // sqrt(1-0.36)=0.8
assert.ok(Math.abs(T.srGamma(0.6) - 1.25) < 1e-12);
// ftdGamma(L=0, v) reduces to SR gamma (f=1)
assert.ok(Math.abs(T.ftdGamma(0, 0.6) - T.srGamma(0.6)) < 1e-12);

// proper-time accumulation
assert.ok(Math.abs(T.properTimeStep(0.5, 2.0) - Math.sqrt(0.75) * 2.0) < 1e-12);

// radial profile bins samples by distance from center
const c = { x: 4, y: 4, z: 4 };
const positions = new Float32Array([4, 4, 4, 6, 4, 4]); // center, then r=2
const values = new Float32Array([0.0, 0.5]);            // L at each
const prof = T.radialProfile(positions, values, c);
assert.equal(prof.length, 2);
assert.ok(prof[0].r < prof[1].r);
assert.ok(Math.abs(prof[0].dtau_dt - 1) < 1e-12);

console.log('time-analysis OK');

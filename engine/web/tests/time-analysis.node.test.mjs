// Node unit test for the pure time-dilation math.
// Run: node engine/web/tests/time-analysis.node.test.mjs
import assert from 'node:assert/strict';
import * as T from '../js/scales/scale0/analysis/time-analysis.js';
import { C_SPEED } from '../js/constants.js';

// lapse, clock rate, slowdown
assert.equal(T.lapse(0), 1);                          // L=0 -> f=1 (flat)
assert.ok(Math.abs(T.lapse(0.5) - 0.75) < 1e-12);     // f = 1 - 0.25
assert.ok(Math.abs(T.clockRate(0.5) - Math.sqrt(0.75)) < 1e-12);
assert.ok(Math.abs(T.slowdownPct(0) - 0) < 1e-12);
assert.ok(T.slowdownPct(0.5) > 0);

// SR dilation + FTD generalized gamma
const halfC = 0.5 * C_SPEED;
assert.ok(Math.abs(T.betaSquared(halfC) - 0.25) < 1e-12);
assert.ok(Math.abs(T.srDilation(0.6 * C_SPEED) - 0.8) < 1e-12);
assert.ok(Math.abs(T.srGamma(0.6 * C_SPEED) - 1.25) < 1e-12);
// ftdGamma(L=0, u_raw) reduces to flat gamma at the same raw speed.
assert.ok(Math.abs(T.ftdGamma(0, 0.6 * C_SPEED) - T.srGamma(0.6 * C_SPEED)) < 1e-12);
assert.ok(Math.abs(T.clockRate(0, halfC) ** 2 - 0.75) < 1e-12);
assert.equal(T.clockRate(0, C_SPEED), 0);

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

const summary = T.radialSummary(positions, values, c, 3, 4);
assert.equal(summary.hasField, true);
assert.ok(Math.abs(summary.lDeep - 0.5) < 1e-12);
assert.ok(Math.abs(summary.lFar - 0.0) < 1e-12);
assert.ok(summary.bins.length > 0);
assert.ok(summary.bins.every((b) => b.dtau_dt >= 0 && b.dtau_dt <= 1));

console.log('time-analysis OK');

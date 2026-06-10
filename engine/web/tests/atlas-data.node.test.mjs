// Node unit test for the Ontology Atlas static analytic field math.
// Run: node engine/web/tests/atlas-data.node.test.mjs
import assert from 'node:assert/strict';
import * as D from '../js/atlas/atlas-data.js';

// flux of a single +1 charge at origin: points radially out, ~1/r^2
const f = D.fluxFromCharges([{ pos: { x: 0, y: 0, z: 0 }, q: 1 }], { x: 2, y: 0, z: 0 });
assert.ok(f.x > 0 && Math.abs(f.y) < 1e-9 && Math.abs(f.z) < 1e-9);
const near = D.fluxFromCharges([{ pos: { x: 0, y: 0, z: 0 }, q: 1 }], { x: 1, y: 0, z: 0 });
assert.ok(near.x > f.x * 3.5);                         // ~1/r^2: |E(1)|/|E(2)| ~ 4

// divergence: positive near +charge, negative near −charge
assert.ok(D.divFlux([{ pos: { x: 0, y: 0, z: 0 }, q: 1 }], { x: 0.2, y: 0, z: 0 }) > 0);
assert.ok(D.divFlux([{ pos: { x: 0, y: 0, z: 0 }, q: -1 }], { x: 0.2, y: 0, z: 0 }) < 0);

// latency well in [0,1), deeper (larger) closer to the mass
const lc = D.latencyWell([{ pos: { x: 0, y: 0, z: 0 }, m: 0.5 }], { x: 0.1, y: 0, z: 0 });
const lf = D.latencyWell([{ pos: { x: 0, y: 0, z: 0 }, m: 0.5 }], { x: 0.9, y: 0, z: 0 });
assert.ok(lc > lf && lc < 1 && lf >= 0);

// ternary from divergence
assert.equal(D.stateFromDiv(1.0), 1);
assert.equal(D.stateFromDiv(-1.0), -1);
assert.equal(D.stateFromDiv(0.0), 0);

// psi packet returns finite complex
const ps = D.psiPacket({ x: 3, y: 0, z: 0 }, { x: 0, y: 0, z: 0 }, { x: 0.2, y: 0, z: 0 }, 0.0);
assert.ok(Number.isFinite(ps.re) && Number.isFinite(ps.im));

// sampleGrid returns n^3 points
assert.equal(D.sampleGrid(3, () => 1).length, 27);

console.log('atlas-data OK');

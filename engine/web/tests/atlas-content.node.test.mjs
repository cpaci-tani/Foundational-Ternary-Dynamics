// Node unit test for the Ontology Atlas content + chain integrity + tag honesty.
// Run: node engine/web/tests/atlas-content.node.test.mjs
import assert from 'node:assert/strict';
import { LAYERS, GROUPS } from '../js/atlas/atlas-content.js';
import { STAGES, STAGE_COUNT } from '../js/atlas/atlas-chain.js';

const ids = Object.keys(LAYERS);
assert.ok(ids.length >= 13);
for (const id of ids) {
  const L = LAYERS[id];
  for (const field of ['group', 'symbol', 'name', 'definition', 'tag', 'doc']) {
    assert.ok(L[field], `${id}.${field} missing`);
  }
  assert.ok(GROUPS.some((g) => g.id === L.group), `${id} group ${L.group} unknown`);
  for (const t of (L.flowsTo || [])) assert.ok(LAYERS[t], `${id} flowsTo unknown ${t}`);
}

// honesty assertions — these must hold or the viz overclaims
assert.match(LAYERS.psi.tag, /SELECTION/);
assert.match(LAYERS.M.tag, /DECLIN|FC-1|CLOSED/);
assert.match(LAYERS.psiWave.tag, /SELECTION/);
assert.match(LAYERS.latency.tag, /imposed/);
assert.equal(LAYERS.J.group, 'full');
assert.equal(LAYERS.M.group, 'declined');

// chain
assert.equal(STAGE_COUNT, 14);
assert.equal(STAGES.length, 14);
for (const s of STAGES) {
  for (const id of s.layersOn) assert.ok(LAYERS[id] || id === 'lattice', `stage ${s.id} unknown layer ${id}`);
}

console.log('atlas-content OK');

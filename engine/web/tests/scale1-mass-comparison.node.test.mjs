// engine/web/tests/scale1-mass-comparison.node.test.mjs
// Run: node engine/web/tests/scale1-mass-comparison.node.test.mjs
import assert from 'node:assert/strict';
import { compareClusterToVoxelMass } from '../js/scales/scale1/telemetry/mass-comparison.js';

const K_B = 0.511;

// Cluster seed: N=6 -> mass 6*K_B. Voxel snapshot: 6 member voxels, masses
// [K_B, K_B, K_B, K_B, K_B, K_B*2] (one denser voxel above the K_B floor).
const seed = { mass: 6 * K_B, size: 6 };
const voxelMasses = [K_B, K_B, K_B, K_B, K_B, K_B * 2];
const cmp = compareClusterToVoxelMass(seed, voxelMasses);

assert.ok(Math.abs(cmp.clusterMass - 6 * K_B) < 1e-12);
assert.ok(Math.abs(cmp.voxelMass - 7 * K_B) < 1e-12);
assert.ok(Math.abs(cmp.delta - (cmp.voxelMass - cmp.clusterMass)) < 1e-12);
assert.equal(cmp.clusterTag, '[DERIVED-linear]/[SMC]');
assert.equal(cmp.voxelTag, '[IMPOSED]');

// No voxel snapshot available -> voxelMass/delta are null, clusterMass still set.
const cmpNone = compareClusterToVoxelMass(seed, null);
assert.ok(Math.abs(cmpNone.clusterMass - 6 * K_B) < 1e-12);
assert.equal(cmpNone.voxelMass, null);
assert.equal(cmpNone.delta, null);

console.log('scale1-mass-comparison OK');

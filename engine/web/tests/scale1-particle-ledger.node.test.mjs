// Run: node engine/web/tests/scale1-particle-ledger.node.test.mjs
import assert from 'node:assert/strict';
import {
    buildParticleHierarchy,
    Scale1ParticleLedger,
} from '../js/scales/scale1/telemetry/particle-ledger.js';

function frame(rows) {
    const count = rows.length;
    const positions = new Float32Array(count * 3);
    const velocities = new Float32Array(count * 3);
    const masses = new Float64Array(count);
    const rEff = new Float32Array(count);
    const charges = new Int8Array(count);
    const ids = new Int32Array(count);
    const locked = new Uint8Array(count);
    rows.forEach((row, index) => {
        positions.set(row.position, index * 3);
        velocities.set(row.velocity || [0, 0, 0], index * 3);
        masses[index] = row.mass ?? 1;
        rEff[index] = row.rEff ?? 0.2;
        charges[index] = row.charge ?? 0;
        ids[index] = row.id;
    });
    return { count, positions, velocities, masses, rEff, charges, ids, locked };
}

function snapshot(rows, tick, events = []) {
    return {
        core: { tick, scenario: 'ledger-test', backend: 'wasm32', mode: 'effective_lab', readOnly: false },
        objects: rows.map(row => ({
            id: row.id,
            position: { x: row.position[0], y: row.position[1], z: row.position[2] },
            velocity: { x: row.velocity?.[0] || 0, y: row.velocity?.[1] || 0, z: row.velocity?.[2] || 0 },
            kineticEnergyAvailable: false,
            mass: row.mass ?? 1,
            effectiveRadius: row.rEff ?? 0.2,
            effectiveState: row.charge ?? 0,
            parentIds: [],
            provenance: { sourceKind: 'test_seed', status: 'imposed' },
        })),
        events,
        conservation: {
            stateEnergy: rows.reduce((sum, row) => {
                const speed2 = (row.velocity?.[0] || 0) ** 2
                    + (row.velocity?.[1] || 0) ** 2
                    + (row.velocity?.[2] || 0) ** 2;
                return sum + 0.5 * (row.mass ?? 1) * speed2;
            }, 0),
            stateEnergyComplete: true,
            coveredMask: 3,
            missingMask: 0,
            nonconservativeMask: 0,
        },
    };
}

const rows = [
    { id: 10, position: [0, 0, 0], velocity: [0.1, 0, 0], mass: 1 },
    { id: 11, position: [1, 0, 0], velocity: [1, 0, 0], mass: 1 },
    { id: 20, position: [20, 0, 0], velocity: [0.1, 0, 0], mass: 1 },
    { id: 21, position: [21, 0, 0], velocity: [1.5, 0, 0], mass: 1 },
];
const hierarchy = buildParticleHierarchy({ peData: frame(rows), snapshot: snapshot(rows, 0) });
assert.equal(hierarchy.particles.length, 4, 'all published particles must be represented');
assert.equal(hierarchy.clusters.length, 2, 'separated local pairs should form two clusters');
assert.deepEqual(hierarchy.clusters.map(cluster => cluster.particles.map(p => p.id)), [[20, 21], [10, 11]]);
assert.equal(hierarchy.clusters[0].anchorId, 21, 'highest-activity member should anchor the first pair');
assert.equal(hierarchy.clusters[1].anchorId, 11, 'highest-activity member should anchor the second pair');
assert.equal(hierarchy.energyBasis, 'dynamic_activity');
assert.ok(hierarchy.particles.every(p => p.id === hierarchy.clusters
    .find(cluster => cluster.particles.some(member => member.id === p.id))
    .particles.find(member => member.id === p.id).id));

const dormant = [
    { id: 1, position: [0, 0, 0], mass: 1 },
    { id: 2, position: [10, 0, 0], mass: 10 },
];
const dormantHierarchy = buildParticleHierarchy({ peData: frame(dormant), snapshot: snapshot(dormant, 0) });
assert.equal(dormantHierarchy.energyBasis, 'mass_fallback');
assert.equal(dormantHierarchy.globalAnchorId, 2);

const ledger = new Scale1ParticleLedger({ maxEvents: 128 });
ledger.beginScenario({ scenarioId: 'ledger-test', label: 'Ledger test', tick: 0 });
ledger.observe({ peData: frame(rows), snapshot: snapshot(rows, 0), scenarioId: 'ledger-test' });
assert.equal(ledger.getView().hierarchy.particles.length, 4);
assert.equal(ledger.getView().events.filter(event => event.type === 'spawn').length, 4);
assert.ok(ledger.getView().events.some(event => event.type === 'hierarchy_initialized'));

const energeticRows = rows.map(row => row.id === 11 ? { ...row, velocity: [2, 0, 0] } : row);
ledger.observe({ peData: frame(energeticRows), snapshot: snapshot(energeticRows, 10), scenarioId: 'ledger-test' });
assert.ok(ledger.getView().events.some(event =>
    event.type === 'particle_energy_change' && event.particleIds.includes(11)));
assert.equal(ledger.getView().events.filter(event => event.type === 'system_energy_change').length, 1);

const survivors = energeticRows.filter(row => row.id !== 10 && row.id !== 11);
const contact = {
    sequence: 0,
    tick: 11,
    type: 'contact_removal',
    participantA: 10,
    participantB: 11,
    stateEnergyDelta: -2,
    accountingComplete: true,
    status: 'selection',
    sourceId: 'contact_events',
};
ledger.observe({ peData: frame(survivors), snapshot: snapshot(survivors, 11, [contact]), scenarioId: 'ledger-test' });
const contactEvents = ledger.getView().events.filter(event => event.type === 'contact_removal');
assert.equal(contactEvents.length, 1, 'native event sequence must be logged exactly once');
assert.equal(contactEvents[0].status, 'selection');
assert.equal(ledger.getView().events.filter(event => event.type === 'system_energy_change').length, 1,
    'system energy changes must be cooldown-gated instead of flooding every tick');
assert.equal(ledger.getView().events.filter(event =>
    event.type === 'despawn' && event.particleIds.some(id => id === 10 || id === 11)).length, 0,
    'native removals must not be duplicated as unexplained despawns');

console.log('scale1-particle-ledger OK');

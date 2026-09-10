import test from 'node:test';
import assert from 'node:assert/strict';
import { FLUID_SCENARIO_CONTRACT as contract, assessFluidScenarioEligibility as assess } from '../js/scales/scale0/fluid-scenario-contract.js';

const law = Object.freeze({ lawId: 'fixture-finite-law-v1', tableHash: 'a'.repeat(64), encodingHash: 'b'.repeat(64) });
function admitted(id = 's0-fluid-equilibrium') {
    const definition = contract[id];
    return {
        expectedLawIdentity: law,
        capability: { lawIdentity: law, model: 'finite-fluid-law', stateOwner: 'render-bridge',
            boundaryDomains: [definition.boundaryDomain], observables: [...definition.requestedObservables] },
        gateEvidence: Object.fromEntries(definition.requiredPhysicalGates.map(gate => [gate,
            { lawIdentity: law, status: 'passed', evidence: [{ artifactId: `reviewed/${gate}`, sha256: 'c'.repeat(64) }] }])),
    };
}

test('nine prospective scenarios are recursively frozen and display native number density', () => {
    assert.deepEqual(Object.keys(contract), ['equilibrium', 'expansion', 'counterflow', 'sound', 'thermal-contact', 'shear', 'shock-tube', 'vortex-decay', 'obstacle-wake'].map(id => `s0-fluid-${id}`));
    function assertFrozen(value) {
        if (!value || typeof value !== 'object') return;
        assert.equal(Object.isFrozen(value), true);
        Object.values(value).forEach(assertFrozen);
    }
    assertFrozen(contract);
    for (const entry of Object.values(contract)) {
        assert.equal(entry.defaultDisplay.kind, 'density-volume');
        assert.equal(entry.defaultDisplay.observable, 'number-density');
        assert.equal(new Set(entry.requiredPhysicalGates).size, entry.requiredPhysicalGates.length);
    }
});

test('no identity, absent evidence and existing wave capability remain ineligible', () => {
    assert.equal(assess('s0-fluid-equilibrium').eligible, false);
    assert.equal(assess('s0-fluid-equilibrium', null).eligible, false);
    const noEvidence = admitted(); delete noEvidence.gateEvidence;
    assert.equal(assess('s0-fluid-equilibrium', noEvidence).eligible, false);
    const wave = admitted(); wave.capability.model = 'wave-field';
    assert.equal(assess('s0-fluid-equilibrium', wave).eligible, false);
    const secondOwner = admitted(); secondOwner.capability.stateOwner = 'hydro-worker';
    assert.equal(assess('s0-fluid-equilibrium', secondOwner).eligible, false);
});

test('all nine scenarios admit only complete declared capabilities and passed receipts', () => {
    for (const id of Object.keys(contract)) {
        const input = admitted(id), before = JSON.stringify(input), result = assess(id, input);
        assert.equal(result.eligible, true, id);
        assert.equal(JSON.stringify(input), before, 'assessment does not mutate evidence');
        assert.ok(Object.isFrozen(result) && Object.isFrozen(result.missingGates));
    }
});

test('each law-identity component is checked on both capability and every gate', () => {
    for (const [field, replacement] of Object.entries({ lawId: 'different-law', tableHash: 'd'.repeat(64), encodingHash: 'e'.repeat(64) })) {
        const capability = admitted(); capability.capability.lawIdentity = { ...law, [field]: replacement };
        assert.equal(assess('s0-fluid-equilibrium', capability).eligible, false);
        const evidence = admitted(); evidence.gateEvidence['thermal-relaxation'].lawIdentity = { ...law, [field]: replacement };
        const result = assess('s0-fluid-equilibrium', evidence);
        assert.equal(result.eligible, false); assert.ok(result.missingGates.includes('thermal-relaxation'));
    }
});

test('every required gate fails closed for missing, failed, empty or unpinned evidence', () => {
    for (const gate of contract['s0-fluid-shear'].requiredPhysicalGates) {
        for (const invalid of [undefined, { status: 'failed', lawIdentity: law, evidence: [{ artifactId: 'failure', sha256: 'c'.repeat(64) }] },
            { status: 'passed', lawIdentity: law, evidence: [] }, { status: 'passed', lawIdentity: law, evidence: [{ artifactId: 'unhashed' }] }]) {
            const input = admitted('s0-fluid-shear'); input.gateEvidence[gate] = invalid;
            const result = assess('s0-fluid-shear', input);
            assert.equal(result.eligible, false); assert.ok(result.missingGates.includes(gate));
        }
    }
});

test('periodic admission cannot authorize wall, reservoir or missing thermal observables', () => {
    for (const id of ['s0-fluid-shock-tube', 's0-fluid-obstacle-wake']) {
        const input = admitted(id); input.capability.boundaryDomains = ['closed-periodic'];
        assert.equal(assess(id, input).eligible, false);
    }
    const input = admitted('s0-fluid-thermal-contact');
    input.capability.observables = input.capability.observables.filter(name => name !== 'heat-flux');
    assert.equal(assess('s0-fluid-thermal-contact', input).eligible, false);
});

test('unknown IDs and inherited gate properties do not create admission', () => {
    for (const id of ['toString', '__proto__', 'flux-vortex', null]) assert.equal(assess(id, admitted()).eligible, false);
    const input = admitted(); input.gateEvidence = Object.create(input.gateEvidence);
    assert.equal(assess('s0-fluid-equilibrium', input).eligible, false);
});

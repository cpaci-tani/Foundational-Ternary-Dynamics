import test from 'node:test';
import assert from 'node:assert/strict';
import { updatePEFields } from '../js/inspector/scales/particles.js';
import { formatMass } from '../js/particle-catalog.js';
import { formatLength } from '../js/units.js';

function inspect(data, catalogId = null) {
    const peFields = new Proxy({}, {get: (fields, key) => fields[key] ??= {textContent: '', style: {}}});
    updatePEFields({
        _selectedPEParticleId: data.id, bridge: {peInspectParticle: () => data},
        _peTypeMap: new Map(catalogId ? [[data.id, catalogId]] : []), peFields,
    });
    return peFields;
}

test('inspection uses actual native mass and charge and Scale 1 radius units', () => {
    const fields = inspect({id: 2, mass: 2, charge: 2, rEff: 0.5}, 'delta_pp');
    assert.equal(fields.mass.textContent, formatMass(2));
    assert.equal(fields.charge.textContent, '+2');
    assert.equal(fields.rEff.textContent, formatLength(0.5, 1).text);
    assert.notEqual(fields.rEff.textContent, formatLength(0.5, 2).text);
});

test('unclassified finite records leave absent continuum quantities unavailable', () => {
    const data = {id: 7};
    for (const key of ['mass', 'charge', 'rEff', 'spin', 'colorId', 'pairId', 'x', 'y', 'z',
        'vx', 'vy', 'vz', 'speed', 'ke', 'momentum', 'acceleration', 'orbitalR', 'nearestId',
        'nearestDist', 'fCoulombNearest', 'fNetMag']) data[key] = null;
    const fields = inspect(data);
    assert.equal(fields.mass.textContent, 'unavailable');
    for (const key of ['charge','rEff','spin','color','pair','pos','vel','speed','ke','momentum',
        'accel','orbital','nearest','dist','fc','fnet']) assert.equal(fields[key].textContent, '--', key);
    const zero = inspect({id: 7, spin: 0, colorId: 0, momentum: 0, acceleration: 0});
    assert.equal(zero.spin.textContent, '0');
    assert.equal(zero.momentum.textContent, '0.00e+0');
});

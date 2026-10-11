import test from 'node:test';
import assert from 'node:assert/strict';
import { sections as atomSections } from '../js/ui/panels/diagnostics-panel/descriptors/scale2.js';
import { sections as moleculeSections } from '../js/ui/panels/diagnostics-panel/descriptors/scale3.js';
import { charts as moleculeCharts } from '../js/ui/panels/charts-panel/descriptors/scale3.js';
import { updateCosmicFields } from '../js/inspector/scales/cosmic.js';
import { CosmicMockBridge } from '../js/bridge/mock-scale5.js';
import { BOHR_RADIUS_ANGSTROM, formatLength } from '../js/units.js';

const rowsOf = (sections, id) => sections.find((section) => section.id === id).rows;

function inspectCosmic(bridge, id) {
    const cosmicFields = new Proxy({}, {get: (fields, key) => fields[key] ??= {textContent: '', style: {}}});
    updateCosmicFields({_selectedCosmicId: id, bridge, cosmicFields});
    return cosmicFields;
}

test('Scale 3 geometry rows use the atom engine length unit, as Scale 2 softening does', () => {
    const softening = rowsOf(atomSections, 'ae-runtime').find((row) => row.id === 'softening');
    const units = Object.fromEntries(rowsOf(moleculeSections, 'mol-geometry').map((row) => [row.id, row.unit]));
    assert.equal(softening.unit, 'a₀');
    for (const id of ['mol-radius', 'mol-bond-mean', 'mol-bond-min', 'mol-bond-max']) {
        assert.equal(units[id], softening.unit, id);
    }
    assert.equal(units['mol-dipole'], `e·${softening.unit}`);
    // The inspector shows the same lengths in ångströms: one unit is one Bohr radius.
    assert.equal(formatLength(1, 2).value, BOHR_RADIUS_ANGSTROM);
});

test('Scale 3 geometry chart series carry the same units as the diagnostics rows', () => {
    const series = moleculeCharts.find((chart) => chart.id === 'mol-geometry').series;
    const units = Object.fromEntries(series.map((entry) => [entry.buffer, entry.unit]));
    assert.equal(units.aeMolRadius, 'a₀');
    assert.equal(units.aeMolDipole, 'e·a₀');
});

test('Scale 5 inspector labels raw radius, speed and age in the engine units', () => {
    const bridge = new CosmicMockBridge();
    const id = bridge.addBody(CosmicMockBridge.TYPE.STAR, 8, 0, 0, 0, 3, 4, 0);
    const body = bridge.cosmicInspectBody(id);
    // No conversion is applied: these are the stored lattice values.
    assert.equal(body.radius, Math.cbrt(8) * 0.1);
    assert.equal(body.speed, 5);
    assert.equal(body.age, 0);

    const fresh = inspectCosmic(bridge, id);
    assert.equal(fresh.radius.textContent, `${body.radius.toFixed(2)} lu`);
    assert.equal(fresh.speed.textContent, '5.00 (sim)');
    assert.equal(fresh.age.textContent, '--');

    bridge._bodies[0].age = 1234;
    const aged = inspectCosmic(bridge, id);
    assert.match(aged.age.textContent, /^1\D?234 ticks$/);
    for (const key of ['radius', 'speed', 'age']) {
        assert.doesNotMatch(aged[key].textContent, /R☉|km\/s|Myr/, key);
    }
});

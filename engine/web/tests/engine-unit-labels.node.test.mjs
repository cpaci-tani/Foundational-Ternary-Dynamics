import test from 'node:test';
import assert from 'node:assert/strict';
import { sections as atomSections } from '../js/ui/panels/diagnostics-panel/descriptors/scale2.js';
import { sections as moleculeSections } from '../js/ui/panels/diagnostics-panel/descriptors/scale3.js';
import { charts as moleculeCharts } from '../js/ui/panels/charts-panel/descriptors/scale3.js';
import { updateCosmicFields } from '../js/inspector/scales/cosmic.js';
import { CosmicMockBridge } from '../js/bridge/mock-scale5.js';
import { BOHR_RADIUS_ANGSTROM, formatLength } from '../js/units.js';
import * as constants from '../js/constants.js';
import { getInspectorPanelTemplate } from '../js/ui/components/panel-resources/template.js';

const rowsOf = (sections, id) => sections.find((section) => section.id === id).rows;
const COSMIC = CosmicMockBridge.TYPE;

function inspectCosmic(bridge, id) {
    const cosmicFields = new Proxy({}, {get: (fields, key) => fields[key] ??= {textContent: '', style: {}}});
    updateCosmicFields({_selectedCosmicId: id, bridge, cosmicFields});
    return cosmicFields;
}

/** Runs the bridge's own scenario telemetry pass over a hand-built body list. */
function cosmicTelemetry(scenarioName, build) {
    const bridge = new CosmicMockBridge();
    bridge._scenarioName = scenarioName;
    build(bridge);
    bridge._updateTelemetry();
    return bridge.getDiagnostics().customTelemetry;
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

test('Scale 5 inspector shows the Chandrasekhar and TOV anchors at their declared solar masses', () => {
    const bridge = new CosmicMockBridge();
    const shown = (mass) => inspectCosmic(bridge, bridge.addBody(COSMIC.STAR, mass, 0, 0, 0)).mass.textContent;
    // constants.js declares 70 lattice mass units as about 1.4 solar masses and 150 as about 3.
    assert.equal(shown(constants.M_CHANDRA_LATTICE), '1.4 M☉');
    assert.equal(shown(constants.M_TOV_LATTICE), '3.0 M☉');
    assert.equal(shown(2.19), '0.0438 M☉');
    // A large mass keeps its magnitude instead of being silently divided by a million.
    assert.equal(shown(1e8), '2.00e+6 M☉');
});

test('Scale 5 calls one solar mass the mass its stellar rule gives the solar temperature', () => {
    const bridge = new CosmicMockBridge();
    bridge._toggles.stellar_evolution = true;
    const id = bridge.addBody(COSMIC.STAR, constants.LATTICE_MASS_PER_SOLAR_MASS, 0, 0, 0);
    bridge._postUpdates();
    // cosmic-postupdates.js main sequence: 5800 * (mass / 50)^0.5.
    assert.equal(bridge.cosmicInspectBody(id).temperature, 5800);
    assert.equal(inspectCosmic(bridge, id).mass.textContent, '1.0 M☉');
});

test('Scale 5 scenario telemetry reports solar masses on the same anchor as the inspector', () => {
    const blackHole = cosmicTelemetry('cosmic-black-hole', (b) => b.addBody(COSMIC.BLACK_HOLE, 100, 0, 0, 0));
    assert.equal(blackHole['BH Mass'], '2.00 M⊙');
    const collapse = cosmicTelemetry('cosmic-ftd-collapse', (b) => b.addBody(COSMIC.BLACK_HOLE, 75, 0, 0, 0));
    assert.equal(collapse['BH Mass'], '1.50 M⊙');
    const merged = cosmicTelemetry('cosmic-merger', (b) => b.addBody(COSMIC.BLACK_HOLE, 6000, 0, 0, 0));
    assert.equal(merged['Singularity Mass'], '120.0 M⊙');
    // Two 25-unit stars inside the r < 10 core: one solar mass in a sphere of volume 4π/3 · 1000 lu³.
    const cluster = cosmicTelemetry('cosmic-globular-cluster', (b) => {
        b.addBody(COSMIC.STAR, 25, 1, 0, 0);
        b.addBody(COSMIC.STAR, 25, 0, 1, 0);
    });
    assert.equal(cluster['Core Density'], '2.39e-4 M⊙/lu³');
});

test('Scale 5 luminosity and jet readouts carry engine units, not solar luminosities or watts', () => {
    const bridge = new CosmicMockBridge();
    // A star stores mass^3.5 in lattice mass units: 8^3.5 = 512 * sqrt(8) = 1448.2.
    const star = bridge.addBody(COSMIC.STAR, 8, 0, 0, 0);
    assert.equal(inspectCosmic(bridge, star).lum.textContent, '1.45e+3 (sim)');
    // A black hole's luminosity field is the jet-intensity gauge.
    const hole = bridge.addBody(COSMIC.BLACK_HOLE, 100, 20, 0, 0);
    bridge._bodies[1].luminosity = 12.5;
    assert.equal(inspectCosmic(bridge, hole).lum.textContent, '1.25e+1 (sim)');

    const disk = cosmicTelemetry('cosmic-black-hole', (b) => {
        b.addBody(COSMIC.BLACK_HOLE, 100, 0, 0, 0);
        b._bodies[0].luminosity = 12.5;
    });
    assert.equal(disk['Accretion Disk Lum'], '1.25e+1 (sim)');
    const binary = cosmicTelemetry('cosmic-binary-agn', (b) => {
        b.addBody(COSMIC.BLACK_HOLE, 200, -5, 0, 0);
        b.addBody(COSMIC.BLACK_HOLE, 200, 5, 0, 0);
        b._bodies[0].luminosity = 12;
        b._bodies[1].luminosity = 30;
    });
    assert.equal(binary['Peak Jet Power'], '3.00e+1 (sim)');
});

test('Scale 5 inspector keeps kelvin for stellar bodies and uses engine units for gas', () => {
    const bridge = new CosmicMockBridge();
    const shown = (type, temperature) =>
        inspectCosmic(bridge, bridge.addBody(type, 1, 0, 0, 0, 0, 0, 0, temperature)).temp.textContent;
    // Stellar temperatures are the rules' own kelvin-scale inputs.
    assert.match(shown(COSMIC.STAR, 5800), /^5\D?800 K$/);
    assert.match(shown(COSMIC.WHITE_DWARF, 12000), /^12\D?000 K$/);
    assert.match(shown(COSMIC.NEUTRON_STAR, 1e6), /^1\D?000\D?000 K$/);
    // Gas temperature is 1000 × internal energy in engine units, with no kelvin conversion.
    assert.equal(shown(COSMIC.GAS, 150), '150 (sim)');
    assert.match(shown(COSMIC.NEBULA, 1e6), /^1\D?000\D?000 \(sim\)$/);
    // So the column header cannot promise kelvin for every body.
    const header = /<dt[^>]*>([^<]*)<\/dt>\s*<dd id="cosmic-insp-temp">/.exec(getInspectorPanelTemplate())[1];
    assert.doesNotMatch(header, /\bK\b/);
});

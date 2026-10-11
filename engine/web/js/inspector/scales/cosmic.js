import { setInspectorSectionVisibility } from '../chrome.js';
import { LATTICE_MASS_PER_SOLAR_MASS } from '../../constants.js';

// Star, Neutron Star, White Dwarf (CosmicMockBridge.TYPE): the bodies whose
// temperature the stellar rules set directly as a kelvin-scale number.
const STELLAR_TYPES = new Set([2, 3, 5]);

export function handleCosmicClick(target, intersects) {
    if (intersects.length > 0) {
        const hit = intersects[0];
        let rawId = -1;

        if (hit.object.userData && hit.object.userData.ids) {
            const geoIdx = hit.index;
            rawId = hit.object.userData.ids[geoIdx];
        } else if (hit.object.userData && hit.object.userData.id !== undefined) {
            rawId = hit.object.userData.id;
        }

        if (rawId >= 0) {
            target._selectedCosmicId = rawId;
            showCosmicInspector(target);
            return;
        }
    }
    target._selectedCosmicId = -1;
    hideCosmicInspector(target);
}

export function showCosmicInspector(target) {
    setInspectorSectionVisibility(target.cosmicEmptyEl, target.cosmicContentEl, true);
    updateCosmicFields(target);
    target._updateInspectorChrome();
}

export function hideCosmicInspector(target) {
    setInspectorSectionVisibility(target.cosmicEmptyEl, target.cosmicContentEl, false);
    target._updateInspectorChrome();
}

export function updateCosmicFields(target) {
    if (target._selectedCosmicId < 0 || !target.bridge.cosmicInspectBody) return;
    const body = target.bridge.cosmicInspectBody(target._selectedCosmicId);
    if (!body) {
        hideCosmicInspector(target);
        return;
    }

    const typeNames = {
        '-3': 'Dark Energy', '-2': 'Quasar', '-1': 'Black Hole',
        '0': 'Dark Matter', '1': 'Gas Cloud', '2': 'Star',
        '3': 'Neutron Star', '4': 'Nebula', '5': 'White Dwarf',
    };
    const colors = {
        '-3': '#5b21b6', '-2': '#facc15', '-1': '#000000',
        '0': '#7c3aed', '1': '#38bdf8', '2': '#fbbf24',
        '3': '#94a3b8', '4': '#f472b6', '5': '#f8fafc',
    };

    if (target.cosmicFields.type) target.cosmicFields.type.textContent = typeNames[body.type] || 'Unknown';
    if (target.cosmicFields.dot) {
        target.cosmicFields.dot.style.background = colors[body.type] || '#ccc';
        target.cosmicFields.dot.style.border = body.type === -1 ? '1px solid #aaa' : 'none';
    }

    if (target.cosmicFields.id) target.cosmicFields.id.textContent = body.id;

    // Solar masses are lattice mass divided by LATTICE_MASS_PER_SOLAR_MASS:
    // the declared anchors put 70 lattice mass units at about 1.4 solar
    // masses (constants.js "Cosmic-Lattice Anchors" [IMPOSED]). This readout
    // multiplied until 2026-10-10 and showed that star as 3500 solar masses.
    const solarMass = body.mass / LATTICE_MASS_PER_SOLAR_MASS;
    let massStr = '';
    if (solarMass >= 1e6) massStr = `${solarMass.toExponential(2)} M\u2609`;
    else if (solarMass < 1) massStr = `${solarMass.toFixed(4)} M\u2609`;
    else massStr = `${solarMass.toFixed(1)} M\u2609`;

    if (target.cosmicFields.mass) target.cosmicFields.mass.textContent = massStr;
    // Radius, speed and age are the bridge's raw values (mock-scale5.js
    // cosmicInspectBody): a lattice length, a lattice speed and a count of
    // stellar-evolution ticks. No conversion is applied and the cosmic lattice
    // has no documented SI calibration (constants.js "Cosmic-Lattice
    // Anchors"), so they carry the Scale 5 diagnostics units. The earlier
    // solar-radius, km/s and (age * 0.1) Myr labels claimed units the readout
    // does not compute, as audit P0-10 found for the Scale 2 kelvin label.
    if (target.cosmicFields.radius) target.cosmicFields.radius.textContent = `${body.radius.toFixed(2)} lu`;
    if (target.cosmicFields.age) target.cosmicFields.age.textContent = body.age > 0 ? `${Math.round(body.age).toLocaleString()} ticks` : '--';
    // Temperature: for stars and their remnants the value is the kelvin-scale
    // number the stellar rules set directly (cosmic-postupdates.js: 5800 at
    // one solar mass, 3500, 15000, 12000, 1e6), so K is that rule's own input
    // and no conversion is involved. Gas and nebula temperature is 1000 times
    // the internal energy in engine units (addBody, gas cooling, the SPH
    // energy equation); nothing declares that factor as a kelvin calibration,
    // so it is labelled in engine units.
    const tempUnit = STELLAR_TYPES.has(body.type) ? 'K' : '(sim)';
    if (target.cosmicFields.temp) target.cosmicFields.temp.textContent = body.temperature > 0 ? `${Math.round(body.temperature).toLocaleString()} ${tempUnit}` : '--';
    // Luminosity is the bridge's raw field: mass^3.5 in lattice mass units
    // for stars, the jet-intensity gauge for black holes. No conversion to
    // solar luminosities exists.
    if (target.cosmicFields.lum) target.cosmicFields.lum.textContent = body.luminosity > 0 ? `${body.luminosity.toExponential(2)} (sim)` : '--';
    if (target.cosmicFields.pos) target.cosmicFields.pos.textContent = `(${body.x.toFixed(1)}, ${body.y.toFixed(1)}, ${body.z.toFixed(1)})`;
    if (target.cosmicFields.vel) target.cosmicFields.vel.textContent = `(${body.vx.toFixed(2)}, ${body.vy.toFixed(2)}, ${body.vz.toFixed(2)})`;
    if (target.cosmicFields.speed) target.cosmicFields.speed.textContent = `${body.speed.toFixed(2)} (sim)`;
    if (target.cosmicFields.fuelFrac) target.cosmicFields.fuelFrac.textContent = `${(body.fuel_fraction * 100).toFixed(1)}%`;

    const phaseNames = ['Protostar', 'Red Giant', 'Core He Burn', 'AGB', 'Pre-SN', 'Core Collapse'];
    if (target.cosmicFields.fuelStage) {
        target.cosmicFields.fuelStage.textContent = body.type === 2
            ? (phaseNames[body.fuel_stage] || 'Main Sequence')
            : '--';
    }
}

import { setInspectorSectionVisibility } from '../chrome.js';

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

    let massStr = '';
    if (body.mass >= 1e6) massStr = `${(body.mass / 1e6).toFixed(2)} M\u2609`;
    else if (body.mass < 1) massStr = `${body.mass.toFixed(4)} M\u2609`;
    else massStr = `${body.mass.toFixed(1)} M\u2609`;

    if (target.cosmicFields.mass) target.cosmicFields.mass.textContent = massStr;
    if (target.cosmicFields.radius) target.cosmicFields.radius.textContent = `${body.radius.toFixed(2)} R\u2609`;
    if (target.cosmicFields.age) target.cosmicFields.age.textContent = body.age > 0 ? `${(body.age * 0.1).toFixed(1)} Myrs` : '--';
    if (target.cosmicFields.temp) target.cosmicFields.temp.textContent = body.temperature > 0 ? `${Math.round(body.temperature).toLocaleString()} K` : '--';
    if (target.cosmicFields.lum) target.cosmicFields.lum.textContent = body.luminosity > 0 ? `${body.luminosity.toExponential(2)} L\u2609` : '--';
    if (target.cosmicFields.pos) target.cosmicFields.pos.textContent = `(${body.x.toFixed(1)}, ${body.y.toFixed(1)}, ${body.z.toFixed(1)})`;
    if (target.cosmicFields.vel) target.cosmicFields.vel.textContent = `(${body.vx.toFixed(2)}, ${body.vy.toFixed(2)}, ${body.vz.toFixed(2)})`;
    if (target.cosmicFields.speed) target.cosmicFields.speed.textContent = `${body.speed.toFixed(2)} km/s`;
    if (target.cosmicFields.fuelFrac) target.cosmicFields.fuelFrac.textContent = `${(body.fuel_fraction * 100).toFixed(1)}%`;

    const phaseNames = ['Protostar', 'Red Giant', 'Core He Burn', 'AGB', 'Pre-SN', 'Core Collapse'];
    if (target.cosmicFields.fuelStage) {
        target.cosmicFields.fuelStage.textContent = body.type === 2
            ? (phaseNames[body.fuel_stage] || 'Main Sequence')
            : '--';
    }
}

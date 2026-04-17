import { setInspectorSectionVisibility } from '../chrome.js';
import { getById, chargeLabel, formatMass } from '../../particle-catalog.js';
import {
    formatPosition,
    formatVec3,
    formatVelocity,
    formatForce,
    formatEnergy,
    formatLength,
} from '../../units.js';

export function handlePEClick(target, intersects) {
    if (intersects.length > 0 && target._cloudParticleMap) {
        const cloudIdx = intersects[0].index;
        if (cloudIdx < target._cloudCount) {
            target._selectedPEParticleId = target._cloudParticleMap[cloudIdx];
            showPEInspector(target);
            return;
        }
    }
    target._selectedPEParticleId = -1;
    hidePEInspector(target);
}

export function showPEInspector(target) {
    setInspectorSectionVisibility(target.peEmptyEl, target.peContentEl, true);
    updatePEFields(target);
    target._updateInspectorChrome();
}

export function hidePEInspector(target) {
    setInspectorSectionVisibility(target.peEmptyEl, target.peContentEl, false);
    target._updateInspectorChrome();
}

export function updatePEFields(target) {
    if (target._selectedPEParticleId < 0) return;

    const data = target.bridge.peInspectParticle(target._selectedPEParticleId);
    if (!data) {
        target._selectedPEParticleId = -1;
        hidePEInspector(target);
        return;
    }

    const catId = target._peTypeMap ? target._peTypeMap.get(data.id) : null;
    const cat = catId ? getById(catId) : null;

    if (cat) {
        const [r, g, b] = cat.display_color;
        target.peFields.dot.style.background = `rgb(${Math.round(r * 255)},${Math.round(g * 255)},${Math.round(b * 255)})`;
        target.peFields.name.textContent = cat.name;
        target.peFields.symbol.textContent = cat.symbol;
        target.peFields.catalog.textContent = catId;
        target.peFields.mass.textContent = formatMass(cat.mass_mev);
        target.peFields.charge.textContent = chargeLabel(cat.charge);
    } else {
        target.peFields.dot.style.background = '#9ca3af';
        target.peFields.name.textContent = 'Unknown';
        target.peFields.symbol.textContent = '?';
        target.peFields.catalog.textContent = catId || '--';
        target.peFields.mass.textContent = `${data.mass.toFixed(3)} MeV`;
        target.peFields.charge.textContent = data.charge > 0 ? `+${data.charge}` : data.charge.toString();
    }

    target.peFields.id.textContent = data.id;
    target.peFields.locked.textContent = data.locked ? 'Yes (fixed)' : 'No';
    target.peFields.pos.textContent = formatPosition(data.x, data.y, data.z, 1);
    target.peFields.vel.textContent = formatVec3(data.vx, data.vy, data.vz, 'velocity', 1);
    target.peFields.speed.textContent = formatVelocity(data.speed, 1).text;
    target.peFields.ke.textContent = formatEnergy(data.ke, 1).text;
    target.peFields.orbital.textContent = data.orbitalR >= 0 ? formatLength(data.orbitalR, 1).text : '--';

    if (data.nearestId >= 0) {
        const nearCatId = target._peTypeMap ? target._peTypeMap.get(data.nearestId) : null;
        const nearCat = nearCatId ? getById(nearCatId) : null;
        target.peFields.nearest.textContent = nearCat ? nearCat.name : `#${data.nearestId}`;
        target.peFields.dist.textContent = formatLength(data.nearestDist, 1).text;
        target.peFields.fc.textContent = formatForce(data.fCoulombNearest, 1).text;
    } else {
        target.peFields.nearest.textContent = '--';
        target.peFields.dist.textContent = '--';
        target.peFields.fc.textContent = '--';
    }
    target.peFields.fnet.textContent = formatForce(data.fNetMag, 1).text;
}

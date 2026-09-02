import { setInspectorSectionVisibility } from '../chrome.js?v=2';
import { getById, chargeLabel, formatMass } from '../../particle-catalog.js';
import {
    formatPosition,
    formatVec3,
    formatVelocity,
    formatForce,
    formatEnergy,
    formatLength,
} from '../../units.js';

export function handlePEClick(target, intersects, pointerEvent = null) {
    if (intersects.length > 0 && target._cloudParticleMap) {
        for (const hit of intersects) {
            const cloudIdx = hit.index;
            if (cloudIdx < target._cloudCount
                && target.selectPEParticle?.(target._cloudParticleMap[cloudIdx])) return;
        }
    }
    const logicalId = target.pickPEParticleAtClientPoint?.(
        pointerEvent?.clientX,
        pointerEvent?.clientY,
        pointerEvent?.pointerType,
    );
    if (logicalId >= 0 && target.selectPEParticle?.(logicalId)) return;
    target.clearPEInspection?.();
}

function fmtScalar(value, digits = 3) {
    if (!Number.isFinite(Number(value))) return '--';
    const number = Number(value);
    const magnitude = Math.abs(number);
    if ((magnitude > 0 && magnitude < 1e-3) || magnitude >= 1e5) {
        return number.toExponential(2);
    }
    return number.toFixed(digits).replace(/(\.\d*?[1-9])0+$|\.0+$/, '$1');
}

function updatePEFocusPresentation(target) {
    const focus = target._peInspectionFocus;
    if (!focus) return;
    const cluster = focus.kind === 'cluster';
    if (target.peClusterFieldsEl) target.peClusterFieldsEl.hidden = !cluster;
    if (target.peFields.identityTitle) {
        target.peFields.identityTitle.textContent = cluster ? 'Energy anchor identity' : 'Identity';
    }
    if (target.peFields.focusKind) {
        target.peFields.focusKind.textContent = cluster ? 'Dynamic cluster focus' : 'Particle focus';
    }
    if (target.peFields.focusScope) {
        target.peFields.focusScope.textContent = cluster
            ? `${focus.clusterId} · ${focus.particleIds.length} particle${focus.particleIds.length === 1 ? '' : 's'}`
            : `#${focus.particleId}`;
    }
    if (target.peFields.focusDescription) {
        target.peFields.focusDescription.textContent = cluster
            ? 'Per-particle overlays and field sources are restricted to this live cluster. The identity cards below follow its current energy anchor.'
            : 'Per-particle overlays and field sources are restricted to this record. Physics and overlay toggle settings are unchanged.';
    }
    if (!cluster) return;
    target.peFields.clusterId.textContent = focus.clusterId;
    target.peFields.clusterMembers.textContent = focus.particleIds.map(id => `#${id}`).join(', ');
    target.peFields.clusterAnchor.textContent = `#${focus.anchorId}`;
    target.peFields.clusterCenter.textContent = `(${fmtScalar(focus.center.x)}, ${fmtScalar(focus.center.y)}, ${fmtScalar(focus.center.z)}) lu`;
    target.peFields.clusterEnergy.textContent = `${fmtScalar(focus.energy)} · ${focus.energyBasis === 'mass_fallback' ? 'dormant mass fallback' : 'dynamic activity'}`;
}

export function showPEInspector(target) {
    setInspectorSectionVisibility(target.peEmptyEl, target.peContentEl, true);
    updatePEFocusPresentation(target);
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
        target.clearPEInspection?.();
        return;
    }

    updatePEFocusPresentation(target);

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
    target.peFields.rEff.textContent = formatLength(data.rEff, 2).text || '--';
    target.peFields.spin.textContent = data.spin !== undefined ? data.spin.toString() : '--';
    target.peFields.color.textContent = data.colorId !== undefined ? data.colorId.toString() : '--';
    target.peFields.pair.textContent = data.pairId >= 0 ? data.pairId.toString() : '--';
    target.peFields.pos.textContent = formatPosition(data.x, data.y, data.z, 1);
    target.peFields.vel.textContent = formatVec3(data.vx, data.vy, data.vz, 'velocity', 1);
    target.peFields.speed.textContent = formatVelocity(data.speed, 1).text;
    target.peFields.ke.textContent = formatEnergy(data.ke, 1).text;
    target.peFields.momentum.textContent = isNaN(data.momentum) ? '--' : data.momentum.toExponential(2);
    target.peFields.accel.textContent = isNaN(data.acceleration) ? '--' : data.acceleration.toExponential(2);
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

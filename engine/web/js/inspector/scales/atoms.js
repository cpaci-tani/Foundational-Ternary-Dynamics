import { setInspectorSectionVisibility } from '../chrome.js';
import { getElement, elementSymbol, cpkColor } from '../../elements.js';
import { getMolecule } from '../../molecules.js';
import {
    formatPosition,
    formatVec3,
    formatVelocity,
    formatForce,
    formatEnergy,
    formatLength,
} from '../../units.js';

export function handleAEClick(target, intersects) {
    if (intersects.length > 0) {
        const hitIdx = intersects[0].index;
        let atomArrayIdx = -1;

        if (target._aeCloudMode && target._aeCloudAtomMap) {
            atomArrayIdx = target._aeCloudAtomMap[hitIdx];
        } else {
            atomArrayIdx = hitIdx;
        }

        if (atomArrayIdx >= 0 && target._aeAtomIds && atomArrayIdx < target._aePointCount) {
            target._selectedAEAtomId = target._aeAtomIds[atomArrayIdx];
            showAEInspector(target);
            return;
        }
    }
    target._selectedAEAtomId = -1;
    hideAEInspector(target);
}

export function showAEInspector(target) {
    setInspectorSectionVisibility(target.aeEmptyEl, target.aeContentEl, true);
    updateAEFields(target);
    target._updateInspectorChrome();
}

export function hideAEInspector(target) {
    setInspectorSectionVisibility(target.aeEmptyEl, target.aeContentEl, false);
    target._updateInspectorChrome();
}

export function updateAEFields(target) {
    if (target._selectedAEAtomId < 0) return;

    const data = target.bridge.aeInspectAtom(target._selectedAEAtomId);
    if (!data) {
        target._selectedAEAtomId = -1;
        hideAEInspector(target);
        return;
    }

    const element = getElement(data.Z);
    const symbol = elementSymbol(data.Z);
    const color = cpkColor(data.Z);

    if (target.aeFields.dot) {
        target.aeFields.dot.style.background = `rgb(${Math.round(color[0] * 255)},${Math.round(color[1] * 255)},${Math.round(color[2] * 255)})`;
    }
    if (target.aeFields.name) target.aeFields.name.textContent = element ? element.name : `Z=${data.Z}`;
    if (target.aeFields.symbol) target.aeFields.symbol.textContent = symbol;
    if (target.aeFields.id) target.aeFields.id.textContent = data.id;
    if (target.aeFields.z) target.aeFields.z.textContent = data.Z;
    if (target.aeFields.charge) {
        target.aeFields.charge.textContent = data.charge === 0 ? '0' : (data.charge > 0 ? `+${data.charge}` : data.charge.toString());
    }
    if (target.aeFields.mass) target.aeFields.mass.textContent = `${data.mass.toFixed(3)} AMU`;
    if (target.aeFields.locked) target.aeFields.locked.textContent = data.locked ? 'Yes' : 'No';
    if (target.aeFields.n) target.aeFields.n.textContent = data.N;
    if (target.aeFields.a) target.aeFields.a.textContent = data.Z + data.N;
    if (target.aeFields.maxBonds) target.aeFields.maxBonds.textContent = data.maxBonds;

    if (target.aeFields.pos) target.aeFields.pos.textContent = formatPosition(data.x, data.y, data.z, 2);
    if (target.aeFields.vel) target.aeFields.vel.textContent = formatVec3(data.vx, data.vy, data.vz, 'velocity', 2);
    if (target.aeFields.speed) target.aeFields.speed.textContent = formatVelocity(data.speed, 2).text;
    if (target.aeFields.ke) target.aeFields.ke.textContent = formatEnergy(data.ke, 2).text;
    if (target.aeFields.fnet) target.aeFields.fnet.textContent = formatForce(data.fNetMag, 2).text;

    if (target.aeFields.bonds) {
        buildAEBondsList(target, data.bonds);
    }

    if (target.aeFields.nearest) {
        if (data.nearestId >= 0) {
            const nearElement = getElement(data.nearestZ);
            const nearSymbol = elementSymbol(data.nearestZ);
            target.aeFields.nearest.textContent = nearElement ? `${nearSymbol} (#${data.nearestId})` : `#${data.nearestId}`;
        } else {
            target.aeFields.nearest.textContent = '--';
        }
    }
    if (target.aeFields.nearestDist) {
        target.aeFields.nearestDist.textContent = data.nearestId >= 0 ? formatLength(data.nearestDist, 2).text : '--';
    }

    if (target.aeFields.sigma) target.aeFields.sigma.textContent = formatLength(data.sigma, 2).text;
    if (target.aeFields.epsilon) target.aeFields.epsilon.textContent = formatEnergy(data.epsilon, 2).text;
}

export function buildAEBondsList(target, bonds) {
    const container = target.aeFields.bonds;
    if (!container) return;
    container.innerHTML = '';

    if (!bonds || bonds.length === 0) {
        const dt = document.createElement('dt');
        dt.textContent = 'Bonds';
        const dd = document.createElement('dd');
        dd.textContent = 'None';
        container.appendChild(dt);
        container.appendChild(dd);
        return;
    }

    for (let i = 0; i < bonds.length; i++) {
        const bond = bonds[i];
        const partnerSymbol = elementSymbol(bond.partnerZ);
        const orderSym = bond.order === 1 ? '\u2014' : bond.order === 2 ? '\u2550' : bond.order === 3 ? '\u2261' : `\u00d7${bond.order}`;

        const dt = document.createElement('dt');
        dt.textContent = `Bond ${i + 1}`;
        const dd = document.createElement('dd');
        dd.textContent = `${partnerSymbol} #${bond.partnerId} ${orderSym}`;
        container.appendChild(dt);
        container.appendChild(dd);

        const dtDist = document.createElement('dt');
        dtDist.textContent = '';
        const ddDist = document.createElement('dd');
        const dStr = formatLength(bond.dist, 2).text;
        const rStr = formatLength(bond.r_eq, 2).text;
        ddDist.textContent = `d=${dStr}, r\u2080=${rStr}`;
        container.appendChild(dtDist);
        container.appendChild(ddDist);
    }
}

export function updateAEMoleculeInfo(target, molId) {
    if (!target.aeMolInfoEl) return;

    if (!molId) {
        target.aeMolInfoEl.style.display = 'none';
        return;
    }

    const molecule = getMolecule(molId);
    if (!molecule) {
        target.aeMolInfoEl.style.display = 'none';
        return;
    }

    target.aeMolInfoEl.style.display = 'block';

    if (target.aeMolFields.title) target.aeMolFields.title.textContent = molecule.name;
    if (target.aeMolFields.desc) target.aeMolFields.desc.textContent = molecule.description || '--';
    if (target.aeMolFields.formula) target.aeMolFields.formula.innerHTML = molecule.formula || '--';
    if (target.aeMolFields.category) target.aeMolFields.category.textContent = molecule.category || '--';

    const atoms = molecule.atoms || [];
    if (target.aeMolFields.atomCount) target.aeMolFields.atomCount.textContent = atoms.length;

    if (target.aeMolFields.composition) {
        const counts = {};
        for (const atom of atoms) {
            const sym = elementSymbol(atom.Z);
            counts[sym] = (counts[sym] || 0) + 1;
        }
        const composition = Object.entries(counts)
            .sort((a, b) => {
                if (a[0] === 'C') return -1;
                if (b[0] === 'C') return 1;
                if (a[0] === 'H') return -1;
                if (b[0] === 'H') return 1;
                return a[0].localeCompare(b[0]);
            })
            .map(([sym, n]) => `${n}${sym}`)
            .join(' + ');
        target.aeMolFields.composition.textContent = composition || '--';
    }

    if (target.aeMolFields.bondCount) {
        target.aeMolFields.bondCount.textContent = '--';
    }

    if (target.aeMolFields.mass) {
        let totalMass = 0;
        for (const atom of atoms) {
            const element = getElement(atom.Z);
            const neutrons = element ? element.neutrons : 0;
            totalMass += atom.Z + neutrons * 1.001;
        }
        target.aeMolFields.mass.textContent = `${totalMass.toFixed(2)} AMU`;
    }
}

export function setAEScenarioInfo(target, info) {
    if (!target.aeScenarioInfoEl) return;
    if (!info) {
        target.aeScenarioInfoEl.style.display = 'none';
        return;
    }
    target.aeScenarioInfoEl.style.display = 'block';
    if (target.aeScenarioTitle) target.aeScenarioTitle.textContent = info.title || '--';
    if (target.aeScenarioDesc) target.aeScenarioDesc.textContent = info.desc || '';
    if (target.aeScenarioFields) {
        target.aeScenarioFields.innerHTML = '';
        for (const [label, value] of Object.entries(info.fields || {})) {
            const dt = document.createElement('dt');
            dt.textContent = label;
            const dd = document.createElement('dd');
            dd.textContent = value;
            target.aeScenarioFields.appendChild(dt);
            target.aeScenarioFields.appendChild(dd);
        }
    }
}

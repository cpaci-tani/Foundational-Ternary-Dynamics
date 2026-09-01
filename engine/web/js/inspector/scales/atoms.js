import { setInspectorSectionVisibility } from '../chrome.js';
import { getElement, elementSymbol, cpkColor } from '../../elements.js';
import { getMolecule } from '../../molecules.js';
import { NEUTRON_PROTON_MASS_RATIO } from '../../constants.js';
import { Sparkline } from '../../ui/charts/sparkline.js';
import { RingBuffer, telemetryHub } from '../../telemetry-hub.js';
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
            const newId = target._aeAtomIds[atomArrayIdx];
            if (newId !== target._selectedAEAtomId) {
                target._selectedAEAtomId = newId;
                if (target._aeTelemetry) {
                    for (const b of Object.values(target._aeTelemetry.buffers)) b.clear();
                    target._aeTelemetry.lastTick = null;
                }
            }
            showAEInspector(target);
            return;
        }
    }
    target._selectedAEAtomId = -1;
    if (target._aeTelemetry) {
        for (const b of Object.values(target._aeTelemetry.buffers)) b.clear();
        target._aeTelemetry.lastTick = null;
    }
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

    // Telemetry and Sparklines
    if (!target._aeTelemetry && target.aeFields.alphaPolSpark) {
        target._aeTelemetry = {
            buffers: {
                alpha_pol: new RingBuffer(80),
                e_ion: new RingBuffer(80),
                e_aff: new RingBuffer(80),
                sigma_scatter: new RingBuffer(80),
                zeff: new RingBuffer(80),
                q_frac: new RingBuffer(80)
            },
            sparks: {},
            lastTick: null,
        };
        const colorAlpha = '#a855f7';
        const colorIon = '#ef4444';
        const colorAff = '#3b82f6';
        const colorSigma = '#f59e0b';
        const colorZeff = '#10b981';
        const colorQFrac = '#8b5cf6';
        
        const historyControl = target.chartHistoryControl || null;
        target._aeTelemetry.sparks.alpha_pol = new Sparkline(target.aeFields.alphaPolSpark, { buffer: target._aeTelemetry.buffers.alpha_pol, color: colorAlpha, historyControl });
        target._aeTelemetry.sparks.e_ion = new Sparkline(target.aeFields.eIonSpark, { buffer: target._aeTelemetry.buffers.e_ion, color: colorIon, historyControl });
        target._aeTelemetry.sparks.e_aff = new Sparkline(target.aeFields.eAffSpark, { buffer: target._aeTelemetry.buffers.e_aff, color: colorAff, historyControl });
        target._aeTelemetry.sparks.sigma_scatter = new Sparkline(target.aeFields.sigmaScatterSpark, { buffer: target._aeTelemetry.buffers.sigma_scatter, color: colorSigma, historyControl });
        target._aeTelemetry.sparks.zeff = new Sparkline(target.aeFields.zeffSpark, { buffer: target._aeTelemetry.buffers.zeff, color: colorZeff, historyControl });
        target._aeTelemetry.sparks.q_frac = new Sparkline(target.aeFields.qFracSpark, { buffer: target._aeTelemetry.buffers.q_frac, color: colorQFrac, historyControl });
    }

    if (target._aeTelemetry) {
        const tick = Number(telemetryHub.s2?.diag?.tick);
        if (Number.isFinite(tick) && tick !== target._aeTelemetry.lastTick) {
            target._aeTelemetry.lastTick = tick;
            target._aeTelemetry.buffers.alpha_pol.push(data.alpha_pol || 0, tick);
            target._aeTelemetry.buffers.e_ion.push(data.e_ion || 0, tick);
            target._aeTelemetry.buffers.e_aff.push(data.e_aff || 0, tick);
            target._aeTelemetry.buffers.sigma_scatter.push(data.sigma_scatter || 0, tick);
            target._aeTelemetry.buffers.zeff.push(data.z_eff || 0, tick);
            target._aeTelemetry.buffers.q_frac.push(data.charge || 0, tick);
        }

        for (const spark of Object.values(target._aeTelemetry.sparks)) spark.update();

        if (target.aeFields.alphaPol) target.aeFields.alphaPol.textContent = (data.alpha_pol || 0).toFixed(4);
        if (target.aeFields.eIon) target.aeFields.eIon.textContent = (data.e_ion || 0).toFixed(4) + ' Ry';
        if (target.aeFields.eAff) target.aeFields.eAff.textContent = (data.e_aff || 0).toFixed(4) + ' \u03c7';
        if (target.aeFields.sigmaScatter) target.aeFields.sigmaScatter.textContent = (data.sigma_scatter || 0).toFixed(4);
        if (target.aeFields.zeff) target.aeFields.zeff.textContent = (data.z_eff || 0).toFixed(4);
        if (target.aeFields.qFrac) target.aeFields.qFrac.textContent = (data.charge || 0).toFixed(4) + ' e';

        const updateStats = (buf, minEl, avgEl, maxEl) => {
            if (!buf || !minEl || !avgEl || !maxEl) return;
            if (buf.count === 0) {
                minEl.textContent = '--'; avgEl.textContent = '--'; maxEl.textContent = '--';
            } else {
                minEl.textContent = buf.min().toFixed(4);
                maxEl.textContent = buf.max().toFixed(4);
                const avg = buf.total > 0 ? Array.from(buf.data.subarray(0, buf.count)).reduce((a,b)=>a+b,0)/buf.count : 0;
                avgEl.textContent = avg.toFixed(4);
            }
        };

        updateStats(target._aeTelemetry.buffers.alpha_pol, target.aeFields.alphaPolMin, target.aeFields.alphaPolAvg, target.aeFields.alphaPolMax);
        updateStats(target._aeTelemetry.buffers.e_ion, target.aeFields.eIonMin, target.aeFields.eIonAvg, target.aeFields.eIonMax);
        updateStats(target._aeTelemetry.buffers.e_aff, target.aeFields.eAffMin, target.aeFields.eAffAvg, target.aeFields.eAffMax);
        updateStats(target._aeTelemetry.buffers.sigma_scatter, target.aeFields.sigmaScatterMin, target.aeFields.sigmaScatterAvg, target.aeFields.sigmaScatterMax);
        updateStats(target._aeTelemetry.buffers.zeff, target.aeFields.zeffMin, target.aeFields.zeffAvg, target.aeFields.zeffMax);
        updateStats(target._aeTelemetry.buffers.q_frac, target.aeFields.qFracMin, target.aeFields.qFracAvg, target.aeFields.qFracMax);
    }
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
            totalMass += atom.Z + neutrons * NEUTRON_PROTON_MASS_RATIO;
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

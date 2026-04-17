import { setInspectorSectionVisibility } from '../chrome.js';
import {
    formatPosition,
    formatVec3,
    formatVelocity,
    formatForce,
    formatDensity,
    formatDivergence,
    formatField_E,
    formatField_B,
} from '../../units.js';

const COLOR_LABELS = { 0: 'colorless', 1: 'red', 2: 'green', 3: 'blue' };
const COLOR_CSS = { 0: '#9ca3af', 1: '#ef5350', 2: '#4ade80', 3: '#60a5fa' };

export function handleLatticeClick(target, intersects) {
    if (intersects.length > 0) {
        let hit = intersects.find((entry) => entry.object !== target.viewport._voidBox);
        if (!hit) {
            target.selectedIndex = -1;
            target._selectedPos = null;
            hideLatticeInspector(target);
            return;
        }
        if (hit.object === target.viewport._voidBox) {
            target.selectedIndex = -1;
            target._selectedPos = {
                x: Math.round(hit.point.x),
                y: Math.round(hit.point.y),
                z: Math.round(hit.point.z),
            };
        } else {
            target.selectedIndex = hit.index;
            const posArr = hit.object.geometry.getAttribute('position').array;
            target._selectedPos = {
                x: Math.round(posArr[target.selectedIndex * 3]),
                y: Math.round(posArr[target.selectedIndex * 3 + 1]),
                z: Math.round(posArr[target.selectedIndex * 3 + 2]),
            };
        }
        const L = target.bridge.latticeSize || 32;
        target._selectedPos.x = Math.max(0, Math.min(L - 1, target._selectedPos.x));
        target._selectedPos.y = Math.max(0, Math.min(L - 1, target._selectedPos.y));
        target._selectedPos.z = Math.max(0, Math.min(L - 1, target._selectedPos.z));
        showLatticeInspector(target);
        return;
    }

    target.selectedIndex = -1;
    target._selectedPos = null;
    hideLatticeInspector(target);
}

export function showLatticeInspector(target) {
    setInspectorSectionVisibility(target.emptyEl, target.contentEl, true);

    if (target.viewport && target.viewport.setVoxelHighlight && target._selectedPos) {
        target.viewport.setVoxelHighlight(target._selectedPos.x, target._selectedPos.y, target._selectedPos.z, true);
        const u1 = document.getElementById('sym-u1')?.checked || false;
        const su2 = document.getElementById('sym-su2')?.checked || false;
        const su3 = document.getElementById('sym-su3')?.checked || false;
        target.viewport.setSymmetryHighlights(target._selectedPos.x, target._selectedPos.y, target._selectedPos.z, u1, su2, su3);
    }

    const symPanel = document.getElementById('floating-symmetry-panel');
    if (symPanel) symPanel.style.display = 'block';
    updateLatticeFields(target);
    target._updateInspectorChrome();
}

export function hideLatticeInspector(target) {
    setInspectorSectionVisibility(target.emptyEl, target.contentEl, false);
    if (target.viewport && target.viewport.setVoxelHighlight) {
        target.viewport.setVoxelHighlight(0, 0, 0, false);
        target.viewport.setSymmetryHighlights(0, 0, 0, false, false, false);
    }
    const symPanel = document.getElementById('floating-symmetry-panel');
    if (symPanel) symPanel.style.display = 'none';
    target._updateInspectorChrome();
}

export function updateLatticeFields(target) {
    if (!target._selectedPos) return;
    const { x, y, z } = target._selectedPos;

    let voxel = null;
    let force = null;
    if (typeof target.bridge.inspectVoxel === 'function') voxel = target.bridge.inspectVoxel(x, y, z);
    if (typeof target.bridge.getForceAt === 'function') force = target.bridge.getForceAt(x, y, z);

    if (!voxel && typeof target.bridge.inspectVoxel !== 'function') {
        voxel = {
            state: 0,
            particleId: -1,
            spin: 0,
            color: 0,
            pairId: -1,
            locked: false,
            fluxX: 0,
            fluxY: 0,
            fluxZ: 0,
            density: 0,
            divJ: 0,
            curlX: 0,
            curlY: 0,
            curlZ: 0,
            velX: 0,
            velY: 0,
            velZ: 0,
            speed: 0,
            accelMag: 0,
        };
    }

    if (voxel) {
        const stateLabel = voxel.state === 1 ? '+1 (positive)' : voxel.state === -1 ? '-1 (negative)' : '0 (void)';
        const stateColor = voxel.state === 1 ? '#4ade80' : voxel.state === -1 ? '#f87171' : '#9ca3af';
        target.fields.id.textContent = voxel.particleId >= 0 ? voxel.particleId : '--';
        target.fields.state.innerHTML = `<span style="color:${stateColor}">${stateLabel}</span>`;
        if (target.fields.pos) target.fields.pos.textContent = formatPosition(x, y, z, 0);
        target.fields.spin.textContent = voxel.spin === 1 ? '+1/2 (up)' : voxel.spin === -1 ? '-1/2 (down)' : '--';
        const cLabel = COLOR_LABELS[voxel.color] || '--';
        const cColor = COLOR_CSS[voxel.color] || '#9ca3af';
        target.fields.color.innerHTML = `<span style="color:${cColor}">${cLabel}</span>`;
        target.fields.pair.textContent = voxel.pairId >= 0 ? voxel.pairId : '--';
        target.fields.locked.textContent = voxel.locked ? 'Yes' : 'No';

        target.fields.flux.textContent = formatVec3(voxel.fluxX, voxel.fluxY, voxel.fluxZ, 'flux', 0);
        target.fields.density.textContent = formatDensity(voxel.density, 0).text;
        target.fields.divj.textContent = formatDivergence(voxel.divJ, 0).text;
        target.fields.curl.textContent = formatVec3(voxel.curlX, voxel.curlY, voxel.curlZ, 'curl', 0);
        target.fields.vel.textContent = formatVec3(voxel.velX, voxel.velY, voxel.velZ, 'velocity', 0);
        target.fields.speed.textContent = formatVelocity(voxel.speed, 0).text;
        target.fields.accel.textContent = formatForce(voxel.accelMag, 0).text;
        target.fields.eMag.textContent = voxel.Emag !== undefined ? formatField_E(voxel.Emag, 0).text : '--';
        target.fields.bMag.textContent = voxel.Bmag !== undefined ? formatField_B(voxel.Bmag, 0).text : '--';
    } else {
        target.fields.id.textContent = '--';
        target.fields.state.textContent = '--';
        if (target.fields.pos) target.fields.pos.textContent = formatPosition(x, y, z, 0);
        target.fields.spin.textContent = '--';
        target.fields.color.textContent = '--';
        target.fields.pair.textContent = '--';
        target.fields.locked.textContent = '--';
        target.fields.flux.textContent = '--';
        target.fields.density.textContent = '--';
        target.fields.divj.textContent = '--';
        target.fields.curl.textContent = '--';
        target.fields.vel.textContent = '--';
        target.fields.speed.textContent = '--';
        target.fields.accel.textContent = '--';
        target.fields.eMag.textContent = '--';
        target.fields.bMag.textContent = '--';
    }

    if (force) {
        target.fields.fCoulomb.textContent = formatForce(force.coulombMag, 0).text;
        target.fields.fGravity.textContent = formatForce(force.gravityMag, 0).text;
        target.fields.fMagnetic.textContent = formatForce(force.magneticMag, 0).text;
        target.fields.fStrong.textContent = formatForce(force.strongMag, 0).text;
        target.fields.fExchange.textContent = formatForce(force.exchangeMag, 0).text;
    } else {
        for (const key of ['fCoulomb', 'fGravity', 'fMagnetic', 'fStrong', 'fExchange']) {
            target.fields[key].textContent = '--';
        }
    }

    const mooreGrid = document.getElementById('insp-moore-grid');
    if (mooreGrid && typeof target.bridge.inspectVoxel === 'function') {
        const L = target.bridge.latticeSize || 64;
        let html = '';
        for (let dz = -1; dz <= 1; dz++) {
            html += '<div style="display:inline-block; margin: 0 8px;">';
            html += `<div style="color:var(--text-muted);font-size:10px;margin-bottom:6px">Z${dz === 0 ? '' : (dz > 0 ? `+${dz}` : dz)}</div>`;
            for (let dy = 1; dy >= -1; dy--) {
                html += '<div style="display:flex;gap:4px;margin-bottom:4px">';
                for (let dx = -1; dx <= 1; dx++) {
                    const nX = (x + dx + L) % L;
                    const nY = (y + dy + L) % L;
                    const nZ = (z + dz + L) % L;
                    const nV = target.bridge.inspectVoxel(nX, nY, nZ);
                    let symbol = '·';
                    let color = '#475569';
                    let bg = '#0f172a';
                    let borderStyle = 'border:1px solid #334155;';

                    if (nV && nV.state === 1) {
                        symbol = '+';
                        color = '#4ade80';
                        bg = 'rgba(74, 222, 128, 0.15)';
                    } else if (nV && nV.state === -1) {
                        symbol = '-';
                        color = '#f87171';
                        bg = 'rgba(248, 113, 113, 0.15)';
                    } else if (nV && nV.state === 0) {
                        const fx = nV.fluxX || 0;
                        const fy = nV.fluxY || 0;
                        const fz = nV.fluxZ || 0;
                        const fluxMag = Math.sqrt(fx * fx + fy * fy + fz * fz);
                        if (fluxMag > 0.001) {
                            const intensity = Math.min(1.0, fluxMag * 2.0);
                            bg = `rgba(56, 189, 248, ${intensity * 0.4})`;
                            color = `rgba(125, 211, 252, ${0.4 + intensity * 0.6})`;
                            if (fluxMag > 0.1) symbol = '~';
                        }
                    }

                    const isCenter = dx === 0 && dy === 0 && dz === 0;
                    if (isCenter) {
                        borderStyle = 'border:1px solid #94a3b8;';
                        if (bg === '#0f172a') bg = '#1e293b';
                    }

                    html += `<div style="width:18px;height:18px;line-height:16px;background:${bg};${borderStyle}border-radius:2px;color:${color};transition:background 0.1s;overflow:hidden">${symbol}</div>`;
                }
                html += '</div>';
            }
            html += '</div>';
        }
        mooreGrid.innerHTML = html;
    }
}

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

// A native point read is an asynchronous command with a device-side query.
// The inspector used to issue its centre read, force read, and all 27 Moore
// cells on every display refresh (roughly 20 Hz). Keep the panel live, but
// make its read budget explicit: one centre + force plus nine neighbours per
// native refresh. Unknown cells are shown as pending rather than invented as
// void, and every displayed resolved cell remains an actual engine read.
const NATIVE_REFRESH_MS = 750;
const LOCAL_REFRESH_MS = 250;
const NATIVE_NEIGHBOUR_READ_BUDGET = 9;
const LOCAL_NEIGHBOUR_READ_BUDGET = 26;

function nowMs() {
    return typeof performance !== 'undefined' ? performance.now() : Date.now();
}

function voxelKey(x, y, z) {
    return `${x},${y},${z}`;
}

function buildNeighbourOrder(x, y, z, L) {
    const order = [];
    for (let dz = -1; dz <= 1; dz++) {
        for (let dy = 1; dy >= -1; dy--) {
            for (let dx = -1; dx <= 1; dx++) {
                if (dx === 0 && dy === 0 && dz === 0) continue;
                order.push({
                    x: (x + dx + L) % L,
                    y: (y + dy + L) % L,
                    z: (z + dz + L) % L,
                });
            }
        }
    }
    return order;
}

function inspectionCache(target, x, y, z) {
    const bridge = target.bridge;
    const L = Math.max(1, Math.trunc(Number(bridge?.latticeSize) || 64));
    const positionKey = `${L}:${voxelKey(x, y, z)}`;
    let cache = target._latticeInspectionCache;
    if (!cache || cache.bridge !== bridge || cache.positionKey !== positionKey) {
        cache = {
            bridge,
            positionKey,
            lastRequestAt: -Infinity,
            lastVisualEpoch: null,
            voxel: null,
            force: null,
            neighbours: new Map(),
            neighbourOrder: buildNeighbourOrder(x, y, z, L),
            cursor: 0,
            revision: 0,
            renderedRevision: -1,
        };
        target._latticeInspectionCache = cache;
    }
    return cache;
}

function refreshInspectionCache(target, x, y, z) {
    const bridge = target.bridge;
    const cache = inspectionCache(target, x, y, z);
    if (typeof bridge?.inspectVoxel !== 'function') return cache;

    const native = !!bridge.isNativeGPU;
    const interval = native ? NATIVE_REFRESH_MS : LOCAL_REFRESH_MS;
    const now = nowMs();
    // WebSocketBridge advances this only when a completed physics frame has
    // made new visual data available. Once all 26 neighbours are resolved,
    // avoid re-requesting a paused lattice simply because the inspector is
    // still being painted.
    const visualEpoch = native && Number.isFinite(Number(bridge._visualEpoch))
        ? Number(bridge._visualEpoch)
        : null;
    const incomplete = !cache.voxel
        || cache.neighbours.size < cache.neighbourOrder.length
        // A native force response can arrive a round later than voxel data.
        // Keep retrying that one bounded read until it is real rather than
        // treating the unresolved value as a physical zero.
        || (typeof bridge.getForceAt === 'function' && !cache.force);
    if (visualEpoch !== null && visualEpoch === cache.lastVisualEpoch && !incomplete) return cache;
    if (now - cache.lastRequestAt < interval) return cache;
    cache.lastRequestAt = now;
    cache.lastVisualEpoch = visualEpoch;
    cache.revision++;

    const voxel = bridge.inspectVoxel(x, y, z);
    if (voxel) cache.voxel = voxel;
    if (typeof bridge.getForceAt === 'function') {
        const force = bridge.getForceAt(x, y, z);
        if (force) cache.force = force;
    }

    const budget = native ? NATIVE_NEIGHBOUR_READ_BUDGET : LOCAL_NEIGHBOUR_READ_BUDGET;
    const order = cache.neighbourOrder;
    for (let i = 0; i < Math.min(budget, order.length); i++) {
        const idx = (cache.cursor + i) % order.length;
        const pos = order[idx];
        const neighbour = bridge.inspectVoxel(pos.x, pos.y, pos.z);
        if (neighbour) cache.neighbours.set(voxelKey(pos.x, pos.y, pos.z), neighbour);
    }
    cache.cursor = (cache.cursor + budget) % order.length;
    return cache;
}

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
            // Voxel k occupies world coords [k, k+1) and is rendered at centre
            // k+0.5. A ray-hit anywhere in [k, k+1) should map to voxel k —
            // Math.floor does that correctly. Math.round snaps half-points
            // toward +∞ (Math.round(16.5) === 17 in JS), so clicking on the
            // rendered centre of voxel 16 was selecting voxel 17.
            target._selectedPos = {
                x: Math.floor(hit.point.x),
                y: Math.floor(hit.point.y),
                z: Math.floor(hit.point.z),
            };
        } else {
            target.selectedIndex = hit.index;
            const posArr = hit.object.geometry.getAttribute('position').array;
            // The geometry stores voxel-centre world coords (k+0.5), so we
            // subtract the 0.5 offset via floor to recover the integer index.
            target._selectedPos = {
                x: Math.floor(posArr[target.selectedIndex * 3]),
                y: Math.floor(posArr[target.selectedIndex * 3 + 1]),
                z: Math.floor(posArr[target.selectedIndex * 3 + 2]),
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
    }
    // Notify the selection card so sel-x/y/z stay in sync with click-to-select
    if (target._selectedPos) {
        document.dispatchEvent(new CustomEvent('ftd:voxel-selected', {
            detail: { ...target._selectedPos },
        }));
    }
    updateLatticeFields(target);
    target._updateInspectorChrome();
}

export function hideLatticeInspector(target) {
    setInspectorSectionVisibility(target.emptyEl, target.contentEl, false);
    if (target.viewport && target.viewport.setVoxelHighlight) {
        target.viewport.setVoxelHighlight(0, 0, 0, false);
        // The Selection card owns the optional area box, but inspector Clear,
        // an empty viewport click, and a scale switch all mean that every
        // selection overlay must disappear together.
        target.viewport.setAreaHighlight?.(0, 0, 0, 1, false);
    }
    target._latticeInspectionCache = null;
    target._updateInspectorChrome();
    document.dispatchEvent(new CustomEvent('ftd:voxel-selection-cleared'));
}

export function updateLatticeFields(target) {
    if (!target._selectedPos) return;
    const { x, y, z } = target._selectedPos;

    const readCache = refreshInspectionCache(target, x, y, z);
    let voxel = readCache.voxel;
    let force = readCache.force;

    // The app asks the inspector to paint more often than the bounded read
    // budget. Rebuilding a 27-cell HTML grid and sixteen text fields with no
    // new engine response only adds layout work; retain the last real snapshot
    // until the next scheduled read revision arrives.
    if (readCache.renderedRevision === readCache.revision) return;

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
            html += `<div style="color:var(--text-muted);font-size:16px;margin-bottom:6px">Z${dz === 0 ? '' : (dz > 0 ? `+${dz}` : dz)}</div>`;
            for (let dy = 1; dy >= -1; dy--) {
                html += '<div style="display:flex;gap:4px;margin-bottom:4px">';
                for (let dx = -1; dx <= 1; dx++) {
                    const nX = (x + dx + L) % L;
                    const nY = (y + dy + L) % L;
                    const nZ = (z + dz + L) % L;
                    const isCenter = dx === 0 && dy === 0 && dz === 0;
                    const neighbourKey = voxelKey(nX, nY, nZ);
                    const nV = isCenter ? voxel : readCache.neighbours.get(neighbourKey);
                    const known = isCenter ? !!voxel : readCache.neighbours.has(neighbourKey);
                    let symbol = known ? '·' : '…';
                    let color = '#475569';
                    let bg = '#0f172a';
                    let borderStyle = 'border:1px solid #334155;';

                    if (!known) {
                        color = '#64748b';
                        bg = '#111827';
                    } else if (nV && nV.state === 1) {
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
    readCache.renderedRevision = readCache.revision;
}

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

export function normalizeLatticePosition(position, latticeSize) {
    const L = Number(latticeSize);
    if (!position || !Number.isSafeInteger(L) || L < 1) return null;
    const result = {};
    for (const axis of ['x', 'y', 'z']) {
        const raw = position[axis];
        if ((typeof raw !== 'number' && typeof raw !== 'string')
            || (typeof raw === 'string' && !raw.trim())) return null;
        const value = Number(raw);
        if (!Number.isFinite(value)) return null;
        result[axis] = Math.max(0, Math.min(L - 1, Math.round(value)));
    }
    return result;
}

function buildNeighbourOrder(x, y, z, L) {
    const order = [];
    const seen = new Set([voxelKey(x, y, z)]);
    for (let dz = -1; dz <= 1; dz++) {
        for (let dy = 1; dy >= -1; dy--) {
            for (let dx = -1; dx <= 1; dx++) {
                const pos = { x: (x + dx + L) % L, y: (y + dy + L) % L, z: (z + dz + L) % L };
                const key = voxelKey(pos.x, pos.y, pos.z);
                if (seen.has(key)) continue;
                seen.add(key);
                order.push(pos);
            }
        }
    }
    return order;
}

function currentTick(bridge) {
    const tick = typeof bridge?.currentTick === 'function' ? bridge.currentTick() : null;
    return Number.isSafeInteger(tick) && tick >= 0 ? tick : null;
}

function inspectionCache(target, x, y, z) {
    const bridge = target.bridge;
    const L = Number(bridge?.latticeSize);
    const positionKey = `${L}:${voxelKey(x, y, z)}`;
    const sourceKey = `${bridge?.configurationToken ?? ''}:${bridge?._scenarioDataGeneration ?? ''}`;
    const tick = currentTick(bridge);
    let cache = target._latticeInspectionCache;
    if (!cache || cache.bridge !== bridge || cache.positionKey !== positionKey
        || cache.sourceKey !== sourceKey || cache.socket !== bridge?._ws
        || (tick !== null && cache.lastTick !== null && tick < cache.lastTick)) {
        cache = {
            bridge, positionKey, centerKey: voxelKey(x, y, z), sourceKey,
            socket: bridge?._ws, lastTick: tick,
            lastRequestAt: -Infinity, lastVisualEpoch: null,
            voxel: null, force: null, voxelMeta: null, forceMeta: null,
            neighbours: new Map(), neighbourMeta: new Map(),
            neighbourOrder: buildNeighbourOrder(x, y, z, L), cursor: 0,
            revision: 0, renderedRevision: -1,
        };
        target._latticeInspectionCache = cache;
    }
    cache.lastTick = tick;
    return cache;
}

function sameRecord(a, b) {
    if (a === b) return true;
    if (!a || !b) return false;
    const keys = Object.keys(a);
    return keys.length === Object.keys(b).length && keys.every(key => Object.is(a[key], b[key]));
}

function storeSample(cache, key, value, meta = null, force = false) {
    const center = key === cache.centerKey;
    const old = force ? cache.force : center ? cache.voxel : cache.neighbours.get(key) ?? null;
    const oldMeta = force ? cache.forceMeta : center ? cache.voxelMeta : cache.neighbourMeta.get(key) ?? null;
    if (sameRecord(old, value) && sameRecord(oldMeta, meta)) return;
    // Own primitive records so later producer mutation cannot rewrite a display.
    const owned = value ? { ...value } : null;
    const ownedMeta = value && meta ? { ...meta } : null;
    if (force) { cache.force = owned; cache.forceMeta = ownedMeta; }
    else if (center) { cache.voxel = owned; cache.voxelMeta = ownedMeta; }
    else if (owned) { cache.neighbours.set(key, owned); cache.neighbourMeta.set(key, ownedMeta); }
    else { cache.neighbours.delete(key); cache.neighbourMeta.delete(key); }
    cache.revision++;
}

function consumeCompleted(cache, x, y, z) {
    const bridge = cache.bridge;
    // Accessors and native Maps contain completed replies and never issue work.
    // The native point-reply protocol currently supplies no actual sampled tick.
    const read = typeof bridge.getInspectionSample === 'function'
        ? pos => bridge.getInspectionSample(pos.x, pos.y, pos.z)
        : bridge.isNativeGPU && bridge._voxelCache instanceof Map
            ? pos => bridge._voxelCache.get(voxelKey(pos.x, pos.y, pos.z)) ?? null : null;
    if (read) {
        for (const pos of [{ x, y, z }, ...cache.neighbourOrder]) {
            const value = read(pos);
            const meta = value && typeof bridge.getInspectionSampleMeta === 'function'
                ? bridge.getInspectionSampleMeta(pos.x, pos.y, pos.z) : null;
            storeSample(cache, voxelKey(pos.x, pos.y, pos.z), value, meta);
        }
    }
    const force = typeof bridge.getForceSample === 'function'
        ? bridge.getForceSample(x, y, z)
        : bridge.isNativeGPU && bridge._forceAtCache instanceof Map
            ? bridge._forceAtCache.get(voxelKey(x, y, z)) ?? null : undefined;
    if (force !== undefined) storeSample(cache, voxelKey(x, y, z), force, null, true);
}

function refreshInspectionCache(target, x, y, z) {
    const bridge = target.bridge;
    const cache = inspectionCache(target, x, y, z);
    if (typeof bridge?.inspectVoxel !== 'function') return cache;
    // Consume completed replies before cadence and paused-epoch guards.
    // Scheduling a read does not establish that its response was observed.
    consumeCompleted(cache, x, y, z);
    const native = !!bridge.isNativeGPU;
    const interval = native ? NATIVE_REFRESH_MS : LOCAL_REFRESH_MS;
    const now = nowMs();
    const visualEpoch = native && Number.isFinite(Number(bridge._visualEpoch))
        ? Number(bridge._visualEpoch) : null;
    const incomplete = !cache.voxel || cache.neighbours.size < cache.neighbourOrder.length
        || (typeof bridge.getForceAt === 'function' && !cache.force);
    if (visualEpoch !== null && visualEpoch === cache.lastVisualEpoch && !incomplete) return cache;
    if (now - cache.lastRequestAt < interval) return cache;
    cache.lastRequestAt = now;
    cache.lastVisualEpoch = visualEpoch;
    const synchronous = !native && typeof bridge.getInspectionSample !== 'function';
    const request = pos => {
        const value = bridge.inspectVoxel(pos.x, pos.y, pos.z);
        const meta = value && typeof bridge.getInspectionSampleMeta === 'function'
            ? bridge.getInspectionSampleMeta(pos.x, pos.y, pos.z)
            : value && synchronous ? { sampleTick: currentTick(bridge), stale: false } : null;
        storeSample(cache, voxelKey(pos.x, pos.y, pos.z), value, meta);
    };
    request({ x, y, z });
    if (typeof bridge.getForceAt === 'function') {
        const force = bridge.getForceAt(x, y, z);
        storeSample(cache, voxelKey(x, y, z), force,
            force && synchronous ? { sampleTick: currentTick(bridge), stale: false } : null, true);
    }
    const budget = native ? NATIVE_NEIGHBOUR_READ_BUDGET : LOCAL_NEIGHBOUR_READ_BUDGET;
    const order = cache.neighbourOrder;
    for (let i = 0; i < Math.min(budget, order.length); i++) request(order[(cache.cursor + i) % order.length]);
    cache.cursor = order.length ? (cache.cursor + budget) % order.length : 0;
    return cache;
}

function sampleSummary(cache) {
    const metas = [cache.voxelMeta, ...cache.neighbourMeta.values()];
    const ticks = metas.map(meta => meta?.sampleTick).filter(tick => Number.isSafeInteger(tick) && tick >= 0);
    const expected = 1 + cache.neighbourOrder.length;
    const coverage = `${Number(!!cache.voxel) + cache.neighbours.size}/${expected} sites resolved`;
    const observed = ticks.length ? `sampled ticks ${Math.min(...ticks)}..${Math.max(...ticks)}` : 'sample ticks unavailable';
    const incomplete = ticks.length < expected ? '; some sample ticks unavailable' : '';
    const stale = metas.some(meta => meta?.stale) ? '; retained older samples' : '';
    const forceTick = cache.forceMeta?.sampleTick;
    const force = Number.isSafeInteger(forceTick) ? `force sampled tick ${forceTick}` : 'force sample tick unavailable';
    return `Periodic geometric neighborhood - ${coverage} - ${observed}${ticks.length ? incomplete : ''}${stale}. ${force}. Separate point observations; values are not one simultaneous neighborhood sample.`;
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
        const positions = hit.object?.geometry?.getAttribute('position');
        if (!Number.isSafeInteger(hit.index) || hit.index < 0 || !positions?.array
            || hit.index * 3 + 2 >= positions.array.length) return;
        const position = normalizeLatticePosition({
            x: Math.floor(positions.array[hit.index * 3]),
            y: Math.floor(positions.array[hit.index * 3 + 1]),
            z: Math.floor(positions.array[hit.index * 3 + 2]),
        }, target.bridge?.latticeSize);
        if (!position) return;
        target.selectedIndex = hit.index;
        target._selectedPos = position;
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
    const position = normalizeLatticePosition(target._selectedPos, target.bridge?.latticeSize);
    if (!position) return;
    target._selectedPos = position;
    const { x, y, z } = position;

    const readCache = refreshInspectionCache(target, x, y, z);
    let voxel = readCache.voxel;
    let force = readCache.force;

    // The app asks the inspector to paint more often than the bounded read
    // budget. Rebuilding a 27-cell HTML grid and sixteen text fields with no
    // new engine response only adds layout work; retain the last real snapshot
    // until the next scheduled read revision arrives.
    if (readCache.renderedRevision === readCache.revision) return;


    if (voxel) {
        const stateLabel = voxel.state === 1 ? '+1 (positive)' : voxel.state === -1 ? '-1 (negative)'
            : voxel.state === 0 ? '0 (void)' : 'Invalid state';
        const stateColor = voxel.state === 1 ? '#4ade80' : voxel.state === -1 ? '#f87171'
            : voxel.state === 0 ? '#9ca3af' : '#fbbf24';
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
        let html = `<div class="inspector-observation-scope" style="font-size:16px;color:var(--text-muted);margin-bottom:8px">${sampleSummary(readCache)}</div>`;
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
                    const centerAlias = nX === x && nY === y && nZ === z;
                    const nV = centerAlias ? voxel : readCache.neighbours.get(neighbourKey);
                    const known = !!nV;
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
                        const fx = nV.fluxX;
                        const fy = nV.fluxY;
                        const fz = nV.fluxZ;
                        const fluxMag = Math.sqrt(fx * fx + fy * fy + fz * fz);
                        if (!Number.isFinite(fluxMag)) { symbol = '?'; color = '#fbbf24'; }
                        else if (fluxMag > 0.001) {
                            const intensity = Math.min(1.0, fluxMag * 2.0);
                            bg = `rgba(56, 189, 248, ${intensity * 0.4})`;
                            color = `rgba(125, 211, 252, ${0.4 + intensity * 0.6})`;
                            if (fluxMag > 0.1) symbol = '~';
                        }
                    }

                    if (known && ![-1, 0, 1].includes(nV.state)) { symbol = '?'; color = '#fbbf24'; }
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

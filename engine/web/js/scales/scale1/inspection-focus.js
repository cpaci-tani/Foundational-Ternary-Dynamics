/**
 * Scale 1 inspection-focus contract.
 *
 * Focus is presentation-only. It narrows visual sources and overlay records;
 * it never mutates the native ParticleEngine, its toggles, or its ledger.
 */

function finiteId(value) {
    const id = Number(value);
    return Number.isInteger(id) && id >= 0 ? id : null;
}

function uniqueIds(values) {
    return Array.from(new Set(Array.from(values || [])
        .map(finiteId)
        .filter(id => id !== null)))
        .sort((a, b) => a - b);
}

function vector(value) {
    return {
        x: Number.isFinite(Number(value?.x)) ? Number(value.x) : 0,
        y: Number.isFinite(Number(value?.y)) ? Number(value.y) : 0,
        z: Number.isFinite(Number(value?.z)) ? Number(value.z) : 0,
    };
}

export function particleInspectionFocus(particleId) {
    const id = finiteId(particleId);
    if (id === null) return null;
    return {
        kind: 'particle',
        particleId: id,
        particleIds: [id],
        particleIdSet: new Set([id]),
        anchorId: id,
    };
}

export function clusterInspectionFocus(cluster, energyBasis = 'dynamic_activity') {
    if (!cluster) return null;
    const particleIds = uniqueIds(cluster.particles?.map(particle => particle.id)
        ?? cluster.particleIds);
    if (!particleIds.length) return null;
    const anchorId = finiteId(cluster.anchorId);
    return {
        kind: 'cluster',
        key: String(cluster.key || particleIds.join('.')),
        clusterId: String(cluster.id || 'Cluster'),
        particleIds,
        particleIdSet: new Set(particleIds),
        anchorId: anchorId !== null && particleIds.includes(anchorId) ? anchorId : particleIds[0],
        center: vector(cluster.center),
        energy: Number.isFinite(Number(cluster.energy)) ? Number(cluster.energy) : 0,
        energyBasis: String(energyBasis || 'dynamic_activity'),
    };
}

export function sameInspectionFocus(a, b) {
    if (a === b) return true;
    if (!a || !b || a.kind !== b.kind || a.anchorId !== b.anchorId) return false;
    if (a.kind === 'particle') return a.particleId === b.particleId;
    if (a.key !== b.key || a.clusterId !== b.clusterId) return false;
    if (a.particleIds.length !== b.particleIds.length) return false;
    return a.particleIds.every((id, index) => id === b.particleIds[index]);
}

/** Keep a dynamic cluster focus attached to the live group with most overlap. */
export function reconcileInspectionFocus(focus, hierarchy) {
    if (!focus) return null;
    const particles = Array.from(hierarchy?.particles || []);
    if (focus.kind === 'particle') {
        return particles.some(particle => particle.id === focus.particleId) ? focus : null;
    }

    const clusters = Array.from(hierarchy?.clusters || []);
    let cluster = clusters.find(candidate => candidate.key === focus.key);
    if (!cluster) {
        let bestOverlap = 0;
        for (const candidate of clusters) {
            const overlap = candidate.particles.reduce((count, particle) =>
                count + (focus.particleIdSet.has(particle.id) ? 1 : 0), 0);
            if (overlap > bestOverlap
                || (overlap === bestOverlap && overlap > 0
                    && candidate.energy > (cluster?.energy ?? -Infinity))) {
                cluster = candidate;
                bestOverlap = overlap;
            }
        }
        if (bestOverlap === 0) return null;
    }
    return clusterInspectionFocus(cluster, hierarchy?.energyBasis);
}

function ensureSourceScratch(scratch, count) {
    const positionsLength = count * 3;
    if (!scratch.positions || scratch.positions.length < positionsLength) {
        scratch.positions = new Float32Array(positionsLength);
    }
    if (!scratch.charges || scratch.charges.length < count) {
        scratch.charges = new Float32Array(count);
    }
    if (!scratch.masses || scratch.masses.length < count) {
        scratch.masses = new Float32Array(count);
    }
    return scratch;
}

/** Return field sources aligned only to the focused native particle IDs. */
export function focusedFieldSources(sources, ids, focus, scratch = {}) {
    if (!focus) return sources;
    const sourceCount = Math.max(0, Math.min(Number(sources?.count) || 0, ids?.length || 0));
    ensureSourceScratch(scratch, Math.min(sourceCount, focus.particleIds.length));
    let count = 0;
    for (let index = 0; index < sourceCount; index++) {
        if (!focus.particleIdSet.has(Number(ids[index]))) continue;
        const source3 = index * 3;
        const target3 = count * 3;
        scratch.positions[target3] = sources.positions[source3];
        scratch.positions[target3 + 1] = sources.positions[source3 + 1];
        scratch.positions[target3 + 2] = sources.positions[source3 + 2];
        scratch.charges[count] = sources.charges?.[index] ?? 0;
        scratch.masses[count] = sources.masses?.[index] ?? 0;
        count++;
    }
    scratch.count = count;
    return scratch;
}

/**
 * Compute focus-local center of mass, total momentum, and angular momentum.
 * Without a focus, the caller should keep using the native conservation row.
 */
export function focusedSystemObservables(peData, focus) {
    if (!focus || !peData?.positions || !peData?.ids) return null;
    const selected = [];
    for (let index = 0; index < peData.count; index++) {
        if (focus.particleIdSet.has(Number(peData.ids[index]))) selected.push(index);
    }
    if (!selected.length) return null;

    let totalMass = 0;
    const center = [0, 0, 0];
    const momentum = [0, 0, 0];
    for (const index of selected) {
        const mass = Math.max(0, Number(peData.masses?.[index]) || 0) || 1;
        const i3 = index * 3;
        totalMass += mass;
        center[0] += mass * peData.positions[i3];
        center[1] += mass * peData.positions[i3 + 1];
        center[2] += mass * peData.positions[i3 + 2];
        momentum[0] += mass * (peData.velocities?.[i3] ?? 0);
        momentum[1] += mass * (peData.velocities?.[i3 + 1] ?? 0);
        momentum[2] += mass * (peData.velocities?.[i3 + 2] ?? 0);
    }
    center[0] /= totalMass;
    center[1] /= totalMass;
    center[2] /= totalMass;

    const angularMomentum = [0, 0, 0];
    for (const index of selected) {
        const mass = Math.max(0, Number(peData.masses?.[index]) || 0) || 1;
        const i3 = index * 3;
        const rx = peData.positions[i3] - center[0];
        const ry = peData.positions[i3 + 1] - center[1];
        const rz = peData.positions[i3 + 2] - center[2];
        const px = mass * (peData.velocities?.[i3] ?? 0);
        const py = mass * (peData.velocities?.[i3 + 1] ?? 0);
        const pz = mass * (peData.velocities?.[i3 + 2] ?? 0);
        angularMomentum[0] += ry * pz - rz * py;
        angularMomentum[1] += rz * px - rx * pz;
        angularMomentum[2] += rx * py - ry * px;
    }
    return { center, momentum, angularMomentum, count: selected.length };
}

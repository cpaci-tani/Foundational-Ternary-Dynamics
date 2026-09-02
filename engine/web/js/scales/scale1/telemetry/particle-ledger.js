/**
 * Scale-1 particle relationship and event ledger.
 *
 * The ParticleEngine remains the dynamics owner. This observer derives a
 * presentation hierarchy from published snapshots; it does not feed forces
 * back into the simulation and must not be read as a recovered bound-state
 * or gravitational ontology.
 */

export const PARTICLE_LOG_CATEGORIES = Object.freeze([
    { id: 'lifecycle', label: 'Lifecycle', color: '#67e8f9' },
    { id: 'energy', label: 'Energy', color: '#fbbf24' },
    { id: 'hierarchy', label: 'Hierarchy', color: '#a78bfa' },
    { id: 'interaction', label: 'Interactions', color: '#fb7185' },
    { id: 'environment', label: 'Environment', color: '#94a3b8' },
]);

const MAX_EVENT_HISTORY = 4096;
const ENERGY_RELATIVE_THRESHOLD = 0.25;
const SYSTEM_ENERGY_RELATIVE_THRESHOLD = 0.05;
const ENERGY_EVENT_COOLDOWN_TICKS = 8;
const EPS = 1e-18;

function finite(value, fallback = 0) {
    return Number.isFinite(Number(value)) ? Number(value) : fallback;
}

function vec3(value, fallback = { x: 0, y: 0, z: 0 }) {
    return {
        x: finite(value?.x, fallback.x),
        y: finite(value?.y, fallback.y),
        z: finite(value?.z, fallback.z),
    };
}

function arrayVec3(values, index) {
    return {
        x: finite(values?.[index * 3]),
        y: finite(values?.[index * 3 + 1]),
        z: finite(values?.[index * 3 + 2]),
    };
}

function distance(a, b) {
    return Math.hypot(a.x - b.x, a.y - b.y, a.z - b.z);
}

function magnitudeAt(values, index) {
    return Math.hypot(
        finite(values?.[index * 3]),
        finite(values?.[index * 3 + 1]),
        finite(values?.[index * 3 + 2]),
    );
}

function weightedCenter(particles) {
    let total = 0;
    let x = 0;
    let y = 0;
    let z = 0;
    for (const particle of particles) {
        const weight = Math.max(EPS, particle.dynamicEnergy);
        total += weight;
        x += particle.position.x * weight;
        y += particle.position.y * weight;
        z += particle.position.z * weight;
    }
    if (total <= EPS) return { x: 0, y: 0, z: 0 };
    return { x: x / total, y: y / total, z: z / total };
}

function nearestAnchor(particles, center) {
    let selected = null;
    let selectedDistance = Infinity;
    for (const particle of particles) {
        const d = distance(particle.position, center);
        if (d < selectedDistance - 1e-12
            || (Math.abs(d - selectedDistance) <= 1e-12
                && (particle.dynamicEnergy > (selected?.dynamicEnergy ?? -Infinity)
                    || (particle.dynamicEnergy === selected?.dynamicEnergy
                        && particle.id < selected.id)))) {
            selected = particle;
            selectedDistance = d;
        }
    }
    return selected?.id ?? null;
}

class UnionFind {
    constructor(size) {
        this.parent = Array.from({ length: size }, (_, index) => index);
        this.rank = new Uint8Array(size);
    }

    find(index) {
        let root = index;
        while (this.parent[root] !== root) root = this.parent[root];
        while (this.parent[index] !== index) {
            const next = this.parent[index];
            this.parent[index] = root;
            index = next;
        }
        return root;
    }

    union(a, b) {
        let rootA = this.find(a);
        let rootB = this.find(b);
        if (rootA === rootB) return;
        if (this.rank[rootA] < this.rank[rootB]) [rootA, rootB] = [rootB, rootA];
        this.parent[rootB] = rootA;
        if (this.rank[rootA] === this.rank[rootB]) this.rank[rootA]++;
    }
}

function makeParticleRecords(peData, snapshot, forceData) {
    const objects = new Map(Array.from(snapshot?.objects || []).map(row => [Number(row.id), row]));
    const particles = [];
    const count = Math.max(0, finite(peData?.count));
    for (let index = 0; index < count; index++) {
        const id = finite(peData?.ids?.[index], index);
        const object = objects.get(id);
        const mass = Math.max(0, finite(peData?.masses?.[index], object?.mass));
        const velocity = arrayVec3(peData?.velocities, index);
        const speed = Math.hypot(velocity.x, velocity.y, velocity.z);
        const kineticEnergy = object?.kineticEnergyAvailable
            ? Math.abs(finite(object.kineticEnergy))
            : Math.max(0, 0.5 * mass * speed * speed);
        const netForce = magnitudeAt(forceData?.net ?? forceData?.forces, index);
        const effectiveRadius = Math.max(0.001,
            finite(peData?.rEff?.[index], object?.effectiveRadius || 0.4));
        particles.push({
            id,
            index,
            charge: finite(peData?.charges?.[index], object?.effectiveState),
            mass,
            effectiveRadius,
            position: object ? vec3(object.position) : arrayVec3(peData?.positions, index),
            velocity,
            speed,
            kineticEnergy,
            netForce,
            dynamicEnergy: kineticEnergy + netForce * effectiveRadius,
            locked: !!(peData?.locked?.[index] ?? object?.locked),
            spin: finite(peData?.spins?.[index]),
            colorId: finite(peData?.colorIds?.[index]),
            parentIds: Array.from(object?.parentIds || []).map(Number).filter(Number.isFinite),
            provenance: object?.provenance || null,
            nearestId: null,
            nearestDistance: Infinity,
            hierarchyParentId: null,
            influenceScore: 0,
        });
    }
    return particles;
}

function assignParents(cluster, softening) {
    const byId = new Map(cluster.particles.map(particle => [particle.id, particle]));
    const root = byId.get(cluster.anchorId);
    if (!root) return;
    root.hierarchyParentId = null;
    root.influenceScore = Infinity;
    const placed = [root];
    const remaining = cluster.particles
        .filter(particle => particle.id !== root.id)
        .sort((a, b) => b.dynamicEnergy - a.dynamicEnergy || a.id - b.id);
    for (const particle of remaining) {
        let parent = placed[0];
        let best = -Infinity;
        for (const candidate of placed) {
            const d = distance(particle.position, candidate.position);
            const score = candidate.dynamicEnergy / (d * d + softening * softening);
            if (score > best) {
                best = score;
                parent = candidate;
            }
        }
        particle.hierarchyParentId = parent.id;
        particle.influenceScore = best;
        placed.push(particle);
    }
}

/**
 * Build an instantaneous energy-weighted relationship hierarchy.
 *
 * Dynamic weight = |kinetic energy| + |net force| times a local length scale.
 * Only a completely dormant frame falls back to mass weighting. The result is
 * a presentation observable, not a new force law.
 */
export function buildParticleHierarchy({
    peData,
    snapshot = null,
    forceData = null,
    softening = 0.1,
} = {}) {
    const particles = makeParticleRecords(peData, snapshot, forceData);
    const count = particles.length;
    if (count === 0) {
        return {
            particles: [], clusters: [], globalCenter: { x: 0, y: 0, z: 0 },
            globalAnchorId: null, totalDynamicEnergy: 0, energyBasis: 'dynamic_activity',
            closePairs: [], signature: '',
        };
    }

    // Nearest-neighbour distances supply the local work length and adaptive
    // cluster reach. This remains deterministic under particle reordering.
    for (let i = 0; i < count; i++) {
        for (let j = i + 1; j < count; j++) {
            const d = distance(particles[i].position, particles[j].position);
            if (d < particles[i].nearestDistance) {
                particles[i].nearestDistance = d;
                particles[i].nearestId = particles[j].id;
            }
            if (d < particles[j].nearestDistance) {
                particles[j].nearestDistance = d;
                particles[j].nearestId = particles[i].id;
            }
        }
    }

    for (const particle of particles) {
        const localLength = Number.isFinite(particle.nearestDistance)
            ? Math.max(particle.effectiveRadius, particle.nearestDistance * 0.5)
            : particle.effectiveRadius;
        particle.dynamicEnergy = particle.kineticEnergy + particle.netForce * localLength;
    }
    let totalDynamicEnergy = particles.reduce((sum, particle) => sum + particle.dynamicEnergy, 0);
    let energyBasis = 'dynamic_activity';
    if (totalDynamicEnergy <= EPS * count) {
        energyBasis = 'mass_fallback';
        for (const particle of particles) particle.dynamicEnergy = Math.max(EPS, particle.mass);
        totalDynamicEnergy = particles.reduce((sum, particle) => sum + particle.dynamicEnergy, 0);
    } else {
        const floor = Math.max(EPS, totalDynamicEnergy * 1e-12);
        for (const particle of particles) particle.dynamicEnergy = Math.max(floor, particle.dynamicEnergy);
        totalDynamicEnergy = particles.reduce((sum, particle) => sum + particle.dynamicEnergy, 0);
    }

    const meanEnergy = Math.max(EPS, totalDynamicEnergy / count);
    const union = new UnionFind(count);
    const closePairs = [];
    const indexById = new Map(particles.map((particle, index) => [particle.id, index]));

    // Native constituent links are authoritative relationships when present.
    for (let i = 0; i < count; i++) {
        for (const parentId of particles[i].parentIds) {
            const parentIndex = indexById.get(parentId);
            if (parentIndex !== undefined) union.union(i, parentIndex);
        }
    }

    for (let i = 0; i < count; i++) {
        for (let j = i + 1; j < count; j++) {
            const a = particles[i];
            const b = particles[j];
            const d = distance(a.position, b.position);
            const nearestScale = Math.min(a.nearestDistance, b.nearestDistance);
            const energyFactor = 1 + 0.2 * Math.min(2,
                Math.sqrt((a.dynamicEnergy + b.dynamicEnergy) / (2 * meanEnergy)));
            const reach = Math.max(
                (a.effectiveRadius + b.effectiveRadius) * 1.5,
                (Number.isFinite(nearestScale) ? nearestScale : d) * 1.8,
            ) * energyFactor;
            if (count === 2 || d <= reach) union.union(i, j);

            const encounterRadius = Math.max(
                (a.effectiveRadius + b.effectiveRadius) * 1.25,
                Math.max(0.25, finite(softening)),
            );
            if (d <= encounterRadius) {
                closePairs.push({
                    key: a.id < b.id ? `${a.id}:${b.id}` : `${b.id}:${a.id}`,
                    a: Math.min(a.id, b.id),
                    b: Math.max(a.id, b.id),
                    distance: d,
                    threshold: encounterRadius,
                });
            }
        }
    }

    const groups = new Map();
    for (let i = 0; i < count; i++) {
        const root = union.find(i);
        if (!groups.has(root)) groups.set(root, []);
        groups.get(root).push(particles[i]);
    }

    const clusters = Array.from(groups.values()).map(members => {
        members.sort((a, b) => a.id - b.id);
        const center = weightedCenter(members);
        const anchorId = nearestAnchor(members, center);
        const energy = members.reduce((sum, particle) => sum + particle.dynamicEnergy, 0);
        const key = members.map(particle => particle.id).join('.');
        return { key, id: '', particles: members, center, anchorId, energy };
    }).sort((a, b) => b.energy - a.energy || a.key.localeCompare(b.key));

    clusters.forEach((cluster, index) => {
        cluster.id = `C${index + 1}`;
        assignParents(cluster, Math.max(0.001, finite(softening, 0.1)));
    });

    const globalCenter = weightedCenter(particles);
    const globalAnchorId = nearestAnchor(particles, globalCenter);
    const signature = clusters.map(cluster => cluster.key).sort().join('|');
    return {
        particles,
        clusters,
        globalCenter,
        globalAnchorId,
        totalDynamicEnergy,
        energyBasis,
        closePairs,
        signature,
    };
}

function relativeChange(previous, current) {
    return Math.abs(current - previous) / Math.max(Math.abs(previous), Math.abs(current), EPS);
}

function environmentState(snapshot, scenarioId) {
    const core = snapshot?.core || {};
    const conservation = snapshot?.conservation || {};
    return {
        scenarioId: scenarioId || core.scenario || '',
        backend: core.backend || 'unknown',
        mode: core.mode || 'unknown',
        readOnly: !!core.readOnly,
        coveredMask: finite(conservation.coveredMask),
        missingMask: finite(conservation.missingMask),
        nonconservativeMask: finite(conservation.nonconservativeMask),
        stateEnergyComplete: !!conservation.stateEnergyComplete,
    };
}

export class Scale1ParticleLedger {
    constructor({ maxEvents = MAX_EVENT_HISTORY } = {}) {
        this.maxEvents = Math.max(32, Math.floor(maxEvents));
        this.events = [];
        this.nextEventId = 1;
        this.eventRevision = 0;
        this.hierarchyRevision = 0;
        this.currentScenarioId = '';
        this.scenarioEpoch = 0;
        this.lastFrameKey = '';
        this.previousParticles = new Map();
        this.previousHierarchy = null;
        this.previousEnvironment = null;
        this.previousStateEnergy = null;
        this.activeEncounters = new Set();
        this.processedNativeEvents = new Set();
        this.lastEnergyEventTick = new Map();
        this.lastSystemEnergyEventTick = -Infinity;
        this.hierarchy = buildParticleHierarchy();
        this.tick = 0;
    }

    beginScenario({ scenarioId = '', label = scenarioId, tick = 0 } = {}) {
        if (scenarioId === this.currentScenarioId && this.previousParticles.size === 0) return;
        this.currentScenarioId = scenarioId;
        this.scenarioEpoch++;
        this.lastFrameKey = '';
        this.previousParticles.clear();
        this.previousHierarchy = null;
        this.previousEnvironment = null;
        this.previousStateEnergy = null;
        this.activeEncounters.clear();
        this.processedNativeEvents.clear();
        this.lastEnergyEventTick.clear();
        this.lastSystemEnergyEventTick = -Infinity;
        this.hierarchy = buildParticleHierarchy();
        this.tick = finite(tick);
        this.hierarchyRevision++;
        this._append({
            tick: this.tick,
            category: 'environment',
            severity: 'info',
            type: 'scenario_loaded',
            title: `Scenario loaded: ${label || scenarioId || 'Scale 1'}`,
            detail: 'Particle identities and hierarchy baselines were reset; prior log entries were retained.',
            source: 'scale1_observer',
        });
    }

    clearEvents() {
        this.events.length = 0;
        this.eventRevision++;
    }

    _append(event) {
        const row = {
            id: this.nextEventId++,
            tick: finite(event.tick, this.tick),
            category: event.category || 'environment',
            severity: event.severity || 'info',
            type: event.type || 'observation',
            title: event.title || 'Scale 1 observation',
            detail: event.detail || '',
            particleIds: Array.from(event.particleIds || []),
            clusterIds: Array.from(event.clusterIds || []),
            energyDelta: Number.isFinite(event.energyDelta) ? event.energyDelta : null,
            source: event.source || 'scale1_observer',
            status: event.status || '',
        };
        this.events.push(row);
        if (this.events.length > this.maxEvents) {
            this.events.splice(0, this.events.length - this.maxEvents);
        }
        this.eventRevision++;
        return row;
    }

    _observeNativeEvents(snapshot, tick) {
        const removedParticipants = new Set();
        for (const event of Array.from(snapshot?.events || [])) {
            const sequence = finite(event.sequence, -1);
            const key = `${this.scenarioEpoch}:${sequence}`;
            if (sequence < 0 || this.processedNativeEvents.has(key)) continue;
            this.processedNativeEvents.add(key);
            const a = finite(event.participantA, -1);
            const b = finite(event.participantB, -1);
            if (event.type === 'contact_removal') {
                if (a >= 0) removedParticipants.add(a);
                if (b >= 0) removedParticipants.add(b);
                this._append({
                    tick: finite(event.tick, tick),
                    category: 'lifecycle',
                    severity: 'critical',
                    type: 'contact_removal',
                    title: `Selected contact removal: #${a} + #${b}`,
                    detail: event.accountingComplete
                        ? 'Native event with complete state-energy accounting.'
                        : 'Native event; state-energy delta is partial or shared across a batch.',
                    particleIds: [a, b].filter(id => id >= 0),
                    energyDelta: finite(event.stateEnergyDelta),
                    source: event.sourceId || 'contact_events',
                    status: event.status || 'selection',
                });
            } else {
                this._append({
                    tick: finite(event.tick, tick),
                    category: 'lifecycle',
                    severity: 'important',
                    type: event.type || 'native_event',
                    title: `Native ${String(event.type || 'event').replaceAll('_', ' ')}`,
                    detail: `Participants ${a >= 0 ? `#${a}` : 'n/a'} and ${b >= 0 ? `#${b}` : 'n/a'}.`,
                    particleIds: [a, b].filter(id => id >= 0),
                    energyDelta: finite(event.stateEnergyDelta),
                    source: event.sourceId || 'particle_engine',
                    status: event.status || '',
                });
            }
        }
        return removedParticipants;
    }

    observe({ peData, snapshot = null, forceData = null, scenarioId = '', scenarioLabel = '', softening = 0.1 } = {}) {
        const resolvedScenario = scenarioId || snapshot?.core?.scenario || this.currentScenarioId;
        if (resolvedScenario !== this.currentScenarioId) {
            this.beginScenario({ scenarioId: resolvedScenario, label: scenarioLabel || resolvedScenario,
                tick: snapshot?.core?.tick });
        }
        const tick = finite(snapshot?.core?.tick, this.tick);
        const ids = Array.from(peData?.ids || []).slice(0, finite(peData?.count)).map(Number);
        const conservation = snapshot?.conservation || {};
        const nativeEvents = Array.from(snapshot?.events || []);
        const lastSequence = nativeEvents.length ? nativeEvents[nativeEvents.length - 1]?.sequence : -1;
        const frameKey = [resolvedScenario, tick, ids.join(','), finite(conservation.coveredMask),
            finite(conservation.missingMask), finite(lastSequence)].join(':');
        if (frameKey === this.lastFrameKey) return false;
        this.lastFrameKey = frameKey;
        this.tick = tick;

        const hierarchy = buildParticleHierarchy({ peData, snapshot, forceData, softening });
        const currentParticles = new Map(hierarchy.particles.map(particle => [particle.id, particle]));
        const nativeRemoved = this._observeNativeEvents(snapshot, tick);

        for (const particle of hierarchy.particles) {
            if (!this.previousParticles.has(particle.id)) {
                this._append({
                    tick,
                    category: 'lifecycle',
                    severity: 'info',
                    type: 'spawn',
                    title: `Particle #${particle.id} entered the active record`,
                    detail: `q=${particle.charge}; dynamic energy=${particle.dynamicEnergy.toExponential(3)}.`,
                    particleIds: [particle.id],
                    source: particle.provenance?.sourceKind || 'snapshot_diff',
                    status: particle.provenance?.status || '',
                });
            }
        }
        for (const [id] of this.previousParticles) {
            if (currentParticles.has(id) || nativeRemoved.has(id)) continue;
            this._append({
                tick,
                category: 'lifecycle',
                severity: 'important',
                type: 'despawn',
                title: `Particle #${id} left the active record`,
                detail: 'No matching native contact-removal event was published for this disappearance.',
                particleIds: [id],
                source: 'snapshot_diff',
            });
        }

        const energyFloor = Math.max(EPS, hierarchy.totalDynamicEnergy * 0.01);
        for (const particle of hierarchy.particles) {
            const previous = this.previousParticles.get(particle.id);
            if (!previous) continue;
            const delta = particle.dynamicEnergy - previous.dynamicEnergy;
            const rel = relativeChange(previous.dynamicEnergy, particle.dynamicEnergy);
            const lastTick = this.lastEnergyEventTick.get(particle.id) ?? -Infinity;
            if (Math.abs(delta) >= energyFloor && rel >= ENERGY_RELATIVE_THRESHOLD
                && tick - lastTick >= ENERGY_EVENT_COOLDOWN_TICKS) {
                this._append({
                    tick,
                    category: 'energy',
                    severity: rel >= 0.75 ? 'critical' : 'important',
                    type: 'particle_energy_change',
                    title: `Particle #${particle.id} dynamic energy ${delta >= 0 ? 'rose' : 'fell'} ${(rel * 100).toFixed(1)}%`,
                    detail: `${previous.dynamicEnergy.toExponential(3)} -> ${particle.dynamicEnergy.toExponential(3)} observer units.`,
                    particleIds: [particle.id],
                    energyDelta: delta,
                    source: 'energy_weight_observer',
                });
                this.lastEnergyEventTick.set(particle.id, tick);
            }
        }

        const stateEnergy = finite(conservation.stateEnergy, NaN);
        if (Number.isFinite(stateEnergy) && Number.isFinite(this.previousStateEnergy)) {
            const rel = relativeChange(this.previousStateEnergy, stateEnergy);
            if (rel >= SYSTEM_ENERGY_RELATIVE_THRESHOLD
                && tick - this.lastSystemEnergyEventTick >= ENERGY_EVENT_COOLDOWN_TICKS) {
                this._append({
                    tick,
                    category: 'energy',
                    severity: rel >= 0.25 ? 'critical' : 'important',
                    type: 'system_energy_change',
                    title: `System state energy changed ${(rel * 100).toFixed(1)}%`,
                    detail: conservation.stateEnergyComplete
                        ? 'Published state-energy ledger is complete for active terms.'
                        : 'Published state-energy ledger is partial; interpret the change with coverage masks.',
                    energyDelta: stateEnergy - this.previousStateEnergy,
                    source: 'scale1_conservation',
                });
                this.lastSystemEnergyEventTick = tick;
            }
        }
        this.previousStateEnergy = Number.isFinite(stateEnergy) ? stateEnergy : null;

        const nextEncounters = new Set(hierarchy.closePairs.map(pair => pair.key));
        for (const pair of hierarchy.closePairs) {
            if (this.activeEncounters.has(pair.key)) continue;
            this._append({
                tick,
                category: 'interaction',
                severity: 'important',
                type: 'close_approach',
                title: `Close approach: #${pair.a} <-> #${pair.b}`,
                detail: `Separation ${pair.distance.toFixed(4)}; encounter threshold ${pair.threshold.toFixed(4)} lu.`,
                particleIds: [pair.a, pair.b],
                source: 'proximity_observer',
            });
        }
        this.activeEncounters = nextEncounters;

        if (!this.previousHierarchy) {
            this._append({
                tick,
                category: 'hierarchy',
                severity: 'info',
                type: 'hierarchy_initialized',
                title: `Hierarchy initialized with ${hierarchy.clusters.length} cluster${hierarchy.clusters.length === 1 ? '' : 's'}`,
                detail: `Global energy anchor is ${hierarchy.globalAnchorId === null ? 'none' : `particle #${hierarchy.globalAnchorId}`}.`,
                particleIds: hierarchy.globalAnchorId === null ? [] : [hierarchy.globalAnchorId],
                source: 'energy_hierarchy_observer',
            });
        } else {
            if (hierarchy.signature !== this.previousHierarchy.signature) {
                const previousCount = this.previousHierarchy.clusters.length;
                const currentCount = hierarchy.clusters.length;
                const action = currentCount < previousCount ? 'merged'
                    : currentCount > previousCount ? 'split' : 'reconfigured';
                this._append({
                    tick,
                    category: 'hierarchy',
                    severity: 'important',
                    type: `cluster_${action}`,
                    title: `Cluster topology ${action}: ${previousCount} -> ${currentCount}`,
                    detail: 'Membership changed under the adaptive proximity and dynamic-energy observer.',
                    clusterIds: hierarchy.clusters.map(cluster => cluster.id),
                    source: 'energy_hierarchy_observer',
                });
            }
            const previousByKey = new Map(this.previousHierarchy.clusters.map(cluster => [cluster.key, cluster]));
            for (const cluster of hierarchy.clusters) {
                const previous = previousByKey.get(cluster.key);
                if (previous && previous.anchorId !== cluster.anchorId) {
                    this._append({
                        tick,
                        category: 'hierarchy',
                        severity: 'important',
                        type: 'anchor_changed',
                        title: `${cluster.id} energy anchor changed #${previous.anchorId} -> #${cluster.anchorId}`,
                        detail: 'The member nearest the instantaneous energy-weighted center changed.',
                        particleIds: [previous.anchorId, cluster.anchorId].filter(id => id !== null),
                        clusterIds: [cluster.id],
                        source: 'energy_hierarchy_observer',
                    });
                }
            }
        }

        const environment = environmentState(snapshot, resolvedScenario);
        if (this.previousEnvironment) {
            const changed = [];
            for (const key of Object.keys(environment)) {
                if (environment[key] !== this.previousEnvironment[key]) {
                    changed.push(`${key}: ${this.previousEnvironment[key]} -> ${environment[key]}`);
                }
            }
            if (changed.length) {
                this._append({
                    tick,
                    category: 'environment',
                    severity: changed.some(row => row.startsWith('stateEnergyComplete')) ? 'important' : 'info',
                    type: 'environment_changed',
                    title: 'Scale 1 environment contract changed',
                    detail: changed.join('; '),
                    source: 'snapshot_contract',
                });
            }
        }

        this.hierarchy = hierarchy;
        this.previousHierarchy = hierarchy;
        this.previousParticles = currentParticles;
        this.previousEnvironment = environment;
        this.hierarchyRevision++;
        return true;
    }

    getView() {
        return {
            tick: this.tick,
            scenarioId: this.currentScenarioId,
            hierarchy: this.hierarchy,
            events: this.events,
            retainedEventCount: this.events.length,
            maxEvents: this.maxEvents,
            eventRevision: this.eventRevision,
            hierarchyRevision: this.hierarchyRevision,
        };
    }
}

export const scale1ParticleLedger = new Scale1ParticleLedger();

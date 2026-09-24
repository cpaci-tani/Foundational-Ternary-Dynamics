/**
 * Worker-owned state for field-line knot detection.
 *
 * Streamline integration already happens in the overlay worker.  Keeping the
 * tracker next to that result prevents the client from doing its dense-cell
 * crossing pass on the rendering thread.  The client receives a detached
 * snapshot only; it remains responsible for contribution integrals because
 * those deliberately join several client-side sampler rows.
 */
// Worker-only cache boundary. The browser page's tracker stays the canonical
// client mirror; this distinct worker module is never mixed with that identity.
import { FieldLineKnotTracker } from './field-line-knots.js?v=2';

const FIELDS = new Set(['e', 'b', 'flux']);
const trackers = new Map();
const namespaceKeys = new Map();
const candidates = new Map();

function copy(value) {
    return value?.slice ? value.slice() : value;
}

function trackerFor(field) {
    if (!FIELDS.has(field)) throw new Error(`Unknown field-line knot field: ${field}`);
    let tracker = trackers.get(field);
    if (!tracker) {
        tracker = new FieldLineKnotTracker();
        tracker._field = field;
        trackers.set(field, tracker);
        namespaceKeys.set(field, '');
    }
    return tracker;
}

function cloneTracker(source) {
    // Scratch arrays are intentionally not copied: record() will grow its own
    // work buffers. Everything below affects public identity/event semantics.
    const clone = new FieldLineKnotTracker({
        cellSize: source.cellSize,
        densityThreshold: source.densityThreshold,
        crossingThreshold: source.crossingThreshold,
        minCellsPerKnot: source.minCellsPerKnot,
        crossingDist: source.crossingDist,
        parallelCos: source.parallelCos,
        maxKnots: source.maxKnots,
        minOverlapCells: source.minOverlapCells,
        maxEvents: source.maxEvents,
        requireCrossings: source.requireCrossings,
        perKnotColor: source.getPerKnotColor(),
        sensitivity: source.getSensitivity(),
    });
    clone._field = source._field;
    clone._prevCellToId = new Map(source._prevCellToId);
    clone._prevIdSize = new Map(source._prevIdSize);
    clone._histories = new Map(Array.from(source._histories,
        ([id, history]) => [id, { ...history }]));
    clone._nextId = source._nextId;
    clone._agg = { ...source._agg };
    clone._events = source._events.map((event) => ({ ...event }));
    return clone;
}

/**
 * Serialize only arrays that are already public tracker observations. Every
 * array is copied before it is transferred, leaving worker identity/history
 * state intact for the next record.
 */
export function snapshotFieldLineKnotTracker(tracker, provenance = null) {
    const telemetry = tracker.getTelemetry();
    const zones = tracker.getKnotZones();
    const events = tracker.getEvents();
    return {
        telemetry: {
            count: telemetry.count,
            ids: copy(telemetry.ids), age: copy(telemetry.age), size: copy(telemetry.size),
            peak: copy(telemetry.peak), birth: copy(telemetry.birth), stride: telemetry.stride,
            fields: copy(telemetry.fields), dirs: copy(telemetry.dirs), extents: copy(telemetry.extents),
            found: telemetry.found, dropped: telemetry.dropped, status: telemetry.status ?? null,
            // record() validates this same tick. Normal telemetry predates
            // provenance metadata, so expose it here without retimestamping.
            sampleTick: telemetry.sampleTick ?? provenance?.sampleTick ?? null,
        },
        aggregate: tracker.getAggregate(),
        zones: {
            count: zones.count, centroids: copy(zones.centroids), extents: copy(zones.extents),
            ids: copy(zones.ids), latticeSize: zones.latticeSize,
        },
        events: {
            count: events.count, tick: copy(events.tick), type: copy(events.type),
            nparents: copy(events.nparents), nchildren: copy(events.nchildren),
        },
    };
}

/**
 * Record one field in the worker's persistent tracker namespace. `resetVersion`
 * is monotonic on the client tracker and makes scenario/checkbox resets exact
 * without creating a second tracker owner on the rendering thread.
 */
export function prepareFieldLineKnotCandidate({
    candidateId, field, streamlines, fieldSamples, provenance,
    sensitivity, perKnotColor, includeLineIds = false,
}) {
    if (!Number.isSafeInteger(candidateId) || candidateId < 1) throw new Error('Invalid knot candidate id');
    const active = trackerFor(field);
    // A field namespace never crosses an authoritative source/load/reset
    // boundary. It is intentionally more specific than a request id: normal
    // successive samples retain their identity/genealogy state.
    const namespace = JSON.stringify({
        loadGeneration: provenance?.loadGeneration ?? null,
        mutationEpoch: provenance?.mutationEpoch ?? null,
        source: provenance?.source ?? null,
        nativeInstanceId: provenance?.nativeInstanceId ?? null,
        sourceEpoch: provenance?.sourceEpoch ?? provenance?.epoch ?? null,
        resetVersion: provenance?.resetVersion ?? null,
    });
    const tracker = namespaceKeys.get(field) === namespace
        ? cloneTracker(active)
        : new FieldLineKnotTracker({ perKnotColor, sensitivity });
    tracker._field = field;
    tracker.setSensitivity(sensitivity);
    tracker.setPerKnotColor(perKnotColor);
    tracker.record(streamlines, fieldSamples, provenance?.sampleTick, provenance?.latticeSize);
    const result = {
        provenance: { ...provenance },
        snapshot: snapshotFieldLineKnotTracker(tracker, provenance),
        // This O(lines × knots) coloring join is a tracker result as well. Do
        // it before transfer so E/B coloring cannot put work back on the UI.
        lineIds: includeLineIds ? tracker.assignLinesToKnots(streamlines) : null,
    };
    candidates.set(candidateId, { field, namespace, tracker });
    return result;
}

/** Commit only a client-adopted candidate. Late/invalid responses cannot alter genealogy. */
export function commitFieldLineKnotCandidate(candidateId) {
    const candidate = candidates.get(candidateId);
    if (!candidate) return false;
    candidates.delete(candidateId);
    trackers.set(candidate.field, candidate.tracker);
    namespaceKeys.set(candidate.field, candidate.namespace);
    return true;
}

export function discardFieldLineKnotCandidate(candidateId) {
    return candidates.delete(candidateId);
}

export function discardAllFieldLineKnotCandidates() {
    candidates.clear();
}

/** Test-only/worker-lifecycle support: discarding the client worker discards
 * this module namespace, but explicit reset keeps direct callers deterministic. */
export function resetFieldLineKnotWorkerState() {
    trackers.clear();
    namespaceKeys.clear();
    candidates.clear();
}

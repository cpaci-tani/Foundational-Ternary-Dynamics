// @ts-check
/** @typedef {Record<string, unknown>} Fields */
/**
 * Only completed-cache capabilities cross this boundary. Direct WASM
 * diagnostics, sampling requests and simulation methods are not accepted.
 * @typedef {object} ObservationOwner
 * @property {boolean} [isFiniteRecord]
 * @property {boolean} [isWorker]
 * @property {boolean} [ready]
 * @property {boolean} [failed]
 * @property {boolean} [disposed]
 * @property {number} [latticeSize]
 * @property {Fields} [observation]
 * @property {()=>Fields} [getProvenance]
 * @property {()=>Fields} [getTelemetrySnapshot]
 * @property {()=>Fields|null} [getDiagnostics]
 * @property {(group:string)=>Fields|null} [getScale0TelemetryGroupMeta]
 */

/** Preserve exact decimal counters; reject rounded numbers instead of repairing them.
 * @param {unknown} value @param {boolean} [signed] @returns {string|null}
 */
function integer(value, signed = false) {
    if (typeof value === 'number' && !Number.isSafeInteger(value)) return null;
    if (!['number', 'string', 'bigint'].includes(typeof value)) return null;
    const text = String(value);
    return (signed ? /^(0|-?[1-9]\d*)$/ : /^(0|[1-9]\d*)$/).test(text) ? text : null;
}

/** @param {unknown} value @returns {Fields|null} */
function fields(value) {
    return value !== null && typeof value === 'object' && !Array.isArray(value)
        ? /** @type {Fields} */ (value) : null;
}

// These fields are copied only from the same completed diagnostics packet.
// No audit reads or field scans are triggered, and absent/nonfinite channels
// remain absent. Flux and energies use the effective engine's simulation
// units; entropy is the dimensionless Shannon spread of |J|², not temperature
// or thermodynamic entropy. Bandwidth and causal budget are dimensionless
// selected-law diagnostics, not measured speeds or relativistic clocks.
const DIAGNOSTIC_SCALARS = Object.freeze([
    'physicalTime', 'dt', 'totalFlux', 'maxBandwidth', 'maxCausalBudget', 'entropy',
    'dynamicEnergy', 'vacuumBaselineEnergy', 'accountedEnergy', 'restEnergy',
    'fieldEnergy', 'waveEnergy', 'particleKE',
]);

/** @param {Fields} data @returns {Fields} */
function diagnosticScalars(data) {
    /** @type {Fields} */
    const result = {};
    for (const key of DIAGNOSTIC_SCALARS) {
        const value = data[key];
        if (typeof value === 'number' && Number.isFinite(value)) result[key] = value;
    }
    // totalEnergy is intentionally excluded: native diagnostics publish
    // sum |born_infeld_core|, whereas Worker diagnostics replace it with the
    // per-tick ledger. Only explicit, independently named channels cross here.
    if (typeof data.energySampleSource === 'string'
        && ['per-tick-ledger', 'same-tick-audit'].includes(data.energySampleSource)
        && result.dynamicEnergy !== undefined) result.energySampleSource = data.energySampleSource;
    return result;
}

/** One pass over an already published frame, cached by identity. No effective
 * field, physical energy, matter identity or SR clock is inferred.
 * @param {Fields} view @returns {Fields}
 */
function summarizeRecords(view) {
    const arrays = [view.field_tokens, view.relation_tokens, view.incidence, view.manifestation_counts];
    if (!arrays.every(Array.isArray)) throw new Error('Record counts are unavailable.');
    const [field, relation, incidence, states] = /** @type {unknown[][]} */ (arrays);
    const size = Number(view.lattice_size);
    if (!Number.isSafeInteger(size) || size < 1 || field.length !== size ** 3
        || arrays.some(array => /** @type {unknown[]} */ (array).length !== field.length)) {
        throw new Error('Record count dimensions are inconsistent.');
    }
    const totals = [0n, 0n, 0n, 0n, 0n, 0n];
    for (let i = 0; i < field.length; i++) {
        const state = states[i];
        if (!Array.isArray(state) || state.length !== 3) throw new Error('Record occupancy is unavailable.');
        const row = [field[i], relation[i], incidence[i], ...state];
        for (let j = 0; j < row.length; j++) {
            const count = integer(row[j], j === 2);
            if (count === null) throw new Error('Record counts must be exact integers.');
            totals[j] += BigInt(count);
        }
    }
    return Object.freeze({ fieldTokens: String(totals[0]), relationTokens: String(totals[1]),
        incidence: String(totals[2]), negative: String(totals[3]), zero: String(totals[4]),
        positive: String(totals[5]), manifested: String(totals[3] + totals[5]), latticeSize: size });
}

/** Read-only identity and completed-state adapter. */
export class LatticeObservationAdapter {
    /** @param {()=>{owner:ObservationOwner|null,mode:string,scenario?:string}} read */
    constructor(read) {
        this.read = read;
        /** @type {WeakMap<object,number>} */ this.identities = new WeakMap();
        /** @type {WeakMap<object,Fields>} */ this.recordSummaries = new WeakMap();
        this.serial = 0;
    }

    snapshot() {
        const { owner, mode, scenario } = this.read();
        const base = { mode, scenario, physicalMapping: 'No lattice-to-SR matter or clock identification' };
        if (!owner || mode !== 'lattice') return { ...base, available: false, stale: true,
            status: 'Unavailable', reason: 'Select Scale 0 to inspect its active owner.' };
        if (!this.identities.has(owner)) this.identities.set(owner, ++this.serial);
        const identity = { ...base, ownerIdentity: this.identities.get(owner), latticeSize: owner.latticeSize ?? null };
        if (owner.disposed || owner.failed || owner.ready === false) return { ...identity, available: false,
            stale: true, status: owner.failed ? 'Failed' : owner.disposed ? 'Disposed' : 'Loading' };
        try {
            if (owner.isFiniteRecord) return { ...identity, ...this.readRecords(owner) };
            if (owner.getTelemetrySnapshot) {
                const snapshot = owner.getTelemetrySnapshot();
                const data = fields(fields(snapshot.groups)?.diagnostics);
                const meta = fields(fields(snapshot.groupMeta)?.diagnostics);
                return { ...identity, ...this.readDiagnostics(data, meta, 'Native', snapshot) };
            }
            if (owner.isWorker && owner.getScale0TelemetryGroupMeta && owner.getDiagnostics) {
                return { ...identity, ...this.readDiagnostics(owner.getDiagnostics(),
                    owner.getScale0TelemetryGroupMeta('diagnostics'), 'Worker') };
            }
            return { ...identity, available: false, stale: true, status: 'Unavailable',
                reason: 'This owner does not expose a completed diagnostic cache.' };
        } catch (error) {
            return { ...identity, available: false, stale: true, status: 'Unavailable',
                reason: error instanceof Error ? error.message : 'Observation unavailable.' };
        }
    }

    /** @param {ObservationOwner} owner @returns {Fields} */
    readRecords(owner) {
        const view = owner.observation;
        const provenance = owner.getProvenance?.();
        const tick = integer(view?.microtick);
        if (!view || tick === null || !provenance?.ownerId || !provenance.law_id
            || view.law_id !== provenance.law_id || integer(provenance.microtick) !== tick) {
            throw new Error('Waiting for a coherent completed record observation.');
        }
        let summary = this.recordSummaries.get(view);
        if (!summary) { summary = summarizeRecords(view); this.recordSummaries.set(view, summary); }
        return { ...summary, available: true, stale: false, status: 'Completed record observation',
            backend: 'Finite records', tick, sampleTick: tick, phase: view.phase,
            // Transport generation may already have advanced before this
            // completed count view is replaced; it is not this sample's epoch.
            epoch: null, sourceId: provenance.ownerId,
            lawId: provenance.law_id, checkpointSha256: provenance.checkpoint_sha256 ?? null,
            canonicalAdoption: provenance.canonical_adoption ?? null,
            provenance: 'Exact counts from the current completed finite-record publication' };
    }

    /** @param {Fields|null} data @param {Fields|null} meta @param {string} backend
     * @param {Fields} [snapshot] @returns {Fields}
     */
    readDiagnostics(data, meta, backend, snapshot = {}) {
        const tick = integer(meta?.sampleTick ?? meta?.tick);
        const sourceEpoch = integer(meta?.sourceEpoch);
        const expectedEpoch = integer(snapshot.sourceEpoch);
        const mismatchedEpoch = expectedEpoch !== null && sourceEpoch !== expectedEpoch;
        const mismatchedTick = data?.tick != null && integer(data.tick) !== tick;
        const stale = !data || !meta || tick === null || sourceEpoch === null || mismatchedEpoch
            || mismatchedTick
            || meta.stale === true || snapshot.stale === true
            || (meta.status != null && meta.status !== 'available');
        const provenance = { backend, tick, sampleTick: tick, sourceEpoch,
            epoch: integer(meta?.epoch), stateVersion: integer(meta?.stateVersion),
            sourceId: snapshot.nativeInstanceId ?? null, stale,
            provenance: 'Read-only completed diagnostic cache; timestamps belong to this sample' };
        if (stale) return { ...provenance, available: false, status: mismatchedEpoch
            ? 'Stale source observation' : 'Waiting for a current completed observation' };
        return { ...provenance, ...diagnosticScalars(data), available: true, status: 'Completed diagnostic observation',
            manifested: integer(data.manifested), positive: integer(data.positive), negative: integer(data.negative) };
    }
}

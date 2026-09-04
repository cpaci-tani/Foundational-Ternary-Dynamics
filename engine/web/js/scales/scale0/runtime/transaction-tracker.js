/**
 * Scale-0 transaction tracker — reconstructs per-record manifestation
 * lifecycles from the native history-journal drain (WASM `drainHistoryEvents`,
 * see engine/wasm/bindings_render_bridge.cpp and
 * engine/include/ftd/eft/history_event_journal.h).
 *
 * WHY THIS EXISTS (P3 / the many-to-one manifestation quotient):
 * The theory reads a manifestation transaction as a four-phase oriented cycle
 * through the null: +1 -> 0-down -> -1 -> 0-up -> +1. The engine's ternary
 * readout `state ∈ {-1,0,+1}` (voxel.h) has only ONE zero — there is no
 * birth-tick field and no null-orientation field on Voxel. The orientation of
 * a given null (which polarity it came from / is heading to) is therefore NOT
 * stored anywhere; it must be RECONSTRUCTED from the event journal's
 * transition history: a null reached from +1 is "0-down", a null reached
 * from -1 is "0-up". This module performs exactly that reconstruction, at
 * two independent granularities:
 *
 *   - SITE-level (`ingestTick` -> `_ingestSiteRow`): the lattice SITE's own
 *     readout history, irrespective of which particle (if any) occupied it.
 *     Movement's vacated source and filled target are each a genuine
 *     site-level death/birth in their own right (the null they create/consume
 *     is physically real), so no event-kind special-casing is needed here —
 *     only the before/after state pair at each touched site matters. This is
 *     what answers "what is the null's orientation" and "how many ticks does
 *     a transaction take" (the minimum transaction latency).
 *
 *   - RECORD-level (`ingestTick` -> `_ingestRecordRow`): a single manifested
 *     entity's birth/relocation/expiry, keyed by the embedded Voxel's
 *     `particle_id` (not by site — a site index is NOT stable under
 *     Movement). Movement's two rows (source vacated, target filled) share
 *     the drained row's `eventId` and the same particle_id, so they are
 *     recognized as ONE relocation rather than a death immediately followed
 *     by an unrelated birth.
 *
 * This module is bridge-agnostic: it only consumes the plain drained-event
 * object shape (parallel typed arrays) that WasmBridge#drainHistoryEvents()
 * returns. It has no DOM, WASM, or Three.js dependency, so its reconstruction
 * logic can be exercised directly from a plain Node/test harness — mirrors
 * the native cross-check in engine/tests/test_history_journal_drain.cpp.
 *
 * Bridge support: the journal (and therefore this tracker) is WASM-only.
 * The `flux-*`/`s0-*` scenarios that run on the JS MockBridge, and the
 * worker-backed WasmBridgeProxy path (which does not yet forward
 * enableHistoryJournal/drainHistoryEvents across the worker boundary), both
 * lack these methods — `isTransactionTrackingSupported()` reports `false` for
 * either, and callers (the tick loop, the panel) must no-op gracefully rather
 * than throw.
 */

/** True only for a bridge that implements the three history-journal WASM exports. */
export function isTransactionTrackingSupported(bridge) {
    return !!bridge
        && typeof bridge.enableHistoryJournal === 'function'
        && typeof bridge.historyJournalEnabled === 'function'
        && typeof bridge.drainHistoryEvents === 'function';
}

/** HistoryEventKind enum values, mirrored from history_event_journal.h. */
export const HISTORY_EVENT_KIND = Object.freeze({
    MOVEMENT: 0,
    GENESIS: 1,
    EVAPORATION: 2,
    PAIR_PRODUCTION: 3,
    ANNIHILATION: 4,
    WEAK_TRANSMUTATION: 5,
});
const KIND_NAMES = Object.freeze(['Movement', 'Genesis', 'Evaporation', 'PairProduction', 'Annihilation', 'WeakTransmutation']);
export function historyEventKindName(kind) { return KIND_NAMES[kind] || `Unknown(${kind})`; }

/** Cap ring-buffer-style history so the tracker cannot leak memory over a long session. */
const MAX_TRACKED_INTERVALS = 4000;
const MAX_TRACKED_LIFETIMES = 4000;
const MAX_TRACKED_CYCLES = 500;
const MAX_CHAIN_PER_SITE = 8;
const MAX_RECORDS = 4000;

function pushCapped(arr, value, cap) {
    arr.push(value);
    if (arr.length > cap) arr.splice(0, arr.length - cap);
}

function median(values) {
    if (!values.length) return null;
    const sorted = [...values].sort((a, b) => a - b);
    const n = sorted.length;
    return (n % 2 === 1) ? sorted[(n - 1) / 2] : 0.5 * (sorted[n / 2 - 1] + sorted[n / 2]);
}

function summarize(values) {
    if (!values.length) return { count: 0, min: null, median: null, max: null };
    return {
        count: values.length,
        min: Math.min(...values),
        median: median(values),
        max: Math.max(...values),
    };
}

/**
 * TransactionTracker owns all cross-tick reconstruction state. One instance
 * is meant to live for the lifetime of a WASM-backed Scale-0 session; call
 * `reset()` on scenario switch so a new lattice does not inherit stale sites.
 */
export class TransactionTracker {
    constructor() {
        this.reset();
    }

    reset() {
        /** @type {Map<number, object>} particle_id -> record */
        this.records = new Map();
        /** @type {Map<number, {tick:number, state:number}>} site -> last death */
        this._lastDeathBySite = new Map();
        /** @type {Map<number, number>} site -> tick of the occupancy currently open */
        this._openBirthBySite = new Map();
        /** @type {Map<number, Array<{tick:number, state:number}>>} site -> recent births, oldest first */
        this._chainBySite = new Map();
        this.nullIntervals = [];       // capped ring buffer of reconstructed nulls
        this.lifetimes = [];           // capped ring buffer of site-occupancy lifetimes (ticks)
        this.completedCycles = [];     // capped ring buffer of {site, fromTick, toTick, length}
        this.eventKindCounts = new Array(KIND_NAMES.length).fill(0);
        this.totalRowsIngested = 0;
        this.totalTicksIngested = 0;
    }

    /**
     * Ingest one tick's drained event object (the exact shape returned by
     * WasmBridge#drainHistoryEvents(), or the EMPTY_HISTORY_EVENTS fallback,
     * which is a harmless no-op since rowCount is 0).
     */
    ingestTick(drained) {
        if (!drained || !drained.rowCount) return;
        this.totalTicksIngested++;
        const n = drained.rowCount;
        const { kind, tick, eventId, site, stateBefore, stateAfter, particleIdBefore, particleIdAfter } = drained;

        let i = 0;
        while (i < n) {
            const id = eventId[i];
            const k = kind[i];
            let j = i;
            while (j < n && eventId[j] === id) j++;
            // rows [i, j) all belong to the same underlying HistoryEvent.
            for (let r = i; r < j; r++) {
                this._ingestSiteRow(tick[r], site[r], stateBefore[r], stateAfter[r]);
                if (k >= 0 && k < this.eventKindCounts.length) this.eventKindCounts[k]++;
            }
            this._ingestRecordRows(k, i, j, drained);
            this.totalRowsIngested += (j - i);
            i = j;
        }
    }

    /** Site-level null reconstruction — see the module doc for the algorithm. */
    _ingestSiteRow(tick, site, stateBefore, stateAfter) {
        const wasNull = stateBefore === 0;
        const isNull = stateAfter === 0;
        if (!wasNull && isNull) {
            this._lastDeathBySite.set(site, { tick, state: stateBefore });
            const birthTick = this._openBirthBySite.get(site);
            if (birthTick !== undefined) {
                pushCapped(this.lifetimes, tick - birthTick, MAX_TRACKED_LIFETIMES);
                this._openBirthBySite.delete(site);
            }
        } else if (wasNull && !isNull) {
            this._openBirthBySite.set(site, tick);
            let chain = this._chainBySite.get(site);
            if (!chain) { chain = []; this._chainBySite.set(site, chain); }
            chain.push({ tick, state: stateAfter });
            if (chain.length > MAX_CHAIN_PER_SITE) chain.shift();
            this._detectCycle(site, chain);

            const death = this._lastDeathBySite.get(site);
            if (death) {
                const orientation = death.state > 0 ? '0-down' : '0-up';
                const cycleConsistent = stateAfter === -death.state;
                pushCapped(this.nullIntervals, {
                    site, fromTick: death.tick, toTick: tick,
                    fromState: death.state, toState: stateAfter,
                    orientation, cycleConsistent,
                }, MAX_TRACKED_INTERVALS);
                this._lastDeathBySite.delete(site);
            }
        }
        // wasNull && isNull cannot occur (a row always records a transition).
        // !wasNull && !isNull is WeakTransmutation's in-place sign flip: the
        // site's readout never actually visited 0, so no null bookkeeping.
    }

    /** Three consecutive alternating-sign births at one site = one complete cycle. */
    _detectCycle(site, chain) {
        const n = chain.length;
        if (n < 3) return;
        const a = chain[n - 3], b = chain[n - 2], c = chain[n - 1];
        if (a.state !== 0 && b.state === -a.state && c.state === a.state) {
            pushCapped(this.completedCycles, {
                site, fromTick: a.tick, toTick: c.tick, length: c.tick - a.tick,
            }, MAX_TRACKED_CYCLES);
        }
    }

    /** Record-level (particle_id-keyed) birth/relocation/expiry bookkeeping. */
    _ingestRecordRows(kind, i, j, drained) {
        const { tick, site, stateBefore, stateAfter, particleIdBefore, particleIdAfter } = drained;
        if (kind === HISTORY_EVENT_KIND.MOVEMENT && j - i === 2) {
            // One row vacates (pidBefore != -1, pidAfter === -1), the other
            // fills (pidBefore === -1, pidAfter !== -1) with the SAME
            // particle_id -- a relocation, not a death+birth.
            let from = -1, to = -1, pid = -1, toSite = -1, fromSite = -1, toTick = tick[i];
            for (let r = i; r < j; r++) {
                if (particleIdBefore[r] !== -1 && particleIdAfter[r] === -1) { from = r; fromSite = site[r]; pid = particleIdBefore[r]; }
                else if (particleIdBefore[r] === -1 && particleIdAfter[r] !== -1) { to = r; toSite = site[r]; if (pid === -1) pid = particleIdAfter[r]; }
            }
            if (from >= 0 && to >= 0 && pid !== -1) {
                const rec = this._recordFor(pid, tick[to], fromSite, stateBefore[from]);
                rec.site = toSite;
                rec.lastTick = tick[to];
                pushCapped(rec.history, { tick: tick[to], site: toSite, kind: 'Movement', from: fromSite }, 32);
                return;
            }
            // Fall through defensively if the pairing didn't resolve (should
            // not happen given the producer contract) -- treat rows generically.
        }
        for (let r = i; r < j; r++) {
            const pidB = particleIdBefore[r], pidA = particleIdAfter[r];
            if (pidB === -1 && pidA !== -1) {
                // Birth: Genesis, PairProduction, or (if not caught above) a
                // Movement arrival whose departure row wasn't in this group.
                const rec = this._recordFor(pidA, tick[r], site[r], stateAfter[r]);
                rec.lastTick = tick[r];
            } else if (pidB !== -1 && pidA === -1) {
                // Expiry: Evaporation, Annihilation, or an unmatched Movement departure.
                const rec = this.records.get(pidB);
                if (rec && rec.expiryTick == null) {
                    rec.expiryTick = tick[r];
                    rec.expirySite = site[r];
                    rec.expiryState = stateBefore[r];
                    rec.expiryKind = historyEventKindName(kind);
                    if (rec.birthTick != null) rec.lifetime = rec.expiryTick - rec.birthTick;
                }
            } else if (pidB !== -1 && pidA === pidB) {
                // In-place continuation (WeakTransmutation sign flip).
                const rec = this.records.get(pidB);
                if (rec) {
                    pushCapped(rec.history, { tick: tick[r], site: site[r], kind: historyEventKindName(kind), stateBefore: stateBefore[r], stateAfter: stateAfter[r] }, 32);
                    rec.lastTick = tick[r];
                }
            }
        }
    }

    _recordFor(pid, birthTick, birthSite, birthState) {
        let rec = this.records.get(pid);
        if (!rec) {
            if (this.records.size >= MAX_RECORDS) {
                // Evict the oldest expired record to bound memory; a live
                // session with a healthy manifestation rate turns records
                // over far faster than this cap.
                for (const [key, value] of this.records) {
                    if (value.expiryTick != null) { this.records.delete(key); break; }
                }
            }
            rec = {
                particleId: pid, birthTick, birthSite, birthState,
                site: birthSite, lastTick: birthTick,
                expiryTick: null, expirySite: null, expiryState: null, expiryKind: null,
                lifetime: null, history: [],
            };
            this.records.set(pid, rec);
        }
        return rec;
    }

    /** Number of records with no observed expiry yet (best-effort; a record
     * present before tracking started never gets a birthTick, but still
     * counts as live once observed). */
    liveRecordCount() {
        let n = 0;
        for (const rec of this.records.values()) if (rec.expiryTick == null) n++;
        return n;
    }

    /** Plain-object telemetry snapshot for the panel and for future hub wiring. */
    snapshotTelemetry() {
        const dwellTicks = this.nullIntervals.map((iv) => iv.toTick - iv.fromTick);
        const cycleLengths = this.completedCycles.map((c) => c.length);
        return {
            ticksIngested: this.totalTicksIngested,
            rowsIngested: this.totalRowsIngested,
            eventCounts: Object.fromEntries(KIND_NAMES.map((name, idx) => [name, this.eventKindCounts[idx]])),
            liveRecordCount: this.liveRecordCount(),
            totalRecordCount: this.records.size,
            lifetime: summarize(this.lifetimes),
            transactionLatency: summarize(dwellTicks),
            cycle: summarize(cycleLengths),
            nullOrientationsReconstructed: this.nullIntervals.length,
        };
    }
}

// One tracker per active Scale-0 controller context. Stored on `ctx` itself
// (not module-global) so a scenario/lattice switch that replaces `ctx`
// naturally drops the old tracker instead of silently mixing sites from two
// different lattices.
export function getOrCreateTransactionTracker(ctx) {
    if (!ctx.__transactionTracker) ctx.__transactionTracker = new TransactionTracker();
    return ctx.__transactionTracker;
}

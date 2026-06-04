/**
 * TimelineBuffer — LOD-tiered ring buffer of lattice snapshots.
 *
 * Each entry is a snapshot object (shape defined in design-spec §1). Two
 * consumers populate this:
 *   - MemoryRecorder (live, rolling window, age-based LOD decay)
 *
 * The buffer is responsibility-light: it holds snapshots, enforces a byte
 * budget (evicting oldest when over), and exposes lookups by tick/age.
 * Decay policy lives in MemoryRecorder (it re-inserts decayed snapshots).
 */

import { blockAverageScalar, blockAverageVec3, snapshotBytes } from './lod.js';

export class TimelineBuffer {
    /**
     * @param {object}  opts
     * @param {number}  opts.budgetBytes  Hard cap; oldest snapshots are evicted when exceeded
     * @param {number}  opts.latticeN     Lattice edge length at capture time
     */
    constructor({ budgetBytes, latticeN }) {
        this.budgetBytes = budgetBytes;
        this.latticeN = latticeN;
        this._snaps = [];      // sorted ascending by tick
        this._bytes = 0;
    }

    /** Append a snapshot. Evicts from the oldest end if the budget is exceeded. */
    push(snap) {
        this._snaps.push(snap);
        this._bytes += snapshotBytes({ lod: snap.lod, N: this.latticeN });
        let dropCount = 0;
        let dropBytes = 0;
        while (this._bytes > this.budgetBytes && this._snaps.length > 1) {
            const drop = this._snaps[dropCount++];
            dropBytes += snapshotBytes({ lod: drop.lod, N: this.latticeN });
            if (this._snaps.length - dropCount <= 1) break;
        }
        if (dropCount) {
            this._snaps.splice(0, dropCount);
            this._bytes -= dropBytes;
        }
    }

    /** Replace a snapshot in place (used by decay passes that lower LOD). */
    replaceAt(index, newSnap) {
        const old = this._snaps[index];
        this._bytes -= snapshotBytes({ lod: old.lod, N: this.latticeN });
        this._bytes += snapshotBytes({ lod: newSnap.lod, N: this.latticeN });
        this._snaps[index] = newSnap;
    }

    /** Number of snapshots currently held. */
    get size() { return this._snaps.length; }

    /** Tick of the most recent snapshot, or -1 if empty. */
    get latestTick() {
        return this._snaps.length ? this._snaps[this._snaps.length - 1].tick : -1;
    }

    /** Tick of the oldest snapshot, or -1 if empty. */
    get oldestTick() {
        return this._snaps.length ? this._snaps[0].tick : -1;
    }

    /** Total bytes currently held. */
    bytesUsed() { return this._bytes; }

    /** Raw snapshot array (read-only view). */
    snapshots() { return this._snaps; }

    /**
     * Return the snapshot with tick ≤ targetTick and the smallest age gap.
     * Returns null if the buffer is empty.
     */
    nearestBefore(targetTick) {
        if (this._snaps.length === 0) return null;
        // Binary search — snapshots are sorted by tick.
        let lo = 0, hi = this._snaps.length - 1, ans = 0;
        while (lo <= hi) {
            const mid = (lo + hi) >>> 1;
            if (this._snaps[mid].tick <= targetTick) {
                ans = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        return this._snaps[ans];
    }

    /**
     * Describe contiguous same-LOD runs of snapshots for the UI's zone display.
     * @returns {Array<{ lod: number, fromTick: number, toTick: number }>}
     */
    asZones() {
        if (!this._snaps.length) return [];
        const zones = [];
        let runStart = 0;
        for (let i = 1; i <= this._snaps.length; i++) {
            if (i === this._snaps.length || this._snaps[i].lod !== this._snaps[runStart].lod) {
                zones.push({
                    lod: this._snaps[runStart].lod,
                    fromTick: this._snaps[runStart].tick,
                    toTick: this._snaps[i - 1].tick,
                });
                runStart = i;
            }
        }
        return zones;
    }

    clear() { this._snaps.length = 0; this._bytes = 0; }
}

/** Build a LOD-k copy of a LOD-0 snapshot. Returns a NEW snapshot object. */
export function degradeSnapshot(snap, targetLod, N) {
    if (targetLod <= snap.lod) return snap;
    if (targetLod >= 3) {
        return { ...snap, lod: 3, lattice: null, flux: null };
    }
    return {
        ...snap,
        lod: targetLod,
        lattice: snap.lattice ? blockAverageScalar(snap.lattice, N, targetLod) : null,
        flux:    snap.flux    ? blockAverageVec3(snap.flux, N, targetLod)    : null,
    };
}

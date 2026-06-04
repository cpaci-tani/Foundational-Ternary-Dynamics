/**
 * MemoryRecorder — live-capture strategy for the playback timeline.
 *
 * Hook: call onTick(scale0Caps) at the end of every Scale 0 tick. Snapshots
 * are pushed at a coarse cadence (controlled by `sampleEveryTicks`), and
 * older snapshots are progressively degraded in place to lower LODs as
 * their age crosses tier boundaries. Result: recent memory is crisp,
 * mid-age is blurry, old is fuzzy, beyond-window is gone.
 *
 * The tier schedule is derived from the user's Memory budget at init. See
 * computeSchedule().
 */

import { TimelineBuffer, degradeSnapshot } from './buffer.js';
import { snapshotBytes } from './lod.js';

export class MemoryRecorder {
    /**
     * @param {object} opts
     * @param {number} opts.budgetBytes    Hard cap (Memory slice, not full user budget)
     * @param {number} opts.latticeN       Lattice edge length
     * @param {Array<{ lod: number, cadenceSeconds: number, durationSeconds: number }>} [opts.tiers]
     *        Tier schedule — newest-first. Default produced by
     *        `computeSchedule(budgetBytes, latticeN)`.
     * @param {number} [opts.ticksPerSecond=60] Used to convert tiers to ticks
     */
    constructor({ budgetBytes, latticeN, tiers, ticksPerSecond = 60 }) {
        this.buffer = new TimelineBuffer({ budgetBytes, latticeN });
        this.latticeN = latticeN;
        this.tiers = tiers || computeSchedule(budgetBytes, latticeN);
        this.tps = ticksPerSecond;
        const lod0 = this.tiers.find(t => t.lod === 0) || { cadenceSeconds: 0.2 };
        this.sampleEveryTicks = Math.max(1, Math.round(lod0.cadenceSeconds * ticksPerSecond));
        this._lastSampledTick = -1;
        this._nextDecayTick = Infinity;
        this._decayBoundaries = buildDecayBoundaries(this.tiers, ticksPerSecond);
    }

    /**
     * Hook called at the end of every sim tick. Takes the Scale 0 capability
     * object and captures + decays as needed.
     */
    onTick(scale0Caps) {
        const tick = this._readTick(scale0Caps);
        if (tick === null) return;
        // Tick regression (scenario reset, Clear Field, engine reseed) — the
        // previously-captured snapshots belong to a different run and their
        // tick numbers will skew the scrub bar's fraction→tick mapping.
        // Wipe the buffer so we re-anchor on the fresh scenario at tick 0.
        if (tick < this._lastSampledTick) {
            this.buffer.clear();
            this._lastSampledTick = -1;
            this._nextDecayTick = Infinity;
        }

        const shouldSample = tick - this._lastSampledTick >= this.sampleEveryTicks;
        const shouldDecay = tick >= this._nextDecayTick;
        if (!shouldSample && !shouldDecay) {
            return;
        }

        if (shouldSample) {
            const snap = scale0Caps?.getScale0Snapshot?.();
            if (!snap) return;
            this.buffer.push(snap);
            this._lastSampledTick = tick;
        }

        this._decayPass(tick);
    }

    _readTick(scale0Caps) {
        const diag = scale0Caps?.getScale0Diagnostics?.();
        if (diag && Number.isFinite(diag.tick)) return diag.tick;
        const snap = scale0Caps?.getScale0Snapshot?.();
        if (!snap || !Number.isFinite(snap.tick)) return null;
        return snap.tick;
    }

    /** Lower-LOD any snapshot whose age has crossed a tier boundary. */
    _decayPass(nowTick) {
        if (Number.isFinite(this._nextDecayTick) && nowTick < this._nextDecayTick) return;
        const snaps = this.buffer.snapshots();
        let nextDecayTick = Infinity;
        for (let i = 0; i < snaps.length; i++) {
            const s = snaps[i];
            const ageTicks = nowTick - s.tick;
            const targetLod = this._lodForAge(ageTicks);
            if (targetLod > s.lod) {
                this.buffer.replaceAt(i, degradeSnapshot(s, targetLod, this.latticeN));
            }
            const next = this._nextBoundaryForSnapshot(snaps[i], nowTick);
            if (next < nextDecayTick) nextDecayTick = next;
        }
        this._nextDecayTick = nextDecayTick;
    }

    /** Map an age (in ticks) to a target LOD using the tier schedule. */
    _lodForAge(ageTicks) {
        const ageSec = ageTicks / this.tps;
        let acc = 0;
        for (const tier of this.tiers) {
            acc += tier.durationSeconds ?? Infinity;
            if (ageSec < acc) return tier.lod;
        }
        return 3; // fell off the end → telemetry-only
    }

    _nextBoundaryForSnapshot(snap, nowTick) {
        for (const boundary of this._decayBoundaries) {
            if (boundary.targetLod <= snap.lod) continue;
            const tick = snap.tick + boundary.ageTicks;
            if (tick > nowTick) return tick;
        }
        return Infinity;
    }

    clear() {
        this.buffer.clear();
        this._lastSampledTick = -1;
        this._nextDecayTick = Infinity;
    }
}

function buildDecayBoundaries(tiers, ticksPerSecond) {
    const out = [];
    let accSeconds = 0;
    for (let i = 0; i < tiers.length; i++) {
        const duration = tiers[i].durationSeconds ?? Infinity;
        if (!Number.isFinite(duration)) break;
        accSeconds += duration;
        out.push({
            ageTicks: Math.max(1, Math.ceil(accSeconds * ticksPerSecond)),
            targetLod: tiers[i + 1]?.lod ?? 3,
        });
    }
    return out;
}

/**
 * Derive a default tier schedule from a byte budget + lattice size.
 *
 * Returns tiers newest-first:
 *   [{ lod: 0, cadenceSeconds, durationSeconds },
 *    { lod: 1, …},
 *    { lod: 2, …}]
 */
export function computeSchedule(budgetBytes, latticeN) {
    const lod0Per = snapshotBytes({ lod: 0, N: latticeN });
    const lod1Per = snapshotBytes({ lod: 1, N: latticeN });
    const lod2Per = snapshotBytes({ lod: 2, N: latticeN });

    const lod0Count = Math.max(5, Math.floor(budgetBytes * 0.5   / lod0Per));
    const lod1Count = Math.max(5, Math.floor(budgetBytes * 0.25  / lod1Per));
    const lod2Count = Math.max(5, Math.floor(budgetBytes * 0.125 / lod2Per));

    const lod0Cadence = 0.2;
    const lod1Cadence = 1.0;
    const lod2Cadence = 3.0;
    return [
        { lod: 0, cadenceSeconds: lod0Cadence, durationSeconds: lod0Count * lod0Cadence },
        { lod: 1, cadenceSeconds: lod1Cadence, durationSeconds: lod1Count * lod1Cadence },
        { lod: 2, cadenceSeconds: lod2Cadence, durationSeconds: lod2Count * lod2Cadence },
    ];
}

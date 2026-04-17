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
    }

    /**
     * Hook called at the end of every sim tick. Takes the Scale 0 capability
     * object and captures + decays as needed.
     */
    onTick(scale0Caps) {
        const snap = scale0Caps?.getScale0Snapshot?.();
        if (!snap) return;
        const tick = snap.tick;
        if (tick - this._lastSampledTick < this.sampleEveryTicks) {
            this._decayPass(tick);
            return;
        }
        this.buffer.push(snap);
        this._lastSampledTick = tick;
        this._decayPass(tick);
    }

    /** Lower-LOD any snapshot whose age has crossed a tier boundary. */
    _decayPass(nowTick) {
        const snaps = this.buffer.snapshots();
        for (let i = 0; i < snaps.length; i++) {
            const s = snaps[i];
            const ageTicks = nowTick - s.tick;
            const targetLod = this._lodForAge(ageTicks);
            if (targetLod > s.lod) {
                this.buffer.replaceAt(i, degradeSnapshot(s, targetLod, this.latticeN));
            }
        }
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

    clear() { this.buffer.clear(); this._lastSampledTick = -1; }
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

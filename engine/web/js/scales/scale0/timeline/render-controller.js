/**
 * RenderController — offline fast-forward into a scrubbable clip.
 *
 * Main-thread fallback: ticks run inside setTimeout(0) slices so the
 * live sim + UI stay responsive. A Web-Worker upgrade is a later
 * optimisation that does not change this event surface.
 *
 * Lifecycle:
 *   start(seconds) → running
 *   cancel()       → idle, buffer discarded
 *   (when done)    → idle, buffer usable
 *
 * Emits CustomEvents: 'start', 'progress', 'done', 'cancel', 'error'.
 */

import { TimelineBuffer, degradeSnapshot } from './buffer.js';
import { snapshotBytes } from './lod.js';

const TICKS_PER_SEC = 60;
const COMPUTE_SLICE_MS = 12;          // soft cap per idle slice so UI stays responsive
const DEFAULT_SAMPLE_EVERY_TICKS = 4;  // 15 fps @ 60 TPS — smooth enough to scrub

/**
 * Pick the coarsest LOD that still lets us fit `seconds` of clip at
 * `sampleEveryTicks` cadence into the budget. Prefers LOD 0 when it fits,
 * falls back to LOD 1, then LOD 2.
 */
function chooseRenderLod(seconds, sampleEveryTicks, latticeN, budgetBytes) {
    const totalSamples = Math.ceil((seconds * TICKS_PER_SEC) / sampleEveryTicks);
    for (const lod of [0, 1, 2]) {
        const perSnap = snapshotBytes({ lod, N: latticeN });
        if (totalSamples * perSnap <= budgetBytes) return lod;
    }
    return 2;
}

export class RenderController extends EventTarget {
    constructor({ budgetBytes, latticeN, scale0Caps, sampleEveryTicks }) {
        super();
        this.budgetBytes = budgetBytes;
        this.latticeN = latticeN;
        this.caps = scale0Caps;
        this.sampleEveryTicks = Math.max(1, sampleEveryTicks || DEFAULT_SAMPLE_EVERY_TICKS);
        this.renderLod = 0;
        this.buffer = null;
        this.running = false;
        this.progress = 0;
        this._cancelled = false;
    }

    start(seconds) {
        if (this.running) return;
        const totalTicks = Math.round(seconds * TICKS_PER_SEC);
        if (totalTicks <= 0) return;
        const originalSnap = this.caps?.getScale0Snapshot?.();
        if (!originalSnap) {
            this._emit('error', { reason: 'snapshot-unsupported' });
            return;
        }
        this.renderLod = chooseRenderLod(seconds, this.sampleEveryTicks, this.latticeN, this.budgetBytes);
        this.buffer = new TimelineBuffer({ budgetBytes: this.budgetBytes, latticeN: this.latticeN });
        this.running = true;
        this._cancelled = false;
        this.progress = 0;
        this._emit('start', { totalTicks, lod: this.renderLod, sampleEveryTicks: this.sampleEveryTicks });
        // Seed the clip with the current state so t=0 has a frame to show.
        this._capture(originalSnap);
        this._slice(originalSnap, totalTicks, 0);
    }

    cancel() {
        if (!this.running) return;
        this._cancelled = true;
    }

    _capture(snap) {
        if (!snap) return;
        const out = (this.renderLod === 0)
            ? snap
            : degradeSnapshot(snap, this.renderLod, this.latticeN);
        this.buffer.push(out);
    }

    _slice(originalSnap, totalTicks, doneTicks) {
        if (this._cancelled) {
            this.caps.loadScale0Snapshot?.(originalSnap);
            this.buffer = null;
            this.running = false;
            this._emit('cancel', {});
            return;
        }
        if (doneTicks >= totalTicks) {
            this.caps.loadScale0Snapshot?.(originalSnap);
            this.running = false;
            this.progress = 1;
            this._emit('done', { snapshots: this.buffer.size });
            return;
        }

        // Run as many ticks as fit in the slice budget, sampling on
        // `sampleEveryTicks` cadence so the render buffer has a frame
        // every ~4 ticks — smooth enough for forward + backward scrub.
        const t0 = performance.now();
        let ran = 0;
        const remaining = totalTicks - doneTicks;
        while (ran < remaining && (performance.now() - t0) < COMPUTE_SLICE_MS) {
            this.caps.tickScale0();
            ran++;
            if (((doneTicks + ran) % this.sampleEveryTicks) === 0) {
                const snap = this.caps.getScale0Snapshot?.();
                if (snap) this._capture(snap);
            }
        }

        this.progress = (doneTicks + ran) / totalTicks;
        this._emit('progress', { progress: this.progress });
        setTimeout(() => this._slice(originalSnap, totalTicks, doneTicks + ran), 0);
    }

    _emit(name, detail) { this.dispatchEvent(new CustomEvent(name, { detail })); }
}

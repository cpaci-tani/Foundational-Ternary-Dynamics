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

import { TimelineBuffer } from './buffer.js';

const TICKS_PER_SEC = 60;
const TICKS_PER_SLICE = 60; // ~1s of ticks per idle slice

export class RenderController extends EventTarget {
    constructor({ budgetBytes, latticeN, scale0Caps }) {
        super();
        this.budgetBytes = budgetBytes;
        this.latticeN = latticeN;
        this.caps = scale0Caps;
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
        this.buffer = new TimelineBuffer({ budgetBytes: this.budgetBytes, latticeN: this.latticeN });
        this.running = true;
        this._cancelled = false;
        this.progress = 0;
        this._emit('start', { totalTicks });
        this._slice(originalSnap, totalTicks, 0);
    }

    cancel() {
        if (!this.running) return;
        this._cancelled = true;
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
        const toRun = Math.min(TICKS_PER_SLICE, totalTicks - doneTicks);
        for (let i = 0; i < toRun; i++) {
            this.caps.tickScale0();
        }
        const snap = this.caps.getScale0Snapshot?.();
        if (snap) this.buffer.push(snap);
        this.progress = (doneTicks + toRun) / totalTicks;
        this._emit('progress', { progress: this.progress });
        setTimeout(() => this._slice(originalSnap, totalTicks, doneTicks + toRun), 0);
    }

    _emit(name, detail) { this.dispatchEvent(new CustomEvent(name, { detail })); }
}

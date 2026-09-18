/**
 * Legacy Scale-0 time-series chart handles — ring-buffer holders.
 *
 * Two handle types:
 *   - FluxEnergyChart: total flux + total energy buffers
 *   - ParticleChart: total / positive / negative buffers
 *
 * Ring buffers are owned by TelemetryHub and injected at construction time;
 * these classes never push data themselves.
 *
 * The Canvas 2D renderer these classes once carried (drawChart + its axis
 * formatter and cached-rect helper) was removed after the panels redesign:
 * Scale-0 charts and the Lagrangian panel own their own uPlot instances via
 * ChartsPanelComponent / LagrangianPanelComponent, and nothing in the app
 * called draw() or push() any more. The handles themselves are retained
 * because app.js still constructs them and hands them to the scale contexts.
 */

import { RingBuffer } from './telemetry-hub.js';

// ── Exported Chart Classes ───────────────────────────────────────────

export class FluxEnergyChart {
    /**
     * @param {HTMLCanvasElement} canvas
     * @param {{ fluxBuf?: RingBuffer, energyBuf?: RingBuffer }} [buffers]
     *   Pass hub buffers to share ownership; falls back to local buffers for
     *   standalone use or tests.
     */
    constructor(canvas, buffers = {}) {
        this.canvas    = canvas;
        this.fluxBuf   = buffers.fluxBuf   || new RingBuffer();
        this.energyBuf = buffers.energyBuf || new RingBuffer();
    }

    clear() {
        this.fluxBuf.clear();
        this.energyBuf.clear();
    }
}

export class ParticleChart {
    /**
     * @param {HTMLCanvasElement} canvas
     * @param {{ totalBuf?: RingBuffer, posBuf?: RingBuffer, negBuf?: RingBuffer }} [buffers]
     */
    constructor(canvas, buffers = {}) {
        this.canvas   = canvas;
        this.totalBuf = buffers.totalBuf || new RingBuffer();
        this.posBuf   = buffers.posBuf   || new RingBuffer();
        this.negBuf   = buffers.negBuf   || new RingBuffer();
    }

    clear() {
        this.totalBuf.clear();
        this.posBuf.clear();
        this.negBuf.clear();
    }
}

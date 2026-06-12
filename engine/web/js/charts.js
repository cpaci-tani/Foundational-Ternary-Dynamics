/**
 * Canvas 2D Time-Series Charts — ring-buffered, auto-scaling.
 *
 * Two chart types:
 *   - FluxEnergyChart: total flux (orange) + total energy (blue)
 *   - ParticleChart: total (white), positive (green), negative (red)
 *
 * Ring buffers are owned by TelemetryHub and injected at construction time.
 * Chart classes are pure renderers; they never push data themselves.
 */

import { createCachedCanvasRect } from './dom-utils.js';
import { RingBuffer } from './telemetry-hub.js';

// Phase C.3: per-canvas cached rect (ResizeObserver-backed). Avoids
// forcing a layout reflow on every frame's drawChart() call.
const _rectCaches = new WeakMap();
function _cachedRect(canvas) {
    let c = _rectCaches.get(canvas);
    if (!c) {
        c = createCachedCanvasRect(canvas);
        _rectCaches.set(canvas, c);
    }
    return c.get();
}

// ── Chart Renderer ───────────────────────────────────────────────────
function drawChart(canvas, series) {
    if (!canvas) return;
    const rect = _cachedRect(canvas);
    const w = rect.width;
    const h = rect.height;
    // PERF: Skip drawing when canvas is hidden (zero-size or offscreen)
    if (w === 0 || h === 0) return;

    const ctx = canvas.getContext('2d');
    const dpr = window.devicePixelRatio || 1;

    // Resize canvas if needed
    if (canvas.width !== w * dpr || canvas.height !== h * dpr) {
        canvas.width = w * dpr;
        canvas.height = h * dpr;
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    }

    // Clear
    ctx.clearRect(0, 0, w, h);

    if (series.length === 0 || series[0].buffer.count < 2) return;

    // Narrow (phone) canvases: trim the left axis gutter and use a slightly
    // larger, more legible label font. Keeps labels readable without eating the
    // already-tight plot width on a ~340px-wide mobile chart.
    const narrow = w < 420;
    const axisFontPx = narrow ? 12 : 10;
    // Compute global range
    const margin = { top: 8, right: 8, bottom: narrow ? 22 : 20, left: narrow ? 36 : 50 };
    const plotW = w - margin.left - margin.right;
    const plotH = h - margin.top - margin.bottom;

    let yMin = Infinity, yMax = -Infinity;
    for (const s of series) {
        const mn = s.buffer.min();
        const mx = s.buffer.max();
        if (mn < yMin) yMin = mn;
        if (mx > yMax) yMax = mx;
    }
    // Padding
    const yRange = yMax - yMin || 1;
    yMin -= yRange * 0.05;
    yMax += yRange * 0.05;

    const count = series[0].buffer.count;

    // Grid lines
    ctx.strokeStyle = '#2a3a5a';
    ctx.lineWidth = 0.5;
    const gridLines = 4;
    for (let i = 0; i <= gridLines; i++) {
        const y = margin.top + (plotH * i) / gridLines;
        ctx.beginPath();
        ctx.moveTo(margin.left, y);
        ctx.lineTo(w - margin.right, y);
        ctx.stroke();

        // Y-axis labels
        const val = yMax - (i / gridLines) * (yMax - yMin);
        ctx.fillStyle = '#6b7280';
        ctx.font = axisFontPx + 'px JetBrains Mono, monospace';
        ctx.textAlign = 'right';
        ctx.fillText(formatValue(val), margin.left - 6, y + 4);
    }

    // Draw series
    for (const s of series) {
        ctx.beginPath();
        ctx.strokeStyle = s.color;
        ctx.lineWidth = 1.5;
        for (let i = 0; i < count; i++) {
            const x = margin.left + (i / (count - 1)) * plotW;
            const v = s.buffer.get(i);
            const y = margin.top + plotH - ((v - yMin) / (yMax - yMin)) * plotH;
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        }
        ctx.stroke();
    }

    // X-axis label
    ctx.fillStyle = '#6b7280';
    ctx.font = axisFontPx + 'px Inter, sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText(`${count} ticks`, w / 2, h - 2);
}

function formatValue(v) {
    if (Math.abs(v) >= 10000) return v.toExponential(1);
    if (Math.abs(v) >= 100) return v.toFixed(0);
    if (Math.abs(v) >= 1) return v.toFixed(1);
    return v.toFixed(3);
}

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

    /** @deprecated Push via telemetryHub.collectScale0() instead. */
    push(diag) {
        this.fluxBuf.push(diag.totalFlux);
        this.energyBuf.push(diag.totalEnergy);
    }

    draw() {
        drawChart(this.canvas, [
            { buffer: this.fluxBuf,   color: '#fb8c00' },
            { buffer: this.energyBuf, color: '#42a5f5' },
        ]);
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

    /** @deprecated Push via telemetryHub.collectScale0() instead. */
    push(diag) {
        this.totalBuf.push(diag.manifested);
        this.posBuf.push(diag.positive);
        this.negBuf.push(diag.negative);
    }

    draw() {
        drawChart(this.canvas, [
            { buffer: this.totalBuf, color: '#e8e8e8' },
            { buffer: this.posBuf,   color: '#4ade80' },
            { buffer: this.negBuf,   color: '#f87171' },
        ]);
    }

    clear() {
        this.totalBuf.clear();
        this.posBuf.clear();
        this.negBuf.clear();
    }
}

/**
 * Canvas 2D Time-Series Charts — ring-buffered, auto-scaling.
 *
 * Two chart types:
 *   - FluxEnergyChart: total flux (orange) + total energy (blue)
 *   - ParticleChart: total (white), positive (green), negative (red)
 */

import { createCachedCanvasRect } from './dom-utils.js';

const BUFFER_SIZE = 500;

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

// ── Ring Buffer ──────────────────────────────────────────────────────
class RingBuffer {
    constructor(size = BUFFER_SIZE) {
        this.data = new Float32Array(size);
        this.size = size;
        this.head = 0;
        this.count = 0;
    }

    push(value) {
        this.data[this.head] = value;
        this.head = (this.head + 1) % this.size;
        if (this.count < this.size) this.count++;
    }

    get(i) {
        if (i >= this.count) return 0;
        const idx = (this.head - this.count + i + this.size) % this.size;
        return this.data[idx];
    }

    last() {
        if (this.count === 0) return 0;
        return this.data[(this.head - 1 + this.size) % this.size];
    }

    max() {
        let m = -Infinity;
        for (let i = 0; i < this.count; i++) {
            const v = this.get(i);
            if (v > m) m = v;
        }
        return m === -Infinity ? 1 : m;
    }

    min() {
        let m = Infinity;
        for (let i = 0; i < this.count; i++) {
            const v = this.get(i);
            if (v < m) m = v;
        }
        return m === Infinity ? 0 : m;
    }

    clear() {
        this.head = 0;
        this.count = 0;
    }
}

// ── Chart Renderer ───────────────────────────────────────────────────
function drawChart(canvas, series, options = {}) {
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

    // Compute global range
    const margin = { top: 8, right: 8, bottom: 20, left: 50 };
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
        ctx.font = '10px JetBrains Mono, monospace';
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
    ctx.font = '10px Inter, sans-serif';
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
    constructor(canvas) {
        this.canvas = canvas;
        this.fluxBuf = new RingBuffer();
        this.energyBuf = new RingBuffer();
    }

    push(diag) {
        this.fluxBuf.push(diag.totalFlux);
        this.energyBuf.push(diag.totalEnergy);
    }

    draw() {
        drawChart(this.canvas, [
            { buffer: this.fluxBuf, color: '#fb8c00' },
            { buffer: this.energyBuf, color: '#42a5f5' },
        ]);
    }

    clear() {
        this.fluxBuf.clear();
        this.energyBuf.clear();
    }
}

export class ParticleChart {
    constructor(canvas) {
        this.canvas = canvas;
        this.totalBuf = new RingBuffer();
        this.posBuf = new RingBuffer();
        this.negBuf = new RingBuffer();
    }

    push(diag) {
        this.totalBuf.push(diag.manifested);
        this.posBuf.push(diag.positive);
        this.negBuf.push(diag.negative);
    }

    draw() {
        drawChart(this.canvas, [
            { buffer: this.totalBuf, color: '#e8e8e8' },
            { buffer: this.posBuf, color: '#4ade80' },
            { buffer: this.negBuf, color: '#f87171' },
        ]);
    }

    clear() {
        this.totalBuf.clear();
        this.posBuf.clear();
        this.negBuf.clear();
    }
}

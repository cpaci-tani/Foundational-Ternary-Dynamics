/**
 * CanvasSparkline — lightweight canvas mini-chart for legacy PE telemetry rows.
 * Prefer ui/charts/sparkline.js (uPlot + hub RingBuffer) for new panel work.
 */

import { createCachedCanvasRect } from '../../dom-utils.js';

const SPARKLINE_LEN = 80;

export class CanvasSparkline {
    constructor(canvas) {
        this.canvas = canvas;
        this._buf = new Float32Array(SPARKLINE_LEN);
        this._head = 0;
        this._count = 0;
        this._rectCache = canvas ? createCachedCanvasRect(canvas) : null;
    }

    push(value) {
        this._buf[this._head] = value;
        this._head = (this._head + 1) % SPARKLINE_LEN;
        if (this._count < SPARKLINE_LEN) this._count++;
    }

    _get(i) {
        return this._buf[(this._head - this._count + i + SPARKLINE_LEN) % SPARKLINE_LEN];
    }

    draw(color = '#60a5fa') {
        const canvas = this.canvas;
        if (!canvas) return;
        const ctx = canvas.getContext('2d');
        const dpr = window.devicePixelRatio || 1;
        const rect = this._rectCache ? this._rectCache.get() : canvas.getBoundingClientRect();
        const w = rect.width;
        const h = rect.height;

        if (w === 0 || h === 0) return;

        if (canvas.width !== w * dpr || canvas.height !== h * dpr) {
            canvas.width = w * dpr;
            canvas.height = h * dpr;
            ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        }

        ctx.clearRect(0, 0, w, h);

        const n = this._count;
        if (n < 2) return;

        let min = Infinity;
        let max = -Infinity;
        for (let i = 0; i < n; i++) {
            const v = this._get(i);
            if (v < min) min = v;
            if (v > max) max = v;
        }
        const range = max - min || 1;

        ctx.beginPath();
        ctx.moveTo(0, h);
        for (let i = 0; i < n; i++) {
            const x = (i / (n - 1)) * w;
            const y = h - ((this._get(i) - min) / range) * (h - 2) - 1;
            ctx.lineTo(x, y);
        }
        ctx.lineTo(w, h);
        ctx.closePath();
        ctx.fillStyle = color + '15';
        ctx.fill();

        ctx.beginPath();
        for (let i = 0; i < n; i++) {
            const x = (i / (n - 1)) * w;
            const y = h - ((this._get(i) - min) / range) * (h - 2) - 1;
            if (i === 0) ctx.moveTo(x, y);
            else ctx.lineTo(x, y);
        }
        ctx.strokeStyle = color;
        ctx.lineWidth = 1.2;
        ctx.stroke();
    }

    clear() {
        this._head = 0;
        this._count = 0;
        this.draw();
    }
}

/** @deprecated Use CanvasSparkline — kept for one release of import stability. */
export const Sparkline = CanvasSparkline;

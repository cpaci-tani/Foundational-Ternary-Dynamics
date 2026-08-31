/**
 * Sparkline — micro uPlot for table Trend cells and chart-chip previews.
 * No axes, no legend, no cursor, no title. ~24px tall by default.
 *
 *   new Sparkline(container, { buffer, color, height?, visibleSamples? });
 *   spark.update();
 *   spark.destroy();
 */

import { resolveChartColor } from './theme.js';

export class Sparkline {
    constructor(container, opts) {
        this.container = container;
        this.buffer    = opts.buffer;
        this.color     = resolveChartColor(opts.color || 'var(--accent, #6366f1)');
        this.height    = opts.height || 24;
        this.visibleSamples = Math.max(2, opts.visibleSamples || this.buffer?.size || 80);
        this._destroyed = false;
        this._resizeFrame = 0;
        this._lastWidth = Math.max(1, Math.round(container.clientWidth || 80));

        const size = Math.min(this.buffer?.size || 80, this.visibleSamples);
        this.xs = new Float64Array(size);
        this.ys = new Float64Array(size);

        const uopts = {
            width:  this._lastWidth,
            height: this.height,
            padding: [2, 2, 2, 2],
            scales: { x: { time: false } },
            axes:   [{ show: false }, { show: false }],
            legend: { show: false },
            cursor: { show: false, x: false, y: false, drag: { x: false, y: false } },
            series: [
                {},
                { stroke: this.color, width: 1.25, points: { show: false } },
            ],
        };

        // eslint-disable-next-line no-undef
        this.uplot = new uPlot(uopts, [new Float64Array(0), new Float64Array(0)], container);

        this._ro = new ResizeObserver(() => this._scheduleResize());
        this._ro.observe(container);
    }

    _scheduleResize() {
        if (this._destroyed || this._resizeFrame) return;
        this._resizeFrame = requestAnimationFrame(() => {
            this._resizeFrame = 0;
            if (this._destroyed) return;
            const width = Math.round(this.container.clientWidth);
            if (width <= 0 || width === this._lastWidth) return;
            this._lastWidth = width;
            this.uplot.setSize({ width, height: this.height });
        });
    }

    update() {
        if (this._destroyed || !this.buffer) return;
        const n = Math.min(this.buffer.count, this.visibleSamples);
        if (n < 2) {
            this.uplot.setData([new Float64Array(0), new Float64Array(0)], true);
            return;
        }
        const xs = this.xs.subarray(0, n);
        const ys = this.ys.subarray(0, n);
        const start = this.buffer.count - n;
        for (let i = 0; i < n; i++) { xs[i] = i; ys[i] = this.buffer.get(start + i); }
        this.uplot.setData([xs, ys], true);
    }

    destroy() {
        if (this._destroyed) return;
        this._destroyed = true;
        if (this._resizeFrame) cancelAnimationFrame(this._resizeFrame);
        this._resizeFrame = 0;
        this._ro.disconnect();
        this.uplot.destroy();
        this.uplot = null;
    }
}

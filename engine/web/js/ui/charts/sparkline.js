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

        const size = Math.min(this.buffer?.size || 80, this.visibleSamples);
        this.xs = new Float64Array(size);
        this.ys = new Float64Array(size);

        const uopts = {
            width:  container.clientWidth || 80,
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

        this._ro = new ResizeObserver(() => {
            if (this._destroyed) return;
            const w = container.clientWidth;
            if (w > 0) this.uplot.setSize({ width: w, height: this.height });
        });
        this._ro.observe(container);
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
        this._ro.disconnect();
        this.uplot.destroy();
        this.uplot = null;
    }
}

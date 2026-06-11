/**
 * UPlotChart — line/area chart primitive.
 *
 * Each instance owns a uPlot chart, a ResizeObserver, preallocated Float64Array
 * data buffers, and localStorage-backed series-visibility state.
 *
 *   new UPlotChart(container, {
 *       id,                        // unique, used for localStorage key
 *       title,
 *       series,                    // [{ key, label, color, buffer }]
 *       xLabel, yLabel,
 *       hub,                       // telemetryHub instance
 *   });
 *
 *   chart.update();                // called every animate frame
 *   chart.destroy();               // frees uPlot + ResizeObserver
 */

import { getChartTheme, makeAxis, resolveChartColor } from './theme.js';

const LS_PREFIX = 'ftd.chart.';

export class UPlotChart {
    constructor(container, opts) {
        this.container = container;
        this.opts      = opts;
        this.id        = opts.id;
        this.hub       = opts.hub;
        this.series    = opts.series;
        this._destroyed = false;

        const theme = getChartTheme();
        const bufSize = this._maxBufferSize();
        this.xs = new Float64Array(bufSize);
        this.ys = this.series.map(() => new Float64Array(bufSize));

        const hiddenKeys = this._loadHiddenKeys();

        const uopts = {
            width:  container.clientWidth  || 320,
            height: container.clientHeight || 180,
            title:  opts.title,
            scales: { x: { time: false } },
            axes: [
                makeAxis(theme, { label: opts.xLabel, scale: 'x', side: 2 }),
                makeAxis(theme, { label: opts.yLabel, scale: 'y', side: 3 }),
            ],
            series: [
                { label: opts.xLabel || 'x' },
                ...this.series.map((s) => ({
                    label:  s.label,
                    stroke: resolveChartColor(s.color),
                    width:  1.5,
                    show:   !hiddenKeys.has(s.key),
                })),
            ],
            legend: { live: true },
            hooks: {
                setSeries: [
                    (u, i) => {
                        if (i >= 1) this._saveHiddenKeys();
                    },
                ],
            },
        };

        // eslint-disable-next-line no-undef
        this.uplot = new uPlot(uopts, this._emptyData(), container);

        this._ro = new ResizeObserver(() => this._onResize());
        this._ro.observe(container);
        container._ftdResize = () => this._onResize();
    }

    _maxBufferSize() {
        return Math.max(...this.series.map((s) => this.hub[s.buffer]?.size || 500));
    }

    _emptyData() {
        return [new Float64Array(0), ...this.series.map(() => new Float64Array(0))];
    }

    _loadHiddenKeys() {
        try {
            const raw = localStorage.getItem(LS_PREFIX + this.id + '.hidden');
            return new Set(raw ? JSON.parse(raw) : []);
        } catch {
            return new Set();
        }
    }

    _saveHiddenKeys() {
        const hidden = [];
        for (let i = 0; i < this.series.length; i++) {
            if (this.uplot.series[i + 1].show === false) hidden.push(this.series[i].key);
        }
        try {
            localStorage.setItem(LS_PREFIX + this.id + '.hidden', JSON.stringify(hidden));
        } catch {
            /* storage quota — ignore */
        }
    }

    _onResize() {
        if (this._destroyed) return;
        const w = this.container.clientWidth;
        const h = this.container.clientHeight;
        if (w > 0 && h > 0) this.uplot.setSize({ width: w, height: h });
    }

    update() {
        if (this._destroyed) return;
        const firstBuf = this.hub[this.series[0].buffer];
        const n = firstBuf?.count || 0;
        if (n < 2) {
            this.uplot.setData(this._emptyData(), true);
            return;
        }

        const xs = this.xs.subarray(0, n);
        for (let i = 0; i < n; i++) xs[i] = i;

        const yColumns = this.series.map((s, idx) => {
            const buf = this.hub[s.buffer];
            const col = this.ys[idx].subarray(0, n);
            for (let i = 0; i < n; i++) col[i] = buf.get(i);
            return col;
        });

        // `true` so uPlot recomputes scale min/max each frame — with ring
        // buffers the range changes constantly and skipping this leaves the
        // chart blank.
        this.uplot.setData([xs, ...yColumns], true);
    }

    destroy() {
        if (this._destroyed) return;
        this._destroyed = true;
        this._ro.disconnect();
        this.uplot.destroy();
        this.uplot = null;
    }
}

/**
 * Stacked-area renderer for uPlot.
 *
 * uPlot doesn't ship stacked-area; this wraps the standard line chart by
 * building cumulative-sum columns in setData. Each series becomes a filled
 * band between its predecessor's cumulative value and its own.
 *
 *   new StackedAreaChart(container, {
 *       id, title,
 *       series,          // [{ key, label, color, buffer }]
 *       xLabel, yLabel,
 *       hub,
 *   });
 *
 *   chart.update();
 *   chart.destroy();
 */

import { getChartTheme, makeAxis } from './theme.js';

const LS_PREFIX = 'ftd.chart.';

export class StackedAreaChart {
    constructor(container, opts) {
        this.container = container;
        this.opts      = opts;
        this.id        = opts.id;
        this.hub       = opts.hub;
        this.series    = opts.series;
        this._destroyed = false;

        const theme  = getChartTheme();
        const bufSize = Math.max(...this.series.map((s) => this.hub.lag[s.buffer]?.size || 400));
        this.xs = new Float64Array(bufSize);
        this.ys = this.series.map(() => new Float64Array(bufSize));
        this._cumul = new Float64Array(bufSize);

        const hiddenKeys = this._loadHiddenKeys();

        const uopts = {
            width:  container.clientWidth  || 400,
            height: container.clientHeight || 220,
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
                    stroke: s.color,
                    fill:   s.color + '55',
                    width:  1,
                    show:   !hiddenKeys.has(s.key),
                })),
            ],
            legend: { live: true },
            hooks: {
                setSeries: [
                    (u, i) => { if (i >= 1) this._saveHiddenKeys(); },
                ],
            },
        };

        // eslint-disable-next-line no-undef
        this.uplot = new uPlot(uopts, this._emptyData(), container);

        this._ro = new ResizeObserver(() => {
            if (this._destroyed) return;
            const w = container.clientWidth;
            const h = container.clientHeight;
            if (w > 0 && h > 0) this.uplot.setSize({ width: w, height: h });
        });
        this._ro.observe(container);
    }

    _emptyData() {
        return [new Float64Array(0), ...this.series.map(() => new Float64Array(0))];
    }

    _loadHiddenKeys() {
        try {
            const raw = localStorage.getItem(LS_PREFIX + this.id + '.hidden');
            return new Set(raw ? JSON.parse(raw) : []);
        } catch { return new Set(); }
    }

    _saveHiddenKeys() {
        const hidden = [];
        for (let i = 0; i < this.series.length; i++) {
            if (this.uplot.series[i + 1].show === false) hidden.push(this.series[i].key);
        }
        try { localStorage.setItem(LS_PREFIX + this.id + '.hidden', JSON.stringify(hidden)); }
        catch {}
    }

    update() {
        if (this._destroyed) return;
        const firstBuf = this.hub.lag[this.series[0].buffer];
        const n = firstBuf?.count || 0;
        if (n < 2) return;

        const xs = this.xs.subarray(0, n);
        for (let i = 0; i < n; i++) xs[i] = i;

        const cum = this._cumul.subarray(0, n);
        cum.fill(0);

        const yColumns = [];
        for (let si = 0; si < this.series.length; si++) {
            const s = this.series[si];
            const buf = this.hub.lag[s.buffer];
            const col = this.ys[si].subarray(0, n);
            const visible = this.uplot.series[si + 1].show !== false;
            for (let i = 0; i < n; i++) {
                cum[i] += visible ? Math.abs(buf.get(i)) : 0;
                col[i] = cum[i];
            }
            yColumns.push(col.slice());
        }

        // Pass `true` so uPlot recomputes scale min/max on each update; with
        // a streaming ring buffer the range changes constantly and skipping
        // the recompute leaves the chart blank.
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

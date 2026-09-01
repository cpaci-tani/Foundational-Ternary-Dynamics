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
import { ChartHoverTooltip, formatChartValue } from './chart-hover-tooltip.js';

const LS_PREFIX = 'ftd.chart.';
const DEFAULT_VISIBLE_SAMPLES = 160;

export class UPlotChart {
    constructor(container, opts) {
        this.container = container;
        this.opts      = opts;
        this.id        = opts.id;
        this.hub       = opts.hub;
        this.series    = opts.series;
        this.historyControl = opts.historyControl || null;
        this.visibleSamples = Math.max(2, opts.visibleSamples || DEFAULT_VISIBLE_SAMPLES);
        this._destroyed = false;
        this._hoverActive = false;
        this._resizeFrame = 0;
        this._lastWidth = Math.max(1, Math.round(container.clientWidth || 320));
        this._lastHeight = Math.max(1, Math.round(container.clientHeight || 180));
        this._lastData = null;
        this._bufferStamps = this.series.map(() => ({
            buffer: null, total: -1, count: -1, last: Number.NaN,
        }));
        this._emptyPublished = true;
        this._historyDirty = false;
        this._unsubscribeHistory = this.historyControl?.subscribe?.(() => {
            this._historyDirty = true;
            for (const stamp of this._bufferStamps) stamp.total = -1;
            this.update();
        }) || null;

        const theme = getChartTheme();
        const bufSize = this._maxBufferSize();
        this.xs = new Float64Array(bufSize);
        this.ys = this.series.map(() => new Float64Array(bufSize));

        const hiddenKeys = this._loadHiddenKeys();

        const uopts = {
            width:  this._lastWidth,
            height: this._lastHeight,
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
                setCursor: [
                    () => this._renderHoverTooltip(),
                ],
            },
        };

        // eslint-disable-next-line no-undef
        this.uplot = new uPlot(uopts, this._emptyData(), container);
        this.tooltip = new ChartHoverTooltip(container);
        this._hoverTarget = this.uplot.over || container;
        this._onPointerEnter = () => {
            this._hoverActive = true;
            this._renderHoverTooltip();
        };
        this._onPointerLeave = () => {
            this._hoverActive = false;
            this.tooltip.hide();
        };
        this._hoverTarget.addEventListener('pointerenter', this._onPointerEnter);
        this._hoverTarget.addEventListener('pointerleave', this._onPointerLeave);
        this._hoverTarget.addEventListener('mouseenter', this._onPointerEnter);
        this._hoverTarget.addEventListener('mouseleave', this._onPointerLeave);

        this._ro = new ResizeObserver(() => this._scheduleResize());
        this._ro.observe(container);
        container._ftdResize = () => this._scheduleResize();
    }

    _maxBufferSize() {
        const maxBuffer = Math.max(...this.series.map((s) => this.hub[s.buffer]?.size || 500));
        return this.historyControl?.isAll ? Math.max(2, Math.min(maxBuffer, 512))
            : Math.max(2, Math.min(maxBuffer, this.historyControl?.ticks || this.visibleSamples));
    }

    _ensureCapacity(size) {
        if (this.xs.length >= size) return;
        let capacity = Math.max(2, this.xs.length || 2);
        while (capacity < size) capacity *= 2;
        this.xs = new Float64Array(capacity);
        this.ys = this.series.map(() => new Float64Array(capacity));
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

    _scheduleResize() {
        if (this._destroyed || this._resizeFrame) return;
        this._resizeFrame = requestAnimationFrame(() => {
            this._resizeFrame = 0;
            if (this._destroyed) return;
            const width = Math.round(this.container.clientWidth);
            const height = Math.round(this.container.clientHeight);
            if (width <= 0 || height <= 0
                || (width === this._lastWidth && height === this._lastHeight)) return;
            this._lastWidth = width;
            this._lastHeight = height;
            this.uplot.setSize({ width, height });
        });
    }

    update() {
        if (this._destroyed) return;
        const firstBuf = this.hub[this.series[0].buffer];
        const n = this.historyControl
            ? this.historyControl.visibleCount(firstBuf)
            : Math.min(firstBuf?.count || 0, this.visibleSamples);
        if (n < 2) {
            this._lastData = null;
            if (!this._emptyPublished) {
                this.uplot.setData(this._emptyData(), true);
                this._emptyPublished = true;
            }
            for (const stamp of this._bufferStamps) {
                stamp.buffer = null; stamp.total = -1; stamp.count = -1; stamp.last = Number.NaN;
            }
            if (this._hoverActive) this.tooltip.hide();
            return;
        }

        let dirty = this._historyDirty;
        this._historyDirty = false;
        for (let i = 0; i < this.series.length; i++) {
            const buf = this.hub[this.series[i].buffer];
            const total = buf?.total ?? -1;
            const count = buf?.count ?? -1;
            const last = count > 0 ? buf.last() : Number.NaN;
            const stamp = this._bufferStamps[i];
            if (stamp.buffer !== buf || stamp.total !== total || stamp.count !== count
                || !Object.is(stamp.last, last)) {
                stamp.buffer = buf;
                stamp.total = total;
                stamp.count = count;
                stamp.last = last;
                dirty = true;
            }
        }
        if (!dirty) return;

        this._ensureCapacity(n);

        const xs = this.xs.subarray(0, n);
        if (typeof firstBuf?.flattenTicksInto === 'function') {
            firstBuf.flattenTicksInto(xs, n);
        } else {
            const xStart = Math.max(0, (firstBuf?.total ?? firstBuf?.count ?? n) - n);
            for (let i = 0; i < n; i++) xs[i] = xStart + i;
        }

        const yColumns = this.series.map((s, idx) => {
            const buf = this.hub[s.buffer];
            const col = this.ys[idx].subarray(0, n);
            if (buf && buf.flattenInto) {
                buf.flattenInto(col, n);
            } else if (buf) {
                const start = Math.max(0, (buf.count || 0) - n);
                for (let i = 0; i < n; i++) col[i] = buf.get(start + i) ?? 0;
            }
            return col;
        });

        // `true` so uPlot recomputes scale min/max each frame — with ring
        // buffers the range changes constantly and skipping this leaves the
        // chart blank.
        this.uplot.setData([xs, ...yColumns], true);
        this._emptyPublished = false;
        this._lastData = { xs, yColumns, n };
        if (this._hoverActive) this._renderHoverTooltip();
    }

    _renderHoverTooltip() {
        if (!this._hoverActive || this._destroyed || !this.uplot || !this._lastData) return;
        const idx = this.uplot.cursor?.idx;
        if (idx == null || idx < 0 || idx >= this._lastData.n) {
            this.tooltip.hide();
            return;
        }
        const rows = this.series.map((s, seriesIdx) => {
            if (this.uplot.series[seriesIdx + 1]?.show === false) return null;
            const value = this._lastData.yColumns[seriesIdx]?.[idx];
            return {
                label: s.label,
                color: resolveChartColor(s.color),
                value: formatChartValue(value, s.unit || this.opts.yUnit || this.opts.yLabel || ''),
            };
        });
        this.tooltip.render({
            title: this.opts.tooltipTitle || this.opts.title || this.id,
            xLabel: this.opts.xLabel || 'sample',
            xValue: this._lastData.xs[idx],
            rows,
            anchorLeft: this.uplot.cursor?.left ?? 0,
            anchorTop: this.uplot.cursor?.top ?? 0,
        });
    }

    destroy() {
        if (this._destroyed) return;
        this._destroyed = true;
        if (this._resizeFrame) cancelAnimationFrame(this._resizeFrame);
        this._resizeFrame = 0;
        this._unsubscribeHistory?.();
        this._unsubscribeHistory = null;
        this._hoverTarget?.removeEventListener('pointerenter', this._onPointerEnter);
        this._hoverTarget?.removeEventListener('pointerleave', this._onPointerLeave);
        this._hoverTarget?.removeEventListener('mouseenter', this._onPointerEnter);
        this._hoverTarget?.removeEventListener('mouseleave', this._onPointerLeave);
        this.tooltip?.destroy();
        this._ro.disconnect();
        if (this.container?._ftdResize) delete this.container._ftdResize;
        this.uplot.destroy();
        this.uplot = null;
    }
}

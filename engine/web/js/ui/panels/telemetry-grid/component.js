import { telemetryHub } from '../../../telemetry-hub.js';
import { SCALE0_GRID_CHANNELS } from '../../../telemetry/registry/scale0-grid-channels.js';
import { ChartHoverTooltip, formatChartValue } from '../../charts/chart-hover-tooltip.js';
import { resolveChartColor } from '../../charts/theme.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';

const PANEL_MIN_INTERVAL_MS = 33;   // ~30 Hz cap for floated panels (SPEC_SCALE0_PERF §6.1)
const GRID_VISIBLE_SAMPLES = 120;   // display window; source ring buffers still retain their full history
const MAX_SPARK = GRID_VISIBLE_SAMPLES;

// ── Telemetry Channel Definitions per Active Scale ──────────────────────────
const CHANNELS = {
    // Scale 0: Substrate Lattice (canonical list in telemetry/registry/scale0-grid-channels.js)
    '0': SCALE0_GRID_CHANNELS,
    // Scale 1: Particle Engine
    '1': [
        { key: 'peTotal',      title: 'Total Energy',      buffer: 'peTotal',       color: 'var(--chart-pe-total, #e8e8e8)',   unit: 'MeV' },
        { key: 'peKE',         title: 'Kinetic Energy',    buffer: 'peKE',          color: 'var(--chart-pe-ke, #4ade80)',      unit: 'MeV' },
        { key: 'peCoulombPE',  title: 'Coulomb PE',        buffer: 'peCoulombPE',   color: 'var(--chart-pe-coulomb, #f87171)', unit: 'MeV' },
        { key: 'peGravityPE',  title: 'Gravity PE',        buffer: 'peGravityPE',   color: 'var(--chart-pe-gravity, #94a3b8)', unit: 'MeV' },
        { key: 'peDrift',      title: 'Energy Drift',      buffer: 'peEnergyDrift', color: 'var(--chart-pe-drift, #fbbf24)',   unit: '%' },
        { key: 'peCount',      title: 'Particle Count',    buffer: 'peCount',       color: 'var(--chart-pe-count, #fb8c00)',   unit: 'ct' },
        { key: 'peLocked',     title: 'Locked Particles',  buffer: 'peLockedCount', color: 'var(--chart-pe-locked, #fbbf24)',  unit: 'ct' },
        { key: 'peMobile',     title: 'Mobile Particles',  buffer: 'peMobileCount', color: 'var(--chart-pe-mobile, #42a5f5)',  unit: 'ct' },
        { key: 'peMomentum',   title: 'Total Momentum',    buffer: 'peMomentum',    color: 'var(--chart-pe-momentum, #a78bfa)', unit: 'sim' },
        { key: 'peAngMom',     title: 'Angular Mom (origin)', buffer: 'peAngMom',      color: 'var(--chart-pe-angmom, #60a5fa)',  unit: 'sim' },
        { key: 'peVirial',     title: 'Virial Ratio',      buffer: 'peVirial',      color: 'var(--chart-pe-virial, #fbbf24)', unit: 'ratio' },
        { key: 'peVrms',       title: 'RMS Velocity',      buffer: 'peRmsVelocity', color: 'var(--chart-pe-vrms, #4ade80)',   unit: 'c' },
        { key: 'peMaxBeta',    title: 'Max |v|/c',         buffer: 'peMaxBeta',     color: 'var(--chart-pe-beta, #f472b6)',   unit: 'c' },
        { key: 'peRadius',     title: 'System Radius',     buffer: 'peSystemRadius',color: 'var(--chart-pe-radius, #42a5f5)', unit: 'lu' },
        { key: 'peMaxForce',   title: 'Max Net Force',     buffer: 'peMaxForce',    color: 'var(--chart-pe-force, #fbbf24)',  unit: 'sim' },
        { key: 'peMeanForce',  title: 'Mean Net Force',    buffer: 'peMeanForce',   color: 'var(--chart-pe-force-mean, #fb923c)', unit: 'sim' },
        { key: 'peSeparation', title: '2-Body Separation', buffer: 'peSeparation',  color: 'var(--chart-pe-radius, #42a5f5)', unit: 'lu' },
        { key: 'peRadialVel',  title: 'Radial Velocity',   buffer: 'peRadialVelocity', color: 'var(--chart-pe-radial, #ef4444)', unit: 'c' }
    ],
    // Scale 2 & 3: Atoms & Molecules — one shared channel set (same engine).
    // AE energies/temperature/momentum are SIM UNITS (implicit k_B = 1,
    // audit P0-10): unit strings say "(sim)", never MeV / MK / Kelvin.
    // Assigned to both '2' and '3' below this object literal.
    '2': [
        { key: 'aeEnergy',    title: 'Tracked Energy',  buffer: 'aeEnergy',    color: 'var(--chart-ae-total, #e8e8e8)',    unit: '(sim)' },
        { key: 'aeKE',        title: 'Kinetic Energy',  buffer: 'aeKE',        color: 'var(--chart-ae-ke, #4ade80)',       unit: '(sim)' },
        { key: 'aePEIonic',   title: 'PE (Ionic)',      buffer: 'aePEIonic',   color: 'var(--chart-ae-pe-ionic, #f87171)', unit: '(sim)' },
        { key: 'aePEVdw',     title: 'PE (vdW)',        buffer: 'aePEVdw',     color: 'var(--chart-ae-pe-vdw, #2dd4bf)',   unit: '(sim)' },
        { key: 'aePEBond',    title: 'PE (Bond)',       buffer: 'aePEBond',    color: 'var(--chart-ae-pe-bond, #fb923c)',  unit: '(sim)' },
        { key: 'aeTemp',      title: 'Temperature',     buffer: 'aeTemp',      color: 'var(--chart-ae-temp, #fb8c00)',     unit: '(sim)' },
        { key: 'aeAtomCount', title: 'Atom Count',      buffer: 'aeAtomCount', color: 'var(--chart-ae-atoms, #42a5f5)',    unit: 'ct' },
        { key: 'aeBonds',     title: 'Bond Count',      buffer: 'aeBonds',     color: 'var(--chart-ae-bonds, #a78bfa)',    unit: 'ct' },
        { key: 'aeMomentum',  title: 'Momentum |p|',    buffer: 'aeMomentum',  color: 'var(--chart-ae-momentum, #60a5fa)', unit: '(sim)' },
        { key: 'aeDrift',     title: 'Conservative Drift', buffer: 'aeDrift',  color: 'var(--chart-ae-drift, #fbbf24)',    unit: '%' }
    ],
    // Scale 4: Planetary N-body
    '4': [
        { key: 'plTotal',      title: 'Total Energy',      buffer: 'plTotal',       color: 'var(--chart-pe-total, #e8e8e8)',   unit: '(sim)' },
        { key: 'plKE',         title: 'Kinetic Energy',    buffer: 'plKE',          color: 'var(--chart-pe-ke, #4ade80)',      unit: '(sim)' },
        { key: 'plPE',         title: 'Potential Energy',  buffer: 'plPE',          color: 'var(--chart-pe-coulomb, #f87171)', unit: '(sim)' },
        { key: 'plDrift',      title: 'Energy Drift',      buffer: 'plEnergyDrift', color: 'var(--chart-pe-drift, #fbbf24)',   unit: '%' },
        { key: 'plCount',      title: 'Body Count',        buffer: 'plCount',       color: 'var(--chart-pe-count, #fb8c00)',   unit: 'ct' },
        { key: 'plMomentum',   title: 'Total Momentum',    buffer: 'plMomentum',    color: 'var(--chart-pe-momentum, #a78bfa)', unit: '(sim)' },
        { key: 'plVirial',     title: 'Virial Ratio',      buffer: 'plVirial',      color: 'var(--chart-pe-virial, #fbbf24)', unit: 'ratio' },
        { key: 'plRadius',     title: 'System Radius',     buffer: 'plSystemRadius',color: 'var(--chart-pe-radius, #42a5f5)', unit: 'lu' }
    ],
    // Scale 5: Cosmic N-body
    '5': [
        { key: 'csBodies',   title: 'Body Count',      buffer: 'csBodies',  color: 'var(--chart-flux, #fb8c00)',   unit: 'ct' },
        { key: 'csHubble',   title: 'Background Hubble H(a)',buffer: 'csHubble',  color: 'var(--chart-energy, #42a5f5)', unit: 'H' },
        { key: 'csDM',       title: 'Dark Matter Frac',buffer: 'csDM',      color: 'var(--chart-eb, #a78bfa)',    unit: '%' }
    ]
};
// Scale 3 (molecules) runs the same AtomEngine as Scale 2 — identical channels.
CHANNELS['3'] = CHANNELS['2'];

export class TelemetryGridPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.activeScale = '0';
        this.charts = new Map(); // channelKey -> uPlotInstance
        this._bound = false;
        this._ro = null;
        // Visibility gate + lazy builder: only charts whose card intersects the
        // viewport are built (new uPlot) and redrawn each tick. The panel is
        // ~4000px tall, so most of its 23–39 sparklines are off-screen at any
        // moment and used to redraw (and be created) needlessly.
        this._io = null;
    }

    init() {
        if (!this.el) return this;
        this.el.innerHTML = `
            <div class="telemetry-grid-panel">
                <div class="telemetry-grid-container"></div>
            </div>
        `;
        this.container = this.el.querySelector('.telemetry-grid-container');
        this.el.dataset.component = 'telemetry-grid';

        // Detect layout active scale and build cards
        const app = document.getElementById('app');
        this.activeScale = app?.dataset.activeScale || '0';

        // Build/redraw only cards near the viewport. rootMargin pre-builds a
        // little above/below so a scroll doesn't reveal a blank chart. root:null
        // (viewport) still accounts for the panel's own scroll clip per spec.
        if (typeof IntersectionObserver === 'function') {
            this._io = new IntersectionObserver(
                (obsEntries) => this._onIntersect(obsEntries),
                { root: null, rootMargin: '150px 0px', threshold: 0 },
            );
        }

        this.rebuildGrid();

        this._ro = new ResizeObserver(() => this.reflowCharts());
        this._ro.observe(this.el);

        // Bind custom _ftdResize directly to panel so FloatingWindow triggers it automatically
        this.el._ftdResize = () => this.reflowCharts();

        this._bound = true;
        return this;
    }

    rebuildGrid() {
        // Stop observing the old cards before they are removed.
        this._io?.disconnect();
        // Destroy existing uPlots
        this.charts.forEach((entry) => {
            entry.hoverTarget?.removeEventListener('pointerenter', entry.onPointerEnter);
            entry.hoverTarget?.removeEventListener('pointerleave', entry.onPointerLeave);
            entry.hoverTarget?.removeEventListener('mouseenter', entry.onPointerEnter);
            entry.hoverTarget?.removeEventListener('mouseleave', entry.onPointerLeave);
            entry.tooltip?.destroy();
            entry?.u?.destroy?.();
        });
        this.charts.clear();
        this.container.innerHTML = '';
        this.el.dataset.activeScale = this.activeScale;

        const activeChannels = CHANNELS[this.activeScale] || [];

        if (activeChannels.length === 0) {
            this.container.innerHTML = `<div class="telemetry-grid-empty">No telemetry channels defined for Scale ${this.activeScale}</div>`;
            return;
        }

        activeChannels.forEach((chan) => {
            // Card DOM is cheap and built for every channel so the panel keeps
            // its full scroll height; the uPlot itself is created lazily by
            // _buildChart the first time the card scrolls into view.
            const card = document.createElement('article');
            card.className = 'telemetry-card';
            card.dataset.channelKey = chan.key;

            card.innerHTML = `
                <div class="telemetry-card-head">
                    <span class="telemetry-card-title">${chan.title}</span>
                    <span class="telemetry-card-value">--</span>
                </div>
                <div class="telemetry-card-plot" id="tele-plot-${chan.key}"></div>
            `;

            this.container.appendChild(card);

            // Preallocate the sparkline buffers ONCE (§6.1) and cache the value
            // <span>. onScreen defaults true so a card already in view draws as
            // soon as it is built; the observer's first callback corrects the
            // off-screen ones.
            const entry = {
                chan,
                card,
                plotContainer: card.querySelector('.telemetry-card-plot'),
                valueEl: card.querySelector('.telemetry-card-value'),
                xs: new Float64Array(MAX_SPARK),
                ys: new Float64Array(MAX_SPARK),
                u: null,
                tooltip: null,
                built: false,
                onScreen: true,
                hoverActive: false,
                lastN: 0,
                color: null,
                // The grid may be asked to repaint at UI cadence while the
                // native telemetry snapshot arrives only a few times per
                // second. Reusing the last uPlot data until its ring buffer
                // advances avoids redrawing a chart with identical samples.
                lastBuffer: null,
                lastTotal: -1,
                lastValue: Number.NaN,
                lastDisplayValue: Number.NaN,
            };
            this.charts.set(chan.key, entry);

            if (this._io) this._io.observe(card);
            else this._buildChart(entry);   // no IntersectionObserver: eager fallback
        });
    }

    // Create the uPlot + tooltip + hover wiring for one card. Called lazily the
    // first time the card intersects the viewport (or eagerly when there is no
    // IntersectionObserver). Idempotent.
    _buildChart(entry) {
        if (entry.built) return;
        const { chan, plotContainer } = entry;

        const strokeColor = resolveChartColor(chan.color);
        // Derive a smooth, glowing translucent fill
        const fillColor = strokeColor.startsWith('rgba')
            ? strokeColor.replace(/[\d\.]+\)$/, '0.05)')
            : `${strokeColor}0c`;
        entry.color = strokeColor;

        const uopts = {
            width: plotContainer.clientWidth || 240,
            height: 70,
            legend: { show: false },
            title: '',
            pxAlign: 1,
            cursor: {
                sync: {
                    key: "telemetry-grid-sync"
                },
                y: false // Only sync X crosshair
            },
            scales: {
                x: { time: false },
                y: { auto: true }
            },
            axes: [
                { show: false }, // Hide X axis
                { show: false }  // Hide Y axis
            ],
            series: [
                {}, // X axis series
                {
                    stroke: strokeColor,
                    fill: fillColor,
                    width: 1.5,
                    points: { show: false }
                }
            ],
            padding: [4, 0, 4, 0],
            hooks: {
                setCursor: [
                    () => { if (entry.hoverActive) this.renderTooltip(entry, chan); },
                ],
            },
        };

        // eslint-disable-next-line no-undef
        entry.u = new uPlot(uopts, [[], []], plotContainer);
        entry.tooltip = new ChartHoverTooltip(plotContainer);

        entry.onPointerEnter = () => {
            entry.hoverActive = true;
            this.renderTooltip(entry, chan);
        };
        entry.onPointerLeave = () => {
            entry.hoverActive = false;
            entry.tooltip.hide();
        };
        const hoverTarget = entry.u.over || plotContainer;
        entry.hoverTarget = hoverTarget;
        hoverTarget.addEventListener('pointerenter', entry.onPointerEnter);
        hoverTarget.addEventListener('pointerleave', entry.onPointerLeave);
        hoverTarget.addEventListener('mouseenter', entry.onPointerEnter);
        hoverTarget.addEventListener('mouseleave', entry.onPointerLeave);

        entry.built = true;
    }

    // IntersectionObserver callback: build-on-first-view and maintain the
    // onScreen flag that update() gates on. A freshly revealed chart has its
    // dirty-check invalidated and is drawn immediately so it never shows blank.
    _onIntersect(obsEntries) {
        for (const oe of obsEntries) {
            const entry = this.charts.get(oe.target.dataset.channelKey);
            if (!entry) continue;
            if (oe.isIntersecting) {
                if (!entry.built) this._buildChart(entry);
                if (!entry.onScreen) {
                    entry.lastBuffer = null;
                    entry.lastTotal = -1;
                    entry.lastValue = Number.NaN;
                }
                entry.onScreen = true;
                if (entry.u) this._drawEntry(entry);
            } else {
                entry.onScreen = false;
            }
        }
    }

    update() {
        if (PerfFlags.panelRenderV2) {
            // Don't redraw an invisible panel: skip when collapsed/hidden, and cap
            // floated panels (driven every frame by app.js) to ~30 Hz (§6.1). A
            // docked active tab is driven at ~20 Hz already, so the cap only bites
            // the floated 60 Hz case.
            if (!isPanelLive(this.el)) return;
            const now = performance.now();
            if (this._lastDraw && (now - this._lastDraw) < PANEL_MIN_INTERVAL_MS) return;
            this._lastDraw = now;
        } else if (!this.el.classList.contains('active') && !this.el.closest('.floating-window')) {
            return;
        }

        const app = document.getElementById('app');
        const currentScale = app?.dataset.activeScale || '0';

        let activeChannels = CHANNELS[this.activeScale] || [];

        // Rebuild cards if user switched scales OR if channels length changed (e.g. from HMR)
        if (currentScale !== this.activeScale || this.charts.size !== activeChannels.length) {
            this.activeScale = currentScale;
            this.rebuildGrid();
            activeChannels = CHANNELS[this.activeScale] || [];
        }

        activeChannels.forEach((chan) => {
            const entry = this.charts.get(chan.key);
            if (!entry) return;
            this._refreshValue(entry);
            // Cull off-screen (and not-yet-built) charts: the observer keeps
            // onScreen accurate, so a ~4000px panel only redraws the handful of
            // sparklines actually in view instead of all 23–39 every tick.
            if (!entry.onScreen || !entry.u) return;
            this._drawEntry(entry);
        });
    }

    _resolveBuffer(chan) {
        const pathParts = chan.buffer.split('.');
        let buf = telemetryHub;
        for (const part of pathParts) {
            if (buf) buf = buf[part];
        }
        return buf;
    }

    // Numeric card values are cheap and must not wait for IntersectionObserver
    // to allocate the heavier uPlot. This keeps the first visible paint live.
    _refreshValue(entry) {
        const buf = this._resolveBuffer(entry.chan);
        if (!buf || buf.count === 0) return;
        const latestValue = buf.last();
        if (Object.is(entry.lastDisplayValue, latestValue)) return;
        entry.lastDisplayValue = latestValue;
        if (entry.valueEl) {
            entry.valueEl.textContent = this.formatValue(latestValue, entry.chan.unit);
        }
    }

    // Pull the latest window from this channel's ring buffer into its sparkline.
    // Skips when nothing advanced (dirty-check) and reuses the preallocated
    // buffers (no per-frame allocation / DOM query, §6.1). Only visible charts
    // reach here (update() culls off-screen ones), which is where the real win
    // is; each visible redraw re-ranges both axes (see the setData note below).
    _drawEntry(entry) {
        const chan = entry.chan;

        const buf = this._resolveBuffer(chan);

        if (!buf || buf.count === 0) return;

        const total = buf.total ?? buf.count;
        const latestValue = buf.last();
        if (entry.lastBuffer === buf && entry.lastTotal === total
            && Object.is(entry.lastValue, latestValue)) return;

        const { u, valueEl, xs, ys } = entry;
        const n = Math.min(buf.count, GRID_VISIBLE_SAMPLES);
        const start = Math.max(0, buf.count - n);
        const xStart = Math.max(0, (buf.total ?? buf.count) - n);
        for (let i = 0; i < n; i++) {
            xs[i] = xStart + i;
            ys[i] = buf.get(start + i);
        }
        entry.lastN = n;
        entry.lastBuffer = buf;
        entry.lastTotal = total;
        entry.lastValue = latestValue;
        entry.lastDisplayValue = latestValue;

        // resetScales MUST stay true: the x window advances every tick (xStart
        // climbs), so drawing with resetScales=false leaves the line plotted
        // against a stale x-domain — it scrolls off the frozen range and looks
        // frozen/clipped. The auto-range is a cheap min/max over the 120-sample
        // window and the redraw happens either way; the perf win is culling the
        // off-screen charts entirely (update()), not skipping this scan.
        u.setData([xs.subarray(0, n), ys.subarray(0, n)], true);
        if (valueEl) valueEl.textContent = this.formatValue(latestValue, chan.unit);
        if (entry.hoverActive) this.renderTooltip(entry, chan);
    }

    renderTooltip(entry, chan) {
        if (!entry.hoverActive || !entry.u || entry.lastN < 2) return;
        const idx = entry.u.cursor?.idx;
        if (idx == null || idx < 0 || idx >= entry.lastN) {
            entry.tooltip.hide();
            return;
        }
        entry.tooltip.render({
            title: chan.title,
            xLabel: 'sample',
            xValue: entry.xs[idx],
            rows: [{
                label: chan.title,
                color: entry.color,
                value: formatChartValue(entry.ys[idx], chan.unit),
            }],
            anchorLeft: entry.u.cursor?.left ?? 0,
            anchorTop: entry.u.cursor?.top ?? 0,
        });
    }

    formatValue(val, unit) {
        if (val === undefined || val === null || isNaN(val)) return '--';
        if (unit === 'ct' || unit === 'b') {
            return Math.round(val).toLocaleString();
        }
        if (Math.abs(val) > 1e6) {
            return `${(val / 1e6).toFixed(3)}M ${unit}`;
        }
        if (Math.abs(val) > 1e3 && unit !== '%' && unit !== 'ratio') {
            return `${(val / 1e3).toFixed(3)}k ${unit}`;
        }
        if (Math.abs(val) < 1e-4 && val !== 0) {
            return val.toExponential(4);
        }
        return `${val.toFixed(4)} ${unit}`;
    }

    reflowCharts() {
        this.charts.forEach((entry, key) => {
            if (!entry.u) return;   // not yet built (still off-screen)
            const card = this.container.querySelector(`[data-channel-key="${key}"]`);
            if (card) {
                const plotContainer = card.querySelector('.telemetry-card-plot');
                if (plotContainer && plotContainer.clientWidth > 0) {
                    entry.u.setSize({
                        width: plotContainer.clientWidth,
                        height: 70
                    });
                }
            }
        });
    }

    cleanup() {
        if (this._ro) {
            this._ro.disconnect();
            this._ro = null;
        }
        if (this._io) {
            this._io.disconnect();
            this._io = null;
        }
        this.charts.forEach((entry) => {
            entry.hoverTarget?.removeEventListener('pointerenter', entry.onPointerEnter);
            entry.hoverTarget?.removeEventListener('pointerleave', entry.onPointerLeave);
            entry.hoverTarget?.removeEventListener('mouseenter', entry.onPointerEnter);
            entry.hoverTarget?.removeEventListener('mouseleave', entry.onPointerLeave);
            entry.tooltip?.destroy();
            entry.u?.destroy();
        });
        this.charts.clear();
    }
}

export function initTelemetryGridPanel() {
    const el = document.getElementById('panel-telemetry-grid');
    return el ? new TelemetryGridPanelComponent(el).init() : null;
}

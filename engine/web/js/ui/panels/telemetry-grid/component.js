import { telemetryHub } from '../../../telemetry-hub.js';
import { ChartHoverTooltip, formatChartValue } from '../../charts/chart-hover-tooltip.js';
import { resolveChartColor } from '../../charts/theme.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';

const PANEL_MIN_INTERVAL_MS = 33;   // ~30 Hz cap for floated panels (SPEC_SCALE0_PERF §6.1)
const GRID_VISIBLE_SAMPLES = 120;   // display window; source ring buffers still retain their full history
const MAX_SPARK = GRID_VISIBLE_SAMPLES;

// ── Telemetry Channel Definitions per Active Scale ──────────────────────────
const CHANNELS = {
    // Scale 0: Substrate Lattice
    '0': [
        // Base Diagnostics
        { key: 'flux',       title: 'Total Flux',        buffer: 'flux',            color: 'var(--chart-flux, #fb8c00)',   unit: 'J' },
        { key: 'energy',     title: 'Total Energy',      buffer: 'energy',          color: 'var(--chart-energy, #42a5f5)', unit: 'E*' },
        { key: 'manifested', title: 'Particle Count',    buffer: 'manifested',      color: 'var(--chart-eb, #a78bfa)',     unit: 'ct' },
        { key: 'charges',    title: 'Net Charge',        buffer: 'charges',         color: 'var(--chart-charge, #4ade80)', unit: 'e' },
        { key: 'positive',   title: 'Positive Charges',  buffer: 'positive',        color: 'var(--chart-positive, #4ade80)', unit: 'e' },
        { key: 'negative',   title: 'Negative Charges',  buffer: 'negative',        color: 'var(--chart-negative, #f87171)', unit: 'e' },
        { key: 'entropy',    title: 'Entropy',           buffer: 'entropy',         color: 'var(--chart-entropy, #60a5fa)', unit: 'nat' },
        { key: 'gauss',      title: 'Gauss Violation',   buffer: 'gauss',           color: 'var(--chart-gauss, #fbbf24)',   unit: 'E*²' },
        
        // Energy Audit
        { key: 'drift',      title: 'Energy Drift',      buffer: 'aud.energyDrift', color: 'var(--chart-eb, #a78bfa)', unit: '%' },
        { key: 'ebDiff',     title: 'E-B Energy Diff',   buffer: 'ebDiff',          color: 'var(--chart-eb, #a78bfa)', unit: 'E*' },
        { key: 'fieldE',     title: 'Field Energy',      buffer: 'aud.fieldEnergy', color: 'var(--chart-energy, #42a5f5)', unit: 'E*' },
        { key: 'waveE',      title: 'Wave Energy',       buffer: 'aud.waveEnergy',  color: 'var(--chart-flux, #fb8c00)', unit: 'E*' },
        { key: 'eField',     title: 'E-Field Energy',    buffer: 'aud.eFieldEnergy',color: 'var(--chart-positive, #4ade80)', unit: 'E*' },
        { key: 'bField',     title: 'B-Field Energy',    buffer: 'aud.bFieldEnergy',color: 'var(--chart-negative, #f87171)', unit: 'E*' },
        { key: 'poynting',   title: 'Poynting Mag',      buffer: 'aud.poyntingMag', color: 'var(--chart-gauss, #fbbf24)', unit: 'S' },
        { key: 'chirality',  title: 'Chirality',         buffer: 'aud.chirality',   color: 'var(--chart-entropy, #60a5fa)', unit: 'χ' },
        { key: 'partKE',     title: 'Particle KE',       buffer: 'aud.particleKE',  color: 'var(--chart-positive, #4ade80)', unit: 'E*' },
        { key: 'coulombPE',  title: 'Coulomb PE',        buffer: 'aud.coulombPE',   color: 'var(--chart-negative, #f87171)', unit: 'E*' },

        // Lagrangian terms
        { key: 'lagTotal',   title: 'Lagrangian (L)',    buffer: 'lag.total',       color: 'var(--chart-eb, #a78bfa)', unit: 'L' },
        { key: 'lagAction',  title: 'Total Action (S)',  buffer: 'lag.action',      color: 'var(--chart-gauss, #fbbf24)', unit: 'S' },
        { key: 'lagHam',     title: 'Hamiltonian (H)',   buffer: 'lag.hamiltonian', color: 'var(--chart-energy, #42a5f5)', unit: 'H' },
        { key: 'lagKinetic', title: 'Field Kinetic (T)', buffer: 'lag.fieldKinetic',color: 'var(--chart-flux, #fb8c00)', unit: 'T' },
        { key: 'lagGrad',    title: 'Field Gradient (V)',buffer: 'lag.fieldGradient',color: 'var(--chart-negative, #f87171)', unit: 'V' }
    ],
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
        { key: 'peMomentum',   title: 'Total Momentum',    buffer: 'peMomentum',    color: 'var(--chart-pe-momentum, #a78bfa)', unit: 'MeV/c' },
        { key: 'peAngMom',     title: 'Angular Momentum',  buffer: 'peAngMom',      color: 'var(--chart-pe-angmom, #60a5fa)',  unit: 'hbar' },
        { key: 'peVirial',     title: 'Virial Ratio',      buffer: 'peVirial',      color: 'var(--chart-pe-virial, #fbbf24)', unit: 'V' },
        { key: 'peVrms',       title: 'RMS Velocity',      buffer: 'peRmsVelocity', color: 'var(--chart-pe-vrms, #4ade80)',   unit: 'c' },
        { key: 'peRadius',     title: 'System Radius',     buffer: 'peSystemRadius',color: 'var(--chart-pe-radius, #42a5f5)', unit: 'lu' },
        { key: 'peMaxForce',   title: 'Max Net Force',     buffer: 'peMaxForce',    color: 'var(--chart-pe-force, #fbbf24)',  unit: 'F' },
        { key: 'peMeanForce',  title: 'Mean Net Force',    buffer: 'peMeanForce',   color: 'var(--chart-pe-force-mean, #fb923c)', unit: 'F' },
        { key: 'peSeparation', title: '2-Body Separation', buffer: 'peSeparation',  color: 'var(--chart-pe-radius, #42a5f5)', unit: 'lu' },
        { key: 'peRadialVel',  title: 'Radial Velocity',   buffer: 'peRadialVelocity', color: 'var(--chart-pe-radial, #ef4444)', unit: 'c' }
    ],
    // Scale 2 & 3: Atoms & Molecules — one shared channel set (same engine).
    // AE energies/temperature/momentum are SIM UNITS (implicit k_B = 1,
    // audit P0-10): unit strings say "(sim)", never MeV / MK / Kelvin.
    // Assigned to both '2' and '3' below this object literal.
    '2': [
        { key: 'aeEnergy',    title: 'Total Energy',    buffer: 'aeEnergy',    color: 'var(--chart-ae-total, #e8e8e8)',    unit: '(sim)' },
        { key: 'aeKE',        title: 'Kinetic Energy',  buffer: 'aeKE',        color: 'var(--chart-ae-ke, #4ade80)',       unit: '(sim)' },
        { key: 'aePEIonic',   title: 'PE (Ionic)',      buffer: 'aePEIonic',   color: 'var(--chart-ae-pe-ionic, #f87171)', unit: '(sim)' },
        { key: 'aePEVdw',     title: 'PE (vdW)',        buffer: 'aePEVdw',     color: 'var(--chart-ae-pe-vdw, #2dd4bf)',   unit: '(sim)' },
        { key: 'aePEBond',    title: 'PE (Bond)',       buffer: 'aePEBond',    color: 'var(--chart-ae-pe-bond, #fb923c)',  unit: '(sim)' },
        { key: 'aeTemp',      title: 'Temperature',     buffer: 'aeTemp',      color: 'var(--chart-ae-temp, #fb8c00)',     unit: '(sim)' },
        { key: 'aeAtomCount', title: 'Atom Count',      buffer: 'aeAtomCount', color: 'var(--chart-ae-atoms, #42a5f5)',    unit: 'ct' },
        { key: 'aeBonds',     title: 'Bond Count',      buffer: 'aeBonds',     color: 'var(--chart-ae-bonds, #a78bfa)',    unit: 'ct' },
        { key: 'aeMomentum',  title: 'Momentum |p|',    buffer: 'aeMomentum',  color: 'var(--chart-ae-momentum, #60a5fa)', unit: '(sim)' },
        { key: 'aeDrift',     title: 'Energy Drift',    buffer: 'aeDrift',     color: 'var(--chart-ae-drift, #fbbf24)',    unit: '%' }
    ],
    // Scale 4: Planetary N-body
    '4': [
        { key: 'plTotal',      title: 'Total Energy',      buffer: 'plTotal',       color: 'var(--chart-pe-total, #e8e8e8)',   unit: '(sim)' },
        { key: 'plKE',         title: 'Kinetic Energy',    buffer: 'plKE',          color: 'var(--chart-pe-ke, #4ade80)',      unit: '(sim)' },
        { key: 'plPE',         title: 'Potential Energy',  buffer: 'plPE',          color: 'var(--chart-pe-coulomb, #f87171)', unit: '(sim)' },
        { key: 'plDrift',      title: 'Energy Drift',      buffer: 'plEnergyDrift', color: 'var(--chart-pe-drift, #fbbf24)',   unit: '%' },
        { key: 'plCount',      title: 'Body Count',        buffer: 'plCount',       color: 'var(--chart-pe-count, #fb8c00)',   unit: 'ct' },
        { key: 'plMomentum',   title: 'Total Momentum',    buffer: 'plMomentum',    color: 'var(--chart-pe-momentum, #a78bfa)', unit: '(sim)' },
        { key: 'plVirial',     title: 'Virial Ratio',      buffer: 'plVirial',      color: 'var(--chart-pe-virial, #fbbf24)', unit: 'V' },
        { key: 'plRadius',     title: 'System Radius',     buffer: 'plSystemRadius',color: 'var(--chart-pe-radius, #42a5f5)', unit: 'lu' }
    ],
    // Scale 5: Cosmic N-body
    '5': [
        { key: 'csBodies',   title: 'Body Count',      buffer: 'csBodies',  color: 'var(--chart-flux, #fb8c00)',   unit: 'ct' },
        { key: 'csHubble',   title: 'Hubble parameter',buffer: 'csHubble',  color: 'var(--chart-energy, #42a5f5)', unit: 'H' },
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

        this.rebuildGrid();

        this._ro = new ResizeObserver(() => this.reflowCharts());
        this._ro.observe(this.el);

        // Bind custom _ftdResize directly to panel so FloatingWindow triggers it automatically
        this.el._ftdResize = () => this.reflowCharts();

        this._bound = true;
        return this;
    }

    rebuildGrid() {
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
            // 1. Create card DOM wrapper
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

            const plotContainer = card.querySelector('.telemetry-card-plot');
            let entry = null;

            // 2. Prepare high-performance sparkline configurations
            const strokeColor = resolveChartColor(chan.color);
            // Derive a smooth, glowing translucent fill
            const fillColor = strokeColor.startsWith('rgba') 
                ? strokeColor.replace(/[\d\.]+\)$/, '0.05)') 
                : `${strokeColor}0c`;

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
                        () => { if (entry) this.renderTooltip(entry, chan); },
                    ],
                },
            };

            // eslint-disable-next-line no-undef
            const u = new uPlot(uopts, [[], []], plotContainer);
            const tooltip = new ChartHoverTooltip(plotContainer);

            // Cache the value <span> + preallocate the sparkline buffers ONCE so
            // update() neither queries the DOM nor allocates per frame (§6.1). xs
            // is a static index ramp; ys is refilled each update.
            const valueEl = card.querySelector('.telemetry-card-value');
            const xs = new Float64Array(MAX_SPARK);
            const ys = new Float64Array(MAX_SPARK);

            entry = { u, valueEl, xs, ys, tooltip, hoverActive: false, lastN: 0, color: strokeColor };
            entry.onPointerEnter = () => {
                entry.hoverActive = true;
                this.renderTooltip(entry, chan);
            };
            entry.onPointerLeave = () => {
                entry.hoverActive = false;
                entry.tooltip.hide();
            };
            const hoverTarget = u.over || plotContainer;
            entry.hoverTarget = hoverTarget;
            hoverTarget.addEventListener('pointerenter', entry.onPointerEnter);
            hoverTarget.addEventListener('pointerleave', entry.onPointerLeave);
            hoverTarget.addEventListener('mouseenter', entry.onPointerEnter);
            hoverTarget.addEventListener('mouseleave', entry.onPointerLeave);

            this.charts.set(chan.key, entry);
        });
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

            // Resolve ring buffer source path from telemetryHub
            const pathParts = chan.buffer.split('.');
            let buf = telemetryHub;
            for (const part of pathParts) {
                if (buf) buf = buf[part];
            }

            if (!buf || buf.count === 0) return;

            // Reuse the preallocated buffers + cached value element — no per-frame
            // allocation and no DOM query (§6.1).
            const { u, valueEl, xs, ys } = entry;
            const n = Math.min(buf.count, GRID_VISIBLE_SAMPLES);
            const start = Math.max(0, buf.count - n);
            const xStart = Math.max(0, (buf.total ?? buf.count) - n);
            for (let i = 0; i < n; i++) {
                xs[i] = xStart + i;
                ys[i] = buf.get(start + i);
            }
            entry.lastN = n;

            u.setData([xs.subarray(0, n), ys.subarray(0, n)], true);
            if (valueEl) valueEl.textContent = this.formatValue(buf.last(), chan.unit);
            if (entry.hoverActive) this.renderTooltip(entry, chan);
        });
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
        if (Math.abs(val) > 1e3 && unit !== '%' && unit !== 'V') {
            return `${(val / 1e3).toFixed(3)}k ${unit}`;
        }
        if (Math.abs(val) < 1e-4 && val !== 0) {
            return val.toExponential(4);
        }
        return `${val.toFixed(4)} ${unit}`;
    }

    reflowCharts() {
        this.charts.forEach((entry, key) => {
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
        this.charts.forEach((entry) => {
            entry.hoverTarget?.removeEventListener('pointerenter', entry.onPointerEnter);
            entry.hoverTarget?.removeEventListener('pointerleave', entry.onPointerLeave);
            entry.hoverTarget?.removeEventListener('mouseenter', entry.onPointerEnter);
            entry.hoverTarget?.removeEventListener('mouseleave', entry.onPointerLeave);
            entry.tooltip?.destroy();
            entry.u.destroy();
        });
        this.charts.clear();
    }
}

export function initTelemetryGridPanel() {
    const el = document.getElementById('panel-telemetry-grid');
    return el ? new TelemetryGridPanelComponent(el).init() : null;
}

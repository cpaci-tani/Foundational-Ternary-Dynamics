import { telemetryHub } from '../../../telemetry-hub.js';
import { getChartTheme, resolveChartColor } from '../../charts/theme.js';

// Create a single synchronized cursor registry for all sparklines in the grid
const telemetrySync = uPlot.sync("telemetry-grid-sync");

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
        { key: 'peTotal',    title: 'Total Energy',    buffer: 'peTotal',   color: 'var(--chart-energy, #42a5f5)', unit: 'MeV' },
        { key: 'peKE',       title: 'Kinetic Energy',  buffer: 'peKE',      color: 'var(--chart-positive, #4ade80)', unit: 'MeV' },
        { key: 'pePE',       title: 'Potential Energy',buffer: 'pePE',      color: 'var(--chart-negative, #f87171)', unit: 'MeV' },
        { key: 'peCount',    title: 'Particle Count',  buffer: 'peCount',   color: 'var(--chart-flux, #fb8c00)',   unit: 'ct' },
        { key: 'peMomentum', title: 'Total Momentum',  buffer: 'peMomentum',color: 'var(--chart-eb, #a78bfa)',    unit: 'MeV/c' },
        { key: 'peAngMom',   title: 'Angular Momentum',buffer: 'peAngMom',  color: 'var(--chart-entropy, #60a5fa)', unit: 'J·s' },
        { key: 'peVirial',   title: 'Virial Ratio',    buffer: 'peVirial',  color: 'var(--chart-gauss, #fbbf24)',  unit: 'V' }
    ],
    // Scale 2 & 3: Atoms & Molecules
    '2': [
        { key: 'aeEnergy',   title: 'Total Energy',    buffer: 'aeEnergy',  color: 'var(--chart-energy, #42a5f5)', unit: 'MeV' },
        { key: 'aeKE',       title: 'Kinetic Energy',  buffer: 'aeKE',      color: 'var(--chart-positive, #4ade80)', unit: 'MeV' },
        { key: 'aeTemp',     title: 'Temperature',     buffer: 'aeTemp',    color: 'var(--chart-flux, #fb8c00)',   unit: 'MK' },
        { key: 'aeBonds',    title: 'Bond Count',      buffer: 'aeBonds',   color: 'var(--chart-eb, #a78bfa)',    unit: 'ct' }
    ],
    '3': [
        { key: 'aeEnergy',   title: 'Total Energy',    buffer: 'aeEnergy',  color: 'var(--chart-energy, #42a5f5)', unit: 'MeV' },
        { key: 'aeKE',       title: 'Kinetic Energy',  buffer: 'aeKE',      color: 'var(--chart-positive, #4ade80)', unit: 'MeV' },
        { key: 'aeTemp',     title: 'Temperature',     buffer: 'aeTemp',    color: 'var(--chart-flux, #fb8c00)',   unit: 'MK' },
        { key: 'aeBonds',    title: 'Bond Count',      buffer: 'aeBonds',   color: 'var(--chart-eb, #a78bfa)',    unit: 'ct' }
    ],
    // Scale 5: Cosmic N-body
    '5': [
        { key: 'csBodies',   title: 'Body Count',      buffer: 'csBodies',  color: 'var(--chart-flux, #fb8c00)',   unit: 'ct' },
        { key: 'csHubble',   title: 'Hubble parameter',buffer: 'csHubble',  color: 'var(--chart-energy, #42a5f5)', unit: 'H' },
        { key: 'csDM',       title: 'Dark Matter Frac',buffer: 'csDM',      color: 'var(--chart-eb, #a78bfa)',    unit: '%' }
    ]
};

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
        this.charts.forEach((chart) => chart.destroy());
        this.charts.clear();
        this.container.innerHTML = '';

        const activeChannels = CHANNELS[this.activeScale] || [];

        if (activeChannels.length === 0) {
            this.container.innerHTML = `<div class="telemetry-grid-empty">No telemetry channels defined for Scale ${this.activeScale}</div>`;
            return;
        }

        const theme = getChartTheme();

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
                padding: [4, 0, 4, 0]
            };

            // eslint-disable-next-line no-undef
            const u = new uPlot(uopts, [[], []], plotContainer);
            this.charts.set(chan.key, u);
        });
    }

    update() {
        if (!this.el.classList.contains('active') && !this.el.closest('.floating-window')) return;

        const app = document.getElementById('app');
        const currentScale = app?.dataset.activeScale || '0';

        const activeChannels = CHANNELS[this.activeScale] || [];

        // Rebuild cards if user switched scales OR if channels length changed (e.g. from HMR)
        if (currentScale !== this.activeScale || this.charts.size !== activeChannels.length) {
            this.activeScale = currentScale;
            this.rebuildGrid();
        }

        activeChannels.forEach((chan) => {
            const chart = this.charts.get(chan.key);
            if (!chart) return;

            // Resolve ring buffer source path from telemetryHub
            const pathParts = chan.buffer.split('.');
            let buf = telemetryHub;
            for (const part of pathParts) {
                if (buf) buf = buf[part];
            }

            if (!buf || buf.count === 0) return;

            const n = buf.count;
            const xs = new Float64Array(n);
            const ys = new Float64Array(n);

            for (let i = 0; i < n; i++) {
                xs[i] = i;
                ys[i] = buf.get(i);
            }

            // Update uPlot sparkline data
            chart.setData([xs, ys], true);

            // Update formatted label value
            const latestVal = buf.last();
            const card = this.container.querySelector(`[data-channel-key="${chan.key}"]`);
            if (card) {
                const valDisplay = card.querySelector('.telemetry-card-value');
                if (valDisplay) {
                    valDisplay.textContent = this.formatValue(latestVal, chan.unit);
                }
            }
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
        this.charts.forEach((chart, key) => {
            const card = this.container.querySelector(`[data-channel-key="${key}"]`);
            if (card) {
                const plotContainer = card.querySelector('.telemetry-card-plot');
                if (plotContainer && plotContainer.clientWidth > 0) {
                    chart.setSize({
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
        this.charts.forEach((chart) => chart.destroy());
        this.charts.clear();
    }
}

export function initTelemetryGridPanel() {
    const el = document.getElementById('panel-telemetry-grid');
    return el ? new TelemetryGridPanelComponent(el).init() : null;
}

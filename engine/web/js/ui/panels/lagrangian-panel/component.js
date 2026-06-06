import { getLagrangianPanelTemplate } from './template.js';
import { UPlotChart } from '../../charts/uplot-chart.js';
import { attachFullscreen } from '../../charts/chart-fullscreen.js';
import { DiagnosticsTable } from '../diagnostics-panel/table.js';
import { TermRow } from './term-row.js';
import { terms, actionRows, constantRows } from './descriptors/scale0.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import * as consts from '../../../constants.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';

const LS_HIDDEN = 'ftd.chart.lagrangian.hidden';

function loadHidden() {
    try {
        const raw = localStorage.getItem(LS_HIDDEN);
        return new Set(raw ? JSON.parse(raw) : []);
    } catch { return new Set(); }
}

function saveHidden(set) {
    try { localStorage.setItem(LS_HIDDEN, JSON.stringify([...set])); } catch {}
}

export class LagrangianPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.tables = [];
        this.cards = new Map(); // term.key → { term, card, chart }
    }

    init() {
        if (!this.el) return this;
        if (!this.el.dataset.panelRedesignMounted) {
            this.el.innerHTML = getLagrangianPanelTemplate();
            this.el.dataset.panelRedesignMounted = '1';
        }
        this.el.dataset.component = 'lagrangian-panel';

        this.hidden = loadHidden();
        this.grid = this.el.querySelector('.lag-charts-grid');

        // Per-term small multiples (stacked vertically, each with its own
        // y-scale) replace the single overlapping stacked-area chart, so every
        // term is legible even when magnitudes span orders (Field KE ~95 vs
        // Dissipation ~1e-3) and signed terms can dip negative.
        this._syncCards();

        // Term checkbox row — each toggle adds/removes that term's card.
        this.termRow = new TermRow(terms.map((t) => ({
            ...t, includeByDefault: !this.hidden.has(t.key),
        })), {
            onToggle: (key, checked) => {
                if (checked) this.hidden.delete(key); else this.hidden.add(key);
                saveHidden(this.hidden);
                this._syncCards();
            },
        });
        this.el.querySelector('.lag-term-row-host').appendChild(this.termRow.el);

        // Sidecar tables.
        const hubView = Object.create(telemetryHub);
        hubView.consts = consts;
        const dataCol = this.el.querySelector('.lag-data-col');
        const actionTable = new DiagnosticsTable(
            { id: 'lag-action', title: 'Action & Constraints', rows: actionRows },
            hubView
        );
        const constantsTable = new DiagnosticsTable(
            { id: 'lag-constants', title: 'Ontic Constants', rows: constantRows, variant: 'static' },
            hubView
        );
        dataCol.appendChild(actionTable.el);
        dataCol.appendChild(constantsTable.el);
        this.tables.push(actionTable, constantsTable);

        // Initial render so cells show 0 / constants immediately.
        for (const t of this.tables) t.update();

        return this;
    }

    /** Build a small-multiple card for one Lagrangian term. The per-term ring
     *  buffers live under `telemetryHub.lag`, so the chart's hub is that
     *  sub-object (UPlotChart reads `hub[buffer]`). */
    _makeTermCard(term) {
        const card = document.createElement('article');
        card.className = 'chart-card lag-term-card';
        card.dataset.term = term.key;
        card.innerHTML = `
            <header class="chart-card-head">
                <h3 class="chart-card-title">
                    <span class="lag-term-swatch" aria-hidden="true" style="--legend-color:${term.color}"></span>
                    ${term.label}
                </h3>
                <button type="button" class="chart-card-expand"
                    title="View fullscreen (Esc to close)"
                    aria-label="View ${term.label} fullscreen">⛶</button>
            </header>
            <div class="chart-card-plot"></div>
        `;
        attachFullscreen(card);
        const plotEl = card.querySelector('.chart-card-plot');
        const chart = new UPlotChart(plotEl, {
            id:     'lag-term-' + term.key,
            title:  '',
            series: [{ key: term.key, label: term.label, color: term.color, buffer: term.buffer }],
            xLabel: 'tick', yLabel: 'ℒ',
            hub:    telemetryHub.lag,
        });
        requestAnimationFrame(() => card.classList.add('is-mounted'));
        return { term, card, chart };
    }

    /** Reconcile rendered cards with the non-hidden term set. */
    _syncCards() {
        // Drop cards for terms that became hidden.
        for (const [key, entry] of this.cards) {
            if (this.hidden.has(key)) {
                entry.chart.destroy();
                entry.card.remove();
                this.cards.delete(key);
            }
        }
        // Add cards for newly-visible terms (preserving descriptor order).
        for (const term of terms) {
            if (this.hidden.has(term.key) || this.cards.has(term.key)) continue;
            const entry = this._makeTermCard(term);
            this.grid.appendChild(entry.card);
            this.cards.set(term.key, entry);
        }
    }

    update() {
        // V2: live when the active tab OR a non-collapsed floated window (fixes
        // the Lagrangian panel freezing while floated). Legacy: active tab only.
        if (PerfFlags.panelRenderV2 ? !isPanelLive(this.el) : !this.el.classList.contains('active')) return;
        for (const entry of this.cards.values()) entry.chart.update();
        for (const t of this.tables) t.update();
    }

    cleanup() {
        for (const entry of this.cards.values()) entry.chart.destroy();
        this.cards.clear();
        for (const t of this.tables) t.destroy();
        this.tables.length = 0;
    }
}

export function initLagrangianPanel() {
    const el = document.getElementById('panel-lagrangian');
    return el ? new LagrangianPanelComponent(el).init() : null;
}

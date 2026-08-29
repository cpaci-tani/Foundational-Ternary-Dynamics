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
import { formatValue } from '../diagnostics-panel/formatters.js';
import { getScale0State } from '../../../scales/scale0/state/store.js';

const LS_HIDDEN = 'ftd.chart.lagrangian.hidden';
const EMPTY_SCENARIO_ID = 'empty';

function finiteNumber(value) {
    return typeof value === 'number' && Number.isFinite(value) ? value : null;
}

function normalizeExactZero(value) {
    return Object.is(value, -0) ? 0 : value;
}

/**
 * Interpret only the qualified Scenario-1 null control. The Born-Infeld term
 * is state-independent there, so subtracting it from the reported total
 * exposes the measured excitation above the observer/functional baseline.
 * No value is synthesized when the current telemetry group is absent/stale.
 */
export function interpretEmptyObserverBaseline(lagrangian, {
    scenarioId = '',
    telemetryMeta = null,
} = {}) {
    if (scenarioId !== EMPTY_SCENARIO_ID) {
        return { status: 'not-applicable', observerBaseline: null, excitation: null };
    }
    if (!telemetryMeta || telemetryMeta.stale === true || !lagrangian) {
        return { status: 'unavailable', observerBaseline: null, excitation: null };
    }
    const observerBaseline = finiteNumber(lagrangian.bornInfeld);
    const total = finiteNumber(lagrangian.total);
    if (observerBaseline === null) {
        return { status: 'unavailable', observerBaseline: null, excitation: null };
    }
    if (total === null) {
        return { status: 'baseline-only', observerBaseline, excitation: null };
    }
    return {
        status: 'supported-null-control',
        observerBaseline,
        excitation: normalizeExactZero(total - observerBaseline),
    };
}

function setTextIfChanged(element, text) {
    if (element && element.textContent !== text) element.textContent = text;
}

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
        this.observerCard = this.el.querySelector('.lag-observer-baseline');
        this.observerValue = this.el.querySelector('[data-lag-observer-value]');
        this.excitationValue = this.el.querySelector('[data-lag-excitation-value]');
        this.observerStatusText = this.el.querySelector('[data-lag-observer-status-text]');

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
            hubView,
            { resetScope: 0 }
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
        this._updateObserverBaseline();

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
            tooltipTitle: term.label,
            series: [{ key: term.key, label: term.label, color: term.color, buffer: term.buffer, unit: term.unit }],
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

    _updateObserverBaseline() {
        if (!this.observerCard) return;
        const interpretation = interpretEmptyObserverBaseline(
            telemetryHub.s0?.lagrangian,
            {
                scenarioId: getScale0State().currentScenarioId,
                telemetryMeta: telemetryHub.getScale0TelemetryMeta?.('lagrangian') ?? null,
            },
        );
        if (this.observerCard.dataset.lagObserverStatus !== interpretation.status) {
            this.observerCard.dataset.lagObserverStatus = interpretation.status;
        }

        const baselineText = interpretation.observerBaseline === null
            ? '—' : formatValue(interpretation.observerBaseline);
        const excitationText = interpretation.excitation === null
            ? '—' : formatValue(interpretation.excitation);
        setTextIfChanged(this.observerValue, baselineText);
        setTextIfChanged(this.excitationValue, excitationText);
        if (this.observerValue?.dataset.value !== baselineText) {
            this.observerValue.dataset.value = baselineText;
        }
        if (this.excitationValue?.dataset.value !== excitationText) {
            this.excitationValue.dataset.value = excitationText;
        }

        const statusText = {
            'not-applicable': 'Baseline subtraction is shown only for Scenario 1 · Empty.',
            unavailable: 'Awaiting a current empty-scenario Lagrangian sample. Unavailable is not zero.',
            'baseline-only': 'Observer baseline published; total Lagrangian is unavailable, so Δℒ is not reported.',
            'supported-null-control': 'Current empty-control sample; Δℒ is baseline-subtracted excitation.',
        }[interpretation.status];
        setTextIfChanged(this.observerStatusText, statusText);
    }

    update() {
        // V2: live when the active tab OR a non-collapsed floated window (fixes
        // the Lagrangian panel freezing while floated). Legacy: active tab only.
        if (PerfFlags.panelRenderV2 ? !isPanelLive(this.el) : !this.el.classList.contains('active')) return;
        for (const entry of this.cards.values()) entry.chart.update();
        for (const t of this.tables) t.update();
        this._updateObserverBaseline();
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

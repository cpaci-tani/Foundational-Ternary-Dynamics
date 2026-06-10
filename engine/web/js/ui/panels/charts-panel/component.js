import { getChartsPanelTemplate } from './template.js';
import { ChartCard } from './chart-card.js';
import { charts as scale0Charts } from './descriptors/scale0.js';
import { charts as scale1Charts } from './descriptors/scale1.js';
import { charts as scale2Charts } from './descriptors/scale2.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';

const LS_ACTIVE_LEGACY = 'ftd.charts.active';
const LS_ACTIVE_PREFIX = 'ftd.charts.active.';

const CHARTS_BY_SCALE = Object.freeze({
    '0': scale0Charts,
    '1': scale1Charts,
    // Scales 2 (atoms) and 3 (molecules) share the AtomEngine and therefore
    // the same chart descriptors / hub buffers.
    '2': scale2Charts,
    '3': scale2Charts,
});

function getScaleCharts(scale) {
    return CHARTS_BY_SCALE[String(scale)] || scale0Charts;
}

function loadActiveSet(defaults, scale, descriptors) {
    try {
        let raw = localStorage.getItem(LS_ACTIVE_PREFIX + scale);
        if (!raw && String(scale) === '0') raw = localStorage.getItem(LS_ACTIVE_LEGACY);
        if (raw) {
            // Intersect with the scale's current descriptor ids: before a scale
            // had its own descriptors it fell back to scale-0 charts, so stored
            // sets can hold foreign ids that would otherwise render zero cards.
            const validIds = new Set(descriptors.map((c) => c.id));
            const stored = new Set(JSON.parse(raw).filter((id) => validIds.has(id)));
            if (stored.size > 0) return stored;
        }
    } catch {}
    return new Set(defaults);
}

function saveActiveSet(set, scale) {
    try { localStorage.setItem(LS_ACTIVE_PREFIX + scale, JSON.stringify([...set])); } catch {}
}

export class ChartsPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.cards = new Map(); // chartId → ChartCard
        this.activeScale = '0';
        this.descriptors = scale0Charts;
    }

    init() {
        if (!this.el) return this;
        if (!this.el.dataset.panelRedesignMounted) {
            this.el.innerHTML = getChartsPanelTemplate();
            this.el.dataset.panelRedesignMounted = '1';
        }
        this.el.dataset.component = 'charts-panel';

        this.chipStrip = this.el.querySelector('.charts-chip-strip');
        this.grid      = this.el.querySelector('.charts-grid');

        const app = document.getElementById('app');
        this._setScale(app?.dataset.activeScale || '0', { force: true });

        return this;
    }

    _setScale(scale, { force = false } = {}) {
        scale = String(scale);
        if (!force && scale === this.activeScale) return false;

        this.activeScale = scale;
        this.descriptors = getScaleCharts(scale);
        this.el.dataset.activeScale = scale;

        const defaults = this.descriptors.filter((c) => c.defaultActive).map((c) => c.id);
        this.active = loadActiveSet(defaults, scale, this.descriptors);

        this._destroyAllCards();
        this._renderChipStrip();
        this._syncCards();
        return true;
    }

    _destroyAllCards() {
        for (const card of this.cards.values()) card.destroy();
        this.cards.clear();
        if (this.grid) this.grid.innerHTML = '';
    }

    _renderChipStrip() {
        this.chipStrip.innerHTML = '';
        for (const desc of this.descriptors) {
            const chip = document.createElement('button');
            chip.type = 'button';
            chip.className = 'charts-chip';
            chip.dataset.chartId = desc.id;
            chip.setAttribute('aria-pressed', this.active.has(desc.id) ? 'true' : 'false');
            chip.textContent = desc.title;
            chip.addEventListener('click', () => this._toggle(desc.id));
            this.chipStrip.appendChild(chip);
        }
    }

    _toggle(chartId) {
        if (this.active.has(chartId)) this.active.delete(chartId);
        else                          this.active.add(chartId);
        saveActiveSet(this.active, this.activeScale);
        const chip = this.chipStrip.querySelector(`[data-chart-id="${chartId}"]`);
        if (chip) chip.setAttribute('aria-pressed', this.active.has(chartId) ? 'true' : 'false');
        this._syncCards();
    }

    _syncCards() {
        // Destroy cards no longer active (fade out, then destroy).
        for (const [id, card] of this.cards) {
            if (!this.active.has(id)) {
                card.el.classList.add('is-leaving');
                const victim = card;
                setTimeout(() => victim.destroy(), 140);
                this.cards.delete(id);
            }
        }
        // Create newly-active cards.
        for (const desc of this.descriptors) {
            if (!this.active.has(desc.id) || this.cards.has(desc.id)) continue;
            const card = new ChartCard(desc, telemetryHub);
            this.grid.appendChild(card.el);
            this.cards.set(desc.id, card);
        }
    }

    update() {
        // V2: live when the active tab OR a non-collapsed floated window (fixes
        // charts freezing while floated). Legacy: active tab only (§6.4).
        if (PerfFlags.panelRenderV2 ? !isPanelLive(this.el) : !this.el.classList.contains('active')) return;
        const app = document.getElementById('app');
        this._setScale(app?.dataset.activeScale || '0');
        for (const card of this.cards.values()) card.update();
    }

    cleanup() {
        for (const card of this.cards.values()) card.destroy();
        this.cards.clear();
    }
}

export function initChartsPanel() {
    const el = document.getElementById('panel-charts');
    return el ? new ChartsPanelComponent(el).init() : null;
}

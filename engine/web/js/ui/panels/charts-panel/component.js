import { getChartsPanelTemplate } from './template.js';
import { ChartCard } from './chart-card.js';
import { charts as scale0Charts } from './descriptors/scale0.js';
import { telemetryHub } from '../../../telemetry-hub.js';

const LS_ACTIVE = 'ftd.charts.active';

function loadActiveSet(defaults) {
    try {
        const raw = localStorage.getItem(LS_ACTIVE);
        if (raw) return new Set(JSON.parse(raw));
    } catch {}
    return new Set(defaults);
}

function saveActiveSet(set) {
    try { localStorage.setItem(LS_ACTIVE, JSON.stringify([...set])); } catch {}
}

export class ChartsPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.cards = new Map(); // chartId → ChartCard
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

        const defaults = scale0Charts.filter((c) => c.defaultActive).map((c) => c.id);
        this.active = loadActiveSet(defaults);

        this._renderChipStrip();
        this._syncCards();

        return this;
    }

    _renderChipStrip() {
        this.chipStrip.innerHTML = '';
        for (const desc of scale0Charts) {
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
        saveActiveSet(this.active);
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
        for (const desc of scale0Charts) {
            if (!this.active.has(desc.id) || this.cards.has(desc.id)) continue;
            const card = new ChartCard(desc, telemetryHub);
            this.grid.appendChild(card.el);
            this.cards.set(desc.id, card);
        }
    }

    update() {
        if (!this.el.classList.contains('active')) return;
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

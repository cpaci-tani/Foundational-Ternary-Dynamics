import { getLagrangianPanelTemplate } from './template.js';
import { StackedAreaChart } from '../../charts/stacked-area.js';
import { DiagnosticsTable } from '../diagnostics-panel/table.js';
import { TermRow } from './term-row.js';
import { terms, actionRows, constantRows } from './descriptors/scale0.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import * as consts from '../../../constants.js';
import { attachFullscreen } from '../../charts/chart-fullscreen.js';

const LS_HIDDEN = 'ftd.chart.lagrangian.hidden';

function loadHidden() {
    try {
        const raw = localStorage.getItem(LS_HIDDEN);
        return new Set(raw ? JSON.parse(raw) : []);
    } catch { return new Set(); }
}

export class LagrangianPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.tables = [];
    }

    init() {
        if (!this.el) return this;
        if (!this.el.dataset.panelRedesignMounted) {
            this.el.innerHTML = getLagrangianPanelTemplate();
            this.el.dataset.panelRedesignMounted = '1';
        }
        this.el.dataset.component = 'lagrangian-panel';

        const hidden = loadHidden();

        // Expose constants module on a hub view so the constants table
        // can resolve `consts.<NAME>` source paths.
        const hubView = Object.create(telemetryHub);
        hubView.consts = consts;

        // Wire fullscreen expand on the static chart card in the template
        const lagCard = this.el.querySelector('.chart-card');
        if (lagCard) attachFullscreen(lagCard);

        // Stacked-area chart
        this.chart = new StackedAreaChart(this.el.querySelector('#lag-plot-host'), {
            id:     'lagrangian',
            title:  '',
            series: terms.map((t) => ({
                key: t.key, label: t.label, color: t.color, buffer: t.buffer,
            })),
            xLabel: 'tick', yLabel: '|ℒ|',
            hub: telemetryHub,
        });

        // Apply initial visibility from localStorage.
        for (let i = 0; i < terms.length; i++) {
            if (hidden.has(terms[i].key)) {
                this.chart.uplot.setSeries(i + 1, { show: false });
            }
        }

        // Term checkbox row — syncs with chart legend state.
        this.termRow = new TermRow(terms.map((t) => ({
            ...t, includeByDefault: !hidden.has(t.key),
        })), {
            onToggle: (key, checked) => {
                const idx = terms.findIndex((t) => t.key === key);
                if (idx >= 0) this.chart.uplot.setSeries(idx + 1, { show: checked });
            },
        });
        this.el.querySelector('.lag-term-row-host').appendChild(this.termRow.el);

        // Sidecar tables.
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

    update() {
        if (!this.el.classList.contains('active')) return;
        this.chart?.update();
        for (const t of this.tables) t.update();
    }

    cleanup() {
        this.chart?.destroy();
        for (const t of this.tables) t.destroy();
        this.tables.length = 0;
    }
}

export function initLagrangianPanel() {
    const el = document.getElementById('panel-lagrangian');
    return el ? new LagrangianPanelComponent(el).init() : null;
}

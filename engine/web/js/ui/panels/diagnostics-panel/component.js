/**
 * DiagnosticsPanelComponent — composes scale-specific diagnostics tables from
 * descriptors. Scale 1 also keeps the detailed PE telemetry block below the
 * descriptor summary for per-particle/orbital drill-down.
 */

import { DiagnosticsTable } from './table.js?v=2';
import { sections as scale0Sections } from './descriptors/scale0.js';
import { sections as scale1Sections } from './descriptors/scale1.js?v=9';
import { sections as scale2Sections } from './descriptors/scale2.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';
import { TickHistoryControl } from '../../charts/history-window.js';

export class DiagnosticsPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.tables = [];
        this.tablesByScale = { '0': [], '1': [], ae: [] };
    }

    init() {
        if (!this.el) return this;
        this.historyControl = new TickHistoryControl(this.el, {
            id: 'diagnostics-panel',
            defaultTicks: 120,
        });
        if (!this.el.dataset.panelRedesignMounted) {
            const scale0Root = document.createElement('div');
            scale0Root.className = 'scale0-only diag-scale0-root';
            for (const section of scale0Sections) {
                const table = new DiagnosticsTable(section, telemetryHub, {
                    resetScope: 0,
                    historyControl: this.historyControl,
                });
                scale0Root.appendChild(table.el);
                this.tables.push(table);
                this.tablesByScale['0'].push(table);
            }
            this.el.insertBefore(scale0Root, this.el.firstChild);

            const scale1Root = document.createElement('div');
            scale1Root.className = 'scale1-only diag-scale1-root';
            for (const section of scale1Sections) {
                const table = new DiagnosticsTable(section, telemetryHub, {
                    resetScope: 1,
                    historyControl: this.historyControl,
                });
                scale1Root.appendChild(table.el);
                this.tables.push(table);
                this.tablesByScale['1'].push(table);
            }
            this.el.insertBefore(scale1Root, scale0Root.nextSibling);

            // Scales 2 + 3 share the AtomEngine descriptor table. Supplemental
            // nuclear/binding cards live in a collapsible details block below.
            const aeRoot = document.createElement('div');
            aeRoot.className = 'scale-ae diag-ae-root';
            for (const section of scale2Sections) {
                const table = new DiagnosticsTable(section, telemetryHub, {
                    resetScope: 2,
                    historyControl: this.historyControl,
                });
                aeRoot.appendChild(table.el);
                this.tables.push(table);
                this.tablesByScale.ae.push(table);
            }
            this.el.insertBefore(aeRoot, scale1Root.nextSibling);
            this.el.dataset.panelRedesignMounted = '1';
        }
        this.el.dataset.component = 'diagnostics-panel';
        // Populate cells immediately so rows never show init em-dash
        // (formatters render 0 for missing/null/undefined).
        this.update(true);
        return this;
    }

    update(force = false) {
        // V2: self-gate on visibility (incl. collapse) so a hidden or
        // floated-collapsed diagnostics panel doesn't redraw ~23 sparklines.
        // `force` lets init() populate once regardless. Legacy: caller-gated (§6.4).
        if (!force && PerfFlags.panelRenderV2 && !isPanelLive(this.el)) return;
        if (force) {
            for (const t of this.tables) t.update();
            return;
        }
        const scale = document.getElementById('app')?.dataset.activeScale || '0';
        const groupId = scale === '1' ? '1'
            : ((scale === '2' || scale === '3') ? 'ae'
                : (scale === '0' ? '0' : null));
        const group = groupId === '1'
            ? this.tablesByScale['1']
            : groupId === 'ae'
                ? this.tablesByScale.ae
                : (groupId === '0' ? this.tablesByScale['0'] : null);
        if (!group) return;
        // Consume the same coherent source samples as the other chart panels.
        // DiagnosticsTable independently stamps every row and spark buffer, so
        // this does not append duplicate measurements or redraw unchanged
        // canvases; it only removes the former extra 200 ms presentation gate.
        for (const t of group) t.update();
    }

    cleanup() {
        for (const t of this.tables) t.destroy();
        this.tables.length = 0;
        this.tablesByScale['0'].length = 0;
        this.tablesByScale['1'].length = 0;
        this.tablesByScale.ae.length = 0;
        this.historyControl?.destroy();
        this.historyControl = null;
        this.el.querySelectorAll('.diag-scale0-root, .diag-scale1-root, .diag-ae-root')
            .forEach((root) => root.remove());
        delete this.el.dataset.panelRedesignMounted;
        if (this.el._ftdDiagnosticsPanel === this) this.el._ftdDiagnosticsPanel = null;
    }
}

export function initDiagnosticsPanel() {
    const el = document.getElementById('panel-diagnostics');
    if (!el) return null;
    if (el._ftdDiagnosticsPanel) return el._ftdDiagnosticsPanel;
    const component = new DiagnosticsPanelComponent(el).init();
    el._ftdDiagnosticsPanel = component;
    return component;
}

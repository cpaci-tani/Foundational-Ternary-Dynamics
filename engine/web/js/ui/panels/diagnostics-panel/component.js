/**
 * DiagnosticsPanelComponent — composes scale-specific diagnostics tables from
 * descriptors. Scale 1 also keeps the detailed PE telemetry block below the
 * descriptor summary for per-particle/orbital drill-down.
 */

import { DiagnosticsTable } from './table.js';
import { sections as scale0Sections } from './descriptors/scale0.js';
import { sections as scale1Sections } from './descriptors/scale1.js';
import { sections as scale2Sections } from './descriptors/scale2.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';

export class DiagnosticsPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.tables = [];
        this.tablesByScale = { '0': [], '1': [], ae: [] };
    }

    init() {
        if (!this.el) return this;
        if (!this.el.dataset.panelRedesignMounted) {
            const scale0Root = document.createElement('div');
            scale0Root.className = 'scale0-only diag-scale0-root';
            for (const section of scale0Sections) {
                const table = new DiagnosticsTable(section, telemetryHub, { resetScope: 0 });
                scale0Root.appendChild(table.el);
                this.tables.push(table);
                this.tablesByScale['0'].push(table);
            }
            this.el.insertBefore(scale0Root, this.el.firstChild);

            const scale1Root = document.createElement('div');
            scale1Root.className = 'scale1-only diag-scale1-root';
            for (const section of scale1Sections) {
                const table = new DiagnosticsTable(section, telemetryHub, { resetScope: 1 });
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
                const table = new DiagnosticsTable(section, telemetryHub, { resetScope: 2 });
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
        const group = scale === '1'
            ? this.tablesByScale['1']
            : (scale === '2' || scale === '3')
                ? this.tablesByScale.ae
                : (scale === '0' ? this.tablesByScale['0'] : null);
        if (!group) return;
        for (const t of group) t.update();
    }

    cleanup() {
        for (const t of this.tables) t.destroy();
        this.tables.length = 0;
        this.tablesByScale['0'].length = 0;
        this.tablesByScale['1'].length = 0;
        this.tablesByScale.ae.length = 0;
    }
}

export function initDiagnosticsPanel() {
    const el = document.getElementById('panel-diagnostics');
    return el ? new DiagnosticsPanelComponent(el).init() : null;
}

/**
 * DiagnosticsPanelComponent — composes Scale 0 diagnostics tables from the
 * scale0 descriptor. Non-Scale-0 content (Scale 1 PE telemetry, etc.) is
 * still rendered by the legacy panel-resources template until descriptors
 * are added for those scales.
 */

import { DiagnosticsTable } from './table.js';
import { sections as scale0Sections } from './descriptors/scale0.js';
import { telemetryHub } from '../../../telemetry-hub.js';
import { PerfFlags } from '../../../config/perf-flags.js';
import { isPanelLive } from '../panel-visibility.js';

export class DiagnosticsPanelComponent {
    constructor(panelEl) {
        this.el = panelEl;
        this.tables = [];
    }

    init() {
        if (!this.el) return this;
        if (!this.el.dataset.panelRedesignMounted) {
            const scale0Root = document.createElement('div');
            scale0Root.className = 'scale0-only diag-scale0-root';
            for (const section of scale0Sections) {
                const table = new DiagnosticsTable(section, telemetryHub);
                scale0Root.appendChild(table.el);
                this.tables.push(table);
            }
            this.el.insertBefore(scale0Root, this.el.firstChild);
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
        for (const t of this.tables) t.update();
    }

    cleanup() {
        for (const t of this.tables) t.destroy();
        this.tables.length = 0;
    }
}

export function initDiagnosticsPanel() {
    const el = document.getElementById('panel-diagnostics');
    return el ? new DiagnosticsPanelComponent(el).init() : null;
}

/**
 * ChartCard — wraps a chart descriptor entry in a .chart-card DOM node
 * and owns the UPlotChart lifecycle.
 *
 *   const card = new ChartCard(descriptor, hub);
 *   parentEl.appendChild(card.el);
 *   card.update();
 *   card.destroy();
 */

import { UPlotChart } from '../../charts/uplot-chart.js';
import { attachFullscreen } from '../../charts/chart-fullscreen.js';

const GROUP_LABELS = Object.freeze({
    diagnostics: 'state',
    audit: 'audit',
    lagrangian: 'action',
    gravity: 'gravity',
});

export function resolveChartTelemetryGroups(descriptor) {
    if (Array.isArray(descriptor.telemetryGroups) && descriptor.telemetryGroups.length) {
        return [...new Set(descriptor.telemetryGroups.filter(Boolean))];
    }
    return descriptor.telemetryGroup ? [descriptor.telemetryGroup] : [];
}

export function getChartFreshnessPresentation(hub, groups) {
    const entries = groups.map((group) => {
        const meta = hub.getScale0TelemetryMeta(group);
        const current = !!meta && meta.stale !== true && Number.isFinite(meta.tick);
        return {
            group,
            label: GROUP_LABELS[group] || group,
            tick: current ? meta.tick : null,
            current,
        };
    });
    const currentCount = entries.filter(entry => entry.current).length;
    let state = 'waiting';
    if (currentCount === entries.length && currentCount > 0) {
        state = new Set(entries.map(entry => entry.tick)).size > 1 ? 'mixed' : 'current';
    } else if (currentCount > 0) {
        state = 'mixed-waiting';
    }
    const text = entries.length === 1
        ? (entries[0].current ? `t${entries[0].tick}` : 'waiting')
        : entries.map(entry => `${entry.label} ${entry.current ? `t${entry.tick}` : 'waiting'}`).join(' · ');
    return { state, text };
}

export class ChartCard {
    constructor(descriptor, hub, historyControl = null) {
        this.descriptor = descriptor;
        this.hub        = hub;
        this.telemetryGroups = resolveChartTelemetryGroups(descriptor);
        this._freshnessStamp = '';

        this.el = document.createElement('article');
        this.el.className = 'chart-card';
        this.el.dataset.chartId = descriptor.id;

        this.el.innerHTML = `
            <header class="chart-card-head">
                <h3 class="chart-card-title">${descriptor.title}</h3>
                ${this.telemetryGroups.length
                    ? '<span class="chart-card-freshness" aria-live="polite">waiting</span>' : ''}
                <button type="button" class="chart-card-expand"
                    title="View fullscreen (Esc to close)"
                    aria-label="View ${descriptor.title} fullscreen">⛶</button>
            </header>
            <div class="chart-card-plot"></div>
        `;

        attachFullscreen(this.el);

        const plotEl = this.el.querySelector('.chart-card-plot');
        this.freshnessEl = this.el.querySelector('.chart-card-freshness');
        this.chart = new UPlotChart(plotEl, {
            id:     descriptor.id,
            title:  '',
            tooltipTitle: descriptor.title,
            series: descriptor.series,
            xLabel: descriptor.xLabel,
            yLabel: descriptor.yLabel,
            hub,
            historyControl,
        });
        requestAnimationFrame(() => this.el.classList.add('is-mounted'));
    }

    update() {
        if (this.telemetryGroups.length
            && typeof this.hub.getScale0TelemetryMeta === 'function') {
            const presentation = getChartFreshnessPresentation(this.hub, this.telemetryGroups);
            const stamp = `${presentation.state}|${presentation.text}`;
            if (stamp !== this._freshnessStamp) {
                this._freshnessStamp = stamp;
                if (this.freshnessEl) this.freshnessEl.textContent = presentation.text;
                const waiting = presentation.state === 'waiting'
                    || presentation.state === 'mixed-waiting';
                this.el.classList.toggle('chart-card-telemetry-stale', waiting);
                this.el.dataset.telemetryState = presentation.state;
            }
        }
        this.chart.update();
    }

    destroy() {
        if (this.el._ftdCard?._isFullscreen) this.el._ftdCard._exitFullscreen();
        this.chart.destroy();
        this.el.remove();
    }
}

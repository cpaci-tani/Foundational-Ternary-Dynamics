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

export class ChartCard {
    constructor(descriptor, hub) {
        this.descriptor = descriptor;
        this.hub        = hub;
        this.el = document.createElement('article');
        this.el.className = 'chart-card';
        this.el.dataset.chartId = descriptor.id;
        this.el.innerHTML = `
            <header class="chart-card-head">
                <h3 class="chart-card-title">${descriptor.title}</h3>
            </header>
            <div class="chart-card-plot"></div>
        `;
        const plotEl = this.el.querySelector('.chart-card-plot');
        this.chart = new UPlotChart(plotEl, {
            id:     descriptor.id,
            title:  '',
            series: descriptor.series,
            xLabel: descriptor.xLabel,
            yLabel: descriptor.yLabel,
            hub,
        });
        requestAnimationFrame(() => this.el.classList.add('is-mounted'));
    }

    update() { this.chart.update(); }

    destroy() {
        this.chart.destroy();
        this.el.remove();
    }
}

import { getChartsPanelTemplate } from './template.js';

/**
 * Charts Panel Component
 * Owns #panel-charts markup and lifecycle.
 */
export class ChartsPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    if (!this.el.dataset.componentMounted) {
      this.el.innerHTML = getChartsPanelTemplate();
      this.el.dataset.componentMounted = '1';
    }
    this.el.dataset.component = 'charts-panel';
    return this;
  }

  cleanup() {}
}

export function initChartsPanel() {
  const el = document.getElementById('panel-charts');
  return el ? new ChartsPanelComponent(el).init() : null;
}

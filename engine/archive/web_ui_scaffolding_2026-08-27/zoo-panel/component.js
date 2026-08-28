/**
 * Zoo Panel Component
 * Wraps #panel-zoo and owns lifecycle for future migration.
 */
export class ZooPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'zoo-panel';
    return this;
  }

  cleanup() {}
}

export function initZooPanel() {
  const el = document.getElementById('panel-zoo');
  return el ? new ZooPanelComponent(el).init() : null;
}

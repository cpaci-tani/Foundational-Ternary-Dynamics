/**
 * Ontic Panel Component
 * Wraps #panel-ontic and owns lifecycle for future migration.
 */
export class OnticPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'ontic-panel';
    return this;
  }

  cleanup() {}
}

export function initOnticPanel() {
  const el = document.getElementById('panel-ontic');
  return el ? new OnticPanelComponent(el).init() : null;
}

/**
 * Inspector Panel Component
 * Wraps #panel-inspector and owns lifecycle for future migration.
 */
export class InspectorPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'inspector-panel';
    return this;
  }

  cleanup() {}
}

export function initInspectorPanel() {
  const el = document.getElementById('panel-inspector');
  return el ? new InspectorPanelComponent(el).init() : null;
}

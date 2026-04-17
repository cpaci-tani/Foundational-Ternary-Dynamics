/**
 * Hierarchy Panel Component
 * Wraps #panel-hierarchy and owns lifecycle for future migration.
 */
export class HierarchyPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'hierarchy-panel';
    return this;
  }

  cleanup() {}
}

export function initHierarchyPanel() {
  const el = document.getElementById('panel-hierarchy');
  return el ? new HierarchyPanelComponent(el).init() : null;
}

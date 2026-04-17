/**
 * Meta Info Panel Component
 * Wraps #panel-meta-info and owns lifecycle for future migration.
 */
export class MetaInfoPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'meta-info-panel';
    return this;
  }

  cleanup() {}
}

export function initMetaInfoPanel() {
  const el = document.getElementById('panel-meta-info');
  return el ? new MetaInfoPanelComponent(el).init() : null;
}

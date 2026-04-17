/**
 * Cosmic Info Panel Component
 * Wraps #panel-cosmic-info and owns lifecycle for future migration.
 */
export class CosmicInfoPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'cosmic-info-panel';
    return this;
  }

  cleanup() {}
}

export function initCosmicInfoPanel() {
  const el = document.getElementById('panel-cosmic-info');
  return el ? new CosmicInfoPanelComponent(el).init() : null;
}

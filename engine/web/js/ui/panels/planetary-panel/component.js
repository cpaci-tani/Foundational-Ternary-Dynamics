/**
 * Planetary Panel Component
 * Wraps #panel-planetary and owns lifecycle for future migration.
 */
export class PlanetaryPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'planetary-panel';
    return this;
  }

  cleanup() {}
}

export function initPlanetaryPanel() {
  const el = document.getElementById('panel-planetary');
  return el ? new PlanetaryPanelComponent(el).init() : null;
}

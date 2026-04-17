/**
 * Physics Panel Component
 * Wraps #panel-physics and owns lifecycle for future migration.
 */
export class PhysicsPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'physics-panel';
    return this;
  }

  cleanup() {}
}

export function initPhysicsPanel() {
  const el = document.getElementById('panel-physics');
  return el ? new PhysicsPanelComponent(el).init() : null;
}

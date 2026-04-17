import { getLagrangianPanelTemplate } from './template.js';

/**
 * Lagrangian Panel Component
 * Owns #panel-lagrangian markup and lifecycle.
 */
export class LagrangianPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    if (!this.el.dataset.componentMounted) {
      this.el.innerHTML = getLagrangianPanelTemplate();
      this.el.dataset.componentMounted = '1';
    }
    this.el.dataset.component = 'lagrangian-panel';
    return this;
  }

  cleanup() {}
}

export function initLagrangianPanel() {
  const el = document.getElementById('panel-lagrangian');
  return el ? new LagrangianPanelComponent(el).init() : null;
}

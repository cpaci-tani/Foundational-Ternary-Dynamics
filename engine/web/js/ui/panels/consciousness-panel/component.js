import { getConsciousnessPanelTemplate } from './template.js';

/**
 * Consciousness Panel Component
 * Owns #panel-consciousness markup and lifecycle.
 */
export class ConsciousnessPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    if (!this.el.dataset.componentMounted) {
      this.el.innerHTML = getConsciousnessPanelTemplate();
      this.el.dataset.componentMounted = '1';
    }
    this.el.dataset.component = 'consciousness-panel';
    return this;
  }

  cleanup() {}
}

export function initConsciousnessPanel() {
  const el = document.getElementById('panel-consciousness');
  return el ? new ConsciousnessPanelComponent(el).init() : null;
}

/**
 * Diagnostics Panel Component
 * Wraps #panel-diagnostics and owns lifecycle for future migration.
 */
export class DiagnosticsPanelComponent {
  constructor(panelEl) {
    this.el = panelEl;
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'diagnostics-panel';
    return this;
  }

  cleanup() {}
}

export function initDiagnosticsPanel() {
  const el = document.getElementById('panel-diagnostics');
  return el ? new DiagnosticsPanelComponent(el).init() : null;
}

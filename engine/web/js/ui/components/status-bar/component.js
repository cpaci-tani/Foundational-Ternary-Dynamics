/**
 * Status Bar Component
 * Wraps #status-bar footer and its 9 child DOM nodes.
 */
export class StatusBarComponent {
  constructor(el) {
    this.el = el;
    this.dot       = el?.querySelector('#status-dot');
    this.state     = el?.querySelector('#status-state');
    this.tick      = el?.querySelector('#status-tick');
    this.ptime     = el?.querySelector('#status-ptime');
    this.particles = el?.querySelector('#status-particles');
    this.energy    = el?.querySelector('#status-energy');
    this.fps       = el?.querySelector('#status-fps');
    this.engine    = el?.querySelector('#status-engine');
    this.compute   = el?.querySelector('#status-compute');
  }

  init() {
    if (!this.el) return this;
    this.el.dataset.component = 'status-bar';
    return this;
  }

  cleanup() {}
}

export function initStatusBar() {
  const el = document.getElementById('status-bar');
  return el ? new StatusBarComponent(el).init() : null;
}

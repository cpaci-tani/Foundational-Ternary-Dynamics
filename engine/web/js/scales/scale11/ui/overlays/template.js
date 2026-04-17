/**
 * Scale 11 Viewport Overlay — Consciousness observer
 */

export function createScale11OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'consciousness-viewport-overlay';
  container.className = 'scale11-only viewport-overlay-panel';
  container.style.alignItems = 'center';
  container.innerHTML = `
    <span class="cs-overlay-status">
      Observable: <span id="cs-observable-pct">37%</span> &middot;
      &theta;<sub>C</sub> = <span id="cs-theta-val">52.5&deg;</span>
    </span>
  `;
  return container;
}

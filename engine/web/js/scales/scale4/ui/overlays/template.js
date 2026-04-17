/**
 * Scale 4 Viewport Overlay — Planetary sandbox
 */

export function createScale4OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'cs-viewport-overlay';
  container.className = 'scale4-only viewport-overlay-panel';
  container.style.alignItems = 'center';
  container.innerHTML = `
    <span class="scale4-overlay-status">
      Orbital mechanics simulator
    </span>
  `;
  return container;
}

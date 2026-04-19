/**
 * Scale 5 Viewport Overlay — Cosmic engine
 */

export function getScale5OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'cosmic-viewport-overlay';
  container.className = 'scale5-only viewport-overlay-panel';
  container.innerHTML = '';
  return container;
}

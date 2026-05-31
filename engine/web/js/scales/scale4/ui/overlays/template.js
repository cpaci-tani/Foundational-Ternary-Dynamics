/**
 * Scale 4 Viewport Overlay — Planetary sandbox
 */

export function getScale4OverlayTemplate() {
  const container = document.createElement('div');
  container.id = 'cs-viewport-overlay';
  container.className = 'scale4-only viewport-overlay-panel';
  container.style.alignItems = 'center';
  // Status text is set dynamically by the Scale-4 controller to reflect the
  // active gravity mode (decorative vs physical). The default copy below is
  // mode-neutral and deliberately avoids asserting AU/yr timing fidelity,
  // which only holds in 'physical' mode (P0-1).
  container.innerHTML = `
    <span class="scale4-overlay-status" id="planetary-overlay-status">
      Orbital mechanics demo
    </span>
  `;
  return container;
}

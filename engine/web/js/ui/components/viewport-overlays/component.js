/**
 * Viewport Overlays Component — mounts scale-specific and universal overlay controls
 *
 * Orchestrates:
 * - Scale-specific field/visualization toggles
 * - Universal axes/grid controls
 * - Bottom bar environment + boundary selectors
 *
 * Top-offset strategy:
 * Chrome's compositor freezes the rendered position of elements inside a WebGL
 * canvas sibling (the Three.js canvas promotes adjacent elements to GPU layers).
 * CSS custom-property changes and even inline style overrides on already-inserted
 * elements are silently ignored.  The only reliable fix is to:
 *   1. Set a concrete px `style.top` on each element BEFORE appending it so the
 *      layer is created at the correct position from the start.
 *   2. For subsequent toolbar-height changes (resize observer), remove the element
 *      and re-append it — this destroys and recreates the compositor layer.
 */

import { getScale0OverlayTemplate } from '../../../scales/scale0/ui/overlays/template.js';
import { getScale1OverlayTemplate } from '../../../scales/scale1/ui/overlays/template.js';
import { getScale2OverlayTemplate, getScale2LegendTemplate } from '../../../scales/scale2/ui/overlays/template.js';
import { getScale3OverlayTemplate, getScale3LegendTemplate } from '../../../scales/scale3/ui/overlays/template.js';
import { getScale4OverlayTemplate } from '../../../scales/scale4/ui/overlays/template.js';
import { getScale5OverlayTemplate } from '../../../scales/scale5/ui/overlays/template.js';

export class ViewportOverlaysComponent {
  constructor(viewportEl) {
    this.viewport = viewportEl;
    this.overlays = new Map();
    this._legends = [];
  }

  init() {
    if (!this.viewport) return this;

    const append = (el) => {
      return this.viewport.appendChild(el);
    };

    this.overlays.set('scale0', append(getScale0OverlayTemplate()));
    this.overlays.set('scale1', append(getScale1OverlayTemplate()));

    const scale2 = getScale2OverlayTemplate();
    append(scale2);
    this.overlays.set('scale2', scale2);
    const legend2 = getScale2LegendTemplate();
    this._legends.push(legend2);
    this.viewport.appendChild(legend2);

    const scale3 = getScale3OverlayTemplate();
    append(scale3);
    this.overlays.set('scale3', scale3);
    const legend3 = getScale3LegendTemplate();
    this._legends.push(legend3);
    this.viewport.appendChild(legend3);

    this.overlays.set('scale4', append(getScale4OverlayTemplate()));
    this.overlays.set('scale5', append(getScale5OverlayTemplate()));

    this._mountUniversalOverlays();
    this._wireCollapsibles();

    return this;
  }

  /**
   * Wire collapse behavior for every viewport overlay. Scale 0 uses its
   * built-in .s0-overlay-collapse header button; other scales get a small
   * button injected at the top of their panel. State persists per scale
   * in localStorage (key: ftd.overlay.<scaleKey>.collapsed).
   */
  _wireCollapsibles() {
    for (const [scaleKey, el] of this.overlays) {
      if (!el) continue;
      const lsKey = `ftd.overlay.${scaleKey}.collapsed`;
      const isS0 = el.classList.contains('s0-overlay-panel');
      let btn;

      if (isS0) {
        btn = el.querySelector('.s0-overlay-collapse');
      } else {
        // Inject a generic collapse button as the first child of the overlay.
        btn = document.createElement('button');
        btn.type = 'button';
        btn.className = 'viewport-overlay-collapse';
        btn.setAttribute('aria-label', 'Collapse overlay');
        btn.setAttribute('aria-expanded', 'true');
        btn.title = 'Collapse overlay';
        btn.innerHTML = '<span class="viewport-overlay-collapse-icon" aria-hidden="true">&#9652;</span>';
        el.prepend(btn);
      }
      if (!btn) continue;

      const apply = (collapsed) => {
        el.classList.toggle('is-collapsed', !!collapsed);
        btn.setAttribute('aria-expanded', collapsed ? 'false' : 'true');
        btn.title = collapsed ? 'Expand overlay' : 'Collapse overlay';
      };

      // Restore persisted state
      try { apply(localStorage.getItem(lsKey) === '1'); } catch { /* ignore */ }

      btn.addEventListener('click', (ev) => {
        ev.stopPropagation();
        const next = !el.classList.contains('is-collapsed');
        apply(next);
        try { localStorage.setItem(lsKey, next ? '1' : '0'); } catch { /* ignore */ }
      });
    }
  }
  _mountUniversalOverlays() {
    // Unified viewport controls panel — all universal overlays in one glass card
    const panel = document.createElement('div');
    panel.id = 'viewport-controls-panel';
    panel.innerHTML = `
      <div class="vcp-header">
        <span class="vcp-title">SCENE</span>
        <button class="vcp-collapse-btn" aria-label="Toggle Visuals" title="Toggle Visuals">&#128065;&#xFE0E;</button>
      </div>
      <div class="vcp-content">
        <div class="vcp-section">
          <div class="vcp-toggle-grid overlays-grid">
            <button class="view-toggle active scale4-hide" id="toggle-axes" title="XYZ axis indicator">Axes</button>
            <button class="view-toggle active scale4-hide" id="toggle-grid" title="Reference grid (XZ plane)">Grid</button>
            <button class="view-toggle" id="toggle-reflective" title="Reflective boundary conditions">Reflect</button>
          </div>
        </div>

        <div class="vcp-section scale0-only">
          <div class="vcp-label-row"><span class="vcp-label">Camera</span></div>
          <div class="vcp-preset-grid">
            <button class="view-toggle vcp-preset-btn" data-cam-preset="front" title="Face-on view (looking -Z)">Front</button>
            <button class="view-toggle vcp-preset-btn" data-cam-preset="side"  title="Side view (looking -X)">Side</button>
            <button class="view-toggle vcp-preset-btn" data-cam-preset="top"   title="Top-down view (looking -Y)">Top</button>
            <button class="view-toggle vcp-preset-btn" data-cam-preset="corner" title="Diagonal corner view">Corner</button>
          </div>
        </div>

        <div class="vcp-section scale0-only">
          <div class="vcp-label-row"><span class="vcp-label">Flux</span></div>
          <div class="vcp-toggle-grid flux-grid">
            <button class="view-toggle active" id="toggle-flux-organic-scene" title="Organic scatter (cloud) vs regular lattice grid">Organic</button>
            <button class="view-toggle active" id="toggle-flux-glow-scene" title="Additive glow bloom on the flux volume">Glow</button>
          </div>
        </div>

        <div class="vcp-section">
          <div class="vcp-select-row">
            <span class="vcp-label">Env</span>
            <select id="bg-select">
              <option value="none">None</option>
              <optgroup label="Cosmic">
                <option value="stars" selected>Star Field</option>
                <option value="nebula">Nebula</option>
                <option value="foam">Quantum Foam</option>
                <option value="beyond">The Beyond</option>
                <option value="storm">Flux Storm</option>
              </optgroup>
              <optgroup label="360°">
                <option value="studio">Studio</option>
                <option value="workshop">Workshop</option>
                <option value="sunset">Sunset</option>
                <option value="night">Night Sky</option>
                <option value="forest">Forest</option>
                <option value="urban">Urban</option>
              </optgroup>
            </select>
          </div>
          <div class="vcp-select-row">
            <span class="vcp-label">Bounds</span>
            <select id="boundary-select">
              <option value="cube" selected>Cube</option>
              <option value="sphere">Sphere</option>
              <option value="dodecahedron">Dodecahedron</option>
              <option value="icosahedron">Icosahedron</option>
              <option value="octahedron">Octahedron</option>
              <option value="cylinder">Cylinder</option>
              <option value="torus">Torus</option>
              <option value="none">None</option>
            </select>
          </div>
        </div>
      </div>
    `;
    this.viewport.appendChild(panel);

    // Wire up universal panel collapse state
    const btn = panel.querySelector('.vcp-collapse-btn');
    const lsKey = 'ftd.overlay.universal.collapsed';
    const apply = (collapsed) => {
        panel.classList.toggle('is-collapsed', !!collapsed);
        btn.setAttribute('aria-expanded', collapsed ? 'false' : 'true');
    };
    try { apply(localStorage.getItem(lsKey) === '1'); } catch { /* ignore */ }
    btn.addEventListener('click', (ev) => {
        ev.stopPropagation();
        const next = !panel.classList.contains('is-collapsed');
        apply(next);
        try { localStorage.setItem(lsKey, next ? '1' : '0'); } catch { /* ignore */ }
    });
  }

  cleanup() {
    this.overlays.clear();
    this._legends = [];
  }
}

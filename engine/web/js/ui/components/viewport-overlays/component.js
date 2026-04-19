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
import { getScale11OverlayTemplate } from '../../../scales/scale11/ui/overlays/template.js';

export class ViewportOverlaysComponent {
  constructor(viewportEl) {
    this.viewport = viewportEl;
    this.overlays = new Map();
    this._legends = [];
    this._toolbarH = 0;
  }

  init() {
    if (!this.viewport) return this;

    // Read toolbar height directly — TopbarComponent has already run and measured it.
    // We use the concrete px value rather than a CSS custom property because Chrome's
    // compositor layer (triggered by the adjacent WebGL canvas) freezes the rendered
    // position and ignores subsequent CSS/style changes on already-inserted elements.
    const toolbar = document.getElementById('toolbar');
    this._toolbarH = toolbar ? toolbar.getBoundingClientRect().height : 0;

    const append = (el) => {
      this._applyTop(el);
      return this.viewport.appendChild(el);
    };

    this.overlays.set('scale0', append(getScale0OverlayTemplate()));
    this.overlays.set('scale1', append(getScale1OverlayTemplate()));

    const scale2 = getScale2OverlayTemplate();
    append(scale2);
    this.overlays.set('scale2', scale2);
    const legend2 = getScale2LegendTemplate();
    this._applyTop(legend2);
    this._legends.push(legend2);
    this.viewport.appendChild(legend2);

    const scale3 = getScale3OverlayTemplate();
    append(scale3);
    this.overlays.set('scale3', scale3);
    const legend3 = getScale3LegendTemplate();
    this._applyTop(legend3);
    this._legends.push(legend3);
    this.viewport.appendChild(legend3);

    this.overlays.set('scale4', append(getScale4OverlayTemplate()));
    this.overlays.set('scale5', append(getScale5OverlayTemplate()));
    this.overlays.set('scale11', append(getScale11OverlayTemplate()));

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

  /** Apply the correct top offset as a concrete px inline style. */
  _applyTop(el) {
    const gapPx = 12;
    el.style.top = `${this._toolbarH + gapPx}px`;
  }

  /**
   * Called by AppShell when the toolbar height changes (ResizeObserver).
   * Re-inserts each overlay so the compositor layer is recreated at the new position.
   */
  updateTopOffset(toolbarH) {
    this._toolbarH = toolbarH;
    const allTopEls = [...this.overlays.values(), ...this._legends];
    for (const el of allTopEls) {
      if (!el.parentElement) continue;
      const parent = el.parentElement;
      const next = el.nextSibling;
      parent.removeChild(el);
      this._applyTop(el);
      parent.insertBefore(el, next);
    }
  }

  _mountUniversalOverlays() {
    // Unified viewport controls panel — all universal overlays in one glass card
    const panel = document.createElement('div');
    panel.id = 'viewport-controls-panel';
    panel.innerHTML = `
      <div class="vcp-toggle-grid">
        <button class="view-toggle active scale4-hide" id="toggle-axes" title="XYZ axis indicator">Axes</button>
        <button class="view-toggle active scale4-hide" id="toggle-grid" title="Reference grid (XZ plane)">Grid</button>
        <button class="view-toggle active" id="toggle-reflective" title="Reflective boundary conditions">Reflect</button>
      </div>
      <!-- Camera preset buttons — snap the orbit camera to a named viewpoint.
           All positions are computed from the current lattice size, so the
           preset reads the same at every N. Only shows for Scale 0 (lattice
           mode); other scales have their own camera logic. -->
      <div class="vcp-label-row scale0-only"><span class="vcp-label">Camera</span></div>
      <div class="vcp-preset-grid scale0-only">
        <button class="view-toggle vcp-preset-btn" data-cam-preset="front" title="Face-on view (looking -Z)">Front</button>
        <button class="view-toggle vcp-preset-btn" data-cam-preset="side"  title="Side view (looking -X)">Side</button>
        <button class="view-toggle vcp-preset-btn" data-cam-preset="top"   title="Top-down (looking -Y)">Top</button>
        <button class="view-toggle vcp-preset-btn" data-cam-preset="iso"   title="Isometric default">Iso</button>
        <button class="view-toggle vcp-preset-btn" data-cam-preset="moore" title="Zoomed into the 3×3×3 Moore neighborhood around the lattice center — ideal for seed scenarios">Moore</button>
        <button class="view-toggle vcp-preset-btn" data-cam-preset="fit"   title="Frame the active flux volume to fit the viewport">Fit</button>
      </div>
      <!-- Reduced motion toggle — freezes all continuous UI animations
           (particle smooth-follow tweens, dash flow, quantum breathing).
           Useful for screenshots and for users sensitive to motion. Sets
           body[data-reduced-motion="1"] + broadcasts a custom event so
           any listener can honour the preference.  -->
      <div class="vcp-toggle-grid vcp-motion-row">
        <button class="view-toggle" id="toggle-reduced-motion"
            title="Freeze all continuous animation (breathing, dash flow). Overlay/physics simulation is unaffected.">
          Reduced Motion
        </button>
      </div>
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
        <span class="vcp-label">Boundary</span>
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
    `;
    this.viewport.appendChild(panel);
  }

  cleanup() {
    this.overlays.clear();
    this._legends = [];
  }
}

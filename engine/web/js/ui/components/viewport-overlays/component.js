/**
 * Viewport Overlays Component — mounts scale-specific and universal overlay controls
 *
 * Orchestrates:
 * - Scale-specific field/visualization toggles
 * - Universal axes/grid controls
 * - Bottom status-bar scene/environment controls
 */

import { getScale0OverlayTemplate } from '../../../scales/scale0/ui/overlays/template.js?v=5';
import { getScale0StandardModelHudTemplate } from '../../../scales/scale0/ui/overlays/standard-model.js?v=2';
import { getScale1OverlayTemplate } from '../../../scales/scale1/ui/overlays/template.js?v=6';
import { getScale2OverlayTemplate } from '../../../scales/scale2/ui/overlays/template.js';
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
    this._legends.push(append(getScale0StandardModelHudTemplate()));
    this.overlays.set('scale1', append(getScale1OverlayTemplate()));

    const scale2 = getScale2OverlayTemplate();
    append(scale2);
    this.overlays.set('scale2', scale2);
    this.overlays.set('scale3', scale2);

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
    const wired = new Set();
    for (const [scaleKey, el] of this.overlays) {
      if (!el || el.hidden || wired.has(el)) continue;
      wired.add(el);
      const lsKey = `ftd.overlay.${scaleKey}.collapsed`;
      const isS0 = el.classList.contains('s0-overlay-panel');
      let btn;

      if (isS0) {
        btn = el.querySelector('.s0-overlay-collapse');
      } else {
        btn = document.createElement('button');
        btn.type = 'button';
        btn.className = 'viewport-overlay-collapse';
        btn.setAttribute('aria-label', 'Collapse overlay');
        btn.setAttribute('aria-expanded', 'true');
        btn.title = 'Collapse overlay';
        btn.innerHTML = '<span class="viewport-overlay-collapse-icon" aria-hidden="true">&#9652;</span>';
        const slot = el.querySelector('.scale-overlay-collapse-slot');
        if (slot) slot.appendChild(btn);
        else el.prepend(btn);
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
    const statusBar = document.getElementById('status-bar');
    if (!statusBar || document.getElementById('status-scene-controls')) return;

    const controls = document.createElement('div');
    controls.id = 'status-scene-controls';
    controls.className = 'status-item status-scene-controls';
    controls.innerHTML = `
      <span class="status-scene-label">Scene</span>

      <details class="status-menu">
        <summary class="status-menu-trigger">View</summary>
        <div class="status-menu-panel status-menu-grid status-menu-grid-2" role="group" aria-label="Scene view toggles">
          <button class="view-toggle active scale4-hide" id="toggle-axes" type="button" aria-pressed="true" title="XYZ axis indicator">Axes</button>
          <button class="view-toggle active scale4-hide" id="toggle-grid" type="button" aria-pressed="true" title="Reference grid (XZ plane)">Grid</button>
          <button class="view-toggle active scale0-only" id="toggle-boundary-orientation" type="button" aria-pressed="true" title="External boundary-orientation arrows">Arrows</button>
          <button class="view-toggle active scale0-only" id="toggle-global-clock" type="button" aria-pressed="true" title="Global ordinal tick, ordered transaction pulse, and selected causal-rate color">Clock</button>
          <button class="view-toggle scale0-only status-scene-flow" id="scene-force-flow" type="button"
              data-style="flow" aria-pressed="false" title="Render active force overlays as animated streamlines">
            <span class="status-scene-flow-icon" aria-hidden="true">&#8767;</span> Flow
          </button>
        </div>
      </details>

      <details class="status-menu scale0-only">
        <summary class="status-menu-trigger">Camera</summary>
        <div class="status-menu-panel status-menu-grid status-menu-grid-2" role="group" aria-label="Camera presets">
          <button class="view-toggle status-preset-btn" type="button" data-cam-preset="front" title="Face-on view (looking -Z)">Front</button>
          <button class="view-toggle status-preset-btn" type="button" data-cam-preset="side" title="Side view (looking -X)">Side</button>
          <button class="view-toggle status-preset-btn" type="button" data-cam-preset="top" title="Top-down view (looking -Y)">Top</button>
          <button class="view-toggle status-preset-btn" type="button" data-cam-preset="corner" title="Diagonal corner view">Corner</button>
        </div>
      </details>

      <label class="status-select-label" for="bg-select">
        <span>Env</span>
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
      </label>

      <label class="status-select-label" for="boundary-select">
        <span>Bounds</span>
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
      </label>
    `;

    const fpsItem = document.getElementById('status-fps')?.closest('.status-item');
    statusBar.insertBefore(controls, fpsItem || null);
    this._wireStatusMenus(controls);
  }

  _wireStatusMenus(root) {
    const menus = Array.from(root.querySelectorAll('.status-menu'));
    const closeOthers = (current) => {
      for (const menu of menus) {
        if (menu !== current) menu.removeAttribute('open');
      }
    };

    for (const menu of menus) {
      menu.querySelector('summary')?.addEventListener('click', () => closeOthers(menu));
      menu.addEventListener('toggle', () => {
        if (menu.open) closeOthers(menu);
      });
    }

    document.addEventListener('click', (ev) => {
      if (root.contains(ev.target)) return;
      closeOthers(null);
    });

    document.addEventListener('keydown', (ev) => {
      if (ev.key !== 'Escape') return;
      closeOthers(null);
    });
  }

  cleanup() {
    this.overlays.clear();
    this._legends = [];
  }
}

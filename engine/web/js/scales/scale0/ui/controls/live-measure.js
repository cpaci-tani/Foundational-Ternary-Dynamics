/** Controls for the live rulers, energy string, voxel clocks, and orbit smoothing. */
import { LifetimeScope } from '../../../../ui/utils/lifetime-scope.js';

export function createLiveMeasureCard() {
    const card = document.createElement('div');
    card.className = 'card scale0-only';
    card.innerHTML = `
    <div class="card-title">Live Measure</div>
    <p>Sizes and visibility for the rulers, energy string, and voxel clocks. Presentation only.</p>

    <div class="toggle-row"><input type="checkbox" id="lm-view-ruler" checked><label for="lm-view-ruler">View ruler</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-lattice-ruler" checked><label for="lm-lattice-ruler">Lattice ruler</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-energy-string" checked><label for="lm-energy-string">Voxel energy string</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-spectrum" checked><label for="lm-spectrum">Point-size color</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-voxel-clocks" checked><label for="lm-voxel-clocks">Voxel clocks</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-moore-bars" checked><label for="lm-moore-bars">Neighborhood size bars</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-moore-waves" checked><label for="lm-moore-waves">Neighborhood wave lines</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-moore-joules" checked><label for="lm-moore-joules">Neighborhood joules</label></div>
    <div class="toggle-row"><input type="checkbox" id="lm-smooth-orbit" checked><label for="lm-smooth-orbit">Smooth orbit</label></div>

    <div class="combo-section-label">Ruler scale</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-ruler-scale" min="0.6" max="2" step="0.05" value="1">
      <span class="pe-ctrl-value" id="lm-ruler-scale-val">1.00</span>
    </div>

    <div class="combo-section-label">Clock size</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-clock-size" min="8" max="36" step="1" value="16">
      <span class="pe-ctrl-value" id="lm-clock-size-val">16</span>
    </div>

    <div class="combo-section-label">String width</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-string-width" min="64" max="280" step="2" value="148">
      <span class="pe-ctrl-value" id="lm-string-width-val">148</span>
    </div>

    <div class="combo-section-label">Value text</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-value-size" min="12" max="32" step="1" value="16">
      <span class="pe-ctrl-value" id="lm-value-size-val">16</span>
    </div>

    <div class="combo-section-label">Line thickness</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-line-thickness" min="0.5" max="4" step="0.25" value="1.5">
      <span class="pe-ctrl-value" id="lm-line-thickness-val">1.50</span>
    </div>

    <div class="combo-section-label">Bar thickness</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-bar-thickness" min="2" max="16" step="1" value="8">
      <span class="pe-ctrl-value" id="lm-bar-thickness-val">8</span>
    </div>

    <div class="combo-section-label">Clock reach</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="lm-clock-radius" min="0.5" max="6" step="0.1" value="2.5">
      <span class="pe-ctrl-value" id="lm-clock-radius-val">2.5</span>
    </div>
  `;
    return card;
}

export function wireLiveMeasureCard(ctx) {
    const scope = new LifetimeScope();
    const viewport = () => ctx.viewport;
    const bindCheck = (id, key) => {
        const input = document.getElementById(id);
        if (!input) return;
        scope.on(input, 'change', () => viewport()?.setLiveMeasure?.({ [key]: input.checked }));
    };
    bindCheck('lm-view-ruler', 'viewRuler');
    bindCheck('lm-lattice-ruler', 'latticeRuler');
    bindCheck('lm-energy-string', 'energyString');
    bindCheck('lm-spectrum', 'spectrum');
    bindCheck('lm-voxel-clocks', 'voxelClocks');
    bindCheck('lm-moore-bars', 'mooreBars');
    bindCheck('lm-moore-waves', 'mooreWaves');
    bindCheck('lm-moore-joules', 'mooreJoules');
    bindCheck('lm-smooth-orbit', 'smoothOrbit');

    const bindRange = (id, valueId, key, digits) => {
        const input = document.getElementById(id);
        const label = document.getElementById(valueId);
        if (!input) return;
        scope.on(input, 'input', () => {
            const value = Number(input.value);
            if (label) label.textContent = value.toFixed(digits);
            viewport()?.setLiveMeasure?.({ [key]: value });
        });
    };
    bindRange('lm-ruler-scale', 'lm-ruler-scale-val', 'rulerScale', 2);
    bindRange('lm-clock-size', 'lm-clock-size-val', 'clockSize', 0);
    bindRange('lm-string-width', 'lm-string-width-val', 'stringWidth', 0);
    bindRange('lm-clock-radius', 'lm-clock-radius-val', 'clockRadius', 1);
    bindRange('lm-value-size', 'lm-value-size-val', 'valueSize', 0);
    bindRange('lm-line-thickness', 'lm-line-thickness-val', 'lineThickness', 2);
    bindRange('lm-bar-thickness', 'lm-bar-thickness-val', 'barThickness', 0);
    return () => scope.dispose();
}

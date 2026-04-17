/**
 * Scale 0 Flux Volume Card
 */

export function createFluxVolumeCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.innerHTML = `
    <div class="card-title">Flux Volume</div>

    <div class="combo-section-label">Shape</div>
    <select class="tb-select ctrl-select-full" id="flux-shape-select">
      <option value="0" selected>Circle</option>
      <option value="1">Square</option>
      <option value="2">Diamond</option>
      <option value="3">Star</option>
      <option value="4">Triangle</option>
      <option value="5">Hexagon</option>
      <option value="6">Ring</option>
      <option value="7">Cross</option>
    </select>

    <div class="combo-section-label">Opacity</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-opacity" min="0.0" max="1.0" step="0.01" value="0.70">
      <span class="pe-ctrl-value" id="flux-opacity-val">0.70</span>
    </div>

    <div class="combo-section-label">Point Size</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-point-scale" min="0.1" max="3.0" step="0.05" value="1.0">
      <span class="pe-ctrl-value" id="flux-point-scale-val">1.0</span>
    </div>

    <div class="combo-section-label">Scenario Scale</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-scenario-scale" min="0.1" max="5.0" step="0.1" value="1.0">
      <span class="pe-ctrl-value" id="flux-scenario-scale-val">1.0</span>
    </div>

    <div class="combo-section-label">Threshold</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-threshold" min="0.0" max="0.1" step="0.001" value="0.005">
      <span class="pe-ctrl-value" id="flux-threshold-val">0.005</span>
    </div>

    <div class="combo-section-label" title="Visual spacing multiplier between rendered lattice voxels (does not change the physics)">Lattice Spacing</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-lattice-spacing" min="0.25" max="3.0" step="0.05" value="1.0">
      <span class="pe-ctrl-value" id="flux-lattice-spacing-val">1.00</span>
    </div>
  `;
  return card;
}

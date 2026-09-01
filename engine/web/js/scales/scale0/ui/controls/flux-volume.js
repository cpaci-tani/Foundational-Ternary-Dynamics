/**
 * Scale 0 Flux Volume Card
 */

import {
  fluxThresholdToSliderPosition,
  formatFluxThreshold,
} from '../../../../viewport/flux-threshold.js';

const DEFAULT_FLUX_THRESHOLD = 0.005;

export function createSelectionCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.id = 'sel-card';
  card.innerHTML = `
    <div class="card-title">Selection</div>

    <div class="combo-section-label">Position</div>
    <div class="ctrl-row ctrl-row-compact">
      <div class="coord-stepper" data-axis="x">
        <label class="ctrl-label ctrl-label-xs axis-x-text">X</label>
        <input type="number" class="ctrl-input ctrl-input-coord" id="sel-x" value="16" min="0" max="31" aria-label="Selection x">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step sel-coord-step" data-step="1" data-for="sel-x" aria-label="Increment x">&#9650;</button>
          <button type="button" class="coord-step sel-coord-step" data-step="-1" data-for="sel-x" aria-label="Decrement x">&#9660;</button>
        </div>
      </div>
      <div class="coord-stepper" data-axis="y">
        <label class="ctrl-label ctrl-label-xs axis-y-text">Y</label>
        <input type="number" class="ctrl-input ctrl-input-coord" id="sel-y" value="16" min="0" max="31" aria-label="Selection y">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step sel-coord-step" data-step="1" data-for="sel-y" aria-label="Increment y">&#9650;</button>
          <button type="button" class="coord-step sel-coord-step" data-step="-1" data-for="sel-y" aria-label="Decrement y">&#9660;</button>
        </div>
      </div>
      <div class="coord-stepper" data-axis="z">
        <label class="ctrl-label ctrl-label-xs axis-z-text">Z</label>
        <input type="number" class="ctrl-input ctrl-input-coord" id="sel-z" value="16" min="0" max="31" aria-label="Selection z">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step sel-coord-step" data-step="1" data-for="sel-z" aria-label="Increment z">&#9650;</button>
          <button type="button" class="coord-step sel-coord-step" data-step="-1" data-for="sel-z" aria-label="Decrement z">&#9660;</button>
        </div>
      </div>
    </div>

    <div class="combo-section-label combo-section-spaced">Navigate Axis</div>
    <div class="sel-axis-nav">
      <div class="sel-axis-pair">
        <button type="button" class="sel-axis-btn axis-x-text" data-axis="x" data-dir="-1">−X</button>
        <button type="button" class="sel-axis-btn axis-x-text" data-axis="x" data-dir="1">+X</button>
      </div>
      <div class="sel-axis-pair">
        <button type="button" class="sel-axis-btn axis-y-text" data-axis="y" data-dir="-1">−Y</button>
        <button type="button" class="sel-axis-btn axis-y-text" data-axis="y" data-dir="1">+Y</button>
      </div>
      <div class="sel-axis-pair">
        <button type="button" class="sel-axis-btn axis-z-text" data-axis="z" data-dir="-1">−Z</button>
        <button type="button" class="sel-axis-btn axis-z-text" data-axis="z" data-dir="1">+Z</button>
      </div>
    </div>

    <div class="selection-card-actions">
      <button class="tb-btn selection-area-toggle" id="sel-area-toggle" data-active="false">◻ Area</button>
      <button class="tb-btn selection-submit" id="btn-select">▶ SELECT</button>
    </div>

    <div id="sel-area-controls" class="selection-area-controls" hidden>
      <div class="combo-section-label">Radius</div>
      <div class="ctrl-slider-row">
        <input type="range" class="pe-slider" id="sel-radius" min="1" max="10" step="1" value="2">
        <span class="pe-ctrl-value" id="sel-radius-val">2</span>
      </div>
    </div>
  `;
  return card;
}

export function createParticleDisplayCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.innerHTML = `
    <div class="card-title">Particle Display</div>

    <div class="combo-section-label">Shape</div>
    <select class="tb-select ctrl-select-full" id="particle-shape-select">
      <option value="0" selected>Circle</option>
      <option value="1">Square</option>
      <option value="2">Diamond</option>
      <option value="3">Star</option>
      <option value="4">Triangle</option>
      <option value="5">Hexagon</option>
      <option value="6">Ring</option>
      <option value="7">Cross</option>
    </select>

    <div class="combo-section-label is-positive">+ Positive Size</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="particle-pos-size" min="1" max="40" step="0.5" value="14">
      <span class="pe-ctrl-value" id="particle-pos-size-val">14.0</span>
    </div>

    <div class="combo-section-label is-negative">− Negative Size</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="particle-neg-size" min="1" max="40" step="0.5" value="10">
      <span class="pe-ctrl-value" id="particle-neg-size-val">10.0</span>
    </div>

    <div class="combo-section-label">Opacity</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="particle-opacity" min="0.0" max="1.0" step="0.01" value="0.90">
      <span class="pe-ctrl-value" id="particle-opacity-val">0.90</span>
    </div>

    <div class="combo-section-label">Glow</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="particle-glow" min="0.0" max="0.6" step="0.01" value="0.15">
      <span class="pe-ctrl-value" id="particle-glow-val">0.15</span>
    </div>
  `;
  return card;
}

export function createFluxVolumeCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  const thresholdPosition = fluxThresholdToSliderPosition(DEFAULT_FLUX_THRESHOLD);
  const thresholdText = formatFluxThreshold(DEFAULT_FLUX_THRESHOLD);
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

    <div class="combo-section-label" title="[PROXY — visualization] Relative local activation-energy cutoff. Each voxel combines its own 1/2|J|², the mean energy of its surrounding 26 Moore neighbours, and |s|E_REST. Every available voxel is evaluated; only voxels below this cutoff are hidden. Point size and colour phase increase with the same energy signal.">Threshold</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-threshold" min="0.0" max="0.5" step="0.0001" value="${thresholdPosition}" aria-label="Relative local activation-energy threshold" aria-valuetext="${thresholdText}">
      <span class="pe-ctrl-value" id="flux-threshold-val">${thresholdText}</span>
    </div>

    <div class="combo-section-label" title="Visual spacing multiplier between rendered lattice voxels (does not change the physics)">Lattice Spacing</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flux-lattice-spacing" min="0.25" max="3.0" step="0.05" value="1.0">
      <span class="pe-ctrl-value" id="flux-lattice-spacing-val">1.00</span>
    </div>

    <div class="combo-section-label" title="Brightness of the bounding wireframe (opacity of the boundary lines)">Wireframe Brightness</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="wireframe-brightness" min="0.0" max="1.0" step="0.01" value="0.18">
      <span class="pe-ctrl-value" id="wireframe-brightness-val">0.18</span>
    </div>
  `;
  return card;
}

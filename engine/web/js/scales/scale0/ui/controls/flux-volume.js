/**
 * Scale 0 Flux Volume Card
 */

export function createSelectionCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.id = 'sel-card';
  card.innerHTML = `
    <div class="card-title">Selection</div>

    <div class="combo-section-label">Position</div>
    <div class="ctrl-row ctrl-row-compact">
      <div class="coord-stepper" data-axis="x">
        <label class="ctrl-label ctrl-label-xs" style="color:#f87171">X</label>
        <input type="number" class="ctrl-input ctrl-input-coord" id="sel-x" value="16" min="0" max="31" aria-label="Selection x">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step sel-coord-step" data-step="1" data-for="sel-x" aria-label="Increment x">&#9650;</button>
          <button type="button" class="coord-step sel-coord-step" data-step="-1" data-for="sel-x" aria-label="Decrement x">&#9660;</button>
        </div>
      </div>
      <div class="coord-stepper" data-axis="y">
        <label class="ctrl-label ctrl-label-xs" style="color:#4ade80">Y</label>
        <input type="number" class="ctrl-input ctrl-input-coord" id="sel-y" value="16" min="0" max="31" aria-label="Selection y">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step sel-coord-step" data-step="1" data-for="sel-y" aria-label="Increment y">&#9650;</button>
          <button type="button" class="coord-step sel-coord-step" data-step="-1" data-for="sel-y" aria-label="Decrement y">&#9660;</button>
        </div>
      </div>
      <div class="coord-stepper" data-axis="z">
        <label class="ctrl-label ctrl-label-xs" style="color:#60a5fa">Z</label>
        <input type="number" class="ctrl-input ctrl-input-coord" id="sel-z" value="16" min="0" max="31" aria-label="Selection z">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step sel-coord-step" data-step="1" data-for="sel-z" aria-label="Increment z">&#9650;</button>
          <button type="button" class="coord-step sel-coord-step" data-step="-1" data-for="sel-z" aria-label="Decrement z">&#9660;</button>
        </div>
      </div>
    </div>

    <div class="combo-section-label" style="margin-top:8px">Navigate Axis</div>
    <div class="sel-axis-nav">
      <div class="sel-axis-pair">
        <button class="sel-axis-btn" data-axis="x" data-dir="-1" style="color:#f87171">−X</button>
        <button class="sel-axis-btn" data-axis="x" data-dir="1"  style="color:#f87171">+X</button>
      </div>
      <div class="sel-axis-pair">
        <button class="sel-axis-btn" data-axis="y" data-dir="-1" style="color:#4ade80">−Y</button>
        <button class="sel-axis-btn" data-axis="y" data-dir="1"  style="color:#4ade80">+Y</button>
      </div>
      <div class="sel-axis-pair">
        <button class="sel-axis-btn" data-axis="z" data-dir="-1" style="color:#60a5fa">−Z</button>
        <button class="sel-axis-btn" data-axis="z" data-dir="1"  style="color:#60a5fa">+Z</button>
      </div>
    </div>

    <div style="display:flex;gap:8px;margin-top:10px;align-items:center">
      <button class="tb-btn" id="sel-area-toggle" data-active="false"
              style="flex:1;font-size:11px;padding:4px 6px">◻ Area</button>
      <button class="tb-btn" id="btn-select"
              style="flex:2;font-size:12px;padding:5px 8px;background:rgba(56,189,248,0.15);border-color:#38bdf8;color:#38bdf8">▶ SELECT</button>
    </div>

    <div id="sel-area-controls" style="display:none;margin-top:8px">
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

    <div class="combo-section-label" style="color:#6fe88a">+ Positive Size</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="particle-pos-size" min="1" max="40" step="0.5" value="14">
      <span class="pe-ctrl-value" id="particle-pos-size-val">14.0</span>
    </div>

    <div class="combo-section-label" style="color:#f87070">− Negative Size</div>
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

    <div class="combo-section-label" title="Brightness of the bounding wireframe (opacity of the boundary lines)">Wireframe Brightness</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="wireframe-brightness" min="0.0" max="1.0" step="0.01" value="0.18">
      <span class="pe-ctrl-value" id="wireframe-brightness-val">0.18</span>
    </div>
  `;
  return card;
}

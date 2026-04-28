/**
 * Scale 0 Substrate Controls Card
 */

import { K_B } from '../../../../constants.js';

export function createSubstrateControlsCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  const kbStr = K_B.toFixed(3);
  card.innerHTML = `
    <div class="card-title">Substrate Controls</div>
    <div class="combo-section-label">Inject</div>
    <div class="combo-btn-row">
      <button class="ctrl-btn-secondary" id="btn-inject" title="Inject point particle">Particle</button>
      <button class="ctrl-btn-secondary" id="btn-inject-wave" title="Inject wavepacket">Wave</button>
      <button class="ctrl-btn-secondary" id="btn-inject-flux" title="Inject flux at position">Flux</button>
      <button class="ctrl-btn-secondary" id="btn-inject-pair" title="Create entangled pair">Pair</button>
    </div>
    <div class="ctrl-row ctrl-row-compact">
      <span class="ctrl-label ctrl-label-xs">Pos</span>
      <div class="coord-stepper" data-axis="x">
        <input type="number" class="ctrl-input ctrl-input-coord" id="inj-x" value="16" min="0" aria-label="Inject x coordinate">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step" data-step="1" data-for="inj-x" aria-label="Increment x">&#9650;</button>
          <button type="button" class="coord-step" data-step="-1" data-for="inj-x" aria-label="Decrement x">&#9660;</button>
        </div>
      </div>
      <div class="coord-stepper" data-axis="y">
        <input type="number" class="ctrl-input ctrl-input-coord" id="inj-y" value="16" min="0" aria-label="Inject y coordinate">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step" data-step="1" data-for="inj-y" aria-label="Increment y">&#9650;</button>
          <button type="button" class="coord-step" data-step="-1" data-for="inj-y" aria-label="Decrement y">&#9660;</button>
        </div>
      </div>
      <div class="coord-stepper" data-axis="z">
        <input type="number" class="ctrl-input ctrl-input-coord" id="inj-z" value="16" min="0" aria-label="Inject z coordinate">
        <div class="coord-stepper-btns">
          <button type="button" class="coord-step" data-step="1" data-for="inj-z" aria-label="Increment z">&#9650;</button>
          <button type="button" class="coord-step" data-step="-1" data-for="inj-z" aria-label="Decrement z">&#9660;</button>
        </div>
      </div>
    </div>
    <div class="ctrl-row ctrl-row-compact">
      <span class="ctrl-label ctrl-label-xs">State</span>
      <div class="combo-btn-row ctrl-btn-row-fill">
        <button class="ctrl-btn-secondary active ctrl-btn-flex-1" id="inj-state-pos" title="Positive manifestation">+1</button>
        <button class="ctrl-btn-secondary ctrl-btn-flex-1" id="inj-state-neg" title="Negative manifestation">&minus;1</button>
      </div>
      <button class="ctrl-btn-secondary ctrl-btn-compact" id="btn-center" title="Center position">Center</button>
      <button class="ctrl-btn-secondary ctrl-btn-compact" id="btn-random" title="Randomize position and inject a wavepacket">Rand</button>
    </div>

    <div class="combo-section-label">Parameters</div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label ctrl-label-md" title="Genesis Threshold (K_B)">K<sub>B</sub> (Thresh)</span>
      <input type="range" class="pe-slider" id="combo-kb" min="0.05" max="2.0" step="0.01" value="${kbStr}">
      <span class="pe-ctrl-value" id="combo-kb-val">${kbStr}</span>
    </div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label ctrl-label-md" title="Gravitational Constant (G_N)">G<sub>N</sub> (Gravity)</span>
      <input type="range" class="pe-slider" id="combo-gn" min="0.0" max="0.1" step="0.001" value="0.010">
      <span class="pe-ctrl-value" id="combo-gn-val">0.010</span>
    </div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label ctrl-label-sm">Damp</span>
      <input type="range" class="pe-slider" id="combo-damp" min="0.0" max="0.05" step="0.0001" value="0.0073">
      <span class="pe-ctrl-value" id="combo-damp-val">0.0073</span>
    </div>

    <div class="combo-section-label">Field</div>
    <div class="combo-btn-row">
      <button class="ctrl-btn-secondary" id="btn-clear-field">Clear Field</button>
      <button class="ctrl-btn-secondary" id="btn-random-flux">Random Flux</button>
    </div>
  `;
  return card;
}

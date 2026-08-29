/** Shared Flux / electric / magnetic streamline presentation controls. */
export function createFlowLinesCard() {
  const card = document.createElement('div');
  card.className = 'card scale0-only';
  card.id = 'flow-lines-card';
  card.innerHTML = `
    <div class="card-title">Flow Lines</div>

    <div class="combo-section-label">Density</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flow-line-density"
             min="0.25" max="1" step="0.05" value="1"
             aria-label="Flux, electric, and magnetic flow-line density">
      <span class="pe-ctrl-value" id="flow-line-density-val">100%</span>
    </div>

    <div class="combo-section-label">Line Length</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flow-line-length"
             min="0.4" max="1" step="0.05" value="1"
             aria-label="Flux, electric, and magnetic flow-line length">
      <span class="pe-ctrl-value" id="flow-line-length-val">100%</span>
    </div>

    <div class="combo-section-label">Opacity</div>
    <div class="ctrl-slider-row">
      <input type="range" class="pe-slider" id="flow-line-opacity"
             min="0.2" max="1" step="0.05" value="0.7"
             aria-label="Flux, electric, and magnetic flow-line opacity">
      <span class="pe-ctrl-value" id="flow-line-opacity-val">70%</span>
    </div>

    <div class="ctrl-footnote">
      <span class="ctrl-footnote-text" id="flow-line-budget"
            title="Audited maximum integration work for the current lattice size"></span>
      <button type="button" class="ctrl-reset-btn" id="flow-line-reset">Reset</button>
    </div>
    <div class="ctrl-footnote-text" style="margin-top:6px;font-size:var(--fs-xs)">
      Shared by Flux Lines, Radiative E Field, and B Field. Density and length
      rebuild the next coherent sweep; opacity updates immediately.
    </div>
  `;
  return card;
}

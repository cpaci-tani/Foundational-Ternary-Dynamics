/**
 * Scale 0 Floating Symmetry Panel
 * Shows U(1)/SU(2)/SU(3) aggregation toggles.
 */

export function mountSymmetryPanel(parentEl) {
  if (!parentEl || document.getElementById('floating-symmetry-panel')) return;

  const panel = document.createElement('div');
  panel.id = 'floating-symmetry-panel';
  panel.className = 'scale0-only sym-panel';
  panel.innerHTML = `
    <div class="sym-panel-title">Symmetry Aggregation</div>
    <label class="sym-panel-label"><input type="checkbox" id="sym-u1"> U(1) [Faces - 6]</label>
    <label class="sym-panel-label"><input type="checkbox" id="sym-su2"> SU(2) [Edges - 12]</label>
    <label class="sym-panel-label"><input type="checkbox" id="sym-su3"> SU(3) [Corners - 8]</label>
  `;
  parentEl.appendChild(panel);
  return panel;
}

/**
 * Scale 0 Floating Symmetry Panel
 *
 * The U(1)/SU(2)/SU(3) aggregation checkboxes were stubbed but never
 * wired to any handler (audit W10 / E §wiring, 2026-05-27). The
 * checkboxes are now disabled and labelled as pending so users don't
 * expect interactivity. To re-enable, add event listeners that
 * surface the symmetry aggregation data via the Inspector or the
 * conservation micropanel.
 */

export function mountSymmetryPanel(parentEl) {
  if (!parentEl || document.getElementById('floating-symmetry-panel')) return;

  const panel = document.createElement('div');
  panel.id = 'floating-symmetry-panel';
  panel.className = 'scale0-only sym-panel';
  panel.innerHTML = `
    <div class="sym-panel-title" title="Symmetry aggregation toggles (pending wiring — audit W10 / 2026-05-27)">Symmetry Aggregation (pending)</div>
    <label class="sym-panel-label"><input type="checkbox" id="sym-u1" disabled> U(1) [Faces - 6]</label>
    <label class="sym-panel-label"><input type="checkbox" id="sym-su2" disabled> SU(2) [Edges - 12]</label>
    <label class="sym-panel-label"><input type="checkbox" id="sym-su3" disabled> SU(3) [Corners - 8]</label>
  `;
  parentEl.appendChild(panel);
  return panel;
}

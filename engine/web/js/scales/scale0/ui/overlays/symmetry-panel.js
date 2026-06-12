import { BaseComponent } from '../../../../core/component.js';

const TEMPLATE = `
    <div id="floating-symmetry-panel" class="scale0-only sym-panel">
        <div class="sym-panel-title" title="Symmetry aggregation toggles (pending wiring — audit W10 / 2026-05-27)">Symmetry Aggregation (pending)</div>
        <label class="sym-panel-label"><input type="checkbox" id="sym-u1" disabled> U(1) [Faces - 6]</label>
        <label class="sym-panel-label"><input type="checkbox" id="sym-su2" disabled> SU(2) [Edges - 12]</label>
        <label class="sym-panel-label"><input type="checkbox" id="sym-su3" disabled> SU(3) [Corners - 8]</label>
    </div>
`;

export class SymmetryPanelComponent extends BaseComponent {
    constructor() {
        super(TEMPLATE);
    }
}

export function mountSymmetryPanel(parentEl) {
    if (!parentEl || document.getElementById('floating-symmetry-panel')) return;

    const comp = new SymmetryPanelComponent();
    comp.mount(parentEl);
    return comp.element;
}

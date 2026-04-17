/**
 * TermRow — renders the Lagrangian term-toggle row. Each checkbox mirrors
 * its corresponding uPlot legend series; the StackedAreaChart pushes
 * state back via an onLegendChange callback.
 *
 *   const row = new TermRow(terms, { onToggle(key, checked) {…} });
 *   parentEl.appendChild(row.el);
 *   row.setChecked(key, bool); // called from chart legend sync
 */

export class TermRow {
    constructor(terms, { onToggle }) {
        this.terms = terms;
        this.onToggle = onToggle;
        this.inputs  = new Map();

        this.el = document.createElement('div');
        this.el.className = 'lag-term-row';

        for (const term of terms) {
            const label = document.createElement('label');
            label.className = 'lag-term-toggle';
            label.dataset.term = term.key;
            label.style.setProperty('--legend-color', term.color);
            label.innerHTML = `
                <input type="checkbox" ${term.includeByDefault ? 'checked' : ''}>
                <span class="lag-term-swatch" aria-hidden="true"></span>
                <span class="lag-term-label">${term.label}</span>
            `;
            const input = label.querySelector('input');
            input.addEventListener('change', () => this.onToggle(term.key, input.checked));
            this.inputs.set(term.key, input);
            this.el.appendChild(label);
        }
    }

    setChecked(key, checked) {
        const input = this.inputs.get(key);
        if (input && input.checked !== checked) input.checked = checked;
    }
}

/**
 * Scale 5 Controls Component
 *
 * Mounts cosmic control cards into the dedicated Scale-5 controls grid.
 *
 * Pass 0b (UI foundations, 2026-09-09): a single placeholder "Dynamics"
 * card — correct mounting, targeting, idempotence and teardown are what
 * this pass verifies. Pass A grows this into the full gravity/dynamics
 * card (sliders for gravitational coupling, softening, timestep, and the
 * speed limit) per the plan.
 *
 * Mirrors scales/scale2/ui/controls/component.js, but targets the stable
 * id `panel-controls-grid-scale5` (added to the `.scale5-only
 * .panel-grid-3` block in ui/components/panel-resources/template.js)
 * rather than a generic `.panel-grid.panel-grid-3` selector — a selector
 * that broad would also match the Scale-4 controls block, the bug
 * documented at scale2/ui/controls/component.js:22-24. Because the target
 * is already scoped by `.scale5-only` in the static template, no fresh
 * container needs to be created here the way Scale 2's component creates
 * `#panel-controls-grid` on first use.
 */

const DYNAMICS_CARD_KEY = 'dynamics-defaults';

function createDynamicsCard() {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.scale5ControlCard = DYNAMICS_CARD_KEY;
    card.innerHTML = `
        <div class="card-title">Dynamics</div>
        <div class="scale-info-copy">
            <div title="Every phenomenological rule toggle (gas cooling, star formation, Bondi accretion, radiation pressure, tidal stretch and disruption, Hawking evaporation, stellar evolution, mergers, emergent black-hole formation) is initialized from THIS scenario's own setting on load, per the plan's owner decision. This button re-runs that seeding without a scene reload.">
                Rule toggles are seeded from the loaded scenario on load. Reset them to that baseline at any time.
            </div>
        </div>
        <button type="button" class="ctrl-btn" id="cosmic-ctrl-reset-toggles" title="Re-runs the bridge's own scenario-toggle sync and refreshes the toolbar checkboxes; does not reload the scene or reseed body state.">Scenario defaults</button>
    `;
    return card;
}

export class Scale5ControlsComponent {
    constructor(panelControlsDiv) {
        this.panel = panelControlsDiv;
    }

    init() {
        if (!this.panel) return this;

        // Target the dedicated, already scale-scoped host (a stable id on
        // the .scale5-only block) rather than a generic .panel-grid-3
        // selector, which would also match the Scale-4 controls block.
        const gridContainer = this.panel.querySelector('#panel-controls-grid-scale5');
        if (!gridContainer) return this;

        // Reconcile by the stable data-scale5-control-card key so re-entry
        // (mount() on every scale switch, plus loadCosmicScenario() on every
        // scenario change) never duplicates the card.
        if (!gridContainer.querySelector(`[data-scale5-control-card="${DYNAMICS_CARD_KEY}"]`)) {
            gridContainer.appendChild(createDynamicsCard());
        }

        return this;
    }
}

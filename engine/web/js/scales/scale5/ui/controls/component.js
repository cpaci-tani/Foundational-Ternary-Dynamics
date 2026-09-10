/**
 * Scale 5 Controls Component
 *
 * Mounts cosmic control cards into the dedicated Scale-5 controls grid.
 *
 * Pass 0b (UI foundations, 2026-09-09): a single placeholder "Dynamics"
 * card — correct mounting, targeting, idempotence and teardown are what
 * this pass verifies.
 *
 * Pass A (2026-09-10) grows the placeholder into the real gravity/dynamics
 * card: a `speed_limit` toggle (this pass's one toggle-registry key to
 * wire — see SCALE5_TOGGLES; it had no UI surface before this pass) plus
 * four sliders (gravitational-coupling multiplier, softening multiplier,
 * integrator timestep, and the speed-limit multiplier). The speed_limit
 * checkbox carries a `data-scale5-toggle` attribute and is bound through
 * the SAME shared toggle-sync module Pass B used for the Gas card's
 * checkboxes; the four sliders are bridge fields/live setters with no
 * second UI surface, so — mirroring the Gas card's alpha/beta/adaptive-h
 * precedent exactly — scale5/controller.js binds them directly rather than
 * through the toggle registry. The "Scenario defaults" button and its
 * explanatory copy are unchanged from Pass 0b.
 *
 * Pass B (2026-09-10) adds the "Gas" card beside it: the existing SPH
 * toggle, viscosity alpha/beta, an adaptive-smoothing toggle, and a legacy
 * gas-repulsion toggle. The SPH and legacy-repulsion checkboxes carry
 * `data-scale5-toggle` attributes and are bound through the shared
 * toggle-sync module (ui/toggle-sync.js, Ruling J2) so this card's
 * `#cosmic-gas-sph` checkbox and the scenario toolbar's `#t-sph-monaghan`
 * checkbox — two surfaces for one `sph_monaghan` key — can never drift
 * apart; scale5/controller.js binds the two range sliders and the
 * adaptive-smoothing checkbox directly (they have no second surface).
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
import { bindScale5ToggleCheckboxes } from '../toggle-sync.js';

const DYNAMICS_CARD_KEY = 'dynamics-defaults';
const GAS_CARD_KEY = 'gas-sph';

function createDynamicsCard() {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.scale5ControlCard = DYNAMICS_CARD_KEY;
    card.innerHTML = `
        <div class="card-title">Dynamics</div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-dynamics-speed-limit" data-scale5-toggle="speed_limit">
            <label for="cosmic-dynamics-speed-limit" title="[SELECTION] The lattice speed limit c = 1/sqrt(3) caps signal speed on the substrate. When on, any body whose speed this tick exceeds it (scaled by the multiplier below) is rescaled back under it; when off, bodies may exceed the lattice light speed uncorrected.">Speed limit</label>
        </div>
        <label class="pe-ctrl-row" title="[IMPOSED] Live multiplier on G_N [IMPOSED] in the gravity kernel; 1 is the scenario default.">
            <span class="pe-ctrl-label">Gravity G_N &times;</span>
            <input type="range" class="pe-slider" id="cosmic-dynamics-gravity" min="0" max="3" step="0.05" value="1.0">
            <span class="pe-ctrl-value" id="cosmic-dynamics-gravity-value">1.00</span>
        </label>
        <label class="pe-ctrl-row" title="[IMPOSED] Live multiplier on the per-type gravitational softening length, the regularization that keeps the 1/r^2 force finite at short range; 1 is the scenario default.">
            <span class="pe-ctrl-label">Softening &times;</span>
            <input type="range" class="pe-slider" id="cosmic-dynamics-softening" min="0.1" max="3" step="0.05" value="1.0">
            <span class="pe-ctrl-value" id="cosmic-dynamics-softening-value">1.00</span>
        </label>
        <label class="pe-ctrl-row" title="[IMPOSED] Fixed Velocity-Verlet integrator substep, in the lattice-internal time unit (not a physical second).">
            <span class="pe-ctrl-label">Timestep dt</span>
            <input type="range" class="pe-slider" id="cosmic-dynamics-dt" min="0.001" max="0.05" step="0.001" value="0.01">
            <span class="pe-ctrl-value" id="cosmic-dynamics-dt-value">0.010</span>
        </label>
        <label class="pe-ctrl-row" title="[SELECTION] Live multiplier on the lattice speed limit c = 1/sqrt(3) used by the speed-limit clamp above; 1 is the scenario default.">
            <span class="pe-ctrl-label">Speed limit &times;</span>
            <input type="range" class="pe-slider" id="cosmic-dynamics-speed-limit-factor" min="0.1" max="3" step="0.05" value="1.0">
            <span class="pe-ctrl-value" id="cosmic-dynamics-speed-limit-factor-value">1.00</span>
        </label>
        <div class="scale-info-copy">
            <div title="Every phenomenological rule toggle (gas cooling, star formation, Bondi accretion, radiation pressure, tidal stretch and disruption, Hawking evaporation, stellar evolution, mergers, emergent black-hole formation) is initialized from THIS scenario's own setting on load, per the plan's owner decision. This button re-runs that seeding without a scene reload.">
                Rule toggles are seeded from the loaded scenario on load. Reset them to that baseline at any time.
            </div>
        </div>
        <button type="button" class="ctrl-btn" id="cosmic-ctrl-reset-toggles" title="Re-runs the bridge's own scenario-toggle sync and refreshes the toolbar checkboxes; does not reload the scene or reseed body state.">Scenario defaults</button>
    `;
    return card;
}

function createGasCard() {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.scale5ControlCard = GAS_CARD_KEY;
    card.innerHTML = `
        <div class="card-title">Gas</div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-gas-sph" data-scale5-toggle="sph_monaghan">
            <label for="cosmic-gas-sph" title="[IMPOSED effective gas dynamics; coefficients in engine units] Monaghan (1992) SPH gas pressure and artificial viscosity for GAS/NEBULA bodies. Mirrors the scenario toolbar's SPH gas checkbox — the two stay in sync.">SPH gas dynamics</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-gas-legacy-repulsion" data-scale5-toggle="legacy_gas_repulsion">
            <label for="cosmic-gas-legacy-repulsion" title="[IMPOSED effective gas dynamics; coefficients in engine units] Ad-hoc gas-gas repulsion term. Only fires while SPH gas dynamics above is OFF, mirroring the bridge's own guard (cosmic-physics.js).">Legacy gas repulsion (SPH off only)</label>
        </div>
        <label class="pe-ctrl-row" title="[IMPOSED effective gas dynamics; coefficients in engine units] Overrides the frozen SPH.ALPHA artificial-viscosity coefficient (linear term) for the Monaghan gas pass.">
            <span class="pe-ctrl-label">Viscosity &alpha;</span>
            <input type="range" class="pe-slider" id="cosmic-gas-alpha" min="0" max="4" step="0.1" value="1.0">
            <span class="pe-ctrl-value" id="cosmic-gas-alpha-value">1.0</span>
        </label>
        <label class="pe-ctrl-row" title="[IMPOSED effective gas dynamics; coefficients in engine units] Overrides the frozen SPH.BETA artificial-viscosity coefficient (quadratic term) for the Monaghan gas pass.">
            <span class="pe-ctrl-label">Viscosity &beta;</span>
            <input type="range" class="pe-slider" id="cosmic-gas-beta" min="0" max="6" step="0.1" value="2.0">
            <span class="pe-ctrl-value" id="cosmic-gas-beta-value">2.0</span>
        </label>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-gas-adaptive-h" checked>
            <label for="cosmic-gas-adaptive-h" title="[IMPOSED effective gas dynamics; coefficients in engine units] When on, each gas body's SPH smoothing length h is recomputed from its local density every tick (h = ETA * cbrt(m/rho)). Turning this off freezes h at its current value.">Adaptive smoothing length</label>
        </div>
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
        let dynamicsCard = gridContainer.querySelector(`[data-scale5-control-card="${DYNAMICS_CARD_KEY}"]`);
        if (!dynamicsCard) {
            dynamicsCard = createDynamicsCard();
            gridContainer.appendChild(dynamicsCard);
        }
        // Idempotent per root (ui/toggle-sync.js tracks registered roots in
        // a Set) — binds this card's speed_limit checkbox (Pass A).
        bindScale5ToggleCheckboxes(dynamicsCard);

        let gasCard = gridContainer.querySelector(`[data-scale5-control-card="${GAS_CARD_KEY}"]`);
        if (!gasCard) {
            gasCard = createGasCard();
            gridContainer.appendChild(gasCard);
        }
        // Idempotent per root (ui/toggle-sync.js tracks registered roots in
        // a Set), so calling this on every init() is safe even when the
        // card already existed.
        bindScale5ToggleCheckboxes(gasCard);

        return this;
    }
}

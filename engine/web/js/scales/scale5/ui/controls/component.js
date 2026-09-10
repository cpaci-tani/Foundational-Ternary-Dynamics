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
 * Pass C (2026-09-10) adds the "Cosmology" card beside Dynamics and Gas: an
 * expansion on/off checkbox (a bridge setter, `setExpansionEnabled` — NOT a
 * SCALE5_TOGGLES registry key, so no `data-scale5-toggle` attribute and no
 * toggle-sync binding, exactly like the Gas card's adaptive-smoothing
 * checkbox), a clock-gain slider (bridge setter `setClockGain`, same shape
 * as the Dynamics/Gas sliders), and a dark-matter-fraction PRE-LOAD select
 * (Scale 4's `planetary-gravity-mode` is the precedent this mirrors: a
 * controller-scope field that survives bridge recreation, applied to the
 * FRESH bridge before setupScenario() runs, then the current scenario is
 * reloaded — DM_FRACTION is baked into body TYPES at construction, so there
 * is no live in-place retrofit). This card's static sibling, "Cosmology
 * (FTD)" (a pre-existing info card in the same grid, from before the
 * controls-card component existed), holds the fixed Omega_m/Omega_Lambda/
 * G_N/gamma/c constants with their load-bearing epistemic tooltips —
 * deliberately UNTOUCHED here (Ruling C-1): this card is live CONTROLS, not
 * a second copy of that info.
 *
 * Pass D (2026-09-10) adds the "Physics Rules" card beside Dynamics, Gas
 * and Cosmology: a labelled checkbox for each of the eleven remaining
 * SCALE5_TOGGLES keys (gas_cooling, radiation_pressure, tidal_stretch,
 * tidal_disruption, star_formation, bondi_accretion, horizon_absorption,
 * mergers, emergent_black_holes, stellar_evolution, hawking_evaporation —
 * sph_monaghan/legacy_gas_repulsion/speed_limit were already wired by
 * Passes B/A). Every checkbox carries a `data-scale5-toggle` attribute and
 * is bound through the SAME shared toggle-sync module as the other cards;
 * none of these eleven keys had ANY UI surface before this pass, so this
 * is each key's first checkbox, not a second surface for an existing one.
 * The card also renders the bridge's bounded event log (getEventLog(),
 * cap 200) as a small scrolling list — scale5/controller.js's
 * animateCosmic() writes into it every frame the same way it already
 * writes the other panel-status spans, since a multi-row scrolling list
 * has no natural fit in the one-value-per-row diagnostics table (see
 * descriptors/scale5.js's header comment for the fuller reasoning).
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
const COSMOLOGY_CARD_KEY = 'cosmology-expansion';
const PHYSICS_RULES_CARD_KEY = 'physics-rules';

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

function createCosmologyCard() {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.scale5ControlCard = COSMOLOGY_CARD_KEY;
    card.innerHTML = `
        <div class="card-title">Cosmology</div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-cosmology-expansion" checked>
            <label for="cosmic-cosmology-expansion" title="[MEASURED — instrument control] When on, the flat-&Lambda;CDM background integrator advances the scale factor a, Hubble rate H, and redshift z each tick. Turning it off FREEZES a/H/z at their current values; N-body dynamics are unaffected either way — the Friedmann step is diagnostics-only regardless of this setting.">Expansion (background clock)</label>
        </div>
        <label class="pe-ctrl-row" title="Display-only acceleration of the cosmic background clock (a, H, z) — NOT physics. Never touches the N-body force kernel or body kinematics; scales only how fast the universe crosses a=1 on screen. Default 40.">
            <span class="pe-ctrl-label">Clock gain &times;</span>
            <input type="range" class="pe-slider" id="cosmic-cosmology-clock-gain" min="0" max="200" step="1" value="40">
            <span class="pe-ctrl-value" id="cosmic-cosmology-clock-gain-value">40</span>
        </label>
        <label class="pe-ctrl-row" title="[SELECTION] Dark-matter mass fraction baked into body TYPES at scenario construction (galaxy-family scenarios only) — there is no in-place retrofit. Changing this RE-SEEDS the population by reloading the current scenario; it does NOT convert existing bodies. Default 17/27 &asymp; 63% does NOT match Planck 2018's observed &Omega;_DM/&Omega;_m &asymp; 84%.">
            <span class="pe-ctrl-label">DM fraction</span>
            <select id="cosmic-cosmology-dm-fraction">
                <option value="default" selected>Default (63%)</option>
                <option value="0.84">Planck (84%)</option>
                <option value="0.5">50 / 50</option>
                <option value="0">Baryon only</option>
            </select>
        </label>
        <div class="scale-info-copy">
            <div title="Reloads the current scenario with the newly-selected dark-matter fraction baked into the freshly-generated body population.">Changing DM fraction reseeds the scenario — it does not convert existing bodies.</div>
        </div>
    `;
    return card;
}

function createPhysicsRulesCard() {
    const card = document.createElement('div');
    card.className = 'card';
    card.dataset.scale5ControlCard = PHYSICS_RULES_CARD_KEY;
    card.innerHTML = `
        <div class="card-title">Physics Rules</div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-gas-cooling" data-scale5-toggle="gas_cooling">
            <label for="cosmic-rules-gas-cooling" title="[IMPOSED effective gas dynamics; coefficients in engine units] Radiative cooling force term applied to gas bodies (cosmic-physics.js). Independent of _enableSubgrid; not gated by the SPH pass.">Gas cooling</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-radiation-pressure" data-scale5-toggle="radiation_pressure">
            <label for="cosmic-rules-radiation-pressure" title="[IMPOSED effective gas dynamics; coefficients in engine units] Outward force on gas from nearby stellar luminosity (cosmic-physics.js).">Radiation pressure</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-tidal-stretch" data-scale5-toggle="tidal_stretch">
            <label for="cosmic-rules-tidal-stretch" title="[IMPOSED effective gas dynamics; coefficients in engine units] Radial tidal-stretch FORCE near a black hole (cosmic-physics.js). Distinct from Tidal disruption below, which is the separate mass-shedding state machine this force feeds into.">Tidal stretch (force)</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-tidal-disruption" data-scale5-toggle="tidal_disruption">
            <label for="cosmic-rules-tidal-disruption" title="[IMPOSED threshold rule] Mass-shedding state machine: sheds a star's mass to GAS once its accumulated tidal-stretch exceeds a fixed threshold near a black hole, and fully disrupts it below a fixed mass/stretch combination (cosmic-postupdates.js).">Tidal disruption</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-star-formation" data-scale5-toggle="star_formation">
            <label for="cosmic-rules-star-formation" title="[IMPOSED threshold rule] Sub-grid: converts a sufficiently dense, cold, crowded GAS body to a STAR with a fixed per-tick probability. Runs only when the scenario's own sub-grid flag (_enableSubgrid) is on.">Star formation</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-bondi-accretion" data-scale5-toggle="bondi_accretion">
            <label for="cosmic-rules-bondi-accretion" title="[IMPOSED threshold rule] Sub-grid: a Bondi-like capture-radius/rate formula transfers nearby gas mass onto a black hole. Runs only when _enableSubgrid is on; the Accretion capture-radius viewport overlay draws its r_acc.">Bondi accretion</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-horizon-absorption" data-scale5-toggle="horizon_absorption">
            <label for="cosmic-rules-horizon-absorption" title="[IMPOSED threshold rule] Swallows any body within a small internal sink radius of a black hole (cosmic-postupdates.js's r_sink — a numerically-bounded tuning parameter, NOT the displayed horizon-radius proxy). Always active regardless of _enableSubgrid.">Horizon absorption</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-mergers" data-scale5-toggle="mergers">
            <label for="cosmic-rules-mergers" title="[IMPOSED threshold rule] Merges two black holes into one (momentum-conserving, 5% mass-to-gravitational-waves) once their separation drops below a fixed mass-scaled radius. Always active regardless of _enableSubgrid.">BH-BH mergers</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-emergent-bh" data-scale5-toggle="emergent_black_holes">
            <label for="cosmic-rules-emergent-bh" title="[IMPOSED threshold rule] Converts the highest-enclosed-mass body within 2x softening to a BLACK_HOLE once its escape velocity exceeds the lattice speed limit c = 1/sqrt(3) AND its enclosed mass exceeds 50 -- at the labs' softening this reduces to just 'enclosed mass above 50'. NEVER a relativistic criterion. Pass D fix: this rule now ALSO requires the scenario's own sub-grid flag (_enableSubgrid) to be on, matching the other sub-grid rules -- it has no effect on the three gas laboratories (_enableSubgrid = false) regardless of this checkbox.">Emergent black holes</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-stellar-evolution" data-scale5-toggle="stellar_evolution">
            <label for="cosmic-rules-stellar-evolution" title="[IMPOSED threshold rule] Fuel-burn and staged death sequence (main sequence -> giant -> late burning -> white dwarf / neutron star / black hole, by fixed mass thresholds against M_Chandrasekhar and M_TOV) for STAR bodies.">Stellar evolution</label>
        </div>
        <div class="toggle-row">
            <input type="checkbox" id="cosmic-rules-hawking-evaporation" data-scale5-toggle="hawking_evaporation">
            <label for="cosmic-rules-hawking-evaporation" title="[IMPOSED threshold rule] Hawking-inspired T ~ 1/(M+1) readout (not the physical Hawking-temperature formula) drives a per-tick mass-loss rate; a black hole is fully removed in an ejecta burst once its mass drops below a fixed threshold (M &lt; 2).">Hawking evaporation</label>
        </div>
        <div class="scale-info-copy">
            <div title="getEventLog() (mock-scale5.js): a bounded ring, capacity 200, of every horizon-absorption, tidal-disruption, merger, emergent-black-hole-formation, star-formation, supernova, and evaporation event this scenario run, each tagged with its tick.">Recent events (newest first; ring buffer capped at 200 total):</div>
        </div>
        <div class="cosmic-event-log" id="cosmic-event-log-list" role="log" aria-live="off">
            <div class="cosmic-event-log-empty">No events yet.</div>
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

        // Cosmology card (Pass C): none of its three controls are
        // SCALE5_TOGGLES keys (expansion on/off and clock gain are bridge
        // setters; DM fraction is a controller-scope pre-load select), so
        // there is no bindScale5ToggleCheckboxes() call here — all three
        // are bound directly by scale5/controller.js, mirroring the Gas
        // card's adaptive-smoothing checkbox and the Dynamics card's
        // sliders.
        let cosmologyCard = gridContainer.querySelector(`[data-scale5-control-card="${COSMOLOGY_CARD_KEY}"]`);
        if (!cosmologyCard) {
            cosmologyCard = createCosmologyCard();
            gridContainer.appendChild(cosmologyCard);
        }

        // Physics rules card (Pass D): all eleven checkboxes carry
        // `data-scale5-toggle`, so this is a single bindScale5ToggleCheckboxes
        // call for the whole card — same idempotent-per-root shape as the
        // Dynamics/Gas cards above. The event-log list is populated by
        // scale5/controller.js's animateCosmic(), not here (it has no
        // registry key of its own to bind).
        let physicsRulesCard = gridContainer.querySelector(`[data-scale5-control-card="${PHYSICS_RULES_CARD_KEY}"]`);
        if (!physicsRulesCard) {
            physicsRulesCard = createPhysicsRulesCard();
            gridContainer.appendChild(physicsRulesCard);
        }
        bindScale5ToggleCheckboxes(physicsRulesCard);

        return this;
    }
}

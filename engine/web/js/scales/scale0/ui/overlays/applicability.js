/**
 * Scenario-aware availability for Scale-0 visualization overlays.
 *
 * This is deliberately capability based, not value based: a legitimate zero
 * field remains inspectable. We only hide an overlay when the frozen scenario
 * cannot produce the quantity under its declared native term profile.
 */

import {
    SCALE0_MASS_GRAVITY_SCENARIOS,
    SCALE0_SCENARIO_OVERRIDES,
    SCALE0_TOGGLES,
} from '../../../../config/toggles.js';
import { getScale0Scenario } from '../../scenario-registry.js';
import {
    FIELD_TOGGLE_BINDINGS,
} from '../dom.js';
import {
    getFieldStateSnapshot,
    getScale0State,
    setFieldToggle,
} from '../../state/store.js';
import { COL_TO_TOGGLES } from './presets.js';
import { refreshOverlayPanelShell } from './panel-shell.js';

const FIELD_KEY_BY_BUTTON = new Map(FIELD_TOGGLE_BINDINGS);

const FLUX_TAGS = new Set([
    'field', 'flux', 'wave', 'genesis', 'drive', 'imposed-field', 'topology',
    'background', 'packet', 'recoil', 'superposition', 'pair-production',
]);

const STATE_TAGS = new Set([
    'pair-production', 'polarity', 'pair', 'cohort', 'markers', 'prepared',
    'collision', 'coulomb', 'transport', 'seed', 'weak',
]);

// Canonical scenario seed domains that cannot be inferred from the public
// semantic tags or from the post-seed term profile.  Most write both J and
// ternary state, then intentionally run with few or no evolution terms.
// Keeping the domain metadata explicit preserves the seeded J visualization at
// tick 0 and after transient particles annihilate/evaporate; broadening tags
// such as `seed`, `prepared`, or `vacuum` would incorrectly expose flux for
// many state-only scenarios.  The Wilson-loop seed is the one flux-only member:
// its native initializer writes the oriented J loop without ternary matter.
export const SCALE0_SCENARIO_DOMAIN_OVERRIDES = Object.freeze({
    ...Object.fromEntries([
        'flux-annihilation',
        'flux-meson',
        'flux-string-breaking',
        'flux-baryon',
        'quantum-entangle',
        's0-seed-ee-annihilation',
        's0-seed-hydrogen',
        's0-seed-helium',
        's0-seed-h2-bond-formation',
        's0-seed-sloop',
        's0-vacuum-proton',
        's0-vacuum-neutron',
        's0-vacuum-pion-charged',
        's0-vacuum-pion-neutral',
        's0-vacuum-kaon-charged',
    ].map((id) => [id, Object.freeze({ flux: true, state: true })])),
    's0-seed-wilson-loop': Object.freeze({ flux: true, state: false }),
});

/**
 * Resolve the term profile for a scenario.
 *
 * `engineTerms`, when supplied, is a readback of what the engine is actually
 * running and WINS over the JS model for every key it reports. The JS tables are
 * only a pre-first-frame stand-in: `setupScenario` rebuilds the RenderBridge at
 * C++ defaults and the C++ body then sets its own profile, so `SCALE0_TOGGLES` +
 * `SCALE0_SCENARIO_OVERRIDES` describe what the dashboard REQUESTED, not what is
 * live. Deriving applicability from the request is how a scenario came to offer
 * overlay channels its engine profile cannot populate.
 */
function resolvedTerms(scenarioId, engineTerms = null) {
    const terms = Object.fromEntries(SCALE0_TOGGLES.map(([key, defaultValue]) => [key, !!defaultValue]));
    for (const [key, value] of SCALE0_SCENARIO_OVERRIDES[scenarioId] || []) {
        terms[key] = !!value;
    }
    if (engineTerms) {
        for (const [key, value] of Object.entries(engineTerms)) terms[key] = !!value;
    }
    return terms;
}

function hasAnyTag(tags, accepted) {
    return tags.some((tag) => accepted.has(tag));
}

/**
 * Pure classification used by both the UI and regression tests.
 */
export function getScale0OverlayApplicability(scenarioId, engineTerms = null) {
    const scenario = getScale0Scenario(scenarioId);
    const terms = resolvedTerms(scenarioId, engineTerms);
    const tags = scenario?.tags || [];
    const domainOverride = SCALE0_SCENARIO_DOMAIN_OVERRIDES[scenarioId] || {};

    if (!scenario || scenarioId === 'empty') {
        return {
            scenarioId,
            terms,
            domains: { flux: false, state: false, dual: false, gravity: false, emForce: false, strong: false },
            applicable: new Set(),
        };
    }

    const flux = domainOverride.flux ?? (hasAnyTag(tags, FLUX_TAGS)
        || terms.wave_propagation
        || terms.coupling
        || terms.gauss_projection
        || terms.dual_substrate
        || terms.de_broglie_clock);
    const state = domainOverride.state ?? (hasAnyTag(tags, STATE_TAGS)
        || terms.genesis
        || terms.color_forces
        || terms.strong_force
        || terms.confinement
        || terms.weak_transmutation);
    const dual = flux && terms.dual_substrate;
    const gravity = SCALE0_MASS_GRAVITY_SCENARIOS.has(scenarioId) || !!terms.gravity;
    const emForce = state && !!(terms.forces || terms.poisson_coulomb || terms.lorentz_force);
    const strong = state && !!(terms.color_forces || terms.strong_force || terms.confinement);
    const selectiveDamping = state && !!terms.selective_damping;
    const genesis = flux && !!terms.genesis;

    const applicable = new Set();
    const allow = (condition, ...ids) => {
        if (condition) ids.forEach((id) => applicable.add(id));
    };

    allow(flux,
        'toggle-flux-volume', 'toggle-flux-slice', 'toggle-flux-lines', 'toggle-div-field',
        'toggle-e-field', 'toggle-b-field', 'toggle-poynting', 'toggle-force-weak',
        'toggle-psi-squared', 'toggle-lagrangian-density', 'toggle-entropy-density',
        'toggle-em-energy', 'toggle-charge-density', 'toggle-vorticity',
        'toggle-e-pressure', 'toggle-b-pressure', 'toggle-dark-halo');
    allow(state, 'toggle-state-field');
    allow(emForce, 'toggle-force-em');
    allow(gravity, 'toggle-force-gravity', 'toggle-grav-potential', 'toggle-latency', 'toggle-horizon');
    allow(strong, 'toggle-force-strong', 'toggle-color-charge', 'toggle-confinement');
    allow(dual, 'toggle-dual-substrate', 'toggle-chirality', 'toggle-phase');
    allow(genesis, 'toggle-genesis-iso', 'toggle-color-charge');
    allow(selectiveDamping, 'toggle-damping-zones');
    allow(flux || state, 'toggle-gauss-residual');

    return {
        scenarioId,
        terms,
        domains: { flux, state, dual, gravity, emForce, strong },
        applicable,
    };
}

/**
 * Apply a scenario's capability mask without discarding user preferences.
 * Hidden active buttons retain their `.active` class so switching back restores
 * the selection, while their runtime/store flags and renderer visibility are
 * forced off for the incompatible scenario.
 */
export function applyScale0OverlayApplicability(scenarioId, viewportAdapter, engineTerms = null) {
    const profile = getScale0OverlayApplicability(scenarioId, engineTerms);
    const panel = document.getElementById('viewport-overlay');
    const body = panel?.querySelector('.s0-overlay-body');
    if (!panel || !body) return profile;

    for (const toggles of Object.values(COL_TO_TOGGLES)) {
        for (const buttonId of toggles) {
            const btn = document.getElementById(buttonId);
            if (!btn) continue;
            const isApplicable = profile.applicable.has(buttonId);
            btn.classList.toggle('is-inapplicable', !isApplicable);
            btn.setAttribute('aria-hidden', isApplicable ? 'false' : 'true');
            if (isApplicable) btn.removeAttribute('tabindex');
            else btn.setAttribute('tabindex', '-1');

            if (!isApplicable) {
                const fieldKey = FIELD_KEY_BY_BUTTON.get(buttonId);
                if (fieldKey) {
                    setFieldToggle(fieldKey, false);
                    viewportAdapter?.setOverlayVisible(fieldKey, false);
                }
            }
        }
    }

    const fluxVolumeApplicable = profile.applicable.has('toggle-flux-volume');
    const fluxSliceApplicable = profile.applicable.has('toggle-flux-slice');
    body.querySelector('[aria-label="Flux volume style"]')
        ?.classList.toggle('is-inapplicable', !fluxVolumeApplicable);
    body.querySelector('[aria-label="Flux slice planes"]')
        ?.classList.toggle('is-inapplicable', !fluxSliceApplicable);
    body.querySelector('.force-style-row')?.classList.toggle(
        'is-inapplicable',
        !COL_TO_TOGGLES.forces.some((id) => profile.applicable.has(id)),
    );

    if (!fluxVolumeApplicable) viewportAdapter?.setFluxVolumeVisible(false);
    if (!fluxSliceApplicable) viewportAdapter?.setFluxSliceVisible(false);

    for (const [colName, toggles] of Object.entries(COL_TO_TOGGLES)) {
        const col = body.querySelector(`.s0-overlay-col[data-col="${colName}"]`);
        const colApplicable = toggles.some((id) => profile.applicable.has(id));
        col?.classList.toggle('is-inapplicable', !colApplicable);
        col?.setAttribute('aria-hidden', colApplicable ? 'false' : 'true');
    }

    body.classList.toggle('is-applicability-empty', profile.applicable.size === 0);
    panel.dataset.scenarioId = scenarioId;
    panel.dataset.overlayDomains = Object.entries(profile.domains)
        .filter(([, enabled]) => enabled)
        .map(([name]) => name)
        .join(' ');

    const state = getScale0State();
    viewportAdapter?.syncForceStyle(state.forceStyle, getFieldStateSnapshot());
    state.fieldNeedsUpdate = true;
    refreshOverlayPanelShell();
    return profile;
}

export function isScale0OverlayApplicable(scenarioId, buttonId) {
    return getScale0OverlayApplicability(scenarioId).applicable.has(buttonId);
}

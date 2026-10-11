/**
 * Scenario-aware availability for Scale-0 visualization layers.
 *
 * Every layer is always listed. A layer the frozen scenario cannot produce
 * under its declared native term profile is dimmed and cannot be switched on,
 * and its tooltip says what is missing. This is capability based, not value
 * based: a quantity that is legitimately zero stays available.
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
import { LAYERS, layerTooltip } from './layer-catalog.js';
import { COL_TO_TOGGLES } from './presets.js';
import { refreshOverlayPanelShell } from './panel-shell.js';
import {
    isScale0StandardModelScenario,
    updateScale0StandardModelContext,
} from './standard-model.js?v=2';

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
    // vacuum.cpp explicitly retains one inert marker in these wave templates.
    // A source-free evolution profile does not erase that initialized record.
    ...Object.fromEntries([
        's0-vacuum-electron', 's0-vacuum-positron',
        's0-vacuum-muon', 's0-vacuum-antimuon',
        's0-vacuum-tau', 's0-vacuum-antitau',
        's0-vacuum-w-boson', 's0-vacuum-w-minus-boson',
    ].map((id) => [id, Object.freeze({ flux: true, state: true })])),
    // s0_seed.cpp assigns these axis labels directly through IPF, without
    // color forces or genesis. Observing a retained label is independent of
    // enabling an interaction and does not identify an SU(3) charge.
    ...Object.fromEntries([
        's0-seed-up-quark', 's0-seed-down-quark', 's0-seed-strange-quark',
        's0-seed-charm-quark', 's0-seed-bottom-quark', 's0-seed-top-quark',
        's0-seed-anti-up-quark', 's0-seed-anti-down-quark',
        's0-seed-anti-strange-quark', 's0-seed-anti-charm-quark',
        's0-seed-anti-bottom-quark', 's0-seed-anti-top-quark',
    ].map((id) => [id, Object.freeze({ flux: true, state: true, color: true })])),
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

    // Why each unavailable layer is unavailable, for its tooltip.
    const reasons = new Map();
    const blockRest = (applicable, reason) => {
        for (const layer of LAYERS) {
            if (!applicable.has(layer.id)) reasons.set(layer.id, reason);
        }
    };

    if (scenario?.backend === 'finite-records') {
        const applicable = new Set(['toggle-flux-volume', 'toggle-state-field']);
        blockRest(applicable, 'finite-record scenarios carry only the record and its state');
        return {scenarioId, terms: {},
            domains: {flux: false, state: true, finiteRecords: true, dual: false,
                gravity: false, emForce: false, strong: false, standardModel: false, properTimeClock: false},
            applicable, reasons};
    }

    if (!scenario || scenarioId === 'empty') {
        const applicable = new Set();
        blockRest(applicable, 'the empty scenario has nothing to draw');
        return {
            scenarioId,
            terms,
            domains: {
                flux: false,
                state: false,
                dual: false,
                gravity: false,
                emForce: false,
                strong: false,
                standardModel: false,
            },
            applicable,
            reasons,
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
    const standardModel = isScale0StandardModelScenario(scenarioId);
    // Proper time τ / lapse dτ/dt / de Broglie phase φ (2026-09-03) — WASM-only
    // Voxel::tau/phase fields, accumulated only at manifested (state≠0) voxels
    // and only while latency_field or de_broglie_clock is requested
    // (accumulate_proper_time's engine-side gate, transmutation_phases.cpp).
    const properTimeClock = state && !!(terms.latency_field || terms.de_broglie_clock);

    const applicable = new Set();
    // A layer can be allowed by more than one rule; the first rule that
    // refuses it supplies the reason, and any rule that allows it clears it.
    const allow = (condition, reason, ...ids) => {
        for (const id of ids) {
            if (condition) {
                applicable.add(id);
                reasons.delete(id);
            } else if (!applicable.has(id) && !reasons.has(id)) {
                reasons.set(id, reason);
            }
        }
    };

    allow(flux, 'this scenario has no flux field',
        'toggle-flux-volume', 'toggle-flux-slice', 'toggle-native-transport', 'toggle-flux-lines', 'toggle-div-field',
        'toggle-e-field', 'toggle-b-field', 'toggle-poynting', 'toggle-force-weak',
        'toggle-psi-squared', 'toggle-lagrangian-density', 'toggle-entropy-density',
        'toggle-em-energy', 'toggle-charge-density', 'toggle-vorticity',
        'toggle-e-pressure', 'toggle-b-pressure', 'toggle-dark-halo');
    allow(state, 'this scenario has no manifested matter', 'toggle-state-field');
    allow(emForce, 'it needs manifested matter and an electric force term', 'toggle-force-em');
    allow(gravity, 'gravity is off in this scenario',
        'toggle-force-gravity', 'toggle-grav-potential', 'toggle-latency', 'toggle-horizon');
    allow(strong, 'it needs manifested matter and a colour or strong force term',
        'toggle-force-strong', 'toggle-color-charge', 'toggle-confinement');
    allow(dual, 'Dual Substrate is off in this scenario', 'toggle-dual-substrate', 'toggle-chirality', 'toggle-phase');
    allow(genesis, 'genesis is off in this scenario', 'toggle-genesis-iso', 'toggle-color-charge');
    allow(state && domainOverride.color, 'it needs axis labels from genesis or a quark seed', 'toggle-color-charge');
    allow(selectiveDamping, 'selective damping is off in this scenario', 'toggle-damping-zones');
    allow(flux || state, 'this scenario has neither flux nor matter', 'toggle-gauss-residual');
    allow(properTimeClock, 'it needs manifested matter and the latency field or de Broglie clock',
        'toggle-proper-time', 'toggle-lapse', 'toggle-db-phase');
    // Static catalog context only. This deliberately does not depend on a live
    // field value and does not make the overlay scheduler sample anything.
    allow(standardModel, 'this scenario is not an elementary particle', 'toggle-sm-reference');

    return {
        scenarioId,
        terms,
        domains: { flux, state, dual, gravity, emForce, strong, standardModel, properTimeClock },
        applicable,
        reasons,
    };
}

// Built on first use: the classifier above is also run on its own, without the panel.
let tooltipById = null;
function baseTooltip(buttonId) {
    tooltipById ||= new Map(LAYERS.map((layer) => [layer.id, layerTooltip(layer)]));
    return tooltipById.get(buttonId);
}

/**
 * Apply a scenario's capability mask without discarding user preferences.
 * A dimmed button that was on keeps its `.active` class so switching back
 * restores the selection, while it reports itself as off and its runtime
 * flag and renderer visibility are forced off for the incompatible scenario.
 */
export function applyScale0OverlayApplicability(scenarioId, viewportAdapter, engineTerms = null) {
    const profile = getScale0OverlayApplicability(scenarioId, engineTerms);
    const panel = document.getElementById('viewport-overlay');
    const body = panel?.querySelector('.s0-overlay-body');
    if (!panel || !body) return profile;
    const state = getScale0State();

    for (const toggles of Object.values(COL_TO_TOGGLES)) {
        for (const buttonId of toggles) {
            const btn = document.getElementById(buttonId);
            if (!btn) continue;
            const isApplicable = profile.applicable.has(buttonId);
            btn.classList.toggle('is-inapplicable', !isApplicable);
            // Still listed and focusable, so its tooltip can be read.
            if (isApplicable) btn.removeAttribute('aria-disabled');
            else btn.setAttribute('aria-disabled', 'true');
            btn.setAttribute('aria-pressed', isApplicable && btn.classList.contains('active') ? 'true' : 'false');
            const tooltip = baseTooltip(buttonId);
            if (tooltip) {
                const reason = profile.reasons?.get(buttonId);
                const text = isApplicable || !reason ? tooltip : `${tooltip} Not available here: ${reason}.`;
                if (btn.dataset.uiTooltip !== text) {
                    btn.removeAttribute('title');
                    btn.dataset.uiTooltip = text;
                    btn.dataset.uiTooltipSource = 'title';
                }
            }

            if (!isApplicable) {
                const fieldKey = FIELD_KEY_BY_BUTTON.get(buttonId);
                // State truth tells us whether there is anything to hide. The
                // old unconditional false-write eagerly allocated every lazy
                // renderer merely to make it invisible on each scenario load.
                if (fieldKey && state.fieldFlags[fieldKey]) {
                    setFieldToggle(fieldKey, false);
                    viewportAdapter?.setOverlayVisible(fieldKey, false);
                }
            }
        }
    }

    // A dimmed layer's own controls (sub-switches, slice height) are kept away
    // by the stylesheet, keyed on the layer's own class.
    if (!profile.applicable.has('toggle-flux-volume') && viewportAdapter?.isFluxVolumeVisible?.()) {
        viewportAdapter.setFluxVolumeVisible(false);
    }
    if (!profile.applicable.has('toggle-flux-slice') && viewportAdapter?.isFluxSliceVisible?.()) {
        viewportAdapter.setFluxSliceVisible(false);
    }
    // The particle card has nothing to show outside an elementary-particle scenario.
    document.getElementById('s0-sm-context-card')
        ?.classList.toggle('is-inapplicable', !profile.domains.standardModel);

    panel.dataset.scenarioId = scenarioId;
    panel.dataset.overlayDomains = Object.entries(profile.domains)
        .filter(([, enabled]) => enabled)
        .map(([name]) => name)
        .join(' ');
    updateScale0StandardModelContext(scenarioId, getScale0Scenario(scenarioId));

    viewportAdapter?.syncForceStyle(state.forceStyle, getFieldStateSnapshot());
    state.fieldNeedsUpdate = true;
    refreshOverlayPanelShell();
    return profile;
}

export function isScale0OverlayApplicable(scenarioId, buttonId) {
    return getScale0OverlayApplicability(scenarioId).applicable.has(buttonId);
}

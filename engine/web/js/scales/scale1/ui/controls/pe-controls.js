/**
 * Scale 1 — Particle Engine Controls Card (native-engine edition).
 *
 * The DOM retains stable IDs for app wiring, while labels, status, enabled
 * state, and checked baseline are hydrated from the native Scale-1 registry.
 * The browser therefore does not maintain an independent claim/applicability
 * table.
 *
 */

import { scale1State } from '../../state/store.js?v=7';
import { scale1BehaviorPresentation } from '../../scenario-registry.js?v=15';
import { escapeHtml } from '../../../../lib/origin-policy.js';
import { DEFAULT_TRAIL_SETTINGS } from '../../trail-settings.js?v=2';

// [id, label, title] — ids are the historical DOM ids app.js wires
// (wireControls' peToggleMap); labels/titles carry the honest tags.
const FORCE_TOGGLES = [
    ['pe-coulomb', 'Coulomb',
     'Coulomb: F = −α·q_i·q_j/(4πr²). 1/r² FORM is [THEOREM]-grade lattice geometry for r ≳ 8 (Phase G); the α coupling is [PARAMETRIC].', true],
    ['pe-gravity', 'Gravity',
     'Gravity: F = +G_PE·m_i·m_j/r². G_PE = 1/(4π·m_P²) (FTD-0131, [SMC]-floored magnitude). Float-invisible next to Coulomb; read Gravity PE in diagnostics.', false],
    ['pe-exchange', 'Exchange',
     'Pauli-style exchange repulsion between same-spin same-charge pairs: α²·exp(−r²/9)/r² [IMPOSED].', false],
    ['pe-strong', 'Strong',
     'Color force toy: running α_s ladder → string tension, fires between color-labelled particles (Zoo quarks get cycled r/g/b) [IMPOSED].', false],
    ['pe-lorentz-p', 'Lorentz',
     'Lorentz v × B from partner magnetic dipoles [IMPOSED].', false],
    ['pe-magnetic-dipole', 'Mag. Dipole',
     'Magnetic dipole–dipole interaction (spin_axis) [IMPOSED].', false],
    ['pe-spin-orbit', 'Spin-Orbit',
     'Spin-orbit coupling: L·S term [IMPOSED].', false],
    ['pe-radiation', 'Radiation',
     'Radiation-reaction (Abraham-Lorentz style) damping [IMPOSED].', false],
];

const DYNAMICS_TOGGLES = [
    ['pe-damping', 'Damping',
     'Velocity damping: v *= (1 − DAMPING·dt) per tick [IMPOSED].', false],
    ['pe-relativistic-verlet', 'Rel. Verlet',
     'Relativistic velocity-Verlet integrator (momentum form) [IMPOSED numerics].', true],
    ['pe-contact-events', 'Contact Events',
     'Selected opposite-effective-charge contact-removal event. Explicit and OFF by default.', false],
];

const TOGGLE_DOM_IDS = Object.freeze({
    coulomb: 'pe-coulomb', gravity: 'pe-gravity', damping: 'pe-damping',
    lorentz: 'pe-lorentz-p', exchange: 'pe-exchange', strong: 'pe-strong',
    magnetic_dipole: 'pe-magnetic-dipole', spin_orbit: 'pe-spin-orbit',
    radiation: 'pe-radiation',
    relativistic_verlet: 'pe-relativistic-verlet', contact_events: 'pe-contact-events',
});

export function hydratePePhysicsControls(registry, bridge) {
    const readOnly = scale1State.mode === 'native_matter';
    for (const spec of Array.from(registry?.physics || [])) {
        const id = TOGGLE_DOM_IDS[spec.toggle];
        const input = id ? document.getElementById(id) : null;
        if (!input) continue;
        const label = document.querySelector(`label[for="${id}"]`);
        input.checked = !!bridge?.peGetToggle?.(spec.toggle);
        input.disabled = !spec.available || readOnly;
        const row = input.closest('.toggle-row');
        row?.setAttribute('data-tier', spec.tier);
        row?.setAttribute('data-validation', spec.validationState);
        const status = document.getElementById(`${id}-status`);
        if (status) {
            status.textContent = spec.available ? spec.status : 'retired';
            status.dataset.tier = spec.tier;
        }
        if (label) {
            label.textContent = spec.label;
            const tooltip = [
                `${spec.summary} [${String(spec.status || 'open').toUpperCase()} · ${String(spec.validationState || 'unvalidated').replaceAll('_', ' ').toUpperCase()}]`,
                `Validation: ${spec.validationCriterion || 'unregistered'}`,
                `Evidence: ${spec.validationEvidence || 'none'}`,
                spec.unavailableReason ? `Boundary: ${spec.unavailableReason}` : '',
                readOnly ? 'Mode: Native Matter replay is read-only.' : '',
            ].filter(Boolean).join('\n');
            for (const target of [row, input, label, status]) {
                if (!(target instanceof HTMLElement)) continue;
                target.dataset.uiTooltip = tooltip;
                target.dataset.uiTooltipSource = 'scale1-physics-registry';
                target.removeAttribute('title');
            }
        }
    }
    for (const id of [
        'pe-dt-slider', 'pe-soft-slider', 'btn-pe-clear',
        'btn-pe-profile-scenario', 'btn-pe-profile-verified',
        'btn-pe-profile-applicable',
    ]) {
        const element = document.getElementById(id);
        if (element) element.disabled = readOnly;
    }
    const profileState = document.getElementById('pe-physics-profile-state');
    if (profileState) {
        profileState.textContent = readOnly ? 'Read-only observer' : 'Scenario profile';
        profileState.classList.remove('is-modified');
    }
    refreshPePhysicsPanelSummary();
}

function toggleRows(defs) {
    return defs.map(([id, label, title, checked]) => {
        const toggle = Object.entries(TOGGLE_DOM_IDS)
            .find(([, domId]) => domId === id)?.[0] || '';
        return `
    <div class="toggle-row">
      <input type="checkbox" id="${id}" data-pe-toggle="${toggle}"${checked ? ' checked' : ''}>
      <label for="${id}" title="${title.replace(/"/g, '&quot;')}">${label}</label>
      <span class="pe-physics-row-status" id="${id}-status" aria-hidden="true"></span>
    </div>`;
    }).join('');
}

export function refreshPePhysicsPanelSummary() {
    const ids = Object.values(TOGGLE_DOM_IDS);
    const inputs = ids.map(id => document.getElementById(id)).filter(Boolean);
    const active = inputs.filter(input => input.checked && !input.disabled).length;
    const available = inputs.filter(input => !input.disabled).length;
    const count = document.getElementById('pe-physics-active-count');
    if (count) count.textContent = `${active} active · ${available} available`;
}

export function markPePhysicsProfileModified() {
    setPePhysicsProfileState('Modified profile', true);
}

export function setPePhysicsProfileState(label, modified = false) {
    const profileState = document.getElementById('pe-physics-profile-state');
    if (profileState) {
        profileState.textContent = label;
        profileState.classList.toggle('is-modified', modified);
    }
    refreshPePhysicsPanelSummary();
}

function formatTrailSetting(key, value) {
    if (key === 'opacity') return Number(value).toFixed(2);
    if (key === 'pointSize') return `${Number(value).toFixed(2)} lu`;
    return `${Math.round(Number(value))} tick${Math.round(Number(value)) === 1 ? '' : 's'}`;
}

export function syncPeTrailControls(settings = scale1State.trailSettings) {
    for (const input of document.querySelectorAll('[data-pe-trail-setting]')) {
        const key = input.dataset.peTrailSetting;
        if (!key || settings?.[key] === undefined) continue;
        input.value = String(settings[key]);
        const output = document.getElementById(`${input.id}-value`);
        if (output) output.textContent = formatTrailSetting(key, settings[key]);
    }
    for (const button of document.querySelectorAll('[data-pe-trail-mode]')) {
        const active = button.dataset.peTrailMode === settings?.renderMode;
        button.classList.toggle('active', active);
        button.setAttribute('aria-pressed', active ? 'true' : 'false');
    }
    const legend = document.getElementById('pe-trail-energy-legend');
    if (legend) legend.hidden = settings?.renderMode !== 'energy';
}

function formatEnergyDensity(value) {
    if (!Number.isFinite(value) || value <= 0) return '0';
    if (value >= 0.01 && value < 1000) return Number(value).toPrecision(3);
    return Number(value).toExponential(2);
}

export function updatePeTrailEnergyLegend(stats, renderMode = scale1State.trailSettings.renderMode) {
    const legend = document.getElementById('pe-trail-energy-legend');
    if (!legend) return;
    legend.hidden = renderMode !== 'energy';
    if (legend.hidden) return;
    const minimum = document.getElementById('pe-trail-energy-min');
    const maximum = document.getElementById('pe-trail-energy-max');
    const minText = formatEnergyDensity(stats?.minEnergyDensity);
    const maxText = formatEnergyDensity(stats?.maxEnergyDensity);
    if (minimum && minimum.textContent !== minText) minimum.textContent = minText;
    if (maximum && maximum.textContent !== maxText) maximum.textContent = maxText;
}

/** Refresh the native scenario-contract summary without duplicating status. */
export function refreshScale1ScenarioContractCard() {
    const element = document.getElementById('pe-scenario-contract');
    if (!element) return;
    const summary = document.getElementById('pe-scenario-details-summary');
    const rows = Array.from(scale1State.registry?.scenarios || []);
    const row = rows.find(spec => spec.id === scale1State.currentScenarioId);
    if (!row) {
        if (summary) summary.textContent = 'Scenario details · loading';
        element.innerHTML = '<span class="pe-promo-empty">Loading native scenario contract…</span>';
        return;
    }
    if (summary) summary.textContent = `Scenario details · ${row.label}`;
    const runnable = rows.filter(spec => spec.available).length;
    const physicsCount = Number(row.physicsMask || 0).toString(2)
        .split('').filter(bit => bit === '1').length;
    const presentation = scale1BehaviorPresentation(row.behavior);
    const snapshot = scale1State.lastSnapshot;
    const object = snapshot?.objects?.[0];
    const lines = [
        `<strong class="pe-scenario-observation-label">${escapeHtml(presentation.label)}</strong>`,
        `<span class="pe-scenario-observation-cue">${escapeHtml(presentation.cue)}</span>`,
        `${row.label} · ${row.scenarioClass} · ${row.status}`,
        `family: ${row.family}`,
        `model: ${row.summary}`,
        `observe: ${row.expectedObservable}`,
        `boundary: ${row.prohibitedClaim}`,
        `validation: ${row.validationState} · ${physicsCount} applicable physics module(s)`,
        `owner: ${row.owner}`,
        `source: ${row.canonicalSource || 'unregistered'}`,
        `runtime source revision: ${snapshot?.core?.sourceRevision || 'unregistered'}`,
        `artifact revision: ${snapshot?.core?.artifactRevision || 'not applicable'}`,
        `object provenance: ${object?.provenance?.status || 'not applicable'} · ${object?.provenance?.qualification || 'not evaluated'}`,
        `${runnable}/${rows.length} registered scenarios currently runnable`,
    ];
    element.dataset.uiTooltip = `${row.validationCriterion || ''}\nEvidence: ${row.validationEvidence || 'none'}`;
    element.dataset.uiTooltipSource = 'scale1-scenario-contract';
    element.removeAttribute('title');
    element.innerHTML = lines.map((line, index) => index < 2
        ? `<div>${line}</div>` : `<div>${escapeHtml(line)}</div>`).join('');
}

export function createPeControlsCard() {
  const card = document.createElement('div');
  card.className = 'card scale1-only';
  card.innerHTML = `
    <div class="card-title" title="Particle-context controls driven by the shared native scenario and physics registries.">Particle Context</div>

    <details class="pe-scenario-details" id="pe-scenario-details">
      <summary class="ctrl-details-summary pe-scenario-details-summary" id="pe-scenario-details-summary"
        title="Expand the selected scenario's behavior, validation, provenance, and observation boundary.">Scenario details · loading</summary>
      <div class="pe-scenario-contract" id="pe-scenario-contract"></div>
    </details>

    <div class="combo-section-label" title="Adjust solver precision and numerical properties.">Parameters</div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label" title="Integration step dt (ticks) for the Velocity-Verlet update. Larger values run faster but can introduce energy drift.">Time Step</span>
      <input type="range" class="pe-slider" id="pe-dt-slider" min="0.1" max="2.0" step="0.1"
        value="1.0" title="Integration step dt (ticks) for the Velocity-Verlet update. Larger values run faster but can introduce energy drift.">
      <span class="pe-ctrl-value" id="pe-dt-value">1.0</span>
    </div>
    <div class="pe-ctrl-row">
      <span class="pe-ctrl-label" title="Plummer softening length (lu) added in quadrature to pair separations. Prevents infinite singular forces when particles are extremely close.">Softening</span>
      <input type="range" class="pe-slider" id="pe-soft-slider" min="0.01" max="1.0" step="0.01"
        value="0.10" title="Plummer softening length (lu) added in quadrature to pair separations. Prevents infinite singular forces when particles are extremely close.">
      <span class="pe-ctrl-value" id="pe-soft-value">0.10</span>
    </div>

    <div class="combo-section-label pe-trail-section-label"
      title="Presentation-only particle trajectory history, measured in native particle-engine ticks rather than display frames.">Trajectory History</div>
    <p class="pe-trail-boundary">Tick-aligned trajectory history preserves motion without changing the particle dynamics.</p>
    <div class="pe-trail-mode-group" role="group" aria-label="Trajectory rendering mode">
      <button type="button" class="ctrl-btn-secondary active" data-pe-trail-mode="breadcrumbs"
        aria-pressed="true" title="Show each tick sample as an age-faded temporal breadcrumb.">Breadcrumbs</button>
      <button type="button" class="ctrl-btn-secondary" data-pe-trail-mode="lines"
        aria-pressed="false" title="Connect adjacent tick samples into an age-faded particle-colored path.">Lines</button>
      <button type="button" class="ctrl-btn-secondary" data-pe-trail-mode="energy"
        aria-pressed="false" title="Connect samples and color the line by effective kinetic-energy density KE / ((4/3)πr_eff³), in MeV/lu³.">Energy heatmap</button>
    </div>
    <div class="pe-trail-energy-legend" id="pe-trail-energy-legend" hidden
      title="Log-normalized effective kinetic-energy density along the currently visible trajectory history. Native snapshot kinetic energy is preferred.">
      <div class="pe-trail-energy-legend-title">Effective kinetic E density · MeV/lu<sup>3</sup></div>
      <div class="pe-trail-energy-ramp" aria-hidden="true"></div>
      <div class="pe-trail-energy-range"><span id="pe-trail-energy-min">0</span><span id="pe-trail-energy-max">0</span></div>
    </div>
    <div class="pe-ctrl-row pe-trail-control-row">
      <span class="pe-ctrl-label" title="How many particle-engine ticks remain visible in each active trajectory.">History span</span>
      <input type="range" class="pe-slider" id="pe-trail-history" min="10" max="1200" step="10"
        value="${DEFAULT_TRAIL_SETTINGS.historyTicks}" data-pe-trail-setting="historyTicks"
        aria-label="Trajectory history span in ticks">
      <span class="pe-ctrl-value pe-trail-value" id="pe-trail-history-value">${DEFAULT_TRAIL_SETTINGS.historyTicks} ticks</span>
    </div>
    <div class="pe-ctrl-row pe-trail-control-row">
      <span class="pe-ctrl-label" title="Record one breadcrumb every N particle-engine ticks. Higher values create a sparser, longer-spaced history.">Tick stride</span>
      <input type="range" class="pe-slider" id="pe-trail-stride" min="1" max="24" step="1"
        value="${DEFAULT_TRAIL_SETTINGS.sampleEveryTicks}" data-pe-trail-setting="sampleEveryTicks"
        aria-label="Trajectory sample interval in ticks">
      <span class="pe-ctrl-value pe-trail-value" id="pe-trail-stride-value">${DEFAULT_TRAIL_SETTINGS.sampleEveryTicks} tick</span>
    </div>
    <div class="pe-ctrl-row pe-trail-control-row">
      <span class="pe-ctrl-label" title="After a particle record disappears, retain and fade its final trajectory for this many ticks. Zero removes it immediately.">Despawn fade</span>
      <input type="range" class="pe-slider" id="pe-trail-despawn" min="0" max="1200" step="10"
        value="${DEFAULT_TRAIL_SETTINGS.disappearDelayTicks}" data-pe-trail-setting="disappearDelayTicks"
        aria-label="Trajectory retention after particle removal in ticks">
      <span class="pe-ctrl-value pe-trail-value" id="pe-trail-despawn-value">${DEFAULT_TRAIL_SETTINGS.disappearDelayTicks} ticks</span>
    </div>
    <div class="pe-ctrl-row pe-trail-control-row">
      <span class="pe-ctrl-label" title="Overall brightness of trajectory breadcrumbs. Age fading and post-despawn fading still apply.">Opacity</span>
      <input type="range" class="pe-slider" id="pe-trail-opacity" min="0.05" max="1" step="0.01"
        value="${DEFAULT_TRAIL_SETTINGS.opacity}" data-pe-trail-setting="opacity"
        aria-label="Trajectory opacity">
      <span class="pe-ctrl-value pe-trail-value" id="pe-trail-opacity-value">${DEFAULT_TRAIL_SETTINGS.opacity.toFixed(2)}</span>
    </div>
    <div class="pe-ctrl-row pe-trail-control-row">
      <span class="pe-ctrl-label" title="World-space size of each history breadcrumb. This replaces unreliable WebGL line-width controls.">Point size</span>
      <input type="range" class="pe-slider" id="pe-trail-size" min="0.08" max="1.2" step="0.02"
        value="${DEFAULT_TRAIL_SETTINGS.pointSize}" data-pe-trail-setting="pointSize"
        aria-label="Trajectory breadcrumb point size">
      <span class="pe-ctrl-value pe-trail-value" id="pe-trail-size-value">${DEFAULT_TRAIL_SETTINGS.pointSize.toFixed(2)} lu</span>
    </div>
    <div class="ctrl-action-row pe-trail-actions">
      <button class="ctrl-btn-secondary" id="btn-pe-trail-reset"
        title="Restore the presentation-only trajectory history defaults.">Reset trail display</button>
    </div>
    <div class="ctrl-action-row">
      <button class="ctrl-btn-secondary" id="btn-pe-clear" title="Clear all active particles, reset simulation time, and reload the selected scenario.">Clear &amp; Reload</button>
    </div>
    <div class="combo-section-label" title="State-complete effective ParticleEngine checkpoints and exact deterministic replay verification.">Checkpoint &amp; Replay</div>
    <p class="pe-trail-boundary">Checkpoints preserve the complete effective-lab transaction state. Replay equality validates software determinism, not physical uniqueness.</p>
    <div class="ctrl-action-row pe-checkpoint-actions">
      <button class="ctrl-btn-secondary" id="btn-pe-checkpoint-save"
        title="Capture every native value that can affect the next ParticleEngine tick.">Capture</button>
      <button class="ctrl-btn-secondary" id="btn-pe-checkpoint-restore"
        title="Restore the most recently captured or imported native checkpoint.">Restore</button>
      <button class="ctrl-btn-secondary" id="btn-pe-checkpoint-export"
        title="Download the current state as a versioned Scale 1 JSON checkpoint.">Export</button>
      <button class="ctrl-btn-secondary" id="btn-pe-checkpoint-import"
        title="Import and restore a versioned Scale 1 JSON checkpoint.">Import</button>
      <input type="file" id="pe-checkpoint-file" accept="application/json,.json" hidden>
    </div>
    <div class="ctrl-action-row pe-checkpoint-actions">
      <button class="ctrl-btn-secondary" id="btn-pe-replay-mark"
        title="Capture the deterministic start of a replay segment.">Mark replay start</button>
      <button class="ctrl-btn-secondary" id="btn-pe-replay-verify"
        title="Restore the marked start, rerun the elapsed ticks, and compare the complete final record.">Verify replay</button>
    </div>
    <div class="pe-trail-boundary" id="pe-checkpoint-status" role="status" aria-live="polite">No checkpoint captured.</div>
    <div id="pe-field-battery-controls" hidden>
      <div class="combo-section-label" title="FTD-0884 isolated finite ready-port field witness.">Finite-port field battery</div>
      <p class="pe-trail-boundary">Isolated EFT reference only: no ParticleEngine force, moving-source, photon, or Born coupling is supplied.</p>
      <div class="ctrl-action-row">
        <button class="ctrl-btn-secondary" id="btn-pe-field-battery-step"
          title="Consume one fresh prepared port and apply one reversible checkerboard Gauss layer.">Advance field layer</button>
        <button class="ctrl-btn-secondary" id="btn-pe-field-battery-reverse"
          title="Reverse the most recently accepted finite-port field layer exactly.">Reverse field layer</button>
      </div>
      <div class="pe-trail-boundary" id="pe-field-battery-status" role="status">Layer 1/8</div>
    </div>
  `;
  queueMicrotask(() => {
    refreshScale1ScenarioContractCard();
  });
  return card;
}

export function createPePhysicsCard() {
  const card = document.createElement('div');
  card.className = 'card scale1-only pe-physics-card';
  card.innerHTML = `
    <div class="pe-physics-card-header">
      <div>
        <div class="card-title" title="Native registry-backed Scale-1 physics switches.">Physics Toggles</div>
        <div class="pe-physics-active-count" id="pe-physics-active-count">0 active</div>
      </div>
      <span class="pe-physics-profile-state" id="pe-physics-profile-state">Scenario profile</span>
    </div>
    <p class="pe-physics-boundary">Each switch controls an implemented effective kernel. Enabling a term validates software behavior only; it does not promote that term to native FTD recovery.</p>
    <div class="pe-physics-profile-actions" role="group" aria-label="Scale 1 physics profiles">
      <button class="ctrl-btn-secondary" id="btn-pe-profile-scenario" data-pe-profile="scenario"
        title="Restore the selected scenario's registered physics mask without reseeding the scene.">Scenario</button>
      <button class="ctrl-btn-secondary" id="btn-pe-profile-verified" data-pe-profile="verified"
        title="Enable only the registry's verified/applicable baseline: Coulomb and relativistic momentum Verlet.">Verified</button>
      <button class="ctrl-btn-secondary" id="btn-pe-profile-applicable" data-pe-profile="applicable"
        title="Enable every available Scale-1 kernel. This includes imposed and quarantined effective terms; retired registry entries are excluded.">All applicable</button>
    </div>
    <div class="pe-physics-columns">
      <section class="pe-physics-section" aria-labelledby="pe-physics-forces-title">
        <div class="combo-section-label" id="pe-physics-forces-title" title="Pairwise force terms consumed by the native integrator.">Forces</div>
        ${toggleRows(FORCE_TOGGLES)}
      </section>
      <section class="pe-physics-section" aria-labelledby="pe-physics-dynamics-title">
        <div class="combo-section-label" id="pe-physics-dynamics-title" title="Motion-governing rules and integrator variants.">Dynamics</div>
        ${toggleRows(DYNAMICS_TOGGLES)}
      </section>
    </div>
  `;
  return card;
}

/**
 * Scale 1 — Particle Engine Controls Card (native-engine edition).
 *
 * All 11 physics toggles are generated from one table matching the native
 * PARTICLE_TOGGLE_SPECS names (engine/include/ftd/particle_engine.h) —
 * fixing the audit finding that six checkboxes were commented out of the
 * DOM while presets and app wiring still drove them. Every toggle here is
 * consumed by the native force kernel the integrator actually runs.
 *
 * Also hosts the promotion info card (⤴ Scale up provenance) and the
 * source-voxel ghost-layer toggle.
 */

import { scale1State } from '../../state/store.js';

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
    ['pe-relativistic', 'Relativistic',
     'Crude isotropic F·(1/γ−1) rescale — NOT covariant (no covariant EOM exists; FTD-0401). Visual cue only.', false],
];

const DYNAMICS_TOGGLES = [
    ['pe-damping', 'Damping',
     'Velocity damping: v *= (1 − DAMPING·dt) per tick [IMPOSED].', false],
    ['pe-relativistic-verlet', 'Rel. Verlet',
     'Relativistic velocity-Verlet integrator (momentum form) [IMPOSED numerics].', false],
];

function toggleRows(defs) {
    return defs.map(([id, label, title, checked]) => `
    <div class="toggle-row">
      <input type="checkbox" id="${id}"${checked ? ' checked' : ''}>
      <label for="${id}" title="${title.replace(/"/g, '&quot;')}">${label}</label>
    </div>`).join('');
}

/** Refresh the promotion provenance card from the store. */
export function refreshPromotionCard() {
    const el = document.getElementById('pe-promotion-info');
    if (!el) return;
    const p = scale1State.lastPromotion;
    if (!p) {
        el.innerHTML = '<span class="pe-promo-empty">No lattice capture yet — '
            + 'use "⤴ Scale up" from Scale 0.</span>';
        return;
    }
    const admissible = p.seeds.filter(s => s.admissible).length;
    const clamped = p.seeds.filter(s => s.chargeClamped).length;
    const lines = [
        `${p.seeds.length} cluster(s) from tick ${p.sourceTick} `
            + `(${p.sourceScenario ?? 'unknown scenario'}, L=${p.latticeSize})`,
        `source: ${p.clusterSource === 'knot-tracker'
            ? 'KnotTracker telemetry' : 'voxel connected-components'}`,
        `mass = N·K_B [DERIVED-linear]/[SMC]; charge = sign·N`,
        `${admissible}/${p.seeds.length} pass the scale-separation heuristic (N ≳ 113)`,
    ];
    if (clamped > 0) {
        lines.push(`⚠ ${clamped} charge(s) clamped to ±127 (native int8 field)`);
    }
    if (p.displayScale && p.displayScale !== 1) {
        lines.push(`positions display-scaled ×${p.displayScale.toFixed(3)} `
            + `[IMPOSED display mapping]`);
    }
    el.innerHTML = lines.map(l => `<div>${l}</div>`).join('');
}

export function createPeControlsCard() {
  const card = document.createElement('div');
  card.className = 'card scale1-only';
  card.innerHTML = `
    <div class="card-title" title="Controls for the native C++/WASM particle engine: pairwise forces, dynamics settings, and integration parameters.">Particle Engine Controls</div>

    <div class="combo-section-label" title="Provenance of the last ⤴ Scale up capture.">Lattice promotion</div>
    <div class="pe-promotion-info" id="pe-promotion-info"></div>
    <div class="toggle-row">
      <input type="checkbox" id="pe-voxel-debug">
      <label for="pe-voxel-debug"
        title="Ghost the per-voxel coarse-graining snapshot behind the promoted cluster particles. Ghost mass convention is the scale-bridge's max(ρ, K_B) [IMPOSED, display only].">Show source voxels</label>
    </div>

    <div class="combo-section-label" title="Pairwise force terms consumed by the native integrator.">Forces</div>
    ${toggleRows(FORCE_TOGGLES)}

    <div class="combo-section-label" title="Motion-governing rules and integrator variants.">Dynamics</div>
    ${toggleRows(DYNAMICS_TOGGLES)}

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
    <div class="ctrl-action-row">
      <button class="ctrl-btn-secondary" id="btn-pe-clear" title="Clear all active particles, reset simulation time, and reload the selected scenario.">Clear &amp; Reload</button>
    </div>
  `;
  queueMicrotask(refreshPromotionCard);
  return card;
}

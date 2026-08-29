/**
 * Contextual Standard Model reference overlay for Scale 0.
 *
 * Scale-0's named particle scenarios are finite lattice templates, not
 * established Standard Model particles (several identities are explicitly
 * [CLOSED NEGATIVE]).  This module therefore keeps two things separate:
 *
 *   - catalog reference quantum numbers, shown in a small HUD; and
 *   - live engine fields, which remain in their substrate/diagnostic overlays.
 *
 * The HUD is available only for the exact elementary-particle scenario IDs
 * below.  Generic waves, atoms, hadrons, processes, and field probes never
 * acquire a misleading "Standard Model" category by name matching.
 */

import { getById } from '../../../../particle-catalog.js';

export const SCALE0_SM_PARTICLE_SCENARIOS = Object.freeze({
    // Charged leptons.
    's0-vacuum-electron': 'electron',
    's0-vacuum-positron': 'positron',
    's0-vacuum-muon': 'muon',
    's0-vacuum-antimuon': 'antimuon',
    's0-vacuum-tau': 'tau',
    's0-vacuum-antitau': 'antitau',

    // Neutrinos.
    's0-vacuum-electron-neutrino': 'nu_e',
    's0-vacuum-electron-antineutrino': 'antinu_e',
    's0-vacuum-muon-neutrino': 'nu_mu',
    's0-vacuum-muon-antineutrino': 'antinu_mu',
    's0-vacuum-tau-neutrino': 'nu_tau',
    's0-vacuum-tau-antineutrino': 'antinu_tau',

    // Quarks and antiquarks.
    's0-seed-up-quark': 'up',
    's0-seed-anti-up-quark': 'anti_up',
    's0-seed-down-quark': 'down',
    's0-seed-anti-down-quark': 'anti_down',
    's0-seed-strange-quark': 'strange',
    's0-seed-anti-strange-quark': 'anti_strange',
    's0-seed-charm-quark': 'charm',
    's0-seed-anti-charm-quark': 'anti_charm',
    's0-seed-bottom-quark': 'bottom',
    's0-seed-anti-bottom-quark': 'anti_bottom',
    's0-seed-top-quark': 'top',
    's0-seed-anti-top-quark': 'anti_top',

    // Gauge/scalar bosons.  The separate Higgs-field scenario is deliberately
    // excluded: it is a field template, not an elementary-particle scenario.
    's0-vacuum-photon': 'photon',
    's0-seed-gluon': 'gluon',
    's0-vacuum-w-boson': 'w_plus',
    's0-vacuum-w-minus-boson': 'w_minus',
    's0-vacuum-z-boson': 'z_boson',
    's0-vacuum-higgs': 'higgs',
});

const QUARK_IDS = new Set([
    'up', 'anti_up', 'down', 'anti_down', 'strange', 'anti_strange',
    'charm', 'anti_charm', 'bottom', 'anti_bottom', 'top', 'anti_top',
]);
const NEUTRINO_IDS = new Set(['nu_e', 'nu_mu', 'nu_tau']);
const ANTINEUTRINO_IDS = new Set(['antinu_e', 'antinu_mu', 'antinu_tau']);

function formatCharge(charge) {
    if (charge === 0) return '0';
    if (charge === 1) return '+1';
    if (charge === -1) return '-1';
    if (charge === 2 / 3) return '+2/3';
    if (charge === -2 / 3) return '-2/3';
    if (charge === 1 / 3) return '+1/3';
    if (charge === -1 / 3) return '-1/3';
    return String(charge);
}

function formatSpin(spin) {
    if (spin === 0.5) return '½';
    return Number.isFinite(spin) ? String(spin) : '—';
}

function formatColorRepresentation(value) {
    if (value === 'r/g/b') return 'triplet 3';
    if (value?.includes('̄') || value?.includes('ḡ')) return 'antitriplet 3̄';
    if (value === 'octet') return 'octet 8';
    return 'singlet 1';
}

function chiralSector(entry) {
    if (!entry) return '—';
    if (NEUTRINO_IDS.has(entry.id)) return 'L weak sector';
    if (ANTINEUTRINO_IDS.has(entry.id)) return 'R weak sector';
    if (entry.spin === 0.5) return 'L / R fields';
    if (entry.id === 'photon' || entry.id === 'gluon') return 'N/A · helicity ±1';
    if (entry.spin === 1) return 'N/A · spin-1 boson';
    return 'N/A · scalar';
}

function shortStatus(status = '') {
    const tags = [...status.matchAll(/\[([^\]]+)\]/g)].map((match) => match[1]);
    return tags[tags.length - 1] || 'REFERENCE';
}

export function getScale0StandardModelContext(scenarioId, scenario = null) {
    const catalogId = SCALE0_SM_PARTICLE_SCENARIOS[scenarioId];
    if (!catalogId) return null;
    const entry = getById(catalogId);
    if (!entry) return null;
    return {
        scenarioId,
        catalogId,
        name: entry.name,
        symbol: entry.symbol,
        spin: formatSpin(entry.spin),
        charge: formatCharge(entry.charge),
        chirality: chiralSector(entry),
        generation: entry.generation ? String(entry.generation) : 'N/A',
        color: formatColorRepresentation(entry.color_charge),
        hasColorAxis: QUARK_IDS.has(entry.id),
        status: shortStatus(scenario?.epistemicStatus),
        statusDetail: scenario?.epistemicStatus || 'Catalog reference only',
    };
}

export function isScale0StandardModelScenario(scenarioId) {
    return Object.prototype.hasOwnProperty.call(SCALE0_SM_PARTICLE_SCENARIOS, scenarioId);
}

export function getScale0StandardModelHudTemplate() {
    const hud = document.createElement('aside');
    hud.id = 's0-sm-reference-hud';
    hud.className = 'scale0-only s0-sm-reference-hud';
    hud.hidden = true;
    hud.setAttribute('aria-label', 'Standard Model reference quantum numbers');
    hud.innerHTML = `
        <div class="s0-sm-hud-head">
            <span class="s0-sm-hud-kicker">SM reference</span>
            <span class="s0-sm-hud-status" data-sm-field="status">REFERENCE</span>
        </div>
        <div class="s0-sm-hud-particle">
            <span class="s0-sm-hud-symbol" data-sm-field="symbol">—</span>
            <span class="s0-sm-hud-name" data-sm-field="name">Standard Model particle</span>
        </div>
        <dl class="s0-sm-quantum-grid">
            <div><dt>Spin</dt><dd data-sm-field="spin">—</dd></div>
            <div><dt>Charge</dt><dd data-sm-field="charge">—</dd></div>
            <div><dt>Chiral sector</dt><dd data-sm-field="chirality">—</dd></div>
            <div><dt>Generation</dt><dd data-sm-field="generation">—</dd></div>
            <div class="s0-sm-grid-wide"><dt>Color representation</dt><dd data-sm-field="color">—</dd></div>
        </dl>
        <p class="s0-sm-hud-caveat">Catalog values, not Scale 0 measurements. Scenario identity retains its audited status.</p>
    `;
    return hud;
}

export function updateScale0StandardModelContext(scenarioId, scenario = null) {
    const context = getScale0StandardModelContext(scenarioId, scenario);
    const panelCard = document.getElementById('s0-sm-context-card');
    const hud = document.getElementById('s0-sm-reference-hud');
    const roots = [panelCard, hud].filter(Boolean);
    for (const root of roots) {
        for (const [key, value] of Object.entries(context || {})) {
            const target = root.querySelector(`[data-sm-field="${key}"]`);
            if (target && target.textContent !== value) target.textContent = value;
        }
        if (context?.statusDetail) root.title = context.statusDetail;
    }
    if (hud) {
        const buttonActive = document.getElementById('toggle-sm-reference')?.classList.contains('active');
        hud.hidden = !(context && buttonActive);
    }
    return context;
}

export function setScale0StandardModelReferenceVisible(visible) {
    const hud = document.getElementById('s0-sm-reference-hud');
    const scenarioId = document.getElementById('viewport-overlay')?.dataset.scenarioId;
    if (hud) hud.hidden = !(visible && isScale0StandardModelScenario(scenarioId));
}

/**
 * Bind the DOM-only reference control after the overlay panel has mounted.
 *
 * This deliberately lives with the panel shell instead of bindScale0UI(): the
 * shell owns the lifetime of the asynchronously mounted overlay markup.  Event
 * binding here prevents a boot-order race where Scale 0 could bind before the
 * button existed, while keeping this static reference out of the live field
 * scheduler and its per-frame work.
 */
export function initScale0StandardModelReferenceControl() {
    const button = document.getElementById('toggle-sm-reference');
    if (!button || button.dataset.smReferenceBound === 'true') return;
    button.dataset.smReferenceBound = 'true';
    button.addEventListener('click', () => {
        if (button.classList.contains('is-inapplicable')) return;
        const visible = !button.classList.contains('active');
        button.classList.toggle('active', visible);
        button.setAttribute('aria-pressed', visible ? 'true' : 'false');
        setScale0StandardModelReferenceVisible(visible);
    });
}

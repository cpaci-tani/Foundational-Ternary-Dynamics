/**
 * Particle Zoo — interactive table of all SM particles with FTD data.
 *
 * Renders categorized rows from particle-catalog.js.
 * "+" inject buttons active only in Scale 1 (ParticleEngine) mode.
 */

import { getAllParticles, getCategories, getByCategory, formatMass, chargeLabel } from './particle-catalog.js';

// Generation buckets — the Moore-layer 3-generation structure ([SELECTION],
// LEDGER FTD-0028: theorem-shaped argument, physical reading is selection).
// Bosons and composite hadrons carry generation: null (not a generational
// fermion) and group under "Not applicable".
const GENERATIONS = {
    1: { label: 'Generation I', order: 0 },
    2: { label: 'Generation II', order: 1 },
    3: { label: 'Generation III', order: 2 },
    na: { label: 'Not applicable (bosons, composites)', order: 3 },
};

let _bridge = null;
let _engineMode = 'lattice';
let _searchTerm = '';
let _filterCat = 'all';
let _groupBy = 'category';

export function initZoo(bridge) {
    _bridge = bridge;

    const searchEl = document.getElementById('zoo-search');
    const filterEl = document.getElementById('zoo-filter');
    const groupByEl = document.getElementById('zoo-group-by');

    if (searchEl) searchEl.addEventListener('input', (e) => {
        _searchTerm = e.target.value.toLowerCase();
        renderZoo();
    });
    if (filterEl) filterEl.addEventListener('change', (e) => {
        _filterCat = e.target.value;
        renderZoo();
    });
    if (groupByEl) groupByEl.addEventListener('change', (e) => {
        _groupBy = e.target.value;
        if (filterEl) filterEl.style.display = _groupBy === 'generation' ? 'none' : '';
        renderZoo();
    });

    renderZoo();
}

export function setEngineMode(mode) {
    _engineMode = mode;
    renderZoo();
}

function matchesSearch(p) {
    if (!_searchTerm) return true;
    return p.name.toLowerCase().includes(_searchTerm) ||
           p.symbol.toLowerCase().includes(_searchTerm) ||
           p.id.toLowerCase().includes(_searchTerm);
}

function renderParticleCard(p) {
    const [r, g, b] = p.display_color;
    const dotColor = `rgb(${Math.round(r*255)},${Math.round(g*255)},${Math.round(b*255)})`;
    const accStr = p.ftd_accuracy !== null ? p.ftd_accuracy.toFixed(p.ftd_accuracy < 0.1 ? 3 : 1) + '%' : '--';
    const accClass = p.ftd_status === 'derived' ? 'color:var(--positive)' :
                     p.ftd_status === 'selection' ? 'color:var(--warning)' :
                     'color:var(--text-muted)';
    const canInject = _engineMode === 'particles' && p.charge !== 0 && p.mass_mev > 0;

    return `<div class="zoo-card">
        <div class="zoo-card-line1">
            <span class="zoo-dot" style="background:${dotColor}"></span>
            <span class="zoo-symbol">${p.symbol}</span>
            <span class="zoo-name">${p.name}</span>
            <span class="zoo-accuracy" style="${accClass}" title="FTD-predicted mass deviation vs measured (yellow = strongly-motivated conjecture, grey = parametric)">${accStr}</span>
            <button class="zoo-inject-btn" data-particle="${p.id}" title="Inject ${p.name}" ${canInject ? '' : 'disabled'}>+</button>
        </div>
        <div class="zoo-card-line2">
            <span class="zoo-mass" title="Measured (PDG) rest mass; electron uses the FTD anchor m_e = 0.511 MeV">${formatMass(p.mass_mev)}</span>
            <span class="zoo-meta">q ${chargeLabel(p.charge)}</span>
            <span class="zoo-meta">spin ${p.spin}</span>
            <span class="zoo-formula" title="FTD-predicted mass formula (motivating match, not a derivation): ${p.ftd_formula}">${p.ftd_formula || '--'}</span>
        </div>
    </div>`;
}

function renderZoo() {
    const container = document.getElementById('zoo-table-container');
    if (!container) return;

    // Two-line cards (was a 9-column table). Each particle is an identity line
    // (dot · symbol · name · accuracy · inject) over a data line (mass · charge ·
    // spin · FTD formula). Flexible fields ellipsis so nothing overflows the
    // panel width at any mount size.
    let html = '<div class="zoo-cards">';

    if (_groupBy === 'generation') {
        const all = getAllParticles();
        for (const [genKey, genMeta] of Object.entries(GENERATIONS).sort((a, b) => a[1].order - b[1].order)) {
            const particles = all.filter((p) => {
                const bucket = p.generation === null || p.generation === undefined ? 'na' : String(p.generation);
                return bucket === genKey && matchesSearch(p);
            });
            if (particles.length === 0) continue;
            html += `<div class="zoo-cat-header">${genMeta.label} (${particles.length})</div>`;
            for (const p of particles) html += renderParticleCard(p);
        }
    } else {
        const categories = getCategories();
        const catOrder = Object.entries(categories).sort((a, b) => a[1].order - b[1].order);

        for (const [catId, catMeta] of catOrder) {
            if (_filterCat !== 'all' && _filterCat !== catId) continue;

            const particles = getByCategory(catId).filter(matchesSearch);
            if (particles.length === 0) continue;

            // Category divider
            html += `<div class="zoo-cat-header" style="border-color:${catMeta.color};color:${catMeta.color}">${catMeta.label} (${particles.length})</div>`;
            for (const p of particles) html += renderParticleCard(p);
        }
    }

    html += '</div>';
    html += `<p class="zoo-note" style="font-size:11px;color:var(--text-muted);margin:6px 4px 0;line-height:1.4;">
        <strong>Mass</strong> is the measured (PDG) value (electron = FTD anchor m_e).
        <strong>FTD Formula</strong> + <strong>Acc.</strong> are FTD's <em>prediction</em> and its deviation —
        motivating matches, not derivations. Colour: <span style="color:var(--warning)">yellow</span> = [SELECTION]/strongly-motivated conjecture,
        grey = [PARAMETRIC]. No Standard-Model mass is currently [DERIVED].
    </p>`;
    container.innerHTML = html;

    // Bind inject buttons
    container.querySelectorAll('.zoo-inject-btn:not([disabled])').forEach(btn => {
        btn.addEventListener('click', () => injectFromZoo(btn.dataset.particle));
    });
}

function injectFromZoo(particleId) {
    if (!_bridge || _engineMode !== 'particles') return;

    const p = getAllParticles().find(x => x.id === particleId);
    if (!p) return;

    // Inject at a random offset from center (moderate distance so Coulomb
    // forces can pull the particle into orbit if there's a nucleus present)
    const spread = 10;
    const x = (Math.random() - 0.5) * spread;
    const y = (Math.random() - 0.5) * spread;
    const z = (Math.random() - 0.5) * spread;

    // Near-zero initial velocity — let Coulomb forces capture the particle
    // Escape velocity at r=5 from a proton is only ~0.02, so any larger
    // injection velocity sends particles flying away immediately.
    const vx = 0, vy = 0, vz = 0;

    // Charge as integer sign for PE (it uses int8_t)
    const charge = p.charge > 0 ? 1 : p.charge < 0 ? -1 : 0;

    _bridge.peAddParticle(p.id, charge, x, y, z, vx, vy, vz, p.mass_mev, 0.1);
}

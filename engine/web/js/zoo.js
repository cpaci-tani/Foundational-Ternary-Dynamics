/**
 * Particle Zoo — interactive table of all SM particles with FTD data.
 *
 * Renders categorized rows from particle-catalog.js.
 * "+" inject buttons active only in Scale 1 (ParticleEngine) mode.
 */

import { getAllParticles, getCategories, getByCategory, formatMass, chargeLabel } from './particle-catalog.js';

let _bridge = null;
let _engineMode = 'lattice';
let _searchTerm = '';
let _filterCat = 'all';

export function initZoo(bridge) {
    _bridge = bridge;

    const searchEl = document.getElementById('zoo-search');
    const filterEl = document.getElementById('zoo-filter');

    if (searchEl) searchEl.addEventListener('input', (e) => {
        _searchTerm = e.target.value.toLowerCase();
        renderZoo();
    });
    if (filterEl) filterEl.addEventListener('change', (e) => {
        _filterCat = e.target.value;
        renderZoo();
    });

    renderZoo();
}

export function setEngineMode(mode) {
    _engineMode = mode;
    renderZoo();
}

function renderZoo() {
    const container = document.getElementById('zoo-table-container');
    if (!container) return;

    const categories = getCategories();
    const catOrder = Object.entries(categories).sort((a, b) => a[1].order - b[1].order);

    let html = `<table class="zoo-table"><thead><tr>
        <th></th><th>Symbol</th><th>Name</th><th>Mass</th>
        <th>Charge</th><th>Spin</th><th>FTD Formula</th><th>Acc.</th><th></th>
    </tr></thead><tbody>`;

    for (const [catId, catMeta] of catOrder) {
        if (_filterCat !== 'all' && _filterCat !== catId) continue;

        const particles = getByCategory(catId).filter(p => {
            if (!_searchTerm) return true;
            return p.name.toLowerCase().includes(_searchTerm) ||
                   p.symbol.toLowerCase().includes(_searchTerm) ||
                   p.id.toLowerCase().includes(_searchTerm);
        });
        if (particles.length === 0) continue;

        // Category header
        html += `<tr class="zoo-cat-header"><td colspan="9" style="border-color:${catMeta.color};color:${catMeta.color}">${catMeta.label} (${particles.length})</td></tr>`;

        for (const p of particles) {
            const [r, g, b] = p.display_color;
            const dotColor = `rgb(${Math.round(r*255)},${Math.round(g*255)},${Math.round(b*255)})`;
            const accStr = p.ftd_accuracy !== null ? p.ftd_accuracy.toFixed(p.ftd_accuracy < 0.1 ? 3 : 1) + '%' : '--';
            const accClass = p.ftd_status === 'derived' ? 'color:var(--positive)' :
                             p.ftd_status === 'selection' ? 'color:var(--warning)' :
                             'color:var(--text-muted)';
            const canInject = _engineMode === 'particles' && p.charge !== 0 && p.mass_mev > 0;

            html += `<tr>
                <td><span class="zoo-dot" style="background:${dotColor}"></span></td>
                <td class="zoo-symbol">${p.symbol}</td>
                <td>${p.name}</td>
                <td class="zoo-mass">${formatMass(p.mass_mev)}</td>
                <td>${chargeLabel(p.charge)}</td>
                <td>${p.spin}</td>
                <td class="zoo-formula" title="${p.ftd_formula}">${truncate(p.ftd_formula, 30)}</td>
                <td class="zoo-accuracy" style="${accClass}">${accStr}</td>
                <td><button class="zoo-inject-btn" data-particle="${p.id}" title="Inject ${p.name}" ${canInject ? '' : 'disabled'}>+</button></td>
            </tr>`;
        }
    }

    html += '</tbody></table>';
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

function truncate(s, len) {
    if (!s) return '--';
    return s.length > len ? s.substring(0, len - 1) + '\u2026' : s;
}

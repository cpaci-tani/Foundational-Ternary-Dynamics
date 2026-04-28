/**
 * Spectroscopy Module — Hydrogen energy levels and spectral series.
 *
 * Every formula uses constants from the ontic chain via constants.js.
 * All energies in MeV (natural units where K_B = m_e = 0.511 MeV).
 */

import { ALPHA, K_B, M_E_PHYS, PI_FTD, N_C, B_3, N_EFF, HBAR_C_MEV_FM } from './constants.js';

// ── Energy Levels ───────────────────────────────────────────────────
//
// Note: K_B is the FTD electron-mass anchor (0.511 MeV exact); M_E_PHYS
// is the PDG-measured electron mass (0.51099895 MeV). Spectroscopy
// formulas below use M_E_PHYS so wavelengths/energies compare directly
// against CODATA Rydberg values without an O(0.2%) FTD-anchor offset.

/** Hydrogen-like energy level E_n = -m_e * Z^2 * alpha^2 / (2*n^2) */
export function hydrogenEnergyLevel(n, Z = 1) {
    return -M_E_PHYS * Z * Z * ALPHA * ALPHA / (2.0 * n * n);
}

/**
 * Fine structure correction to energy level.
 * delta_E = E_n * alpha^2 / n * (1/(j+1/2) - 3/(4n))
 * where j = l +/- 1/2 is the total angular momentum quantum number.
 */
export function fineStructureCorrection(n, j, Z = 1) {
    const E_n = hydrogenEnergyLevel(n, Z);
    return E_n * ALPHA * ALPHA / n * (1.0 / (j + 0.5) - 3.0 / (4.0 * n));
}

/** Bohr radius in femtometers: a_0 = hbar*c / (m_e * c^2 * alpha) */
export function bohrRadiusFm(Z = 1) {
    return HBAR_C_MEV_FM / (M_E_PHYS * ALPHA * Z);
}

/** Compton wavelength in fm: lambda_C = 2*pi * hbar*c / (m_e * c^2) */
export function comptonWavelengthFm() {
    return 2.0 * PI_FTD * HBAR_C_MEV_FM / M_E_PHYS;
}

// ── Spectral Series ─────────────────────────────────────────────────

const SERIES_NAMES = ['Lyman', 'Balmer', 'Paschen', 'Brackett', 'Pfund'];
const SERIES_COLORS = ['#9b59b6', '#3498db', '#e74c3c', '#e67e22', '#95a5a6'];

/**
 * Compute spectral transitions for hydrogen-like atom.
 * Returns array of { n_i, n_f, energy, wavelength_nm, series, color }.
 */
export function spectralSeries(Z = 1, n_max = 7) {
    const transitions = [];

    for (let n_f = 1; n_f <= Math.min(5, n_max); n_f++) {
        for (let n_i = n_f + 1; n_i <= n_max; n_i++) {
            const E_i = hydrogenEnergyLevel(n_i, Z);
            const E_f = hydrogenEnergyLevel(n_f, Z);
            const dE = E_f - E_i; // negative (photon emitted), take abs

            // Wavelength: lambda = 2*pi*hbar*c / |dE|, convert MeV -> nm
            // hbar*c = 197.327 MeV*fm, 1 fm = 1e-6 nm
            const lambda_fm = 2.0 * PI_FTD * HBAR_C_MEV_FM / Math.abs(dE);
            const lambda_nm = lambda_fm * 1e-6; // fm -> nm

            transitions.push({
                n_i, n_f,
                energy: Math.abs(dE),     // MeV
                energy_eV: Math.abs(dE) * 1e6, // eV
                wavelength_nm: lambda_nm,
                series: SERIES_NAMES[n_f - 1] || `n=${n_f}`,
                color: SERIES_COLORS[n_f - 1] || '#888',
            });
        }
    }

    return transitions;
}

/** Ionization energy for hydrogen-like atom: |E_1| */
export function ionizationEnergy(Z = 1) {
    return Math.abs(hydrogenEnergyLevel(1, Z));
}

/** Rydberg constant from ontic chain: R_inf = m_e * alpha^2 / (2 * hbar * c) */
export function rydbergEnergy() {
    return M_E_PHYS * ALPHA * ALPHA / 2.0; // in MeV
}

// ── DOM rendering ───────────────────────────────────────────────────

/**
 * Render SVG energy level diagram into a container.
 * Shows levels n=1..6 with transition lines colored by series.
 */
export function renderEnergyLevels(Z, container) {
    const nMax = 6;
    const trans = spectralSeries(Z, nMax);
    const E1 = hydrogenEnergyLevel(1, Z);
    const E_ion = ionizationEnergy(Z);

    const W = 360, H = 200;
    const leftMargin = 50, rightMargin = 20, topPad = 15, botPad = 20;
    const levelWidth = W - leftMargin - rightMargin;

    let svg = `<svg viewBox="0 0 ${W} ${H}" style="width:100%;height:100%;font-family:var(--font-mono);font-size:9px">`;

    // Draw energy levels
    for (let n = 1; n <= nMax; n++) {
        const E = hydrogenEnergyLevel(n, Z);
        // Map energy: E1 (most negative) at bottom, 0 at top
        const y = topPad + (1 - E / E1) * (H - topPad - botPad);
        const x1 = leftMargin;
        const x2 = leftMargin + levelWidth * 0.6;

        svg += `<line x1="${x1}" y1="${y}" x2="${x2}" y2="${y}" stroke="var(--text-secondary)" stroke-width="1.5"/>`;
        svg += `<text x="${x1 - 4}" y="${y + 3}" fill="var(--text-muted)" text-anchor="end">n=${n}</text>`;

        // Energy label on right
        const eV = E * 1e6;
        svg += `<text x="${x2 + 6}" y="${y + 3}" fill="var(--accent)" font-size="8">${eV.toFixed(1)} eV</text>`;
    }

    // Draw transition arrows (only first 3 series, limited transitions)
    const drawn = new Set();
    for (const t of trans) {
        if (t.n_f > 3) continue; // only Lyman, Balmer, Paschen
        const key = `${t.n_i}-${t.n_f}`;
        if (drawn.has(key)) continue;
        drawn.add(key);

        const y1 = topPad + (1 - hydrogenEnergyLevel(t.n_i, Z) / E1) * (H - topPad - botPad);
        const y2 = topPad + (1 - hydrogenEnergyLevel(t.n_f, Z) / E1) * (H - topPad - botPad);
        const x = leftMargin + levelWidth * 0.3 + (t.n_f - 1) * 30;

        svg += `<line x1="${x}" y1="${y1}" x2="${x}" y2="${y2}" stroke="${t.color}" stroke-width="1" opacity="0.6"/>`;
    }

    // Ionization line at E=0
    const y0 = topPad;
    svg += `<line x1="${leftMargin}" y1="${y0}" x2="${leftMargin + levelWidth * 0.6}" y2="${y0}" stroke="var(--text-muted)" stroke-width="0.5" stroke-dasharray="3,3"/>`;
    svg += `<text x="${leftMargin + levelWidth * 0.6 + 6}" y="${y0 + 3}" fill="var(--text-muted)" font-size="8">ionized</text>`;

    svg += '</svg>';
    container.innerHTML = `
        <div class="card-title">Energy Levels (Z=${Z})</div>
        ${svg}
        <div style="display:flex;gap:10px;margin-top:4px;font-size:10px">
            <span style="color:${SERIES_COLORS[0]}">Lyman</span>
            <span style="color:${SERIES_COLORS[1]}">Balmer</span>
            <span style="color:${SERIES_COLORS[2]}">Paschen</span>
            <span style="color:var(--text-muted)">a<sub>0</sub>=${bohrRadiusFm(Z).toFixed(0)} fm</span>
        </div>`;
}

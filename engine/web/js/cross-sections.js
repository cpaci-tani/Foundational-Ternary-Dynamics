/**
 * Cross-Sections Module — scattering cross-sections from the ontic chain.
 *
 * Every formula uses constants from constants.js (which traces to ontic.h).
 * All energies in MeV, lengths in fm, cross-sections in fm².
 */

import { ALPHA, K_B, M_E_PHYS, PI_FTD, HBAR_C_MEV_FM } from './constants.js';

// ── Classical Electron Radius ────────────────────────────────────────

/** Classical electron radius: r_e = alpha * hbar*c / (m_e * c^2) in fm.
 *  Uses M_E_PHYS (PDG electron mass) so cross-sections compare directly
 *  against measured values; K_B is the FTD mass anchor and would shift
 *  r_e by O(0.2%). */
export function classicalElectronRadiusFm() {
    return ALPHA * HBAR_C_MEV_FM / M_E_PHYS;
}

// ── Thomson Cross-Section ────────────────────────────────────────────

/**
 * Thomson scattering cross-section (low-energy photon off electron).
 * sigma_T = (8*pi/3) * r_e^2   [fm^2]
 *
 * Experimental: 6.6524e-29 m^2 = 6.6524e5 fm^2
 */
export function thomsonCrossSection() {
    const r_e = classicalElectronRadiusFm();
    return 8.0 * PI_FTD / 3.0 * r_e * r_e;
}

// ── Rutherford Scattering ────────────────────────────────────────────

/**
 * Rutherford differential cross-section: dsigma/dOmega.
 *
 * dsigma/dOmega = (Z1*Z2*alpha*hbar*c / (4*E_kin))^2 / sin^4(theta/2)
 *
 * @param {number} theta - scattering angle in radians (0, pi)
 * @param {number} E_kin - kinetic energy in MeV (CM frame)
 * @param {number} Z1 - charge of projectile
 * @param {number} Z2 - charge of target
 * @returns {number} differential cross-section in fm^2/sr
 */
export function rutherfordDiffCS(theta, E_kin, Z1 = 1, Z2 = 1) {
    const sinHalf = Math.sin(theta / 2.0);
    if (Math.abs(sinHalf) < 1e-12) return Infinity;
    const a = Z1 * Z2 * ALPHA * HBAR_C_MEV_FM / (4.0 * E_kin);
    return (a * a) / (sinHalf * sinHalf * sinHalf * sinHalf);
}

/**
 * Integrated Rutherford cross-section between theta_min and theta_max.
 * Analytical integral: sigma = pi * a^2 * [cot^2(theta_min/2) - cot^2(theta_max/2)]
 */
export function rutherfordIntegrated(theta_min, theta_max, E_kin, Z1 = 1, Z2 = 1) {
    const a = Z1 * Z2 * ALPHA * HBAR_C_MEV_FM / (4.0 * E_kin);
    const cotMin = 1.0 / Math.tan(theta_min / 2.0);
    const cotMax = 1.0 / Math.tan(theta_max / 2.0);
    return PI_FTD * a * a * (cotMin * cotMin - cotMax * cotMax);
}

// ── Mott Cross-Section ───────────────────────────────────────────────

/**
 * Mott scattering: relativistic correction to Rutherford.
 *
 * dsigma/dOmega_Mott = dsigma/dOmega_Ruth * (1 - beta^2 * sin^2(theta/2))
 *
 * @param {number} theta - scattering angle in radians
 * @param {number} E_kin - kinetic energy in MeV
 * @param {number} Z - target charge
 * @returns {number} differential cross-section in fm^2/sr
 */
export function mottDiffCS(theta, E_kin, Z = 1) {
    const ruth = rutherfordDiffCS(theta, E_kin, 1, Z);
    // beta = v/c, from relativistic kinematics
    // E_total = E_kin + m_e, gamma = E_total / m_e, beta^2 = 1 - 1/gamma^2
    const E_total = E_kin + M_E_PHYS;
    const gamma = E_total / M_E_PHYS;
    const beta2 = 1.0 - 1.0 / (gamma * gamma);
    const sinHalf = Math.sin(theta / 2.0);
    return ruth * (1.0 - beta2 * sinHalf * sinHalf);
}

// ── Pair Production ──────────────────────────────────────────────────

/**
 * Pair production threshold energy.
 * Minimum photon energy to create e+e- pair: E_thresh = 2 * m_e
 * This is the genesis threshold — K_GENESIS = N_c * K_B in FTD.
 */
export function pairProductionThreshold() {
    return 2.0 * K_B;  // 1.022 MeV
}

/**
 * Bethe-Heitler pair production cross-section (high-energy approximation).
 * sigma_pair ~ (7/9) * alpha * r_e^2 * Z^2 * (28/3 * ln(2*E/m_e) - 218/27)
 *
 * The numerical coefficients 7/9, 28/3, 218/27 are [PARAMETRIC QED-asymptotic]
 * — exact rationals from the leading-log expansion of the Bethe-Heitler
 * integral, kept inline rather than promoted to constants.js because
 * they only appear in this single formula. See PDG Reviews of Particle
 * Physics §34 (Passage of Particles Through Matter).
 *
 * @param {number} E_photon - photon energy in MeV
 * @param {number} Z - target nuclear charge
 * @returns {number} cross-section in fm^2 (0 if below threshold)
 */
export function pairProductionCS(E_photon, Z = 1) {
    if (E_photon < 2.0 * M_E_PHYS) return 0.0;
    const r_e = classicalElectronRadiusFm();
    const logTerm = 28.0 / 3.0 * Math.log(2.0 * E_photon / M_E_PHYS) - 218.0 / 27.0;
    if (logTerm <= 0) return 0.0;
    return 7.0 / 9.0 * ALPHA * r_e * r_e * Z * Z * logTerm;
}

// ── Compton Scattering ───────────────────────────────────────────────

/**
 * Klein-Nishina differential cross-section for Compton scattering.
 *
 * @param {number} theta - scattering angle in radians
 * @param {number} E_photon - incident photon energy in MeV
 * @returns {number} differential cross-section in fm^2/sr
 */
export function kleinNishinaDiffCS(theta, E_photon) {
    const r_e = classicalElectronRadiusFm();
    const x = E_photon / M_E_PHYS;  // photon energy in electron masses
    const cosTheta = Math.cos(theta);
    const P = 1.0 / (1.0 + x * (1.0 - cosTheta)); // ratio E'/E
    return 0.5 * r_e * r_e * P * P * (P + 1.0 / P - 1.0 + cosTheta * cosTheta);
}

/**
 * Total Compton cross-section (Klein-Nishina integrated).
 * Low-energy limit → Thomson; high-energy → decreases as 1/E.
 */
export function comptonTotalCS(E_photon) {
    const x = E_photon / M_E_PHYS;
    const r_e = classicalElectronRadiusFm();
    if (x < 0.01) return thomsonCrossSection(); // Thomson limit
    // Klein-Nishina total:
    const lnTerm = Math.log(1.0 + 2.0 * x);
    const sigma = 2.0 * PI_FTD * r_e * r_e * (
        (1.0 + x) / (x * x * x) * (2.0 * x * (1.0 + x) / (1.0 + 2.0 * x) - lnTerm) +
        lnTerm / (2.0 * x) -
        (1.0 + 3.0 * x) / ((1.0 + 2.0 * x) * (1.0 + 2.0 * x))
    );
    return Math.max(sigma, 0);
}

// ── DOM Rendering ────────────────────────────────────────────────────

/**
 * Render cross-section summary card into a container.
 */
export function renderCrossSections(container) {
    const sigma_T = thomsonCrossSection();
    const r_e = classicalElectronRadiusFm();
    const E_pair = pairProductionThreshold();

    // Polar plot data for Rutherford at 1 MeV, Z=79 (gold)
    const angles = [];
    const ruthData = [];
    const mottData = [];
    for (let deg = 10; deg <= 170; deg += 5) {
        const theta = deg * PI_FTD / 180;
        angles.push(deg);
        ruthData.push(Math.log10(Math.max(rutherfordDiffCS(theta, 1.0, 1, 79), 1e-10)));
        mottData.push(Math.log10(Math.max(mottDiffCS(theta, 1.0, 79), 1e-10)));
    }

    // SVG polar plot
    const W = 360, H = 200;
    const cx = W / 2, cy = H / 2 + 5, R = 75;
    let svg = `<svg viewBox="0 0 ${W} ${H}" class="pc-svg-container sm">`;

    // Polar grid
    for (let r = 0.25; r <= 1; r += 0.25) {
        svg += `<circle cx="${cx}" cy="${cy}" r="${R * r}" fill="none" stroke="var(--bg-card)" stroke-width="0.5"/>`;
    }

    // Rutherford curve (gold target, 1 MeV)
    const logMin = Math.min(...ruthData);
    const logMax = Math.max(...ruthData);
    const norm = (v) => (v - logMin) / (logMax - logMin + 1e-10);

    let ruthPath = '';
    let mottPath = '';
    angles.forEach((deg, i) => {
        const theta = deg * PI_FTD / 180;
        const rR = norm(ruthData[i]) * R;
        const rM = norm(mottData[i]) * R;
        const xR = cx + rR * Math.sin(theta);
        const yR = cy - rR * Math.cos(theta);
        const xM = cx + rM * Math.sin(theta);
        const yM = cy - rM * Math.cos(theta);
        ruthPath += (i === 0 ? 'M' : 'L') + `${xR.toFixed(1)},${yR.toFixed(1)}`;
        mottPath += (i === 0 ? 'M' : 'L') + `${xM.toFixed(1)},${yM.toFixed(1)}`;
    });

    svg += `<path d="${ruthPath}" fill="none" stroke="#e67e22" stroke-width="1.5" opacity="0.8"/>`;
    svg += `<path d="${mottPath}" fill="none" stroke="#3498db" stroke-width="1.5" opacity="0.8"/>`;

    // Labels
    svg += `<text x="${cx}" y="14" fill="var(--text-secondary)" text-anchor="middle" font-size="9">Rutherford vs Mott (Au, 1 MeV)</text>`;
    svg += `<text x="10" y="${H - 5}" fill="#e67e22" font-size="8">Rutherford</text>`;
    svg += `<text x="80" y="${H - 5}" fill="#3498db" font-size="8">Mott</text>`;

    svg += '</svg>';

    container.innerHTML = `
        <div class="card-title">Cross-Sections (from ontic chain)</div>
        ${svg}
        <div class="pc-grid-2">
            <div>r<sub>e</sub> = <span class="pc-text-accent">${r_e.toFixed(2)} fm</span></div>
            <div>σ<sub>T</sub> = <span class="pc-text-accent">${sigma_T.toFixed(0)} fm²</span></div>
            <div>E<sub>pair</sub> = <span class="pc-text-accent">${(E_pair * 1e3).toFixed(1)} keV</span></div>
            <div>σ<sub>T</sub>/m² = <span class="pc-text-accent">${(sigma_T * 1e-30).toExponential(2)}</span></div>
        </div>`;
}

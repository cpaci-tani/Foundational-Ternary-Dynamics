/**
 * Scale-0 telemetry grid channel registry.
 * Shared by TelemetryGridPanelComponent; mirrors diagnostics-panel trend buffers.
 * Canonical table rows live in ui/panels/diagnostics-panel/descriptors/scale0.js.
 */

export const SCALE0_GRID_CHANNELS = [
    { key: 'flux',       title: 'Total Flux',        buffer: 'flux',            color: 'var(--chart-flux, #fb8c00)',   unit: 'J' },
    { key: 'energy',     title: 'Total Energy',      buffer: 'energy',          color: 'var(--chart-energy, #42a5f5)', unit: 'E*' },
    { key: 'manifested', title: 'Particle Count',    buffer: 'manifested',      color: 'var(--chart-eb, #a78bfa)',     unit: 'ct' },
    { key: 'charges',    title: 'Net Charge',        buffer: 'charges',         color: 'var(--chart-charge, #4ade80)', unit: 'e' },
    { key: 'positive',   title: 'Positive Charges',  buffer: 'positive',        color: 'var(--chart-positive, #4ade80)', unit: 'e' },
    { key: 'negative',   title: 'Negative Charges',  buffer: 'negative',        color: 'var(--chart-negative, #f87171)', unit: 'e' },
    { key: 'entropy',    title: 'Entropy',           buffer: 'entropy',         color: 'var(--chart-entropy, #60a5fa)', unit: 'nat' },
    { key: 'gauss',      title: 'Gauss Violation',   buffer: 'gauss',           color: 'var(--chart-gauss, #fbbf24)',   unit: 'E*²' },
    { key: 'drift',      title: 'Energy Drift',      buffer: 'aud.energyDrift', color: 'var(--chart-eb, #a78bfa)', unit: '%' },
    { key: 'ebDiff',     title: 'E-B Energy Diff',   buffer: 'ebDiff',          color: 'var(--chart-eb, #a78bfa)', unit: 'E*' },
    { key: 'fieldE',     title: 'Field Energy',      buffer: 'aud.fieldEnergy', color: 'var(--chart-energy, #42a5f5)', unit: 'E*' },
    { key: 'waveE',      title: 'Wave Energy',       buffer: 'aud.waveEnergy',  color: 'var(--chart-flux, #fb8c00)', unit: 'E*' },
    { key: 'eField',     title: 'E-Field Energy',    buffer: 'aud.eFieldEnergy',color: 'var(--chart-positive, #4ade80)', unit: 'E*' },
    { key: 'bField',     title: 'B-Field Energy',    buffer: 'aud.bFieldEnergy',color: 'var(--chart-negative, #f87171)', unit: 'E*' },
    { key: 'poynting',   title: 'Poynting Mag',      buffer: 'aud.poyntingMag', color: 'var(--chart-gauss, #fbbf24)', unit: 'S' },
    { key: 'chirality',  title: 'Chirality',         buffer: 'aud.chirality',   color: 'var(--chart-entropy, #60a5fa)', unit: 'χ' },
    { key: 'partKE',     title: 'Particle KE',       buffer: 'aud.particleKE',  color: 'var(--chart-positive, #4ade80)', unit: 'E*' },
    { key: 'coulombPE',  title: 'Coulomb PE',        buffer: 'aud.coulombPE',   color: 'var(--chart-negative, #f87171)', unit: 'E*' },
    { key: 'lagTotal',   title: 'Lagrangian (L)',    buffer: 'lag.total',       color: 'var(--chart-eb, #a78bfa)', unit: 'L' },
    { key: 'lagAction',  title: 'Total Action (S)',  buffer: 'lag.action',      color: 'var(--chart-gauss, #fbbf24)', unit: 'S' },
    { key: 'lagHam',     title: 'Hamiltonian (H)',   buffer: 'lag.hamiltonian', color: 'var(--chart-energy, #42a5f5)', unit: 'H' },
    { key: 'lagKinetic', title: 'Field Kinetic (T)', buffer: 'lag.fieldKinetic',color: 'var(--chart-flux, #fb8c00)', unit: 'T' },
    { key: 'lagGrad',    title: 'Field Gradient (V)',buffer: 'lag.fieldGradient',color: 'var(--chart-negative, #f87171)', unit: 'V' },
];

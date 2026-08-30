/**
 * Scale-0 telemetry grid channel registry.
 * Shared by TelemetryGridPanelComponent; mirrors diagnostics-panel trend buffers.
 * Canonical table rows live in ui/panels/diagnostics-panel/descriptors/scale0.js.
 */

export const SCALE0_GRID_CHANNELS = [
    { key: 'flux',       title: 'Total Flux',        buffer: 'flux',            telemetryGroup: 'diagnostics', color: 'var(--chart-flux, #fb8c00)',   unit: 'J' },
    { key: 'energy',     title: 'Total Energy',      buffer: 'aud.dynamicEnergy', telemetryGroup: 'audit', color: 'var(--chart-energy, #42a5f5)', unit: 'E*' },
    { key: 'manifested', title: 'Particle Count',    buffer: 'manifested',      telemetryGroup: 'diagnostics', color: 'var(--chart-eb, #a78bfa)',     unit: 'ct' },
    { key: 'charges',    title: 'Net Charge',        buffer: 'charges',         telemetryGroup: 'diagnostics', color: 'var(--chart-charge, #4ade80)', unit: 'e' },
    { key: 'positive',   title: 'Positive Charges',  buffer: 'positive',        telemetryGroup: 'diagnostics', color: 'var(--chart-positive, #4ade80)', unit: 'e' },
    { key: 'negative',   title: 'Negative Charges',  buffer: 'negative',        telemetryGroup: 'diagnostics', color: 'var(--chart-negative, #f87171)', unit: 'e' },
    { key: 'entropy',    title: 'Entropy',           buffer: 'entropy',         telemetryGroup: 'diagnostics', color: 'var(--chart-entropy, #60a5fa)', unit: 'nat' },
    { key: 'gauss',      title: 'Gauss Violation',   buffer: 'gauss',           telemetryGroup: 'audit', color: 'var(--chart-gauss, #fbbf24)',   unit: 'E*²' },
    { key: 'drift',      title: 'Energy Drift',      buffer: 'aud.energyDrift', telemetryGroup: 'audit', color: 'var(--chart-eb, #a78bfa)', unit: '%' },
    { key: 'ebDiff',     title: 'E-B Energy Diff',   buffer: 'ebDiff',          telemetryGroup: 'audit', color: 'var(--chart-eb, #a78bfa)', unit: 'E*' },
    { key: 'fieldE',     title: 'Field Energy',      buffer: 'aud.fieldEnergy', telemetryGroup: 'audit', color: 'var(--chart-energy, #42a5f5)', unit: 'E*' },
    { key: 'waveE',      title: 'Wave Energy',       buffer: 'aud.waveEnergy',  telemetryGroup: 'audit', color: 'var(--chart-flux, #fb8c00)', unit: 'E*' },
    { key: 'eField',     title: 'E-Field Energy',    buffer: 'aud.eFieldEnergy',telemetryGroup: 'audit', color: 'var(--chart-positive, #4ade80)', unit: 'E*' },
    { key: 'bField',     title: 'B-Field Energy',    buffer: 'aud.bFieldEnergy',telemetryGroup: 'audit', color: 'var(--chart-negative, #f87171)', unit: 'E*' },
    { key: 'poynting',   title: 'Poynting Mag',      buffer: 'aud.poyntingMag', telemetryGroup: 'audit', color: 'var(--chart-gauss, #fbbf24)', unit: 'S' },
    { key: 'chirality',  title: 'Chirality',         buffer: 'aud.chirality',   telemetryGroup: 'audit', color: 'var(--chart-entropy, #60a5fa)', unit: 'χ' },
    { key: 'partKE',     title: 'Particle KE',       buffer: 'aud.particleKE',  telemetryGroup: 'audit', color: 'var(--chart-positive, #4ade80)', unit: 'E*' },
    { key: 'coulombPE',  title: 'Coulomb PE',        buffer: 'aud.coulombPE',   telemetryGroup: 'audit', color: 'var(--chart-negative, #f87171)', unit: 'E*' },
    { key: 'lagTotal',   title: 'Lagrangian (L)',    buffer: 'lag.total',       telemetryGroup: 'lagrangian', color: 'var(--chart-eb, #a78bfa)', unit: 'L' },
    { key: 'lagAction',  title: 'Total Action (S)',  buffer: 'lag.action',      telemetryGroup: 'lagrangian', color: 'var(--chart-gauss, #fbbf24)', unit: 'S' },
    { key: 'lagHam',     title: 'Hamiltonian (H)',   buffer: 'lag.hamiltonian', telemetryGroup: 'lagrangian', color: 'var(--chart-energy, #42a5f5)', unit: 'H' },
    { key: 'lagKinetic', title: 'Field Kinetic (T)', buffer: 'lag.fieldKinetic',telemetryGroup: 'lagrangian', color: 'var(--chart-flux, #fb8c00)', unit: 'T' },
    { key: 'lagGrad',    title: 'Field Gradient (V)',buffer: 'lag.fieldGradient',telemetryGroup: 'lagrangian', color: 'var(--chart-negative, #f87171)', unit: 'V' },
];

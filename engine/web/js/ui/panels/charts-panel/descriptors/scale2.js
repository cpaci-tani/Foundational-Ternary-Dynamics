/**
 * Scale 2/3 (Atom / Molecule Engine) charts panel descriptor.
 * Buffers are populated by telemetryHub.collectScale2 — shared by both AE
 * scales ('2' atoms, '3' molecules) since they run the same engine.
 *
 * Units: AE energies/temperature are SIM UNITS (implicit k_B = 1, audit
 * P0-10) — labels say "(sim)", never MeV / Kelvin.
 */

export const charts = [
    {
        id: 'ae-energy',
        title: 'Atomic Energy',
        xLabel: 'sample',
        yLabel: 'E (sim)',
        defaultActive: true,
        series: [
            { key: 'ke',      label: 'KE',       color: 'var(--chart-ae-ke, #4ade80)',       buffer: 'aeKE', unit: '(sim)' },
            { key: 'ionic',   label: 'PE Ionic', color: 'var(--chart-ae-pe-ionic, #f87171)', buffer: 'aePEIonic', unit: '(sim)' },
            { key: 'vdw',     label: 'PE vdW',   color: 'var(--chart-ae-pe-vdw, #2dd4bf)',   buffer: 'aePEVdw', unit: '(sim)' },
            { key: 'bond',    label: 'PE Bond',  color: 'var(--chart-ae-pe-bond, #fb923c)',  buffer: 'aePEBond', unit: '(sim)' },
            { key: 'total',   label: 'Total',    color: 'var(--chart-ae-total, #e8e8e8)',    buffer: 'aeEnergy', unit: '(sim)' },
        ],
    },
    {
        id: 'ae-temperature',
        title: 'Temperature',
        xLabel: 'sample',
        yLabel: 'T (sim)',
        defaultActive: true,
        series: [
            { key: 'temp', label: 'T (sim)', color: 'var(--chart-ae-temp, #fb8c00)', buffer: 'aeTemp', unit: '(sim)' },
        ],
    },
    {
        id: 'ae-structure',
        title: 'Atoms & Bonds',
        xLabel: 'sample',
        yLabel: 'ct',
        defaultActive: true,
        series: [
            { key: 'atoms', label: 'Atoms', color: 'var(--chart-ae-atoms, #42a5f5)', buffer: 'aeAtomCount', unit: 'ct' },
            { key: 'bonds', label: 'Bonds', color: 'var(--chart-ae-bonds, #a78bfa)', buffer: 'aeBonds', unit: 'ct' },
        ],
    },
    {
        id: 'ae-momentum',
        title: 'Momentum',
        xLabel: 'sample',
        yLabel: '|p| (sim)',
        defaultActive: false,
        series: [
            { key: 'p', label: '|p|', color: 'var(--chart-ae-momentum, #60a5fa)', buffer: 'aeMomentum', unit: '(sim)' },
        ],
    },
    {
        id: 'ae-drift',
        title: 'Energy Drift',
        xLabel: 'sample',
        yLabel: '%',
        defaultActive: false,
        series: [
            { key: 'drift', label: 'Drift %', color: 'var(--chart-ae-drift, #fbbf24)', buffer: 'aeDrift', unit: '%' },
        ],
    },
];

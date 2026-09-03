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
            { key: 'angle',   label: 'PE Angle', color: 'var(--chart-ae-pe-angle, #facc15)', buffer: 'aePEAngle', unit: '(sim)' },
            { key: 'total',   label: 'Tracked',  color: 'var(--chart-ae-total, #e8e8e8)',    buffer: 'aeEnergy', unit: '(sim)' },
        ],
    },
    {
        id: 'ae-nuclear',
        title: 'Nuclear Reaction',
        xLabel: 'tick',
        yLabel: 'MeV / events',
        defaultActive: true,
        series: [
            { key: 'released', label: 'Released MeV', color: '#f97316', buffer: 'aeNuclearReleased', unit: 'MeV' },
            { key: 'deposited', label: 'Deposited', color: '#fb923c', buffer: 'aeNuclearDeposited', unit: 'MeV' },
            { key: 'transit', label: 'In transit', color: '#67e8f9', buffer: 'aeNuclearTransit', unit: 'MeV' },
            { key: 'escaped', label: 'Escaped', color: '#c084fc', buffer: 'aeNuclearEscaped', unit: 'MeV' },
            { key: 'events', label: 'Events', color: '#fde047', buffer: 'aeReactionEvents', unit: 'ct' },
        ],
    },
    {
        id: 'ae-nuclear-population',
        title: 'Nuclear Population',
        xLabel: 'tick',
        yLabel: 'count',
        defaultActive: true,
        series: [
            { key: 'fuel', label: 'Fuel records', color: '#a3e635', buffer: 'aeNuclearFuel', unit: 'ct' },
            { key: 'neutrons', label: 'Live neutrons', color: '#67e8f9', buffer: 'aeNuclearLiveNeutrons', unit: 'ct' },
            { key: 'generation', label: 'Generation', color: '#38bdf8', buffer: 'aeNuclearGeneration', unit: 'ct' },
            { key: 'events', label: 'Events', color: '#fde047', buffer: 'aeReactionEvents', unit: 'ct' },
            { key: 'rate', label: 'Events / 100 ticks', color: '#fb7185', buffer: 'aeNuclearEventRate', unit: 'ct' },
            { key: 'k', label: 'Observed reproduction', color: '#c084fc', buffer: 'aeNuclearK', unit: '' },
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
        title: 'Conservative Drift',
        xLabel: 'sample',
        yLabel: '%',
        defaultActive: false,
        series: [
            { key: 'drift', label: 'Drift %', color: 'var(--chart-ae-drift, #fbbf24)', buffer: 'aeDrift', unit: '%' },
        ],
    },
];

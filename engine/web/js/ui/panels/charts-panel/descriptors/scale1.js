/**
 * Scale 1 charts panel descriptor.
 * Buffers are populated by telemetryHub.collectScale1/collectScale1Extended.
 */

export const charts = [
    {
        id: 'pe-energy',
        title: 'Particle Energy',
        xLabel: 'sample',
        yLabel: 'MeV',
        defaultActive: true,
        series: [
            { key: 'ke',      label: 'KE',         color: 'var(--chart-pe-ke, #4ade80)',      buffer: 'peKE', unit: 'MeV' },
            { key: 'coulomb', label: 'Coulomb PE', color: 'var(--chart-pe-coulomb, #f87171)', buffer: 'peCoulombPE', unit: 'MeV' },
            { key: 'gravity', label: 'Gravity PE', color: 'var(--chart-pe-gravity, #94a3b8)', buffer: 'peGravityPE', unit: 'MeV' },
            { key: 'total',   label: 'Total',      color: 'var(--chart-pe-total, #e8e8e8)',   buffer: 'peTotal', unit: 'MeV' },
        ],
    },
    {
        id: 'pe-momentum',
        title: 'Momentum & Angular Momentum',
        xLabel: 'sample',
        yLabel: 'magnitude',
        defaultActive: true,
        series: [
            { key: 'p', label: '|p|', color: 'var(--chart-pe-momentum, #a78bfa)', buffer: 'peMomentum', unit: 'MeV/c' },
            { key: 'l', label: '|L|', color: 'var(--chart-pe-angmom, #60a5fa)',   buffer: 'peAngMom', unit: 'hbar' },
        ],
    },
    {
        id: 'pe-forces',
        title: 'Net Forces',
        xLabel: 'sample',
        yLabel: '|F|',
        defaultActive: true,
        series: [
            { key: 'max',  label: 'Max |F|',  color: 'var(--chart-pe-force, #fbbf24)',      buffer: 'peMaxForce', unit: '|F|' },
            { key: 'mean', label: 'Mean |F|', color: 'var(--chart-pe-force-mean, #fb923c)', buffer: 'peMeanForce', unit: '|F|' },
        ],
    },
    {
        id: 'pe-counts',
        title: 'Particle Counts',
        xLabel: 'sample',
        yLabel: 'ct',
        defaultActive: false,
        series: [
            { key: 'total',  label: 'Total',  color: 'var(--chart-pe-count, #fb8c00)',  buffer: 'peCount', unit: 'ct' },
            { key: 'locked', label: 'Locked', color: 'var(--chart-pe-locked, #fbbf24)', buffer: 'peLockedCount', unit: 'ct' },
            { key: 'mobile', label: 'Mobile', color: 'var(--chart-pe-mobile, #42a5f5)', buffer: 'peMobileCount', unit: 'ct' },
        ],
    },
    {
        id: 'pe-orbit',
        title: 'Two-Body Orbit',
        xLabel: 'sample',
        yLabel: 'lu / c',
        defaultActive: false,
        series: [
            { key: 'r',  label: 'Separation r', color: 'var(--chart-pe-radius, #42a5f5)', buffer: 'peSeparation', unit: 'lu' },
            { key: 'vr', label: 'Radial v',     color: 'var(--chart-pe-radial, #ef4444)', buffer: 'peRadialVelocity', unit: 'c' },
        ],
    },
    {
        id: 'pe-thermo',
        title: 'Virial & RMS Velocity',
        xLabel: 'sample',
        yLabel: 'ratio / c',
        defaultActive: true,
        series: [
            { key: 'virial', label: '2K/|U|', color: 'var(--chart-pe-virial, #fbbf24)', buffer: 'peVirial', unit: 'ratio' },
            { key: 'vrms',   label: 'v RMS',  color: 'var(--chart-pe-vrms, #4ade80)',   buffer: 'peRmsVelocity', unit: 'c' },
        ],
    },
];

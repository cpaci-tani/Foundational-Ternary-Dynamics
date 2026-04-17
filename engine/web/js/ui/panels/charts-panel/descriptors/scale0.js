/**
 * Scale 0 charts panel descriptor.
 * Each chart declares id / title / series[] / labels. Colors use CSS custom
 * properties so themes can override.
 */

export const charts = [
    {
        id: 'flux-energy',
        title: 'Flux & Energy',
        xLabel: 'tick',
        yLabel: 'E*',
        defaultActive: true,
        series: [
            { key: 'flux',   label: 'Flux',   color: 'var(--chart-flux,   #fb8c00)', buffer: 'flux' },
            { key: 'energy', label: 'Energy', color: 'var(--chart-energy, #42a5f5)', buffer: 'energy' },
        ],
    },
    {
        id: 'particles',
        title: 'Particle Count',
        xLabel: 'tick',
        yLabel: 'ct',
        defaultActive: true,
        series: [
            { key: 'total',    label: 'Total',    color: 'var(--chart-total,    #e8e8e8)', buffer: 'manifested' },
            { key: 'positive', label: 'Positive', color: 'var(--chart-positive, #4ade80)', buffer: 'positive' },
            { key: 'negative', label: 'Negative', color: 'var(--chart-negative, #f87171)', buffer: 'negative' },
        ],
    },
    {
        id: 'charge',
        title: 'Charge Balance',
        xLabel: 'tick',
        yLabel: 'pos − neg',
        defaultActive: true,
        series: [
            { key: 'charge', label: 'Charge', color: 'var(--chart-charge, #4ade80)', buffer: 'charges' },
        ],
    },
    {
        id: 'eb-energy',
        title: 'E vs B Field Energy',
        xLabel: 'tick',
        yLabel: 'E* (E − B)',
        defaultActive: false,
        series: [
            { key: 'eb', label: 'E − B', color: 'var(--chart-eb, #a78bfa)', buffer: 'ebDiff' },
        ],
    },
    {
        id: 'gauss',
        title: 'Gauss Violation',
        xLabel: 'tick',
        yLabel: 'E*²',
        defaultActive: false,
        series: [
            { key: 'gauss', label: 'Violation', color: 'var(--chart-gauss, #fbbf24)', buffer: 'gauss' },
        ],
    },
    {
        id: 'entropy',
        title: 'Entropy',
        xLabel: 'tick',
        yLabel: 'nat',
        defaultActive: true,
        series: [
            { key: 'entropy', label: 'Entropy', color: 'var(--chart-entropy, #60a5fa)', buffer: 'entropy' },
        ],
    },
];

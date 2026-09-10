/**
 * Scale 5 Cosmic chart descriptors.
 *
 * Pass 0b (UI foundations, 2026-09-09): dynamics charts only (mirrors the
 * diagnostics descriptor's "runtime and dynamics sections only" scope for
 * this pass — runtime fields such as scenario/solver-path/dt are static
 * snapshots, not time series, so they have no chart card of their own here,
 * matching the scale4 descriptor precedent). Gas (Pass B), cosmology
 * (Pass C), and population/event (Pass D) charts follow in later passes.
 */

export const charts = [
    {
        id: 'cosmic-energy', title: 'Mechanical Energy Proxy (Softened)', xLabel: 'public tick', yLabel: '(sim)', defaultActive: true,
        series: [
            { key: 'ke', label: 'Kinetic', color: '#4ade80', buffer: 'csKE', unit: '(sim)' },
            { key: 'pe', label: 'Potential (softened)', color: '#f87171', buffer: 'csPE', unit: '(sim)' },
            { key: 'total', label: 'Total', color: '#f8fafc', buffer: 'csTotal', unit: '(sim)' },
        ],
    },
    {
        id: 'cosmic-conservation', title: 'Mechanical Change & Momentum', xLabel: 'public tick', yLabel: 'relative / (sim)', defaultActive: true,
        series: [
            { key: 'drift', label: 'Mechanical proxy Δ', color: '#fbbf24', buffer: 'csDrift', unit: '%' },
            { key: 'momentum', label: '|Total momentum|', color: '#a78bfa', buffer: 'csMomentum', unit: '(sim)' },
        ],
    },
    {
        id: 'cosmic-virial', title: 'Virial Ratio', xLabel: 'public tick', yLabel: '2K/|U|', defaultActive: false,
        series: [
            { key: 'virial', label: '2K/|U|', color: '#22d3ee', buffer: 'csVirial', unit: 'ratio' },
        ],
    },
    {
        id: 'cosmic-angular-com', title: 'Angular Momentum & COM Drift', xLabel: 'public tick', yLabel: '(sim) / lu', defaultActive: false,
        series: [
            { key: 'ang-mom', label: '|Angular momentum|', color: '#60a5fa', buffer: 'csAngMom', unit: '(sim)' },
            { key: 'com-drift', label: 'COM drift', color: '#fb923c', buffer: 'csComDrift', unit: 'lu' },
        ],
    },
];

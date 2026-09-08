/** Scale 4 Solar System chart descriptors. */

export const charts = [
    {
        id: 'solar-energy', title: 'Mechanical Energy Proxy', xLabel: 'public tick', yLabel: 'M☉·AU²/yr²', defaultActive: true,
        series: [
            { key: 'ke', label: 'Kinetic', color: '#4ade80', buffer: 'plKE', unit: 'M☉·AU²/yr²' },
            { key: 'pe', label: 'Potential', color: '#f87171', buffer: 'plPE', unit: 'M☉·AU²/yr²' },
            { key: 'total', label: 'Total', color: '#f8fafc', buffer: 'plTotal', unit: 'M☉·AU²/yr²' },
        ],
    },
    {
        id: 'solar-core-gravity', title: 'Newtonian Acceleration', xLabel: 'public tick', yLabel: 'AU/yr²', defaultActive: false,
        series: [
            { key: 'newtonian', label: 'Newtonian max', color: '#60a5fa', buffer: 'plNewtonianAccel', unit: 'AU/yr²' },
        ],
    },
    {
        id: 'solar-perturbations', title: 'Effective-Physics Accelerations', xLabel: 'public tick', yLabel: 'AU/yr²', defaultActive: true,
        series: [
            { key: 'relativity', label: '1PN', color: '#c084fc', buffer: 'plRelAccel', unit: 'AU/yr²' },
            { key: 'j2', label: 'J₂', color: '#38bdf8', buffer: 'plJ2Accel', unit: 'AU/yr²' },
            { key: 'radiation-pressure', label: 'Radiation pressure', color: '#facc15', buffer: 'plRadiationPressureAccel', unit: 'AU/yr²' },
            { key: 'pr-drag', label: 'P-R drag', color: '#fb923c', buffer: 'plPrDragAccel', unit: 'AU/yr²' },
            { key: 'solar-wind', label: 'Solar-wind drag', color: '#a3e635', buffer: 'plSolarWindAccel', unit: 'AU/yr²' },
            { key: 'tide', label: 'Tides', color: '#2dd4bf', buffer: 'plTideAccel', unit: 'AU/yr²' },
            { key: 'atmosphere', label: 'Atmosphere', color: '#fb7185', buffer: 'plAtmosphereAccel', unit: 'AU/yr²' },
        ],
    },
    {
        id: 'solar-events', title: 'Dissipation & Event Ledger', xLabel: 'public tick', yLabel: 'cumulative', defaultActive: true,
        series: [
            { key: 'dissipated', label: 'Dissipated energy', color: '#f97316', buffer: 'plDissipated', unit: 'M☉·AU²/yr²' },
            { key: 'collisions', label: 'Impacts', color: '#ef4444', buffer: 'plCollisions', unit: 'ct' },
            { key: 'roche', label: 'Roche events', color: '#a78bfa', buffer: 'plRoche', unit: 'ct' },
            { key: 'mass-loss', label: 'Stellar mass lost', color: '#38bdf8', buffer: 'plMassLoss', unit: 'M☉' },
        ],
    },
    {
        id: 'solar-dissipation-rate', title: 'Instantaneous Dissipation', xLabel: 'public tick', yLabel: 'M☉·AU²/yr³', defaultActive: false,
        series: [
            { key: 'power', label: 'Dissipation rate', color: '#fb7185', buffer: 'plDissipationPower', unit: 'M☉·AU²/yr³' },
        ],
    },
    {
        id: 'solar-conservation', title: 'Mechanical Change & Momentum', xLabel: 'public tick', yLabel: 'relative', defaultActive: true,
        series: [
            { key: 'drift', label: 'Mechanical proxy Δ', color: '#fbbf24', buffer: 'plEnergyDrift', unit: '%' },
            { key: 'momentum', label: '|Material momentum|', color: '#a78bfa', buffer: 'plMomentum', unit: 'M☉·AU/yr' },
        ],
    },
    {
        id: 'solar-geometry', title: 'System Geometry', xLabel: 'public tick', yLabel: 'AU / count', defaultActive: true,
        series: [
            { key: 'radius', label: 'System radius', color: '#38bdf8', buffer: 'plSystemRadius', unit: 'AU' },
            { key: 'bodies', label: 'Bodies', color: '#fb923c', buffer: 'plCount', unit: 'ct' },
        ],
    },
    {
        id: 'solar-virial', title: 'Virial Ratio', xLabel: 'public tick', yLabel: '2K/|U|', defaultActive: false,
        series: [
            { key: 'virial', label: '2K/|U|', color: '#22d3ee', buffer: 'plVirial', unit: 'ratio' },
        ],
    },
];

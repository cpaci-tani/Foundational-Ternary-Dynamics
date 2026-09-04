/** Scale 3 molecule-only chart descriptors. */

export const charts = [
    {
        id: 'mol-modes', title: 'Molecular Kinetic Modes', xLabel: 'tick', yLabel: 'KE (sim)', defaultActive: true,
        series: [
            { key: 'translation', label: 'Translation', color: '#38bdf8', buffer: 'aeMolTranslation', unit: '(sim)' },
            { key: 'rotation', label: 'Rotation', color: '#c084fc', buffer: 'aeMolRotation', unit: '(sim)' },
            { key: 'vibration', label: 'Internal / vibration', color: '#facc15', buffer: 'aeMolVibration', unit: '(sim)' },
            { key: 'total', label: 'Total KE', color: '#f8fafc', buffer: 'aeKE', unit: '(sim)' },
        ],
    },
    {
        id: 'mol-energy', title: 'Molecular Energy', xLabel: 'tick', yLabel: 'E (sim)', defaultActive: true,
        series: [
            { key: 'vdw', label: 'PE vdW', color: '#2dd4bf', buffer: 'aePEVdw', unit: '(sim)' },
            { key: 'bond', label: 'PE bond', color: '#fb923c', buffer: 'aePEBond', unit: '(sim)' },
            { key: 'angle', label: 'PE angle', color: '#facc15', buffer: 'aePEAngle', unit: '(sim)' },
            { key: 'total', label: 'Tracked total', color: '#e8e8e8', buffer: 'aeEnergy', unit: '(sim)' },
        ],
    },
    {
        id: 'mol-topology', title: 'Molecular Topology', xLabel: 'tick', yLabel: 'count', defaultActive: true,
        series: [
            { key: 'atoms', label: 'Atoms', color: '#60a5fa', buffer: 'aeAtomCount', unit: 'ct' },
            { key: 'bonds', label: 'Bonds', color: '#a78bfa', buffer: 'aeBonds', unit: 'ct' },
            { key: 'components', label: 'Components', color: '#34d399', buffer: 'aeMolComponents', unit: 'ct' },
            { key: 'largest', label: 'Largest component', color: '#f472b6', buffer: 'aeMolLargest', unit: 'atoms' },
            { key: 'changes', label: 'Topology changes', color: '#fb7185', buffer: 'aeMolTopologyChanges', unit: 'ct' },
        ],
    },
    {
        id: 'mol-geometry', title: 'Molecular Geometry', xLabel: 'tick', yLabel: 'relative', defaultActive: true,
        series: [
            { key: 'radius', label: 'Radius of gyration', color: '#22d3ee', buffer: 'aeMolRadius', unit: 'lu' },
            { key: 'strain', label: 'RMS bond strain', color: '#f97316', buffer: 'aeMolBondStrain', unit: 'frac' },
            { key: 'dipole', label: 'Dipole magnitude', color: '#e879f9', buffer: 'aeMolDipole', unit: 'e·lu' },
        ],
    },
    {
        id: 'mol-temperature', title: 'Temperature Proxy', xLabel: 'tick', yLabel: 'T (sim)', defaultActive: false,
        series: [{ key: 'temperature', label: 'T*', color: '#fb8c00', buffer: 'aeTemp', unit: '(sim)' }],
    },
    {
        id: 'mol-conservation', title: 'Conservation', xLabel: 'tick', yLabel: 'relative', defaultActive: false,
        series: [
            { key: 'momentum', label: '|p|', color: '#60a5fa', buffer: 'aeMomentum', unit: '(sim)' },
            { key: 'drift', label: 'Energy drift', color: '#fbbf24', buffer: 'aeDrift', unit: '%' },
            { key: 'clamps', label: 'Force clamps', color: '#ef4444', buffer: 'aeForceClampEvents', unit: 'ct' },
        ],
    },
];

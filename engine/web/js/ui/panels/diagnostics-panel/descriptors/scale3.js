/** Scale 3 live molecule diagnostics. Every row is runtime-derived. */

export const sections = [
    {
        id: 'mol-topology',
        title: 'Molecular Topology',
        rows: [
            { id: 'mol-atoms', label: 'Atoms', unit: 'ct', source: 's2.diag.atomCount', trend: 'aeAtomCount', tooltip: 'Live atom records in the molecule engine.' },
            { id: 'mol-bonds', label: 'Bonds', unit: 'ct', source: 's2.molecule.bondCount', trend: 'aeBonds', tooltip: 'Live undirected edges in the current molecular graph.' },
            { id: 'mol-components', label: 'Connected Components', unit: 'ct', source: 's2.molecule.componentCount', trend: 'aeMolComponents', tooltip: 'Bond-connected components derived from the live graph; isolated atoms each count as one component.' },
            { id: 'mol-molecules', label: 'Multi-Atom Molecules', unit: 'ct', source: 's2.molecule.moleculeCount', tooltip: 'Current connected components containing two or more atoms.' },
            { id: 'mol-isolated', label: 'Isolated Atoms', unit: 'ct', source: 's2.molecule.isolatedAtoms', tooltip: 'Current components containing exactly one atom.' },
            { id: 'mol-largest', label: 'Largest Component', unit: 'atoms', source: 's2.molecule.largestComponent', trend: 'aeMolLargest', tooltip: 'Atom count of the largest live bond-connected component.' },
            { id: 'mol-formed', label: 'Bonds Formed', unit: 'ct', source: 's2.molecule.formedBonds', trend: 'aeMolTopologyChanges', tooltip: 'Current edges absent from the topology snapshot captured immediately after scenario construction.' },
            { id: 'mol-broken', label: 'Bonds Broken', unit: 'ct', source: 's2.molecule.brokenBonds', tooltip: 'Snapshot edges absent from the current live topology.' },
            { id: 'mol-order', label: 'Order Changes', unit: 'ct', source: 's2.molecule.orderChanges', tooltip: 'Live edges whose effective integer bond order differs from the scenario snapshot.' },
            { id: 'mol-match', label: 'Topology Matches Seed', unit: '', format: 'boolean', source: 's2.molecule.topologyMatch', tooltip: 'Whether atom count, edges, and effective bond orders match the post-construction reference snapshot.' },
        ],
    },
    {
        id: 'mol-kinetic-modes',
        title: 'Kinetic Mode Decomposition',
        rows: [
            { id: 'mol-translation', label: 'Translation KE', unit: '(sim)', source: 's2.molecule.translationalKE', trend: 'aeMolTranslation', tooltip: 'Sum of center-of-mass translational kinetic energy over live bond-connected components.' },
            { id: 'mol-rotation', label: 'Rotation KE', unit: '(sim)', source: 's2.molecule.rotationalKE', trend: 'aeMolRotation', tooltip: 'Classical rigid-body rotational kinetic energy from each component inertia tensor and angular momentum.' },
            { id: 'mol-vibration', label: 'Internal / Vibration KE', unit: '(sim)', source: 's2.molecule.vibrationalKE', trend: 'aeMolVibration', tooltip: 'Residual internal kinetic energy after center-of-mass translation and rigid rotation are removed.' },
            { id: 'mol-closure', label: 'Mode-Sum KE', unit: '(sim)', source: 's2.molecule.kineticClosure', tooltip: 'Translation plus rotation plus residual internal kinetic energy.' },
            { id: 'mol-ke', label: 'Engine Total KE', unit: '(sim)', source: 's2.diag.totalKE', trend: 'aeKE', tooltip: 'Total kinetic energy reported by the AtomEngine; compare with the mode sum.' },
            { id: 'mol-angular', label: 'Internal |L|', unit: '(sim)', source: 's2.molecule.angularMomentum', tooltip: 'Magnitude of component angular momenta about their own centers of mass.' },
            { id: 'mol-temperature', label: 'Temperature Proxy', unit: '(sim)', source: 's2.diag.temperature', trend: 'aeTemp', tooltip: 'Reduced-unit equipartition proxy 2 KE/(3N); this is not Kelvin.' },
        ],
    },
    {
        id: 'mol-geometry',
        title: 'Live Geometry',
        rows: [
            { id: 'mol-radius', label: 'Radius of Gyration', unit: 'lu', source: 's2.molecule.radiusOfGyration', trend: 'aeMolRadius', tooltip: 'Mass-weighted RMS distance from each component center of mass.' },
            { id: 'mol-dipole', label: 'Dipole Magnitude', unit: 'e·lu', source: 's2.molecule.dipoleMagnitude', trend: 'aeMolDipole', tooltip: 'Magnitude of effective-charge displacement about component centers; charge transfer is empirical when enabled.' },
            { id: 'mol-bond-mean', label: 'Mean Bond Length', unit: 'lu', source: 's2.molecule.meanBondLength', tooltip: 'Arithmetic mean of current lengths over live graph edges.' },
            { id: 'mol-bond-min', label: 'Minimum Bond Length', unit: 'lu', source: 's2.molecule.minBondLength', tooltip: 'Shortest current live bond length.' },
            { id: 'mol-bond-max', label: 'Maximum Bond Length', unit: 'lu', source: 's2.molecule.maxBondLength', tooltip: 'Longest current live bond length.' },
            { id: 'mol-bond-strain', label: 'RMS Relative Bond Strain', unit: 'frac', source: 's2.molecule.bondRmsStrain', trend: 'aeMolBondStrain', tooltip: 'Root-mean-square of (length minus equilibrium length) divided by equilibrium length over live bonds.' },
        ],
    },
    {
        id: 'mol-energy',
        title: 'Tracked Energy & Stability',
        rows: [
            { id: 'mol-total', label: 'Tracked Energy', unit: '(sim)', source: 's2.diag.totalEnergy', trend: 'aeEnergy', tooltip: 'Kinetic plus implemented scalar potential terms. Consult accounting status when non-conservative kernels are active.' },
            { id: 'mol-vdw', label: 'PE van der Waals', unit: '(sim)', source: 's2.diag.totalPEVdw', trend: 'aePEVdw', tooltip: 'Tracked Lennard-Jones pair potential excluding bonded and 1–3 pairs.' },
            { id: 'mol-bond-pe', label: 'PE Bond', unit: '(sim)', source: 's2.diag.totalPEBond', trend: 'aePEBond', tooltip: 'Tracked harmonic bond-spring potential.' },
            { id: 'mol-angle-pe', label: 'PE Angle', unit: '(sim)', source: 's2.diag.totalPEAngle', trend: 'aePEAngle', tooltip: 'Tracked effective harmonic angle-strain potential.' },
            { id: 'mol-accounting', label: 'Energy Accounting', unit: '', format: 'text', source: 's2.diag.energyStatus', tooltip: 'Complete-conservative only when every active kernel has a tracked scalar potential and no driver, clamp, or topology mutation is active.' },
            { id: 'mol-drift', label: 'Conservative Drift', unit: '%', compute: (hub) => hub.aeDrift.last(), trend: 'aeDrift', tooltip: 'Relative tracked-energy change from the current baseline; unavailable when the runtime is not conservative.' },
            { id: 'mol-momentum', label: 'Momentum', unit: '(sim)', format: 'vector', source: ['s2.diag.momentumX', 's2.diag.momentumY', 's2.diag.momentumZ'], tooltip: 'Live total linear momentum vector.' },
            { id: 'mol-clamps', label: 'Force Clamp Events', unit: 'ct', source: 's2.diag.forceClampEvents', trend: 'aeForceClampEvents', tooltip: 'Cumulative safety rescalings caused by force magnitude exceeding the stable integrator ceiling.' },
            { id: 'mol-tick', label: 'Molecule Tick', unit: 'tick', source: 's2.diag.tick', tooltip: 'Global AtomEngine integration tick for the current Scale 3 run.' },
            { id: 'mol-status', label: 'Engine Status', unit: '', format: 'text', source: 's2.diag.lastError', tooltip: 'Latest finite-state validation or intervention error; ok means no active engine error.' },
        ],
    },
];

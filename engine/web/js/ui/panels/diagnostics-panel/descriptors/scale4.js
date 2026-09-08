/** Scale 4 Solar System diagnostics. Every numeric row is live runtime state. */

export const sections = [
    {
        id: 'solar-runtime',
        title: 'Solar System Runtime',
        rows: [
            { id: 'scenario', label: 'Scenario', unit: '', format: 'text', source: 's4.diag.scenario', tooltip: 'Active Scale 4 dynamical system.' },
            { id: 'units', label: 'Unit System', unit: '', format: 'text', source: 's4.diag.units', tooltip: 'The default Solar System state uses astronomical units, solar masses, and Julian years.' },
            { id: 'gravity-mode', label: 'Dynamics Mode', unit: '', format: 'text', source: 's4.diag.gravityMode', tooltip: 'Physical is period-faithful Newtonian AU/M☉/yr dynamics. Presentation is an explicitly nonphysical slow comparison.' },
            { id: 'gravity', label: 'G', unit: 'AU³ M☉⁻¹ yr⁻²', source: 's4.runtime.G', tooltip: 'Gravitational constant actually used by the live bridge.' },
            { id: 'dt', label: 'Integrator Step', unit: 'day', source: 's4.diag.dtDays', tooltip: 'Fixed Velocity-Verlet substep in Julian days.' },
            { id: 'tick', label: 'Public Tick', unit: 'tick', source: 's4.diag.tick', tooltip: 'Number of user-visible Scale 4 ticks since scenario initialization. Each tick can contain multiple fixed solver substeps.' },
            { id: 'time-days', label: 'Elapsed Time', unit: 'day', source: 's4.diag.timeDays', tooltip: 'Physical elapsed model time since scenario initialization; the default Solar System reference starts at J2000.' },
            { id: 'time-years', label: 'Elapsed Time', unit: 'yr', source: 's4.diag.timeYears', tooltip: 'Physical elapsed model time in Julian years.' },
        ],
    },
    {
        id: 'solar-physics-stack',
        title: 'Effective Physics Stack',
        rows: [
            { id: 'active-kernels', label: 'Active Kernels', unit: 'ct', source: 's4.diag.activePhysicsCount', tooltip: 'Requested and structurally applicable physics kernels evaluated by the solver. An active event or drag kernel can have an instantaneous value of zero.' },
            { id: 'applicable-kernels', label: 'Applicable Kernels', unit: 'ct', source: 's4.diag.applicablePhysicsCount', tooltip: 'Physics kernels that have the bodies and parameters required by the current scenario.' },
            { id: 'available-kernels', label: 'Available Kernels', unit: 'ct', source: 's4.diag.availablePhysicsCount', tooltip: 'Total independently toggleable effective physics kernels implemented at Scale 4.' },
            { id: 'newtonian-max', label: 'Max Newtonian Acceleration', unit: 'AU/yr²', source: 's4.diag.forceAudit.newtonianAcceleration', trend: 'plNewtonianAccel', tooltip: 'Largest direct pairwise Newtonian acceleration in the latest force sweep.' },
            { id: 'relativity-max', label: 'Max 1PN Acceleration', unit: 'AU/yr²', source: 's4.diag.forceAudit.relativityAcceleration', trend: 'plRelAccel', tooltip: 'Largest dominant-star Schwarzschild first-post-Newtonian acceleration in the latest force sweep; this is not a full EIH integrator.' },
            { id: 'j2-max', label: 'Max J₂ Acceleration', unit: 'AU/yr²', source: 's4.diag.forceAudit.j2Acceleration', trend: 'plJ2Accel', tooltip: 'Largest axisymmetric quadrupole acceleration from a configured oblate parent.' },
            { id: 'radiation-max', label: 'Max Total Radiation Acceleration', unit: 'AU/yr²', source: 's4.diag.forceAudit.radiationAcceleration', trend: 'plRadiationAccel', tooltip: 'Largest combined radiation-pressure plus Poynting-Robertson and optional solar-wind acceleration in the latest force sweep.' },
            { id: 'radiation-pressure-max', label: 'Max Radiation Pressure', unit: 'AU/yr²', source: 's4.diag.forceAudit.radiationPressureAcceleration', trend: 'plRadiationPressureAccel', tooltip: 'Largest outward photon-pressure acceleration in the latest force sweep.' },
            { id: 'pr-drag-max', label: 'Max P-R Drag', unit: 'AU/yr²', source: 's4.diag.forceAudit.poyntingRobertsonDragAcceleration', trend: 'plPrDragAccel', tooltip: 'Largest Poynting-Robertson drag acceleration before the optional solar-wind correction.' },
            { id: 'solar-wind-max', label: 'Max Solar-Wind Drag', unit: 'AU/yr²', source: 's4.diag.forceAudit.solarWindDragAcceleration', trend: 'plSolarWindAccel', tooltip: 'Largest separately audited imposed solar-wind drag correction; zero when that toggle is off or inapplicable.' },
            { id: 'tide-max', label: 'Max Tidal Acceleration', unit: 'AU/yr²', source: 's4.diag.forceAudit.tideAcceleration', trend: 'plTideAccel', tooltip: 'Largest constant-Q equilibrium-tide tangential acceleration in the latest force sweep.' },
            { id: 'atmosphere-max', label: 'Max Atmospheric Drag', unit: 'AU/yr²', source: 's4.diag.forceAudit.atmosphereAcceleration', trend: 'plAtmosphereAccel', tooltip: 'Largest co-rotating exponential-atmosphere drag acceleration in the latest force sweep.' },
        ],
    },
    {
        id: 'solar-event-ledger',
        title: 'Dissipation & Events',
        rows: [
            { id: 'dissipated', label: 'Dissipated Energy Ledger', unit: 'M☉·AU²/yr²', source: 's4.diag.dissipatedEnergy', trend: 'plDissipated', tooltip: 'Cumulative energy transferred by tides, atmospheric/P-R drag, and perfectly inelastic impacts. It is tracked separately from mechanical energy.' },
            { id: 'dissipation-power', label: 'Instantaneous Dissipation', unit: 'M☉·AU²/yr³', source: 's4.diag.forceAudit.instantaneousDissipationPower', trend: 'plDissipationPower', tooltip: 'Latest force-sweep energy-transfer rate from tidal, atmospheric, Poynting-Robertson, and solar-wind drag.' },
            { id: 'mass-loss', label: 'Stellar Mass Lost', unit: 'M☉', source: 's4.diag.stellarMassLost', trend: 'plMassLoss', tooltip: 'Cumulative isotropic stellar mass removed by the configured fractional mass-loss law.' },
            { id: 'collisions', label: 'Finite-Body Impacts', unit: 'ct', source: 's4.diag.collisionCount', trend: 'plCollisions', tooltip: 'Momentum-conserving perfectly inelastic merge events resolved from physical-radius contact.' },
            { id: 'roche', label: 'Roche Disruptions', unit: 'ct', source: 's4.diag.rocheDisruptionCount', trend: 'plRoche', tooltip: 'Bodies fragmented after crossing a configured fluid Roche threshold above the parent surface.' },
            { id: 'atmosphere-entries', label: 'Atmosphere Entries', unit: 'ct', source: 's4.diag.atmosphereEntryCount', tooltip: 'Distinct outside-to-inside transitions into configured atmospheric envelopes.' },
            { id: 'latest-event', label: 'Latest Event', unit: '', format: 'text', source: 's4.diag.latestEvent.message', tooltip: 'Newest bounded event-ledger entry, including scenario load, atmosphere entry, collision, or Roche disruption.' },
        ],
    },
    {
        id: 'solar-population',
        title: 'Modeled Population',
        rows: [
            { id: 'bodies', label: 'Bodies', unit: 'ct', source: 's4.diag.bodyCount', trend: 'plCount', tooltip: 'Gravitating body records in the live N-body state. Belt particles are visual context and are not included.' },
            { id: 'planets', label: 'Planets', unit: 'ct', source: 's4.diag.planetCount', tooltip: 'Modeled primary planets orbiting the host star; dwarf planets are excluded from this count.' },
            { id: 'moons', label: 'Major Moons', unit: 'ct', source: 's4.diag.moonCount', tooltip: 'Selected major moons represented as gravitating bodies. This is not the complete catalog of known moons.' },
            { id: 'radius', label: 'System Radius', unit: 'AU', compute: (hub) => hub.plSystemRadius.last(), trend: 'plSystemRadius', tooltip: 'Maximum live distance from the system center of mass.' },
        ],
    },
    {
        id: 'solar-conservation',
        title: 'Mechanical Energy & Momentum',
        rows: [
            { id: 'ke', label: 'Kinetic Energy', unit: 'M☉·AU²/yr²', compute: (hub) => hub.plKE.last(), trend: 'plKE', tooltip: 'Live Newtonian kinetic energy summed over all modeled bodies.' },
            { id: 'pe', label: 'Potential Energy', unit: 'M☉·AU²/yr²', compute: (hub) => hub.plPE.last(), trend: 'plPE', tooltip: 'Live pairwise Newtonian gravitational potential energy using the same softening as the force solver.' },
            { id: 'total', label: 'Newtonian Mechanical Proxy', unit: 'M☉·AU²/yr²', compute: (hub) => hub.plTotal.last(), trend: 'plTotal', tooltip: 'Kinetic plus pairwise Newtonian potential energy. It excludes 1PN/J2/radiation potentials and must not be read as the complete Hamiltonian.' },
            { id: 'drift', label: 'Mechanical Proxy Change', unit: '%', compute: (hub) => hub.plEnergyDrift.last(), trend: 'plEnergyDrift', tooltip: 'Relative change in the Newtonian mechanical proxy since the first nonzero sample. It includes physical changes from enabled nonconservative kernels and is not labeled as pure numerical drift.' },
            { id: 'momentum', label: '|Material Momentum|', unit: 'M☉·AU/yr', compute: (hub) => hub.plMomentum.last(), trend: 'plMomentum', tooltip: 'Magnitude of modeled-body momentum. Photon, solar-wind, and stellar-ejecta reservoirs are not explicit bodies, so this is not a closed-system residual while those kernels are active.' },
            { id: 'virial', label: 'Virial Ratio 2K/|U|', unit: 'ratio', compute: (hub) => hub.plVirial.last(), trend: 'plVirial', tooltip: 'Instantaneous system-wide virial ratio. A multi-orbit Solar System snapshot need not equal one exactly.' },
        ],
    },
    {
        id: 'solar-provenance',
        title: 'Model Provenance',
        rows: [
            { id: 'epoch', label: 'Initialization Epoch', unit: '', format: 'text', source: 's4.runtime.provenance.epoch', tooltip: 'Reference epoch for the imported approximate planetary elements.' },
            { id: 'source', label: 'Orbital Source', unit: '', format: 'text', source: 's4.runtime.provenance.orbitalSource', tooltip: 'Source class for the primary-planet orbital elements.' },
            { id: 'solver', label: 'Dynamics', unit: '', format: 'text', source: 's4.runtime.provenance.dynamics', tooltip: 'Standard effective mechanics used by this scale.' },
            { id: 'perturbations', label: 'Perturbation Contract', unit: '', format: 'text', source: 's4.runtime.provenance.perturbationSource', tooltip: 'Imported approximation families represented by the toggle-gated effective physics stack.' },
            { id: 'epistemic', label: 'Epistemic Status', unit: '', format: 'text', source: 's4.runtime.provenance.epistemic', tooltip: 'Honest status of the astronomy inputs and mechanics relative to foundational FTD.' },
        ],
    },
];

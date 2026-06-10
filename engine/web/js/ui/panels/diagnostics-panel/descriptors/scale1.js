/**
 * Scale 1 diagnostics table descriptor.
 * These rows summarize the current ParticleEngine scenario using the same
 * descriptor-driven diagnostics surface as Scale 0.
 */

export const sections = [
    {
        id: 'pe-runtime',
        title: 'Scenario Dynamics',
        rows: [
            { id: 'scenario', label: 'Scenario', unit: '', format: 'text',
              source: 's1.runtime.scenario' },
            { id: 'dt', label: 'Time Step', unit: 'tick', source: 's1.runtime.dt' },
            { id: 'softening', label: 'Softening', unit: 'lu', source: 's1.runtime.softening' },
            { id: 'coulomb-on', label: 'Coulomb', unit: '', format: 'boolean',
              source: 's1.runtime.toggles.coulomb' },
            { id: 'gravity-on', label: 'Gravity', unit: '', format: 'boolean',
              source: 's1.runtime.toggles.gravity' },
            { id: 'damping-on', label: 'Damping', unit: '', format: 'boolean',
              source: 's1.runtime.toggles.damping' },
            { id: 'relativistic-on', label: 'Relativistic', unit: '', format: 'boolean',
              source: 's1.runtime.toggles.relativistic' },
        ],
    },
    {
        id: 'pe-hamiltonian',
        title: 'Active Hamiltonian',
        rows: [
            { id: 'count', label: 'Particles', unit: 'ct',
              source: 's1.diag.particleCount', trend: 'peCount' },
            { id: 'locked', label: 'Locked / Mobile', unit: 'ct', format: 'pair',
              compute: (hub) => [hub.peLockedCount.last(), hub.peMobileCount.last()] },
            { id: 'ke', label: 'Kinetic Energy', unit: 'MeV',
              source: 's1.diag.totalKE', trend: 'peKE' },
            { id: 'pe', label: 'Potential Energy', unit: 'MeV',
              source: 's1.diag.totalPE', trend: 'pePE' },
            { id: 'coulomb-pe', label: 'Coulomb PE', unit: 'MeV',
              source: 's1.diag.coulombPE', trend: 'peCoulombPE' },
            { id: 'gravity-pe', label: 'Gravity PE', unit: 'MeV',
              source: 's1.diag.gravityPE', trend: 'peGravityPE' },
            { id: 'total', label: 'Total Energy', unit: 'MeV',
              source: 's1.diag.totalEnergy', trend: 'peTotal' },
            { id: 'drift', label: 'Energy Drift', unit: '%',
              compute: (hub) => hub.peEnergyDrift.last(), trend: 'peEnergyDrift' },
        ],
    },
    {
        id: 'pe-conservation',
        title: 'Conservation',
        rows: [
            { id: 'momentum', label: 'Momentum', unit: 'MeV/c', format: 'vector',
              source: ['s1.diag.momentumX', 's1.diag.momentumY', 's1.diag.momentumZ'] },
            { id: 'momentum-mag', label: '|p|', unit: 'MeV/c',
              compute: (hub) => hub.peMomentum.last(), trend: 'peMomentum' },
            { id: 'angmom', label: 'Angular Mom', unit: 'hbar', format: 'vector',
              source: ['s1.diag.angMomX', 's1.diag.angMomY', 's1.diag.angMomZ'] },
            { id: 'angmom-mag', label: '|L|', unit: 'hbar',
              compute: (hub) => hub.peAngMom.last(), trend: 'peAngMom' },
            { id: 'virial', label: 'Virial 2K/|U|', unit: '',
              compute: (hub) => hub.peVirial.last(), trend: 'peVirial' },
        ],
    },
    {
        id: 'pe-field-forces',
        title: 'Forces & Geometry',
        rows: [
            { id: 'max-force', label: 'Max Net Force', unit: 'F',
              compute: (hub) => hub.peMaxForce.last(), trend: 'peMaxForce' },
            { id: 'mean-force', label: 'Mean Net Force', unit: 'F',
              compute: (hub) => hub.peMeanForce.last(), trend: 'peMeanForce' },
            { id: 'vrms', label: 'RMS Velocity', unit: 'c',
              compute: (hub) => hub.peRmsVelocity.last(), trend: 'peRmsVelocity' },
            { id: 'temperature', label: 'Temperature', unit: 'MeV',
              compute: (hub) => hub.peTemperature.last(), trend: 'peTemperature' },
            { id: 'radius', label: 'System Radius', unit: 'lu',
              compute: (hub) => hub.peSystemRadius.last(), trend: 'peSystemRadius' },
            { id: 'separation', label: '2-Body Separation', unit: 'lu',
              compute: (hub) => hub.peSeparation.last(), trend: 'peSeparation' },
            { id: 'radial-velocity', label: 'Radial Velocity', unit: 'c',
              compute: (hub) => hub.peRadialVelocity.last(), trend: 'peRadialVelocity' },
        ],
    },
];

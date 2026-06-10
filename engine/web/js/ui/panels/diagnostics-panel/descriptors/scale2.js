/**
 * Scale 2/3 diagnostics table descriptor (Atom / Molecule Engine).
 * Rows summarize the current AE scenario via the same descriptor-driven
 * diagnostics surface as Scales 0/1. One descriptor serves both AE scales
 * ('2' atoms, '3' molecules) — the panel root is `.scale-ae`-classed.
 *
 * Sources:
 *   s2.diag    — aeGetDiagnostics() snapshot: tick, atomCount, bondCount,
 *                totalKE, totalPEIonic, totalPEVdw, totalPEBond,
 *                totalEnergy, momentumX/Y/Z, temperature
 *   s2.runtime — aeGetRuntimeState() snapshot + scenario label:
 *                scenario, dt, softening, thermostatTemp, toggles.*
 *
 * Units: AE energies/temperature/momentum are SIM UNITS (implicit k_B = 1,
 * audit P0-10) — the unit column says "(sim)", never MeV / Kelvin.
 */

export const sections = [
    {
        id: 'ae-runtime',
        title: 'Scenario Dynamics',
        rows: [
            { id: 'scenario', label: 'Scenario', unit: '', format: 'text',
              source: 's2.runtime.scenario' },
            { id: 'dt', label: 'Time Step', unit: 'tick', source: 's2.runtime.dt' },
            { id: 'softening', label: 'Softening', unit: 'a₀', source: 's2.runtime.softening' },
            { id: 'ionic-on', label: 'Ionic (Coulomb)', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.ionic' },
            { id: 'vdw-on', label: 'Van der Waals', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.vdw' },
            { id: 'bonds-force-on', label: 'Bond Springs', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.bonds_force' },
            { id: 'bonding-on', label: 'Auto-Bonding', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.bonding' },
            { id: 'damping-on', label: 'Damping', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.damping' },
            { id: 'speed-limit-on', label: 'Speed Limit', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.speed_limit' },
        ],
    },
    {
        id: 'ae-phase3',
        title: 'Phase 3 Forces',
        rows: [
            { id: 'hbonds-on', label: 'H-Bonds', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.h_bonds' },
            { id: 'angle-on', label: 'VSEPR Angle Strain', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.angle_strain' },
            { id: 'dipole-on', label: 'Dipole-Dipole', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.dipole_dipole' },
            { id: 'thermostat-on', label: 'Thermostat', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.thermostat' },
            { id: 'electronegativity-on', label: 'Electronegativity', unit: '', format: 'boolean',
              source: 's2.runtime.toggles.electronegativity' },
            { id: 'thermostat-temp', label: 'Thermostat Target', unit: '(sim)',
              source: 's2.runtime.thermostatTemp' },
        ],
    },
    {
        id: 'ae-hamiltonian',
        title: 'Active Hamiltonian',
        rows: [
            { id: 'atoms', label: 'Atoms', unit: 'ct',
              source: 's2.diag.atomCount', trend: 'aeAtomCount' },
            { id: 'bonds', label: 'Bonds', unit: 'ct',
              source: 's2.diag.bondCount', trend: 'aeBonds' },
            { id: 'ke', label: 'Kinetic Energy', unit: '(sim)',
              source: 's2.diag.totalKE', trend: 'aeKE' },
            { id: 'pe-ionic', label: 'PE Ionic', unit: '(sim)',
              source: 's2.diag.totalPEIonic', trend: 'aePEIonic' },
            { id: 'pe-vdw', label: 'PE vdW', unit: '(sim)',
              source: 's2.diag.totalPEVdw', trend: 'aePEVdw' },
            { id: 'pe-bond', label: 'PE Bond', unit: '(sim)',
              source: 's2.diag.totalPEBond', trend: 'aePEBond' },
            { id: 'total', label: 'Total Energy', unit: '(sim)',
              source: 's2.diag.totalEnergy', trend: 'aeEnergy' },
            { id: 'drift', label: 'Energy Drift', unit: '%',
              compute: (hub) => hub.aeDrift.last(), trend: 'aeDrift' },
        ],
    },
    {
        id: 'ae-conservation',
        title: 'Conservation & Thermal',
        rows: [
            { id: 'momentum', label: 'Momentum', unit: '(sim)', format: 'vector',
              source: ['s2.diag.momentumX', 's2.diag.momentumY', 's2.diag.momentumZ'] },
            { id: 'momentum-mag', label: '|p|', unit: '(sim)',
              compute: (hub) => hub.aeMomentum.last(), trend: 'aeMomentum' },
            { id: 'temperature', label: 'Temperature', unit: '(sim)',
              source: 's2.diag.temperature', trend: 'aeTemp' },
            { id: 'tick', label: 'AE Tick', unit: 'tick',
              source: 's2.diag.tick' },
        ],
    },
];

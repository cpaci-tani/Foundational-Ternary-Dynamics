/**
 * Scale 5 Cosmic diagnostics.
 *
 * Pass 0b (UI foundations, 2026-09-09): runtime and dynamics sections only.
 * Gas (Pass B), cosmology (Pass C), and population/event-log (Pass D)
 * sections follow in later passes per the plan. `source` paths read against
 * `s5.cosmic` (telemetry-hub.js collectScale5) and `s5.diag` (the raw bridge
 * getDiagnostics() snapshot); `trend` names one of the widened `_s5_cs` ring
 * channels (telemetry-hub.js).
 */

export const sections = [
    {
        id: 'cosmic-runtime',
        title: 'Cosmic Runtime',
        rows: [
            { id: 'scenario', label: 'Scenario', unit: '', format: 'text', source: 's5.cosmic.runtime.scenarioName', tooltip: 'Active Scale 5 cosmic scenario.' },
            { id: 'solver-path', label: 'Solver Path', unit: '', format: 'text', source: 's5.cosmic.runtime.solverPath', tooltip: 'Direct pairwise summation below the body-count threshold; Barnes-Hut monopole approximation at or above it. The softened potential-energy row below is populated only on the direct branch.' },
            { id: 'solver-threshold', label: 'BH Threshold', unit: 'bodies', source: 's5.cosmic.runtime.bhThreshold', tooltip: 'Body count at or above which the solver switches from direct summation to the Barnes-Hut approximation.' },
            { id: 'dt', label: 'Integrator Step', unit: '', source: 's5.cosmic.runtime.dt', tooltip: 'Fixed Velocity-Verlet substep, in the lattice-internal time unit (not a physical second).' },
            { id: 'softening', label: 'Softening', unit: '(sim)', source: 's5.cosmic.runtime.softening', tooltip: 'Base gravitational softening length before any live per-type or global-scale multiplier.' },
            { id: 'gravity-scale', label: 'Gravity Scale', unit: 'x', source: 's5.cosmic.runtime.gravityScale', tooltip: 'Live multiplier on G_N [IMPOSED] in the gravity kernel; 1 is the scenario default.' },
            { id: 'box-size', label: 'Box Size', unit: 'lu', source: 's5.cosmic.runtime.boxSize', tooltip: 'Reference box size in lattice length units, used by the gas-lab profile binning and the comoving-box readout.' },
            { id: 'bodies', label: 'Bodies', unit: 'ct', source: 's5.cosmic.counts.bodyCount', trend: 'csBodies', tooltip: 'Live gravitating body records in the cosmic N-body state.' },
            { id: 'tick', label: 'Public Tick', unit: 'tick', source: 's5.diag.tick', tooltip: 'Number of cosmic-engine ticks since scenario initialization.' },
        ],
    },
    {
        id: 'cosmic-dynamics',
        title: 'Gravity & Dynamics',
        rows: [
            { id: 'ke', label: 'Kinetic Energy', unit: '(sim)', source: 's5.cosmic.energy.ke', trend: 'csKE', tooltip: 'Live Newtonian kinetic energy summed over all bodies, in lattice-internal units. Always available; independent of the potential-energy audit below.' },
            { id: 'pe', label: 'Softened Potential Energy', unit: '(sim)', source: 's5.cosmic.energy.pe', trend: 'csPE', tooltip: 'Pairwise gravitational potential energy on the SAME softened convention as the force kernel (not the unsoftened physical potential energy). Folded into the existing direct-sum gravity loop at no extra cost; NaN/unavailable while a Diagnostics, Charts, or Telemetry Grid consumer is not on screen, or while the Barnes-Hut branch is running (no pairwise loop exists there).' },
            { id: 'total', label: 'Mechanical Proxy', unit: '(sim)', source: 's5.cosmic.energy.total', trend: 'csTotal', tooltip: 'Kinetic plus softened pairwise potential energy. Unavailable whenever the potential-energy row above is unavailable.' },
            { id: 'virial', label: 'Virial Ratio 2K/|U|', unit: 'ratio', source: 's5.cosmic.energy.virial', trend: 'csVirial', tooltip: 'Instantaneous system-wide virial ratio using the softened potential energy above. A multi-orbit snapshot need not equal one exactly.' },
            { id: 'drift', label: 'Mechanical Proxy Change', unit: '%', source: 's5.cosmic.energy.drift', trend: 'csDrift', tooltip: 'Relative change in the mechanical proxy since its first available sample this scenario run. Includes physical changes from active phenomenological rules (accretion, mergers, evaporation, etc.), not pure numerical drift.' },
            { id: 'momentum', label: '|Total Momentum|', unit: '(sim)', source: 's5.cosmic.energy.momentum', trend: 'csMomentum', tooltip: 'Magnitude of total body momentum. O(N) and always available regardless of the potential-energy audit gate; a nonzero steady value can reflect real momentum injection or loss by the active phenomenological rules (accretion, mergers, ejecta, evaporation), not necessarily a numerical defect.' },
            { id: 'ang-mom', label: '|Angular Momentum|', unit: '(sim)', source: 's5.cosmic.energy.angMom', trend: 'csAngMom', tooltip: 'Magnitude of total angular momentum about the coordinate origin (not the center of mass). O(N) and always available.' },
            { id: 'com-drift', label: 'COM Drift', unit: 'lu', source: 's5.cosmic.energy.comDrift', trend: 'csComDrift', tooltip: 'Distance the center of mass has moved from its position at scenario load. O(N) and always available.' },
        ],
    },
];

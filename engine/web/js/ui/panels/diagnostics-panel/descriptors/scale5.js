/**
 * Scale 5 Cosmic diagnostics.
 *
 * Pass 0b (UI foundations, 2026-09-09): runtime and dynamics sections only.
 * Pass B (2026-09-10) adds `cosmic-gas`. Cosmology (Pass C), and
 * population/event-log (Pass D) sections follow in later passes per the
 * plan. `source` paths read against `s5.cosmic` (telemetry-hub.js
 * collectScale5) and `s5.diag` (the raw bridge getDiagnostics() snapshot);
 * `trend` names one of the widened `_s5_cs` ring channels
 * (telemetry-hub.js).
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
    {
        id: 'cosmic-gas',
        title: 'Gas (Monaghan SPH)',
        // s5.cosmic.stats is CosmicMockBridge.getPassStats() (the SAME
        // object populated by computeSphForces()); cosmic-pass-stats.js
        // zeroes sphGasCount whenever the SPH pass does not run this tick,
        // so gating on it hides the section on every non-gas scenario
        // (Ruling C3, Pass B — this section deliberately does NOT introduce
        // a separate s5.cosmic.sph block, since that would duplicate the
        // same twelve-plus numbers into a second location).
        visibleWhen: (hub) => (hub.s5?.cosmic?.stats?.sphGasCount ?? 0) > 0,
        rows: [
            { id: 'gas-count', label: 'Gas Bodies', unit: 'ct', source: 's5.cosmic.stats.sphGasCount', tooltip: 'GAS/NEBULA bodies swept into the Monaghan SPH pass this tick. [MEASURED — instrument]' },
            { id: 'gas-pairs', label: 'Neighbour Pairs', unit: 'ct', source: 's5.cosmic.stats.sphPairCount', tooltip: 'Unique gas-gas pairs within the smoothing-length cutoff this tick. [MEASURED — instrument]' },
            { id: 'gas-neighbor-min', label: 'Neighbour Count Min', unit: 'ct', source: 's5.cosmic.stats.neighborMin', tooltip: 'Fewest neighbours held by any gas body this tick. [MEASURED — instrument]' },
            { id: 'gas-neighbor-mean', label: 'Neighbour Count Mean', unit: 'ct', source: 's5.cosmic.stats.neighborMean', tooltip: 'Mean neighbour count over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-neighbor-max', label: 'Neighbour Count Max', unit: 'ct', source: 's5.cosmic.stats.neighborMax', tooltip: 'Most neighbours held by any gas body this tick. [MEASURED — instrument]' },
            { id: 'gas-h-min', label: 'Smoothing Length Min', unit: '(sim)', source: 's5.cosmic.stats.hMin', tooltip: 'Smallest adaptive SPH smoothing length h over gas bodies this tick. [IMPOSED effective gas dynamics; coefficients in engine units]' },
            { id: 'gas-h-mean', label: 'Smoothing Length Mean', unit: '(sim)', source: 's5.cosmic.stats.hMean', tooltip: 'Mean adaptive SPH smoothing length h over gas bodies this tick. [IMPOSED effective gas dynamics; coefficients in engine units]' },
            { id: 'gas-h-max', label: 'Smoothing Length Max', unit: '(sim)', source: 's5.cosmic.stats.hMax', tooltip: 'Largest adaptive SPH smoothing length h over gas bodies this tick. [IMPOSED effective gas dynamics; coefficients in engine units]' },
            { id: 'gas-rho-min', label: 'Density Min', unit: '(sim)', source: 's5.cosmic.stats.rhoMin', tooltip: 'Lowest SPH kernel-summed density over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-rho-max', label: 'Density Max', unit: '(sim)', source: 's5.cosmic.stats.rhoMax', tooltip: 'Highest SPH kernel-summed density over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-p-min', label: 'Pressure Min', unit: '(sim)', source: 's5.cosmic.stats.pMin', tooltip: 'Lowest ideal-gas-EOS pressure over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-p-max', label: 'Pressure Max', unit: '(sim)', source: 's5.cosmic.stats.pMax', tooltip: 'Highest ideal-gas-EOS pressure over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-c-min', label: 'Sound Speed Min', unit: '(sim)', source: 's5.cosmic.stats.cMin', tooltip: 'Lowest local sound speed c = sqrt(gamma P / rho) over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-c-max', label: 'Sound Speed Max', unit: '(sim)', source: 's5.cosmic.stats.cMax', tooltip: 'Highest local sound speed c = sqrt(gamma P / rho) over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-thermal', label: 'Thermal Energy', unit: '(sim)', source: 's5.cosmic.stats.thermal', tooltip: 'Sum of mass times internal energy over gas bodies this tick — the SPH energy-equation counterpart to kinetic energy below. [MEASURED — instrument]' },
            { id: 'gas-kinetic', label: 'Kinetic Energy', unit: '(sim)', source: 's5.cosmic.stats.kinetic', tooltip: 'Gas-only kinetic energy this tick, for direct comparison against thermal energy above (the whole-system kinetic energy in Gravity & Dynamics above includes non-gas bodies). [MEASURED — instrument]' },
        ],
    },
];

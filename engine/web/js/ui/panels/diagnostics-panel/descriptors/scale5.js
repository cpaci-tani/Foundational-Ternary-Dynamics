/**
 * Scale 5 Cosmic diagnostics.
 *
 * Pass 0b (UI foundations, 2026-09-09): runtime and dynamics sections only.
 * Pass B (2026-09-10) adds `cosmic-gas`. Pass A (2026-09-10) fills out
 * `cosmic-dynamics` with the system-radius and speed-limit/cull rows, and
 * carries Ruling B-M1 from Pass B's review: the three gas smoothing-length
 * rows below were mistagged [IMPOSED] and are now [MEASURED — instrument],
 * matching the density/pressure/sound-speed rows they sit beside (a
 * diagnostic reading of the running SPH scheme, not a control that sets an
 * imposed coefficient). Pass C (2026-09-10) adds `cosmic-expansion`: the
 * flat-ΛCDM background (scale factor, Hubble parameter, adot, redshift, H0,
 * Ω_m, Ω_Λ, dark-matter fraction, the display-only clock gain, and the
 * comoving box's expansion) — almost entirely reading the `cosmology` block
 * telemetry-hub.js's Pass-0b `collectScale5` already publishes; the one
 * addition is the `csScaleFactor` trend channel. Pass D (2026-09-10) adds
 * `cosmic-population`: the full nine-type census reading
 * `s5.cosmic.counts.countsByType` (already published; TYPE + 3 indexing,
 * matching mock-scale5.js's getDiagnostics()). The bounded event log
 * (`s5.cosmic.events` / `getEventLog()`, also already published) is NOT a
 * diagnostics-panel row — Scale 4's single `latestEvent.message` text row
 * (descriptors/scale4.js) is a fine precedent for one line, but a bounded
 * list of many events wants its own small scrolling widget, which this
 * pass places in the Physics rules controls card instead (Ruling: see
 * scales/scale5/ui/controls/component.js). Pass D also carries Ruling A-M1
 * from Pass A's review: the `cosmic-dynamics` ke/pe/total/virial/drift/
 * momentum/ang-mom/com-drift rows below now carry the same
 * `[MEASURED — instrument]` tag every later section's rows already had.
 * `source` paths read against `s5.cosmic` (telemetry-hub.js collectScale5)
 * and `s5.diag` (the raw bridge getDiagnostics() snapshot); `trend` names
 * one of the widened `_s5_cs` ring channels (telemetry-hub.js).
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
            { id: 'ke', label: 'Kinetic Energy', unit: '(sim)', source: 's5.cosmic.energy.ke', trend: 'csKE', tooltip: 'Live Newtonian kinetic energy summed over all bodies, in lattice-internal units. Always available; independent of the potential-energy audit below. [MEASURED — instrument]' },
            { id: 'pe', label: 'Softened Potential Energy', unit: '(sim)', source: 's5.cosmic.energy.pe', trend: 'csPE', tooltip: 'Pairwise gravitational potential energy on the SAME softened convention as the force kernel (not the unsoftened physical potential energy). Folded into the existing direct-sum gravity loop at no extra cost; NaN/unavailable while a Diagnostics, Charts, or Telemetry Grid consumer is not on screen, or while the Barnes-Hut branch is running (no pairwise loop exists there). [MEASURED — instrument]' },
            { id: 'total', label: 'Mechanical Proxy', unit: '(sim)', source: 's5.cosmic.energy.total', trend: 'csTotal', tooltip: 'Kinetic plus softened pairwise potential energy. Unavailable whenever the potential-energy row above is unavailable. [MEASURED — instrument]' },
            { id: 'virial', label: 'Virial Ratio 2K/|U|', unit: 'ratio', source: 's5.cosmic.energy.virial', trend: 'csVirial', tooltip: 'Instantaneous system-wide virial ratio using the softened potential energy above. A multi-orbit snapshot need not equal one exactly. [MEASURED — instrument]' },
            { id: 'drift', label: 'Mechanical Proxy Change', unit: '%', source: 's5.cosmic.energy.drift', trend: 'csDrift', tooltip: 'Relative change in the mechanical proxy since its first available sample this scenario run. Includes physical changes from active phenomenological rules (accretion, mergers, evaporation, etc.), not pure numerical drift. [MEASURED — instrument]' },
            { id: 'momentum', label: '|Total Momentum|', unit: '(sim)', source: 's5.cosmic.energy.momentum', trend: 'csMomentum', tooltip: 'Magnitude of total body momentum. O(N) and always available regardless of the potential-energy audit gate; a nonzero steady value can reflect real momentum injection or loss by the active phenomenological rules (accretion, mergers, ejecta, evaporation), not necessarily a numerical defect. [MEASURED — instrument]' },
            { id: 'ang-mom', label: '|Angular Momentum|', unit: '(sim)', source: 's5.cosmic.energy.angMom', trend: 'csAngMom', tooltip: 'Magnitude of total angular momentum about the coordinate origin (not the center of mass). O(N) and always available. [MEASURED — instrument]' },
            { id: 'com-drift', label: 'COM Drift', unit: 'lu', source: 's5.cosmic.energy.comDrift', trend: 'csComDrift', tooltip: 'Distance the center of mass has moved from its position at scenario load. O(N) and always available. [MEASURED — instrument]' },
            { id: 'system-radius', label: 'System Radius', unit: 'lu', source: 's5.cosmic.energy.systemRadius', trend: 'csSystemRadius', tooltip: 'Maximum distance from the instantaneous centre of mass to any body this tick. O(N) second pass over the body list (comX/comY/comZ above must finish first); always available regardless of the potential-energy audit gate. [MEASURED — instrument]' },
            { id: 'speed-limit-clamps', label: 'Speed-Limit Clamps', unit: 'ct', source: 's5.cosmic.stats.speedLimitClamps', tooltip: 'Bodies whose speed this tick exceeded the lattice speed limit c = 1/sqrt(3) [SELECTION] (scaled by the live speed-limit multiplier) and were rescaled back under it by the speed_limit gate. Zero whenever that gate is off. [MEASURED — instrument]' },
            { id: 'speed-limit-max-factor', label: 'Speed-Limit Max Factor', unit: 'x', source: 's5.cosmic.stats.speedLimitMaxFactor', tooltip: 'How far over the speed limit the fastest clamped body was this tick, as a ratio of its pre-clamp speed to the limit (0 when no clamps occurred this tick, not 1 — see the clamp-count row above). [MEASURED — instrument]' },
            { id: 'bodies-culled', label: 'Bodies Culled', unit: 'ct', source: 's5.cosmic.stats.bodiesCulled', tooltip: 'Bodies removed this tick by the mass > 0.01 cleanup filter that runs every tick regardless of toggle state (e.g. fully evaporated or fully accreted remnants). [MEASURED — instrument]' },
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
            { id: 'gas-h-min', label: 'Smoothing Length Min', unit: '(sim)', source: 's5.cosmic.stats.hMin', tooltip: 'Smallest adaptive SPH smoothing length h over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-h-mean', label: 'Smoothing Length Mean', unit: '(sim)', source: 's5.cosmic.stats.hMean', tooltip: 'Mean adaptive SPH smoothing length h over gas bodies this tick. [MEASURED — instrument]' },
            { id: 'gas-h-max', label: 'Smoothing Length Max', unit: '(sim)', source: 's5.cosmic.stats.hMax', tooltip: 'Largest adaptive SPH smoothing length h over gas bodies this tick. [MEASURED — instrument]' },
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
    {
        id: 'cosmic-expansion',
        title: 'Cosmology & Expansion',
        // Every row below reads the `cosmology` block telemetry-hub.js's
        // Pass-0b collectScale5 already publishes — this section adds no
        // new bridge field, only the csScaleFactor trend channel (Pass C).
        rows: [
            { id: 'scale-factor', label: 'Scale Factor a(t)', unit: '', source: 's5.cosmic.cosmology.scaleFactor', trend: 'csScaleFactor', tooltip: '[MEASURED — instrument] Flat-ΛCDM background scale factor, integrated each tick by the Friedmann solver from an early-universe initial condition (a=1 is "today"). Diagnostics-only — never feeds back into N-body dynamics.' },
            { id: 'hubble', label: 'Hubble Parameter H(t)', unit: '', source: 's5.cosmic.cosmology.hubbleParameter', trend: 'csHubble', tooltip: '[MEASURED — instrument] Present background expansion rate H(a) = H0 * sqrt(Omega_m * a^-3 + Omega_Lambda), decreasing toward the de Sitter floor H0*sqrt(Omega_Lambda) as a(t) grows.' },
            { id: 'adot', label: 'da/dt', unit: '', source: 's5.cosmic.cosmology.adot', tooltip: '[MEASURED — instrument] Instantaneous rate of change of the scale factor, adot = a * H — derived here exactly as the bridge\'s own Friedmann integrator computes it internally (not a separate bridge field).' },
            { id: 'redshift', label: 'Redshift z', unit: '', source: 's5.cosmic.cosmology.redshift', tooltip: '[MEASURED — instrument] z = 1/a - 1, the cosmological redshift implied by the current scale factor.' },
            { id: 'hubble0', label: 'H0 (anchor)', unit: '', source: 's5.cosmic.cosmology.hubble0', tooltip: '[IMPOSED] Lattice-unit Hubble-constant anchor (H0_LATTICE, constants.js "Cosmic-Lattice Anchors") calibrating the Friedmann integrator above; a tuning anchor, not a derived quantity — its calibration to physical SI units is undocumented.' },
            { id: 'omega-m', label: 'Omega_m', unit: '', source: 's5.cosmic.cosmology.omegaMatter', tooltip: '[CONJECTURE] Matter density parameter (1/3), the flat-universe complement of Omega_Lambda below (Omega_m + Omega_Lambda = 1). Not a derived quantity — see the Omega_Lambda tooltip for the fuller caveat this pair shares.' },
            { id: 'omega-l', label: 'Omega_Lambda', unit: '', source: 's5.cosmic.cosmology.omegaLambda', tooltip: '[CONJECTURE] Engine value, NOT a derived dark-energy density: Omega_Lambda = 2/3 does NOT match the observed Omega_Lambda ~ 0.685. FTD natively predicts Lambda = 0 (FC-1 declines hbar); any nonzero value is a [BOUNDARY] needing a horizon length. Same claim as the "Cosmology (FTD)" info card below, restated here as a live reading rather than a static string.' },
            { id: 'dm-fraction', label: 'Dark-Matter Fraction', unit: '%', source: 's5.cosmic.cosmology.dmFraction', trend: 'csDM', tooltip: '[SELECTION] FTD\'s native dark-matter fraction (Moore-shell selection) is 17/27 ~ 63%; that value does NOT match Planck 2018\'s observed Omega_DM/Omega_m ~ 84%. The number displayed here is the live measured fraction of the current population, which may reflect a chosen override — see the Cosmology controls card\'s DM-fraction select to change it (reseeds the population; does not convert existing bodies).' },
            { id: 'clock-gain', label: 'Clock Gain', unit: 'x', source: 's5.cosmic.cosmology.clockGain', tooltip: 'Presentation-only acceleration of the cosmic background clock — NOT physics. Multiplies the per-tick cosmic-time increment fed to the Friedmann integrator above so the dashboard shows readable expansion on human timescales; never enters the N-body force kernel or body kinematics (default 40).' },
            { id: 'box-comoving', label: 'Comoving Box Size', unit: 'lu', source: 's5.cosmic.cosmology.boxComoving', tooltip: '[MEASURED — instrument] Reference box size (Cosmic Runtime section above) times the current scale factor — how far the comoving box has expanded from its lattice-unit reference size. The viewport\'s comoving reference grid overlay (Cosmology overlay section) renders this literal size.' },
        ],
    },
    {
        id: 'cosmic-population',
        title: 'Population',
        // The full nine-type census, replacing the four-type readout the
        // controls-panel status strip has shown since before this pass
        // (scales/scale5/controller.js's cosmic-n-dm/gas/stars/bh spans,
        // unchanged by this pass). `s5.cosmic.counts.countsByType` is a
        // 9-element array indexed by TYPE + 3 (CosmicMockBridge.TYPE,
        // mock-scale5.js getDiagnostics()); every index below matches that
        // convention exactly. All nine rows are O(1) reads of a count the
        // bridge's existing O(N) getDiagnostics() sweep already produces —
        // no new bridge field, no new pass.
        rows: [
            { id: 'n-dark-energy', label: 'Dark Energy', unit: 'ct', source: 's5.cosmic.counts.countsByType.0', tooltip: '[MEASURED — instrument] Live DARK_ENERGY body count.' },
            { id: 'n-quasar', label: 'Quasars', unit: 'ct', source: 's5.cosmic.counts.countsByType.1', tooltip: '[MEASURED — instrument] Live QUASAR body count.' },
            { id: 'n-black-hole', label: 'Black Holes', unit: 'ct', source: 's5.cosmic.counts.countsByType.2', tooltip: '[MEASURED — instrument] Live BLACK_HOLE body count (excludes QUASAR, counted separately above).' },
            { id: 'n-dark-matter', label: 'Dark Matter', unit: 'ct', source: 's5.cosmic.counts.countsByType.3', tooltip: '[MEASURED — instrument] Live DARK_MATTER body count.' },
            { id: 'n-gas', label: 'Gas', unit: 'ct', source: 's5.cosmic.counts.countsByType.4', tooltip: '[MEASURED — instrument] Live GAS body count (excludes NEBULA, counted separately below).' },
            { id: 'n-star', label: 'Stars', unit: 'ct', source: 's5.cosmic.counts.countsByType.5', tooltip: '[MEASURED — instrument] Live STAR body count (excludes WHITE_DWARF and NEUTRON_STAR remnants, counted separately).' },
            { id: 'n-neutron-star', label: 'Neutron Stars', unit: 'ct', source: 's5.cosmic.counts.countsByType.6', tooltip: '[MEASURED — instrument] Live NEUTRON_STAR body count.' },
            { id: 'n-nebula', label: 'Nebulae', unit: 'ct', source: 's5.cosmic.counts.countsByType.7', tooltip: '[MEASURED — instrument] Live NEBULA body count (tidal/supernova/Hawking ejecta).' },
            { id: 'n-white-dwarf', label: 'White Dwarfs', unit: 'ct', source: 's5.cosmic.counts.countsByType.8', tooltip: '[MEASURED — instrument] Live WHITE_DWARF body count.' },
        ],
    },
];

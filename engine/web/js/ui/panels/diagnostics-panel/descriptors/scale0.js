/**
 * Scale 0 diagnostics table descriptor.
 *
 * Each row declares:
 *   id       — unique per section (DOM data-row + cell cache key)
 *   label    — Metric column
 *   unit     — Unit column ('' → em-dash rendered)
 *   source   — dotted path OR array of dotted paths on the telemetryHub
 *              (alternatively, use `compute(hub)` for derived values)
 *   compute  — optional (hub) => value function for computed metrics
 *   format   — 'scalar' (default) | 'vector' | 'pair' | 'triple'
 *   trend    — dotted path to a RingBuffer on the hub (e.g. 'aud.fieldEnergy')
 *   variant  — optional 'positive' | 'negative'
 *
 * Unit conventions (FTD lattice natural units):
 *   E*    — lattice energy unit (flux² · dx³)
 *   E*²   — energy squared (for squared violations)
 *   |J|   — flux magnitude
 *   |S|   — Poynting magnitude (E* · c / dx)
 *   ℏ     — action / angular-momentum unit
 *   nat   — natural-log entropy
 *   ct    — pure count
 *   ''    — dimensionless ratio (em-dash rendered)
 *
 * Scenario-conditional activation (when each metric reads non-zero):
 *
 *   ┌──────────────────────┬─────────────────────────────────────────┐
 *   │ Metric               │ Activates when                          │
 *   ├──────────────────────┼─────────────────────────────────────────┤
 *   │ Particle KE          │ ≥1 manifested particle with v ≠ 0       │
 *   │ Coulomb PE           │ ≥1 manifested charge AND `poisson_      │
 *   │                      │ coulomb` toggle on (computes phi_C)     │
 *   │ E-Field |E|²/2       │ Always (E = -wave_vel)                  │
 *   │ B-Field |B|²/2       │ Spatially-varying flux (B = curl J)     │
 *   │ Poynting |S|         │ Both E and B non-zero, non-aligned      │
 *   │ Angular Mom          │ ≥1 manifested particle off-center       │
 *   │ Gauss violation      │ Always; near-zero = constraint OK       │
 *   │ Max Gauss err        │ Always; spike = local violation         │
 *   │ Self-field inj       │ `self_field_floor` injection events     │
 *   │ E_L / E_R / wv L/R   │ `dual_substrate` toggle on              │
 *   │ Chirality            │ `dual_substrate` on AND L/R asymmetric  │
 *   └──────────────────────┴─────────────────────────────────────────┘
 *
 * On the WasmBridge path, all metrics flow from
 * `compute_energy_audit` in `engine/src/diagnostics_compute.cpp` → the
 * `get_energy_audit` binding in `engine/wasm/ftd_wasm.cpp`. On the
 * MockBridge path (when `useFluxMock` is true for flux-* / s0-seed-* /
 * s0-field-* scenarios), `getEnergyAudit` in
 * `engine/web/js/bridge/mock-diagnostics.js` returns a partial-audit
 * fallback (field/wave energies populated; particle, EM, Gauss, and
 * dual-substrate metrics hardcoded to 0). For the MockBridge path,
 * a flat metric is expected behavior, not drift.
 */

export const sections = [
    {
        id: 'particle-state',
        title: 'Particle State',
        rows: [
            { id: 'manifested', label: 'Manifested',      unit: 'ct', source: 's0.diag.manifested', trend: 'manifested' },
            { id: 'positive',   label: 'Positive',        unit: 'ct', source: 's0.diag.positive',   trend: 'positive',   variant: 'positive' },
            { id: 'negative',   label: 'Negative',        unit: 'ct', source: 's0.diag.negative',   trend: 'negative',   variant: 'negative' },
            { id: 'charge',     label: 'Charge (net)',    unit: 'ct', source: 's0.diag.chargeBalance', trend: 'charges' },
            { id: 'spin',       label: 'Spin Up/Down',    unit: 'ct', format: 'pair',
              source: ['s0.diag.spinUp', 's0.diag.spinDown'] },
            { id: 'color',      label: 'Color R/G/B',     unit: 'ct', format: 'triple',
              source: ['s0.diag.colorRed', 's0.diag.colorGreen', 's0.diag.colorBlue'] },
            { id: 'colorless',  label: 'Colorless',       unit: 'ct', source: 's0.diag.colorless' },
        ],
    },
    {
        id: 'energy-budget',
        title: 'Energy Budget',
        rows: [
            { id: 'energy',       label: 'Total Energy',      unit: 'E*', source: 's0.diag.totalEnergy',  trend: 'energy' },
            { id: 'field-energy', label: 'Field |J|\u00B2',   unit: 'E*', source: 's0.audit.fieldEnergy', trend: 'aud.fieldEnergy' },
            { id: 'wave-energy',  label: 'Wave |w|\u00B2',    unit: 'E*', source: 's0.audit.waveEnergy',  trend: 'aud.waveEnergy' },
            { id: 'particle-ke',  label: 'Particle KE',       unit: 'E*', source: 's0.audit.particleKE',  trend: 'aud.particleKE' },
            { id: 'coulomb-pe',   label: 'Coulomb PE',        unit: 'E*', source: 's0.audit.coulombPE',   trend: 'aud.coulombPE' },
            { id: 'flux',         label: 'Total Flux',        unit: '|J|', source: 's0.diag.totalFlux',   trend: 'flux' },
            { id: 'entropy',      label: 'Entropy',           unit: 'nat', source: 's0.diag.entropy',    trend: 'entropy' },
        ],
    },
    {
        id: 'electromagnetic',
        title: 'Electromagnetic',
        rows: [
            { id: 'e-field',  label: 'E-Field |E|\u00B2/2', unit: 'E*',
              compute: (hub) => hub.s0.audit?.EFieldEnergy ?? hub.s0.audit?.eFieldEnergy,
              trend: 'aud.eFieldEnergy' },
            { id: 'b-field',  label: 'B-Field |B|\u00B2/2', unit: 'E*',
              compute: (hub) => hub.s0.audit?.BFieldEnergy ?? hub.s0.audit?.bFieldEnergy,
              trend: 'aud.bFieldEnergy' },
            { id: 'poynting', label: 'Poynting |S|',        unit: '|S|',
              compute: (hub) => {
                  const a = hub.s0.audit; if (!a) return undefined;
                  const px = a.totalPoynting?.x ?? a.poyntingX ?? 0;
                  const py = a.totalPoynting?.y ?? a.poyntingY ?? 0;
                  const pz = a.totalPoynting?.z ?? a.poyntingZ ?? 0;
                  return Math.sqrt(px*px + py*py + pz*pz);
              },
              trend: 'aud.poyntingMag' },
            { id: 'ang-mom', label: 'Angular Mom',  unit: '\u210F', format: 'vector',
              source: ['s0.diag.angMomX', 's0.diag.angMomY', 's0.diag.angMomZ'] },
        ],
    },
    {
        id: 'constraints',
        title: 'Constraints',
        rows: [
            { id: 'gauss',     label: 'Gauss \u03A3(div J\u2212s)\u00B2', unit: 'E*\u00B2',
              source: 's0.audit.gaussViolation',     trend: 'gauss' },
            { id: 'max-gauss', label: 'Max Gauss err', unit: 'E*',
              source: 's0.audit.maxGaussError',      trend: 'aud.maxGaussError' },
            { id: 'self-inj',  label: 'Self-field inj', unit: 'E*',
              source: 's0.audit.selfFieldInjection', trend: 'aud.selfFieldInjection' },
        ],
    },
    {
        id: 'dual-substrate',
        title: 'Dual Substrate',
        rows: [
            { id: 'e-left',    label: 'E_L (left)',   unit: 'E*',
              compute: (hub) => hub.s0.audit?.ELTotal ?? hub.s0.audit?.eLTotal,
              trend: 'aud.eLeftEnergy' },
            { id: 'e-right',   label: 'E_R (right)',  unit: 'E*',
              compute: (hub) => hub.s0.audit?.ERTotal ?? hub.s0.audit?.eRTotal,
              trend: 'aud.eRightEnergy' },
            { id: 'chirality', label: 'Chirality',    unit: '',
              source: 's0.audit.chiralityTotal',     trend: 'aud.chirality' },
            { id: 'wave-lr',   label: 'Wave L / R',   unit: '|w|\u00B2', format: 'pair',
              source: ['s0.audit.wvLTotal', 's0.audit.wvRTotal'] },
        ],
    },
];

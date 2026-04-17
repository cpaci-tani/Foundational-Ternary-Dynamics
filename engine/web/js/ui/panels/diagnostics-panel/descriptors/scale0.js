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
 *   trend    — optional buffer name on the hub (dotted path allowed)
 *   variant  — optional 'positive' | 'negative'
 *
 * Field names match the live hub shape per diagnostics.js and telemetry-hub.js.
 */

export const sections = [
    {
        id: 'particle-state',
        title: 'Particle State',
        rows: [
            { id: 'manifested', label: 'Manifested', unit: '', source: 's0.diag.manifested', trend: 'sp.manifested' },
            { id: 'positive',   label: 'Positive',   unit: '', source: 's0.diag.positive',   variant: 'positive' },
            { id: 'negative',   label: 'Negative',   unit: '', source: 's0.diag.negative',   variant: 'negative' },
            { id: 'charge',     label: 'Charge (net)', unit: '', source: 's0.diag.chargeBalance', trend: 'sp.charges' },
            { id: 'spin',       label: 'Spin Up/Down', unit: '', format: 'pair',
              source: ['s0.diag.spinUp', 's0.diag.spinDown'] },
            { id: 'color',      label: 'Color R/G/B',  unit: '', format: 'triple',
              source: ['s0.diag.colorRed', 's0.diag.colorGreen', 's0.diag.colorBlue'] },
            { id: 'colorless',  label: 'Colorless',   unit: '', source: 's0.diag.colorless' },
        ],
    },
    {
        id: 'energy-budget',
        title: 'Energy Budget',
        rows: [
            { id: 'energy',       label: 'Total Energy',  unit: 'ftd', source: 's0.diag.totalEnergy', trend: 'sp.energy' },
            { id: 'field-energy', label: 'Field |J|\u00B2',  unit: 'ftd', source: 's0.audit.fieldEnergy' },
            { id: 'wave-energy',  label: 'Wave |w|\u00B2',   unit: 'ftd', source: 's0.audit.waveEnergy' },
            { id: 'particle-ke',  label: 'Particle KE',   unit: 'ftd', source: 's0.audit.particleKE' },
            { id: 'coulomb-pe',   label: 'Coulomb PE',    unit: 'ftd', source: 's0.audit.coulombPE' },
            { id: 'flux',         label: 'Total Flux',    unit: 'ftd', source: 's0.diag.totalFlux', trend: 'sp.flux' },
            { id: 'entropy',      label: 'Entropy',       unit: 'nat', source: 's0.diag.entropy',  trend: 'sp.entropy' },
        ],
    },
    {
        id: 'electromagnetic',
        title: 'Electromagnetic',
        rows: [
            { id: 'e-field',  label: 'E-Field |E|\u00B2/2', unit: 'ftd',
              compute: (hub) => hub.s0.audit?.EFieldEnergy ?? hub.s0.audit?.eFieldEnergy },
            { id: 'b-field',  label: 'B-Field |B|\u00B2/2', unit: 'ftd',
              compute: (hub) => hub.s0.audit?.BFieldEnergy ?? hub.s0.audit?.bFieldEnergy },
            { id: 'poynting', label: 'Poynting |S|',        unit: 'ftd',
              compute: (hub) => {
                  const a = hub.s0.audit; if (!a) return undefined;
                  const px = a.totalPoynting?.x ?? a.poyntingX ?? 0;
                  const py = a.totalPoynting?.y ?? a.poyntingY ?? 0;
                  const pz = a.totalPoynting?.z ?? a.poyntingZ ?? 0;
                  return Math.sqrt(px*px + py*py + pz*pz);
              } },
            { id: 'ang-mom', label: 'Angular Mom', unit: '', format: 'vector',
              source: ['s0.diag.angMomX', 's0.diag.angMomY', 's0.diag.angMomZ'] },
        ],
    },
    {
        id: 'constraints',
        title: 'Constraints',
        rows: [
            { id: 'gauss',     label: 'Gauss \u03A3(div J\u2212s)\u00B2', unit: 'ftd', source: 's0.audit.gaussViolation', trend: 'gauss' },
            { id: 'max-gauss', label: 'Max Gauss err',    unit: 'ftd', source: 's0.audit.maxGaussError' },
            { id: 'self-inj',  label: 'Self-field inj',   unit: 'ftd', source: 's0.audit.selfFieldInjection' },
        ],
    },
    {
        id: 'dual-substrate',
        title: 'Dual Substrate',
        rows: [
            { id: 'e-left',    label: 'E_L (left)',   unit: 'ftd',
              compute: (hub) => hub.s0.audit?.ELTotal ?? hub.s0.audit?.eLTotal },
            { id: 'e-right',   label: 'E_R (right)',  unit: 'ftd',
              compute: (hub) => hub.s0.audit?.ERTotal ?? hub.s0.audit?.eRTotal },
            { id: 'chirality', label: 'Chirality',    unit: '',    source: 's0.audit.chiralityTotal' },
            { id: 'wave-lr',   label: 'Wave L / R',   unit: '', format: 'pair',
              source: ['s0.audit.wvLTotal', 's0.audit.wvRTotal'] },
        ],
    },
];

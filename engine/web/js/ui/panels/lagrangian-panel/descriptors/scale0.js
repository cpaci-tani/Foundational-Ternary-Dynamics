/**
 * Scale 0 Lagrangian panel descriptor.
 * `terms` drives the per-term small-multiple charts + term-checkbox row (each
 * term renders as its own mini line-chart, stacked in the grid — not overlaid).
 * `actionRows` and `constantRows` drive the two sidecar tables.
 */

export const terms = [
    { key: 'fieldKinetic',  label: 'Field KE',    color: 'var(--legend-field-kinetic,  #66bb6a)', buffer: 'fieldKinetic',  includeByDefault: true },
    { key: 'fieldGradient', label: 'Gradient',    color: 'var(--legend-field-gradient, #26a69a)', buffer: 'fieldGradient', includeByDefault: true },
    { key: 'bornInfeld',    label: 'Born-Infeld', color: 'var(--legend-bi,             #ef5350)', buffer: 'bornInfeld',    includeByDefault: true },
    { key: 'coupling',      label: 'Coupling',    color: 'var(--legend-coupling,       #fb8c00)', buffer: 'coupling',      includeByDefault: true },
    { key: 'velocity',      label: 'Velocity',    color: 'var(--legend-velocity,       #fdd835)', buffer: 'velocity',      includeByDefault: true },
    { key: 'gauss',         label: 'Gauss',       color: 'var(--legend-gauss,          #42a5f5)', buffer: 'gauss',         includeByDefault: true },
    { key: 'dissipation',   label: 'Dissipation', color: 'var(--legend-dissipation,    #78909c)', buffer: 'dissipation',   includeByDefault: true },
];

export const actionRows = [
    { id: 'action',     label: 'Action S',              unit: 'ℏ',    source: 's0.lagrangian.totalAction', trend: 'lag.action' },
    { id: 'total',      label: 'ℒ total',               unit: 'E*',   source: 's0.lagrangian.total',       trend: 'lag.total' },
    { id: 'hamiltonian',label: 'Hamiltonian H',         unit: 'E*',   source: 's0.lagrangian.hamiltonian', trend: 'lag.hamiltonian' },
    { id: 'gauss',      label: 'Gauss ‖div J−s‖',       unit: 'E*²',  source: 's0.lagrangian.gauss',       trend: 'lag.gauss' },
    { id: 'max-gauss',  label: 'Max Gauss err',         unit: 'E*',   source: 's0.audit.maxGaussError',    trend: 'aud.maxGaussError' },
    { id: 'flux-mag',   label: 'Total |J|',             unit: '|J|',  source: 's0.diag.totalFlux',         trend: 'flux' },
    { id: 'wave-ke',    label: 'Wave KE',               unit: 'E*',   source: 's0.audit.waveEnergy',       trend: 'aud.waveEnergy' },
    { id: 'manifested', label: 'Manifested',            unit: 'ct',   source: 's0.diag.manifested',        trend: 'manifested' },
];

export const constantRows = [
    { id: 'gstar',     label: 'G*',      unit: '',     source: 'consts.G_STAR' },
    { id: 'alpha-inv', label: '1/α',     unit: '',     source: 'consts.X_PLUS' },
    { id: 'alpha',     label: 'α',       unit: '',     source: 'consts.ALPHA' },
    { id: 'kb',        label: 'K_B',     unit: 'MeV',  source: 'consts.K_B' },
    { id: 'gn',        label: 'G_N',     unit: '',     source: 'consts.G_N' },
    { id: 'gc',        label: 'g_c',     unit: '',     source: 'consts.G_C' },
    { id: 'nc',        label: 'N_c',     unit: 'ct',   source: 'consts.N_C' },
    { id: 'neff',      label: 'N_eff',   unit: 'ct',   source: 'consts.N_EFF' },
];

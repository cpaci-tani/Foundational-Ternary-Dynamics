/**
 * Scale 1 diagnostics table descriptor.
 * These rows summarize the active Native Matter observer, effective
 * ParticleEngine, or catalog mode through one shared snapshot.
 */

import { G_PE, GRAVITY_VIS_GAIN } from '../../../../constants.js';
import { scale1State } from '../../../../scales/scale1/state/store.js?v=7';
import { SCALE1_M3_VIEWS } from '../../../../scales/scale1/scenario-registry.js?v=15';

const SCALE1_ROW_HELP = Object.freeze({
    count: 'Number of particle records in the current coherent Scale-1 snapshot.',
    locked: 'Counts particles fixed by the scenario separately from particles advanced by the integrator.',
    'charge-comp': 'Counts positive, neutral, and negative effective-charge records in that order.',
    ke: 'Sum of translational kinetic energy over active particle records.',
    pe: 'Potential energy from active terms that publish an implemented potential channel.',
    'coulomb-pe': 'Potential-energy contribution from the active softened Coulomb pair kernel.',
    'gravity-pe': 'Potential-energy contribution from the active Newtonian pair-gravity kernel.',
    total: 'Current accounted state energy: kinetic plus implemented active potential channels.',
    'coverage-complete': 'Whether every active dynamics term has a represented state-energy or explicit sink channel.',
    'missing-mask': 'Bitmask of active energy channels that are not represented in the current state-energy ledger.',
    'drift-eligible': 'Whether conservative energy drift is meaningful for the current active kernel set.',
    drift: 'Relative state-energy change from the current scenario baseline, shown only when the ledger is complete and conservative.',
    'damping-sink': 'Cumulative energy removed by the imposed environment-damping term.',
    'speed-sink': 'Cumulative energy removed by enforcing the selected causal speed ceiling.',
    'contact-delta': 'Cumulative state-energy change recorded by selected contact-removal events.',
    unavailable: 'Claims or accounting channels currently unavailable for the active Scale-1 dynamics profile.',
    momentum: 'Vector sum of particle momenta in the native simulation frame.',
    'momentum-mag': 'Magnitude of the total native-frame momentum vector.',
    angmom: 'Vector sum of r cross p about the simulation origin.',
    'angmom-mag': 'Magnitude of total angular momentum about the simulation origin.',
    virial: 'Instantaneous virial proxy 2K divided by the absolute represented potential energy.',
    'net-charge': 'Algebraic sum of effective particle charges.',
    'max-beta': 'Largest particle speed divided by the selected engine light-speed ceiling.',
    'cap-count': 'Number of particles currently at the selected causal speed ceiling.',
    'max-force': 'Largest magnitude of the total active force on any particle.',
    'mean-force': 'Mean magnitude of the total active force over all particles.',
    vrms: 'Root-mean-square particle speed in units of the selected light-speed ceiling.',
    temperature: 'Kinetic temperature proxy computed from the current particle velocity distribution.',
    radius: 'Largest particle distance from the current center of mass.',
    separation: 'Distance between the first two particle records when a two-body readout is available.',
    'radial-velocity': 'Relative two-body velocity projected onto the instantaneous separation direction.',
    age: 'Number of global ticks represented by the observed record.',
    covered: 'Bitmask of state-energy channels represented by the current record.',
    missing: 'Bitmask of required state-energy channels absent from the current record.',
    nonconservative: 'Bitmask of active channels explicitly classified as non-conservative.',
    'm3-complete': 'Whether the registered M3 evidence record has complete state-energy coverage.',
    'm3-drift-eligible': 'Whether drift analysis is scientifically meaningful for the registered M3 evidence record.',
    'orbit-period': 'Measured interval between successive two-body orbital phase crossings; unavailable until enough history exists.',
    'potential-min': 'Minimum sampled potential in the currently rendered heatmap frame.',
    'potential-max': 'Maximum sampled potential in the currently rendered heatmap frame.',
});

function withRowTooltips(section) {
    return {
        ...section,
        rows: section.rows.map(row => ({
            ...row,
            tooltip: row.tooltip || SCALE1_ROW_HELP[row.id]
                || `${row.label} reported by the current ${section.title} snapshot channel.`,
        })),
    };
}

function formatCompact(value) {
    if (!Number.isFinite(value)) return '—';
    const magnitude = Math.abs(value);
    return magnitude !== 0 && (magnitude < 1e-4 || magnitude >= 1e5)
        ? value.toExponential(5) : value.toFixed(6).replace(/\.?0+$/, '');
}

function formatVector(vector) {
    if (!vector) return '—';
    return `(${formatCompact(vector.x)}, ${formatCompact(vector.y)}, ${formatCompact(vector.z)})`;
}

function m3ViewIs(id) {
    return scale1State.currentScenarioId === 's1-native-m3-replay'
        && scale1State.m3ViewId === id;
}

function m3Object(index) {
    return Array.from(scale1State.lastSnapshot?.objects || [])[index] || null;
}

function m3Field(channel) {
    return Array.from(scale1State.lastSnapshot?.fields || [])
        .find(row => row.channel === channel) || null;
}

function fieldSummary(channel) {
    const field = m3Field(channel);
    if (!field) return 'not recorded';
    if (!field.available) return `unavailable · ${field.unavailableReason || 'no certified producer'}`;
    const reason = field.unavailableReason ? ` · note: ${field.unavailableReason}` : '';
    return `available · E=${formatCompact(field.energy)} · ${field.producerId || 'producer unregistered'}${reason}`;
}

function maskSummary(value) {
    const mask = Number(value || 0) >>> 0;
    return `0x${mask.toString(16).padStart(8, '0')}`;
}

const sectionDefinitions = [
    {
        id: 'pe-m3-anatomy',
        title: 'M3 Evidence · Anatomy',
        variant: 'static',
        visibleWhen: () => m3ViewIs('anatomy'),
        rows: [
            { id: 'view', label: 'Observation View', unit: '', format: 'text',
              compute: () => SCALE1_M3_VIEWS.find(row => row.id === 'anatomy')?.cue },
            { id: 'object-count', label: 'Object Records', unit: 'ct',
              compute: () => Array.from(scale1State.lastSnapshot?.objects || []).length },
            { id: 'object-a', label: 'Record 0', unit: '', format: 'text', compute: () => {
                const row = m3Object(0);
                return row ? `state ${row.effectiveState > 0 ? '+' : ''}${row.effectiveState} · support ${row.manifestationSupportCount} · age ${row.ageTicks}` : '—';
            } },
            { id: 'object-b', label: 'Record 1', unit: '', format: 'text', compute: () => {
                const row = m3Object(1);
                return row ? `state ${row.effectiveState > 0 ? '+' : ''}${row.effectiveState} · support ${row.manifestationSupportCount} · age ${row.ageTicks}` : '—';
            } },
            { id: 'source-kind', label: 'Source Kind', unit: '', format: 'text',
              compute: () => m3Object(0)?.provenance?.sourceKind ?? '—' },
        ],
    },
    {
        id: 'pe-m3-graph',
        title: 'M3 Evidence · Constituent Graph',
        variant: 'static',
        visibleWhen: () => m3ViewIs('graph'),
        rows: [
            { id: 'view', label: 'Observation View', unit: '', format: 'text',
              compute: () => SCALE1_M3_VIEWS.find(row => row.id === 'graph')?.cue },
            { id: 'constituents', label: 'Constituent Records', unit: 'ct',
              compute: () => Array.from(scale1State.lastSnapshot?.objects || []).filter(row => row.constituent).length },
            { id: 'parents-a', label: 'Record 0 Links', unit: '', format: 'text',
              compute: () => Array.from(m3Object(0)?.parentIds || []).join(', ') || 'none recorded' },
            { id: 'parents-b', label: 'Record 1 Links', unit: '', format: 'text',
              compute: () => Array.from(m3Object(1)?.parentIds || []).join(', ') || 'none recorded' },
            { id: 'graph-margin', label: 'Graph Margin', unit: '',
              compute: () => m3Object(0)?.graphMargin ?? 0 },
            { id: 'lineage', label: 'Shared Lineage', unit: '', format: 'text',
              compute: () => m3Object(0)?.provenance?.sourceRevision ?? '—' },
        ],
    },
    {
        id: 'pe-m3-fields',
        title: 'M3 Evidence · Field Channels',
        variant: 'static',
        visibleWhen: () => m3ViewIs('fields'),
        rows: [
            { id: 'view', label: 'Observation View', unit: '', format: 'text',
              compute: () => SCALE1_M3_VIEWS.find(row => row.id === 'fields')?.cue },
            ...['actual', 'selected_bound', 'residual', 'outgoing', 'background'].map(channel => ({
                id: channel, label: channel.replaceAll('_', ' '), unit: '', format: 'text',
                compute: () => fieldSummary(channel),
            })),
        ],
    },
    {
        id: 'pe-m3-centers',
        title: 'M3 Evidence · Center Observers',
        variant: 'static',
        visibleWhen: () => m3ViewIs('centers'),
        rows: [
            { id: 'view', label: 'Observation View', unit: '', format: 'text',
              compute: () => SCALE1_M3_VIEWS.find(row => row.id === 'centers')?.cue },
            { id: 'a-integer', label: 'Record 0 Integer', unit: 'lu', format: 'text',
              compute: () => m3Object(0)?.integerCenterAvailable ? formatVector(m3Object(0).integerCenter) : 'unavailable' },
            { id: 'a-fractional', label: 'Record 0 Fractional', unit: 'lu', format: 'text',
              compute: () => m3Object(0)?.fractionalCenterAvailable ? formatVector(m3Object(0).fractionalCenter) : 'unavailable' },
            { id: 'b-integer', label: 'Record 1 Integer', unit: 'lu', format: 'text',
              compute: () => m3Object(1)?.integerCenterAvailable ? formatVector(m3Object(1).integerCenter) : 'unavailable' },
            { id: 'b-fractional', label: 'Record 1 Fractional', unit: 'lu', format: 'text',
              compute: () => m3Object(1)?.fractionalCenterAvailable ? formatVector(m3Object(1).fractionalCenter) : 'unavailable' },
        ],
    },
    {
        id: 'pe-m3-identity',
        title: 'M3 Evidence · Identity Margins',
        variant: 'static',
        visibleWhen: () => m3ViewIs('identity'),
        rows: [
            { id: 'view', label: 'Observation View', unit: '', format: 'text',
              compute: () => SCALE1_M3_VIEWS.find(row => row.id === 'identity')?.cue },
            { id: 'qualification', label: 'Qualification', unit: '', format: 'text',
              compute: () => m3Object(0)?.provenance?.qualification ?? '—' },
            { id: 'identity', label: 'Identity Margin', unit: '', compute: () => m3Object(0)?.identityMargin ?? 0 },
            { id: 'graph', label: 'Graph Margin', unit: '', compute: () => m3Object(0)?.graphMargin ?? 0 },
            { id: 'energy', label: 'Energy Margin', unit: '', compute: () => m3Object(0)?.energyMargin ?? 0 },
            { id: 'age', label: 'Finite Observation Age', unit: 'ticks', compute: () => m3Object(0)?.ageTicks ?? 0 },
        ],
    },
    {
        id: 'pe-m3-coverage',
        title: 'M3 Evidence · Coverage Ledger',
        variant: 'static',
        visibleWhen: () => m3ViewIs('coverage'),
        rows: [
            { id: 'view', label: 'Observation View', unit: '', format: 'text',
              compute: () => SCALE1_M3_VIEWS.find(row => row.id === 'coverage')?.cue },
            { id: 'covered', label: 'Covered Mask', unit: '', format: 'text',
              compute: () => maskSummary(scale1State.lastSnapshot?.conservation?.coveredMask) },
            { id: 'missing', label: 'Missing Mask', unit: '', format: 'text',
              compute: () => maskSummary(scale1State.lastSnapshot?.conservation?.missingMask) },
            { id: 'nonconservative', label: 'Non-conservative Mask', unit: '', format: 'text',
              compute: () => maskSummary(scale1State.lastSnapshot?.conservation?.nonconservativeMask) },
            { id: 'm3-complete', label: 'State Energy Complete', unit: '', format: 'boolean',
              compute: () => !!scale1State.lastSnapshot?.conservation?.stateEnergyComplete },
            { id: 'm3-drift-eligible', label: 'Drift Eligible', unit: '', format: 'boolean',
              compute: () => !!scale1State.lastSnapshot?.conservation?.driftEligible },
        ],
    },
    {
        id: 'pe-hamiltonian',
        title: 'State Energy & Coverage',
        rows: [
            { id: 'count', label: 'Particles', unit: 'ct',
              source: 's1.diag.particleCount', trend: 'peCount' },
            { id: 'locked', label: 'Locked / Mobile', unit: 'ct', format: 'pair',
              compute: (hub) => [hub.peLockedCount.last(), hub.peMobileCount.last()] },
            { id: 'charge-comp', label: 'Charge +/0/-', unit: '', format: 'triple',
              compute: (hub) => [hub.pePosCount.last(), hub.peZeroCount.last(), hub.peNegCount.last()] },
            { id: 'ke', label: 'Kinetic Energy', unit: 'MeV',
              source: 's1.diag.totalKE', trend: 'peKE' },
            { id: 'pe', label: 'Potential (active terms)', unit: 'MeV',
              source: 's1.diag.totalPE', trend: 'pePE' },
            { id: 'coulomb-pe', label: 'Coulomb PE', unit: 'MeV',
              source: 's1.diag.coulombPE', trend: 'peCoulombPE' },
            { id: 'gravity-pe', label: 'Gravity PE', unit: 'MeV',
              source: 's1.diag.gravityPE', trend: 'peGravityPE' },
            { id: 'total', label: 'State Energy', unit: 'MeV',
              source: 's1.diag.totalEnergy', trend: 'peTotal' },
            { id: 'coverage-complete', label: 'State Energy Complete', unit: '', format: 'boolean',
              source: 's1.diag.stateEnergyComplete' },
            { id: 'missing-mask', label: 'Missing Coverage Mask', unit: '',
              source: 's1.diag.missingMask' },
            { id: 'drift-eligible', label: 'Drift Eligible', unit: '', format: 'boolean',
              source: 's1.diag.driftEligible' },
            { id: 'drift', label: 'Conservative Drift', unit: '%',
              compute: (hub) => hub.peEnergyDrift.last(), trend: 'peEnergyDrift' },
            { id: 'damping-sink', label: 'Damping Sink', unit: 'MeV',
              source: 's1.diag.cumulativeDampingSink' },
            { id: 'speed-sink', label: 'Speed Projection Sink', unit: 'MeV',
              source: 's1.diag.cumulativeSpeedProjectionSink' },
            { id: 'contact-delta', label: 'Contact State Δ', unit: 'MeV',
              source: 's1.diag.cumulativeContactDelta' },
            { id: 'unavailable', label: 'Active Unavailable Claims', unit: '', format: 'text',
              compute: () => Array.from(scale1State.lastSnapshot?.unavailableReasons || []).join(' · ') || 'none' },
        ],
    },
    {
        id: 'pe-conservation',
        title: 'Conservation',
        rows: [
            { id: 'momentum', label: 'Momentum', unit: 'sim', format: 'vector',
              source: ['s1.diag.momentumX', 's1.diag.momentumY', 's1.diag.momentumZ'] },
            { id: 'momentum-mag', label: '|p|', unit: 'sim',
              compute: (hub) => hub.peMomentum.last(), trend: 'peMomentum' },
            { id: 'angmom', label: 'Angular Mom (origin)', unit: 'sim', format: 'vector',
              source: ['s1.diag.angMomX', 's1.diag.angMomY', 's1.diag.angMomZ'] },
            { id: 'angmom-mag', label: '|L| (origin)', unit: 'sim',
              compute: (hub) => hub.peAngMom.last(), trend: 'peAngMom' },
            { id: 'virial', label: 'Virial 2K/|U|', unit: '',
              compute: (hub) => hub.peVirial.last(), trend: 'peVirial' },
            { id: 'net-charge', label: 'Net Charge Q', unit: 'e',
              compute: (hub) => hub.peNetCharge.last(), trend: 'peNetCharge' },
        ],
    },
    {
        id: 'pe-field-forces',
        title: 'Forces & Geometry',
        rows: [
            { id: 'max-beta', label: 'Max |v|/c', unit: 'c',
              compute: (hub) => hub.peMaxBeta.last(), trend: 'peMaxBeta' },
            { id: 'cap-count', label: 'At Causal Cap', unit: 'ct',
              compute: (hub) => hub.peCapCount.last(), trend: 'peCapCount' },
            { id: 'max-force', label: 'Max Net Force', unit: 'sim',
              compute: (hub) => hub.peMaxForce.last(), trend: 'peMaxForce' },
            { id: 'mean-force', label: 'Mean Net Force', unit: 'sim',
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
    {
        id: 'pe-velocities',
        title: 'Velocities Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayVelocitiesOn,
        rows: [
            { id: 'vrms2', label: 'RMS Velocity', unit: 'c',
              compute: (hub) => hub.peRmsVelocity.last() },
            { id: 'max-beta2', label: 'Max |v|/c', unit: 'c',
              compute: (hub) => hub.peMaxBeta.last() },
            { id: 'cap-count2', label: 'At Causal Cap', unit: 'ct',
              compute: (hub) => hub.peCapCount.last() },
            { id: 'legend', label: 'Color legend', unit: '', format: 'text',
              compute: () => 'green→yellow <0.5c · yellow→orange 0.5-0.85c · orange→red 0.85-0.985c · red→white >0.985c (cap)' },
        ],
    },
    {
        id: 'pe-trails',
        title: 'Trails Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayTrailsOn,
        rows: [
            { id: 'orbit-period', label: 'Orbit period (2-body proxy)', unit: 'tick',
              format: 'text',
              compute: (hub) => hub.s1._orbitPeriod === null ? '—' : String(hub.s1._orbitPeriod) },
        ],
    },
    {
        id: 'pe-efield',
        title: 'E-Field Streamlines Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayEfieldOn,
        rows: [
            { id: 'coulomb-pe2', label: 'Coulomb PE', unit: 'MeV',
              compute: (hub) => hub.s1.diag?.coulombPE ?? 0 },
            { id: 'grid-res', label: 'Sample grid', unit: '', format: 'text',
              compute: () => '25×20 (XZ plane, fixed)' },
        ],
    },
    {
        id: 'pe-potential',
        title: 'Potential Heatmap Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayPotentialOn,
        rows: [
            { id: 'coulomb-pe3', label: 'Coulomb PE', unit: 'MeV',
              compute: (hub) => hub.s1.diag?.coulombPE ?? 0 },
            { id: 'potential-min', label: 'Min potential (this frame)', unit: '',
              compute: (hub) => hub.s1._potentialMin ?? 0 },
            { id: 'potential-max', label: 'Max potential (this frame)', unit: '',
              compute: (hub) => hub.s1._potentialMax ?? 0 },
        ],
    },
    {
        id: 'pe-gravity-field',
        title: 'Gravity Vectors Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayGravityFieldOn,
        rows: [
            { id: 'gravity-pe2', label: 'Gravity PE', unit: 'MeV',
              compute: (hub) => hub.s1.diag?.gravityPE ?? 0 },
            { id: 'g-pe2', label: 'G_PE constant', unit: 'MeV⁻²', compute: () => G_PE },
            { id: 'vis-gain', label: 'Visual gain applied', unit: '×',
              compute: () => GRAVITY_VIS_GAIN,
              format: 'text' },
        ],
    },
    {
        id: 'pe-force-decomp',
        title: 'Force Decomposition Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayForceOn,
        rows: [
            { id: 'max-force2', label: 'Max Net Force', unit: 'sim',
              compute: (hub) => hub.peMaxForce.last() },
            { id: 'mean-force2', label: 'Mean Net Force', unit: 'sim',
              compute: (hub) => hub.peMeanForce.last() },
        ],
    },
    {
        id: 'pe-system',
        title: 'System Overlay (CoM / p / L)',
        visibleWhen: (hub) => !!hub.s1?._overlaySystemOn,
        rows: [
            { id: 'l-com-note', label: 'L (native origin)', unit: 'sim',
              compute: (hub) => hub.s1._overlaySystemL ?? 0 },
            { id: 'l-origin-xref', label: 'cf. Conservation table', unit: 'sim',
              compute: (hub) => hub.peAngMom.last() },
        ],
    },
    {
        id: 'pe-provenance',
        title: 'Provenance Overlay',
        visibleWhen: (hub) => !!hub.s1?._overlayProvenanceOn,
        rows: [
            { id: 'source-kind', label: 'Source kind', unit: '', format: 'text',
              compute: () => scale1State.lastSnapshot?.objects?.[0]
                  ?.provenance?.sourceKind ?? '—' },
            { id: 'source-revision', label: 'Source revision', unit: '', format: 'text',
              compute: () => scale1State.lastSnapshot?.core?.sourceRevision ?? '—' },
        ],
    },
];

export const sections = sectionDefinitions.map(withRowTooltips);

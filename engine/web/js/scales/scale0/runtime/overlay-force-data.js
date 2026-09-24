/** Force sampling, derived proxy buffers, and force-style application. */
import { computeStreamlines, generateImportanceSeeds } from '../../../fieldlines.js';
import { DUAL_DELTA } from '../../../constants.js';
import { cachedSeeds } from './overlay-streamline-seeds.js';

export function buildForceOverlayData(state, fieldCapability, sampled, latticeSize, stride, stepsScale, seedSpacing, params = {}, forceCache = null) {
    const anyForceOn = state.fieldFlags.showForceEM || state.fieldFlags.showForceGravity ||
        state.fieldFlags.showForceStrong || state.fieldFlags.showForceWeak;
    if (!anyForceOn) return { anyForceOn: false, style: state.forceStyle, items: [] };

    // Force samplers need a finer stride than field samplers because particle-
    // anchored physics (Coulomb + flux tubes + nuclear) is sharply peaked at
    // voxel centres. With stride=2, the sampler hits only EVEN voxels, so a
    // Moore-cell scenario anchored at mc=16 (even) captures the centre but
    // SKIPS the neighbour particles at voxels 15 and 17 (odd). The resulting
    // arrow pattern looks off-centre because the tube envelopes between
    // adjacent particles — the most intense region — have zero samples. Drop
    // to stride=1 at small lattices so every voxel is caught regardless of
    // the particle-parity pattern; keep the field stride where it is so E/B
    // streamlines stay cheap.
    const forceStride = latticeSize <= 33 ? 1 : Math.max(1, Math.min(4, Math.floor(stride / 2) || 1));
    const getForce = forceCache
        ? (type) => forceCache.get(type, forceStride)
        : (type) => fieldCapability.getScale0ForceField(type, forceStride);

    const items = [];
    if (state.fieldFlags.showForceEM) {
        const emData = getForce('em');
        if (emData.count > 0) items.push({ type: 'em', data: emData });
    }
    if (state.fieldFlags.showForceGravity) {
        const gravityData = getForce('gravity');
        if (gravityData.count > 0) items.push({ type: 'gravity', data: gravityData });
    }
    if (state.fieldFlags.showForceStrong) {
        const strongData = getForce('strong');
        if (strongData.count > 0) items.push({ type: 'strong', data: strongData });
    }
    if (state.fieldFlags.showForceWeak && sampled.curlJ?.count > 0) {
        // Weak-force proxy = ∇×J, the curl of the (polar) flux vector J. The
        // curl of a polar vector is an axial (pseudo)vector, i.e. parity-EVEN
        // — NOT parity-odd. (An earlier comment justified this proxy by "weak is
        // parity-violating, so its proxy should be parity-odd"; that rationale is
        // backwards and has been removed. This overlay is a [PROXY —
        // VISUALIZATION ONLY] view of J's rotational structure, not a parity-odd
        // stand-in for the SM weak force.)
        //
        // Why the curl instead of J directly: J itself points uniformly along
        // the flux direction, so for any polarised scenario (e.g. a flux pulse
        // with J = (Gaussian, 0, 0)) every arrow pointed along +X — physically
        // uninformative. The curl is zero for irrotational (purely compressive)
        // flow and non-zero wherever J has rotational structure. Magnitude is
        // still scaled by DUAL_DELTA so the overlay reads as "weak" (small
        // relative to EM/strong) in comparison rendering.
        const curl = sampled.curlJ;
        const scalarFactor = DUAL_DELTA;
        if (!state.weakValues || state.weakValues.length < curl.count) {
            state.weakValues = new Float32Array(curl.count);
        }
        if (!state.weakVectors || state.weakVectors.length < curl.count * 3) {
            state.weakVectors = new Float32Array(curl.count * 3);
        }
        for (let i = 0; i < curl.count; i++) {
            const x = curl.vectors[i * 3];
            const y = curl.vectors[i * 3 + 1];
            const z = curl.vectors[i * 3 + 2];
            const mag = Math.sqrt(x * x + y * y + z * z);
            state.weakValues[i] = mag * scalarFactor;
            state.weakVectors[i * 3]     = x * scalarFactor;
            state.weakVectors[i * 3 + 1] = y * scalarFactor;
            state.weakVectors[i * 3 + 2] = z * scalarFactor;
        }
        items.push({
            type: 'weak',
            data: { positions: curl.positions, vectors: state.weakVectors, count: curl.count },
            weakScalar: { positions: curl.positions, values: state.weakValues, count: curl.count },
        });
    }

    // The flow-style streamline integration (one full computeStreamlines per
    // force, the heaviest part of this builder) is deferred to the caller when
    // `deferFlow` is set, so the overlay scheduler can spread it one force per
    // frame. Default-false ⇒ unchanged behaviour for any non-scheduled caller:
    // the whole flow loop runs inline and `item.flowLines` is populated here.
    if (state.forceStyle === 'flow' && !params.deferFlow) {
        for (const item of items) {
            item.flowLines = computeForceItemFlow(item, latticeSize, stride, params);
        }
    }

    return { anyForceOn, style: state.forceStyle, items };
}

/**
 * Compute the flow streamlines for ONE force item. Verbatim extraction of the
 * per-item body of buildForceOverlayData's flow loop, so the geometry is
 * identical whether run inline or as a scheduled per-force job.
 */
export function forceItemFlowPlan(item, latticeSize, stride, params = {}, state = null) {
    const { stepSize = 0.5, maxSteps = 100, maxSeeds = 150, maxLines = 200 } = params;
    // Force flow lines stay shorter than EM streamlines (≈ 40% of full length)
    // so the field-arrow visualization stays visually distinct from B/E lines.
    const flowMaxSteps = Math.max(20, Math.ceil(maxSteps * 0.4));
    // Weak is the flux-vector field itself (chirality transmutation follows
    // flux flow); give it denser coverage and longer lines so it reads as a
    // coherent field instead of a sparse cluster.
    const isWeak = item.type === 'weak';
    const seedCount = isWeak ? Math.min(maxSeeds * 2, 320) : maxSeeds;
    const stepCount = isWeak ? maxSteps : flowMaxSteps;
    const lineCount = isWeak ? Math.min(maxLines * 2, 400) : maxLines;
    // Importance-sample by |force| so streamlines cluster where the interaction
    // is strongest (e.g., near charges for EM, near masses for gravity),
    // matching the iron-filing visualization metaphor.
    const buildSeeds = () => generateImportanceSeeds(item.data, seedCount);
    const seeds = state
        ? cachedSeeds(state, `force-${item.type}`, latticeSize, buildSeeds)
        : buildSeeds();
    return { seeds, opts: {
        N: latticeSize, stride, maxSteps: stepCount, stepSize,
        maxLines: lineCount, bidirectional: true,
    } };
}

function computeForceItemFlow(item, latticeSize, stride, params = {}) {
    const plan = forceItemFlowPlan(item, latticeSize, stride, params);
    return computeStreamlines(item.data, plan.seeds, plan.opts);
}

// Force-flow type table, allocated ONCE at module load. Filtered into the
// persistent `sched.flowTypes` scratch (in place) per sweep — no new array.
export const FLOW_TYPES = [
    ['showForceEM', 'em'], ['showForceGravity', 'gravity'],
    ['showForceStrong', 'strong'], ['showForceWeak', 'weak'],
];

// Apply the force overlay's NON-flow styles (arrows / heatmap / glyphs). These
// are cheap (the data was already sampled by buildForceOverlayData) so they all
// run in the single force-fields job. Flow streamlines are NOT applied here —
// they are computed+applied per force in their own scheduled jobs (see below),
// because each is a full streamline integration.
export function applyForceFieldsJob(sched, viewportAdapter) {
    const forceFrame = sched.forceFrame;
    if (!forceFrame || !forceFrame.anyForceOn) return;
    if (forceFrame.style === 'arrows') {
        for (const item of forceFrame.items) {
            viewportAdapter.applyForceArrowField(item.type, item.type === 'weak' ? item.weakScalar : item.data);
        }
    } else if (forceFrame.style === 'heatmap') {
        for (const item of forceFrame.items) viewportAdapter.applyForceHeatmap(item.data, item.type);
    } else if (forceFrame.style === 'glyphs') {
        for (const item of forceFrame.items) viewportAdapter.applyForceGlyphs(item.data, item.type);
    }
    // A sampler may legitimately return no vectors (uniform density is the
    // canonical gravity example). Clear only that force's resident geometry;
    // otherwise a previous non-empty sweep remains visible as stale physics.
    if (forceFrame.style !== 'flow') {
        for (let i = 0; i < FLOW_TYPES.length; i++) {
            const [flag, type] = FLOW_TYPES[i];
            if (sched.state.fieldFlags[flag] && !findForceItem(forceFrame.items, type)) {
                viewportAdapter.clearForceVisualization(type, forceFrame.style);
            }
        }
    }
    // 'flow' falls through: handled by the per-force flow jobs.
}

// Linear lookup of a force item by type. Replaces `items.find((it) => ...)` in
// the flow job so the drain loop allocates no per-job arrow closure even when
// force-flow is active. Order/result are identical to Array.prototype.find.
export function findForceItem(items, type) {
    if (!items) return undefined;
    for (let i = 0; i < items.length; i++) {
        if (items[i].type === type) return items[i];
    }
    return undefined;
}

export function forceTypeEnabled(state, type) {
    if (type === 'em') return !!state.fieldFlags.showForceEM;
    if (type === 'gravity') return !!state.fieldFlags.showForceGravity;
    if (type === 'strong') return !!state.fieldFlags.showForceStrong;
    return type === 'weak' && !!state.fieldFlags.showForceWeak;
}

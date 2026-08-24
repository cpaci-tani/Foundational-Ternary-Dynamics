// Classic-worker sampler + toggle-requires registry. Loaded via importScripts
// from wasm-bridge.worker.js. Keep keys in lockstep with SCALE0_SAMPLER_METHODS
// in bridge-contract.js (audit-fix-contracts.spec.js asserts the match).
self.FTD_SAMPLER_METHODS = {
    e: ['getEFieldSampled', 'vec'],
    b: ['getBFieldSampled', 'vec'],
    poynting: ['getPoyntingSampled', 'vec'],
    divJ: ['getDivJSampled', 'val'],
    fluxVector: ['getFluxVectorSampled', 'vec'],
    vorticity: ['getVorticitySampled', 'val'],
    helicity: ['getHelicitySampled', 'val'],
    kretschmann: ['getKretschmannSampled', 'val'],
    latency: ['getLatencySampled', 'val'],
    poissonLatency: ['getPoissonLatencySampled', 'val'],
    fisher: ['getFisherSampled', 'val'],
    coherence: ['getCoherenceSampled', 'val'],
    curlJ: ['getCurlJSampled', 'vec'],
    state: ['getStateFieldSampled', 'val'],
    gaussResidual: ['getGaussResidualSampled', 'val'],
    em: ['getEMForceField', 'vec'],
    gravity: ['getGravityFieldSampled', 'vec'],
    strong: ['getStrongForceField', 'vec'],
    gravityMetricAgg: ['getGravityMetricAgg', 'obj'],
};

self.FTD_TOGGLE_REQUIRES = [
    ['selective_damping', 'damping'],
    ['larmor_radiation', 'damping'],
    ['lorentz_force', 'forces'],
    ['weak_transmutation', 'dual_substrate'],
    ['triad_binding', 'dual_substrate'],
    ['latency_field', 'gravity'],
];

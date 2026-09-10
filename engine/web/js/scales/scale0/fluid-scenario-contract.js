/** Prospective fluid recovery contract; not the public scenario catalog.
 * These declarations neither adopt a law nor activate a runtime. The ordinary
 * RenderBridge remains the sole intended state owner. Each scenario is withheld
 * until reviewed evidence and capabilities for one exact finite law are present.
 */

function freeze(value) {
    if (value && typeof value === 'object' && !Object.isFrozen(value)) {
        for (const child of Object.values(value)) freeze(child);
        Object.freeze(value);
    }
    return value;
}

const COMMON_GATES = Object.freeze([
    'finite-law-admission',
    'number-momentum-energy-accounting',
    'collision-invariant-census',
    'thermal-state-identification',
    'thermal-equilibrium-isotropy',
    'thermal-relaxation',
]);
const COMMON_OBSERVABLES = Object.freeze([
    'number-density', 'momentum-density', 'material-velocity', 'kinetic-energy-density',
]);

function scenario(id, title, gates, observables, boundaryDomain = 'closed-periodic') {
    return {
        id: `s0-fluid-${id}`, title,
        requiredPhysicalGates: [...COMMON_GATES, ...gates],
        boundaryDomain,
        requestedObservables: [...new Set([...COMMON_OBSERVABLES, ...observables])],
        defaultDisplay: { kind: 'density-volume', observable: 'number-density' },
    };
}

/** Frozen prospective declarations. No preparation numbers or material values
 * are supplied: those belong to the eventual accepted law and scenario protocol.
 * Closed-periodic cases have no wall or reservoir flux. The two boundary cases
 * require independently admitted wall/reservoir laws and exchange accounting.
 */
export const FLUID_SCENARIO_CONTRACT = freeze(Object.fromEntries([
    scenario('equilibrium', 'Fluid equilibrium',
        ['equilibrium-stationarity'], ['temperature', 'pressure', 'central-stress']),
    scenario('expansion', 'Expansion into a dilute region',
        ['vacuum-interface-transport', 'nonlinear-momentum-transport'],
        ['temperature', 'central-stress', 'conservation-residual']),
    scenario('counterflow', 'Counterflow relaxation',
        ['momentum-exchange-relaxation', 'nonlinear-momentum-transport'],
        ['temperature', 'central-stress', 'conservation-residual']),
    scenario('sound', 'Sound propagation',
        ['linear-acoustic-response', 'orientation-response'],
        ['pressure', 'acoustic-mode-amplitude']),
    scenario('thermal-contact', 'Thermal contact',
        ['heat-transport', 'thermal-balance'], ['temperature', 'heat-flux']),
    scenario('shear', 'Shear relaxation',
        ['constitutive-stress', 'viscous-transport', 'orientation-response', 'fluid-time-error-control'],
        ['central-stress', 'effective-stress', 'strain-rate', 'shear-mode-amplitude']),
    scenario('shock-tube', 'Shock tube',
        ['nonlinear-momentum-transport', 'compressible-shock-response', 'wall-exchange-accounting', 'fluid-time-error-control'],
        ['temperature', 'pressure', 'central-stress', 'boundary-exchange', 'conservation-residual'], 'closed-wall'),
    scenario('vortex-decay', 'Vortex decay',
        ['constitutive-stress', 'viscous-transport', 'nonlinear-momentum-transport', 'orientation-response', 'fluid-time-error-control'],
        ['central-stress', 'effective-stress', 'vorticity', 'conservation-residual']),
    scenario('obstacle-wake', 'Flow past an obstacle',
        ['constitutive-stress', 'viscous-transport', 'nonlinear-momentum-transport', 'wall-reservoir-balance', 'obstacle-boundary-response', 'wake-response', 'fluid-time-error-control'],
        ['pressure', 'effective-stress', 'vorticity', 'boundary-exchange', 'obstacle-force', 'conservation-residual'], 'wall-reservoir'),
].map(entry => [entry.id, entry])));

const record = value => value !== null && typeof value === 'object' && !Array.isArray(value);
const nonempty = value => typeof value === 'string' && value.trim().length > 0;
const hash = value => typeof value === 'string' && /^[a-f0-9]{64}$/.test(value);
function identity(value) {
    return record(value) && nonempty(value.lawId) && value.lawId === value.lawId.trim()
        && hash(value.tableHash) && hash(value.encodingHash);
}
function sameLaw(a, b) {
    return identity(a) && a.lawId === b.lawId && a.tableHash === b.tableHash && a.encodingHash === b.encodingHash;
}
function outcome(eligible, reason, missingGates = []) {
    return Object.freeze({ eligible, reason, missingGates: Object.freeze(missingGates) });
}

/** Pure metadata admission; performs no I/O and does not authenticate artifacts.
 * expectedLawIdentity: accepted {lawId,tableHash,encodingHash}; there is no default.
 * capability: {lawIdentity,model:'finite-fluid-law',stateOwner:'render-bridge',
 *              boundaryDomains:string[],observables:string[]}.
 * gateEvidence[gateId]: {lawIdentity,status:'passed',
 *                       evidence:[{artifactId:string,sha256:lowercaseHex64}]}.
 * Reviewed receipts must refer to this exact law. A software test result or
 * another candidate's transport coefficient cannot substitute for these gates.
 */
export function assessFluidScenarioEligibility(scenarioId, assessment = {}) {
    if (typeof scenarioId !== 'string' || !Object.hasOwn(FLUID_SCENARIO_CONTRACT, scenarioId)) {
        return outcome(false, 'Unknown fluid recovery scenario.');
    }
    if (!record(assessment)) return outcome(false, 'Native capability and reviewed gate evidence are required.');
    const { expectedLawIdentity, capability, gateEvidence } = assessment;
    if (!identity(expectedLawIdentity)) return outcome(false, 'An accepted exact law identity is required.');
    if (!record(capability) || !sameLaw(capability.lawIdentity, expectedLawIdentity)) {
        return outcome(false, 'Native capability is absent or belongs to a different law identity.');
    }
    if (capability.model !== 'finite-fluid-law' || capability.stateOwner !== 'render-bridge') {
        return outcome(false, 'A finite fluid law owned by the existing RenderBridge is required.');
    }
    const definition = FLUID_SCENARIO_CONTRACT[scenarioId];
    if (!Array.isArray(capability.boundaryDomains) || !capability.boundaryDomains.includes(definition.boundaryDomain)) {
        return outcome(false, `The ${definition.boundaryDomain} boundary domain is unavailable.`);
    }
    if (!Array.isArray(capability.observables)) return outcome(false, 'Native fluid observables are unavailable.');
    const missingObservables = definition.requestedObservables.filter(name => !capability.observables.includes(name));
    if (missingObservables.length) return outcome(false, `Missing native observables: ${missingObservables.join(', ')}.`);
    const missingGates = definition.requiredPhysicalGates.filter(name => {
        if (!record(gateEvidence) || !Object.hasOwn(gateEvidence, name)) return true;
        const gate = gateEvidence[name];
        return !record(gate) || gate.status !== 'passed' || !sameLaw(gate.lawIdentity, expectedLawIdentity)
            || !Array.isArray(gate.evidence) || gate.evidence.length === 0
            || !gate.evidence.every(receipt => record(receipt) && nonempty(receipt.artifactId) && hash(receipt.sha256));
    });
    if (missingGates.length) return outcome(false, `Unpassed, missing or mismatched physical gate evidence: ${missingGates.join(', ')}.`, missingGates);
    return outcome(true, 'All declared native capabilities and law-matched physical gate receipts are present.');
}

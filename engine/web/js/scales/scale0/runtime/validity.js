// Read-only checks of supplied cached scalars. No bridge calls or field scans.
const SCOPE = 'Checks available Scale 0 readings for numerical errors. Gravitational singularities are not evaluated.';
const COUNTERS = ['tick', 'manifested', 'positive', 'negative', 'spinUp', 'spinDown',
    'colorless', 'colorRed', 'colorGreen', 'colorBlue', 'causalProjectionEvents'];
const DIAGNOSTICS = ['physicalTime', 'dt', 'totalFlux', 'totalEnergy', 'dynamicEnergy',
    'maxBandwidth', 'avgDrag', 'entropy', 'chargeBalance', 'angMomX', 'angMomY', 'angMomZ',
    'maxCausalBudget', 'vacuumBaselineEnergy'];
const AUDIT = ['gaussViolation', 'maxGaussError', 'energyDrift', 'selfFieldInjection',
    'dynamicEnergy', 'totalEnergy', 'fieldEnergy', 'waveEnergy', 'particleKE',
    'particleRestEnergy', 'restEnergy', 'coulombPE', 'eFieldEnergy', 'bFieldEnergy'];

function status(label, reason) {
    return { label, severity: label === 'Unchecked' ? 'neutral' : label === 'Clear' ? 'ok' : 'error',
        details: `${reason} ${SCOPE}` };
}

function scalarIssue(name, value, counter = false) {
    if (value == null) return null; // Optional unavailable values are not faults.
    if (counter && typeof value === 'bigint') {
        return value < 0n ? status('Invalid', `${name} is negative: ${value}.`) : null;
    }
    if (typeof value !== 'number') return status('Invalid', `${name} has an unsupported scalar type (${typeof value}).`);
    if (Number.isNaN(value)) return status('Undefined', `${name} is NaN.`);
    if (!Number.isFinite(value)) return status('Overflow', `${name} is ${value}.`);
    if (counter && Number.isInteger(value) && !Number.isSafeInteger(value)) {
        return status('Overflow', `${name} exceeds JavaScript's exact integer range: ${value}.`);
    }
    if (counter && (!Number.isInteger(value) || value < 0)) return status('Invalid', `${name} is not a nonnegative integer: ${value}.`);
    return null;
}

export function classifyScale0RuntimeFailure(error) {
    const message = error instanceof Error ? error.message : String(error ?? 'Engine operation failed.');
    const label = /\boverflow\b|\bout of memory\b|\bInfinity\b/i.test(message) ? 'Overflow'
        : /\bNaN\b/.test(message) ? 'Undefined' : 'Runtime';
    return status(label, message);
}

function inspectScale0Validity({ diag, audit, diagMeta, diagCurrent = false, auditCurrent = false } = {}) {
    // The hub owns source/epoch invalidation. Never inspect a retained stale group.
    if (!diag || !diagMeta || diagMeta.stale === true
        || (diagMeta.status != null && diagMeta.status !== 'available')) {
        return status('Unchecked', 'Waiting for current readings.');
    }
    let checked = 0;
    let observations = 0;
    for (const [source, names, counters] of [[diag, COUNTERS, true], [diag, DIAGNOSTICS, false],
        [auditCurrent ? audit : null, AUDIT, false]]) {
        if (!source) continue;
        for (const name of names) {
            if (source[name] == null) continue;
            const issue = scalarIssue(name, source[name], counters);
            if (issue) return issue;
            checked++;
            if (name !== 'tick') observations++;
        }
    }
    for (const name of ['tick', 'epoch', 'sourceEpoch', 'stateVersion', 'snapshotVersion']) {
        const issue = scalarIssue(`diagnostics metadata ${name}`, diagMeta[name], true);
        if (issue) return issue;
    }
    if (!diagCurrent || observations === 0) return status('Unchecked', 'No current readings are available to check.');
    return status('Clear', `No numerical issue detected in the available readings (${checked} checked). `
        + (auditCurrent && audit ? 'Energy audit included.' : 'Energy audit unavailable for this tick.')
        + ' Unavailable readings are not checked.');
}

function contextDetails(context, tick, prefix = 'Sample tick') {
    return `${prefix}: ${tick ?? 'unavailable'}; scenario: ${context?.scenarioId ?? 'unavailable'}; `
        + `load: ${context?.authoritativeLoad?.loadGeneration ?? context?.anchor?.loadGeneration ?? 'unavailable'}; `
        + `change: ${context?.mutationEpoch ?? 'unavailable'}.`;
}

export function classifyScale0Validity(values = {}) {
    const result = inspectScale0Validity(values);
    return { ...result, details: `${result.details} ${contextDetails(values.context, values.diagMeta?.tick ?? values.diag?.tick)}` };
}

export function createScale0ValidityMonitor(indicator) {
    let mode = 'lattice';
    let qualification = null;
    let fault = null;
    let faultGeneration = null;
    let lastObservedTick = null;
    const generation = q => `${q?.authoritativeLoad?.scenarioId ?? q?.scenarioId ?? ''}:`
        + `${q?.authoritativeLoad?.loadGeneration ?? q?.anchor?.loadGeneration ?? 'unloaded'}`;
    const publish = value => { indicator?.set(value); return value; };
    const unchecked = reason => publish(status('Unchecked', reason));
    const outside = () => unchecked(`Scale 0 checks are inactive in ${mode} mode.`);
    const ready = () => qualification?.status === 'within-contract'
        && qualification.authoritativeLoad == null
        && qualification.anchor?.scenarioId === qualification.scenarioId;
    const api = {
        setMode(value) {
            mode = value;
            return mode === 'lattice' ? unchecked('Waiting for current Scale 0 readings.') : outside();
        },
        setQualification(value) {
            if (generation(value) !== generation(qualification)) lastObservedTick = null;
            qualification = value;
            if (ready() && generation(value) !== faultGeneration) fault = null;
            if (mode !== 'lattice') return outside();
            if (value?.authoritativeLoad?.status === 'failed') {
                return api.runtimeFailure(value.authoritativeLoad.failureReason || 'Scenario setup failed.');
            }
            if (ready() && fault) return publish(fault);
            return unchecked(ready() ? 'Waiting for readings from the completed Scale 0 load.'
                : 'Waiting for confirmed readings after the latest Scale 0 load or change.');
        },
        runtimeFailure(error) {
            fault = classifyScale0RuntimeFailure(error);
            fault.details += ` ${contextDetails(qualification, lastObservedTick, 'Last observed tick')}`;
            faultGeneration = generation(qualification);
            return mode === 'lattice' ? publish(fault) : outside();
        },
        sample(values) {
            if (mode !== 'lattice') return outside();
            if (fault && generation(qualification) === faultGeneration) return publish(fault);
            if (!ready()) return unchecked('Waiting for confirmed readings after the latest Scale 0 load or change.');
            const tick = values?.diagMeta?.tick;
            if (values?.diagCurrent && ((Number.isSafeInteger(tick) && tick >= 0)
                || (typeof tick === 'bigint' && tick >= 0n))) lastObservedTick = tick;
            return publish(classifyScale0Validity({ ...values, context: qualification }));
        },
    };
    unchecked('Waiting for current Scale 0 readings.');
    return api;
}

// Passive checks of already-published observations; no state evolution or grid scan.
const COVERAGE = 'Checks displayed projection and decay-fit arithmetic only. Full-grid validity and continuum recovery are not certified.';

export function hydroDecayFit(history, kSquared, enabled = true) {
    const windowed = history.slice(-32);
    if (!enabled || windowed.length < 2) return { gamma: null, diffusivity: null, issue: null };
    if (windowed.some((value) => !Number.isFinite(value) || value <= 0)) {
        return { gamma: null, diffusivity: null,
            issue: 'Decay fit is undefined: its last 32 samples include a missing, non-finite, zero, or negative amplitude. No logarithm clamp was applied.' };
    }
    if (!Number.isFinite(kSquared) || kSquared <= 0) {
        return { gamma: null, diffusivity: null, issue: 'Decay quotient is undefined: |k|² must be finite and positive.' };
    }
    const ys = windowed.map(Math.log);
    const n = ys.length;
    const xbar = (n - 1) / 2;
    const ybar = ys.reduce((sum, value) => sum + value, 0) / n;
    let numerator = 0, denominator = 0;
    for (let i = 0; i < n; i++) {
        numerator += (i - xbar) * (ys[i] - ybar);
        denominator += (i - xbar) ** 2;
    }
    const gamma = -numerator / denominator;
    const diffusivity = gamma / kSquared;
    if (!Number.isFinite(gamma) || !Number.isFinite(diffusivity)) {
        return { gamma: null, diffusivity: null, issue: 'Decay fit or γ/|k|² exceeded finite observation arithmetic.' };
    }
    return { gamma, diffusivity, issue: null };
}

export function hydroObservationValidity({ amplitude, history, kSquared, fitEnabled, microtick }) {
    const amplitudeValid = Number.isFinite(amplitude) && amplitude >= 0;
    const fit = hydroDecayFit(history, kSquared, fitEnabled);
    const issue = !amplitudeValid
        ? 'Projected amplitude is undefined or non-finite. This chart point was omitted; the finite-state runtime can continue.'
        : fit.issue;
    return {
        amplitudeValid, fit,
        status: {
            label: issue ? 'Undefined' : 'Clear', severity: issue ? 'warning' : 'ok',
            details: `Observed microtick: ${microtick}.\n${issue || 'No arithmetic issue found in the checked observations.'}\n${COVERAGE}`,
        },
    };
}

export function hydroFailureValidity(error, lastMicrotick = null) {
    const message = String(error?.message ?? error);
    return {
        label: /overflow|outside uint64|uint64.*range/i.test(message) ? 'Overflow' : 'Invalid',
        severity: 'error',
        details: `Last observed microtick: ${lastMicrotick ?? 'unavailable'}.\n${message}\nRuntime stopped; this reports a runtime or validation failure, not a physical singularity.`,
    };
}

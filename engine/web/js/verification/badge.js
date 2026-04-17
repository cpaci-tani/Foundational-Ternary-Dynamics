/**
 * Verification Lab — pass/fail badge logic.
 *
 * Every experiment's catalog entry carries a `tolerance` ({relative?, absolute?})
 * and an `epistemicTag`. `computeBadge` returns a status enum the UI maps to
 * color + glyph.
 *
 * - PASS  — measured value is within tolerance of theory
 * - CLOSE — within 2× tolerance (almost passing)
 * - FAIL  — beyond 2× tolerance
 * - NOT_RUN — no measurement yet
 * - EMERGENT — experiment is tagged EMERGENT, no hard pass/fail
 *
 * THEOREM / SELECTION tags apply the tolerance strictly.
 * EMERGENT tags report measurement only (always `EMERGENT` badge).
 * CONJECTURE tags apply tolerance softly (always `CLOSE` or `FAIL`).
 */

export const BADGE = Object.freeze({
    NOT_RUN:  'NOT_RUN',
    PASS:     'PASS',
    CLOSE:    'CLOSE',
    FAIL:     'FAIL',
    EMERGENT: 'EMERGENT',
});

export function computeBadge(measured, theory, tolerance, epistemicTag) {
    if (measured == null || !Number.isFinite(measured)) return BADGE.NOT_RUN;
    if (epistemicTag === 'EMERGENT') return BADGE.EMERGENT;

    const absTol = tolerance?.absolute;
    const relTol = tolerance?.relative;
    let err;
    if (absTol != null) {
        err = Math.abs(measured - theory);
        if (err <= absTol) return BADGE.PASS;
        if (err <= absTol * 2) return BADGE.CLOSE;
        return BADGE.FAIL;
    }
    if (relTol != null) {
        const denom = Math.max(Math.abs(theory), 1e-12);
        err = Math.abs(measured - theory) / denom;
        if (err <= relTol) return BADGE.PASS;
        if (err <= relTol * 2) return BADGE.CLOSE;
        return BADGE.FAIL;
    }
    // No tolerance specified — can't judge, display as emergent.
    return BADGE.EMERGENT;
}

export function badgeLabel(badge) {
    switch (badge) {
        case BADGE.PASS:     return '✓ PASS';
        case BADGE.CLOSE:    return '~ CLOSE';
        case BADGE.FAIL:     return '✗ FAIL';
        case BADGE.EMERGENT: return '● MEASURED';
        case BADGE.NOT_RUN:
        default:             return '— —';
    }
}

export function badgeClass(badge) {
    return `badge-${badge.toLowerCase().replace('_', '-')}`;
}

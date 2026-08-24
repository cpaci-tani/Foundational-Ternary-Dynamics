// engine/web/js/scales/scale0/runtime/knot-streamline-plan.js
//
// When to BUILD vs DRAW streamline jobs for field-line knot tracking.
// Tracking is observation-only and must not depend on the visual overlay flags.
// Drawing still follows those flags so the viewport stays empty when overlays
// are off.

export const KNOT_STREAMLINE_FIELDS = Object.freeze(['e', 'b', 'flux']);

export function overlayWorkActive(anyFieldActive, knotTracking) {
    return !!(anyFieldActive || knotTracking);
}

export function wantsStreamlineJob(flags, knotTracking, field) {
    if (field === 'e') return !!(flags?.showEField || knotTracking);
    if (field === 'b') return !!(flags?.showBField || knotTracking);
    if (field === 'flux') return !!(flags?.showFluxLines || knotTracking);
    return false;
}

export function wantsStreamlineApply(flags, field) {
    if (field === 'e') return !!flags?.showEField;
    if (field === 'b') return !!flags?.showBField;
    if (field === 'flux') return !!flags?.showFluxLines;
    return false;
}

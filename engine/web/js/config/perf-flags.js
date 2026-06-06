/**
 * Perf rollout flags — SPEC_SCALE0_PERF_TELEMETRY_PANELS.md §10.
 *
 * Each flag is a LIVE getter so the perf harness can A/B at runtime
 * (e.g. `window.__ftdTelemetryOnDemand = true`) with no rebuild, and so a
 * Playwright spec can flip it per-test. Defaults are OFF until each phase passes
 * its verification gate (§9/§11), then flipped on here. Flag OFF reproduces the
 * pre-2026-06-05 behavior exactly — the instant-rollback path.
 *
 * (Mirrors the `FTD_PHYSICS_WORKER` / `FTD_SPARSE_TICK` module-const convention,
 * promoted to a shared module because these flags are read on multiple paths.)
 */

function _flag(winKey, def) {
    if (typeof window !== 'undefined' && window[winKey] !== undefined) return !!window[winKey];
    return def;
}

export const PerfFlags = {
    /** Phase 1 — demand-gate the audit/Lagrangian telemetry streams on consumer
     *  visibility + field-version change (main thread + worker want-mask). */
    get telemetryOnDemand() { return _flag('__ftdTelemetryOnDemand', false); },

    /** Phase 2 — panel/overlay render optimizations (collapsed-grid gate,
     *  no-rescale chart draws, overlay sample amortization). */
    get panelRenderV2() { return _flag('__ftdPanelRenderV2', false); },
};

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
     *  visibility + field-version change (main thread + worker want-mask).
     *  DEFAULT ON (2026-06-05): verified by scale0-telemetry-gating.spec.js +
     *  the all-scenario panel-wiring gate. Set `window.__ftdTelemetryOnDemand =
     *  false` to restore the legacy always-collect behavior (instant rollback). */
    get telemetryOnDemand() { return _flag('__ftdTelemetryOnDemand', true); },

    /** Phase 2 — panel render optimizations: the unified isPanelLive predicate
     *  (active OR non-collapsed floated; fixes floated charts/Lagrangian
     *  freezing + adds the collapsed-grid gate), the telemetry-grid ~30 Hz cap,
     *  and its preallocated/cached per-channel buffers. DEFAULT ON (2026-06-05),
     *  verified by scale0-panel-render.spec.js. Set window.__ftdPanelRenderV2 =
     *  false to restore the legacy per-panel behavior (instant rollback). */
    get panelRenderV2() { return _flag('__ftdPanelRenderV2', true); },
};

/**
 * Shared panel-visibility predicate — SPEC_SCALE0_PERF_TELEMETRY_PANELS §6.4.
 *
 * A panel is "live" (worth updating/redrawing this frame) when it is the active
 * dock tab OR a floated window that is NOT collapsed. This unifies the three
 * different predicates the panels used to use:
 *   - charts/Lagrangian gated on `.active` only → froze when floated (a bug);
 *   - the telemetry grid gated on `active || .floating-window` → kept redrawing
 *     even while the floating window was collapsed (invisible work).
 *
 * `el` is the panel root (e.g. `#panel-telemetry-grid`). `.floating-window` is
 * the host set by the floating-window manager; `is-collapsed` is the CSS-only
 * collapse class it toggles (floating-window/component.js).
 */
export function isPanelLive(el) {
    if (!el) return false;
    if (el.classList.contains('active')) return true;
    const fw = el.closest('.floating-window');
    return !!fw && !fw.classList.contains('is-collapsed');
}

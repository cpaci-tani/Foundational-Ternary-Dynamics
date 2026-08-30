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
export const PANEL_VISIBILITY_CHANGE_EVENT = 'ftd:panel-visibility-change';

/**
 * Publish an explicit, synchronous visibility boundary after dock/floating DOM
 * state changes. Low-rate panel coordinators are still useful for recovery,
 * but they are too slow to revoke expensive measurement work on the exact
 * hide/collapse event.
 */
export function notifyPanelVisibilityChange(detail = {}) {
    if (typeof window === 'undefined' || typeof window.dispatchEvent !== 'function') return;
    window.dispatchEvent(new CustomEvent(PANEL_VISIBILITY_CHANGE_EVENT, {
        detail: Object.freeze({ ...detail }),
    }));
}

export function isPanelLive(el) {
    if (!el) return false;
    if (document.documentElement.classList.contains('ui-hidden')) return false;
    if (el.classList.contains('active')) {
        const app = el.closest('#app');
        return !app?.classList.contains('panels-collapsed');
    }
    const fw = el.closest('.floating-window');
    return !!fw && !fw.classList.contains('is-collapsed');
}

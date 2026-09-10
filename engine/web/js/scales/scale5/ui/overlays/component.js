/**
 * Scale 5 Viewport Overlay Component — interactive body (Pass B, Ruling P1).
 *
 * Scale 0 is the only interactive viewport-overlay precedent in the repo
 * (scales/scale0/ui/overlays/); scales 1-5 were static templates until this
 * pass. Rather than building Scale 0's much larger multi-panel machinery,
 * this mirrors the minimal shape scale5/ui/toolbar/component.js already
 * uses for the SAME lifetime hazard: `ViewportOverlaysComponent.init()`
 * (ui/components/viewport-overlays/component.js) appends the Scale-5
 * overlay template ONCE at app boot and never rebuilds it, while
 * `CosmicRenderer` is destroyed and recreated on every scenario load
 * (scale5/controller.js, loadCosmicScenario). A captured renderer reference
 * would go stale on the very first scenario switch, so `_activeRenderer`
 * plus `syncScale5Overlays(renderer)` — called by the controller right
 * after it builds a fresh renderer, and with `null` on destroy() — keeps
 * the overlay DOM and the live renderer decoupled.
 *
 * This module owns Pass B's content (colour-by selector, smoothing-length
 * circles) and Pass A's "Dynamics" section (velocity-vector overlay,
 * centre-of-mass marker) — both follow the SAME pending-state-plus-
 * active-renderer shape, since both faced the identical renderer-recreated-
 * on-scenario-load hazard. Passes C and D extend the same overlay body with
 * their own sections/controls; they may add to this file or add their own
 * sibling module bound from the same `bindScale5OverlayControls` call site.
 */

let _activeRenderer = null;
let _pendingColorBy = 'none';
let _pendingSmoothingCircles = false;
let _pendingVelocityVectors = false;
let _pendingComMarker = false;

/**
 * Bind the Gas-visualization and Dynamics overlay controls under `rootEl`
 * (the Scale-5 overlay panel element `ViewportOverlaysComponent.init()` just
 * appended). Called exactly ONCE at app boot from that component, mirroring
 * the toolbar's one-time `_bindToggleCheckboxes` call.
 *
 * @param {Element|null} rootEl
 */
export function bindScale5OverlayControls(rootEl) {
    const colorBySelect = rootEl?.querySelector('#cosmic-overlay-colorby');
    const circlesInput = rootEl?.querySelector('#cosmic-overlay-smoothing-circles');
    const velocityInput = rootEl?.querySelector('#cosmic-overlay-velocity-vectors');
    const comInput = rootEl?.querySelector('#cosmic-overlay-com-marker');

    colorBySelect?.addEventListener('change', () => {
        _pendingColorBy = colorBySelect.value;
        _activeRenderer?.setColorBy?.(_pendingColorBy);
    });

    circlesInput?.addEventListener('change', () => {
        _pendingSmoothingCircles = circlesInput.checked;
        _activeRenderer?.setSmoothingCircles?.(_pendingSmoothingCircles);
    });

    velocityInput?.addEventListener('change', () => {
        _pendingVelocityVectors = velocityInput.checked;
        _activeRenderer?.setVelocityVectors?.(_pendingVelocityVectors);
    });

    comInput?.addEventListener('change', () => {
        _pendingComMarker = comInput.checked;
        _activeRenderer?.setComMarker?.(_pendingComMarker);
    });
}

/**
 * Point the overlay controls at `renderer` (or `null` to stop driving a
 * disposed one) and immediately apply the last-chosen colour-by mode,
 * smoothing-circle, velocity-vector and centre-of-mass-marker state to it —
 * so a choice made before a scenario reload survives the reload, matching
 * syncScale5Toggles(bridge)'s shape exactly.
 *
 * @param {{setColorBy?: Function, setSmoothingCircles?: Function, setVelocityVectors?: Function, setComMarker?: Function}|null} renderer
 */
export function syncScale5Overlays(renderer) {
    _activeRenderer = renderer || null;
    _activeRenderer?.setColorBy?.(_pendingColorBy);
    _activeRenderer?.setSmoothingCircles?.(_pendingSmoothingCircles);
    _activeRenderer?.setVelocityVectors?.(_pendingVelocityVectors);
    _activeRenderer?.setComMarker?.(_pendingComMarker);
}

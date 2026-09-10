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
 * circles), Pass A's "Dynamics" section (velocity-vector overlay,
 * centre-of-mass marker), and Pass C's "Cosmology" section (comoving
 * reference-grid toggle) — all follow the SAME pending-state-plus-
 * active-renderer shape, since all three faced the identical
 * renderer-recreated-on-scenario-load hazard. Pass D extends the same
 * overlay body with its own section/controls; it may add to this file or
 * add its own sibling module bound from the same
 * `bindScale5OverlayControls` call site.
 *
 * Pass D adds: the five dead renderer-visibility toggles (plan step 0.7),
 * the "Type" colour-by option (routed through the SAME colorBySelect
 * listener above — no new binding needed), black-hole location + Bondi
 * accretion-radius markers, body trails, and the body-size-scale slider.
 * All follow the identical pending-state-plus-active-renderer shape as
 * everything else in this file, for the identical reason.
 */

let _activeRenderer = null;
let _pendingColorBy = 'none';
let _pendingSmoothingCircles = false;
let _pendingVelocityVectors = false;
let _pendingComMarker = false;
let _pendingComovingGrid = false;
let _pendingShowDM = true;
let _pendingShowGas = true;
let _pendingShowStars = true;
let _pendingShowBH = true;
let _pendingShowDisks = true;
let _pendingBhMarkers = false;
let _pendingAccretionMarkers = false;
let _pendingTrails = false;
let _pendingBodySizeScale = 1;

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
    const comovingGridInput = rootEl?.querySelector('#cosmic-overlay-comoving-grid');
    // Pass D
    const showDmInput = rootEl?.querySelector('#cosmic-overlay-show-dm');
    const showGasInput = rootEl?.querySelector('#cosmic-overlay-show-gas');
    const showStarsInput = rootEl?.querySelector('#cosmic-overlay-show-stars');
    const showBhInput = rootEl?.querySelector('#cosmic-overlay-show-bh');
    const showDisksInput = rootEl?.querySelector('#cosmic-overlay-show-disks');
    const bhMarkersInput = rootEl?.querySelector('#cosmic-overlay-bh-markers');
    const accretionMarkersInput = rootEl?.querySelector('#cosmic-overlay-accretion-markers');
    const trailsInput = rootEl?.querySelector('#cosmic-overlay-trails');
    const bodySizeInput = rootEl?.querySelector('#cosmic-overlay-body-size');
    const bodySizeValue = rootEl?.querySelector('#cosmic-overlay-body-size-value');

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

    comovingGridInput?.addEventListener('change', () => {
        _pendingComovingGrid = comovingGridInput.checked;
        _activeRenderer?.setComovingGrid?.(_pendingComovingGrid);
    });

    // Pass D: five dead renderer toggles (plan step 0.7).
    showDmInput?.addEventListener('change', () => {
        _pendingShowDM = showDmInput.checked;
        _activeRenderer?.toggleDarkMatter?.(_pendingShowDM);
    });
    showGasInput?.addEventListener('change', () => {
        _pendingShowGas = showGasInput.checked;
        _activeRenderer?.toggleGasClouds?.(_pendingShowGas);
    });
    showStarsInput?.addEventListener('change', () => {
        _pendingShowStars = showStarsInput.checked;
        _activeRenderer?.toggleStars?.(_pendingShowStars);
    });
    showBhInput?.addEventListener('change', () => {
        _pendingShowBH = showBhInput.checked;
        _activeRenderer?.toggleBlackHoles?.(_pendingShowBH);
    });
    showDisksInput?.addEventListener('change', () => {
        _pendingShowDisks = showDisksInput.checked;
        _activeRenderer?.toggleAccretionDisks?.(_pendingShowDisks);
    });

    // Pass D: black-hole location + Bondi accretion-radius markers.
    bhMarkersInput?.addEventListener('change', () => {
        _pendingBhMarkers = bhMarkersInput.checked;
        _activeRenderer?.setBhMarkers?.(_pendingBhMarkers);
    });
    accretionMarkersInput?.addEventListener('change', () => {
        _pendingAccretionMarkers = accretionMarkersInput.checked;
        _activeRenderer?.setAccretionMarkers?.(_pendingAccretionMarkers);
    });

    // Pass D: body trails + presentation-only body-size scale.
    trailsInput?.addEventListener('change', () => {
        _pendingTrails = trailsInput.checked;
        _activeRenderer?.setTrails?.(_pendingTrails);
    });
    bodySizeInput?.addEventListener('input', () => {
        _pendingBodySizeScale = Number(bodySizeInput.value);
        _activeRenderer?.setBodySizeScale?.(_pendingBodySizeScale);
        if (bodySizeValue) bodySizeValue.textContent = _pendingBodySizeScale.toFixed(2);
    });
}

/**
 * Point the overlay controls at `renderer` (or `null` to stop driving a
 * disposed one) and immediately apply the last-chosen colour-by mode,
 * smoothing-circle, velocity-vector and centre-of-mass-marker state to it —
 * so a choice made before a scenario reload survives the reload, matching
 * syncScale5Toggles(bridge)'s shape exactly.
 *
 * @param {{setColorBy?: Function, setSmoothingCircles?: Function, setVelocityVectors?: Function, setComMarker?: Function, setComovingGrid?: Function, toggleDarkMatter?: Function, toggleGasClouds?: Function, toggleStars?: Function, toggleBlackHoles?: Function, toggleAccretionDisks?: Function, setBhMarkers?: Function, setAccretionMarkers?: Function, setTrails?: Function, setBodySizeScale?: Function}|null} renderer
 */
export function syncScale5Overlays(renderer) {
    _activeRenderer = renderer || null;
    _activeRenderer?.setColorBy?.(_pendingColorBy);
    _activeRenderer?.setSmoothingCircles?.(_pendingSmoothingCircles);
    _activeRenderer?.setVelocityVectors?.(_pendingVelocityVectors);
    _activeRenderer?.setComMarker?.(_pendingComMarker);
    _activeRenderer?.setComovingGrid?.(_pendingComovingGrid);
    // Pass D
    _activeRenderer?.toggleDarkMatter?.(_pendingShowDM);
    _activeRenderer?.toggleGasClouds?.(_pendingShowGas);
    _activeRenderer?.toggleStars?.(_pendingShowStars);
    _activeRenderer?.toggleBlackHoles?.(_pendingShowBH);
    _activeRenderer?.toggleAccretionDisks?.(_pendingShowDisks);
    _activeRenderer?.setBhMarkers?.(_pendingBhMarkers);
    _activeRenderer?.setAccretionMarkers?.(_pendingAccretionMarkers);
    _activeRenderer?.setTrails?.(_pendingTrails);
    _activeRenderer?.setBodySizeScale?.(_pendingBodySizeScale);
}

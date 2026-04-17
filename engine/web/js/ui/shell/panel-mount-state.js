/**
 * Single source of truth for panel-mount state.
 * The attribute html[data-panel-mount] is the canonical value at runtime;
 * localStorage key ftd.panel.mount persists it across reloads.
 */

const VALID_MOUNTS = Object.freeze(['left', 'bottom', 'right']);
const STORAGE_KEY = 'ftd.panel.mount';
const DEFAULT_MOUNT = 'right'; // 'bottom' reserved for mobile sheet only

export function isValidMount(value) {
    return typeof value === 'string' && VALID_MOUNTS.includes(value);
}

export function readPanelMount() {
    try {
        const stored = localStorage.getItem(STORAGE_KEY);
        return isValidMount(stored) ? stored : DEFAULT_MOUNT;
    } catch (_err) {
        return DEFAULT_MOUNT;
    }
}

export function writePanelMount(value) {
    const next = isValidMount(value) ? value : DEFAULT_MOUNT;
    document.documentElement.dataset.panelMount = next;
    try {
        localStorage.setItem(STORAGE_KEY, next);
    } catch (_err) {
        // localStorage unavailable — attribute still authoritative for this session.
    }
    return next;
}

export function getValidMounts() {
    return VALID_MOUNTS.slice();
}

export function getDefaultMount() {
    return DEFAULT_MOUNT;
}

/** Minimum viewport width (px) required to honour a side mount (≥ tablet). */
export function getSideMountMinWidth() {
    return 1024;
}

/**
 * Returns the mount that should actually be applied given the current
 * viewport width.  If the stored preference is left/right but the viewport
 * is too narrow, returns 'bottom' without mutating localStorage.
 */
export function resolveEffectiveMount(storedMount, viewportWidth) {
    const side = storedMount === 'left' || storedMount === 'right';
    if (side && viewportWidth < getSideMountMinWidth()) return 'bottom'; // mobile sheet
    return isValidMount(storedMount) ? storedMount : DEFAULT_MOUNT;
}

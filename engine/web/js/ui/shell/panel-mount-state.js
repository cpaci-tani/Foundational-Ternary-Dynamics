/**
 * Single source of truth for panel-mount state.
 * The attribute html[data-panel-mount] is the canonical value at runtime;
 * localStorage key ftd.panel.mount persists it across reloads.
 */

const VALID_MOUNTS = Object.freeze(['left', 'bottom', 'right']);
const STORAGE_KEY = 'ftd.panel.mount';
const VERSION_KEY = 'ftd.panel.mount.version';
const DEFAULT_MOUNT = 'left';
// Bump this to force every existing user back to DEFAULT_MOUNT once. The old
// default was 'bottom'; v2 moves the default to the left dock. The migration
// clears the stored mount so DEFAULT_MOUNT applies, then records the version so
// it only happens once — the user can still re-toggle afterward (which writes a
// fresh preference and is preserved across future loads at the same version).
const MOUNT_PREF_VERSION = '2';

export function isValidMount(value) {
    return typeof value === 'string' && VALID_MOUNTS.includes(value);
}

/**
 * One-time preference reset. Idempotent: only acts when the stored version is
 * behind MOUNT_PREF_VERSION. Mirrored by the pre-paint inline script in
 * index.html so first paint already reflects the new default (no flash).
 */
export function migratePanelMount() {
    try {
        if (localStorage.getItem(VERSION_KEY) !== MOUNT_PREF_VERSION) {
            localStorage.removeItem(STORAGE_KEY);     // fall back to DEFAULT_MOUNT
            localStorage.setItem(VERSION_KEY, MOUNT_PREF_VERSION);
        }
    } catch (_err) {
        // localStorage unavailable — DEFAULT_MOUNT applies anyway.
    }
}

export function readPanelMount() {
    migratePanelMount();
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

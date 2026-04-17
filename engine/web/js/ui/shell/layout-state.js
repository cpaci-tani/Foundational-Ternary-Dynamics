/**
 * UI shell layout state helpers.
 *
 * Phase 0 keeps the existing DOM intact, but centralizes breakpoint,
 * orientation, and panel-collapse state so later component extraction
 * has a stable shell boundary to build on.
 */

export const LAYOUT_MODE_BREAKPOINTS = Object.freeze([
    { name: 'compact-sm', maxWidth: 479 },
    { name: 'compact-lg', maxWidth: 767 },
    { name: 'tablet', maxWidth: 1023 },
    { name: 'desktop', maxWidth: 1439 },
    { name: 'wide', maxWidth: Number.POSITIVE_INFINITY },
]);

export function resolveLayoutMode(width) {
    const numericWidth = Number.isFinite(width) ? width : window.innerWidth;
    return LAYOUT_MODE_BREAKPOINTS.find((entry) => numericWidth <= entry.maxWidth)?.name || 'desktop';
}

export function resolveOrientation(width, height) {
    const w = Number.isFinite(width) ? width : window.innerWidth;
    const h = Number.isFinite(height) ? height : window.innerHeight;
    return w >= h ? 'landscape' : 'portrait';
}

export function isCompactLayout(layoutMode) {
    return layoutMode === 'compact-sm' || layoutMode === 'compact-lg';
}

export function isTabletLayout(layoutMode) {
    return layoutMode === 'tablet';
}

export function readStoredBoolean(key, fallback = false) {
    try {
        const raw = window.localStorage?.getItem(key);
        if (raw === null) return fallback;
        return raw === 'true';
    } catch {
        return fallback;
    }
}

export function writeStoredBoolean(key, value) {
    try {
        window.localStorage?.setItem(key, value ? 'true' : 'false');
    } catch {
        // Ignore storage failures; shell state still lives in memory.
    }
}

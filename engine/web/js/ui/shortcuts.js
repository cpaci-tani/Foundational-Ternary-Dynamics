/** Shared keyboard metadata and editable-target guard. */
const workspaceShortcuts = new Map();
let activeShortcutWorkspace = 'dashboard';

/** Register a live shortcut description without installing another key handler. */
export function registerWorkspaceShortcuts(id, readRows) {
    workspaceShortcuts.set(id, readRows);
    return () => { if (workspaceShortcuts.get(id) === readRows) workspaceShortcuts.delete(id); };
}

export function getWorkspaceShortcuts(id = activeShortcutWorkspace) {
    return workspaceShortcuts.get(id)?.() || [];
}

export function setShortcutWorkspace(id) { activeShortcutWorkspace = id; }

export const DASHBOARD_SHORTCUTS = Object.freeze([
    { group: 'Playback', rows: Object.freeze([
        { keys: ['Space'], label: 'Play / Pause' },
        { keys: ['S'], label: 'Step one tick' },
        { keys: ['R'], label: 'Reset scenario' },
    ]) },
    { group: 'Scale 0 overlays', rows: Object.freeze([
        { keys: ['1'], label: 'Toggle E field' }, { keys: ['2'], label: 'Toggle B field' },
        { keys: ['3'], label: 'Toggle Poynting vector' }, { keys: ['4'], label: 'Toggle ∇·J (divergence)' },
        { keys: ['5'], label: 'Toggle flux streamlines' }, { keys: ['6'], label: 'Toggle EM force' },
        { keys: ['7'], label: 'Toggle gravity force' }, { keys: ['8'], label: 'Toggle strong force' },
        { keys: ['9'], label: 'Toggle weak force' },
    ]) },
    { group: 'Help', rows: Object.freeze([
        { keys: ['?'], label: 'Show this keyboard shortcuts list' },
        { keys: ['Esc'], label: 'Close this overlay (or the settings popover)' },
    ]) },
]);

export function isEditableTarget(target) {
    if (!(target instanceof Element)) return false;
    return !!target.closest('input, textarea, select, [contenteditable]:not([contenteditable="false"])');
}

/**
 * Playback shortcuts must not steal activation keys from a control, or cross
 * an active modal boundary. The shared guard keeps keyboard-help and the app
 * transport handler consistent.
 */
export function isShortcutBlockedTarget(target) {
    if (activeShortcutWorkspace !== 'dashboard') return true;
    if (isEditableTarget(target)) return true;
    if (target instanceof Element
        && target.closest('button, a[href], [role="button"], [role="menuitem"], [role="tab"], [role="switch"]')) return true;
    return [...document.querySelectorAll('[aria-modal="true"]')].some(modal => {
        if (modal.hidden) return false;
        const style = getComputedStyle(modal);
        return style.display !== 'none' && style.visibility !== 'hidden' && modal.getClientRects().length > 0;
    });
}

/**
 * app-wire/keyboard.js — keyboard-shortcut handler for the FTD dashboard.
 *
 * Extracted from `app.js::wireKeyboard()` as refactoring-analyst ticket
 * RF-9 (partial). Companion extracts: `status.js`, `bridge-boot.js`.
 * Toolbar, preferences, particle/atom, and viewport bindings have independent
 * lifecycle owners beside this module. Playback actions are shared with buttons.
 *
 * Shortcut contract (matches pre-refactor behavior 1:1):
 *   Space            → play/pause
 *   S                → single-step
 *   R                → reload scenario
 *   1–9 (Scale 0)    → field-visualization toggles (delegated to Scale0Controller)
 *
 * Typing inside any editable control is ignored so the shortcuts don't
 * interfere with text entry or scenario dropdowns.
 *
 * @param {{
 *   getEngineMode: () => string,
 *   getBridge: () => object,
 *   pauseSimulation: () => void,
 *   togglePlay: () => void,
 *   stepScenario: () => void,       // handles Scale0Controller.step / peTick / aeTick / etc.
 *   reloadScenario: () => void,     // handles loadAEScenario / loadPEScenario / Scale0Controller.reset
 *   Scale0Controller: object,
 * }} deps
 */
import { isShortcutBlockedTarget } from '../ui/shortcuts.js';

export function wireKeyboard(deps) {
    const {
        getEngineMode,
        pauseSimulation,
        togglePlay,
        stepScenario,
        reloadScenario,
        Scale0Controller,
    } = deps;

    const onKeyDown = (e) => {
        // Preserve editor/browser chords and never route keys from text entry.
        if (e.defaultPrevented || e.ctrlKey || e.metaKey || e.altKey || isShortcutBlockedTarget(e.target)) return;

        switch (e.key.toLowerCase()) {
            case ' ':
                e.preventDefault();
                togglePlay();
                break;
            case 's':
                pauseSimulation();
                stepScenario();
                break;
            case 'r':
                pauseSimulation();
                reloadScenario();
                break;
        }

        // Field-visualization shortcuts (1-9) — Scale 0 only
        if (getEngineMode() === 'lattice') {
            if (Scale0Controller.handleShortcutKey(e.key)) e.preventDefault();
        }
    };
    document.addEventListener('keydown', onKeyDown);
    return () => document.removeEventListener('keydown', onKeyDown);
}

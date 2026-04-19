/**
 * app-wire/keyboard.js — keyboard-shortcut handler for the FTD dashboard.
 *
 * Extracted from `app_dag.js::wireKeyboard()` as refactoring-analyst ticket
 * RF-9 (partial). The rest of the wire* functions (wireToolbar,
 * wireControls, wireViewportToggles) are too deeply entangled with
 * app_dag module-scope state to extract cleanly without a larger
 * state-plumbing pass; deferred until the scale-controller interface
 * unification lands.
 *
 * Shortcut contract (matches pre-refactor behavior 1:1):
 *   Space            → global play/pause
 *   Shift+Space      → scenario play/pause
 *   S                → single-step
 *   R                → reload scenario
 *   1–8 (Scale 0)    → field-visualization toggles (delegated to Scale0Controller)
 *
 * Typing inside <input> or <select> is ignored so the shortcuts don't
 * interfere with text entry or scenario dropdowns.
 *
 * @param {{
 *   getEngineMode: () => string,
 *   getBridge: () => object,
 *   setRunning: (v: boolean) => void,
 *   updatePlayButton: () => void,
 *   togglePlay: () => void,
 *   toggleScenarioPlay: () => void,
 *   stepScenario: () => void,       // handles Scale0Controller.step / peTick / aeTick / etc.
 *   reloadScenario: () => void,     // handles loadAEScenario / loadPEScenario / Scale0Controller.reset
 *   Scale0Controller: object,
 * }} deps
 */
export function wireKeyboard(deps) {
    const {
        getEngineMode,
        setRunning,
        updatePlayButton,
        togglePlay,
        toggleScenarioPlay,
        stepScenario,
        reloadScenario,
        Scale0Controller,
    } = deps;

    document.addEventListener('keydown', (e) => {
        // Ignore if typing in an input or select
        if (e.target.tagName === 'INPUT' || e.target.tagName === 'SELECT') return;

        switch (e.key.toLowerCase()) {
            case ' ':
                e.preventDefault();
                // Shift+Space toggles scenario pause; plain Space toggles global.
                if (e.shiftKey) toggleScenarioPlay();
                else togglePlay();
                break;
            case 's':
                setRunning(false);
                updatePlayButton();
                stepScenario();
                break;
            case 'r':
                setRunning(false);
                updatePlayButton();
                reloadScenario();
                break;
        }

        // Field-visualization shortcuts (1-8) — Scale 0 only
        if (getEngineMode() === 'lattice') {
            if (Scale0Controller.handleShortcutKey(e.key)) e.preventDefault();
        }
    });
}

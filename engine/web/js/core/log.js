/**
 * Lightweight debug logger for the web dashboard.
 *
 * Logging is OFF by default. Enable it with one of:
 *   - `?ftd_debug=1` in the page URL
 *   - `localStorage.setItem('ftd:debug', '1')`
 *   - `window.__FTD_DEBUG__ = true` before module load
 */

const DEBUG_ENABLED = (() => {
    try {
        if (typeof window === 'undefined') return false;

        const urlFlag = new URL(window.location.href).searchParams.get('ftd_debug');
        if (urlFlag !== null) {
            return !['0', 'false', 'off'].includes(urlFlag.toLowerCase());
        }

        const storedFlag = window.localStorage?.getItem('ftd:debug');
        if (storedFlag !== null) {
            return !['0', 'false', 'off'].includes(storedFlag.toLowerCase());
        }

        return window.__FTD_DEBUG__ === true;
    } catch {
        return false;
    }
})();

export function debugLog(...args) {
    if (DEBUG_ENABLED) {
        console.log(...args);
    }
}

export function isDebugLoggingEnabled() {
    return DEBUG_ENABLED;
}

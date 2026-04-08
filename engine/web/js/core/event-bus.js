/**
 * Event Bus — Lightweight pub/sub for decoupled module communication.
 *
 * Used to decouple scale controllers from app.js and viewport.js.
 * Instead of direct function calls across module boundaries, modules
 * emit events and subscribe to events they care about.
 *
 * Example:
 *   bus.on('scale-changed', (level) => renderer.switchScale(level));
 *   bus.emit('scale-changed', 5);
 *
 * Human coders: use named event constants (not magic strings) when adding new events.
 */

class EventBus {
    constructor() {
        this._listeners = new Map();
    }

    on(event, fn) {
        if (!this._listeners.has(event)) {
            this._listeners.set(event, new Set());
        }
        this._listeners.get(event).add(fn);
        return () => this.off(event, fn); // Return unsubscribe function
    }

    off(event, fn) {
        const set = this._listeners.get(event);
        if (set) set.delete(fn);
    }

    emit(event, data) {
        const set = this._listeners.get(event);
        if (set) {
            for (const fn of set) {
                try { fn(data); }
                catch (e) { console.error(`[EventBus] Error in handler for '${event}':`, e); }
            }
        }
    }

    clear() {
        this._listeners.clear();
    }
}

// Singleton instance shared across all modules
export const bus = new EventBus();

// Named event constants — use these instead of magic strings
export const EVENTS = {
    SCALE_CHANGED: 'scale-changed',
    SCENARIO_LOADED: 'scenario-loaded',
    TOGGLE_CHANGED: 'toggle-changed',
    TICK: 'tick',
    DIAGNOSTICS_UPDATED: 'diagnostics-updated',
};

/**
 * @file app-store.js
 * @brief Reactive App State Store for FTD web dashboard.
 *
 * Implements a simple EventTarget-based store to keep global variables
 * reactive and decouple DOM mediators and scale controllers from app.js.
 */

import { appRegistry } from './registry.js';

class AppStore extends EventTarget {
    constructor() {
        super();
        this._state = {
            running: false,
            ticksPerFrame: 0.25,
            activeScale: '0', // '0' through '6', '11'
            activeTab: 'observables' // active tab in sidebar panel
        };
    }

    /**
     * Set a state property and dispatch a change event.
     * @param {string} key
     * @param {any} value
     */
    set(key, value) {
        if (this._state[key] === value) return;
        this._state[key] = value;
        
        // Dispatch specific event (e.g. change:running)
        this.dispatchEvent(new CustomEvent(`change:${key}`, { detail: value }));
        
        // Dispatch general event
        this.dispatchEvent(new CustomEvent('change', { detail: { key, value } }));
    }

    /**
     * Retrieve a state property.
     * @param {string} key
     * @returns {any}
     */
    get(key) {
        return this._state[key];
    }

    /**
     * Batch update multiple state keys.
     * @param {Record<string, any>} updates
     */
    update(updates) {
        for (const [key, value] of Object.entries(updates)) {
            this.set(key, value);
        }
    }
}

export const appStore = new AppStore();
appRegistry.register('store', appStore);

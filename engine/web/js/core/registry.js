/**
 * @file registry.js
 * @brief Service Registry for the FTD web dashboard.
 *
 * Implements a simple Service Registry pattern to decouple modular references
 * and eliminate mutable window globals, while maintaining Playwright and
 * debugging accessibility via a read-only namespace.
 */

class ServiceRegistry {
    constructor() {
        this._services = new Map();
    }

    /**
     * Register a service instance.
     * @param {string} token
     * @param {any} instance
     */
    register(token, instance) {
        this._services.set(token, instance);
    }

    /**
     * Retrieve a service instance.
     * @param {string} token
     * @returns {any}
     */
    get(token) {
        return this._services.get(token) || null;
    }

    /**
     * Unregister a service instance.
     * @param {string} token
     */
    unregister(token) {
        this._services.delete(token);
    }

    /**
     * Clear all services (useful for clean test isolation).
     */
    clear() {
        this._services.clear();
    }
}

export const appRegistry = new ServiceRegistry();

// Safe, read-only developer & Playwright automation hook
if (typeof window !== 'undefined') {
    Object.defineProperty(window, '__FTD_DEV__', {
        value: Object.freeze({
            get registry() { return appRegistry; },
            get store() { return appRegistry.get('store'); },
            get bridge() { return appRegistry.get('activeBridge'); },
            get viewport() { return appRegistry.get('viewport'); },
            get scale0Ctx() { return appRegistry.get('scale0Ctx'); },
            get panels() {
                return {
                    conservation: appRegistry.get('panel:conservation'),
                    fluxSlice: appRegistry.get('panel:fluxSlice'),
                    spectrum: appRegistry.get('panel:spectrum'),
                    gravity: appRegistry.get('panel:gravity'),
                    time: appRegistry.get('panel:time'),
                };
            }
        }),
        configurable: false,
        writable: false
    });
}

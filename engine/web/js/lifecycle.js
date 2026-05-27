/**
 * Unified Lifecycle Controller for FTD Web Frontend
 * ────────────────────────────────────────────────────────────────────
 *
 * Base class providing robust, automated resource reclamation.
 * Tracks bound event listeners, intervals/timeouts, and Three.js / WebGL
 * objects, ensuring they are recursively traversed and disposed of on destroy.
 */

export class BaseLifecycleController {
    constructor() {
        this._listeners = [];
        this._timers = [];
        this._threeObjects = [];
    }

    /**
     * Bind an event listener to a target and track it for automatic unbinding on destroy.
     * @param {EventTarget|null} target - Element/window/document to bind to (no-op if null)
     * @param {string} type - Event type (e.g., 'click', 'change')
     * @param {Function} listener - Callback function
     * @param {any} [options] - Optional event listener options
     */
    bindEvent(target, type, listener, options) {
        if (!target) return;
        target.addEventListener(type, listener, options);
        this._listeners.push({ target, type, listener, options });
    }

    /**
     * Start a interval loop and track it for automatic clearance on destroy.
     * @param {Function} callback
     * @param {number} delay
     * @returns {number}
     */
    setInterval(callback, delay) {
        const id = setInterval(callback, delay);
        this._timers.push({ id, type: 'interval' });
        return id;
    }

    /**
     * Start a timeout and track it for automatic clearance on destroy.
     * @param {Function} callback
     * @param {number} delay
     * @returns {number}
     */
    setTimeout(callback, delay) {
        const id = setTimeout(callback, delay);
        this._timers.push({ id, type: 'timeout' });
        return id;
    }

    /**
     * Track a Three.js / WebGL object (e.g. Mesh, Group, Material, Geometry) for automatic disposal.
     * @param {any} obj - Object to track
     * @returns {any} Returns the object for chaining
     */
    trackThreeObject(obj) {
        if (obj) this._threeObjects.push(obj);
        return obj;
    }

    /**
     * mount is designed to be overridden by subclasses.
     * @param {any} ctx - AppContext
     */
    mount(ctx) {
        // Subclasses implement setup here
    }

    /**
     * update is designed to be overridden by subclasses.
     * @param {number} dt
     * @param {any} ctx - AppContext
     */
    update(dt, ctx) {
        // Subclasses implement per-frame update here
    }

    /**
     * Unbinds all listeners, clears all timers, and disposes of all tracked Three.js/WebGL objects.
     * Can be overridden by subclasses but they MUST call super.destroy(ctx) or dispose().
     * @param {any} ctx - AppContext
     */
    destroy(ctx) {
        // 1. Remove all event listeners automatically
        for (const { target, type, listener, options } of this._listeners) {
            try {
                target.removeEventListener(type, listener, options);
            } catch (e) {
                console.warn(`[Lifecycle] Error removing listener for event "${type}":`, e);
            }
        }
        this._listeners = [];

        // 2. Clear all timers automatically
        for (const { id, type } of this._timers) {
            if (type === 'interval') clearInterval(id);
            if (type === 'timeout') clearTimeout(id);
        }
        this._timers = [];

        // 3. Automatically traverse and dispose of Three.js objects
        for (const obj of this._threeObjects) {
            if (!obj) continue;
            
            // If the object has a traverse method (like Group, Mesh, Scene)
            if (typeof obj.traverse === 'function') {
                obj.traverse(child => {
                    this._disposeSingleThreeResource(child);
                });
            }
            
            // Dispose of the object itself if it has a dispose method
            this._disposeSingleThreeResource(obj);

            // Remove from parent if applicable
            if (obj.parent && typeof obj.parent.remove === 'function') {
                try {
                    obj.parent.remove(obj);
                } catch (e) {
                    // Ignore hierarchy removal errors if parent is already gone
                }
            }
        }
        this._threeObjects = [];
    }

    /**
     * Safely disposes of a single Three.js geometry, material, texture or other disposable resource.
     * @private
     */
    _disposeSingleThreeResource(resource) {
        if (!resource) return;

        // Dispose of geometry
        if (resource.geometry && typeof resource.geometry.dispose === 'function') {
            try {
                resource.geometry.dispose();
            } catch (e) {
                console.warn('[Lifecycle] Error disposing geometry:', e);
            }
        }

        // Dispose of material
        if (resource.material) {
            const materials = Array.isArray(resource.material) ? resource.material : [resource.material];
            for (const mat of materials) {
                if (mat && typeof mat.dispose === 'function') {
                    try {
                        // Dispose textures inside uniforms/maps of this material before disposing the material
                        this._disposeMaterialTextures(mat);
                        mat.dispose();
                    } catch (e) {
                        console.warn('[Lifecycle] Error disposing material:', e);
                    }
                }
            }
        }

        // Dispose of textures directly tracked
        if (typeof resource.dispose === 'function' && resource.geometry === undefined && resource.material === undefined) {
            try {
                resource.dispose();
            } catch (e) {
                // Ignore silent errors for non-disposables
            }
        }
    }

    /**
     * Dispose of textures referenced by a material to prevent GPU memory leaks.
     * @private
     */
    _disposeMaterialTextures(material) {
        // Common map/texture properties in Three.js materials
        const textureKeys = [
            'map', 'lightMap', 'bumpMap', 'normalMap', 'specularMap',
            'displacementMap', 'roughnessMap', 'metalnessMap', 'alphaMap', 'envMap'
        ];

        for (const key of textureKeys) {
            if (material[key] && typeof material[key].dispose === 'function') {
                try {
                    material[key].dispose();
                } catch (e) {
                    console.warn(`[Lifecycle] Error disposing texture ${key}:`, e);
                }
            }
        }

        // Check inside custom shaders uniforms
        if (material.uniforms) {
            for (const uniformKey in material.uniforms) {
                const uniform = material.uniforms[uniformKey];
                if (uniform && uniform.value && typeof uniform.value.dispose === 'function') {
                    try {
                        uniform.value.dispose();
                    } catch (e) {
                        console.warn(`[Lifecycle] Error disposing uniform texture ${uniformKey}:`, e);
                    }
                }
            }
        }
    }
}

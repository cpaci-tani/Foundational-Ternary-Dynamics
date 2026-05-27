/**
 * FTD Environment Backgrounds — registry + BackgroundManager.
 *
 * Pseudo-environmental effects rendered behind the simulation lattice.
 * Each procedural theme lives in its own module under ./backgrounds/
 * and exports a Theme descriptor { label, build, animate }. This file
 * wires them together and exposes the BackgroundManager lifecycle
 * (set, update, dispose) consumed by app.js.
 *
 * Refactored 2026-04-18 (BG-1, BG-2): theme implementations and the
 * HDRI loader extracted to engine/web/js/backgrounds/*.js.
 */
import * as THREE from 'three';

import { StarfieldTheme }  from './backgrounds/starfield.js';
import { NebulaTheme }     from './backgrounds/nebula.js';
import { FoamTheme }       from './backgrounds/foam.js';
import { BeyondTheme }     from './backgrounds/beyond.js';
import { FluxStormTheme }  from './backgrounds/flux-storm.js';
import {
    HDRI_ENVIRONMENTS,
    createHDRILoader,
    loadHDRI,
    applyHDRITexture,
} from './backgrounds/hdri-loader.js';


// ── Background Registry ──────────────────────────────────────────────
const BACKGROUNDS = {
    // Cosmic (procedural particle effects)
    none:   { label: 'None',        build: null,                animate: null },
    stars:  { label: StarfieldTheme.label,  build: StarfieldTheme.build,  animate: StarfieldTheme.animate },
    nebula: { label: NebulaTheme.label,     build: NebulaTheme.build,     animate: NebulaTheme.animate },
    foam:   { label: FoamTheme.label,       build: FoamTheme.build,       animate: FoamTheme.animate },
    beyond: { label: BeyondTheme.label,     build: BeyondTheme.build,     animate: BeyondTheme.animate },
    storm:  { label: FluxStormTheme.label,  build: FluxStormTheme.build,  animate: FluxStormTheme.animate },
};

// Add HDRI environments to registry (marker: hdri = true)
for (const [key, env] of Object.entries(HDRI_ENVIRONMENTS)) {
    BACKGROUNDS[key] = { label: env.label, hdri: env.file, build: null, animate: null };
}


// ── BackgroundManager ────────────────────────────────────────────────
export class BackgroundManager {
    constructor(scene) {
        this._scene = scene;
        this._current = null;       // name string
        this._group = null;         // THREE.Group (for particle backgrounds)
        this._animateFn = null;     // per-frame callback
        this._time = 0;
        this._defaultBg = new THREE.Color(0x0f1729);
        this._hdriCache = {};       // cache loaded HDRI textures by key
        this._hdriTexture = null;   // currently active HDRI texture
        this._loader = createHDRILoader();
        this._pmremGenerator = null;
    }

    /** Available background names for populating UI */
    static get options() {
        return Object.entries(BACKGROUNDS).map(([key, val]) => ({
            value: key,
            label: val.label
        }));
    }

    /** Set the active background by name */
    set(name, renderer) {
        // tear down previous particle background
        if (this._group) {
            this._scene.remove(this._group);
            this._disposeGroup(this._group);
            this._group = null;
            this._animateFn = null;
        }
        // clear previous HDRI state (textures stay in cache)
        this._hdriTexture = null;
        this._scene.environment = null;
        this._scene.fog = null;
        this._scene.backgroundIntensity = 1.0;
        this._scene.backgroundBlurriness = 0.0;

        this._current = name;
        const entry = BACKGROUNDS[name];
        if (!entry) {
            this._scene.background = this._defaultBg;
            return;
        }

        // ── HDRI environment ──
        if (entry.hdri) {
            this._scene.background = new THREE.Color(0x111111); // temp while loading
            this._loadHDRI(name, entry.hdri, renderer);
            return;
        }

        // ── 'none' ──
        if (!entry.build) {
            this._scene.background = this._defaultBg;
            return;
        }

        // ── Particle background ──
        this._scene.background = new THREE.Color(0x060a14);
        this._group = entry.build();
        this._animateFn = entry.animate;
        this._scene.add(this._group);
    }

    /** Load HDRI from cache or Poly Haven CDN */
    _loadHDRI(name, file, renderer) {
        // Use cached texture if available
        if (this._hdriCache[name]) {
            this._applyHDRI(this._hdriCache[name], renderer);
            return;
        }

        loadHDRI(
            this._loader,
            file,
            (texture) => {
                // Only apply if still the current selection (user may have switched)
                this._hdriCache[name] = texture;
                if (this._current === name) {
                    this._applyHDRI(texture, renderer);
                }
            },
            (err, url) => {
                console.warn(`Failed to load HDRI "${name}" from ${url}:`, err);
                // Fall back to default dark background
                if (this._current === name) {
                    this._scene.background = this._defaultBg;
                }
            }
        );
    }

    /** Apply loaded HDRI texture as scene background + environment */
    _applyHDRI(texture, _renderer) {
        this._hdriTexture = texture;
        applyHDRITexture(this._scene, texture);
    }

    /** Call each frame from the render loop */
    update(dt) {
        if (!this._animateFn || !this._group) return;
        this._time += dt;
        this._animateFn(this._group, this._time);
    }

    /** Current background name */
    get current() { return this._current; }

    /** Clean up everything */
    dispose() {
        if (this._group) {
            this._scene.remove(this._group);
            this._disposeGroup(this._group);
        }
        // Dispose cached HDRI textures
        for (const tex of Object.values(this._hdriCache)) {
            tex.dispose();
        }
        this._hdriCache = {};
        this._scene.environment = null;
    }

    _disposeGroup(obj) {
        obj.traverse(child => {
            if (child.geometry) child.geometry.dispose();
            if (child.material) {
                if (Array.isArray(child.material)) child.material.forEach(m => m.dispose());
                else child.material.dispose();
            }
        });
    }
}

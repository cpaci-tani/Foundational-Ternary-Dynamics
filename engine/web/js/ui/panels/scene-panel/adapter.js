/**
 * SceneAdapter — one place to translate "the user moved a slider" into
 * "change this Three.js object on the Viewport". Consumers of the
 * ScenePanel import the adapter, not Three.js directly, so the panel
 * component stays a pure UI module.
 *
 * The adapter reads from the live Viewport on every setter so it always
 * writes to the currently-active objects even if a scale switch replaces
 * them. Lazy-creates a DirectionalLight the first time the key-light
 * intensity moves above zero.
 */

import * as THREE from 'three';

const DEFAULTS = Object.freeze({
    fov: 45,
    orbitRotateSpeed: 0.6,
    orbitZoomSpeed: 1.2,
    ambientIntensity: 0.5,
    ambientColor: '#404060',
    keyLightIntensity: 0,
    exposure: 1.0,
    bloomEnabled: false,
    bloomStrength: 1.5,
    bloomRadius: 0.4,
    bloomThreshold: 0.2,
    fogEnabled: false,
    fogDensity: 0.02,
    backgroundColor: '#0f1729',
    hdriIntensity: 0.8,
});

const KEY_LIGHT_POSITION = new THREE.Vector3(5, 5, 5);

function findAmbientLight(scene) {
    let found = null;
    scene.traverse((obj) => {
        if (!found && obj.isAmbientLight) found = obj;
    });
    return found;
}

function findKeyLight(scene) {
    return scene.getObjectByName('ftd-scene-key-light') || null;
}

export class SceneAdapter {
    constructor({ viewport, backgroundManager, backgroundSelectEl = null } = {}) {
        if (!viewport) throw new Error('SceneAdapter: viewport is required');
        this.viewport = viewport;
        this.backgroundManager = backgroundManager || null;
        // Optional reference to the existing #bg-select dropdown so we can
        // gate hdri-intensity and background-color controls on its value.
        this.backgroundSelectEl = backgroundSelectEl
            || (typeof document !== 'undefined' ? document.getElementById('bg-select') : null);
    }

    static get DEFAULTS() { return DEFAULTS; }

    // ── Camera ──────────────────────────────────────────────────────

    setFov(value) {
        const cam = this.viewport.camera;
        if (!cam) return;
        cam.fov = value;
        cam.updateProjectionMatrix();
    }

    setOrbitRotateSpeed(value) {
        if (this.viewport.controls) this.viewport.controls.rotateSpeed = value;
    }

    setOrbitZoomSpeed(value) {
        if (this.viewport.controls) this.viewport.controls.zoomSpeed = value;
    }

    // ── Lighting ────────────────────────────────────────────────────

    setAmbientIntensity(value) {
        const ambient = findAmbientLight(this.viewport.scene);
        if (ambient) ambient.intensity = value;
    }

    setAmbientColor(hexString) {
        const ambient = findAmbientLight(this.viewport.scene);
        if (ambient) ambient.color.set(hexString);
    }

    /**
     * Lazy key-light semantics: intensity 0 means no DirectionalLight in
     * the scene; first non-zero intensity creates and adds one at
     * KEY_LIGHT_POSITION. Setting back to 0 removes it.
     */
    setKeyLightIntensity(value) {
        const scene = this.viewport.scene;
        let light = findKeyLight(scene);
        if (value <= 0) {
            if (light) {
                scene.remove(light);
                if (light.dispose) light.dispose();
            }
            return;
        }
        if (!light) {
            light = new THREE.DirectionalLight(0xffffff, value);
            light.name = 'ftd-scene-key-light';
            light.position.copy(KEY_LIGHT_POSITION);
            scene.add(light);
        } else {
            light.intensity = value;
        }
    }

    // ── Post-processing ─────────────────────────────────────────────

    setExposure(value) {
        if (this.viewport.renderer) this.viewport.renderer.toneMappingExposure = value;
    }

    setBloomEnabled(on) {
        if (on) {
            this.viewport.enablePostProcessing?.();
        } else {
            this.viewport.disablePostProcessing?.();
        }
    }

    setBloomStrength(value) {
        this.viewport.setBloomParams?.({ strength: value });
    }

    setBloomRadius(value) {
        this.viewport.setBloomParams?.({ radius: value });
    }

    setBloomThreshold(value) {
        this.viewport.setBloomParams?.({ threshold: value });
    }

    // ── Environment ─────────────────────────────────────────────────

    /**
     * Enable or disable exponential fog. When enabling, use the current
     * scene-background color as the fog color so objects fade into the
     * backdrop rather than a mismatched hue.
     */
    setFogEnabled(on, density = DEFAULTS.fogDensity) {
        const scene = this.viewport.scene;
        if (!on) {
            scene.fog = null;
            return;
        }
        const bgColor = this._currentBackgroundColor();
        scene.fog = new THREE.FogExp2(bgColor, density);
    }

    setFogDensity(density) {
        const scene = this.viewport.scene;
        if (scene.fog && typeof scene.fog.density === 'number') {
            scene.fog.density = density;
        }
    }

    /**
     * Apply a solid background color. Only meaningful when the user has
     * the 'none' background option selected in #bg-select; otherwise the
     * BackgroundManager owns the scene.background. The panel greys out
     * this control in that case but it is safe to call regardless.
     */
    setBackgroundColor(hexString) {
        const scene = this.viewport.scene;
        if (!scene) return;
        if (scene.background && scene.background.isColor) {
            scene.background.set(hexString);
        } else {
            scene.background = new THREE.Color(hexString);
        }
        // Keep fog in sync with the backdrop so objects blend to the new color.
        if (scene.fog && scene.fog.color) scene.fog.color.set(hexString);
    }

    /** HDRI backgrounds: adjust both backgroundIntensity and environmentIntensity. */
    setHdriIntensity(value) {
        const scene = this.viewport.scene;
        if (!scene) return;
        scene.backgroundIntensity = value;
        // environmentIntensity is respected by PBR materials reading the env map.
        if ('environmentIntensity' in scene) scene.environmentIntensity = value;
    }

    // ── Conditional-control predicates ──────────────────────────────

    isHdriActive() {
        const sel = this.backgroundSelectEl;
        if (!sel || typeof sel.value !== 'string') return false;
        return sel.value.startsWith('hdri-');
    }

    isBackgroundNone() {
        const sel = this.backgroundSelectEl;
        if (!sel || typeof sel.value !== 'string') return true; // permissive default
        return sel.value === 'none' || sel.value === '';
    }

    // ── Helpers ─────────────────────────────────────────────────────

    _currentBackgroundColor() {
        const scene = this.viewport.scene;
        if (scene.background && scene.background.isColor) return scene.background.clone();
        return new THREE.Color(DEFAULTS.backgroundColor);
    }
}

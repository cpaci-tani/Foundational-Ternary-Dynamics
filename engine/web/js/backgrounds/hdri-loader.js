/**
 * HDRI environment loader for 360 degree scene backgrounds.
 *
 * Equirectangular HDRIs from Poly Haven (CC0) loaded via RGBELoader.
 * Set as scene.background + scene.environment for realistic IBL.
 */
import * as THREE from 'three';
import { RGBELoader } from 'three/addons/loaders/RGBELoader.js';

export const HDRI_BASE = 'https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/1k/';

export const HDRI_ENVIRONMENTS = {
    studio:    { label: 'Studio',       file: 'studio_small_09_1k.hdr' },
    workshop:  { label: 'Workshop',     file: 'machine_shop_02_1k.hdr' },
    sunset:    { label: 'Sunset',       file: 'kloofendal_48d_partly_cloudy_puresky_1k.hdr' },
    night:     { label: 'Night Sky',    file: 'moonless_golf_1k.hdr' },
    forest:    { label: 'Forest',       file: 'syferfontein_18d_clear_puresky_1k.hdr' },
    urban:     { label: 'Urban',        file: 'potsdamer_platz_1k.hdr' },
};

/**
 * Construct a reusable RGBELoader. One instance can serve many loads —
 * the BackgroundManager holds one and uses it for every HDRI URL.
 */
export function createHDRILoader() {
    return new RGBELoader();
}

/**
 * Load an HDRI by file name and invoke onLoad(texture) / onError(err).
 * The caller is responsible for caching and for setting EquirectangularReflectionMapping
 * if desired (see applyHDRITexture).
 */
export function loadHDRI(loader, file, onLoad, onError) {
    const url = HDRI_BASE + file;
    loader.load(url, onLoad, undefined, (err) => {
        if (onError) onError(err, url);
        else console.warn(`Failed to load HDRI from ${url}:`, err);
    });
}

/**
 * Apply a loaded HDRI texture to a scene as both background and
 * image-based-lighting environment.
 */
export function applyHDRITexture(scene, texture) {
    texture.mapping = THREE.EquirectangularReflectionMapping;
    scene.background = texture;
    scene.environment = texture;
    scene.backgroundIntensity = 0.8;
    scene.backgroundBlurriness = 0.0;
}

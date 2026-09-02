// engine/web/js/scales/scale1/overlay-billboards.js
/**
 * Canvas-texture billboard-sprite factories for Scale-1 native-record
 * overlays (admissibility ring and provenance label).
 * Generalizes the existing pattern in engine/web/js/cosmic/sprites.js
 * (CanvasTexture -> THREE.Sprite) rather than introducing a new rendering
 * technique (no CSS2DRenderer, no DOM-projection layer).
 */
import * as THREE from 'three';

/**
 * A ring/halo texture: solid stroke for admissible, dashed for marginal.
 * @param {{color?: string, dashed?: boolean}} [opts]
 */
export function makeRingTexture({ color = '#4ade80', dashed = false } = {}) {
    const c = document.createElement('canvas');
    c.width = 128; c.height = 128;
    const ctx = c.getContext('2d');
    ctx.strokeStyle = color;
    ctx.lineWidth = 6;
    if (dashed) ctx.setLineDash([10, 8]);
    ctx.beginPath();
    ctx.arc(64, 64, 52, 0, Math.PI * 2);
    ctx.stroke();
    return new THREE.CanvasTexture(c);
}

/**
 * Short text on a transparent canvas, sized to fit. Used for provenance
 * labels and mass-delta badges — same technique, different text.
 * @param {string} text
 * @param {{color?: string, fontPx?: number}} [opts]
 */
export function makeTextTexture(text, { color = '#e8e8e8', fontPx = 28 } = {}) {
    const c = document.createElement('canvas');
    c.width = 256; c.height = 64;
    const ctx = c.getContext('2d');
    ctx.font = `${fontPx}px monospace`;
    ctx.fillStyle = color;
    ctx.textAlign = 'center';
    ctx.textBaseline = 'middle';
    ctx.fillText(text, 128, 32);
    return new THREE.CanvasTexture(c);
}

/**
 * Build a billboard THREE.Sprite from a CanvasTexture, sized in world units.
 * @param {THREE.CanvasTexture} texture
 * @param {number} worldSize - sprite width/height in scene units
 */
export function makeBillboardSprite(texture, worldSize = 2.0) {
    const material = new THREE.SpriteMaterial({
        map: texture, transparent: true, depthWrite: false, sizeAttenuation: true,
    });
    const sprite = new THREE.Sprite(material);
    sprite.scale.set(worldSize, worldSize, 1);
    return sprite;
}

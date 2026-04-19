/**
 * cosmic/sprites.js — procedural canvas-texture factories for CosmicRenderer.
 * Extracted from cosmic-renderer.js. Each factory builds a CanvasTexture
 * independently; callers cache the result.
 */

import * as THREE from 'three';

export function makeStarSprite() {
    const c = document.createElement('canvas');
    c.width = 128; c.height = 128;
    const ctx = c.getContext('2d');
    const cx = 64, cy = 64;

    // Soft radial glow
    const grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, 64);
    grad.addColorStop(0, 'rgba(255,255,255,1)');
    grad.addColorStop(0.08, 'rgba(255,255,255,0.9)');
    grad.addColorStop(0.2, 'rgba(255,240,220,0.4)');
    grad.addColorStop(0.45, 'rgba(255,200,150,0.1)');
    grad.addColorStop(0.7, 'rgba(200,150,255,0.03)');
    grad.addColorStop(1, 'rgba(0,0,0,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 128, 128);

    // Diffraction cross (4-point star)
    ctx.globalCompositeOperation = 'lighter';
    for (let angle = 0; angle < 4; angle++) {
        const a = angle * Math.PI / 2;
        const gd = ctx.createLinearGradient(
            cx, cy,
            cx + Math.cos(a) * 60, cy + Math.sin(a) * 60
        );
        gd.addColorStop(0, 'rgba(255,255,255,0.5)');
        gd.addColorStop(0.3, 'rgba(255,255,255,0.08)');
        gd.addColorStop(1, 'rgba(255,255,255,0)');
        ctx.strokeStyle = gd;
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.moveTo(cx, cy);
        ctx.lineTo(cx + Math.cos(a) * 58, cy + Math.sin(a) * 58);
        ctx.stroke();
    }

    return new THREE.CanvasTexture(c);
}

export function makeGasSprite() {
    const c = document.createElement('canvas');
    c.width = 128; c.height = 128;
    const ctx = c.getContext('2d');
    // Soft cloud with irregular edges
    const grad = ctx.createRadialGradient(64, 64, 0, 64, 64, 64);
    grad.addColorStop(0, 'rgba(255,255,255,0.6)');
    grad.addColorStop(0.25, 'rgba(255,255,255,0.3)');
    grad.addColorStop(0.5, 'rgba(255,255,255,0.1)');
    grad.addColorStop(0.75, 'rgba(255,255,255,0.03)');
    grad.addColorStop(1, 'rgba(255,255,255,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 128, 128);
    return new THREE.CanvasTexture(c);
}

export function makeHaloSprite() {
    const c = document.createElement('canvas');
    c.width = 256; c.height = 256;
    const ctx = c.getContext('2d');
    const grad = ctx.createRadialGradient(128, 128, 0, 128, 128, 128);
    grad.addColorStop(0, 'rgba(255,255,255,0.25)');
    grad.addColorStop(0.15, 'rgba(255,200,100,0.15)');
    grad.addColorStop(0.35, 'rgba(200,100,255,0.06)');
    grad.addColorStop(0.6, 'rgba(100,50,200,0.02)');
    grad.addColorStop(1, 'rgba(0,0,0,0)');
    ctx.fillStyle = grad;
    ctx.fillRect(0, 0, 256, 256);
    return new THREE.CanvasTexture(c);
}
